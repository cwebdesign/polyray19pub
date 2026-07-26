/* csg.cc

   Polyray - MIT Licensed Revival
  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Copyright (C) 1999-2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", (C), WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <utility>

#include "defs3.h"
#include "io_ply.h"
#include "memory.h"
#include "vector.h"
#include "csg.h"
#include "runtime_state.h"
#include "symtab.h"
#include "bound.h"
#include "factory.h" //factory methods

void CSGRender(Viewpoint *, BinTree *, Object *);
int CSGIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                 Flt mindist, Flt maxdist, Isect *hit);
int CSGInside(Object *, Vec);
void CSGCopy(Object*, Object*);

/* The allowed types of CSG objects:
   T_CLIP, T_UNION, T_INTERSECTION, T_MERGE, T_INVERSE, T_BASE_OBJECT
*/

openpolyray::dispatch::ObjectProcs CSGProcs = {
   .render = CSGRender,
   .evaluate = nullptr,
   .initialize = GenericInitialize,
   .intersect = CSGIntersect,
   .inside = CSGInside,
   .copy = CSGCopy,
   .del = CSGDelete,
   };

typedef struct csg_stack_struct *csg_stack_ptr;
struct csg_stack_struct {
   csgnodeptr node, parent;
   bbox_info bbox;
   csg_stack_ptr next;
   };

/**
 * Push a CSG node/parent pair onto the traversal stack.
 * @param node Node to store in the new stack entry.
 * @param parent Parent node associated with `node`.
 * @param stack Current stack head.
 * @return New stack head containing the pushed entry.
 */
static csg_stack_ptr push_csg_node(csgnodeptr node, csgnodeptr parent, csg_stack_ptr stack)
{
   csg_stack_ptr entry;
   entry = (csg_stack_ptr)polyray_malloc(sizeof(struct csg_stack_struct));
   if (entry == nullptr)
      serror("Out of memory");
   entry->node = node;
   entry->parent = parent;
   entry->next = stack;
   return entry;
}

/**
 * Pop the current traversal stack entry and return the next head.
 * @param node Receives the node stored in the popped entry.
 * @param parent Receives the parent stored in the popped entry.
 * @param stack Current stack head to remove.
 * @return New stack head after removing the top entry.
 */
static csg_stack_ptr pop_csg_node(csgnodeptr *node, csgnodeptr *parent, csg_stack_ptr stack)
{
   csg_stack_ptr entry;
   entry = stack;
   stack = stack->next;
   *node = entry->node;
   *parent = entry->parent;
   polyray_free(entry);
   return stack;
}

/**
 * Instantiate all primitive members contained in a CSG tree.
 * @param root Scene bin tree that receives instantiated primitives.
 * @param node Root CSG node to expand.
 * @param displ_flag Nonzero when displacement processing is already required.
 * @return No return value.
 */
void instantiate_csg(BinTree *root, csgnodeptr node, int displ_flag)
{
   rmode old_method;
   int OldOptim;
   Object *tobj1, *tobj2;
   BinTree temp_root;
   csgnodeptr tnode, tparent;
   csg_stack_ptr stack;

   stack = push_csg_node(node, nullptr, nullptr);
   while (stack != nullptr) {
      /* Pull off the top of the csg stack */
      stack = pop_csg_node(&tnode, &tparent, stack);
      switch (tnode->type) {
      case std::to_underlying(ShapeType::Base_Object):
         tobj1 = (Object *)tnode->left;
         displ_flag = 0;
         for (tobj2=tobj1;tobj2!=nullptr;tobj2=tobj2->o_parent)
            if (tobj2->o_displace)
               displ_flag = 1;
         if (tobj1->o_type != ShapeType::Csg) {
            /* This is a primitive object, instantiate it */
            tobj2 = FactoryObject();
            if (tobj2 == nullptr)
               serror("Failed to allocate CSG node\n");
            Copy_Object(tobj1, *tobj2);
            if ((runtimeState::settings.Render_Method == rmode::RAY_TRACING ||
                ((runtimeState::settings.Render_Method == rmode::SCAN_CONVERSION) &&
                 (runtimeState::scene.Global_Shade_Flag &
                  (SHADOW_CHECK | REFLECT_CHECK | TRANSMIT_CHECK)))) &&
               (displ_flag || tobj1->o_type == ShapeType::Bezier ||
                tobj1->o_type == ShapeType::Nurb || 
                tobj1->o_type == ShapeType::Parametric)) {
               old_method = runtimeState::settings.Render_Method;
               runtimeState::settings.Render_Method = rmode::MESH_CONVERSION;

               /* Create a temporary BinTree to hold the polygons as they are
                  made by the scan converter */
               Initialize_BinTree(temp_root);
               tobj2->o_procs->render(nullptr, &temp_root, tobj2);
               OldOptim = RuntimeState::settings.Optimizer;
               RuntimeState::settings.Optimizer = 1;
               BuildBoundingSlabs(temp_root);//&temp_root in old code
               RuntimeState::settings.Optimizer = OldOptim;

               /* Now add the slabbed patch pieces to the global set of
                  objects */
               if (temp_root.slab_root == nullptr)
                  serror("Failed to process triangulated object");

               root->members.list = push_object(root->members.list,
                                                temp_root.slab_root);
               root->members.count++;
               while (temp_root.members.list != nullptr)
                  pop_object(&temp_root.members.list);

               root->polyprims.list = push_object(root->polyprims.list, tobj2);
               root->polyprims.count++;
               runtimeState::settings.Render_Method = old_method;
               }
            else {
               root->members.list = push_object(root->members.list, tobj2);
               root->members.count++;
               }
            }
         else
            instantiate_csg(root, (csgnodeptr)tobj1->o_data, displ_flag);
         break;
      case std::to_underlying(ShapeType::Merge):
      case std::to_underlying(ShapeType::Intersection):
      case std::to_underlying(ShapeType::Union):
         stack = push_csg_node((csgnodeptr)tnode->left, nullptr, stack);
         stack = push_csg_node((csgnodeptr)tnode->right, nullptr, stack);
         break;
      case std::to_underlying(ShapeType::Inverse):
      case std::to_underlying(ShapeType::Clip):
         stack = push_csg_node((csgnodeptr)tnode->left, nullptr, stack);
         break;
      default:
         serror("Bad CSG node type in instantiate_csg: %d\n", tnode->type);
      }
      }
}

/* Note that in set_parent_ptrs, the order that the objects are pushed
   onto the stack is the most efficient.  Since the YACC parser creates
   a tree of CSG objects that is generally deepest to the left, by
   pushing left and then right, we will almost always be popping a base
   object rather than another tree.  If the parser changes, then this
   routine should be revisited to see if this is still true. */
/**
 * Propagate parent links, transforms, and constrained bounds through a CSG tree.
 * @param node Current CSG node being processed.
 * @param parent Parent CSG node for `node`.
 * @param obj Owning object for primitive descendants.
 * @param world_tx Accumulated transform inherited from ancestor objects.
 * @param box Bounding box constraints inherited from ancestor CSG operations.
 * @return No return value.
 */
void set_parent_ptrs(csgnodeptr node, csgnodeptr parent, Object *obj,
                Transform *world_tx, bbox_info *box)
{
   bbox_info ibox;
   Object *tobj;
   csg_stack_ptr stack;
   csgnodeptr tnode, tparent;

   stack = push_csg_node(node, parent, nullptr);

   while (stack != nullptr) {
      /* Pull off the top of the csg stack */
      stack = pop_csg_node(&tnode, &tparent, stack);

      tnode->parent = tparent;
      switch (tnode->type) {
      case std::to_underlying(ShapeType::Base_Object):
         tobj = (Object *)tnode->left;
         tobj->o_parent = obj;
         tobj->o_csg_tree = tnode;

         /* Keep track of the transformations that have been applied
            up to this point */
         if (world_tx != nullptr) {
            if (tobj->o_trans == nullptr)
               tobj->o_trans = Get_Transformation().release();
            Compose_Transformations(*tobj->o_trans, *world_tx);
            recompute_bbox(&tobj->o_bnd, world_tx);
            }
   
         /* The bounds on this object must be as tight as both its own
            bounds and the bounds imposed on its parent objects */
         bbox_intersect(box, &tobj->o_bnd, &ibox);
   
         /* See if we need to go any farther down the tree */
         if (tobj->o_type == ShapeType::Csg)
            set_parent_ptrs((csgnodeptr)tobj->o_data, tnode, tobj, tobj->o_trans, &ibox);
         else
            tobj->o_bnd = ibox;
         break;
      case std::to_underlying(ShapeType::Union):
         stack = push_csg_node((csgnodeptr)tnode->left,  tnode, stack);
         stack = push_csg_node((csgnodeptr)tnode->right, tnode, stack);
         break;
      case std::to_underlying(ShapeType::Merge):
      case std::to_underlying(ShapeType::Intersection):
      case std::to_underlying(ShapeType::Clip):
         set_parent_ptrs((csgnodeptr)tnode->left, tnode, obj, world_tx, box);
         set_parent_ptrs((csgnodeptr)tnode->right, tnode, obj, world_tx, box);
         break;
      case std::to_underlying(ShapeType::Inverse):
         set_parent_ptrs((csgnodeptr)tnode->left, tnode, obj, world_tx, box);
         break;
      default:
         serror("Bad CSG node type in set_parent_ptrs: %d\n", tnode->type);
      }
      }
}

/**
 * CSG objects are expanded into component primitives before intersection time.
 * @param Eye Viewpoint issuing the ray.
 * @param obj CSG object being queried.
 * @param ray Ray to intersect.
 * @param mindist Minimum valid hit distance.
 * @param maxdist Maximum valid hit distance.
 * @param hit Intersection list that would receive hits.
 * @return Always `0` because intersections are handled by instantiated children.
 */
int CSGIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
             Flt mindist, Flt maxdist, Isect *hit)
{
   /* No longer intersecting the CSG object, only it's component objects */
   return 0;
}

/* This routine has to be called after set_parent_ptrs in order to
   have the correct transform information in each object. */
/**
 * Compute a conservative bounding box that encloses all base objects in a CSG tree.
 * @param box Receives the union of all leaf-object bounds.
 * @param node Root CSG node whose bounds are being accumulated.
 * @return No return value.
 */
static void set_csg_bounds(bbox_info *box, csgnodeptr node)
{
   bbox_info ubox;
   csg_stack_ptr stack;
   csgnodeptr tnode, tparent;
   int flag = 0;

   stack = push_csg_node(node, nullptr, nullptr);

   while (stack != nullptr) {
      /* Pull off the top of the csg stack */
      stack = pop_csg_node(&tnode, &tparent, stack);

      switch (tnode->type) {
      case std::to_underlying(ShapeType::Base_Object):
         ubox = ((Object *)tnode->left)->o_bnd;
         if (flag)
            bbox_union(box, &ubox, box);
         else {
            *box = ubox;
            flag = 1;
            }

         break;
      case std::to_underlying(ShapeType::Merge):
      case std::to_underlying(ShapeType::Intersection):
      case std::to_underlying(ShapeType::Clip):
      case std::to_underlying(ShapeType::Union):
         stack = push_csg_node((csgnodeptr)tnode->left,  nullptr, stack);
         stack = push_csg_node((csgnodeptr)tnode->right, nullptr, stack);
         break;
      case std::to_underlying(ShapeType::Inverse):
         stack = push_csg_node((csgnodeptr)tnode->left,  nullptr, stack);
         break;
      default:
         serror("Bad CSG node type in set_csg_bounds: %d\n", tnode->type);
      }
      }
}

/**
 * Initialize an object as a CSG container.
 * @param obj Object being configured as a CSG object.
 * @param data Root node of the CSG expression tree.
 * @return `obj` configured as a CSG object.
 */
Object *MakeCSG(Object *obj, csgnodeptr data)
{
   obj->o_type = ShapeType::Csg;
   obj->o_procs = &CSGProcs;
   obj->o_data = (void *)data;
   obj->o_trans = nullptr;

   set_csg_bounds(&obj->o_bnd, data);
#if 0
printf("Bnd: <%g,%g,%g> - <%g,%g,%g>\n",
       obj->o_bnd.lower_left[0],
       obj->o_bnd.lower_left[1],
       obj->o_bnd.lower_left[2],
       obj->o_bnd.lengths[0],
       obj->o_bnd.lengths[1],
       obj->o_bnd.lengths[2]);
#endif

   return obj;
}

/* Climbs up a CSG node tree, checking to see if a point is inside */
/**
 * Test whether a point remains inside all ancestor constraints of a CSG leaf node.
 * @param node Leaf or subtree node from which to climb toward the CSG root.
 * @param W Point to test in object/world space expected by the child primitives.
 * @return `1` when the point satisfies all ancestor CSG operations, otherwise `0`.
 */
int Inside_CSG_Node(csgnodeptr node, Vec W)
{
   csgnodeptr tnode;
   int flag = 1;

   if (node == nullptr)
      return 1;
   for (tnode=node,node=node->parent;
        node!=nullptr;
        tnode=tnode->parent,node=node->parent) {
      switch (node->type) {
         case std::to_underlying(ShapeType::Base_Object):
         case std::to_underlying(ShapeType::Union):
         case std::to_underlying(ShapeType::Inverse):
            break;
         case std::to_underlying(ShapeType::Clip):
            if (node->left == tnode)
               flag = Inside_CSG_Nodes((csgnodeptr)node->right, W);
            else
               swarning("Bad clipping point");
            break;
         case std::to_underlying(ShapeType::Intersection):
            if (node->left == tnode)
               flag = Inside_CSG_Nodes((csgnodeptr)node->right, W);
            else
               flag = Inside_CSG_Nodes((csgnodeptr)node->left, W);
            break;
         case std::to_underlying(ShapeType::Merge):
            if (node->left == tnode)
               flag = 1 - Inside_CSG_Nodes((csgnodeptr)node->right, W);
            else
               flag = 1 - Inside_CSG_Nodes((csgnodeptr)node->left, W);
            break;
         default:
            serror("Bad CSG node type in inside_csg_node: %d\n", node->type);
         }
      if (!flag)
         return 0;
      }
   return 1;
}

/**
 * Evaluate point containment for an entire CSG expression subtree.
 * @param node Root of the CSG subtree to test.
 * @param P Point to test in object/world space expected by the child primitives.
 * @return `1` when `P` lies inside the subtree result, otherwise `0`.
 */
int Inside_CSG_Nodes(csgnodeptr node, Vec P)
{
   Object *tobj;

   if (node == nullptr)
      serror("nullptr node in Inside_CSG_Nodes\n");

   switch (node->type) {
   case std::to_underlying(ShapeType::Base_Object):
      tobj = (Object *)node->left;
      return (tobj->o_procs->inside)(tobj, P);
   case std::to_underlying(ShapeType::Intersection):
      if (Inside_CSG_Nodes((csgnodeptr)node->left, P) &&
          Inside_CSG_Nodes((csgnodeptr)node->right, P))
         return 1;
      break;
   case std::to_underlying(ShapeType::Clip):
      if (Inside_CSG_Nodes((csgnodeptr)node->left, P) &&
          !Inside_CSG_Nodes((csgnodeptr)node->right, P))
         return 1;
      break;
   case std::to_underlying(ShapeType::Merge):
   case std::to_underlying(ShapeType::Union):
      if (Inside_CSG_Nodes((csgnodeptr)node->left, P)) return 1;
      if (Inside_CSG_Nodes((csgnodeptr)node->right, P)) return 1;
      break;
   case std::to_underlying(ShapeType::Inverse):
      return 1 - Inside_CSG_Nodes((csgnodeptr)node->left, P);
   default:
      serror("Bad CSG node type in inside_csg_nodes: %d\n", node->type);
   }
   return 0;
}

/**
 * Test whether a point lies inside the result of a CSG object.
 * @param obj CSG object being queried.
 * @param Pos Point to test in object/world space expected by the CSG tree.
 * @return `1` when `Pos` is inside the CSG result, otherwise `0`.
 */
int CSGInside(Object *obj, Vec Pos)
{
   return Inside_CSG_Nodes((csgnodeptr)obj->o_data, Pos);
}

/**
 * Deep-copy a CSG expression tree and its base objects.
 * @param node Root node of the CSG subtree to copy.
 * @return Root node of the copied subtree.
 */
static csgnodeptr copy_csg_nodes(csgnodeptr node)
{
   csgnodeptr new_node = Factorycsgnode();

   if (new_node == nullptr)
      serror("Failed to allocate CSG node\n");
   new_node->type = node->type;
   switch (node->type) {
   case std::to_underlying(ShapeType::Base_Object):
      { Object *temp1, *temp2;
        temp1 = (Object *)node->left;
        temp2 = FactoryObject();
        if (temp2 == nullptr)
           serror("Failed to allocate CSG node\n");
        Copy_Object(temp1, *temp2);
        new_node->left = temp2;
        new_node->right = nullptr;
        }
      break;
   case std::to_underlying(ShapeType::Intersection):
   case std::to_underlying(ShapeType::Merge):
   case std::to_underlying(ShapeType::Union):
   case std::to_underlying(ShapeType::Clip):
      new_node->left  = copy_csg_nodes((csgnodeptr)node->left);
      new_node->right = copy_csg_nodes((csgnodeptr)node->right);
      break;
   case std::to_underlying(ShapeType::Inverse):
      new_node->left  = copy_csg_nodes((csgnodeptr)node->left);
      new_node->right = nullptr;
      break;
   default:
      serror("Bad CSG node type in copy_csg_nodes: %d\n", node->type);
   }
   return new_node;
}

/**
 * Copy the CSG-specific payload from one object into another.
 * @param objin Source object providing the CSG tree.
 * @param objout Destination object receiving the copied CSG tree.
 * @return No return value.
 */
void CSGCopy(Object *objin, Object *objout)
{
   objout->o_data = copy_csg_nodes((csgnodeptr)objin->o_data);
   objout->o_copy = 0;
}

/**
 * Recursively free a CSG tree and any owned base objects.
 * @param node Root node of the subtree to destroy.
 * @return No return value.
 */
static void delete_csg_nodes(csgnodeptr node)
{

   if (node == nullptr)
      return;
   if (node->type == std::to_underlying(ShapeType::Base_Object))
      Delete_Object((Object*)node->left);
   else {
      delete_csg_nodes((csgnodeptr)node->left);
      delete_csg_nodes((csgnodeptr)node->right);
      }
   delete node;
}

/**
 * Release the CSG tree owned by an object.
 * @param object CSG object whose tree is being deleted.
 * @return No return value.
 */
void CSGDelete(Object *object)
{
   delete_csg_nodes((csgnodeptr)object->o_data);
}

/**
 * CSG objects do not render directly because their members are instantiated separately.
 * @param eye Viewpoint used for rendering.
 * @param Root Scene bin tree that would receive rendered primitives.
 * @param Object CSG object being rendered.
 * @return No return value.
 */
void CSGRender(Viewpoint *eye, BinTree *Root, Object *Object)
{
   /* Since all the elements of a CSG node have been individually
      instantiated in the list of primitives, there is no longer
      a need to render the csg tree */
   return;
}

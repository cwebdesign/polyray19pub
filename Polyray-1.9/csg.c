/* csg.c

  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.

  This software may be used for any private and non-commercial
  use.

  You may not distribute this software, in whole or in part,
  for any commercial purpose, without the express consent of
  the authors.

  There is no warranty or other guarantee of fitness of this software
  for any purpose.  It is provided solely "as is".

*/

#include "defs.h"
#include "io.h"
#include "memory.h"
#include "vector.h"
#include "csg.h"
#include "symtab.h"
#include "bound.h"

void CSGRender(Viewpoint *, BinTree *, Object *);
int CSGIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                 Flt mindist, Flt maxdist, Isect *hit);
int CSGInside(Object *, Vec);
void CSGCopy(Object *, Object *);
void CSGDelete(Object *);

/* The allowed types of CSG objects:
   T_CLIP, T_UNION, T_INTERSECTION, T_MERGE, T_INVERSE, T_BASE_OBJECT
*/

ObjectProcs CSGProcs = {
   CSGRender,
   NULL,
   GenericInitialize,
   CSGIntersect,
   CSGInside,
   CSGCopy,
   CSGDelete,
   };

typedef struct csg_stack_struct *csg_stack_ptr;
struct csg_stack_struct {
   csgnodeptr node, parent;
   bbox_info bbox;
   csg_stack_ptr next;
   };

static csg_stack_ptr
push_csg_node(csgnodeptr node, csgnodeptr parent, csg_stack_ptr stack)
{
   csg_stack_ptr entry;
   entry = polyray_malloc(sizeof(struct csg_stack_struct));
   if (entry == NULL)
      error("Out of memory");
   entry->node = node;
   entry->parent = parent;
   entry->next = stack;
   return entry;
}

static csg_stack_ptr
pop_csg_node(csgnodeptr *node, csgnodeptr *parent, csg_stack_ptr stack)
{
   csg_stack_ptr entry;
   entry = stack;
   stack = stack->next;
   *node = entry->node;
   *parent = entry->parent;
   polyray_free(entry);
   return stack;
}

void
instantiate_csg(BinTree *root, csgnodeptr node, int displ_flag)
{
   int old_method, OldOptim;
   Object *tobj1, *tobj2;
   BinTree temp_root;
   csgnodeptr tnode, tparent;
   csg_stack_ptr stack;

   stack = push_csg_node(node, NULL, NULL);
   while (stack != NULL) {
      /* Pull off the top of the csg stack */
      stack = pop_csg_node(&tnode, &tparent, stack);
      switch (tnode->type) {
      case T_BASE_OBJECT:
         tobj1 = (Object *)tnode->left;
         displ_flag = 0;
         for (tobj2=tobj1;tobj2!=NULL;tobj2=tobj2->o_parent)
            if (tobj2->o_displace)
               displ_flag = 1;
         if (tobj1->o_type != T_CSG) {
            /* This is a primitive object, instantiate it */
            tobj2 = polyray_malloc(sizeof(Object));
            if (tobj2 == NULL)
               error("Failed to allocate CSG node\n");
            Copy_Object(tobj1, tobj2);
            if ((Rendering_Method == RAY_TRACING ||
                ((Rendering_Method == SCAN_CONVERSION) &&
                 (Global_Shade_Flag &
                  (SHADOW_CHECK | REFLECT_CHECK | TRANSMIT_CHECK)))) &&
               (displ_flag || tobj1->o_type == T_BEZIER ||
                tobj1->o_type == T_NURB || tobj1->o_type == T_PARAMETRIC)) {
               old_method = Rendering_Method;
               Rendering_Method = MESH_CONVERSION;

               /* Create a temporary BinTree to hold the polygons as they are
                  made by the scan converter */
               Initialize_BinTree(&temp_root);
               tobj2->o_procs->render(NULL, &temp_root, tobj2);
               OldOptim = Optimizer;
               Optimizer = 1;
               BuildBoundingSlabs(&temp_root);
               Optimizer = OldOptim;

               /* Now add the slabbed patch pieces to the global set of
                  objects */
               if (temp_root.slab_root == NULL)
                  error("Failed to process triangulated object");

               root->members.list = push_object(root->members.list,
                                                temp_root.slab_root);
               root->members.count++;
               while (temp_root.members.list != NULL)
                  pop_object(&temp_root.members.list);

               root->polyprims.list = push_object(root->polyprims.list, tobj2);
               root->polyprims.count++;
               Rendering_Method = old_method;
               }
            else {
               root->members.list = push_object(root->members.list, tobj2);
               root->members.count++;
               }
            }
         else
            instantiate_csg(root, tobj1->o_data, displ_flag);
         break;
      case T_MERGE:
      case T_INTERSECTION:
      case T_UNION:
         stack = push_csg_node(tnode->left, NULL, stack);
         stack = push_csg_node(tnode->right, NULL, stack);
         break;
      case T_INVERSE:
      case T_CLIP:
         stack = push_csg_node(tnode->left, NULL, stack);
         break;
      default:
         error("Bad CSG node type in instantiate_csg: %d\n", tnode->type);
      }
      }
}

/* Note that in set_parent_ptrs, the order that the objects are pushed
   onto the stack is the most efficient.  Since the YACC parser creates
   a tree of CSG objects that is generally deepest to the left, by
   pushing left and then right, we will almost always be popping a base
   object rather than another tree.  If the parser changes, then this
   routine should be revisited to see if this is still true. */
void
set_parent_ptrs(csgnodeptr node, csgnodeptr parent, Object *obj,
                Transform *world_tx, bbox_info *box)
{
   bbox_info ibox;
   Object *tobj;
   csg_stack_ptr stack;
   csgnodeptr tnode, tparent;

   stack = push_csg_node(node, parent, NULL);

   while (stack != NULL) {
      /* Pull off the top of the csg stack */
      stack = pop_csg_node(&tnode, &tparent, stack);

      tnode->parent = tparent;
      switch (tnode->type) {
      case T_BASE_OBJECT:
         tobj = (Object *)tnode->left;
         tobj->o_parent = obj;
         tobj->o_csg_tree = tnode;

         /* Keep track of the transformations that have been applied
            up to this point */
         if (world_tx != NULL) {
            if (tobj->o_trans == NULL)
               tobj->o_trans = Get_Transformation();
            Compose_Transformations(tobj->o_trans, world_tx);
            recompute_bbox(&tobj->o_bnd, world_tx);
            }
   
         /* The bounds on this object must be as tight as both its own
            bounds and the bounds imposed on its parent objects */
         bbox_intersect(box, &tobj->o_bnd, &ibox);
   
         /* See if we need to go any farther down the tree */
         if (tobj->o_type == T_CSG)
            set_parent_ptrs(tobj->o_data, tnode, tobj, tobj->o_trans, &ibox);
         else
            tobj->o_bnd = ibox;
         break;
      case T_UNION:
         stack = push_csg_node(tnode->left,  tnode, stack);
         stack = push_csg_node(tnode->right, tnode, stack);
         break;
      case T_MERGE:
      case T_INTERSECTION:
      case T_CLIP:
         set_parent_ptrs(tnode->left, tnode, obj, world_tx, box);
         set_parent_ptrs(tnode->right, tnode, obj, world_tx, box);
         break;
      case T_INVERSE:
         set_parent_ptrs(tnode->left, tnode, obj, world_tx, box);
         break;
      default:
         error("Bad CSG node type in set_parent_ptrs: %d\n", tnode->type);
      }
      }
}

int
CSGIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
             Flt mindist, Flt maxdist, Isect *hit)
{
   /* No longer intersecting the CSG object, only it's component objects */
   return 0;
}

/* This routine has to be called after set_parent_ptrs in order to
   have the correct transform information in each object. */
static void
set_csg_bounds(bbox_info *box, csgnodeptr node)
{
   bbox_info ubox;
   csg_stack_ptr stack;
   csgnodeptr tnode, tparent;
   int flag = 0;

   stack = push_csg_node(node, NULL, NULL);

   while (stack != NULL) {
      /* Pull off the top of the csg stack */
      stack = pop_csg_node(&tnode, &tparent, stack);

      switch (tnode->type) {
      case T_BASE_OBJECT:
         ubox = ((Object *)tnode->left)->o_bnd;
         if (flag)
            bbox_union(box, &ubox, box);
         else {
            *box = ubox;
            flag = 1;
            }

         break;
      case T_MERGE:
      case T_INTERSECTION:
      case T_CLIP:
      case T_UNION:
         stack = push_csg_node(tnode->left,  NULL, stack);
         stack = push_csg_node(tnode->right, NULL, stack);
         break;
      case T_INVERSE:
         stack = push_csg_node(tnode->left,  NULL, stack);
         break;
      default:
         error("Bad CSG node type in set_csg_bounds: %d\n", tnode->type);
      }
      }
}

Object *
MakeCSG(Object *obj, csgnodeptr data)
{
   obj->o_type = T_CSG;
   obj->o_procs = &CSGProcs;
   obj->o_data = (void *)data;
   obj->o_trans = NULL;

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
int
Inside_CSG_Node(csgnodeptr node, Vec W)
{
   csgnodeptr tnode;
   int flag = 1;

   if (node == NULL)
      return 1;
   for (tnode=node,node=node->parent;
        node!=NULL;
        tnode=tnode->parent,node=node->parent) {
      switch (node->type) {
         case T_BASE_OBJECT:
         case T_UNION:
         case T_INVERSE:
            break;
         case T_CLIP:
            if (node->left == tnode)
               flag = Inside_CSG_Nodes(node->right, W);
            else
               warning("Bad clipping point");
            break;
         case T_INTERSECTION:
            if (node->left == tnode)
               flag = Inside_CSG_Nodes(node->right, W);
            else
               flag = Inside_CSG_Nodes(node->left, W);
            break;
         case T_MERGE:
            if (node->left == tnode)
               flag = 1 - Inside_CSG_Nodes(node->right, W);
            else
               flag = 1 - Inside_CSG_Nodes(node->left, W);
            break;
         default:
            error("Bad CSG node type in inside_csg_node: %d\n", node->type);
         }
      if (!flag)
         return 0;
      }
   return 1;
}

int
Inside_CSG_Nodes(csgnodeptr node, Vec P)
{
   Object *tobj;

   if (node == NULL)
      error("NULL node in Inside_CSG_Nodes\n");

   switch (node->type) {
   case T_BASE_OBJECT:
      tobj = (Object *)node->left;
      return (tobj->o_procs->inside)(tobj, P);
   case T_INTERSECTION:
      if (Inside_CSG_Nodes(node->left, P) &&
          Inside_CSG_Nodes(node->right, P))
         return 1;
      break;
   case T_CLIP:
      if (Inside_CSG_Nodes(node->left, P) &&
          !Inside_CSG_Nodes(node->right, P))
         return 1;
      break;
   case T_MERGE:
   case T_UNION:
      if (Inside_CSG_Nodes(node->left, P)) return 1;
      if (Inside_CSG_Nodes(node->right, P)) return 1;
      break;
   case T_INVERSE:
      return 1 - Inside_CSG_Nodes(node->left, P);
   default:
      error("Bad CSG node type in inside_csg_nodes: %d\n", node->type);
   }
   return 0;
}

int
CSGInside(Object *obj, Vec Pos)
{
   return Inside_CSG_Nodes(obj->o_data, Pos);
}

static csgnodeptr
copy_csg_nodes(csgnodeptr node)
{
   csgnodeptr new_node = polyray_malloc(sizeof(struct csgnode));

   if (node == NULL)
      error("Failed to allocate CSG node\n");
   new_node->type = node->type;
   switch (node->type) {
   case T_BASE_OBJECT:
      { Object *temp1, *temp2;
        temp1 = (Object *)node->left;
        temp2 = polyray_malloc(sizeof(Object));
        if (temp2 == NULL)
           error("Failed to allocate CSG node\n");
        Copy_Object(temp1, temp2);
        new_node->left = temp2;
        new_node->right = NULL;
        }
      break;
   case T_INTERSECTION:
   case T_MERGE:
   case T_UNION:
   case T_CLIP:
      new_node->left  = copy_csg_nodes(node->left);
      new_node->right = copy_csg_nodes(node->right);
      break;
   case T_INVERSE:
      new_node->left  = copy_csg_nodes(node->left);
      new_node->right = NULL;
      break;
   default:
      error("Bad CSG node type in copy_csg_nodes: %d\n", node->type);
   }
   return new_node;
}

void
CSGCopy(Object *objin, Object *objout)
{
   objout->o_data = copy_csg_nodes(objin->o_data);
   objout->o_copy = 0;
}

static void
delete_csg_nodes(csgnodeptr node)
{

   if (node == NULL)
      return;
   else if (node->type == T_BASE_OBJECT)
      Delete_Object(node->left);
   else {
      delete_csg_nodes(node->left);
      delete_csg_nodes(node->right);
      }
   polyray_free(node);
}

void
CSGDelete(Object *object)
{
   delete_csg_nodes(object->o_data);
}

void
CSGRender(Viewpoint *eye, BinTree *Root, Object *Object)
{
   /* Since all the elements of a CSG node have been individually
      instantiated in the list of primitives, there is no longer
      a need to render the csg tree */
   return;
}

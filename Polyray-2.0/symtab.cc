/**
 * @file symtab.cc
 * @brief Symbol table, object lifecycle, and scene-global support routines.
 *
 * Support routines for reusable objects, strings, vectors, and scene setup.
 *
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

#include <cstdarg>
#include <new>
#include <string>
#include <string_view>
#include <algorithm>

#ifdef DEBUG_POINTERS
#include <unordered_set>
#endif
#include <gsl/gsl>

#include "defs3.h"

#define SI_NO_CONVERSION //for SimpleIni.h - no conversion
#include "SimpleIni.h"

#define IN_SYMTABC
#include "runtime_state.h"
#include "symtab.h"
#include "bound.h"
#include "particle.h"
#include "glyph.h"
#include "sweep.h"
#include "height.h"
#include "display.h"
#include "height.h"
#include "memory.h"
#include "io_ply.h"
#include "polynom.h"
#include "revolve.h"
#include "raw.h"
#include "eval.h"
#include "builder.h"
#include "csg.h"
#include "scan.h"
#include "texture.h"
#include "function.h"
#include "display.h"
#include "intersec.h"
#include "util.h"
#include "superq.h"
#include "sweep.h"
#include "subdiv.h"
#include "blob.h"
#include "box.h"
#include "bezier.h"
#include "cone.h"
#include "sphere.h"
#include "disc.h"
#include "cylinder.h"
#include "glyph.h"
#include "gridded.h"
#include "hypertex.h"
#include "light.h"
#include "parabola.h"
#include "parametr.h"
#include "poly.h"
#include "torus.h"
#include "tri.h"
#include "superq.h"
#include "factory.h"

#include "lex.yy.h"//for YY_BUFFER_STATE via Re-Flex
#include "enqueue.h"

#ifdef DEBUG_POINTERS
namespace {
std::unordered_set<const Object*> g_deleted_objects;
}
#endif


void CallSpecificDeleteObj(Object& obj);

DrawNode *Draw_Commands = nullptr;





/* Particle variables */
Particle *CurrentParticle = nullptr;
Particle *Particles = nullptr;
ParticleObject *ParticleObjects = nullptr;

/* Clipping variables */
Poly_box pbox;
Window win;
Vec ViewVec;


/* Use this define to print each function name as it is entered */
/* #define DEBUG_FN_CALLS */

/* Data structures and routines for maintaining name/value pairs in a
   2-3 tree */
typedef struct key_data *nodedata;
typedef struct node *nodeptr;

/** @brief A key/value pair stored as a leaf in the 2-3 symbol-table tree.
 *
 *  Multiple definitions of the same name form a linked list via @c next,
 *  supporting the overload-stack behaviour used for scoped definitions.
 */
struct key_data {
   std::string key;  /**< Interned symbol name. */
   void       *data; /**< Opaque payload (tokenptr cast to void*). */
   nodedata    next; /**< Next shadowed definition for this name, or nullptr. */
   };

/** @brief Internal 2-3 tree leaf node payload. */
struct leaf_entry {
   nodedata leaf_data; /**< The key/value pair stored at this leaf. */
   };

/** @brief Internal 2-3 tree two-child (2-node) internal node. */
struct two_entry {
   nodeptr  left_two_tree,  /**< Left subtree (keys < two_data). */
            right_two_tree; /**< Right subtree (keys >= two_data). */
   nodedata two_data;       /**< Separator key copied up from the left subtree's maximum. */
   };

/** @brief Internal 2-3 tree three-child (3-node) internal node. */
struct three_entry {
   nodeptr  left_three_tree,   /**< Left subtree (keys < left_three_data). */
            middle_three_tree, /**< Middle subtree (left_three_data <= keys < right_three_data). */
            right_three_tree;  /**< Right subtree (keys >= right_three_data). */
   nodedata left_three_data,   /**< Separator key for the left/middle boundary. */
            right_three_data;  /**< Separator key for the middle/right boundary. */
   };

/** @brief Discriminant for the polymorphic @c node union. */
enum NodeFlags {
   LEAF_NODE  = 1, /**< Node holds a leaf_entry (key/value pair). */
   TWO_NODE   = 2, /**< Node holds a two_entry (two children). */
   THREE_NODE = 3  /**< Node holds a three_entry (three children). */
};

/** @brief A node in the 2-3 balanced search tree used as the symbol table. */
struct node {
   int            index; /**< Reserved/unused order index. */
   enum NodeFlags kind;  /**< Discriminant selecting the active union member. */
   union {
      struct leaf_entry  leaf;  /**< Valid when kind == LEAF_NODE. */
      struct two_entry   two;   /**< Valid when kind == TWO_NODE. */
      struct three_entry three; /**< Valid when kind == THREE_NODE. */
      } data;
   };


static nodeptr Token_Tree = nullptr;
static nodeptr New_Token_Tree = nullptr;

/* Main symbol table data structure */
typedef struct token_struct *tokenptr;
/** @brief One symbol-table entry (definition of a named scene-language identifier). */
struct token_struct {
   std::string sname; /**< Stored symbol name. */
   int      type;     /**< Type tag (T_OBJECT, T_TEXTURE, T_EXPRESSION, ...). */
   int      sflag;    /**< Non-zero when this is a static (persistent across frames) definition. */
   void    *data;     /**< Owned payload; type-specific deallocation via Delete_Definition(). */
   tokenptr next;     /**< Next overloaded definition for the same name, or nullptr. */
   };
/** @brief Callback signature for symbol-table traversal (shread / process). */
typedef void (*datafunc)(const char *token, tokenptr data);

/** @brief Allocate and default-construct a new token_struct.
 *  @return Pointer to the freshly allocated entry; aborts on OOM via GSL Ensures.
 */
static token_struct* FactoryTokenStruc(void)
{
    auto new_p = new token_struct();//works
    //auto new_node = std::make_unique<struct exper_node_struct>();
    Ensures(new_p != nullptr);
    return new_p;
}

#if 0
/* Routines to manipulate the symbol table */
void Initialize_Symbol_Table(void);
void Terminate_Symbol_Table(datafunc process);
void Show_Symbol_Table(void);

void Install_Token(const char *token, tokenptr value);
void Delete_Token(const char *token);
tokenptr Lookup_Token(const char *token);
void Push_Token(const char *token, tokenptr data);
tokenptr Pop_Token(const char *token);

void Process_Symbol_Table(datafunc process);
#endif

/**
 * @brief Case-insensitive comparison helper. still used in test.cpp so might move it elsewhere
 * @param str1 Left-hand string view.
 * @param str2 Right-hand string that will be upper-cased before compare.
 * @return Result of std::string_view::compare semantics.
 */
int istrcmp(std::string_view str1, const std::string& str2)
{
    
    return str1.compare(ToUpperString(str2));
}


/**
 * @brief Initialize a substitution/evaluation context with default vectors.
 * @param subst Substitution context to reset.
 */
void reset_subst(SUBST_PTR subst)
{
   /* Set default values for the evaluation structure */
   MakeVector(0, 0, 0, subst->U);
   MakeVector(0, 0, 0, subst->UT);
   MakeVector(0, 0, 0, subst->P);
   MakeVector(0, 0, 0, subst->PT);
   MakeVector(0, 0, 0, subst->W);
   MakeVector(1, 0, 0, subst->N);
   MakeVector(0, 0, 0, subst->I);
}


/**
 * @brief Push an object pointer onto the object stack.
 * @param stack Existing stack head.
 * @param obj Object to push.
 * @return New stack head.
 */
ostackptr push_object(ostackptr stack, Object *obj)
{
   ostackptr node = FactoryObject_stack();

   if (node == nullptr)
      serror("Failed to allocate a node on the object stack\n");
   node->element = obj;
   node->next = stack;
   return node;
}

/**
 * @brief Pop the top object pointer from the object stack.
 * @param stack Stack head pointer.
 * @return Popped object pointer.
 */
Object *
pop_object(ostackptr *stack)
{
   ostackptr node = *stack;
   Object *obj;

   obj = node->element;
   *stack = node->next;
   delete node;

   return obj;
}

/** @brief Reset a Viewpoint to safe default values.
 *
 *  Sets a 256x256 resolution, places the camera at (0,0,-1) looking at the
 *  origin, 22.5 deg half-angle, and clears all buffer pointers.
 *  @param eye  Viewpoint to initialise in place.
 */
static void
Initialize_Eye(Viewpoint *eye)
{
   /* Uninitialize the focal distance */
   eye->view_x0 = -1;
   eye->view_y0 = -1;
   eye->view_xl = -1;
   eye->view_yl = -1;
   eye->view_xres = 256;
   eye->view_yres = 256;
   MakeVector(0, 0, -1, eye->view_from);
   MakeVector(0, 0,  0, eye->view_at);
   MakeVector(0, 1,  0, eye->view_up);
   eye->view_angle = degtorad(22.5);
   eye->view_hither = SMALL;
   eye->view_yon = PLY_HUGE;
   eye->view_aperture = 0.0;
   eye->view_aspect = 1.0;
   eye->view_focaldist = -1.0;
   eye->WS = nullptr;
   eye->ZBuffer = nullptr;
   eye->SBuffer = nullptr;
}

/** @brief Free all expression-node fields of a Special_Surface and then delete it.
 *
 *  Each NODE_PTR field is released via deallocate_node(); the struct itself
 *  is freed with @c delete.
 *  @param surf  Special_Surface to destroy; must not be nullptr.
 */
static void
deallocate_surface(Special_Surface *surf)
{
   if (surf == nullptr)
      return;

   deallocate_node(surf->body_color);
   deallocate_node(surf->normal);
   deallocate_node(surf->position);
   deallocate_node(surf->Ka_color);
   deallocate_node(surf->Ka_scale);
   deallocate_node(surf->Kb_power);
   deallocate_node(surf->Kd_color);
   deallocate_node(surf->Kd_scale);
   deallocate_node(surf->Ks_color);
   deallocate_node(surf->Ks_scale);
   deallocate_node(surf->Kr_color);
   deallocate_node(surf->Kr_scale);
   deallocate_node(surf->Kt_color);
   deallocate_node(surf->Kt_scale);
   deallocate_node(surf->ior);
   deallocate_node(surf->D_angle);
   deallocate_node(surf->Position_fn);
   deallocate_node(surf->Pos_scale);
   deallocate_node(surf->Lookup_fn);
   deallocate_node(surf->Turbulence);
   deallocate_node(surf->Octaves);
   deallocate_node(surf->Frequency);
   deallocate_node(surf->Phase);
   deallocate_node(surf->Bump_scale);
   deallocate_cmap_node(surf->map);
   delete surf;
}

/** @brief Dispatch deallocation of the payload pointed to by a symbol-table entry.
 *
 *  Calls the appropriate destructor based on @c entry->type.  Does not free
 *  the token_struct itself - the caller is responsible for that.
 *  @param entry  Symbol-table entry whose @c data payload is to be freed.
 */
static void Delete_Definition(tokenptr entry)
{
   Ensures(entry != nullptr);
   switch (ShapeType(entry->type)) {
   case ShapeType::Particle:
      DeleteParticle((Particle *)entry->data);
      break;
   case ShapeType::Expression:
      deallocate_node((NODE_PTR)entry->data);
      break;
   case ShapeType::Transform:
      delete static_cast<Transform*>(entry->data);
      break;
   case ShapeType::Surface:
      deallocate_surface((Special_Surface*)entry->data);
      break;
   case ShapeType::String:
      polyray_free(entry->data);
      break;
   case ShapeType::Texture:
      TextureDelete((Texture*)entry->data);
      break;
   case ShapeType::Texture_Map:
      delete_texture_map((texture_map_entries)entry->data);
      break;
   case ShapeType::Object:
      Delete_Object((Object*)entry->data);
      break;
   default:
      serror("Bad type value in 'Delete_Definition'\n");
   }
}

/**
 * @brief Initialize all bins/lists in a binary scene tree.
 * @param root Tree instance to initialize.
 */
void
Initialize_BinTree(BinTree& root)
{
   root.slab_root = nullptr;
   root.members.list = nullptr;
   root.members.count = 0;
   root.csgprims.list = nullptr;
   root.csgprims.count = 0;
   root.polyprims.list = nullptr;
   root.polyprims.count = 0;
   root.eyeprims.list = nullptr;
   root.eyeprims.count = 0;
   root.lights.list = nullptr;
   root.lights.count = 0;
   root.MaxDepth = 64;
   root.MaxListLength = 4;
}

/**
 * @brief Add an object to the scene tree, handling CSG/mesh conversion cases.
 * @param root Destination scene tree.
 * @param obj Object to classify and insert.
 */
void
Add_To_BinTree(BinTree& root, Object *obj)
{
   BinTree temp_root;
   rmode old_method;
   Texture *text;
   Surface *surf;
   Special_Surface *spec_surf;
   Object *tobj;
   int displace_flag, OldOptim;

   /* First see if we can tweak the shading flags to improve speed during
      rendering. */
   if (obj->o_type != ShapeType::Composite && 
         obj->o_type != ShapeType::Polygon) {
      for (tobj=obj,text=obj->o_texture;
           text!=nullptr && tobj!=nullptr;
           tobj=tobj->o_parent)
         text = tobj->o_texture;
      if (text == nullptr)
         ;
      else if (text->type == ShapeType::Special) {
         spec_surf = (Special_Surface *)(text->data);
         if ((spec_surf->Kr_scale == nullptr) && (obj->o_sflag & REFLECT_CHECK))
            obj->o_sflag ^= REFLECT_CHECK;
         }
      else if (text->type == ShapeType::Plain) {
         surf = (Surface *)(text->data);
         if ((surf->Kt_scale == 0.0 || (surf->Kt_color[0] == 0.0 &&
                                        surf->Kt_color[1] == 0.0 &&
                                        surf->Kt_color[2] == 0.0)) &&
             (obj->o_sflag & TRANSMIT_CHECK))
            obj->o_sflag ^= TRANSMIT_CHECK;
         if ((surf->Kr_scale == 0.0 || (surf->Kr_color[0] == 0.0 &&
                                        surf->Kr_color[1] == 0.0 &&
                                        surf->Kr_color[2] == 0.0)) &&
             (obj->o_sflag & REFLECT_CHECK))
            obj->o_sflag ^= REFLECT_CHECK;
         /* We are guaranteed that this texture is opaque.  Reset the
           shading quality flag for this object */
         if (obj->o_sflag & UV_CHECK &&
             obj->o_uv_bounds[0] == -PLY_HUGE &&
             obj->o_uv_bounds[1] ==  PLY_HUGE &&
             obj->o_uv_bounds[2] == -PLY_HUGE &&
             obj->o_uv_bounds[3] ==  PLY_HUGE)
             /* We are working with the default bounds - no need to do any
                intersection tests on the actual values */
            obj->o_sflag ^= UV_CHECK;
         }
      }

   for (tobj=obj,displace_flag=0;tobj!=nullptr&&!displace_flag;tobj=tobj->o_parent)
      if (tobj->o_displace != nullptr)
         displace_flag = 1;

   if (obj->o_type == ShapeType::Csg) {
      set_parent_ptrs((csgnodeptr)obj->o_data, nullptr, obj, obj->o_trans, &obj->o_bnd);
      instantiate_csg(&root, (csgnodeptr)obj->o_data, displace_flag);
      root.csgprims.list = push_object(root.csgprims.list, obj);
      ++root.csgprims.count;
      }
   else if ((RuntimeState::settings.Render_Method == rmode::RAY_TRACING ||
             ((RuntimeState::settings.Render_Method == rmode::SCAN_CONVERSION) &&
              (RuntimeState::scene.Global_Shade_Flag &
               (SHADOW_CHECK | REFLECT_CHECK | TRANSMIT_CHECK)))) &&
            (displace_flag ||
             obj->o_type == ShapeType::Bezier ||
             obj->o_type == ShapeType::Nurb ||
             obj->o_type == ShapeType::Parametric)) {
      old_method = RuntimeState::settings.Render_Method;
      RuntimeState::settings.Render_Method = rmode::MESH_CONVERSION;

      /* Create a temporary BinTree to hold the polygons as they are
         made by the scan converter */
      Initialize_BinTree(temp_root);
      obj->o_procs->render(nullptr, &temp_root, obj);
      OldOptim = RuntimeState::settings.Optimizer;
      RuntimeState::settings.Optimizer = 1;
      BuildBoundingSlabs(temp_root);//old code &
      RuntimeState::settings.Optimizer = OldOptim;

      /* Now add the slabbed patch pieces to the global set of objects */
      if (temp_root.slab_root == nullptr)
         serror("Failed to add triangulated object to bintree");

      root.members.list = push_object(root.members.list,
                                       temp_root.slab_root);//returns ostackptr
      root.members.count++;
      while (temp_root.members.list != nullptr)
         pop_object(&temp_root.members.list);
      root.polyprims.list = push_object(root.polyprims.list, obj);
      ++root.polyprims.count;
      RuntimeState::settings.Render_Method = old_method;
      }
   else {
      root.members.list = push_object(root.members.list, obj);
      root.members.count++;
      }
}

void CallSpecificDeleteObj(Object& obj)
{
    switch (obj.o_type)
    {
        case ShapeType::Blob:
            BlobDelete(&obj);
            break;
        case ShapeType::Box:
            BoxDelete(&obj);
            break;
        case ShapeType::Bezier:
            BezierDelete(&obj);
            break;
        case ShapeType::Cone:
            ConeDelete(&obj);
            break;
        case ShapeType::Sphere:
            SphereDelete(&obj);
            break;
        case ShapeType::Csg:
            CSGDelete(&obj);
            break;
        case ShapeType::Cylinder:
            CylinderDelete(&obj);
            break;
        case ShapeType::Cyl_Height_Field:
            HeightDelete(&obj);
            break;
        case ShapeType::Disc:
            DiscDelete(&obj);
            break;
        case ShapeType::Function:
            FunctionDelete(&obj);
            break;
        case ShapeType::Glyph:
            GlyphDelete(&obj);
            break;
        case ShapeType::Gridded:
            GridDelete(&obj);
            break;
        case ShapeType::Height_Field:
            HeightDelete(&obj);
            break;
        case ShapeType::Hypertexture:
            HypertextureDelete(&obj);
            break;
        case ShapeType::Nurb:
            NurbDelete(&obj);
            break;
        case ShapeType::Parabola:
            ParabolaDelete(&obj);
            break;
        case ShapeType::Parametric:
            ParametricDelete(&obj);
            break;
        case ShapeType::Poly:
            PolyDelete(&obj);
            break;
        case ShapeType::Polynomial:
            PolynomialDelete(&obj);
            break;
        case ShapeType::Raw_Triangles:
            RawDelete(&obj);
            break;
        case ShapeType::Revolve:
            RevolveDelete(&obj);
            break;
        case ShapeType::Sph_Height_Field:
            HeightDelete(&obj);
            break;
        case ShapeType::SuperQ:
            SuperQDelete(&obj);
            break;
        case ShapeType::Sweep:
            SweepDelete(&obj);
            break;
        case ShapeType::Torus:
            TorusDelete(&obj);
            break;
        case ShapeType::Tri:
            TriDelete(&obj);
            break;
        case ShapeType::Light:
            LightDelete(&obj);
            break;
        default:
            // Fallback for any unhandled or legacy types (e.g. during transition)
            GenericDelete(&obj);
            break;
    }
}
/**
 * @brief Delete all object lists and owned resources contained in a scene tree.
 * @param root Tree to clear.
 */
void Delete_BinTree(BinTree *root)
{
   ostackptr objs;
   //Object *obj;

   if (root->slab_root != nullptr) {
      Delete_Object(root->slab_root);
      root->slab_root = nullptr;

      /* Delete the entire list of primitives, excluding any
         that are part of a CSG object */
      objs = root->members.list;
      while (objs != nullptr)
         Object *obj = pop_object(&objs);
      }
   else {
      objs = root->members.list;
      while (objs != nullptr) {
         Object *obj = pop_object(&objs);
         if (obj->o_type != ShapeType::Light)
            Delete_Object(obj);
         }
      }
   root->members.list = nullptr;
   root->members.count = 0;

   /* Delete the list of object bounds that contain the eye */
   objs = root->eyeprims.list;
   while (objs != nullptr)
      Object *obj = pop_object(&objs);
   root->eyeprims.list = nullptr;
   root->eyeprims.count = 0;


   /* Delete all polygon objects */
   objs = root->polyprims.list;
   while (objs != nullptr) {
      Object *obj = pop_object(&objs);
      Delete_Object(obj);
      }
   root->polyprims.list = nullptr;
   root->polyprims.count = 0;

   /* Delete all CSG objects */
   objs = root->csgprims.list;
   while (objs != nullptr) {
      Object *obj = pop_object(&objs);
      Delete_Object(obj);
      }
   root->csgprims.list = nullptr;
   root->csgprims.count = 0;

   /* Delete all light objects */
   objs = root->lights.list;
   while (objs != nullptr) {
      Object *obj = pop_object(&objs);
      Delete_Object(obj);
      }
   root->lights.list = nullptr;
   root->lights.count = 0;

}

/**
 * @brief Free dynamic resources associated with an object.
 * @param obj Object to deallocate.
 */
void Delete_Object(Object* obj)
{

    if (obj == nullptr) return;//cm
#ifdef DEBUG_POINTERS
    if (g_deleted_objects.find(obj) != g_deleted_objects.end()) {
        smessage("Delete_Object: duplicate delete attempt for object=%p\n", (void*)obj);
        return;
    }
#endif

    /* Be careful if this is an object generated by the bounding routines */
    if (obj->o_type == ShapeType::Composite) {
        auto* composite = static_cast<CompositeObject*>(obj);
        if (composite->c_lbnd != nullptr) {
            polyray_free(composite->c_lbnd);
            composite->c_lbnd = nullptr;
        }
#ifdef DEBUG_POINTERS
        obj->o_data = nullptr;
        obj->o_texture = nullptr;
        obj->o_trans = nullptr;
        obj->o_displace = nullptr;
        obj->o_vertices = nullptr;
        obj->o_procs = nullptr;
        obj->o_type = ShapeType::Null;
        g_deleted_objects.insert(obj);
#endif
        delete composite;
    }
    else if (obj->o_type == ShapeType::Polygon) {
#ifdef DEBUG_POINTERS
        g_deleted_objects.insert(obj);
        obj->o_type = ShapeType::Null;
#endif
        polyray_free(obj);
    }
    else {
        /* Delete any memory specific to this object */
#ifdef DEBUG
        smessage("type of obj is %d\n", std::to_underlying(obj->o_type));
#endif
        if (obj->o_type != ShapeType::Null) {
            if (obj->o_copy == 0 && obj->o_procs != nullptr && obj->o_procs->del != nullptr)
                CallSpecificDeleteObj(*obj);// (obj->o_procs->del)(obj);
        }
#ifdef DEBUG_POINTERS
        obj->o_data = nullptr;
#endif

#ifdef DEBUG
        smessage("cpoint2\n");
#endif
        /* Delete any memory used for this objects texture description */
        if (obj->o_texture != nullptr) {
            TextureDelete(obj->o_texture);
#ifdef DEBUG_POINTERS
            obj->o_texture = nullptr;
#endif
        }

        /* Delete any memory associated with this objects transformation */
        if (obj->o_trans != nullptr) {
            delete obj->o_trans;
#ifdef DEBUG_POINTERS
            obj->o_trans = nullptr;
#endif
        }

        /* Deallocate displacement information */
        deallocate_node(obj->o_displace);
#ifdef DEBUG_POINTERS
        obj->o_displace = nullptr;
#endif

        /* Deallocate any triangle information */
        if (obj->o_vertices != nullptr &&
            (obj->o_type != ShapeType::Raw_Triangles || obj->o_copy == 0)) {
            polyray_free(obj->o_vertices->V);
            if (obj->o_vertices->N != nullptr)
                polyray_free(obj->o_vertices->N);
            if (obj->o_vertices->U != nullptr)
                polyray_free(obj->o_vertices->U);
            polyray_free(obj->o_vertices);
#ifdef DEBUG_POINTERS
            obj->o_vertices = nullptr;
#endif
        }

        /* Free the memory used by this object */
#ifdef DEBUG_POINTERS
        smessage("type of obj is %d\n", std::to_underlying(obj->o_type));
#endif
        const auto object_type = obj->o_type;
#ifdef DEBUG_POINTERS
        obj->o_procs = nullptr;
        obj->o_type = ShapeType::Null;
        g_deleted_objects.insert(obj);
#endif
        if (object_type > ShapeType::Null) {
            delete obj;
        }
    }

}

/**
 * @brief Perform a deep copy of object state and owned substructures.
 * @param start_obj Source object.
 * @param result_obj Destination object.
 */
void Copy_Object(Object* start_obj, Object& result_obj)
{
   /* Just in case, perform a type check on this thing to see if it
      really is an object. */
   Expects(start_obj != nullptr);
   Expects(std::to_underlying(start_obj->o_type) >= FIRST_OBJECT_TYPE && 
           std::to_underlying(start_obj->o_type) <= LAST_OBJECT_TYPE);

   result_obj = *start_obj;

   // Copy object specific information
   start_obj->o_procs->copy(start_obj, &result_obj);

   // Copy the texture characteristics 
   if (start_obj->o_texture != nullptr) {
      result_obj.o_texture = FactoryTexture();
      TextureCopy(start_obj->o_texture, result_obj.o_texture);
      }

   // Copy any associated transformations 
   if (start_obj->o_trans != nullptr) {
      result_obj.o_trans = FactoryTransform().release();
      *result_obj.o_trans=*start_obj->o_trans;
      }

   /* Copy displacement expression */
   result_obj.o_displace = copy_node(start_obj->o_displace);
}

/**
 * @brief Generic object initializer when no type-specific setup is required.
 * @param obj Object being initialized.
 * @return Always returns success (1).
 */
int
GenericInitialize(Object *obj)
{
   return 1;
}

/**
 * @brief Generic copy handler for objects with no owned dynamic payload.
 * @param objin Source object.
 * @param objout Destination object.
 */
void
GenericCopy(Object *objin, Object *objout)
{
   objout->o_copy = 1;
}

/**
 * @brief Generic deletion handler for object payload data.
 * @param object Object instance whose payload may be deleted.
 */
void GenericDelete(Object *obj)
{
   if (obj->o_copy != 0 || obj->o_data == nullptr)
      return;

   /* Payload allocation:
      data allocated by
      Factory*Data() and must be destroyed with delete. */
   switch (obj->o_type) {
   case ShapeType::Sphere:
       delete static_cast<SphereData*>(obj->o_data);
       break;
   case ShapeType::Cone:
       delete static_cast<ConeData*>(obj->o_data);
       break;
   case ShapeType::Cylinder:
       delete static_cast<CylData*>(obj->o_data);
       break;
   case ShapeType::Disc:
       delete static_cast<DiscData*>(obj->o_data);
       break;
   case ShapeType::Torus:
       delete static_cast<TorusData*>(obj->o_data);
       break;
   case ShapeType::Parabola:
       delete static_cast<ParabolaData*>(obj->o_data);
       break;
   case ShapeType::SuperQ:
       delete static_cast<SuperQData*>(obj->o_data);
       break;
   case ShapeType::Tri:
       delete static_cast<TriData*>(obj->o_data);
       break;
   case ShapeType::Nurb:
       delete static_cast<NurbData*>(obj->o_data);
       break;
   case ShapeType::Bezier:
       delete static_cast<BezierData*>(obj->o_data);
       break;
   case ShapeType::Sweep:
       delete static_cast<SweepData*>(obj->o_data);
       break;
   case ShapeType::Box:
       delete static_cast<BoxData*>(obj->o_data);
       break;
   case ShapeType::Poly:
       delete static_cast<PolyData*>(obj->o_data);
       break;
   case ShapeType::Polynomial:
       delete static_cast<PolynomialData*>(obj->o_data);
       break;
   case ShapeType::Revolve:
       delete static_cast<RevolveData*>(obj->o_data);
       break;
   case ShapeType::Raw_Triangles:
       delete static_cast<RawData*>(obj->o_data);
       break;
   case ShapeType::Glyph:
       delete static_cast<GlyphData*>(obj->o_data);
       break;
   case ShapeType::Gridded:
       delete static_cast<GridData*>(obj->o_data);
       break;
   case ShapeType::Function:
       delete static_cast<FunctionData*>(obj->o_data);
       break;
   case ShapeType::Hypertexture:
       delete static_cast<HypertextureData*>(obj->o_data);
       break;
   case ShapeType::Parametric:
       delete static_cast<ParametricData*>(obj->o_data);
       break;
   case ShapeType::Height_Field:
   case ShapeType::Cyl_Height_Field:
   case ShapeType::Sph_Height_Field:
       delete static_cast<HeightData*>(obj->o_data);
       break;
   default:
         polyray_free(obj->o_data);
         break;
   }
   obj->o_data = nullptr;
}

/**
 * @brief Generic render entry that subdivides using current mesh strategy.
 * @param eye Active viewpoint.
 * @param Root Scene tree receiving generated primitives.
 * @param obj Object to render/subdivide.
 */
void GenericRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
#if 0
   int u_steps, v_steps;

   if (obj->o_uv_bounds[0] == -PLY_HUGE) {
      /* u/v bounds weren't set - make the mesh adaptive to
         the screen size by setting the values of uv_steps. */
      u_steps = obj->o_uv_steps[0];
      v_steps = obj->o_uv_steps[1];

      /* Render the primitive */
      Adaptive_Subdivide(eye, Root, obj);

      /* Reset the u/v steps to their starting state */
      obj->o_uv_steps[0] = u_steps;
      obj->o_uv_steps[1] = v_steps;
      }
   else
#endif
      Uniform_Subdivide(eye, Root, obj);
}

/** @brief Emit a warning about an unrecognised or out-of-range ini-file value.
 *  @param line  Line number in polyray.ini where the bad value appeared.
 *  @param key   The key whose value was invalid.
 */
static void
tools_error(int line, std::string_view key)
{
    std::string skey{ key };
    swarning("Bad initialization value for '" + skey + "', at line " +
       int2string(line) +"\n");
}

/**
 * @brief Load default runtime values from `polyray.ini` (if present)
 * using SimpleIni
 * Each entry in the file has the form `key value`.
 */
#include <SimpleIni.h>
#include <string>
#include <algorithm>

 // Helper to simulate case-insensitive matching for values if needed
bool iequals(const std::string& a, const std::string& b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end(),
        [](char ch1, char ch2) { return ::tolower(ch1) == ::tolower(ch2); });
}

void LoadPolyrayIni() {
    RuntimeState::settings.check_colineartriangles = 0;

    CSimpleIniA ini;
    ini.SetUnicode();

    // Load the file (assumes PathFileOpencpp logic or direct path string is available)
    // If your framework requires PathFileOpencpp, you can read it into a string buffer first,
    // but typically SimpleIni can load directly via file path:
    std::string fullfilename = RuntimeState::POLYRAY_DAT + "\\polyray.ini";

    SI_Error rc = ini.LoadFile(fullfilename.c_str());
    if (rc < 0) {
        std::cout << "INI FILE LOCATION: " << fullfilename << "\n";
        swarning("NO POLYRAY.INI FILE FOUND!");
        return;
    }

    // Since SimpleIni loads the entire structure into memory, we query by key.
    // We can iterate through all keys in the default/global section ("").
    CSimpleIniA::TNamesDepend keys;
    ini.GetAllKeys("", keys);

    // Track a dummy line count if your error reporting absolutely requires it, 
    // though SimpleIni doesn't expose line numbers directly per key.
    int line_count = 0;

    for (const auto& entry : keys) {
        std::string key = entry.pItem;
        line_count++; // Approximation for tools_error context

        // Fetch the raw value string assigned to this key
        const char* raw_val = ini.GetValue("", key.c_str(), nullptr);
        if (!raw_val) continue;
        std::string val1(raw_val);

        // -----------------------------------------------------------------
        // 1. RENDERER SETTINGS
        // -----------------------------------------------------------------
        if (iequals(key, "error_log")) {
            SetMessageLog(raw_val);
        }
        else if (iequals(key, "renderer")) {
            if (iequals(val1, "ray_trace") || iequals(val1, "raytrace"))
                RuntimeState::settings.Render_Method = rmode::RAY_TRACING;
            else if (iequals(val1, "scan_convert"))
                RuntimeState::settings.Render_Method = rmode::SCAN_CONVERSION;
            else if (iequals(val1, "wire_frame") || iequals(val1, "wireframe"))
                RuntimeState::settings.Render_Method = rmode::WIRE_FRAME;
            else if (iequals(val1, "hidden_line"))
                RuntimeState::settings.Render_Method = rmode::HIDDEN_LINE;
            else if (iequals(val1, "gourad"))
                RuntimeState::settings.Render_Method = rmode::GOURAD_SHADE;
            else if (iequals(val1, "raw_triangles") || iequals(val1, "triangles"))
                RuntimeState::settings.Render_Method = rmode::RAW_TRIANGLES;
            else if (iequals(val1, "uv_triangles"))
                RuntimeState::settings.Render_Method = rmode::UV_TRIANGLES;
            else if (iequals(val1, "csg_triangles"))
                RuntimeState::settings.Render_Method = rmode::CSG_TRIANGLES;
            else
                tools_error(line_count, key.c_str());
        }
        // -----------------------------------------------------------------
        // 2. INTEGER & FLOATING POINT OPTIONS
        // -----------------------------------------------------------------
        else if (iequals(key, "max_level") || iequals(key, "max_trace_level") || iequals(key, "maxlevel")) {
            long val = ini.GetLongValue("", key.c_str(), 7);
            if (val < 1 || val >= 128) {
                serror("Maxlevel must be less than 128\n");
                RuntimeState::settings.maxlevel = 7;
            }
            else {
                RuntimeState::settings.maxlevel = static_cast<int>(val);
            }
        }
        else if (iequals(key, "display")) {
            if (iequals(val1, "none")) Global::Display_Flag = 0;
            else if (iequals(val1, "vga") || iequals(val1, "vga1")) Global::Display_Flag = 1;
            else if (iequals(val1, "vga2")) Global::Display_Flag = 2;
            else if (iequals(val1, "vga3")) Global::Display_Flag = 3;
            else if (iequals(val1, "vga4")) Global::Display_Flag = 4;
            else if (iequals(val1, "vga5")) Global::Display_Flag = 5;
            else if (iequals(val1, "hicolor") || iequals(val1, "hicolor1")) Global::Display_Flag = 6;
            else if (iequals(val1, "hicolor2")) Global::Display_Flag = 7;
            else if (iequals(val1, "hicolor3")) Global::Display_Flag = 8;
            else if (iequals(val1, "hicolor4")) Global::Display_Flag = 9;
            else if (iequals(val1, "hicolor5")) Global::Display_Flag = 10;
            else if (iequals(val1, "16bit1")) Global::Display_Flag = 11;
            else if (iequals(val1, "16bit2")) Global::Display_Flag = 12;
            else if (iequals(val1, "16bit3")) Global::Display_Flag = 13;
            else if (iequals(val1, "16bit4")) Global::Display_Flag = 14;
            else if (iequals(val1, "16bit5")) Global::Display_Flag = 15;
            else if (iequals(val1, "truecolor1")) Global::Display_Flag = 16;
            else if (iequals(val1, "truecolor2")) Global::Display_Flag = 17;
            else if (iequals(val1, "truecolor3")) Global::Display_Flag = 18;
            else if (iequals(val1, "truecolor4")) Global::Display_Flag = 19;
            else if (iequals(val1, "truecolor5")) Global::Display_Flag = 20;
            else if (iequals(val1, "4bit1")) Global::Display_Flag = 21;
            else if (iequals(val1, "4bit2")) Global::Display_Flag = 22;
            else tools_error(line_count, key.c_str());
        }
        else if (iequals(key, "pallette")) {
            if (iequals(val1, "grey") || iequals(val1, "greyscale") || iequals(val1, "gray") || iequals(val1, "grayscale"))
                Global::Pallette_Flag = 0;
            else if (iequals(val1, "884")) Global::Pallette_Flag = 1;
            else if (iequals(val1, "666")) Global::Pallette_Flag = 2;
            else if (iequals(val1, "4bit")) Global::Pallette_Flag = 3;
            else tools_error(line_count, key.c_str());
        }
        else if (iequals(key, "pallette_start")) {
            long temp = ini.GetLongValue("", key.c_str(), -1);
            if (temp < 0 || temp > 240) {
                swarning("First entry of pallette must be between 0 and 240\n");
            }
            else {
                Global::Pallette_Start = static_cast<int>(temp);
            }
        }
        else if (iequals(key, "antialias")) {
            if (iequals(val1, "none")) RuntimeState::settings.antialias = 0;
            else if (iequals(val1, "filter")) RuntimeState::settings.antialias = 1;
            else if (iequals(val1, "adaptive1")) RuntimeState::settings.antialias = 2;
            else if (iequals(val1, "adaptive2")) RuntimeState::settings.antialias = 3;
            else tools_error(line_count, key.c_str());
        }
        else if (iequals(key, "maxsamples") || iequals(key, "max_samples")) {
            RuntimeState::settings.maxsamples = static_cast<int>(ini.GetLongValue("", key.c_str(), 0));
            if (RuntimeState::settings.maxsamples < 1) tools_error(line_count, key.c_str());
        }
        else if (iequals(key, "maxscreenbuffer")) {
            long pages = ini.GetLongValue("", key.c_str(), 0);
            RuntimeState::settings.MaxBufferRAM = 1024L * pages;
            if (RuntimeState::settings.MaxBufferRAM < 1) tools_error(line_count, key.c_str());
        }
        else if (iequals(key, "aliasthreshold") || iequals(key, "alias_threshold")) {
            double threshold = ini.GetDoubleValue("", key.c_str(), 0.0);
            RuntimeState::settings.antialias_threshold = threshold * threshold;
            if (RuntimeState::settings.antialias_threshold < 0.0) tools_error(line_count, key.c_str());
        }
        else if (iequals(key, "pixelsize") || iequals(key, "pixel_size")) {
            int p_size = static_cast<int>(ini.GetLongValue("", key.c_str(), 0));
            if (p_size != 8 && p_size != 16 && p_size != 24 && p_size != 32) {
                swarning("Pixel size must be 8, 16, 24, or 32 bits\n");
                tools_error(line_count, key.c_str());
            }
            else {
                RuntimeState::settings.pixelsize = p_size;
            }
        }
        else if (iequals(key, "pixelencoding") || iequals(key, "pixel_encoding")) {
            if (iequals(val1, "none")) RuntimeState::settings.pixel_encoding = 0;
            else if (iequals(val1, "rle")) RuntimeState::settings.pixel_encoding = 1;
            else {
                // Handle raw fallback fallback numbers if applicable, or process as string
                RuntimeState::settings.pixel_encoding = static_cast<int>(ini.GetLongValue("", key.c_str(), 0));
            }
        }
        else if (iequals(key, "clustersize") || iequals(key, "cluster_size")) {
            RuntimeState::settings.clustersize = static_cast<int>(ini.GetLongValue("", key.c_str(), 0));
        }
        else if (iequals(key, "colinear")) {
            RuntimeState::settings.check_colineartriangles = iequals(val1, "off") ? 0 : 1;
        }
        else if (iequals(key, "status") || iequals(key, "line_counter")) {
            RuntimeState::status_flag = 1;
            if (iequals(val1, "none") || iequals(val1, "off")) {
                RuntimeState::status_flag = 0;
                RuntimeState::settings.tickflag = 0;
            }
            else if (iequals(val1, "totals")) RuntimeState::settings.tickflag = 1;
            else if (iequals(val1, "line"))   RuntimeState::settings.tickflag = 2;
            else if (iequals(val1, "pixel"))  RuntimeState::settings.tickflag = 3;
            else tools_error(line_count, key.c_str());
        }
        else if (iequals(key, "aborttest") || iequals(key, "abort_test")) {
            if (iequals(val1, "off") || iequals(val1, "false")) RuntimeState::Check_Abort_Flag = 0;
            else if (iequals(val1, "slow"))                     RuntimeState::Check_Abort_Flag = 2;
            else if (iequals(val1, "on") || iequals(val1, "true"))   RuntimeState::Check_Abort_Flag = 1;
            else tools_error(line_count, key.c_str());
        }
        else if (iequals(key, "warnings")) {
            if (iequals(val1, "on"))       RuntimeState::warnings_flag = 1;
            else if (iequals(val1, "off")) RuntimeState::warnings_flag = 0;
            else tools_error(line_count, key.c_str());
        }
        else if (iequals(key, "errors")) {
            if (iequals(val1, "on"))       RuntimeState::errors_flag = 1;
            else if (iequals(val1, "off")) RuntimeState::errors_flag = 0;
            else tools_error(line_count, key.c_str());
        }
        else if (iequals(key, "dither")) {
            if (iequals(val1, "on"))       Global::Dither_Flag = 1;
            else if (iequals(val1, "off")) Global::Dither_Flag = 0;
            else tools_error(line_count, key.c_str());            
        }
        else if (iequals(key, "shadeflags") || iequals(key, "shade_flags")) {
            if (iequals(val1, "default")) {
                if (RuntimeState::settings.Render_Method == rmode::RAY_TRACING) {
                    RuntimeState::scene.Global_Shade_Flag = SHADOW_CHECK | REFLECT_CHECK |
                        TRANSMIT_CHECK | UV_CHECK | NORMAL_CORRECT;
                }
                else {
                    RuntimeState::scene.Global_Shade_Flag = 0;
                }
            }
            else {
                RuntimeState::scene.Global_Shade_Flag = static_cast<int>(ini.GetLongValue("", key.c_str(), 0));
            }
            if (RuntimeState::scene.Global_Shade_Flag > ALL_SHADE_FLAGS) tools_error(line_count, key.c_str());
        }
        else if (iequals(key, "shadow_tolerance")) {
            RuntimeState::rayeps = ini.GetDoubleValue("", key.c_str(), 0.0);
            if (RuntimeState::rayeps < 0.0) tools_error(line_count, key.c_str());
        }
        else if (iequals(key, "csg_tolerance")) {
            RuntimeState::settings.csg_leg_tolerance = ini.GetDoubleValue("", key.c_str(), 0.0);
            if (RuntimeState::settings.csg_leg_tolerance < 0.0) tools_error(line_count, key.c_str());
        }
        else if (iequals(key, "csg_subdivisions")) {
            RuntimeState::settings.csg_subdivision_depth = static_cast<int>(ini.GetLongValue("", key.c_str(), 0));
            if (RuntimeState::settings.csg_subdivision_depth < 0) tools_error(line_count, key.c_str());
        }
        else if (iequals(key, "optimizer")) {
            if (iequals(val1, "none")) RuntimeState::settings.Optimizer = 0;
            else if (iequals(val1, "slabs") || iequals(val1, "bounding_slabs")) RuntimeState::settings.Optimizer = 1;
            else {
                swarning("Optimization method must be: none or slabs\n");
                tools_error(line_count, key.c_str());
            }
        }
        // -----------------------------------------------------------------
        // 3. MULTI-VALUE KEYS (Resolution / Image Windows)
        // -----------------------------------------------------------------
        else if (iequals(key, "resolution")) {
            // SimpleIni parses multiple space-separated strings as one single value string.
            // We can break it up cleanly via sscanf on the sub-value string.
            int res_x = 0, res_y = 0;
            if (sscanf(raw_val, "%d %d", &res_x, &res_y) == 2) {
                RuntimeState::scene.Eye.view_xres = res_x;
                RuntimeState::scene.Eye.view_yres = res_y;
            }
        }
        else if (iequals(key, "screen_window")) {
            int x0, y0, xl, yl;
            if (sscanf(raw_val, "%d %d %d %d", &x0, &y0, &xl, &yl) == 4) {
                Global::Display_x0 = x0;
                Global::Display_y0 = y0;
                Global::Display_xl = xl;
                Global::Display_yl = yl;
            }
        }
        else if (iequals(key, "image_window")) {
            int x0, y0, xl, yl;
            if (sscanf(raw_val, "%d %d %d %d", &x0, &y0, &xl, &yl) == 4) {
                RuntimeState::scene.Eye.view_x0 = x0;
                RuntimeState::scene.Eye.view_y0 = y0;
                RuntimeState::scene.Eye.view_xl = xl;
                RuntimeState::scene.Eye.view_yl = yl;
            }
        }
        else {
            swarning("Unknown key: %s in polyray.ini\n", key.c_str());
        }
    }
}


/** @brief Recursively print the structure of a 2-3 tree to the message log.
 *  @param tree   Root of the subtree to display (nullptr prints "nullptr").
 *  @param depth  Current indentation depth (call with 0 at the root).
 */
static void show_two_three(nodeptr tree, int depth)
{
   int i;

   for (i=0;i<depth;i++)
      smessage(" ");
   if (tree == nullptr)
      smessage("nullptr\n");
   else if (tree->kind == LEAF_NODE)
      smessage("l(%s)\n", tree->data.leaf.leaf_data->key);
   else if (tree->kind == TWO_NODE) {
      smessage("n2(");
      switch (tree->data.two.left_two_tree->kind) {
         case LEAF_NODE:
            smessage("-l-,");
            break;
         case TWO_NODE:
            smessage("-n2-,");
            break;
         case THREE_NODE:
            smessage("-n3-,");
         }
      smessage("%s", tree->data.two.two_data->key);
      switch (tree->data.two.right_two_tree->kind) {
         case LEAF_NODE:
            smessage(",-l-)\n");
            break;
         case TWO_NODE:
            smessage(",-n2-)\n");
            break;
         case THREE_NODE:
            smessage(",-n3-)\n");
         }
      show_two_three(tree->data.two.left_two_tree, depth+1);
      show_two_three(tree->data.two.right_two_tree, depth+1);
      }
   else {
      smessage("n3(");
      switch (tree->data.three.left_three_tree->kind) {
         case LEAF_NODE:
            smessage("-l-,");
            break;
         case TWO_NODE:
            smessage("-n2-,");
            break;
         case THREE_NODE:
            smessage("-n3-,");
         }
      smessage("%s", tree->data.three.left_three_data->key);
      switch (tree->data.three.middle_three_tree->kind) {
         case LEAF_NODE:
            smessage(",-l-,");
            break;
         case TWO_NODE:
            smessage(",-n2-,");
            break;
         case THREE_NODE:
            smessage(",-n3-,");
         }
      smessage("%s", tree->data.three.right_three_data->key);
      switch (tree->data.three.right_three_tree->kind) {
         case LEAF_NODE:
            smessage(",-l-)\n");
            break;
         case TWO_NODE:
            smessage(",-n2-)\n");
            break;
         case THREE_NODE:
            smessage(",-n3-)\n");
         }
      show_two_three(tree->data.three.left_three_tree, depth+1);
      show_two_three(tree->data.three.middle_three_tree, depth+1);
      show_two_three(tree->data.three.right_three_tree, depth+1);
      }
}

nodeptr FactoryLeafNODEPTR(void);

/** @brief Allocate and default-initialise an struct node leaf ).
 *  @return struct node * (nodeptr) pointing to the newly allocated expression node.
 */
nodeptr FactoryLeafNODEPTR(void)
{
    auto new_node = new struct node();//works
    //auto new_node = std::make_unique<struct exper_node_struct>();
    Ensures(new_node != nullptr);
    //if (new_node == nullptr)
    //   serror("FactoryNODEPTR:Failed to allocate a node\n");
    //std::cout<<"hey I used new() in factory\n"<<std::flush;
    //NODE_PTR rawptr=new_node.release();
    return new_node;
}


/** @brief Allocate a new leaf node holding @p key and @p data.
 *  @param key   Symbol name to store.
 *  @param data  Opaque payload (tokenptr) to associate with @p key.
 *  @return      Newly allocated LEAF_NODE; aborts via serror() on OOM.
 */
static nodeptr
new_node(const std::string& key, void *data)
{
   nodeptr temp_node;
   nodedata temp_data;

   temp_node = FactoryLeafNODEPTR();
   temp_data = new (std::nothrow) key_data;
   if (temp_node == nullptr || temp_data == nullptr)
      serror("Out of memory\n");
   temp_node->kind = LEAF_NODE;
   temp_node->data.leaf.leaf_data = temp_data;
   temp_data->key  = key;
   temp_data->data = data;
   temp_data->next = nullptr;
   return temp_node;
}

/** @brief Compare two 2-3 tree key records lexicographically.
 *  @param key1  Left-hand key record.
 *  @param key2  Right-hand key record.
 *  @return      Negative when key1 < key2, 0 when equal, positive when key1 > key2.
 */
static int key_lessp(nodedata key1, nodedata key2)
{
    return key1->key.compare(key2->key);// strcmp(key1->key, key2->key);
}

/** @brief Attempt to insert @p indata into a 2-3 tree, splitting a 3-node if necessary.
 *
 *  When called on a LEAF_NODE or THREE_NODE that is full, creates a split and
 *  propagates the median key upward via @p outdata.  Used internally by
 *  insert3() and two_three_insert().
 *  @param intree     Subtree to insert into.
 *  @param indata     Key/value pair to insert.
 *  @param left_tree  Output: left subtree after a split.
 *  @param outdata    Output: median key promoted to the parent on a split.
 *  @param right_tree Output: right subtree after a split.
 *  @return           1 if a split occurred, 0 otherwise.
 */
static int split(nodeptr intree, nodedata indata,
      nodeptr *left_tree, nodedata *outdata, nodeptr *right_tree)
{
   int result;
   nodedata data1, new_data;
   nodeptr temp1, new_tree1, new_tree2;

   if (intree == nullptr)
      result = 0;
   else if (intree->kind == LEAF_NODE) {
      result = key_lessp(indata, intree->data.leaf.leaf_data);
      if (result < 0) {
            *left_tree  = new_node(indata->key, indata->data);
            *outdata    = intree->data.leaf.leaf_data;
            *right_tree = intree;
            result      = 1;
      } else if (result == 0) {
            result = 0;
      } else {
            *left_tree  = intree;
            *right_tree = new_node(indata->key, indata->data);
            *outdata    = (*right_tree)->data.leaf.leaf_data;
            result      = 1;
      }
   } else if (intree->kind == TWO_NODE)
      result = 0;
   else if (key_lessp(indata, intree->data.three.left_three_data) < 0)
      if (split(intree->data.three.left_three_tree, indata,
                &new_tree1, &new_data, &new_tree2)) {
          *right_tree = FactoryLeafNODEPTR();
          if (*right_tree == nullptr)
            serror("Out of memory\n");
         (*right_tree)->kind = TWO_NODE;
         (*right_tree)->data.two.left_two_tree =
            intree->data.three.middle_three_tree;
         (*right_tree)->data.two.two_data = intree->data.three.right_three_data;
         (*right_tree)->data.two.right_two_tree =
            intree->data.three.right_three_tree;
         *outdata = intree->data.three.left_three_data;
         *left_tree = intree;
         intree->kind = TWO_NODE;
         intree->data.two.left_two_tree  = new_tree1;
         intree->data.two.two_data       = new_data;
         intree->data.two.right_two_tree = new_tree2;
         result = 1;
         }
      else
         result = 0;
   else if (key_lessp(intree->data.three.left_three_data, indata) < 0 &&
            key_lessp(indata, intree->data.three.right_three_data) < 0) {
      if (split(intree->data.three.middle_three_tree, indata,
                &new_tree1, &new_data, &new_tree2)) {
          *right_tree = FactoryLeafNODEPTR();
         temp1 = intree->data.three.left_three_tree;
         data1 = intree->data.three.left_three_data;
         (*right_tree)->kind = TWO_NODE;
         (*right_tree)->data.two.left_two_tree = new_tree2;
         (*right_tree)->data.two.two_data = intree->data.three.right_three_data;
         (*right_tree)->data.two.right_two_tree =
            intree->data.three.right_three_tree;
         *outdata = new_data;
         *left_tree = intree;
         intree->kind = TWO_NODE;
         intree->data.two.left_two_tree  = temp1;
         intree->data.two.two_data       = data1;
         intree->data.two.right_two_tree = new_tree1;
         result = 1;
         }
      else
         result = 0;
      }
   else if (key_lessp(intree->data.three.left_three_data, indata) < 0)
      if (split(intree->data.three.right_three_tree, indata,
                &new_tree1, &new_data, &new_tree2)) {
          *left_tree = FactoryLeafNODEPTR();
         (*left_tree)->kind = TWO_NODE;
         (*left_tree)->data.two.left_two_tree =
            intree->data.three.left_three_tree;
         (*left_tree)->data.two.two_data = intree->data.three.left_three_data;
         (*left_tree)->data.two.right_two_tree =
            intree->data.three.middle_three_tree;
         *outdata = intree->data.three.right_three_data;
         *right_tree = intree;
         intree->kind = TWO_NODE;
         intree->data.two.left_two_tree  = new_tree1;
         intree->data.two.two_data       = new_data;
         intree->data.two.right_two_tree = new_tree2;
         result = 1;
         }
      else
         result = 0;
   else
      result = 0;

   return result;
}

/** @brief Recursively insert @p indata into a 2-3 tree rooted at @p intree.
 *
 *  Descends to the correct leaf position and, if the parent is a 2-node,
 *  promotes it to a 3-node without splitting.  Returns 1 when the tree
 *  pointer in @p outtree was updated, 0 when no structural change occurred.
 *  @param intree   Subtree to insert into.
 *  @param indata   Key/value pair to insert.
 *  @param outtree  Output: updated subtree root after the insertion.
 *  @return         1 if the subtree root changed, 0 otherwise.
 */
static int
insert3(nodeptr intree, nodedata indata, nodeptr *outtree)
{
   int result;
   nodedata new_data, data1, data2;
   nodeptr temp1, temp3;
   nodeptr new_tree1, new_tree2;

   if (intree == nullptr) {
      *outtree = new_node(indata->key, indata->data);
      result = 1;
      }
   else if (intree->kind == LEAF_NODE)
      result = 0;
   else if (intree->kind == TWO_NODE)
      if (key_lessp(indata, intree->data.two.two_data) < 0)
         if (insert3(intree->data.two.left_two_tree, indata, &new_tree1)) {
            intree->data.two.left_two_tree = new_tree1;
            *outtree = intree;
            result = 1;
            }
         else if (split(intree->data.two.left_two_tree, indata,
                        &new_tree1, &new_data, &new_tree2)) {
            data2 = intree->data.two.two_data;
            temp3 = intree->data.two.right_two_tree;
            intree->kind = THREE_NODE;
            intree->data.three.left_three_tree   = new_tree1;
            intree->data.three.left_three_data   = new_data;
            intree->data.three.middle_three_tree = new_tree2;
            intree->data.three.right_three_data  = data2;
            intree->data.three.right_three_tree  = temp3;
            *outtree = intree;
            result = 1;
            }
         else
            result = 0;
      else if (key_lessp(intree->data.two.two_data, indata) < 0)
         if (insert3(intree->data.two.right_two_tree, indata, &new_tree1)) {
            intree->data.two.right_two_tree = new_tree1;
            *outtree = intree;
            result = 1;
            }
         else if (split(intree->data.two.right_two_tree, indata,
                        &new_tree1, &new_data, &new_tree2)) {
            temp1 = intree->data.two.left_two_tree;
            data1 = intree->data.two.two_data;
            intree->kind = THREE_NODE;
            intree->data.three.left_three_tree   = temp1;
            intree->data.three.left_three_data   = data1;
            intree->data.three.middle_three_tree = new_tree1;
            intree->data.three.right_three_data  = new_data;
            intree->data.three.right_three_tree  = new_tree2;
            *outtree = intree;
            result = 1;
            }
         else
            result = 0;
      else
         result = 0;
   else if (key_lessp(indata, intree->data.three.left_three_data) < 0)
      if (insert3(intree->data.three.left_three_tree, indata, &new_tree1)) {
         intree->data.three.left_three_tree = new_tree1;
         *outtree = intree;
         result = 1;
         }
      else
         result = 0;
   else if (key_lessp(intree->data.three.left_three_data, indata) < 0 &&
            key_lessp(indata, intree->data.three.right_three_data) < 0)
      if (insert3(intree->data.three.middle_three_tree, indata, &new_tree1)) {
         intree->data.three.middle_three_tree = new_tree1;
         *outtree = intree;
         result = 1;
         }
      else
         result = 0;
   else if (key_lessp(intree->data.three.right_three_data, indata) < 0)
      if (insert3(intree->data.three.right_three_tree, indata, &new_tree1)) {
         intree->data.three.right_three_tree = new_tree1;
         *outtree = intree;
         result = 1;
         }
      else
         result = 0;
   else
      result = 0;

   return result;
}

/** @brief Insert @p indata into the 2-3 tree rooted at @p intree and return the new root.
 *
 *  Handles root splits by allocating a new TWO_NODE root when insert3() and
 *  split() both indicate that the old root overflowed.
 *  @param intree  Current tree root (may be nullptr for an empty tree).
 *  @param indata  Key/value pair to insert.
 *  @return        New root of the tree (may differ from @p intree after a root split).
 */
static nodeptr
two_three_insert(nodeptr intree, nodedata indata)
{
   nodeptr left_tree, right_tree, result;
   nodedata new_data;

   if (insert3(intree, indata, &left_tree))
      result = left_tree;
   else if (split(intree, indata, &left_tree, &new_data, &right_tree)) {
       result = FactoryLeafNODEPTR();
      if (result == nullptr)
         serror("Out of memory\n");
      result->kind = TWO_NODE;
      result->data.two.left_two_tree  = left_tree;
      result->data.two.two_data       = new_data;
      result->data.two.right_two_tree = right_tree;
      }
   else
      result = intree;
   return result;
}

/** @brief Return the leftmost (smallest) key record in a 2-3 subtree.
 *  @param intree  Subtree to search; may be nullptr.
 *  @return        Pointer to the minimum key record, or nullptr if @p intree is nullptr.
 */
static nodedata two_three_least(nodeptr intree)
{
   if (intree == nullptr)
      return nullptr;
   else if (intree->kind == LEAF_NODE)
      return intree->data.leaf.leaf_data;
   else if (intree->kind == TWO_NODE)
      return two_three_least(intree->data.two.left_two_tree);
   else
      return two_three_least(intree->data.three.left_three_tree);
}

/** @brief Remove the entry matching @p indata from a 2-3 tree, 
 *  rebalancing as needed.
 *
 *  Handles all merge and sibling-steal cases for both TWO_NODE and THREE_NODE
 *  parents.  Frees the deleted leaf node and its key_data via delete
 *  
 *  @param intree   Root of the subtree to delete from.
 *  @param indata   Key record to match for deletion.
 *  @param outtree  Output: updated subtree root (may become nullptr if the tree empties).
 *  @return         1 if a structural change propagates upward, 0 otherwise.
 */
static int two_three_delete(nodeptr intree, nodedata indata, nodeptr *outtree)
{
   nodedata data1, data2;
   nodeptr tree1, tree2, tree3, tree4;

   *outtree = intree;

   if (intree == nullptr)
      return 0;

   if (intree->kind == LEAF_NODE)
      if (key_lessp(indata, intree->data.leaf.leaf_data) != 0) {
         delete intree->data.leaf.leaf_data;
         delete (intree);
         *outtree = nullptr;
         return 1;
         }
      else
         return 0;
   else if (intree->kind == TWO_NODE)
      if (key_lessp(indata, intree->data.two.two_data) < 0)
         if (two_three_delete(intree->data.two.left_two_tree, indata, &tree1))
            if (tree1 == nullptr) {
               /* (1, 1) -> leaf */
               *outtree = intree->data.two.right_two_tree;
               return 1;
               }
            else if (intree->data.two.right_two_tree->kind == TWO_NODE) {
               tree2 = intree->data.two.right_two_tree;
               tree3 = tree2->data.two.left_two_tree;
               data1 = tree2->data.two.two_data;
               tree4 = tree2->data.two.right_two_tree;

               tree2->kind = THREE_NODE;
               tree2->data.three.left_three_tree = tree1;
               tree2->data.three.left_three_data = intree->data.two.two_data;
               tree2->data.three.middle_three_tree = tree3;
               tree2->data.three.right_three_data  = data1;
               tree2->data.three.right_three_tree  = tree4;
               *outtree = tree2;
               return 1;
               }
            else {
               /* Split one from the right and add it to the left */
               tree2 = intree->data.two.right_two_tree;
               tree3 = intree->data.two.left_two_tree;
               tree3->kind = TWO_NODE;
               tree3->data.two.left_two_tree = tree1;
               tree3->data.two.two_data      = intree->data.two.two_data;
               tree3->data.two.right_two_tree = tree2->data.three.left_three_tree;
               intree->data.two.two_data = tree2->data.three.left_three_data;
               tree1 = tree2->data.three.middle_three_tree;
               data1 = tree2->data.three.right_three_data;
               tree3 = tree3->data.three.right_three_tree;
               tree2->kind = TWO_NODE;
               tree2->data.two.left_two_tree = tree1;
               tree2->data.two.two_data = data1;
               tree2->data.two.right_two_tree = tree3;
               *outtree = intree;
               return 0;
               }
         else {
            intree->data.two.left_two_tree = tree1;
            *outtree = intree;
            return 0;
            }
      else if (two_three_delete(intree->data.two.right_two_tree, indata, &tree1))
         if (tree1 == nullptr) {
            *outtree = intree->data.two.left_two_tree;
            return 1;
            }
         else if (intree->data.two.left_two_tree->kind == TWO_NODE) {
            /* Take the node from the right and insert in the left */
            tree2 = intree->data.two.left_two_tree;
            tree3 = tree2->data.two.left_two_tree;
            data1 = tree2->data.two.two_data;
            tree4 = tree2->data.two.right_two_tree;
            tree2->kind = THREE_NODE;
            tree2->data.three.left_three_tree = tree3;
            tree2->data.three.left_three_data = data1;
            tree2->data.three.middle_three_tree = tree4;
            tree2->data.three.right_three_data = two_three_least(tree1);
            tree2->data.three.right_three_tree = tree1;
            *outtree = tree2;
            return 1;
            }
         else {
            /* Split one from the left and add it to the right */
            tree2 = intree->data.two.left_two_tree;
            tree3 = intree->data.two.right_two_tree;
            tree3->kind = TWO_NODE;
            tree3->data.two.left_two_tree = tree2->data.three.right_three_tree;
            tree3->data.two.two_data = two_three_least(tree1);
            tree3->data.two.right_two_tree = tree1;
            intree->data.two.two_data = tree2->data.three.right_three_data;
            tree1 = tree2->data.three.left_three_tree;
            data1 = tree2->data.three.left_three_data;
            tree3 = tree2->data.three.middle_three_tree;
            tree2->kind = TWO_NODE;
            tree2->data.two.left_two_tree = tree1;
            tree2->data.two.two_data = data1;
            tree2->data.two.right_two_tree = tree3;
            *outtree = intree;
            return 0;
            }
      else {
         intree->data.two.right_two_tree = tree1;
         intree->data.two.two_data = two_three_least(tree1);
         *outtree = intree;
         return 0;
         }
   else if (key_lessp(indata, intree->data.three.left_three_data) < 0)
      if (two_three_delete(intree->data.three.left_three_tree, indata, &tree1))
         if (tree1 == nullptr) {
            tree1 = intree->data.three.middle_three_tree;
            data1 = intree->data.three.right_three_data;
            tree2 = intree->data.three.right_three_tree;
            intree->kind = TWO_NODE;
            intree->data.two.left_two_tree = tree1;
            intree->data.two.two_data = data1;
            intree->data.two.right_two_tree = tree2;
            *outtree = intree;
            return 0;
            }
         else if (intree->data.three.middle_three_tree->kind == TWO_NODE) {
            /* Take the node from the left and insert into the middle */
            tree2 = intree->data.three.middle_three_tree;
            tree3 = tree2->data.two.left_two_tree;
            data1 = tree2->data.two.two_data;
            tree4 = tree2->data.two.right_two_tree;
            tree2->kind = THREE_NODE;
            tree2->data.three.left_three_tree = tree1;
            tree2->data.three.left_three_data = two_three_least(tree3);
            tree2->data.three.middle_three_tree = tree3;
            tree2->data.three.right_three_data = data1;
            tree2->data.three.right_three_tree = tree4;
            data1 = intree->data.three.right_three_data;
            tree3 = intree->data.three.right_three_tree;
            polyray_free(intree->data.three.left_three_tree);
            intree->kind = TWO_NODE;
            intree->data.two.left_two_tree = tree2;
            intree->data.two.two_data = data1;
            intree->data.two.right_two_tree = tree3;
            *outtree = intree;
            return 0;
            }
         else {
            /* Split off one from the middle and attach it to the left */
            tree2 = intree->data.three.middle_three_tree;
            tree3 = intree->data.three.left_three_tree;
            tree3->kind = TWO_NODE;
            tree3->data.two.left_two_tree = tree1;
            tree3->data.two.two_data = intree->data.three.left_three_data;
            tree3->data.two.right_two_tree = tree2->data.three.left_three_tree;
            data1 = tree2->data.three.left_three_data;
            tree3 = tree2->data.three.middle_three_tree;
            data2 = tree2->data.three.right_three_data;
            tree4 = tree2->data.three.right_three_tree;
            tree2->kind = TWO_NODE;
            tree2->data.two.left_two_tree = tree3;
            tree2->data.two.two_data = data2;
            tree2->data.two.right_two_tree = tree4;
            intree->data.three.left_three_data = data1;
            *outtree = intree;
            return 0;
            }
      else {
         *outtree = intree;
         return 0;
         }
   else if (key_lessp(indata, intree->data.three.right_three_data) < 0)
      if (two_three_delete(intree->data.three.middle_three_tree, indata, &tree1))
         if (tree1 == nullptr) {
            /* Make it into a two node.  No loss of height */
            tree1 = intree->data.three.left_three_tree;
            data1 = intree->data.three.right_three_data;
            tree2 = intree->data.three.right_three_tree;
            intree->kind = TWO_NODE;
            intree->data.two.left_two_tree = tree1;
            intree->data.two.two_data = data1;
            intree->data.two.right_two_tree = tree2;
            *outtree = intree;
            return 0;
            }
         else if (intree->data.three.left_three_tree->kind == TWO_NODE) {
            /* Take the node from the middle and insert it to the left */
            tree2 = intree->data.three.left_three_tree;
            tree3 = tree2->data.two.left_two_tree;
            data1 = tree2->data.two.two_data;
            tree4 = tree2->data.two.right_two_tree;
            tree2->kind = THREE_NODE;
            tree2->data.three.left_three_tree = tree3;
            tree2->data.three.left_three_data = data1;
            tree2->data.three.middle_three_tree = tree4;
            tree2->data.three.right_three_data = two_three_least(tree1);
            tree2->data.three.right_three_tree = tree1;
            delete (intree->data.three.middle_three_tree);
            data1 = intree->data.three.right_three_data;
            tree3 = intree->data.three.right_three_tree;
            intree->kind = TWO_NODE;
            intree->data.two.left_two_tree = tree2;
            intree->data.two.two_data = data1;
            intree->data.two.right_two_tree = tree3;
            *outtree = intree;
            return 0;
            }
         else {
            /* Split off one from the left and attach it to the middle */
            tree2 = intree->data.three.left_three_tree;
            tree3 = intree->data.three.middle_three_tree;
            tree3->kind = TWO_NODE;
            tree3->data.two.left_two_tree = tree2->data.three.right_three_tree;
            tree3->data.two.two_data = two_three_least(tree1);
            tree3->data.two.right_two_tree = tree1;
            tree3 = tree2->data.three.left_three_tree;
            data1 = tree2->data.three.left_three_data;
            tree4 = tree2->data.three.middle_three_tree;
            data2 = tree2->data.three.right_three_data;
            tree2->kind = TWO_NODE;
            tree2->data.two.left_two_tree = tree3;
            tree2->data.two.two_data = data1;
            tree2->data.two.right_two_tree = tree4;
            intree->data.three.left_three_data = data2;
            *outtree = intree;
            return 0;
            }
      else {
         intree->data.three.left_three_data = two_three_least(tree1);
         intree->data.three.middle_three_tree = tree1;
         *outtree = intree;
         return 0;
         }
   else if (two_three_delete(intree->data.three.right_three_tree, indata, &tree1))
      if (tree1 == nullptr) {
         /* Collapse a three way branch to a two way branch */
         tree1 = intree->data.three.left_three_tree;
         data1 = intree->data.three.left_three_data;
         tree2 = intree->data.three.middle_three_tree;
         intree->kind = TWO_NODE;
         intree->data.two.left_two_tree = tree1;
         intree->data.two.two_data = data1;
         intree->data.two.right_two_tree = tree2;
         *outtree = intree;
         return 0;
         }
      else if (intree->data.three.middle_three_tree->kind == TWO_NODE) {
         /* Take the node from the right and insert it into the middle */
         tree2 = intree->data.three.middle_three_tree;
         tree3 = tree2->data.two.left_two_tree;
         data1 = tree2->data.two.two_data;
         tree4 = tree2->data.two.right_two_tree;
         tree2->kind = THREE_NODE;
         tree2->data.three.left_three_tree = tree3;
         tree2->data.three.left_three_data = data1;
         tree2->data.three.middle_three_tree = tree4;
         tree2->data.three.right_three_data = two_three_least(tree1);
         tree2->data.three.right_three_tree = tree1;
         tree1 = intree->data.three.left_three_tree;
         data1 = intree->data.three.left_three_data;
         delete (intree->data.three.right_three_tree);
         intree->kind = TWO_NODE;
         intree->data.two.left_two_tree = tree1;
         intree->data.two.two_data = data1;
         intree->data.two.right_two_tree = tree2;
         *outtree = intree;
         return 0;
         }
      else {
         /* Split off one from the middle and attach it to the right */
         tree2 = intree->data.three.middle_three_tree;
         tree3 = intree->data.three.right_three_tree;
         tree3->kind = TWO_NODE;
         tree3->data.two.left_two_tree = tree2->data.three.right_three_tree;
         tree3->data.two.two_data = two_three_least(tree1);
         tree3->data.two.right_two_tree = tree1;
         tree3 = tree2->data.three.left_three_tree;
         data1 = tree2->data.three.left_three_data;
         tree4 = tree2->data.three.middle_three_tree;
         data2 = tree2->data.three.right_three_data;
         tree2->kind = TWO_NODE;
         tree2->data.two.left_two_tree = tree3;
         tree2->data.two.two_data = data1;
         tree2->data.two.right_two_tree = tree4;
         intree->data.three.right_three_data = data2;
         *outtree = intree;
         return 0;
         }
   else {
      intree->data.three.right_three_data = two_three_least(tree1);
      intree->data.three.right_three_tree = tree1;
      *outtree = intree;
      return 0;
      }
}

/* Retrieve the value for a name from a 2-3 tree. */

/** @brief Insert a new token name/value pair into the root token 2-3 tree.
 *  @param token  Null-terminated symbol name.
 *  @param value  tokenptr payload to associate with @p token.
 */
static void
Install_Token(const char *token, tokenptr value)
{
   struct key_data data;

   data.key = token;
   data.data = value;
   Token_Tree = two_three_insert(Token_Tree, &data);
}
#if 0
/* Remove a token from the tree */
static void
Delete_Token(const char *token)
{
   struct key_data data;

   data.key = token;
   data.data = nullptr;
   two_three_delete(Token_Tree, &data, &Token_Tree);
}
#endif
/** @brief Search a 2-3 tree for an exact key match and return the containing node.
 *  @param indata  Key record to search for (only the @c key field is used).
 *  @param tree    Root of the subtree to search.
 *  @return        The LEAF_NODE holding the match, or nullptr if not found.
 */
static nodeptr
two_three_lookup(nodedata indata, nodeptr tree)
{
   if (tree == nullptr)
      return nullptr;
   else if (tree->kind == LEAF_NODE)
      if (! indata->key.compare(tree->data.leaf.leaf_data->key))
         return tree;
      else
         return nullptr;
   else if (tree->kind == TWO_NODE)
      if (key_lessp(indata, tree->data.two.two_data) < 0)
         return two_three_lookup(indata, tree->data.two.left_two_tree);
      else
         return two_three_lookup(indata, tree->data.two.right_two_tree);
   else if (key_lessp(indata, tree->data.three.left_three_data) < 0)
      return two_three_lookup(indata, tree->data.three.left_three_tree);
   else if (key_lessp(indata, tree->data.three.right_three_data) < 0)
      return two_three_lookup(indata, tree->data.three.middle_three_tree);
   else
      return two_three_lookup(indata, tree->data.three.right_three_tree);
}

/** @brief Look up a symbol by name in the root Token_Tree.
 *  @param token  Null-terminated symbol name to search for.
 *  @return       The tokenptr for the most-recently pushed definition,
 *                or nullptr if @p token is not in the table.
 */
static tokenptr Lookup_Token(const char *token)
{
	//printf("symtab::lookup_token: %s\n",token);
   struct key_data data;
   nodeptr node;

   data.key = token;
   data.data = nullptr;
   node = two_three_lookup(&data, Token_Tree);
   if (node == nullptr) {
	   //printf("symtab::lookup_token: not found\n");
      return nullptr;
  }
   else
      return (tokenptr)node->data.leaf.leaf_data->data;
}

/** @brief Push a new definition for @p token onto the symbol-table overload stack.
 *
 *  If @p token is not yet in the tree, calls Install_Token() to add it.
 *  Otherwise prepends @p data to the existing leaf's linked list so that
 *  Lookup_Token() returns it first (shadowing the previous definition).
 *  @param token  Symbol name.
 *  @param data   New tokenptr to push as the active definition.
 */
static void Push_Token(std::string token, tokenptr data)
{
   nodeptr node;
   nodedata tnode;
   struct key_data tdata;

   tdata.key = token;
   tdata.data = data;
   node = two_three_lookup(&tdata, Token_Tree);
   if (node == nullptr)
      /* Need to insert this information */
      Install_Token(token.c_str(), data);
   else {
#if 0
warning("Overloading '%s'(%d) with '%s'(%d)\n",
       ((tokenptr)(node->data.leaf.leaf_data->data))->name,
       ((tokenptr)(node->data.leaf.leaf_data->data))->type,
       data->name, data->type);
#endif
      /* Put the new information at the head of the list */
      tnode = new (std::nothrow) key_data;
      if (tnode == nullptr)
         serror("Failed to allocate a symbol table entry");
      tnode->key = token;
      tnode->data = data;
      tnode->next = node->data.leaf.leaf_data;
      node->data.leaf.leaf_data = tnode;
      }
}
#if 0
static tokenptr
Pop_Token(const char *token)
{
   nodeptr node;
   nodedata tnode;
   struct key_data data;
   void *result;

   data.key = token;
   data.data = nullptr;
   node = two_three_lookup(&data, Token_Tree);
   if (node == nullptr)
      /* It wasn't there - perhaps this is an error. */
      return nullptr;
   else {
      tnode = node->data.leaf.leaf_data;
      node->data.leaf.leaf_data = tnode->next;
      result = tnode->data;
      delete tnode;
      return result;
      }
}
#endif
/** @brief Traverse a 2-3 tree, call @p process on each leaf, then free the entire tree.
 *
 *  Post-order traversal: children are visited before the parent node is freed.
 *  All key_data chain nodes at each leaf are iterated and freed after calling
 *  @p process on each one.
 *  @param intree   Root of the subtree to shred.
 *  @param process  Callback invoked for each key/tokenptr pair before freeing.
 */
static void
shread_two_three(nodeptr intree, datafunc process)
{
   nodedata tnode1, tnode2;

   if (intree == nullptr)
      return;
   else if (intree->kind == LEAF_NODE) {
      /* Remove all entries */
      for (tnode1=intree->data.leaf.leaf_data;tnode1!=nullptr;) {
         tnode2 = tnode1;
         tnode1 = tnode1->next;
         process(tnode2->key.c_str(), (tokenptr)tnode2->data); /* User supplied deallocation */
         delete tnode2;
         }
      }
   else if (intree->kind == TWO_NODE) {
      shread_two_three(intree->data.two.left_two_tree, process);
      shread_two_three(intree->data.two.right_two_tree, process);
      }
   else {
      shread_two_three(intree->data.three.left_three_tree, process);
      shread_two_three(intree->data.three.middle_three_tree, process);
      shread_two_three(intree->data.three.right_three_tree, process);
      }
   delete (intree);
}

/** @brief Traverse a 2-3 tree in-order and call @p process on each leaf entry.
 *
 *  Unlike shread_two_three(), this does not free the tree - it is a read-only
 *  traversal used to iterate over all current definitions.
 *  @param tree     Root of the subtree to traverse.
 *  @param process  Callback invoked for each key/tokenptr pair found.
 */
static void
process_two_three(nodeptr tree, datafunc process)
{
   if (tree == nullptr)
      return;
   else if (tree->kind == LEAF_NODE)
      process(tree->data.leaf.leaf_data->key.c_str(),
              (tokenptr)tree->data.leaf.leaf_data->data);
   else if (tree->kind == TWO_NODE) {
      process_two_three(tree->data.two.left_two_tree, process);
      process_two_three(tree->data.two.right_two_tree, process);
      }
   else {
      process_two_three(tree->data.three.left_three_tree, process);
      process_two_three(tree->data.three.middle_three_tree, process);
      process_two_three(tree->data.three.right_three_tree, process);
      }
}
#if 0
static void
Process_Symbol_Table(datafunc process)
{
   process_two_three(Token_Tree, process);
}

static void
Initialize_Symbol_Table(void)
{
   Token_Tree = nullptr;
}

static void
Terminate_Symbol_Table(datafunc process)
{
   shread_two_three(Token_Tree, process);
   Token_Tree = nullptr;
}
#endif
/**
 * @brief Insert or override a symbol-table definition.
 * @param name Token name.
 * @param def_type Data type tag.
 * @param data Data payload.
 * @param static_flag Non-zero when definition is static.
 * @param noeval_flag Non-zero to skip expression folding.
 */
void Insert_Definition(const char *name, int def_type, void *data,
                  int static_flag, int noeval_flag)
{
   int found, i;
   Flt fval;
   Vec vval;
   NODE_PTR nval;
   tokenptr entries;

   entries = Lookup_Token(name);
   found = (entries == nullptr ? 0 : 1);

   /* If it wasn't there then add it. If it was there but wasn't a static
      variable, then overload it. */
   if (!found || !static_flag) {
      entries = FactoryTokenStruc();
      if (entries == nullptr)
         serror("Failed to allocate space for symbol: '%s'\n", name);
      //entries->name = new char[name.length() + 1];
      entries->sname = name; // new char[strlen(name) + 1];               
      }
   else
      /* If it was there & was static, then remove the old definition */
      Delete_Definition(entries);

   /* Build the new symbol entry */
   entries->type = def_type;
   entries->sflag = static_flag;

   // For expressions, we try to reduce the data to a float or vector 
   if (!noeval_flag && def_type == std::to_underlying(ShapeType::Expression)) {
      i = eval_node(nullptr, (NODE_PTR)data, &fval, vval, &nval);
      if (i == 1) {
         entries->data = make_value_node(fval);
         deallocate_node((NODE_PTR)data);
         }
      else if (i == 2) {
         entries->data = make_vec_node(vval[0], vval[1], vval[2]);
         deallocate_node((NODE_PTR)data);
         }
      else
         entries->data = data;
      }
   else
      entries->data = data;

   /* If this is a new entry, then push it onto the symbol table */
   if (!found || !static_flag)
      Push_Token(entries->sname, entries);
}

/**
 * @brief Look up a symbol-table definition.
 * @param name Token name.
 * @param def_type Output type tag.
 * @param data Output payload pointer.
 */
void Lookup_Definition(const char *name, int *def_type, void **data)
{
   #ifdef DEBUG_FN_CALLS
   smessage("symtab::Lookup_Definition");
   #endif
   tokenptr entries;

   entries = Lookup_Token(name);
   if (entries == nullptr) {
      *def_type = std::to_underlying(ShapeType::Null);
      *data = nullptr;
      }
   else {
      *def_type = entries->type;
      *data = entries->data;
      }

   #ifdef DEBUG_FN_CALLS
   smessage("symtab::Lookup_Definition returning");
   #endif
}

/**
 * @brief Return interned token text if a symbol exists.
 * @param name Token to query.
 * @return Stored token name string, or "" if not found.
 */
std::string Lookup_String(const char *name)
{
   tokenptr entries;

   entries = Lookup_Token(name);
   if (entries == nullptr)
      return "";
   else
      return entries->sname;
}

/** @brief Shread callback: retain static definitions, free non-static ones.
 *
 *  Static entries (sflag != 0) are re-inserted into New_Token_Tree so they
 *  survive the frame reset.  Non-static entries are freed via Delete_Definition().
 *  @param token  Symbol name (used as the re-insertion key).
 *  @param value  Token entry to evaluate and either retain or free.
 */
static void
Non_Static_Deallocation(const char *token, tokenptr value)
{
   struct key_data data;

   if (value->sflag) {
      data.key = token;
      data.data = value;
      New_Token_Tree = two_three_insert(New_Token_Tree, &data);
      }
   else {
      Delete_Definition(value);
      //delete[] value->name;
      value->sname="";
      delete value;
      }
}

/** @brief Shread callback: unconditionally free every definition including static ones.
 *  @param token  Symbol name (unused; present to satisfy datafunc signature).
 *  @param value  Token entry to free.
 */
static void
Complete_Deallocation(const char *token, tokenptr value)
{
    Ensures(value != nullptr);
    Delete_Definition(value);
   value->sname="";
   delete value;
}

/** @brief Clear the symbol table, retaining static definitions when @p all_flag is 0.
 *  @param all_flag  Non-zero to delete everything; 0 to preserve static entries.
 */
static void
Delete_All_Definitions(int all_flag)
{
   if (all_flag) {
      shread_two_three(Token_Tree, Complete_Deallocation);
      Token_Tree = nullptr;
      }
   else {
      shread_two_three(Token_Tree, Non_Static_Deallocation);
      Token_Tree = New_Token_Tree;
      New_Token_Tree = nullptr;
      }
}

/**
 * @brief Initialize frame-local/global symbol and scene state.
 */
void
Initialize_Symtab(void)
{
   /* Reset the root storage for all objects */
   Initialize_BinTree(RuntimeState::scene.Root);

   /* Lets make sure the background color is properly reset every frame */
   MakeVector(0, 0, 0, RuntimeState::BackgroundColor);
   RuntimeState::Background = nullptr;

   /* Set default values for the viewpoint */
   Initialize_Eye(&RuntimeState::scene.Eye);

   /* Uninitialize haze */
   RuntimeState::Global_Haze = 0.0;

   Draw_Commands = nullptr;
}

/**
 * @brief Release symbol-table and scene resources.
 * @param all_flag Non-zero to remove all definitions, including static ones.
 */
void Deallocate_Symtab(int all_flag)
{
   Delete_BinTree(&RuntimeState::scene.Root);
   Delete_All_Definitions(all_flag);

   if (RuntimeState::Background != nullptr) {
      deallocate_node(RuntimeState::Background);
      RuntimeState::Background = nullptr;
      }

   delete_draw_nodes(Draw_Commands);
   Draw_Commands = nullptr;
}

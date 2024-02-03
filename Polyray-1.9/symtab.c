/* symtab.c

   Support routines for reusable objects, strings, vectors, ...

  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.

  This software may be used for any private and non-commercial
  use.

  You may not distribute this software, in whole or in part,
  for any commercial purpose, without the express consent of
  the authors.

  There is no warranty or other guarantee of fitness of this software
  for any purpose.  It is provided solely "as is".

*/
#include <stdarg.h>
#include "defs.h"

#define IN_SYMTABC
#include "symtab.h"
#include "bound.h"
#include "particle.h"
#include "glyph.h"
#include "sweep.h"
#include "display.h"
#include "memory.h"
#include "io.h"
#include "eval.h"
#include "builder.h"
#include "csg.h"
#include "scan.h"
#include "texture.h"
#include "display.h"
#include "intersec.h"

/* Run-time globals */
int Parsed_Flag = 0;          /* First time reading input file */
int Shadow_Test = 0;          /* Are we just looking for shadows? */
int Particle_Test = 0;        /* Are we building particles? */
int File_Generation_Flag = 1; /* Normally generate a Targa image */

/* Variables used for generating sequences of frames */
int start_frame = 0;
int end_frame = -1;
int total_frames = 0;
int current_frame = 0;
Flt frame_time = 1.0;       /* Amount of time passing each frame */

DrawNode *Draw_Commands = NULL;

char *POLYRAY_PATH_STRING = "POLYRAY_PATH";


/* Statistics variables */
unsigned long nChecked = 0;
unsigned long nRays = 0;
unsigned long nShadows = 0;
unsigned long nReflected = 0;
unsigned long nRefracted = 0;
unsigned long nTIR = 0;
unsigned long nJittered = 0;

/* Global image variables */
int             buffer_update = 0;
unsigned long   buffer_size = 0;

/* Rendering quality options */
int pixelsize = 24;       /* 24 bits per pixel, 8 each RGB. */
int pixel_encoding = 1;   /* Default is RLE compression */
int DepthRender = 0;      /* Default is normal coloring */
int Optimizer = 1;        /* Slabs, array based sorting */
Flt minweight = 0.01;
int maxlevel = MAXLEVEL;
int tickflag = 2;
int antialias = 0; /* 0=none, 1=filter, 2-3=adaptive */
int maxsamples = 4;
Flt antialias_threshold = 0.0004;

long MaxBufferRAM = 2048L * 2048L; /* Maximum RAM (in Kbytes) used by S&Z buffers */

int             Rendering_Method = RAY_TRACING;
unsigned short  Global_Shade_Flag = UNSET_SFLAG;
Viewpoint       Eye = { 256, 256, 0, 0, 256, 256, 0, 255,
                        {0, 0, -1}, {0, 0, 0}, {0, 1, 0},
                        45.0, SMALL, PLY_HUGE, 0.0, 1.0, -1.0,
                        NULL, NULL, NULL};
Flt             Global_Haze = 0.0;
Flt             Global_Haze_Start = 0.0;
Vec             Global_Haze_Color = {0, 0, 0};
NODE_PTR        Background = NULL;
Vec             BackgroundColor;
Vec             White = {1.0, 1.0, 1.0};
BinTree         Root;
Flt             rayeps = 1e-3;
char            outfilebase[128];
int             filebaseflag = 0;
int             Check_Abort_Flag = 1; /* Look for an abort */
jmp_buf         abort_environ;
int             Abort_Flag = 0;

/* A matte white surface is used when none is specified */
extern float D_Phong(Vec, Vec, Vec, Flt);
Surface DefaultSurface =
   { {1.0f, 1.0f, 1.0f}, 0.2f, /* Ambient */
     1.0f,                     /* Brilliance */
     {1.0f, 1.0f, 1.0f}, 0.8f, /* Diffuse */
     {1.0f, 1.0f, 1.0f}, 0.0f, /* Specular */
     {1.0f, 1.0f, 1.0f}, 0.0f, /* Reflection */
     {1.0f, 1.0f, 1.0f}, 0.0f, /* Transmission */
     D_Phong,
     1.0f,
     1.0f};

/* Particle variables */
Particle *CurrentParticle = NULL;
Particle *Particles = NULL;
ParticleObject *ParticleObjects = NULL;

/* Clipping variables */
Poly_box box;
Window win;
Vec ViewVec;

/* Global light variables */
Light *Lights = NULL;
int   nLights = 0;
Light **light_array;

/* Statistics variables */
unsigned long totalShadows, totalShadowCaches;
unsigned long maxQueueSize = 0;
unsigned long totalQueues = 0;
unsigned long totalQueueResets = 0;
unsigned long nEnqueued = 0;

/* Bounding cluster size */
int clustersize = 4;

/* Global variables to keep track of where we are currently tracing */
int current_row;
int current_col;
int recursion_depth;

/* Variables for trimming CSG raw triangles */
Flt csg_leg_tolerance = 0.05;
Flt csg_subdivision_depth = 1;

/* Define runtime stack size */
#if defined( DOS386 )
unsigned _stack = 80000;
#endif

/* Use this define to print each function name as it is entered */
/* #define DEBUG_FN_CALLS */

/* Data structures and routines for maintaining name/value pairs in a
   2-3 tree */
typedef struct key_data *nodedata;
typedef struct node *nodeptr;

struct key_data {
   char *key;
   void *data;
   nodedata next;
   };

struct leaf_entry {
   nodedata leaf_data;
   };

struct two_entry {
   nodeptr left_two_tree, right_two_tree;
   nodedata two_data;
   };

struct three_entry {
   nodeptr left_three_tree, middle_three_tree, right_three_tree;
   nodedata left_three_data, right_three_data;
   };

struct node {
   int index;
   unsigned char kind;
   union {
      struct leaf_entry leaf;
      struct two_entry two;
      struct three_entry three;
      } data;
   };

#define LEAF_NODE  1
#define TWO_NODE   2
#define THREE_NODE 3
static nodeptr Token_Tree = NULL;
static nodeptr New_Token_Tree = NULL;

/* Main symbol table data structure */
typedef struct token_struct *tokenptr;
struct token_struct {
   char *name; /* Pointer to the stored name */
   int type;   /* Type of the data associated with a token */
   int sflag;  /* Flag indicating if this is static data */
   void *data; /* Anonymous data associated with a token */
   tokenptr next; /* Next entry in a stack of token values */
   };
typedef void (*datafunc)(char *token, tokenptr data);

#if 0
/* Routines to manipulate the symbol table */
void Initialize_Symbol_Table(void);
void Terminate_Symbol_Table(datafunc process);
void Show_Symbol_Table(void);

void Install_Token(char *token, tokenptr value);
void Delete_Token(char *token);
tokenptr Lookup_Token(char *token);
void Push_Token(char *token, tokenptr data);
tokenptr Pop_Token(char *token);

void Process_Symbol_Table(datafunc process);
#endif

int
istrcmp(char *str1, char *str2)
{
   int i=0;
   int l1 = strlen(str1);
   int l2 = strlen(str2);
   char temp1[128], temp2[128];

   strcpy(temp1, str1);
   strcpy(temp2, str2);
   while (i<l1 && i < l2) {
      if (temp1[i] >= 'A' && temp1[i] <= 'Z') temp1[i] += 32;
      if (temp2[i] >= 'A' && temp2[i] <= 'Z') temp2[i] += 32;
      i++;
      }
   return strcmp(temp1, temp2);
}

void
reset_subst(SUBST_PTR subst)
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

void
Initialize_Bean_Counters(void)
{
   nRays             = 0;
   nShadows          = 0;
   nReflected        = 0;
   nRefracted        = 0;
   nTIR              = 0;
   nChecked          = 0;
   nEnqueued         = 0;
   maxQueueSize      = 0;
   totalQueues       = 0;
   totalQueueResets  = 0;
   totalShadows      = 0;
   totalShadowCaches = 0;
}

ostackptr
push_object(ostackptr stack, Object *obj)
{
   ostackptr node = polyray_malloc(sizeof(struct object_stack_struct));

   if (node == NULL)
      error("Failed to allocate a node on the object stack\n");
   node->element = obj;
   node->next = stack;
   stack = node;
   return stack;
}

Object *
pop_object(ostackptr *stack)
{
   ostackptr node = *stack;
   Object *obj;

   obj = node->element;
   *stack = node->next;
   polyray_free(node);

   return obj;
}

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
   if (eye->WS != NULL) polyray_free(eye->WS);
   eye->WS = NULL;
   eye->ZBuffer = NULL;
   eye->SBuffer = NULL;
}

static void
deallocate_surface(Special_Surface *surf)
{
   deallocate_node(surf->body_color);
   deallocate_node(surf->normal);
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
   polyray_free(surf);
}

static void
Delete_Definition(tokenptr entry)
{
   switch (entry->type) {
   case T_PARTICLE:
      DeleteParticle((Particle *)entry->data);
      break;
   case T_EXPRESSION:
      deallocate_node((NODE_PTR)entry->data);
      break;
   case T_TRANSFORM:
      polyray_free(entry->data);
      break;
   case T_SURFACE:
      deallocate_surface(entry->data);
      break;
   case T_STRING:
      polyray_free(entry->data);
      break;
   case T_TEXTURE:
      TextureDelete(entry->data);
      break;
   case T_TEXTURE_MAP:
      delete_texture_map(entry->data);
      break;
   case T_OBJECT:
      Delete_Object(entry->data);
      break;
   default:
      error("Bad type value in 'Delete_Definition'\n");
   }
}

void
Initialize_BinTree(BinTree *root)
{
   root->slab_root = NULL;
   root->members.list = NULL;
   root->members.count = 0;
   root->csgprims.list = NULL;
   root->csgprims.count = 0;
   root->polyprims.list = NULL;
   root->polyprims.count = 0;
   root->eyeprims.list = NULL;
   root->eyeprims.count = 0;
   root->lights.list = NULL;
   root->lights.count = 0;
   root->MaxDepth = 64;
   root->MaxListLength = 4;
}

void
Add_To_BinTree(BinTree *root, Object *obj)
{
   BinTree temp_root;
   int old_method;
   Texture *text;
   Surface *surf;
   Special_Surface *spec_surf;
   Object *tobj;
   int displace_flag, OldOptim;

   /* First see if we can tweak the shading flags to improve speed during
      rendering. */
   if (obj->o_type != T_COMPOSITE && obj->o_type != T_POLYGON) {
      for (tobj=obj,text=obj->o_texture;
           text!=NULL && tobj!=NULL;
           tobj=tobj->o_parent)
         text = tobj->o_texture;
      if (text == NULL)
         ;
      else if (text->type == T_SPECIAL) {
         spec_surf = (Special_Surface *)(text->data);
         if ((spec_surf->Kr_scale == NULL) && (obj->o_sflag & REFLECT_CHECK))
            obj->o_sflag ^= REFLECT_CHECK;
         }
      else if (text->type == T_PLAIN) {
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

   for (tobj=obj,displace_flag=0;tobj!=NULL&&!displace_flag;tobj=tobj->o_parent)
      if (tobj->o_displace != NULL)
         displace_flag = 1;

   if (obj->o_type == T_CSG) {
      set_parent_ptrs(obj->o_data, NULL, obj, obj->o_trans, &obj->o_bnd);
      instantiate_csg(root, obj->o_data, displace_flag);
      root->csgprims.list = push_object(root->csgprims.list, obj);
      root->csgprims.count++;
      }
   else if ((Rendering_Method == RAY_TRACING ||
             ((Rendering_Method == SCAN_CONVERSION) &&
              (Global_Shade_Flag &
               (SHADOW_CHECK | REFLECT_CHECK | TRANSMIT_CHECK)))) &&
            (displace_flag ||
             obj->o_type == T_BEZIER ||
             obj->o_type == T_NURB ||
             obj->o_type == T_PARAMETRIC)) {
      old_method = Rendering_Method;
      Rendering_Method = MESH_CONVERSION;

      /* Create a temporary BinTree to hold the polygons as they are
         made by the scan converter */
      Initialize_BinTree(&temp_root);
      obj->o_procs->render(NULL, &temp_root, obj);
      OldOptim = Optimizer;
      Optimizer = 1;
      BuildBoundingSlabs(&temp_root);
      Optimizer = OldOptim;

      /* Now add the slabbed patch pieces to the global set of objects */
      if (temp_root.slab_root == NULL)
         error("Failed to add triangulated object to bintree");

      root->members.list = push_object(root->members.list,
                                       temp_root.slab_root);
      root->members.count++;
      while (temp_root.members.list != NULL)
         pop_object(&temp_root.members.list);
      root->polyprims.list = push_object(root->polyprims.list, obj);
      root->polyprims.count++;
      Rendering_Method = old_method;
      }
   else {
      root->members.list = push_object(root->members.list, obj);
      root->members.count++;
      }
}

void
Delete_BinTree(BinTree *root)
{
   ostackptr objs;
   Object *obj;

   if (root->slab_root != NULL) {
      Delete_Object(root->slab_root);
      root->slab_root = NULL;

      /* Delete the entire list of primitives, excluding any
         that are part of a CSG object */
      objs = root->members.list;
      while (objs != NULL)
         obj = pop_object(&objs);
      }
   else {
      objs = root->members.list;
      while (objs != NULL) {
         obj = pop_object(&objs);
         if (obj->o_type != T_LIGHT)
            Delete_Object(obj);
         }
      }
   root->members.list = NULL;
   root->members.count = 0;

   /* Delete the list of object bounds that contain the eye */
   objs = root->eyeprims.list;
   while (objs != NULL)
      obj = pop_object(&objs);
   root->eyeprims.list = NULL;
   root->eyeprims.count = 0;


   /* Delete all polygon objects */
   objs = root->polyprims.list;
   while (objs != NULL) {
      obj = pop_object(&objs);
      Delete_Object(obj);
      }
   root->polyprims.list = NULL;
   root->polyprims.count = 0;

   /* Delete all CSG objects */
   objs = root->csgprims.list;
   while (objs != NULL) {
      obj = pop_object(&objs);
      Delete_Object(obj);
      }
   root->csgprims.list = NULL;
   root->csgprims.count = 0;

   /* Delete all light objects */
   objs = root->lights.list;
   while (objs != NULL) {
      obj = pop_object(&objs);
      Delete_Object(obj);
      }
   root->lights.list = NULL;
   root->lights.count = 0;

}

/* Free any dynamic memory associated with this object */
void
Delete_Object(Object *obj)
{
   CompositeObject *cd;
   unsigned short i;

   /* Be careful if this is an object generated by the bounding routines */
   if (obj->o_type == T_COMPOSITE) {
      cd = (CompositeObject *)obj;
      for (i=0;i<cd->c_size;i++)
         Delete_Object(cd->c_object[i]);
      polyray_free(cd->c_object);
      polyray_free(obj);
      }
   else if (obj->o_type == T_POLYGON)
      polyray_free(obj);
   else {
      /* Delete any memory specific to this object */
      if (obj->o_copy == 0)
         (obj->o_procs->del)(obj);

      /* Delete any memory used for this objects texture description */
      if (obj->o_texture != NULL)
         TextureDelete(obj->o_texture);

      /* Delete any memory associated with this objects transformation */
      if (obj->o_trans != NULL)
         polyray_free(obj->o_trans);

      /* Deallocate displacement information */
      deallocate_node(obj->o_displace);

      /* Deallocate any triangle information */
      if (obj->o_vertices != NULL &&
          (obj->o_type != T_RAW_TRIANGLES || obj->o_copy == 0)) {
         polyray_free(obj->o_vertices->V);
         if (obj->o_vertices->N != NULL)
            polyray_free(obj->o_vertices->N);
         if (obj->o_vertices->U != NULL)
            polyray_free(obj->o_vertices->U);
         polyray_free(obj->o_vertices);
         }

      /* Free the memory used by this object */
      polyray_free(obj);
      }
}

/* Perform a "deep" copy of the object */
void
Copy_Object(Object *start_obj, Object *result_obj)
{
   /* Just in case, perform a type check on this thing to see if it
      really is an object. */
   if (start_obj->o_type < FIRST_OBJECT_TYPE ||
       start_obj->o_type > LAST_OBJECT_TYPE)
      error("Bad object type in Copy_Object: %d\n", start_obj->o_type);
   /* Copy the basic information */
   memcpy(result_obj, start_obj, sizeof(Object));

   /* Copy object specific information */
   (start_obj->o_procs->copy)(start_obj, result_obj);

   /* Copy the texture characteristics */
   if (start_obj->o_texture != NULL) {
      result_obj->o_texture = (Texture *)polyray_malloc(sizeof(Texture));
      if (result_obj->o_texture == NULL)
         error("Failed to allocate texture data\n");
      TextureCopy(start_obj->o_texture, result_obj->o_texture);
      }

   /* Copy any associated transformations */
   if (start_obj->o_trans != NULL) {
      result_obj->o_trans = polyray_malloc(sizeof(Transform));
      if (result_obj->o_trans == NULL)
         error("Failed to allocate transform data\n");
      memcpy(result_obj->o_trans, start_obj->o_trans, sizeof(Transform));
      }
   else
      result_obj->o_trans = NULL;

   /* Copy displacement expression */
   result_obj->o_displace = copy_node(start_obj->o_displace);
}

/* If there are no special initialization requirements for this
   object type then we return without doing anything.  */
int
GenericInitialize(Object *obj)
{
   return 1;
}

/* If there is no dynamically allocated memory associated with this
   object, then a simple copy flag is all that is necessary. */
void
GenericCopy(Object *objin, Object *objout)
{
   objout->o_copy = 1;
}

/* Most objects only need to free a single block of memory.  In this case
   this generic deletion call can be used. */
void
GenericDelete(Object *object)
{
   if (object->o_copy == 0)
      polyray_free(object->o_data);
}

void
GenericRender(Viewpoint *eye, BinTree *Root, Object *obj)
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

static void
tools_error(int line, char *key)
{
   warning("Bad initialization value for '%s', at line %d\n", key, line);
}

/* Open the file "Polyray.ini" and scan it for default values.  Each
   entry in the file has the form "a b" where a is the name of a default
   and b is the value of the default. */

FILE *PathFileOpen(char *,char *,char *);

void
Read_Initialization_Data()
{
   int temp, cnt, line_count = 0;
   FILE *ifile;
   char buffer[128], key[64];
   char val1[32], val2[32], val3[32], val4[32];

  if ((ifile = PathFileOpen(POLYRAY_PATH_STRING, "polyray.ini", "r")) == NULL)
      return;
   while (fgets(&buffer[0], 128, ifile) != NULL) {
      line_count++;
      if ((cnt = sscanf(buffer, "%s %s %s %s %s",
                 key, val1, val2, val3, val4)) == 2) {
         /* Got the options. Now figure out what they are */
         if (!istrcmp(key, "//"))
            continue;
         else if (!istrcmp(key, "error_log")) {
            SetMessageLog(val1);
            }
         else if (!istrcmp(key, "renderer"))
         {
            if (!istrcmp(val1, "ray_trace") || !istrcmp(val1, "raytrace"))
               Rendering_Method = RAY_TRACING;
            else {
               if (!istrcmp(val1, "scan_convert"))
                  Rendering_Method = SCAN_CONVERSION;
               else { if (!istrcmp(val1, "wire_frame") ||
                         !istrcmp(val1, "wireframe"))
                       Rendering_Method = WIRE_FRAME;
               else { if (!istrcmp(val1, "hidden_line"))
               Rendering_Method = HIDDEN_LINE;
            else { if (!istrcmp(val1, "gourad"))
               Rendering_Method = GOURAD_SHADE;
            else { if (!istrcmp(val1, "raw_triangles") ||
                     !istrcmp(val1, "triangles"))
               Rendering_Method = RAW_TRIANGLES;
            else { if (!istrcmp(val1, "uv_triangles"))
               Rendering_Method = UV_TRIANGLES;
            else { if (!istrcmp(val1, "csg_triangles"))
               Rendering_Method = CSG_TRIANGLES;
            else
               tools_error(line_count, key);
			   } } } } } } }
		}
         else if (!istrcmp(key, "max_level") ||
                  !istrcmp(key, "max_trace_level") ||
                  !istrcmp(key, "maxlevel")) {
            maxlevel = atoi(val1);
            if (maxlevel < 1 || maxlevel >= 128) {
               warning("Maxlevel must be less than 128\n");
               maxlevel = 7;
               }
            }
         else if (!istrcmp(key, "display")) {
            if (!istrcmp(val1, "none"))
               Display_Flag = 0;
            else if (!istrcmp(val1, "vga") || !istrcmp(val1, "vga1"))
               Display_Flag = 1;
            else if (!istrcmp(val1, "vga2"))
               Display_Flag = 2;
            else if (!istrcmp(val1, "vga3"))
               Display_Flag = 3;
            else if (!istrcmp(val1, "vga4"))
               Display_Flag = 4;
            else if (!istrcmp(val1, "vga5"))
               Display_Flag = 5;

            else if (!istrcmp(val1, "hicolor") || !istrcmp(val1, "hicolor1"))
               Display_Flag = 6;
            else if (!istrcmp(val1, "hicolor2"))
               Display_Flag = 7;
            else if (!istrcmp(val1, "hicolor3"))
               Display_Flag = 8;
            else if (!istrcmp(val1, "hicolor4"))
               Display_Flag = 9;
            else if (!istrcmp(val1, "hicolor5"))
               Display_Flag = 10;

            else if (!istrcmp(val1, "16bit1"))
               Display_Flag = 11;
            else if (!istrcmp(val1, "16bit2"))
               Display_Flag = 12;
            else if (!istrcmp(val1, "16bit3"))
               Display_Flag = 13;
            else if (!istrcmp(val1, "16bit4"))
               Display_Flag = 14;
            else if (!istrcmp(val1, "16bit5"))
               Display_Flag = 15;

            else if (!istrcmp(val1, "truecolor1"))
               Display_Flag = 16;
            else if (!istrcmp(val1, "truecolor2"))
               Display_Flag = 17;
            else if (!istrcmp(val1, "truecolor3"))
               Display_Flag = 18;
            else if (!istrcmp(val1, "truecolor4"))
               Display_Flag = 19;
            else if (!istrcmp(val1, "truecolor5"))
               Display_Flag = 20;

            else if (!istrcmp(val1, "4bit1"))
               Display_Flag = 21;
            else if (!istrcmp(val1, "4bit2"))
               Display_Flag = 22;

            else
               tools_error(line_count, key);
            }
         else if (!istrcmp(key, "pallette")) {
            if (!istrcmp(val1, "grey") || !istrcmp(val1, "greyscale") ||
                !istrcmp(val1, "gray") || !istrcmp(val1, "grayscale"))
               Pallette_Flag = 0;
            else if (!istrcmp(val1, "884"))
               Pallette_Flag = 1;
            else if (!istrcmp(val1, "666"))
               Pallette_Flag = 2;
            else if (!istrcmp(val1, "4bit"))
               Pallette_Flag = 3;
            else
               tools_error(line_count, key);
            }
         else if (!istrcmp(key, "pallette_start")) {
            temp = atoi(val1);
            if (temp < 0 || temp > 240) {
             warning("First entry of pallette must be between 0 and 240\n");
             }
            else
               Pallette_Start = temp;
            }
         else if (!istrcmp(key, "antialias")) {
            if (!istrcmp(val1, "none"))
               antialias = 0;
            else if (!istrcmp(val1, "filter"))
               antialias = 1;
            else if (!istrcmp(val1, "adaptive1"))
               antialias = 2;
            else if (!istrcmp(val1, "adaptive2"))
               antialias = 3;
            else
               tools_error(line_count, key);
            }
         else if (!istrcmp(key, "maxsamples") ||
                  !istrcmp(key, "max_samples")) {
            maxsamples = atoi(val1);
            if (maxsamples < 1)
               tools_error(line_count, key);
            }
         else if (!istrcmp(key, "maxscreenbuffer")) {
            MaxBufferRAM = 1024L * atoi(val1);
            if (MaxBufferRAM < 1)
               tools_error(line_count, key);
            }
         else if (!istrcmp(key, "aliasthreshold") ||
                  !istrcmp(key, "alias_threshold")) {
            antialias_threshold = atof(val1);
            antialias_threshold *= antialias_threshold;
            if (antialias_threshold < 0.0)
               tools_error(line_count, key);
            }
         else if (!istrcmp(key, "pixelsize") ||
                  !istrcmp(key, "pixel_size")) {
            pixelsize = atoi(val1);
            if (pixelsize != 8 && pixelsize != 16 &&
                pixelsize != 24 && pixelsize != 32) {
               warning("Pixel size must be 8, 16, 24, or 32 bits\n");
               tools_error(line_count, key);
               }
            }
         else if (!istrcmp(key, "pixelencoding") ||
                  !istrcmp(key, "pixel_encoding")) {
            pixel_encoding = atoi(val1);
            if (!istrcmp(val1, "none"))
               pixel_encoding = 0;
            else if (!istrcmp(val1, "rle"))
               pixel_encoding = 1;
            else
               tools_error(line_count, key);
            }
         else if (!istrcmp(key, "clustersize") ||
                  !istrcmp(key, "cluster_size")) {
            clustersize = atoi(val1);
            }
         else if (!istrcmp(key, "status") || !istrcmp(key, "line_counter")) {
            status_flag = 1;
            if (!istrcmp(val1, "none") || !istrcmp(val1, "off")) {
               status_flag = 0;
               tickflag = 0;
               }
            else if (!istrcmp(val1, "totals"))
               tickflag = 1;
            else if (!istrcmp(val1, "line"))
               tickflag = 2;
            else if (!istrcmp(val1, "pixel"))
               tickflag = 3;
            else
               tools_error(line_count, key);
            }
         else if (!istrcmp(key, "aborttest") || !istrcmp(key, "abort_test")) {
            if (!istrcmp(val1, "off"))
               Check_Abort_Flag = 0;
            else if (!istrcmp(val1, "false"))
               Check_Abort_Flag = 0;
            else if (!istrcmp(val1, "slow"))
               Check_Abort_Flag = 2;
            else if (!istrcmp(val1, "on"))
               Check_Abort_Flag = 1;
            else if (!istrcmp(val1, "true"))
               Check_Abort_Flag = 1;
            else
               tools_error(line_count, key);
            }
         else if (!istrcmp(key, "warnings")) {
            if (!istrcmp(val1, "on"))
               warnings_flag = 1;
            else if (!istrcmp(val1, "off"))
               warnings_flag = 0;
            else
               tools_error(line_count, key);
            }
         else if (!istrcmp(key, "errors")) {
            if (!istrcmp(val1, "on"))
               errors_flag = 1;
            else if (!istrcmp(val1, "off"))
               errors_flag = 0;
            else
               tools_error(line_count, key);
            }
         else if (!istrcmp(key, "dither")) {
            if (!istrcmp(val1, "on"))
               Dither_Flag = 1;
            else if (!istrcmp(val1, "off"))
               Dither_Flag = 0;
            else
               tools_error(line_count, key);
            }
         else if (!istrcmp(key, "shadeflags") ||
                  !istrcmp(key, "shade_flags")) {
            if (!istrcmp(val1, "default"))
               if (Rendering_Method == RAY_TRACING)
                  Global_Shade_Flag = SHADOW_CHECK | REFLECT_CHECK |
                                      TRANSMIT_CHECK | UV_CHECK | NORMAL_CORRECT;
               else
                  Global_Shade_Flag = 0;
            else
               Global_Shade_Flag = atoi(val1);
            if (Global_Shade_Flag > ALL_SHADE_FLAGS)
               tools_error(line_count, key);
            }
         else if (!istrcmp(key, "shadow_tolerance")) {
            rayeps = atof(val1);
            if (rayeps < 0.0) tools_error(line_count, key);
            }
         else if (!istrcmp(key, "csg_tolerance")) {
            csg_leg_tolerance = atof(val1);
            if (csg_leg_tolerance < 0.0) tools_error(line_count, key);
            }
         else if (!istrcmp(key, "csg_subdivisions")) {
            csg_subdivision_depth = atoi(val1);
            if (csg_subdivision_depth < 0) tools_error(line_count, key);
            }
         else if (!istrcmp(key, "optimizer")) {
            if (!istrcmp(val1, "none"))
               Optimizer = 0;
            else if (!istrcmp(val1, "slabs") ||
                     !istrcmp(val1, "bounding_slabs"))
               Optimizer = 1;
            else {
               warning("Optimization method must be: none or slabs\n");
               tools_error(line_count, key);
               }
            }
         else {
            warning("Unknown key: %s in polyray.ini\n", key);
            }
         }
      else if (cnt == 3) {
         if (!istrcmp(key, "//"))
            continue;
         else if (!istrcmp(key, "resolution")) {
            Eye.view_xres = atoi(val1);
            Eye.view_yres = atoi(val2);
            }
         else {
            warning("Unknown key: %s in polyray.ini\n", key);
            }
         }
      else if (cnt == 5) {
         if (!istrcmp(key, "//"))
            continue;
         else if (!istrcmp(key, "screen_window")) {
            Display_x0 = atoi(val1);
            Display_y0 = atoi(val2);
            Display_xl = atoi(val3);
            Display_yl = atoi(val4);
            }
         else if (!istrcmp(key, "image_window")) {
            Eye.view_x0 = atoi(val1);
            Eye.view_y0 = atoi(val2);
            Eye.view_xl = atoi(val3);
            Eye.view_yl = atoi(val4);
            }
         else {
            warning("Unknown key: %s in polyray.ini\n", key);
            }
         }
      else if (!istrcmp(key, "//"))
         continue;
      else {
         warning("Unknown key: %s in polyray.ini\n", key);
         }
      }
   if (fclose(ifile))
      warning("Failed to close 'polyray.ini'\n");
}

static void
show_two_three(nodeptr tree, int depth)
{
   int i;

   for (i=0;i<depth;i++)
      message(" ");
   if (tree == NULL)
      message("NULL\n");
   else if (tree->kind == LEAF_NODE)
      message("l(%s)\n", tree->data.leaf.leaf_data->key);
   else if (tree->kind == TWO_NODE) {
      message("n2(");
      switch (tree->data.two.left_two_tree->kind) {
         case LEAF_NODE:
            message("-l-,");
            break;
         case TWO_NODE:
            message("-n2-,");
            break;
         case THREE_NODE:
            message("-n3-,");
         }
      message("%s", tree->data.two.two_data->key);
      switch (tree->data.two.right_two_tree->kind) {
         case LEAF_NODE:
            message(",-l-)\n");
            break;
         case TWO_NODE:
            message(",-n2-)\n");
            break;
         case THREE_NODE:
            message(",-n3-)\n");
         }
      show_two_three(tree->data.two.left_two_tree, depth+1);
      show_two_three(tree->data.two.right_two_tree, depth+1);
      }
   else {
      message("n3(");
      switch (tree->data.three.left_three_tree->kind) {
         case LEAF_NODE:
            message("-l-,");
            break;
         case TWO_NODE:
            message("-n2-,");
            break;
         case THREE_NODE:
            message("-n3-,");
         }
      message("%s", tree->data.three.left_three_data->key);
      switch (tree->data.three.middle_three_tree->kind) {
         case LEAF_NODE:
            message(",-l-,");
            break;
         case TWO_NODE:
            message(",-n2-,");
            break;
         case THREE_NODE:
            message(",-n3-,");
         }
      message("%s", tree->data.three.right_three_data->key);
      switch (tree->data.three.right_three_tree->kind) {
         case LEAF_NODE:
            message(",-l-)\n");
            break;
         case TWO_NODE:
            message(",-n2-)\n");
            break;
         case THREE_NODE:
            message(",-n3-)\n");
         }
      show_two_three(tree->data.three.left_three_tree, depth+1);
      show_two_three(tree->data.three.middle_three_tree, depth+1);
      show_two_three(tree->data.three.right_three_tree, depth+1);
      }
}

/* Insert a name/attribute pair into a 2-3 tree. */
/* Create a new leaf node */
static nodeptr
new_node(char *key, void *data)
{
   nodeptr temp_node;
   nodedata temp_data;

   temp_node = (nodeptr)polyray_malloc(sizeof(struct node));
   temp_data = (nodedata)polyray_malloc(sizeof(struct key_data));
   if (temp_node == NULL || temp_data == NULL)
      error("Out of memory\n");
   temp_node->kind = LEAF_NODE;
   temp_node->data.leaf.leaf_data = temp_data;
   temp_data->key  = key;
   temp_data->data = data;
   temp_data->next = NULL;
   return temp_node;
}

static int
key_lessp(nodedata key1, nodedata key2)
{
   return strcmp(key1->key, key2->key);
}

static int
split(nodeptr intree, nodedata indata,
      nodeptr *left_tree, nodedata *outdata, nodeptr *right_tree)
{
   int result;
   nodedata data1, new_data;
   nodeptr temp1, new_tree1, new_tree2;

   if (intree == NULL)
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
         if ((*right_tree = (nodeptr)polyray_malloc(sizeof(struct node))) == NULL)
            error("Out of memory\n");
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
         if ((*right_tree = (nodeptr)polyray_malloc(sizeof(struct node))) == NULL)
            error("Out of memory\n");
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
         if ((*left_tree = (nodeptr)polyray_malloc(sizeof(struct node))) == NULL)
            error("Out of memory\n");
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

static int
insert3(nodeptr intree, nodedata indata, nodeptr *outtree)
{
   int result;
   nodedata new_data, data1, data2;
   nodeptr temp1, temp3;
   nodeptr new_tree1, new_tree2;

   if (intree == NULL) {
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

static nodeptr
two_three_insert(nodeptr intree, nodedata indata)
{
   nodeptr left_tree, right_tree, result;
   nodedata new_data;

   if (insert3(intree, indata, &left_tree))
      result = left_tree;
   else if (split(intree, indata, &left_tree, &new_data, &right_tree)) {
      result = (nodeptr)polyray_malloc(sizeof(struct node));
      if (result == NULL)
         error("Out of memory\n");
      result->kind = TWO_NODE;
      result->data.two.left_two_tree  = left_tree;
      result->data.two.two_data       = new_data;
      result->data.two.right_two_tree = right_tree;
      }
   else
      result = intree;
   return result;
}

static nodedata
two_three_least(nodeptr intree)
{
   if (intree == NULL)
      return NULL;
   else if (intree->kind == LEAF_NODE)
      return intree->data.leaf.leaf_data;
   else if (intree->kind == TWO_NODE)
      return two_three_least(intree->data.two.left_two_tree);
   else
      return two_three_least(intree->data.three.left_three_tree);
}

/* Delete a name/attribute pair from a 2-3 tree. */
static int
two_three_delete(nodeptr intree, nodedata indata, nodeptr *outtree)
{
   nodedata data1, data2;
   nodeptr tree1, tree2, tree3, tree4;

   *outtree = intree;

   if (intree == NULL)
      return 0;

   if (intree->kind == LEAF_NODE)
      if (key_lessp(indata, intree->data.leaf.leaf_data) != 0) {
         polyray_free(intree->data.leaf.leaf_data);
         polyray_free(intree);
         *outtree = NULL;
         return 1;
         }
      else
         return 0;
   else if (intree->kind == TWO_NODE)
      if (key_lessp(indata, intree->data.two.two_data) < 0)
         if (two_three_delete(intree->data.two.left_two_tree, indata, &tree1))
            if (tree1 == NULL) {
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
         if (tree1 == NULL) {
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
         if (tree1 == NULL) {
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
         if (tree1 == NULL) {
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
            polyray_free(intree->data.three.middle_three_tree);
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
      if (tree1 == NULL) {
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
         polyray_free(intree->data.three.right_three_tree);
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

/* Insert values for all of the predefined attribute names (none of these
   things actually have data, we just want to save names) */
static void
Install_Token(char *token, tokenptr value)
{
   struct key_data data;

   data.key = token;
   data.data = value;
   Token_Tree = two_three_insert(Token_Tree, &data);
}
#if 0
/* Remove a token from the tree */
static void
Delete_Token(char *token)
{
   struct key_data data;

   data.key = token;
   data.data = NULL;
   two_three_delete(Token_Tree, &data, &Token_Tree);
}
#endif
static nodeptr
two_three_lookup(nodedata indata, nodeptr tree)
{
   if (tree == NULL)
      return NULL;
   else if (tree->kind == LEAF_NODE)
      if (!strcmp(indata->key, tree->data.leaf.leaf_data->key))
         return tree;
      else
         return NULL;
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

/* Get the data associated with a token */
static tokenptr
Lookup_Token(char *token)
{
	//printf("symtab::lookup_token: %s\n",token);
   struct key_data data;
   nodeptr node;

   data.key = token;
   data.data = NULL;
   node = two_three_lookup(&data, Token_Tree);
   if (node == NULL) {
	   //printf("symtab::lookup_token: not found\n");
      return NULL;
  }
   else
      return node->data.leaf.leaf_data->data;
}

static void
Push_Token(char *token, tokenptr data)
{
   nodeptr node;
   nodedata tnode;
   struct key_data tdata;

   tdata.key = token;
   tdata.data = data;
   node = two_three_lookup(&tdata, Token_Tree);
   if (node == NULL)
      /* Need to insert this information */
      Install_Token(token, data);
   else {
#if 0
warning("Overloading '%s'(%d) with '%s'(%d)\n",
       ((tokenptr)(node->data.leaf.leaf_data->data))->name,
       ((tokenptr)(node->data.leaf.leaf_data->data))->type,
       data->name, data->type);
#endif
      /* Put the new information at the head of the list */
      tnode = polyray_malloc(sizeof(struct key_data));
      if (tnode == NULL)
         error("Failed to allocate a symbol table entry");
      tnode->key = token;
      tnode->data = data;
      tnode->next = node->data.leaf.leaf_data;
      node->data.leaf.leaf_data = tnode;
      }
}
#if 0
static tokenptr
Pop_Token(char *token)
{
   nodeptr node;
   nodedata tnode;
   struct key_data data;
   void *result;

   data.key = token;
   data.data = NULL;
   node = two_three_lookup(&data, Token_Tree);
   if (node == NULL)
      /* It wasn't there - perhaps this is an error. */
      return NULL;
   else {
      tnode = node->data.leaf.leaf_data;
      node->data.leaf.leaf_data = tnode->next;
      result = tnode->data;
      polyray_free(tnode);
      return result;
      }
}
#endif
static void
shread_two_three(nodeptr intree, datafunc process)
{
   nodedata tnode1, tnode2;

   if (intree == NULL)
      return;
   else if (intree->kind == LEAF_NODE) {
      /* Remove all entries */
      for (tnode1=intree->data.leaf.leaf_data;tnode1!=NULL;) {
         tnode2 = tnode1;
         tnode1 = tnode1->next;
         process(tnode2->key, tnode2->data); /* User supplied deallocation */
         /* Do we need this: polyray_free(tnode2->key); */
         polyray_free(tnode2);
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
   polyray_free(intree);
}

/* Here we get to some implementation specfic uses of the 2-3 tree. */
/* Access each entry in the tree and do something with it... */
static void
process_two_three(nodeptr tree, datafunc process)
{
   if (tree == NULL)
      return;
   else if (tree->kind == LEAF_NODE)
      process(tree->data.leaf.leaf_data->key,
              tree->data.leaf.leaf_data->data);
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
   Token_Tree = NULL;
}

static void
Terminate_Symbol_Table(datafunc process)
{
   shread_two_three(Token_Tree, process);
   Token_Tree = NULL;
}
#endif
void
Insert_Definition(char *name, int def_type, void *data,
                  int static_flag, int noeval_flag)
{
   int found, i;
   Flt fval;
   Vec vval;
   NODE_PTR nval;
   tokenptr entries;

   entries = Lookup_Token(name);
   found = (entries == NULL ? 0 : 1);

   /* If it wasn't there then add it. If it was there but wasn't a static
      variable, then overload it. */
   if (!found || !static_flag) {
      entries = (tokenptr)polyray_malloc(sizeof(struct token_struct));
      if (entries == NULL)
         error("Failed to allocate space for symbol: '%s'\n", name);
      entries->name = (char *)polyray_malloc((strlen(name)+1) * sizeof(char));
      if (entries->name == NULL)
         error("Failed to allocate space for symbol: '%s'\n", name);
      strcpy(entries->name, name);
      }
   else
      /* If it was there & was static, then remove the old definition */
      Delete_Definition(entries);

   /* Build the new symbol entry */
   entries->type = def_type;
   entries->sflag = static_flag;

   /* For expressions, we try to reduce the data to a float or vector */
   if (!noeval_flag && def_type == T_EXPRESSION) {
      i = eval_node(NULL, data, &fval, vval, &nval);
      if (i == 1) {
         entries->data = make_value_node(fval);
         deallocate_node(data);
         }
      else if (i == 2) {
         entries->data = make_vec_node(vval[0], vval[1], vval[2]);
         deallocate_node(data);
         }
      else
         entries->data = data;
      }
   else
      entries->data = data;

   /* If this is a new entry, then push it onto the symbol table */
   if (!found || !static_flag)
      Push_Token(entries->name, entries);
}

/* The values def_type and data are set to the values found in the symbol
   table. */
void
Lookup_Definition(char *name, int *def_type, void **data)
{
   tokenptr entries;

   entries = Lookup_Token(name);
   if (entries == NULL) {
      *def_type = T_NULL;
      *data = NULL;
      }
   else {
      *def_type = entries->type;
      *data = entries->data;
      }
}

/* See if we already know about this token.  If so, then return a
   pointer to the saved name of the token. */
char *Lookup_String(char *name)
{
   tokenptr entries;

   entries = Lookup_Token(name);
   if (entries == NULL)
      return NULL;
   else
      return entries->name;
}

static void
Non_Static_Deallocation(char *token, tokenptr value)
{
   struct key_data data;

   if (value->sflag) {
      data.key = token;
      data.data = value;
      New_Token_Tree = two_three_insert(New_Token_Tree, &data);
      }
   else {
      Delete_Definition(value);
      polyray_free(value->name);
      polyray_free(value);
      }
}

static void
Complete_Deallocation(char *token, tokenptr value)
{
   Delete_Definition(value);
   polyray_free(value->name);
   polyray_free(value);
}

static void
Delete_All_Definitions(int all_flag)
{
   if (all_flag) {
      shread_two_three(Token_Tree, Complete_Deallocation);
      Token_Tree = NULL;
      }
   else {
      shread_two_three(Token_Tree, Non_Static_Deallocation);
      Token_Tree = New_Token_Tree;
      New_Token_Tree = NULL;
      }
}

void
Initialize_Symtab(void)
{
   /* Reset the root storage for all objects */
   Initialize_BinTree(&Root);

   /* Lets make sure the background color is properly reset every frame */
   MakeVector(0, 0, 0, BackgroundColor);
   Background = NULL;

   /* Set default values for the viewpoint */
   Initialize_Eye(&Eye);

   /* Uninitialize haze */
   Global_Haze = 0.0;

   Draw_Commands = NULL;
}

void
Deallocate_Symtab(int all_flag)
{
   Delete_BinTree(&Root);
   Delete_All_Definitions(all_flag);

   if (Background != NULL) {
      deallocate_node(Background);
      Background = NULL;
      }

   delete_draw_nodes(Draw_Commands);
   Draw_Commands = NULL;
}

/* psupport.c

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
#include "parse.h"
#include "memory.h"
#include "builder.h"
#include "psupport.h"
#include "ytab.h"
#include "symtab.h"
#include "eval.h"
#include "pic.h"
#include "bound.h"
#include "blob.h"
#include "polynom.h"
#include "torus.h"
#include "revolve.h"
#include "roots.h"
#include "csg.h"
#include "texture.h"
#include "simplify.h"
#include "vector.h"


extern int outformat;


/* #define DEBUG_FN_CALLS */
/* Parser support variables */
fVec *pl, *plist;
ostackptr Object_Stack = NULL;
tstackptr Texture_Stack = NULL;
blobstackptr blob_components = NULL;
blobstackptr blob_component;
int condition_flags[MAX_CONDITION_DEPTH];
int condition_depth = 0;
Transform *Current_Transform = NULL;
Special_Surface *CurrentSurface;
int npoints = 0;
int ObjectDepth = 0;
txstackptr txstack = NULL;
UVVert tri_vertex[3];

void
InitializeSurface(Surface *NewSurf)
{
    MakeVector(1.0, 1.0, 1.0, NewSurf->Ka_color);/* ambient is White */
    NewSurf->Ka_scale = 0.1;                     /* ambient scale small */
    NewSurf->Kb_power = 1.0;                     /* No diffuse modification */
    MakeVector(1.0, 1.0, 1.0, NewSurf->Kd_color);/* diffuse is White */
    NewSurf->Kd_scale = 0.9;                     /* diffuse scale high */
    MakeVector(1.0, 1.0, 1.0, NewSurf->Ks_color);/* Specular is White */
    NewSurf->Ks_scale = 0.0;                     /* No specular (matte) */
    MakeVector(1.0, 1.0, 1.0, NewSurf->Kr_color);/* Reflect all colors */
    NewSurf->Kr_scale = 0.0;                     /* No reflectivity */
    MakeVector(1.0, 1.0, 1.0, NewSurf->Kt_color);/* Reflect all colors */
    NewSurf->Kt_scale = 0.0;                     /* No transmission */
    NewSurf->D = NULL;                           /* Use Phong highlighting */
    NewSurf->D_coeff = 2.0;                      /* Precompute Phong coeff */
    NewSurf->ior = 1.0;                          /* Index of refraction = air */
}

void
InitializeSpecialSurface(Special_Surface *NewSurf)
{
   NewSurf->Position_fn = NULL;
   NewSurf->Pos_scale   = NULL;
   NewSurf->Lookup_fn   = NULL;
   NewSurf->Turbulence  = NULL;
   NewSurf->Octaves     = NULL;
   NewSurf->Frequency   = NULL;
   NewSurf->Phase       = NULL;
   NewSurf->Bump_scale  = NULL;
   NewSurf->body_color  = NULL;
   NewSurf->normal      = NULL;
   NewSurf->position    = NULL;
   NewSurf->Ka_color    = NULL;
   NewSurf->Ka_scale    = NULL;
   NewSurf->Kb_power    = NULL;
   NewSurf->Kd_color    = NULL;
   NewSurf->Kd_scale    = NULL;
   NewSurf->Ks_color    = NULL;
   NewSurf->Ks_scale    = NULL;
   NewSurf->Kr_color    = NULL;
   NewSurf->Kr_scale    = NULL;
   NewSurf->Kt_color    = NULL;
   NewSurf->Kt_scale    = NULL;
   NewSurf->ior         = NULL;
   NewSurf->D_type      = PHONG;
   NewSurf->D_angle     = NULL;
   NewSurf->map         = NULL;
}

void
build_outfile_name(char *outfilebase, char *outfilename)
{
     char tmpstr[128];

   if (outfilebase == NULL)
      error("NULL outfile name\n");
   else if (strlen(outfilebase) == 0) {
      warning("Zero length outfile name, resetting to 'out'\n");
      strcpy(outfilebase, "out");
      }
   else if (strlen(outfilebase) > 120) {
      outfilebase[120] = '\0';
      warning("Output file name too long, truncating to: '%s'\n",
              outfilebase);
      }
   strcpy(outfilename, outfilebase);
   sprintf(tmpstr, "%03d", current_frame);
   strcat(outfilename, tmpstr);

/* CM added 10/mar/2000 */
   if (outformat == OUT_TARGA) {
     strcat(outfilename, ".tga");
   }
   else
   if (outformat == OUT_RAWPPM) {
     strcat(outfilename, ".ppm");
   }
   else
     strcat(outfilename, ".bin");
/* CM end -- original line was strcat(outfilename, ".tga"); */

}

int
check_condition(void)
{
   int i;

   for (i=0;i<=condition_depth;i++)
      if (!condition_flags[i]) {
         return 0;
         }
   return 1;
}

void
surface_action1(void)
{
   CurrentSurface = (Special_Surface *)polyray_malloc(sizeof(Special_Surface));

   if (CurrentSurface == NULL) error("Failed to allocate a surface\n");
   InitializeSpecialSurface(CurrentSurface);
}

void
surface_action2(char *surf_name)
{
   int c;
   void *d;

   Lookup_Definition(surf_name, &c, &d);
   if (c != T_SURFACE)
      error("Surface '%s' not found in symbol table\n", surf_name);
   CurrentSurface = (Special_Surface *)polyray_malloc(sizeof(Special_Surface));
   if (CurrentSurface == NULL) error("Failed to allocate a surface\n");
   copy_special0((Special_Surface *)d, CurrentSurface);
}

void
push_texture(Texture *text)
{
   tstackptr node = polyray_malloc(sizeof(struct texture_stack_struct));

   if (node == NULL)
      error("Failed to allocate a node on the texture stack\n");
   node->element = text;
   node->next = Texture_Stack;
   Texture_Stack = node;
}

Texture *
pop_texture(void)
{
   tstackptr node = Texture_Stack;
   Texture *last_texture = Texture_Stack->element;

   Texture_Stack = Texture_Stack->next;
   polyray_free(node);
   return last_texture;
}

texture_map_entries
make_texture_map_entry(Flt p0, Flt p1, Texture *t0, Texture *t1)
{
   texture_map_entries node = polyray_malloc(sizeof(struct texture_map_struct));
   if (node == NULL)
      error("Failed to allocate a texture map entry\n");
   node->p0 = p0;
   node->p1 = p1;
   node->t0 = t0;
   node->t1 = t1;
   node->next = NULL;
   return node;
}

texture_map_entries
copy_texture_map(texture_map_entries map)
{
   Texture *t0, *t1;
   texture_map_entries head = NULL, tail = NULL, temp;

   for (temp=map;temp!=NULL;temp=temp->next) {
      t0 = (Texture *)polyray_malloc(sizeof(Texture));
      t1 = (Texture *)polyray_malloc(sizeof(Texture));
      if (t0 == NULL || t1 == NULL)
         error("Failed to allocate texture data\n");
      TextureCopy(temp->t0, t0);
      TextureCopy(temp->t1, t1);
      if (head == NULL) {
         head = make_texture_map_entry(temp->p0, temp->p1, t0, t1);
         tail = head;
         }
      else {
         tail->next = make_texture_map_entry(temp->p0, temp->p1, t0, t1);
         tail = tail->next;
         }
      }
   return head;
}

texture_map_entries
texture_map_action1(char *name)
{
   int c;
   void *d;
   texture_map_entries ptr;

   Lookup_Definition(name, &c, &d);
   if (c != T_TEXTURE_MAP)
      error("Texture map '%s' not found in symbol table\n", name);
   ptr = copy_texture_map(d);
   return ptr;
}

/* Append a texture map to another texture map */
texture_map_entries
texture_map_action2(texture_map_entries head, texture_map_entries tail)
{
   texture_map_entries temp;
   if (head == NULL) {
      if (tail == NULL)
         fatal("Two NULL textures in texture map");
      else
         return tail;
      }
   for (temp=head;temp->next!=NULL;temp=temp->next) /* do nothing */ ;
   temp->next = tail;
   return head;
}

texture_fn_entries
make_texture_fn_entry(NODE_PTR fn, Texture *t0)
{
   texture_fn_entries node = polyray_malloc(sizeof(struct texture_fn_struct));
   if (node == NULL)
      error("Failed to allocate a texture map entry\n");
   node->fn = fn;
   node->t0 = t0;
   node->next = NULL;
   return node;
}

/* Append a texture function to another texture function */
texture_fn_entries
texture_fn_action2(texture_fn_entries head, texture_fn_entries tail)
{
   texture_fn_entries temp;
   if (head == NULL) {
      if (tail == NULL)
         fatal("Two NULL textures in summed texture list");
      else
         return tail;
      }
   for (temp=head;temp->next!=NULL;temp=temp->next) /* do nothing */ ;
   temp->next = tail;
   return head;
}

tstackptr
texture_list_action1(Texture *text)
{
   tstackptr node = polyray_malloc(sizeof(struct texture_stack_struct));
   if (node == NULL)
      error("Failed to allocate a node on the texture stack\n");
   node->element = text;
   node->next = NULL;
   return node;
}

tstackptr
texture_list_action2(tstackptr text_list, Texture *text)
{
  tstackptr temp;
  tstackptr node = polyray_malloc(sizeof(struct texture_stack_struct));
  if (node == NULL)
     error("Failed to allocate a node on the texture stack\n");
  node->element = text;
  node->next = NULL;
  temp = text_list;
  while (temp->next != NULL)
     temp = temp->next;
  temp->next = node;
  return text_list;
}

LIST_PTR
expression_action1(LIST_PTR elist, NODE_PTR node)
{
  LIST_PTR temp = elist;
  LIST_PTR entry = make_list_node(node);

  while (temp->next != NULL)
     temp = temp->next;
  temp->next = entry;
  return elist;
}

Transform *
transform_action1(void)
{
   Transform *new_transform = (Transform *)polyray_malloc(sizeof(Transform));

   if (new_transform == NULL) error("Failed to allocate a transform\n");
   MIdentity(new_transform->matrix);
   MIdentity(new_transform->inverse);
   return new_transform;
}

Transform *
transform_action2(char *text_name)
{
   int c;
   void *d;
   Transform *new_transform;

   Lookup_Definition(text_name, &c, &d);
   if (c != T_TRANSFORM)
      error("FATAL: Transform '%s' not found in symbol table\n", text_name);
   new_transform = (Transform *)polyray_malloc(sizeof(Transform));
   if (new_transform == NULL) error("Failed to allocate a transform\n");
   /* Copy all the default stuff */
   memcpy(new_transform, d, sizeof(Transform));
   return new_transform;
}

void
translate_transform(Transform *tx, Vec Vector)
{
   Transform trans;

   Get_Translation_Transformation(&trans, Vector);
   Compose_Transformations(tx, &trans);
}

void
rotate_transform(Transform *tx, Vec v)
{
   Transform trans;
   Vec vt;

   VecCopy(v, vt);
   VecScale(M_PI/180.0, vt);
   Get_Rotation_Transformation(&trans, vt);
   Compose_Transformations(tx, &trans);
}

void
axis_rotate_transform(Transform *tx, Vec v, Flt angle)
{
   Transform trans;

   Get_Rotate_Transform(&trans, v, angle*M_PI/180.0);
   Compose_Transformations(tx, &trans);
}

void
scale_transform(Transform *tx, Vec Vector)
{
   Transform trans;

   Get_Scaling_Transformation (&trans, Vector);
   Compose_Transformations(tx, &trans);
}

Texture *
texture_action1(void)
{
   Texture *new_texture = (Texture *)polyray_malloc(sizeof(Texture));

   if (new_texture == NULL) error("Failed to allocate a texture\n");
   new_texture->type      = T_NULL;
   new_texture->copy_flag = 0;
   new_texture->del       = NULL;
   new_texture->eval      = NULL;
   new_texture->t_trans   = NULL;
   new_texture->data      = NULL;
   return new_texture;
}

Texture *
texture_action2(char *text_name)
{
   int c;
   void *d;
   Texture *new_texture;

   Lookup_Definition(text_name, &c, &d);
   if (c != T_TEXTURE)
      error("Texture '%s' not found in symbol table\n", text_name);
   new_texture = (Texture *)polyray_malloc(sizeof(Texture));
   if (new_texture == NULL) error("Failed to allocate a texture\n");
   /* Copy all the default stuff */
   TextureCopy((Texture *)d, new_texture);
   return new_texture;
}

NODE_PTR
exper_action(char *exper_name)
{
   int c;
   void *d;
   NODE_PTR ptr;

   Lookup_Definition(exper_name, &c, &d);
   if (c != T_EXPRESSION)
      error("Expression '%s' not found in symbol table\n", exper_name);
   ptr = copy_node(d);
   return ptr;
}

static void
InitializeObject(Object *obj)
{
   obj->o_type         = T_OBJECT;
   obj->o_texture      = NULL;
   obj->o_parent       = NULL;
   obj->o_trans        = NULL;
   obj->o_copy         = 0;
   obj->o_uv_steps[0]  = 8;
   obj->o_uv_steps[1]  = 4;
   obj->o_uv_steps[2]  = 4;
   obj->o_uv_bounds[0] = -PLY_HUGE;
   obj->o_uv_bounds[1] =  PLY_HUGE;
   obj->o_uv_bounds[2] = -PLY_HUGE;
   obj->o_uv_bounds[3] =  PLY_HUGE;
   obj->o_sflag        = SHADOW_CHECK | REFLECT_CHECK | TRANSMIT_CHECK |
                         UV_CHECK | CAST_SHADOW | NORMAL_CORRECT |
                                                 ADAPTIVE_UV | SMOOTH_FLAG;
   obj->o_dither       =-1.0;
   obj->o_displace     = NULL;
   obj->o_vertices     = NULL;
   MakeVector(-PLY_HUGE/2.0, -PLY_HUGE/2.0, -PLY_HUGE/2.0,
              obj->o_bnd.lower_left);
   MakeVector(PLY_HUGE, PLY_HUGE, PLY_HUGE, obj->o_bnd.lengths);
   obj->o_csg_tree = NULL;
   obj->o_data     = NULL;
}

/* Create a holder for an object */
Object *
object_action1(void)
{
   Object *obj = (Object *)polyray_malloc(sizeof(Object));

   if (obj == NULL)
      error("Failed to allocate object memory\n");
   InitializeObject(obj);
   return obj;
}

Object *
object_action2(char *obj_name)
{
   int c;
   void *d;
   Object *obj;

   Lookup_Definition(obj_name, &c, &d);
   if (c != T_OBJECT)
      error("Object '%s' not found in symbol table\n", obj_name);
   obj = (Object *)polyray_malloc(sizeof(Object));
   if (obj == NULL)
      error("Failed to allocate object memory\n");
   InitializeObject(obj);
   Copy_Object((Object *)d, obj);
   return obj;
}

void
haze_action(Flt haze_pow, Flt haze_start, Vec haze_color)
{
   if (haze_pow < 0.0 || haze_pow > 1.0)
      error("Bad haze value\n");
   Global_Haze = haze_pow;
   Global_Haze_Start = haze_start;
   VecCopy(haze_color, Global_Haze_Color);
}

void
color_action(Special_Surface *surf, NODE_PTR color)
{
   if (surf->body_color != NULL) deallocate_node(surf->body_color);
   surf->body_color = color;
}

void
ambient_action(Special_Surface *surf, NODE_PTR color, NODE_PTR scale)
{
   if (surf->Ka_color != NULL) deallocate_node(surf->Ka_color);
   if (surf->Ka_scale != NULL) deallocate_node(surf->Ka_scale);
   surf->Ka_color = color;
   surf->Ka_scale = scale;
}

void
color_map_action(Special_Surface *surf, map_entries map, NODE_PTR def)
{
   if (surf->map != NULL) deallocate_cmap_node(surf->map);
   surf->map = map;
   if (def != NULL) {
      if (surf->body_color != NULL) deallocate_node(surf->body_color);
      surf->body_color = def;
      }
}

void
brilliance_action(Special_Surface *surf, NODE_PTR power)
{
   if (surf->Kb_power != NULL) deallocate_node(surf->Kb_power);
   surf->Kb_power = power;
}

void
diffuse_action(Special_Surface *surf, NODE_PTR color, NODE_PTR scale)
{
   if (surf->Kd_color != NULL) deallocate_node(surf->Kd_color);
   if (surf->Kd_scale != NULL) deallocate_node(surf->Kd_scale);
   surf->Kd_color = color;
   surf->Kd_scale = scale;
}

void
lookup_function_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->Lookup_fn != NULL) deallocate_node(surf->Lookup_fn);
   surf->Lookup_fn = exper;
}

void
microfacet_action(Special_Surface *surf, int type, NODE_PTR angle)
{
   if (surf->D_angle != NULL) deallocate_node(surf->D_angle);
   surf->D_type  = type;
   surf->D_angle = angle;
}

void
normal_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->normal != NULL) deallocate_node(surf->normal);
   surf->normal = exper;
}

void
position_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->position != NULL) deallocate_node(surf->position);
   surf->position = exper;
}

void
octaves_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->Octaves != NULL) deallocate_node(surf->Octaves);
   surf->Octaves = exper;
}

void
frequency_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->Frequency != NULL) deallocate_node(surf->Frequency);
   surf->Frequency = exper;
}

void
bump_scale_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->Bump_scale != NULL) deallocate_node(surf->Bump_scale);
   surf->Bump_scale = exper;
}

void
phase_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->Phase != NULL) deallocate_node(surf->Phase);
   surf->Phase = exper;
}

void
position_function_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->Position_fn != NULL) deallocate_node(surf->Position_fn);
   surf->Position_fn = exper;
}

void
position_scale_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->Pos_scale != NULL) deallocate_node(surf->Pos_scale);
   surf->Pos_scale = exper;
}

void
reflection_action(Special_Surface *surf, NODE_PTR color, NODE_PTR scale)
{
   if (surf->Kr_color != NULL) deallocate_node(surf->Kr_color);
   if (surf->Kr_scale != NULL) deallocate_node(surf->Kr_scale);
   surf->Kr_color = color;
   surf->Kr_scale = scale;
}

void
specular_action(Special_Surface *surf, NODE_PTR color, NODE_PTR scale)
{
   if (surf->Ks_color != NULL) deallocate_node(surf->Ks_color);
   if (surf->Ks_scale != NULL) deallocate_node(surf->Ks_scale);
   surf->Ks_color = color;
   surf->Ks_scale = scale;
}

void
transmission_action(Special_Surface *surf, NODE_PTR color,
                    NODE_PTR scale, NODE_PTR ior)
{
   if (surf->Kt_color != NULL) deallocate_node(surf->Kt_color);
   if (surf->Kt_scale != NULL) deallocate_node(surf->Kt_scale);
   if (surf->ior != NULL) deallocate_node(surf->ior);
   surf->Kt_color = color;
   surf->Kt_scale = scale;
   surf->ior = ior;
}

void
turbulence_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->Turbulence != NULL) deallocate_node(surf->Turbulence);
   surf->Turbulence = exper;
}

void
background_action(NODE_PTR color)
{
   Flt ftemp;
   NODE_PTR tnode;

   /* Run-time background */
   if (eval_node(NULL, color, &ftemp, BackgroundColor, &tnode) != 2) {
      if (Background != NULL) deallocate_node(Background);
      Background = color;
      }
   else {
      deallocate_node(color);
      Background = NULL;
      }
}

void
draw_action(Flt low, Flt high, int steps, NODE_PTR draw_fn, NODE_PTR color_fn)
{
   DrawNode *tlist;

   /* Run-time background */
   tlist = make_draw_node(low, high, steps, draw_fn, color_fn);
   tlist->next = Draw_Commands;
   Draw_Commands = tlist;
}

void
flush_action(int pixel_count)
{
   if (buffer_update != 1) {
      /* Only use this count if it hasn't been overridden
         from the command line */
      buffer_update = 2;
      buffer_size = pixel_count;
      }
}

map_entries
map_entry_action1(Flt start, Flt end, Vec svec, Flt strans,
                  Vec evec, Flt etrans)
{
   map_entries new_entry = polyray_malloc(sizeof(struct color_map_entry));

   if (new_entry == NULL)
      error("Failed to allocate a color map entry\n");
   new_entry->p0 = start;
   new_entry->p1 = end;
   VecCopy(svec, new_entry->v0);
   VecCopy(evec, new_entry->v1);
   new_entry->t0 = strans;
   new_entry->t1 = etrans;
   new_entry->next = NULL;
   return new_entry;
}

map_entries
map_entry_action2(map_entries head, map_entries tail)
{
   map_entries mlist = head;

   while (mlist->next != NULL)
      mlist = mlist->next;
   mlist->next = tail;
   return head;
}

void
push_tx(Transform *tx)
{
   txstackptr element;

   element = (txstackptr)polyray_malloc(sizeof(struct transform_stack_struct));
   if (element == NULL)
      error("Failed to allocate a transform");
   element->tx = tx;
   element->next = txstack;
   txstack = element;
}

void
pop_tx()
{
   txstackptr element;

   element = txstack;
   txstack = txstack->next;
   polyray_free(element);
}

void
TransformObject(Object *obj, Transform *t)
{
   if (obj == NULL || t == NULL) return;
   if (obj->o_trans == NULL) obj->o_trans = Get_Transformation();
   recompute_bbox(&obj->o_bnd, t);
   Compose_Transformations(obj->o_trans, t);
}

void
ShearObject(Object *obj, Flt xy, Flt xz, Flt yx, Flt yz, Flt zx, Flt zy)
{
   Transform trans;

   Get_Shear_Transformation(&trans, xy, xz, yx, yz, zx, zy);
   if (obj->o_trans == NULL) obj->o_trans = Get_Transformation();
   recompute_bbox(&obj->o_bnd, &trans);
   Compose_Transformations(obj->o_trans, &trans);
}

void
TranslateObject(Object *obj, Vec v)
{
   Transform trans;

   Get_Translation_Transformation(&trans, v);
   if (obj->o_trans == NULL) obj->o_trans = Get_Transformation();
   VecAdd(obj->o_bnd.lower_left, v, obj->o_bnd.lower_left);
   Compose_Transformations(obj->o_trans, &trans);
}

void
RotateObject(Object *obj, Vec v)
{
   Transform trans;
   Vec vt;

   VecCopy(v, vt);
   VecScale(M_PI/180.0, vt);
   Get_Rotation_Transformation(&trans, vt);
   if (obj->o_trans == NULL) obj->o_trans = Get_Transformation();
   recompute_bbox(&obj->o_bnd, &trans);
   Compose_Transformations(obj->o_trans, &trans);
}

void
RotateAxisObject(Object *obj, Vec v, Flt ang)
{
   Transform trans;

   Get_Rotate_Transform(&trans, v, M_PI * ang / 180.0);
   if (obj->o_trans == NULL) obj->o_trans = Get_Transformation();
   recompute_bbox(&obj->o_bnd, &trans);
   Compose_Transformations(obj->o_trans, &trans);
}

void
ScaleObject(Object *obj, Vec v)
{
   Transform trans;

   Get_Scaling_Transformation (&trans, v);
   if (obj->o_trans == NULL) obj->o_trans = Get_Transformation();
   recompute_bbox(&obj->o_bnd, &trans);
   Compose_Transformations(obj->o_trans, &trans);
}

void
root_solver_action(Object *obj, int solver)
{
   if (obj->o_type == T_TORUS)
      Set_Torus_Solver(obj, solver);
   else if (obj->o_type == T_POLYNOMIAL)
      Set_Polynomial_Solver(obj, solver);
   else if (obj->o_type == T_BLOB)
      Set_Blob_Solver(obj, solver);
   else if (obj->o_type == T_REVOLVE)
      Set_Lathe_Solver(obj, solver);
}

void
spherical_component_action(Vec pos, Flt strength, Flt radius)
{
   blob_component = polyray_malloc(sizeof(struct blob_list_struct));
   if (blob_component == NULL)
      error("Failed to allocate a blob component\n");
   blob_component->elem.type = T_SPHERICAL_BLOB;
   blob_component->elem.coeffs[2] = strength;
   blob_component->elem.radius2   = radius;
   VecCopy(pos, blob_component->elem.pos);
   blob_component->next = blob_components;
   blob_components = blob_component;
   npoints++;
}

void
cylindrical_component_action(Vec pos0, Vec pos1, Flt strength, Flt radius)
{
   blob_component = polyray_malloc(sizeof(struct blob_list_struct));
   if (blob_component == NULL)
      error("Failed to allocate a blob component\n");
   blob_component->elem.type = T_CYLINDRICAL_BLOB;
   blob_component->elem.coeffs[2] = strength;
   blob_component->elem.radius2   = radius;
   VecCopy(pos0, blob_component->elem.pos);
   VecCopy(pos1, blob_component->elem.dir);
   blob_component->next = blob_components;
   blob_components = blob_component;
   npoints++;
}

void
planar_component_action(Vec N, Flt d, Flt strength, Flt radius)
{
   blob_component = polyray_malloc(sizeof(struct blob_list_struct));
   if (blob_component == NULL)
      error("Failed to allocate a blob component\n");
   blob_component->elem.type = T_PLANAR_BLOB;
   blob_component->elem.coeffs[2] = strength;
   blob_component->elem.radius2   = radius;
   VecCopy(N, blob_component->elem.dir);
   blob_component->elem.len = d;
   blob_component->next = blob_components;
   blob_components = blob_component;
   npoints++;
}

void
toroidal_component_action(Vec C, Vec N, Flt major,
                          Flt strength, Flt radius)
{
   blob_component = polyray_malloc(sizeof(struct blob_list_struct));
   if (blob_component == NULL)
      error("Failed to allocate a blob component\n");
   blob_component->elem.type = T_TOROIDAL_BLOB;
   blob_component->elem.coeffs[2] = strength;
   blob_component->elem.len       = major;
   blob_component->elem.radius2   = radius;
   VecCopy(C, blob_component->elem.pos);
   VecCopy(N, blob_component->elem.dir);
   blob_component->next = blob_components;
   blob_components = blob_component;
   npoints++;
}

/* Determine the highest order term used in the polynomial. */
static int
max_power_used(LIST_PTR list)
{
   NODE_PTR term;
   int t, result = 0;
   while (list != NULL) {
      term = list->element;
      if (term->exper_type == TERM) {
         t = (int)term->exper_data.coeff.x_power +
             (int)term->exper_data.coeff.y_power +
             (int)term->exper_data.coeff.z_power;
         if (t > result) result = t;
         }
      list = list->next;
      }
   return result;
}

/* Given the powers of x, y, and z, return the index into the polynomial */
static int
roll(int order, int x, int y, int z)
{
   int xstart, ystart, zstart;
   xstart = binomial(order-x+2,order-x-1);
   order = order - x;
   ystart = binomial(order-y+1,order-y-1);
   order = order - y;
   zstart = binomial(order-z,order-z-1);
   return xstart+ystart+zstart;
}

/* Translate a list of terms into an array of coefficients. */
static Flt *
generate_coeffs(LIST_PTR list, int *oorder)
{
   NODE_PTR term;
   int order, i, term_count;
   Flt *coeffs;

   order = max_power_used(list);
   if (order > MAX_POLYNOMIAL_ORDER)
      /* Would need more than 64K to store all of the coefficients. */
      error("Polynomial is of order %d, this is too large\n", order);
   term_count = (order + 1) * (order + 2) * (order + 3) / 6;
   coeffs = polyray_malloc(sizeof(Flt) * term_count);
   if (coeffs == NULL)
      error("Failed to allocate polynomial coeffs\n");
   for (i=0;i<term_count;i++)
      coeffs[i] = 0.0;
   while (list != NULL) {
      term = list->element;
      if (term->exper_type == TERM) {
         i = roll(order, (int)term->exper_data.coeff.x_power,
                  (int)term->exper_data.coeff.y_power,
                  (int)term->exper_data.coeff.z_power);
         coeffs[i] += term->exper_data.coeff.coeff;
         }
      else {
         message(" { Omitting term: ");
         show_node(term);
         message("} \n");
         }
      list = list->next;
      }
   *oorder = order;
   return coeffs;
}

void
polynomial_action1(NODE_PTR data, int solver)
{
   Flt *Coeffs;
   NODE_PTR parse_tree;
   LIST_PTR exper_list;
   int CurrentOrder;

   parse_tree = data;
   parse_tree = simplify(parse_tree, 0);
   exper_list = collect_additive_terms(parse_tree);
   Coeffs = generate_coeffs(exper_list, &CurrentOrder);
   deallocate_list(exper_list);
   (void)MakePolynomial(Object_Stack->element, CurrentOrder, Coeffs, solver);
}

csgnodeptr
make_csg_node(int type, void *left, void *right)
{
   csgnodeptr result = polyray_malloc(sizeof(struct csgnode));
   if (result == NULL)
      error("Failed to allocate a CSG node\n");
   result->type = type;
   result->left = left;
   result->right = right;
   return result;
}

void
csg_action1(csgnodeptr data)
{
   (void)MakeCSG(Object_Stack->element, data);
}

VList *
add_bezier_point(VList *points, Vec point)
{
   VList *plist = NULL;

   if (points == NULL) {
      plist = polyray_malloc(sizeof(VList));
      if (plist == NULL)
         error("Failed to allocate Bezier structure\n");
      plist->points = polyray_malloc(16 * sizeof(Vec));
      if (plist->points == NULL)
         error("Failed to allocate Bezier point list\n");
      plist->count = 1;
      VecCopy(point, plist->points[0]);
      }
   else if (points->count == 16)
      error("Too many points in Bezier patch, must only be 16");
   else {
      plist = points;
      VecCopy(point, plist->points[plist->count]);
      plist->count++;
      }
   return plist;
}

char *
translate_string(char *defname)
{
   int c;
   void *d;
   char *newstr, *oldstr;

   Lookup_Definition(defname, &c, &d);
   if (c != T_STRING)
      error("String '%s' not found in symbol table\n", defname);
   oldstr = (char *)d;
   newstr = (char *)polyray_malloc((strlen(d) + 1) * sizeof(char));
   if (newstr == NULL)
      error("Failed to allocate string space");
   strcpy(newstr, oldstr);
   return newstr;
}

char *
build_string(LIST_PTR args)
{
   static char temp_str[256];
   char *tstr;
   int i, j, k, argc;
   LIST_PTR targs;

   /* Figure out how many arguments there are */
   for (argc=0,targs=args;targs!=NULL;argc++,targs=targs->next) ;

   temp_str[0] = '\0';
   for (i=0,j=1,k=0;i<argc && j==1;i++) {
      /* Evaluate each argument, appending the results as
         we go */
      j = create_string(args->element, &tstr);
      k += strlen(tstr);
      if (k > 255)
         error("String too long\n");
      else if (j == 1)
         strcat(temp_str, tstr);
      else
         error("Non-string used in system call\n");
      deallocate_node(args->element);
      args = args->next;
      }

   /* Clean up the memory used to hold the arguments */
   while (args != NULL) {
      targs = args;
      args = args->next;
      polyray_free(targs);
      }

   return &temp_str[0];
}

/* Create a string from an expression - the contents of "name" are disposable */
int
create_string(NODE_PTR exper, char **name)
{
   int i;
   Flt ftmp;
   Vec vtmp;
   char *stmp, buffer[128];
   NODE_PTR ntmp;

   if (exper->exper_type == STRING)
      /* Simple copy */
      stmp = exper->exper_data.str;
   else {
      i = eval_node(NULL, exper, &ftmp, vtmp, &ntmp);
      if (i == 1) {
         /* Create a string from a number */
         sprintf(buffer, "%g", ftmp);
         stmp = &buffer[0];
         }
      else if (i == 2) {
         /* Create a string from a vector */
         sprintf(buffer, "{%g, %g, %g}", vtmp[0], vtmp[1], vtmp[2]);
         stmp = &buffer[0];
         }
      else
         return 0;
      }

   /* If we get to here, we have a valid string sitting in "buffer" */
   *name = polyray_malloc((strlen(stmp) + 1) * sizeof(char));
   strcpy(*name, stmp);
   return 1;
}

void
evaluate_system_call(LIST_PTR args)
{
   system(build_string(args));
}

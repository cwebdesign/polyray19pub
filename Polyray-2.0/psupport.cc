/** @file psupport.cc
 *  @brief Parser support - action routines called by the Bison grammar.
 *
 *  Every public function in this file is invoked directly from grammar
 *  actions in polyray.y.  They bridge the parser and the scene-graph
 *  builder, allocating and wiring up scene objects, textures, transforms,
 *  surfaces, and blob components.
 *
 *  Polyray - MIT Licensed Revival
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

#include <iomanip>
#include <iostream> 
#include <sstream> 
#include <string>
#include <bitset>
#include <gsl/gsl>

#include "defs3.h"
#include "io_ply.h"
#include "parse.h"
#include "memory.h"
#include "builder.h"
#include "psupport.h"
#include "polyray.tab.h"
#include "runtime_state.h"
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
#include "util.h"
#include "unixcompat.h"//for polyray_pause
#include "factory.h" //factory methods




/* #define DEBUG_FN_CALLS */
/* Parser support variables */
fVec *pl, *plist;
ostackptr Object_Stack = nullptr;
tstackptr Texture_Stack = nullptr;
blobstackptr blob_components = nullptr;
blobstackptr blob_component;
//int condition_flags[MAX_CONDITION_DEPTH];
std::bitset<MAX_CONDITION_DEPTH> condition_flags;
int condition_depth = 0;
Transform *Current_Transform = nullptr;
Special_Surface *CurrentSurface;
int npoints = 0;
int ObjectDepth = 0;
txstackptr txstack = nullptr;
UVVert tri_vertex[3];

/** @brief Set all fields of a Surface to their default values.
 *
 *  Defaults: white ambient (0.1), white diffuse (0.9), no specular/reflect/transmit,
 *  Phong highlighting with coefficient 2, IOR = 1 (air).
 *  @param NewSurf  Surface to initialise.
 *  @return No return value.
 */
void InitializeSurface(Surface& NewSurf)
{
    MakeVector(1.0, 1.0, 1.0, NewSurf.Ka_color);/* ambient is White */
    NewSurf.Ka_scale = 0.1;                     /* ambient scale small */
    NewSurf.Kb_power = 1.0;                     /* No diffuse modification */
    MakeVector(1.0, 1.0, 1.0, NewSurf.Kd_color);/* diffuse is White */
    NewSurf.Kd_scale = 0.9;                     /* diffuse scale high */
    MakeVector(1.0, 1.0, 1.0, NewSurf.Ks_color);/* Specular is White */
    NewSurf.Ks_scale = 0.0;                     /* No specular (matte) */
    MakeVector(1.0, 1.0, 1.0, NewSurf.Kr_color);/* Reflect all colors */
    NewSurf.Kr_scale = 0.0;                     /* No reflectivity */
    MakeVector(1.0, 1.0, 1.0, NewSurf.Kt_color);/* Reflect all colors */
    NewSurf.Kt_scale = 0.0;                     /* No transmission */
    NewSurf.D = nullptr;                           /* Use Phong highlighting */
    NewSurf.D_coeff = 2.0;                      /* Precompute Phong coeff */
    NewSurf.ior = 1.0;                          /* Index of refraction = air */
}

/** @brief Set all expression-node pointers in a Special_Surface to nullptr.
 *
 *  Called before populating a surface from grammar actions so that every
 *  field starts in a known, unset state.
 *  @param NewSurf  Special_Surface to zero-initialise.
 *  @return No return value.
 */
void InitializeSpecialSurface(Special_Surface& NewSurf)
{
   NewSurf.Position_fn = nullptr;
   NewSurf.Pos_scale   = nullptr;
   NewSurf.Lookup_fn   = nullptr;
   NewSurf.Turbulence  = nullptr;
   NewSurf.Octaves     = nullptr;
   NewSurf.Frequency   = nullptr;
   NewSurf.Phase       = nullptr;
   NewSurf.Bump_scale  = nullptr;
   NewSurf.body_color  = nullptr;
   NewSurf.normal      = nullptr;
   NewSurf.position    = nullptr;
   NewSurf.Ka_color    = nullptr;
   NewSurf.Ka_scale    = nullptr;
   NewSurf.Kb_power    = nullptr;
   NewSurf.Kd_color    = nullptr;
   NewSurf.Kd_scale    = nullptr;
   NewSurf.Ks_color    = nullptr;
   NewSurf.Ks_scale    = nullptr;
   NewSurf.Kr_color    = nullptr;
   NewSurf.Kr_scale    = nullptr;
   NewSurf.Kt_color    = nullptr;
   NewSurf.Kt_scale    = nullptr;
   NewSurf.ior         = nullptr;
   NewSurf.D_type      = PHONG;
   NewSurf.D_angle     = nullptr;
   NewSurf.map         = nullptr;
}

/** @brief Build the output file path for the current animation frame.
 *
 *  Appends a zero-padded frame number and the appropriate extension
 *  (.tga, .ppm, or .bin) to @p outfilebase.  Names longer than 120
 *  characters are truncated with a warning.
 *  @param outfilebase  Base filename (no extension, no frame number).
 *  @return             Full output filename for the current frame.
 */
std::string build_outfile_name(std::string outfilebase)
{
   #ifdef DEBUG_FN_CALLS
   std::cout<<"psupport::build_outfile_name "<<outfilebase<<"\n";
   openpolyray::polyray_pause();
   #endif
   std::stringstream buffer;//  char tmpstr[128];
   std::string out{};
   if (outfilebase.empty()) {
      swarning("Zero length outfile name, resetting to 'out'\n");
      outfilebase="out";
      }
   else if (outfilebase.length() > 120) {
      outfilebase=outfilebase.substr(0,120);
      swarning("Output file name too long, truncating to: '%s'\n",
              outfilebase.c_str());
      }
   out=outfilebase;
   //sprintf(tmpstr, "%03d", current_frame);
   buffer <<  std::setprecision(3)<< runtimeState::animator.current_frame<<std::endl;
   std::string bs{buffer.str()};
   while (bs.length()<4) bs="0"+bs;
   #ifdef DEBUG_FN_CALLS
   std::cout<<"bs = "<<bs<<"\n"<<std::endl;
   std::cout << "debug - psupport - penultimate char of bs=" << bs[bs.length() - 2] << "\n";
   std::cout << "debug - psupport - last char of bs=" << bs[bs.length() - 1] << "\n";
   //exit(2);//just to check
   #endif
   if (bs[bs.length()-1]=='\n') bs.pop_back();
   out+=bs;
  
/* CM added 10/mar/2000 */
   if (RuntimeState::outformat == OUT_FORMAT::OUT_TARGA) {
     out+=".tga";
   }
   else
   if (RuntimeState::outformat == OUT_FORMAT::OUT_RAWPPM) {
     out+=".ppm";
   }
   else
     out+=".bin";
   return out;
}

/** @brief Return true if all conditional flags up to the current nesting depth are set.
 *
 *  Used by the parser to skip grammar actions inside false @c if/@c else branches.
 *  @return  true when every condition_flags[0..condition_depth] is set, false otherwise.
 */
bool check_condition()
{
   for (int i=0;i<=condition_depth;i++)
      if (!condition_flags[i]) {
         return false;
         }
   return true;
}

//bool check_conditionarr(std::span<bool> )


/** @brief Parser action: allocate and zero-initialise a new anonymous Special_Surface.
 *
 *  Sets the global CurrentSurface pointer ready for subsequent surface
 *  attribute actions.
 *  @return No return value.
 */
void surface_action1()
{
   CurrentSurface = FactorySpecial_Surface();
   
   InitializeSpecialSurface(*CurrentSurface);
}

/** @brief Parser action: begin a surface definition by copying a named surface.
 *
 *  Looks up @p surf_name in the symbol table, copies it into a freshly
 *  allocated Special_Surface, and sets CurrentSurface.
 *  @param surf_name  Name of the previously defined surface to inherit from.
 *  @return No return value.
 */
void surface_action2(char *surf_name)
{
   int c;
   void *d;

   Lookup_Definition(surf_name, &c, &d);
   if (c != std::to_underlying(ShapeType::Surface))
      serror("Surface '%s' not found in symbol table\n", surf_name);
   CurrentSurface = FactorySpecial_Surface();
   copy_special0((Special_Surface *)d, CurrentSurface);
}

/** @brief Push a texture onto the global Texture_Stack.
 *  @param text  Texture to push; must be heap-allocated.
 *  @return No return value.
 */
void push_texture(Texture *text)
{
   tstackptr node = Factorytexture_stack();

   if (node == nullptr)
      serror("Failed to allocate a node on the texture stack\n");
   node->element = text;
   node->next = Texture_Stack;
   Texture_Stack = node;
}

/** @brief Pop and return the top texture from the global Texture_Stack.
 *
 *  The stack node itself is deleted; the returned Texture* is still valid.
 *  @return  Pointer to the texture that was on top of the stack.
 */
Texture *pop_texture(void)
{
   tstackptr node = Texture_Stack;
   Texture *last_texture = Texture_Stack->element;

   Texture_Stack = Texture_Stack->next;
   delete node;
   return last_texture;
}

/** @brief Allocate and initialise a single texture map entry.
 *  @param p0  Start parameter value for the map interval.
 *  @param p1  End parameter value for the map interval.
 *  @param t0  Texture at the start of the interval (ownership transferred).
 *  @param t1  Texture at the end of the interval (ownership transferred).
 *  @return    Pointer to the new entry; next is set to nullptr.
 */
texture_map_entries make_texture_map_entry(Flt p0, Flt p1, Texture *t0, Texture *t1)
{
   texture_map_entries node = Factorytexture_map();
   if (node == nullptr)
      serror("Failed to allocate a texture map entry\n");
   node->p0 = p0;
   node->p1 = p1;
   node->t0 = t0;
   node->t1 = t1;
   node->next = nullptr;
   return node;
}

/** @brief Type-erased wrapper around copy_texture_map().
 *
 *  Exists so that the symbol-table void* can be passed without an explicit cast
 *  at each call site.
 *  @param vmap  Pointer to a texture_map_entries list (stored as void*).
 *  @return      Deep copy of the texture map list.
 */
texture_map_entries copy_texture_map_void(void *vmap)
{
  texture_map_entries map = static_cast<texture_map_entries>(vmap);
  return copy_texture_map(map);
}

/** @brief Deep-copy a linked list of texture map entries.
 *
 *  Each entry's textures are copied via TextureCopy(); the copies are
 *  owned by the returned list.
 *  @param map  Source list to copy; may be nullptr (returns nullptr).
 *  @return     Head of a newly allocated copy of @p map.
 */
texture_map_entries copy_texture_map(texture_map_entries map)
{
   Texture *t0, *t1;
   texture_map_entries head = nullptr, tail = nullptr, temp;

   for (temp=map;temp!=nullptr;temp=temp->next) {
      t0 = FactoryTexture();
      t1 = FactoryTexture();
      if (t0 == nullptr || t1 == nullptr)
         serror("Failed to allocate texture data\n");
      TextureCopy(temp->t0, t0);
      TextureCopy(temp->t1, t1);
      if (head == nullptr) {
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

/** @brief Parser action: look up and deep-copy a named texture map from the symbol table.
 *  @param name  Symbol-table key of the texture map.
 *  @return      Deep copy of the resolved texture map list.
 */
texture_map_entries texture_map_action1(char *name)
{
   int c;
   void *d;
   texture_map_entries ptr;

   Lookup_Definition(name, &c, &d);
   if (c != std::to_underlying(ShapeType::Texture_Map))
      serror("Texture map '%s' not found in symbol table\n", name);
   ptr = (texture_map_entries)copy_texture_map_void(d);
   return ptr;
}

/** @brief Parser action: append @p tail to the end of the @p head texture map list.
 *  @param head  First texture map list; may be nullptr.
 *  @param tail  List to append; may be nullptr.
 *  @return      Head of the combined list.
 */
texture_map_entries
texture_map_action2(texture_map_entries head, texture_map_entries tail)
{
   texture_map_entries temp;
   if (head == nullptr) {
      if (tail == nullptr)
         sfatal("Two nullptr textures in texture map");
      else
         return tail;
      }
   for (temp=head;temp->next!=nullptr;temp=temp->next) /* do nothing */ ;
   temp->next = tail;
   return head;
}

/** @brief Allocate and initialise a single texture function entry.
 *  @param fn  Expression node that selects among textures; ownership transferred.
 *  @param t0  Texture to apply when @p fn evaluates to this entry; ownership transferred.
 *  @return    Pointer to the new entry; next is set to nullptr.
 */
texture_fn_entries
make_texture_fn_entry(NODE_PTR fn, Texture *t0)
{
   texture_fn_entries node = Factorytexture_fn();
   if (node == nullptr)
      serror("Failed to allocate a texture map entry\n");
   node->fn = fn;
   node->t0 = t0;
   node->next = nullptr;
   return node;
}

/** @brief Parser action: append @p tail to the end of the @p head texture function list.
 *  @param head  First texture function list; may be nullptr.
 *  @param tail  List to append; may be nullptr.
 *  @return      Head of the combined list.
 */
texture_fn_entries
texture_fn_action2(texture_fn_entries head, texture_fn_entries tail)
{
   texture_fn_entries temp;
   if (head == nullptr) {
      if (tail == nullptr)
         sfatal("Two nullptr textures in summed texture list");
      else
         return tail;
      }
   for (temp=head;temp->next!=nullptr;temp=temp->next) /* do nothing */ ;
   temp->next = tail;
   return head;
}

/** @brief Parser action: begin a texture list with a single entry.
 *  @param text  Texture to place at the head; ownership transferred.
 *  @return      New single-element texture stack list.
 */
tstackptr
texture_list_action1(Texture *text)
{
   tstackptr node = Factorytexture_stack();
   if (node == nullptr)
      serror("Failed to allocate a node on the texture stack\n");
   node->element = text;
   node->next = nullptr;
   return node;
}

/** @brief Parser action: append a texture to an existing texture list.
 *  @param text_list  Existing list to append to.
 *  @param text       Texture to add at the tail; ownership transferred.
 *  @return           @p text_list (head unchanged).
 */
tstackptr
texture_list_action2(tstackptr text_list, Texture *text)
{
  tstackptr temp;
  tstackptr node = Factorytexture_stack();
  if (node == nullptr)
     serror("Failed to allocate a node on the texture stack\n");
  node->element = text;
  node->next = nullptr;
  temp = text_list;
  while (temp->next != nullptr)
     temp = temp->next;
  temp->next = node;
  return text_list;
}

/** @brief Parser action: append an expression node to an existing expression list.
 *  @param elist  Existing list; the new node is added at the tail.
 *  @param node   Expression node to append; ownership transferred.
 *  @return       @p elist (head unchanged).
 */
LIST_PTR
expression_action1(LIST_PTR elist, NODE_PTR node)
{
  LIST_PTR temp = elist;
  LIST_PTR entry = make_list_node(node);

  while (temp->next != nullptr)
     temp = temp->next;
  temp->next = entry;
  return elist;
}

/** @brief Parser action: allocate a new identity Transform.
 *  @return  Newly allocated Transform with both matrix and inverse set to identity.
 */
Transform *transform_action1(void)
{
   std::unique_ptr<Transform> new_transform = FactoryTransform();

   MIdentity(new_transform->matrix);
   MIdentity(new_transform->inverse);
   return new_transform.release();
}

/** @brief Parser action: copy a named Transform from the symbol table.
 *  @param text_name  Symbol-table key of the transform.
 *  @return           Newly allocated copy of the resolved Transform.
 */
Transform *transform_action2(char *text_name)
{
   int c;
   void *d;
   Transform *new_transform;

   Lookup_Definition(text_name, &c, &d);
   if (c != std::to_underlying(ShapeType::Transform))
      serror("FATAL: Transform '%s' not found in symbol table\n", text_name);
   new_transform = FactoryTransform().release();
   // Copy all the default stuff
   *new_transform = *static_cast<Transform*>(d);
   return new_transform;
}

/** @brief Compose a translation onto an existing transform.
 *  @param tx      Transform to modify in place.
 *  @param Vector  Translation vector.
 *  @return No return value.
 */
void translate_transform(Transform *tx, Vec Vector)
{
   Transform trans;
   NuVec nVector{ Vector[0], Vector[1], Vector[2] };

   Get_Translation_Transformation_CPP(trans, nVector);
   Compose_Transformations(*tx, trans);
}

/** @brief Compose an Euler-angle rotation (degrees) onto an existing transform.
 *  @param tx  Transform to modify in place.
 *  @param v   Rotation angles in degrees around X, Y, and Z axes.
 *  @return No return value.
 */
void rotate_transform(Transform *tx, Vec v)
{
   Transform trans;
   NuVec vt;

   VecCopy(v, vt);
   VecScale(PYM_PI/180.0, vt);
   Get_Rotation_Transformation_CPP(trans, vt);
   Compose_Transformations(*tx, trans);
}

/** @brief Compose an axis-angle rotation (degrees) onto an existing transform.
 *  @param tx     Transform to modify in place.
 *  @param v      Rotation axis (need not be unit length).
 *  @param angle  Rotation angle in degrees.
 *  @return No return value.
 */
void axis_rotate_transform(Transform *tx, Vec v, Flt angle)
{
   Transform trans;
   NuVec nv{ v[0], v[1], v[2] };
   Get_Rotate_Transform_CPP(trans, nv, angle*PYM_PI/180.0);
   Compose_Transformations(*tx, trans);
}

/** @brief Compose a non-uniform scale onto an existing transform.
 *  @param tx      Transform to modify in place.
 *  @param Vector  Scale factors along X, Y, and Z.
 *  @return No return value.
 */
void
scale_transform(Transform *tx, Vec Vector)
{
   Transform trans;
   NuVec nVector{Vector[0],Vector[1],Vector[2]};
   Get_Scaling_Transformation_CPP (trans, nVector);
   Compose_Transformations(*tx, trans);
}

/** @brief Parser action: allocate a blank, null-typed Texture.
 *  @return  Newly allocated Texture with all fields zeroed/nullptr.
 */
Texture *texture_action1(void)
{
   Texture *new_texture = FactoryTexture();

   new_texture->type      = ShapeType::Null;
   new_texture->copy_flag = 0;
   new_texture->del       = nullptr;
   new_texture->eval      = nullptr;
   new_texture->t_trans   = nullptr;
   new_texture->data      = nullptr;
   return new_texture;
}

/** @brief Parser action: deep-copy a named Texture from the symbol table.
 *  @param text_name  Symbol-table key of the texture.
 *  @return           Newly allocated copy of the resolved Texture.
 */
Texture *texture_action2(char *text_name)
{
   int c;
   void *d;
   Texture *new_texture;

   Lookup_Definition(text_name, &c, &d);
   if (c != std::to_underlying(ShapeType::Texture))
      serror("Texture '%s' not found in symbol table\n", text_name);
   new_texture = FactoryTexture();
   if (new_texture == nullptr) serror("Failed to allocate a texture\n");
   /* Copy all the default stuff */
   TextureCopy((Texture *)d, new_texture);
   return new_texture;
}

/** @brief Parser action: deep-copy a named expression node from the symbol table.
 *  @param exper_name  Symbol-table key of the expression.
 *  @return            Copy of the resolved expression tree.
 */
NODE_PTR
exper_action(char *exper_name)
{
   int c;
   void *d;
   NODE_PTR ptr;

   Lookup_Definition(exper_name, &c, &d);
   if (c != std::to_underlying(ShapeType::Expression))
      serror("Expression '%s' not found in symbol table\n", exper_name);
   ptr = copy_node_void(d);
   return ptr;
}

/** @brief Set all fields of an Object to their default values.
 *
 *  Sets type to T_OBJECT, clears all pointers, enables the standard shadow/
 *  reflect/transmit/UV flags, and sets a near-infinite bounding box.
 *  @param obj  Object to initialise.
 *  @return No return value.
 */
#ifndef TESTING
namespace {
#endif
void
InitializeObject(Object& obj)
{
   obj.o_type         = ShapeType::Object;
   obj.o_texture      = nullptr;
   obj.o_parent       = nullptr;
   obj.o_trans        = nullptr;
   obj.o_copy         = 0;
   obj.o_uv_steps[0]  = 8;
   obj.o_uv_steps[1]  = 4;
   obj.o_uv_steps[2]  = 4;
   obj.o_uv_bounds[0] = -PLY_HUGE;
   obj.o_uv_bounds[1] =  PLY_HUGE;
   obj.o_uv_bounds[2] = -PLY_HUGE;
   obj.o_uv_bounds[3] =  PLY_HUGE;
   obj.o_sflag        = SHADOW_CHECK | REFLECT_CHECK | TRANSMIT_CHECK |
                         UV_CHECK | CAST_SHADOW | NORMAL_CORRECT |
                                                 ADAPTIVE_UV | SMOOTH_FLAG;
   obj.o_dither       =-1.0;
   obj.o_displace     = nullptr; //todo:fix heap buffer overflow here
   obj.o_vertices     = nullptr;
   MakeVector(-PLY_HUGE/2.0, -PLY_HUGE/2.0, -PLY_HUGE/2.0,
              obj.o_bnd.lower_left);
   MakeVector(PLY_HUGE, PLY_HUGE, PLY_HUGE, obj.o_bnd.lengths);
   obj.o_csg_tree = nullptr;
   obj.o_data     = nullptr;
}

#ifndef TESTING
} // anonymous namespace
#endif

/** @brief Parser action: allocate and default-initialise a new Object.
 *  @return  Newly allocated Object ready for shape data to be attached.
 */
Object *
object_action1(void)
{
   Object *obj = FactoryObject();

   if (obj == nullptr)
      serror("Failed to allocate object memory\n");
   InitializeObject(*obj);
   return obj;
}

/** @brief Parser action: allocate an Object and copy a named Object from the symbol table.
 *  @param obj_name  Symbol-table key of the source object.
 *  @return          Newly allocated Object that is a copy of the named object.
 */
Object *
object_action2(char *obj_name)
{
   int c;
   void *d;

   Lookup_Definition(obj_name, &c, &d);
   if (c != std::to_underlying(ShapeType::Object))
      serror("Object '%s' not found in symbol table\n", obj_name);
   Object *obj = FactoryObject();
   if (obj == nullptr)
      serror("Failed to allocate object memory\n");
   InitializeObject(*obj);
   Copy_Object((Object *)d, *obj);
   return obj;
}

/** @brief Parser action: set global atmospheric haze parameters.
 *  @param haze_pow    Haze exponent in [0, 1]; values outside this range are an error.
 *  @param haze_start  Distance at which haze begins.
 *  @param haze_color  RGB colour of the haze medium.
 *  @return No return value.
 */
void
haze_action(Flt haze_pow, Flt haze_start, Vec haze_color)
{
   if (haze_pow < 0.0 || haze_pow > 1.0)
      serror("Bad haze value\n");
   RuntimeState::Global_Haze = haze_pow;
   RuntimeState::Global_Haze_Start = haze_start;
   VecCopy(haze_color, RuntimeState::Global_Haze_Color);
}

/** @brief Parser action: set the body colour expression on a surface.
 *  @param surf   Surface to modify; existing node is freed first.
 *  @param color  Expression node for the body colour; ownership transferred.
 *  @return No return value.
 */
void
color_action(Special_Surface *surf, NODE_PTR color)
{
   if (surf->body_color != nullptr) deallocate_node(surf->body_color);
   surf->body_color = color;
}

/** @brief Parser action: set the ambient colour and scale expressions on a surface.
 *  @param surf   Surface to modify; existing nodes are freed first.
 *  @param color  Expression node for Ka_color; ownership transferred.
 *  @param scale  Expression node for Ka_scale; ownership transferred.
 *  @return No return value.
 */
void
ambient_action(Special_Surface *surf, NODE_PTR color, NODE_PTR scale)
{
   if (surf->Ka_color != nullptr) deallocate_node(surf->Ka_color);
   if (surf->Ka_scale != nullptr) deallocate_node(surf->Ka_scale);
   surf->Ka_color = color;
   surf->Ka_scale = scale;
}

/** @brief Parser action: attach a colour map and optional default colour to a surface.
 *  @param surf  Surface to modify; existing map and body_color nodes are freed first.
 *  @param map   Colour map list; ownership transferred.
 *  @param def   Default body colour node when outside the map range; may be nullptr.
 *  @return No return value.
 */
void
color_map_action(Special_Surface *surf, map_entries map, NODE_PTR def)
{
   if (surf->map != nullptr) deallocate_cmap_node(surf->map);
   surf->map = map;
   if (def != nullptr) {
      if (surf->body_color != nullptr) deallocate_node(surf->body_color);
      surf->body_color = def;
      }
}

/** @brief Parser action: set the diffuse brilliance (Kb) exponent on a surface.
 *  @param surf   Surface to modify; existing node is freed first.
 *  @param power  Expression node for Kb_power; ownership transferred.
 *  @return No return value.
 */
void
brilliance_action(Special_Surface *surf, NODE_PTR power)
{
   if (surf->Kb_power != nullptr) deallocate_node(surf->Kb_power);
   surf->Kb_power = power;
}

/** @brief Parser action: set the diffuse colour and scale expressions on a surface.
 *  @param surf   Surface to modify; existing nodes are freed first.
 *  @param color  Expression node for Kd_color; ownership transferred.
 *  @param scale  Expression node for Kd_scale; ownership transferred.
 *  @return No return value.
 */
void
diffuse_action(Special_Surface *surf, NODE_PTR color, NODE_PTR scale)
{
   if (surf->Kd_color != nullptr) deallocate_node(surf->Kd_color);
   if (surf->Kd_scale != nullptr) deallocate_node(surf->Kd_scale);
   surf->Kd_color = color;
   surf->Kd_scale = scale;
}

/** @brief Parser action: set the texture lookup function on a surface.
 *  @param surf   Surface to modify; existing node is freed first.
 *  @param exper  Expression node for Lookup_fn; ownership transferred.
 *  @return No return value.
 */
void
lookup_function_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->Lookup_fn != nullptr) deallocate_node(surf->Lookup_fn);
   surf->Lookup_fn = exper;
}

/** @brief Parser action: set the microfacet distribution type and angle on a surface.
 *  @param surf   Surface to modify; existing angle node is freed first.
 *  @param type   Distribution type constant (e.g. PHONG, COOK, WARD).
 *  @param angle  Expression node for D_angle; ownership transferred.
 *  @return No return value.
 */
void
microfacet_action(Special_Surface *surf, int type, NODE_PTR angle)
{
   if (surf->D_angle != nullptr) deallocate_node(surf->D_angle);
   surf->D_type  = type;
   surf->D_angle = angle;
}

/** @brief Parser action: set the normal-perturbation expression on a surface.
 *  @param surf   Surface to modify; existing node is freed first.
 *  @param exper  Expression node for the normal perturbation; ownership transferred.
 *  @return No return value.
 */
void
normal_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->normal != nullptr) deallocate_node(surf->normal);
   surf->normal = exper;
}

/** @brief Parser action: set the position expression on a surface.
 *  @param surf   Surface to modify; existing node is freed first.
 *  @param exper  Expression node for the position field; ownership transferred.
 *  @return No return value.
 */
void
position_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->position != nullptr) deallocate_node(surf->position);
   surf->position = exper;
}

/** @brief Parser action: set the noise octave count expression on a surface.
 *  @param surf   Surface to modify; existing node is freed first.
 *  @param exper  Expression node for Octaves; ownership transferred.
 *  @return No return value.
 */
void
octaves_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->Octaves != nullptr) deallocate_node(surf->Octaves);
   surf->Octaves = exper;
}

/** @brief Parser action: set the noise frequency expression on a surface.
 *  @param surf   Surface to modify; existing node is freed first.
 *  @param exper  Expression node for Frequency; ownership transferred.
 *  @return No return value.
 */
void
frequency_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->Frequency != nullptr) deallocate_node(surf->Frequency);
   surf->Frequency = exper;
}

/** @brief Parser action: set the bump-map scale expression on a surface.
 *  @param surf   Surface to modify; existing node is freed first.
 *  @param exper  Expression node for Bump_scale; ownership transferred.
 *  @return No return value.
 */
void
bump_scale_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->Bump_scale != nullptr) deallocate_node(surf->Bump_scale);
   surf->Bump_scale = exper;
}

/** @brief Parser action: set the noise phase expression on a surface.
 *  @param surf   Surface to modify; existing node is freed first.
 *  @param exper  Expression node for Phase; ownership transferred.
 *  @return No return value.
 */
void
phase_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->Phase != nullptr) deallocate_node(surf->Phase);
   surf->Phase = exper;
}

/** @brief Parser action: set the position-function expression on a surface.
 *  @param surf   Surface to modify; existing node is freed first.
 *  @param exper  Expression node for Position_fn; ownership transferred.
 *  @return No return value.
 */
void
position_function_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->Position_fn != nullptr) deallocate_node(surf->Position_fn);
   surf->Position_fn = exper;
}

/** @brief Parser action: set the position-scale expression on a surface.
 *  @param surf   Surface to modify; existing node is freed first.
 *  @param exper  Expression node for Pos_scale; ownership transferred.
 *  @return No return value.
 */
void
position_scale_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->Pos_scale != nullptr) deallocate_node(surf->Pos_scale);
   surf->Pos_scale = exper;
}

/** @brief Parser action: set the reflection colour and scale expressions on a surface.
 *  @param surf   Surface to modify; existing nodes are freed first.
 *  @param color  Expression node for Kr_color; ownership transferred.
 *  @param scale  Expression node for Kr_scale; ownership transferred.
 *  @return No return value.
 */
void
reflection_action(Special_Surface *surf, NODE_PTR color, NODE_PTR scale)
{
   if (surf->Kr_color != nullptr) deallocate_node(surf->Kr_color);
   if (surf->Kr_scale != nullptr) deallocate_node(surf->Kr_scale);
   surf->Kr_color = color;
   surf->Kr_scale = scale;
}

/** @brief Parser action: set the specular colour and scale expressions on a surface.
 *  @param surf   Surface to modify; existing nodes are freed first.
 *  @param color  Expression node for Ks_color; ownership transferred.
 *  @param scale  Expression node for Ks_scale; ownership transferred.
 *  @return No return value.
 */
void
specular_action(Special_Surface *surf, NODE_PTR color, NODE_PTR scale)
{
   if (surf->Ks_color != nullptr) deallocate_node(surf->Ks_color);
   if (surf->Ks_scale != nullptr) deallocate_node(surf->Ks_scale);
   surf->Ks_color = color;
   surf->Ks_scale = scale;
}

/** @brief Parser action: set the transmission colour, scale, and IOR expressions on a surface.
 *  @param surf   Surface to modify; existing nodes are freed first.
 *  @param color  Expression node for Kt_color; ownership transferred.
 *  @param scale  Expression node for Kt_scale; ownership transferred.
 *  @param ior    Expression node for the index of refraction; ownership transferred.
 *  @return No return value.
 */
void
transmission_action(Special_Surface *surf, NODE_PTR color,
                    NODE_PTR scale, NODE_PTR ior)
{
   if (surf->Kt_color != nullptr) deallocate_node(surf->Kt_color);
   if (surf->Kt_scale != nullptr) deallocate_node(surf->Kt_scale);
   if (surf->ior != nullptr) deallocate_node(surf->ior);
   surf->Kt_color = color;
   surf->Kt_scale = scale;
   surf->ior = ior;
}

/** @brief Parser action: set the turbulence expression on a surface.
 *  @param surf   Surface to modify; existing node is freed first.
 *  @param exper  Expression node for Turbulence; ownership transferred.
 *  @return No return value.
 */
void
turbulence_action(Special_Surface *surf, NODE_PTR exper)
{
   if (surf->Turbulence != nullptr) deallocate_node(surf->Turbulence);
   surf->Turbulence = exper;
}

/** @brief Parser action: set the scene background colour.
 *
 *  If @p color evaluates to a constant vector at parse time it is stored
 *  directly in RuntimeState::BackgroundColor and the node is freed;
 *  otherwise the node is retained for run-time evaluation.
 *  @param color  Expression node for the background colour; ownership transferred.
 *  @return No return value.
 */
void
background_action(NODE_PTR color)
{
   Flt ftemp;
   NODE_PTR tnode;

   /* Run-time background */
   if (eval_node(nullptr, color, &ftemp, RuntimeState::BackgroundColor, &tnode) != 2) {
      if (RuntimeState::Background != nullptr) deallocate_node(RuntimeState::Background);
      RuntimeState::Background = color;
      }
   else {
      deallocate_node(color);
      RuntimeState::Background = nullptr;
      }
}

/** @brief Parser action: queue a parametric draw command for the current scene.
 *  @param low       Start parameter value for the draw sweep.
 *  @param high      End parameter value for the draw sweep.
 *  @param steps     Number of steps across [low, high].
 *  @param draw_fn   Expression that maps the parameter to a point; ownership transferred.
 *  @param color_fn  Expression that maps the parameter to a colour; ownership transferred.
 *  @return No return value.
 */
void draw_action(Flt low, Flt high, int steps, NODE_PTR draw_fn, NODE_PTR color_fn)
{
   DrawNode *tlist;

   /* Run-time background */
   tlist = make_draw_node(low, high, steps, draw_fn, color_fn);
   tlist->next = Draw_Commands;
   Draw_Commands = tlist;
}

/** @brief Parser action: set the display-buffer flush interval (if not overridden by the command line).
 *  @param pixel_count  Number of pixels to render between display updates.
 *  @return No return value.
 */
void flush_action(int pixel_count)
{
   if (RuntimeState::buffer_update != 1) {
      /* Only use this count if it hasn't been overridden
         from the command line */
      RuntimeState::buffer_update = 2;
      RuntimeState::buffer_size = pixel_count;
     }
}

/** @brief Parser action: allocate a single colour map entry.
 *  @param start   Start of the parameter interval.
 *  @param end     End of the parameter interval.
 *  @param svec    Colour vector at the start of the interval.
 *  @param strans  Transparency at the start of the interval.
 *  @param evec    Colour vector at the end of the interval.
 *  @param etrans  Transparency at the end of the interval.
 *  @return        Newly allocated map entry; next is set to nullptr.
 */
map_entries map_entry_action1(Flt start, Flt end, Vec svec, Flt strans,
                  Vec evec, Flt etrans)
{
   map_entries new_entry = Factorycolor_map_entry();
   if (new_entry == nullptr)
      serror("Failed to allocate a color map entry\n");
   new_entry->p0 = start;
   new_entry->p1 = end;
   VecCopy(svec, new_entry->v0);
   VecCopy(evec, new_entry->v1);
   new_entry->t0 = strans;
   new_entry->t1 = etrans;
   new_entry->next = nullptr;
   return new_entry;
}

/** @brief Parser action: append @p tail to the end of the @p head colour map list.
 *  @param head  First colour map list.
 *  @param tail  List to append.
 *  @return      @p head (head pointer unchanged).
 */
map_entries
map_entry_action2(map_entries head, map_entries tail)
{
   map_entries mlist = head;

   while (mlist->next != nullptr)
      mlist = mlist->next;
   mlist->next = tail;
   return head;
}

/** @brief Push a Transform onto the global transform stack (txstack).
 *  @param tx  Transform to push; must be heap-allocated.
 *  @return No return value.
 */
void
push_tx(Transform *tx)
{
   txstackptr element = Factorytransform_stack();
   if (element == nullptr)
      serror("Failed to allocate a transform");
   element->tx = tx;
   element->next = txstack;
   txstack = element;
}

/** @brief Pop and discard the top entry from the global transform stack (txstack).
 *  @return No return value.
 */
void
pop_tx()
{
   txstackptr element;

   element = txstack;
   txstack = txstack->next;
   delete element;
}

/** @brief Apply an arbitrary transform to an object, updating its bounding box and matrix.
 *  @param obj  Object to transform; o_trans is allocated lazily if nullptr.
 *  @param t    Transform to compose onto the object.
 *  @return No return value.
 */
void TransformObject(Object *obj, Transform *t)
{
   if (obj == nullptr || t == nullptr) return;
   if (obj->o_trans == nullptr) 
      obj->o_trans = Get_Transformation().release();
   recompute_bbox(&obj->o_bnd, t);
   Compose_Transformations(* obj->o_trans, *t);
}

/** @brief Apply a shear transform to an object, updating its bounding box and matrix.
 *  @param obj  Object to shear; o_trans is allocated lazily if nullptr.
 *  @param xy   Shear of X proportional to Y.
 *  @param xz   Shear of X proportional to Z.
 *  @param yx   Shear of Y proportional to X.
 *  @param yz   Shear of Y proportional to Z.
 *  @param zx   Shear of Z proportional to X.
 *  @param zy   Shear of Z proportional to Y.
 *  @return No return value.
 */
void ShearObject(Object *obj, Flt xy, Flt xz, Flt yx, Flt yz, Flt zx, Flt zy)
{
   Transform trans;

   Get_Shear_Transformation(&trans, xy, xz, yx, yz, zx, zy);
   if (obj->o_trans == nullptr) 
      obj->o_trans = Get_Transformation().release();
   recompute_bbox(&obj->o_bnd, &trans);
   Compose_Transformations(* obj->o_trans, trans);
}

/** @brief Translate an object, updating its bounding box lower-left corner.
 *  @param obj  Object to translate; o_trans is allocated lazily if nullptr.
 *  @param v    Translation vector.
 *  @return No return value.
 */
void
TranslateObject(Object *obj, Vec v)
{
   Transform trans;

   Get_Translation_Transformation(&trans, v);
   if (obj->o_trans == nullptr) 
      obj->o_trans = Get_Transformation().release();
   VecAdd(obj->o_bnd.lower_left, v, obj->o_bnd.lower_left);
   Compose_Transformations(* obj->o_trans, trans);
}

/** @brief Rotate an object by Euler angles (degrees), updating its bounding box.
 *  @param obj  Object to rotate; o_trans is allocated lazily if nullptr.
 *  @param v    Rotation angles in degrees around X, Y, and Z axes.
 *  @return No return value.
 */
void RotateObject(Object *obj, Vec v)
{
   Transform trans;
   Vec vt;

   VecCopy(v, vt);
   VecScale(PYM_PI/180.0, vt);
   Get_Rotation_Transformation(&trans, vt);
   if (obj->o_trans == nullptr) 
      obj->o_trans = Get_Transformation().release();
   recompute_bbox(&obj->o_bnd, &trans);
   Compose_Transformations(* obj->o_trans, trans);
}

/** @brief Rotate an object about an arbitrary axis, updating its bounding box.
 *  @param obj  Object to rotate; o_trans is allocated lazily if nullptr.
 *  @param v    Rotation axis (need not be unit length).
 *  @param ang  Rotation angle in degrees.
 *  @return No return value.
 */
void
RotateAxisObject(Object *obj, Vec v, Flt ang)
{
   Transform trans;

   Get_Rotate_Transform(&trans, v, PYM_PI * ang / 180.0);
   if (obj->o_trans == nullptr) 
      obj->o_trans = Get_Transformation().release();
   recompute_bbox(&obj->o_bnd, &trans);
   Compose_Transformations(* obj->o_trans, trans);
}

/** @brief Scale an object non-uniformly, updating its bounding box.
 *  @param obj  Object to scale; o_trans is allocated lazily if nullptr.
 *  @param v    Scale factors along X, Y, and Z.
 *  @return No return value.
 */
void
ScaleObject(Object *obj, Vec v)
{
   Transform trans;

   Get_Scaling_Transformation (&trans, v);
   if (obj->o_trans == nullptr) 
      obj->o_trans = Get_Transformation().release();
   recompute_bbox(&obj->o_bnd, &trans);
   Compose_Transformations(* obj->o_trans, trans);
}

/** @brief Parser action: set the root-finding solver on a torus, polynomial, blob, or lathe.
 *  @param obj     Object whose solver is to be changed (must be one of the supported types).
 *  @param solver  Solver identifier forwarded to the type-specific setter.
 *  @return No return value.
 */
void
root_solver_action(Object *obj, int solver)
{
   if (obj->o_type == ShapeType::Torus)
      Set_Torus_Solver(obj, solver);
   else if (obj->o_type == ShapeType::Polynomial)
      Set_Polynomial_Solver(obj, solver);
   else if (obj->o_type == ShapeType::Blob)
      Set_Blob_Solver(obj, solver);
   else if (obj->o_type == ShapeType::Revolve)
      Set_Lathe_Solver(obj, solver);
}

/** @brief Parser action: prepend a spherical blob component to blob_components.
 *  @param pos       Centre of the sphere in object space.
 *  @param strength  Field strength coefficient.
 *  @param radius    Radius of influence.
 *  @return No return value.
 */
void
spherical_component_action(Vec pos, Flt strength, Flt radius)
{
   blob_component = (blobstackptr)polyray_malloc(sizeof(struct blob_list_struct));
   if (blob_component == nullptr)
      serror("Failed to allocate a blob component\n");
   blob_component->elem.type = BlobType::T_SPHERICAL_BLOB;
   blob_component->elem.coeffs[2] = strength;
   blob_component->elem.radius2   = radius;
   VecCopy(pos, blob_component->elem.pos);
   blob_component->next = blob_components;
   blob_components = blob_component;
   npoints++;
}

/** @brief Parser action: prepend a cylindrical blob component to blob_components.
 *  @param pos0      One endpoint of the cylinder axis.
 *  @param pos1      Other endpoint of the cylinder axis.
 *  @param strength  Field strength coefficient.
 *  @param radius    Radius of influence around the axis.
 *  @return No return value.
 */
void
cylindrical_component_action(Vec pos0, Vec pos1, Flt strength, Flt radius)
{
   blob_component = (blobstackptr)polyray_malloc(sizeof(struct blob_list_struct));
   if (blob_component == nullptr)
      serror("Failed to allocate a blob component\n");
   blob_component->elem.type = BlobType::T_CYLINDRICAL_BLOB;
   blob_component->elem.coeffs[2] = strength;
   blob_component->elem.radius2   = radius;
   VecCopy(pos0, blob_component->elem.pos);
   VecCopy(pos1, blob_component->elem.dir);
   blob_component->next = blob_components;
   blob_components = blob_component;
   npoints++;
}

/** @brief Parser action: prepend a planar blob component to blob_components.
 *  @param N         Plane normal vector.
 *  @param d         Plane offset (distance from origin along @p N).
 *  @param strength  Field strength coefficient.
 *  @param radius    Radius of influence from the plane.
 *  @return No return value.
 */
void
planar_component_action(Vec N, Flt d, Flt strength, Flt radius)
{
   blob_component = (blobstackptr)polyray_malloc(sizeof(struct blob_list_struct));
   if (blob_component == nullptr)
      serror("Failed to allocate a blob component\n");
   blob_component->elem.type = BlobType::T_PLANAR_BLOB;
   blob_component->elem.coeffs[2] = strength;
   blob_component->elem.radius2   = radius;
   VecCopy(N, blob_component->elem.dir);
   blob_component->elem.len = d;
   blob_component->next = blob_components;
   blob_components = blob_component;
   npoints++;
}

/** @brief Parser action: prepend a toroidal blob component to blob_components.
 *  @param C         Centre of the torus ring.
 *  @param N         Normal to the plane of the torus ring.
 *  @param major     Major radius of the torus.
 *  @param strength  Field strength coefficient.
 *  @param radius    Minor radius of influence (tube radius).
 *  @return No return value.
 */
void
toroidal_component_action(Vec C, Vec N, Flt major,
                          Flt strength, Flt radius)
{
   blob_component = (blobstackptr)polyray_malloc(sizeof(struct blob_list_struct));
   if (blob_component == nullptr)
      serror("Failed to allocate a blob component\n");
   blob_component->elem.type = BlobType::T_TOROIDAL_BLOB;
   blob_component->elem.coeffs[2] = strength;
   blob_component->elem.len       = major;
   blob_component->elem.radius2   = radius;
   VecCopy(C, blob_component->elem.pos);
   VecCopy(N, blob_component->elem.dir);
   blob_component->next = blob_components;
   blob_components = blob_component;
   npoints++;
}

/** @brief Find the maximum total degree among all TERM nodes in an expression list.
 *  @param list  Linked list of expression nodes from the polynomial parser.
 *  @return      Highest sum (x_power + y_power + z_power) found; 0 if no TERM nodes.
 */
static int max_power_used(LIST_PTR list)
{
   NODE_PTR term;
   int t, result = 0;
   while (list != nullptr) {
      term = list->element;
      if (term->exper_type == TERM) {
         auto cn=std::get<coeff_node>(term->exper_data);
         t = (int)cn.x_power +
             (int)cn.y_power +
             (int)cn.z_power;
         if (t > result) result = t;
         }
      list = list->next;
      }
   return result;
}

/** @brief Map a monomial (x^x * y^y * z^z) of given total @p order to a flat coefficient index.
 *
 *  Coefficients are stored in descending lexicographic order; this function
 *  computes the index using binomial coefficients.
 *  @param order  Total degree of the polynomial.
 *  @param x      Power of x in the monomial.
 *  @param y      Power of y in the monomial.
 *  @param z      Power of z in the monomial.
 *  @return       Index into the coefficient array for this monomial.
 */
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

/** @brief Convert a parse-tree expression list into a flat polynomial coefficient array.
 *
 *  Determines the polynomial degree, allocates the coefficient array via
 *  polyray_malloc, and accumulates each TERM node into the appropriate slot
 *  using roll().  Non-TERM nodes emit a warning and are skipped.
 *  @param list    Linked list of expression nodes (TERM nodes expected).
 *  @param oorder  Output: degree of the polynomial.
 *  @return        Newly allocated coefficient array (caller owns; free with polyray_free).
 */
static Flt *
generate_coeffs(LIST_PTR list, int *oorder)
{
   NODE_PTR term;
   int order, i, term_count;
   Flt *coeffs;

   order = max_power_used(list);
   if (order > MAX_POLYNOMIAL_ORDER)
      /* Would need more than 64K to store all of the coefficients. */
      serror("Polynomial is of order %d, this is too large\n", order);
   term_count = (order + 1) * (order + 2) * (order + 3) / 6;
   coeffs = (Flt*)polyray_malloc(sizeof(Flt) * term_count);
   if (coeffs == nullptr)
      serror("Failed to allocate polynomial coeffs\n");
   for (i=0;i<term_count;i++)
      coeffs[i] = 0.0;
   while (list != nullptr) {
      term = list->element;
      if (term->exper_type == TERM) {
         auto cn=std::get<coeff_node>(term->exper_data);
         i = roll(order, (int)cn.x_power,
                  (int)cn.y_power,
                  (int)cn.z_power);
         coeffs[i] += cn.coeff;
         }
      else {
         smessage(" { Omitting term: ");
         show_node(term);
         smessage("} \n");
         }
      list = list->next;
      }
   *oorder = order;
   return coeffs;
}

/** @brief Parser action: simplify an expression, extract polynomial coefficients, and build the object.
 *  @param data    Raw parse-tree expression for the polynomial; ownership transferred.
 *  @param solver  Root-solver selection forwarded to MakeNewPolynomial().
 *  @return No return value.
 */
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
   (void)MakeNewPolynomial(Object_Stack->element, CurrentOrder, Coeffs, solver);
}

/** @brief Allocate and initialise a CSG tree node.
 *  @param type   CSG operation type (e.g. T_UNION, T_INTERSECTION, T_DIFFERENCE).
 *  @param left   Left child; either an Object* or a csgnodeptr; ownership transferred.
 *  @param right  Right child; same semantics as @p left.
 *  @return       Newly allocated CSG node.
 */
csgnodeptr
make_csg_node(int type, void *left, void *right)
{
   csgnodeptr result = Factorycsgnode();
   if (result == nullptr)
      serror("Failed to allocate a CSG node\n");
   result->type = type;
   result->left = left;
   result->right = right;
   return result;
}

/** @brief Parser action: attach a CSG tree to the current object on the Object_Stack.
 *  @param data  Root of the CSG tree; ownership transferred to the object.
 *  @return No return value.
 */
void
csg_action1(csgnodeptr data)
{
   (void)MakeCSG(Object_Stack->element, data);
}

/** @brief Append a control point to a Bezier patch point list (maximum 16 points).
 *
 *  Allocates a new VList and 16-Vec point buffer when @p points is nullptr.
 *  @param points  Existing point list, or nullptr to start a new one.
 *  @param point   Control point to append.
 *  @return        @p points (or the newly allocated list if @p points was nullptr).
 */
VList *
add_bezier_point(VList *points, Vec point)
{
   VList *plist = nullptr;

   if (points == nullptr) {
      plist = (VList*)polyray_malloc(sizeof(VList));
      if (plist == nullptr)
         serror("Failed to allocate Bezier structure\n");
      plist->points = (Vec*)polyray_malloc(16 * sizeof(Vec));
      if (plist->points == nullptr)
         serror("Failed to allocate Bezier point list\n");
      plist->count = 1;
      VecCopy(point, plist->points[0]);
      }
   else if (points->count == 16)
      serror("Too many points in Bezier patch, must only be 16");
   else {
      plist = points;
      VecCopy(point, plist->points[plist->count]);
      plist->count++;
      }
   return plist;
}

/** @brief Look up a named string in the symbol table and return a heap copy.
 *
 *  The returned buffer is allocated with polyray_malloc and must be freed
 *  by the caller via polyray_free.
 *  @param defname  Symbol-table key of the string.
 *  @return         Newly allocated copy of the string value.
 */
static char *translate_string_mark_for_deletion(char *defname)
{
   int c;
   void *d;
   char *newstr, *oldstr;

   Lookup_Definition(defname, &c, &d);
   if (c != std::to_underlying(ShapeType::String))
      serror("String '%s' not found in symbol table\n", defname);
   oldstr = (char *)d;
   const size_t oldstrlen = strlen(oldstr);
   newstr = (char *)polyray_malloc((oldstrlen + 1) * sizeof(char));
   if (newstr == nullptr)
      serror("Failed to allocate string space");
   memcpy(newstr, oldstr, oldstrlen + 1);
   return newstr;
}

/** @brief Evaluate a list of expression nodes and concatenate them into a C string.
 *
 *  Each node is evaluated via C_create_string(); the results are concatenated
 *  into a static 256-character buffer.
 *  @param args  Linked list of expression nodes to evaluate; nodes are freed as consumed.
 *  @return      Pointer to a static buffer containing the concatenated string (max 255 chars).
 */
char *
build_string(LIST_PTR args)
{
   static char temp_str[256];
   char *tstr;
   int i, j, k, argc;
   LIST_PTR targs;

   /* Figure out how many arguments there are */
   for (argc=0,targs=args;targs!=nullptr;argc++,targs=targs->next) ;

   temp_str[0] = '\0';
   for (i=0,j=1,k=0;i<argc && j==1;++i) {
      /* Evaluate each argument, appending the results as
         we go */
      j = C_create_string(args->element, &tstr);
      if (j == 1) {
         k += strlen(tstr);
         if (k > 255)
            serror("String too long\n");
         strcat(temp_str, tstr);
         polyray_free(tstr);
         }
      else
         serror("Non-string used in system call\n");
      deallocate_node(args->element);
      args = args->next;
      }

   /* Clean up the memory used to hold the arguments */
   while (args != nullptr) {
      targs = args;
      args = args->next;
      polyray_free(targs);
      }

   return &temp_str[0];
}

/** @brief Evaluate a parse-tree node to a std::string.
 *
 *  Handles STRING nodes directly; scalar results are formatted as numbers,
 *  vector results as "{x, y, z}".
 *  @param exper  Expression node to evaluate.
 *  @param name   Output: string result.
 *  @return       1 on success, 0 if the node cannot be converted to a string.
 */
int create_string(NODE_PTR exper, std::string& name)
{
   int i;
   Flt ftmp;
   Vec vtmp;
   //char *stmp,
   std::string stmp{""};
   char buffer[128];
   NODE_PTR ntmp;

   if (exper->exper_type == STRING)
      /* Simple copy */
      stmp = std::get<std::string>(exper->exper_data);
   else {
      i = eval_node(nullptr, exper, &ftmp, vtmp, &ntmp);
      if (i == 1) {
         // number to string
         stmp=int2string(ftmp);
         }
      else if (i == 2) {
         /* Create a string from a vector */
         stmp="{"+int2string(vtmp[0])+", "+int2string(vtmp[1])+", "+int2string(vtmp[2])+"}";
         }
      else
         return 0;
      }

   //returning stmp then
   name = stmp;
   return 1;
}

/** @brief C-compatible wrapper around create_string() that allocates a char* output buffer.
 *
 *  The buffer is allocated with @c polyray_malloc and must be freed with
 *  @c polyray_free.
 *  @param exper  Expression node to evaluate.
 *  @param name   Output: pointer set to a newly allocated null-terminated string.
 *  @return       1 on success, 0 if the node cannot be converted to a string.
 */
int C_create_string(NODE_PTR exper, char **name)
{
   std::string sname;
   const int i = create_string(exper, sname);
   if (i == 0) {
      *name = nullptr;
      return 0;
      }
   *name = static_cast<char*>(polyray_malloc(sname.length() + 1));
   if (*name == nullptr)
      serror("Failed to allocate string buffer\n");
   memcpy(*name, sname.c_str(), sname.length() + 1);
   return i;
}

/** @brief Parser action: evaluate an expression list to a string and execute it as a shell command.
 *  @param args  Expression list forming the command; nodes are freed by build_string().
 *  @return No return value.
 */ 
void evaluate_system_call(LIST_PTR args)
{
   /* build_string() must run unconditionally: it frees the argument nodes
      and the list. Only the shell invocation is gated by -s. */
   char *command = build_string(args);
   if (runtimeState::Allow_SystemCalls)
      system(command);
}

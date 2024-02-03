%{
/*

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
#include "psupport.h"
#include "symtab.h"
#include "texture.h"
#include "particle.h"
#include "light.h"
#include "parse.h"
#include "vector.h"
#include "eval.h"
#include "builder.h"

/* Include files for the various shapes */
#include "bezier.h"
#include "blob.h"
#include "box.h"
#include "cone.h"
#include "csg.h"
#include "cylinder.h"
#include "disc.h"
#include "function.h"
#include "glyph.h"
#include "gridded.h"
#include "height.h"
#include "hypertex.h"
#include "parabola.h"
#include "parametr.h"
#include "poly.h"
#include "polynom.h"
#include "raw.h"
#include "revolve.h"
#include "sphere.h"
#include "superq.h"
#include "sweep.h"
#include "torus.h"
#include "tri.h"

#define ACTION(x) { if (check_condition()) { x } }
#define alloca malloc
#define yyerror error

struct def_tok_struct {
   int type;
   void *data;
   } temp_def;
static Contour *cl, *contours;
static int gcount;
%}

%token ACCELERATION
%token ACOS
%token AMBIENT
%token AND_EXPER
%token ANGLE
%token ANTIALIAS
%token ANTIALIAS_THRESHOLD
%token APERTURE
%token ARRAY
%token ASSIGNMENT
%token ASIN
%token ASPECT
%token AT
%token ATAN
%token ATAN_TWO
%token AVOID
%token BACKGROUND
%token BEZIER
%token BIAS
%token BIRTH
%token BLINN
%token BLOB
%token BOUNDING_BOX
%token BOX
%token BRILLIANCE
%token BUMP_SCALE
%token CEIL
%token CHECKER
%token CHEIGHT_FIELD
%token CHEIGHT_FN
%token COLOR
%token COLOR_MAP
%token COLOR_WHEEL
%token CONCAT
%token CONCAVE
%token CONDITIONAL_EXPER
%token CONE
%token CONTOUR
%token COOK
%token COS
%token COSH
%token COUNT
%token CSG
%token CYLINDER
%token CYLINDRICAL_BUMPMAP
%token CYLINDRICAL_IMAGEMAP
%token CYLINDRICAL_INDEXED
%token DEATH
%token DEFINE
%token DEGREES
%token DEPTH
%token DEPTHMAPPED_LIGHT
%token DIFFUSE
%token DIRECTIONAL_LIGHT
%token DISC
%token DISPLACE
%token DITHER
%token DIV_EXPER
%token DNOISE
%token DOT_EXPER
%token DRAW
%token ELSE
%token END_FRAME
%token ENVIRONMENT
%token ENVIRONMENT_MAP
%token EQUAL_EXPER
%token EXP
%token EXPRESSION_SYM
%token FABS
%token FBM
%token FERRARI
%token FILE_FLUSH
%token FLARE
%token FLOCK
%token FLOOR
%token FNOISE
%token FOCAL_DISTANCE
%token FMOD
%token FRAME
%token FRAME_TIME
%token FREQUENCY
%token FROM
%token FUNCTION
%token GAIN
%token GAUSSIAN
%token GLYPH
%token GREATER_EXPER
%token GRIDDED
%token GTEQ_EXPER
%token HAZE
%token HEIGHT_FIELD
%token HEIGHT_FN
%token HEIGHT_MAP
%token HEXAGON
%token HITHER
%token HYPERTEXTURE
%token I_EXPER
%token IF
%token IMAGE
%token IMAGE_FORMAT
%token IMAGE_WINDOW
%token INCLUDE
%token INDEXED
%token INDEXED_MAP
%token LATHE
%token LAYERED
%token LEGENDRE
%token LENSES
%token LESS_EXPER
%token LIGHT
%token LN
%token LOG
%token LOOKUP_FUNCTION
%token LTEQ_EXPER
%token MAXT
%token MAX_SAMPLES
%token MAX_TRACE_DEPTH
%token MICROFACET
%token MINT
%token MINUS_EXPER
%token N_EXPER
%token NO_SHADOW
%token NOEVAL
%token NOISE
%token NORMAL
%token NOT_EXPER
%token NUM
%token NURB
%token OBJECT
%token OBJECT_SYM
%token OCTAVES
%token OR_EXPER
%token OPACITY
%token OUTFILE
%token P_EXPER
%token PARABOLA
%token PARAMETRIC
%token PARTICLE
%token PARTICLE_SYM
%token PATCH
%token PHASE
%token PHONG
%token PIXEL_ENCODING
%token PIXELSIZE
%token PLANAR_BUMPMAP
%token PLANAR_IMAGEMAP
%token PLANE
%token PLUS_EXPER
%token POINT
%token POLYGON
%token POLYNOMIAL
%token POSITION
%token POSITION_FUNCTION
%token POSITION_SCALE
%token POWER_EXPER
%token RADIANS
%token RAMP
%token RANDOM
%token RAW
%token REFLECT
%token REFLECTION
%token REITZ
%token RESOLUTION
%token RIPPLE
%token ROOT_SOLVER
%token ROTATE
%token SAWTOOTH
%token SCALE
%token SEED
%token SHADING_FLAGS
%token SHEAR
%token SIN
%token SINH
%token SIZE
%token SMOOTH_HEIGHT_FIELD
%token SMOOTH_HEIGHT_FN
%token SMOOTH_CHEIGHT_FIELD
%token SMOOTH_CHEIGHT_FN
%token SMOOTH_SHEIGHT_FIELD
%token SMOOTH_SHEIGHT_FN
%token SPACING
%token SPECIAL
%token SPECIAL_SURFACE_SYM
%token SHEIGHT_FIELD
%token SHEIGHT_FN
%token SPHERICAL_BUMPMAP
%token SPHERICAL_IMAGEMAP
%token SPHERICAL_INDEXED
%token SPLINE
%token SPOT_LIGHT
%token SQRT
%token SPECULAR
%token SPHERE
%token START_FRAME
%token STATIC
%token STRING
%token STURM
%token SUBSCRIPT_EXPER
%token SUMMED
%token SUPERQ
%token SURFACE
%token SURFACE_SYM
%token SYSTEM
%token SWEEP
%token TAN
%token TANH
%token TERM
%token TEXTURE
%token TEXTURE_MAP
%token TEXTURE_MAP_SYM
%token TEXTURE_SYM
%token TEXTURED_LIGHT
%token TIMES_EXPER
%token TOKEN
%token TORUS
%token TOTAL_FRAMES
%token TRACE
%token TRANSFORM
%token TRANSFORM_SYM
%token TRANSLATE
%token TRANSMISSION
%token TURBULENCE
%token UMINUS_EXPER
%token UP
%token U_EXPER
%token UU_EXPER
%token UV_EXPER
%token UW_EXPER
%token U_STEPS
%token V_STEPS
%token UV
%token UV_STEPS
%token UV_BOUNDS
%token VELOCITY
%token VIETA
%token VIEWPOINT
%token VISIBLE
%token VAL_EXPER
%token VEC_EXPER
%token VECTOR_EXPER
%token WAVE
%token W_EXPER
%token W_STEPS
%token X_EXPER
%token X_OFFSET
%token Y_EXPER
%token Y_OFFSET
%token YON
%token Z_EXPER
%token '&'
%token '='
%token '['
%token ']'
%token '('
%token ')'
%token '{'
%token '}'
%token '+'
%token '-'
%token '*'
%token '.'
%token '/'
%token '>'
%token '<'
%token '^'
%token '!'
%token '~'
%token AND_SYM
%token OR_SYM
%token LTEQ_SYM
%token GTEQ_SYM
%token EQUAL_SYM
%token NEQUAL_SYM

%union {
   Vec vec;
   VList *vecl;
   Flt flt;
   Flt *fltptr;
   Object *obj;
   NODE_PTR exper;
   LIST_PTR elist;
   char *name;
   void *data;
   csgnodeptr csgtree;
   Special_Surface *surf;
   Texture *text;
   texture_map_entries text_map;
   texture_fn_entries text_fn;
   map_entries cmap_entry;
   Transform *trns;
   ostackptr objlist;
   tstackptr textlist;
   Particle *part;
   Light *lgt;
}

%type <elist> expression_list
%type <exper> expression conditional
%type <vec> point
%type <flt> fexper NUM
%type <obj> object
%type <lgt> light
%type <objlist> object_list
%type <name> defined_token TOKEN TRANSFORM_SYM TEXTURE_MAP_SYM
%type <name> SURFACE_SYM OBJECT_SYM TEXTURE_SYM EXPRESSION_SYM
%type <name> PARTICLE_SYM sexper STRING
%type <surf> surface
%type <text> texture
%type <text_map> texture_map texture_map_elements
%type <text_fn> texture_fn_element texture_fn_elements
%type <trns> transform
%type <csgtree> csg_tree
%type <vecl> bezier_points
%type <cmap_entry> map_entry map_entries
%type <textlist> texture_list
%type <part> particle

/* Precedence table */
%left AND_SYM OR_SYM
%left EQUAL_SYM NEQUAL_SYM
%left '<' '>' LTEQ_SYM GTEQ_SYM
%left '&'
%left '+' '-'
%left '*' '/' '.' '%'
%left '^'
%left '~'
%left UMINUS
%right '!'
%nonassoc '(' '['
%%

scene
   : { condition_flags[0] = 1; }
     elementlist 
   ;

elementlist
   : elementlist element
   | element
   ;

element
   : background
   | camera
   | definition
   | draw_statement
   | flush_statement
   | frame_decl
   | if_statement
   | include_statement
   | light
     { ACTION(Add_To_Lights($<lgt>1);) }
   | haze_statement
   | object
     { ACTION(Add_To_BinTree(&Root, $<obj>1);) }
   | outfile
   | system_call
   | particle
     { ACTION(InsertParticle($<part>1);) }
   ;

include_statement
   : INCLUDE STRING
      { ACTION(include_file_action((char *)POLYRAY_PATH_STRING, $<name>2);)
        polyray_free($<name>2); }
   ;

defined_token
   : SURFACE_SYM
      { $<name>$ = $<name>1; }
   | TEXTURE_SYM
      { $<name>$ = $<name>1; }
   | TEXTURE_MAP_SYM
      { $<name>$ = $<name>1; }
   | OBJECT_SYM
      { $<name>$ = $<name>1; }
   | EXPRESSION_SYM
      { $<name>$ = $<name>1; }
   | TRANSFORM_SYM
      { $<name>$ = $<name>1; }
   | PARTICLE_SYM
      { $<name>$ = $<name>1; }
   ;

definition_types
   : surface
      { ACTION(temp_def.type = T_SURFACE;
               temp_def.data = $<surf>1;) }
   | texture
      { ACTION(temp_def.type = T_TEXTURE;
               temp_def.data = $<text>1;) }
   | texture_map
      { ACTION(temp_def.type = T_TEXTURE_MAP;
               temp_def.data = $<text_map>1;) }
   | object
      { ACTION(temp_def.type = T_OBJECT;
               temp_def.data = $<obj>1;) }
   | transform
      { ACTION(temp_def.type = T_TRANSFORM;
               temp_def.data = $<trns>1;) }
   | expression
      { ACTION(temp_def.type = T_EXPRESSION;
               temp_def.data = $<exper>1;) }
   | particle
      { ACTION(temp_def.type = T_PARTICLE;
               temp_def.data = $<part>1;) }
   ;

definition
   : DEFINE defined_token definition_types
      { ACTION(Insert_Definition($<name>2, temp_def.type,
                                 temp_def.data, 0, 0);) }
   | STATIC DEFINE defined_token definition_types
      { ACTION(Insert_Definition($<name>3, temp_def.type,
                                 temp_def.data, 1, 0);) }
   | DEFINE TOKEN definition_types
      { ACTION(Insert_Definition($<name>2, temp_def.type,
                                 temp_def.data, 0, 0);)
        polyray_free($<name>2); }
   | STATIC DEFINE TOKEN definition_types
      { ACTION(Insert_Definition($<name>3, temp_def.type,
                                 temp_def.data, 1, 0);)
        polyray_free($<name>3); }
   | DEFINE NOEVAL defined_token definition_types
      { ACTION(Insert_Definition($<name>3, temp_def.type,
                                 temp_def.data, 0, 1);) }
   | STATIC DEFINE NOEVAL defined_token definition_types
      { ACTION(Insert_Definition($<name>4, temp_def.type,
                                 temp_def.data, 1, 1);) }
   | DEFINE NOEVAL TOKEN definition_types
      { ACTION(Insert_Definition($<name>3, temp_def.type,
                                 temp_def.data, 0, 1);)
        polyray_free($<name>3); }
   | STATIC DEFINE NOEVAL TOKEN definition_types
      { ACTION(Insert_Definition($<name>4, temp_def.type,
                                 temp_def.data, 1, 1);)
        polyray_free($<name>4); }
   ;

particle_decls
   : particle_decl
   | particle_decls particle_decl
   ;

particle_decl
   : BIRTH expression
      { ACTION(SetParticleBirth(CurrentParticle, $<exper>2);) }
   | DEATH expression
      { ACTION(SetParticleDeath(CurrentParticle, $<exper>2);) }
   | POSITION expression
      { ACTION(SetParticleP(CurrentParticle, $<exper>2);) }
   | VELOCITY expression
      { ACTION(SetParticleV(CurrentParticle, $<exper>2);) }
   | ACCELERATION expression
      { ACTION(SetParticleA(CurrentParticle, $<exper>2);) }
   | AVOID expression
      { ACTION(SetParticleAvoid(CurrentParticle, $<exper>2);) }
   | FLOCK expression
      { ACTION(SetParticleFlock(CurrentParticle, $<exper>2);) }
   | COUNT expression
      { ACTION(SetParticleCount(CurrentParticle, $<exper>2);) }
   | OBJECT expression
      { ACTION(SetParticleObjName(CurrentParticle, $<exper>2);) }
   ;

particle
   : PARTICLE '{'
     { ACTION(CurrentParticle = CreateParticle();) }
     particle_decls '}'
     { ACTION($<part>$ = CurrentParticle;) }
   | PARTICLE_SYM
      { ACTION($<part>$ = CopyParticle($<name>1);) }
   ;

object
   : OBJECT '{'
      { ACTION(Object_Stack = push_object(Object_Stack, object_action1());) }
     object_decls '}'
      { ACTION($<obj>$ = pop_object(&Object_Stack);) }
   | OBJECT_SYM
      { ACTION($<obj>$ = object_action2($<name>1);) }
   | OBJECT_SYM '{'
      { ACTION(Object_Stack =
               push_object(Object_Stack, object_action2($<name>1));) }
     object_modifier_decls '}'
      { ACTION($<obj>$ = pop_object(&Object_Stack);) }
   ;

object_modifier_decls
   : object_modifier_decl
   | object_modifier_decls object_modifier_decl
   ;

object_modifier_decl
   : texture
      { ACTION(if (Object_Stack->element->o_texture != NULL)
                        TextureDelete(Object_Stack->element->o_texture);
                     Object_Stack->element->o_texture = $<text>1;) }
   | transform
      { ACTION(TransformObject(Object_Stack->element, $<trns>1);
               polyray_free($<trns>1);) }
   | ROTATE point
      { ACTION(RotateObject(Object_Stack->element, $<vec>2);) }
   | ROTATE point ',' fexper
      { ACTION(RotateAxisObject(Object_Stack->element, $<vec>2, $<flt>4);) }
   | SHEAR fexper ',' fexper ',' fexper ',' fexper ','
           fexper ',' fexper
      { ACTION(ShearObject(Object_Stack->element, $<flt>2, $<flt>4,
                           $<flt>6, $<flt>8, $<flt>10, $<flt>12);) }
   | TRANSLATE point
      { ACTION(TranslateObject(Object_Stack->element, $<vec>2);) }
   | SCALE point
      { ACTION(ScaleObject(Object_Stack->element, $<vec>2);) }
   | uv_information
   | SHADING_FLAGS fexper
      { ACTION(Object_Stack->element->o_sflag = (int)$<flt>2;) }
   | DITHER fexper
      { ACTION(Object_Stack->element->o_dither = $<flt>2;) }
   | BOUNDING_BOX point ',' point
     { ACTION(VecCopy($<vec>2, Object_Stack->element->o_bnd.lower_left);
              VecCopy($<vec>4, Object_Stack->element->o_bnd.lengths);
              VecSub(Object_Stack->element->o_bnd.lengths,
                     Object_Stack->element->o_bnd.lower_left,
                     Object_Stack->element->o_bnd.lengths);) }
   | root_solver
   | DISPLACE expression
      { ACTION(Object_Stack->element->o_displace = $<exper>2;) }
   ;

uv_information
   : UV_STEPS fexper ',' fexper
      { ACTION(Object_Stack->element->o_uv_steps[0] = (int)$<flt>2;
               Object_Stack->element->o_uv_steps[1] = (int)$<flt>4;
               Object_Stack->element->o_uv_steps[2] = (int)$<flt>4;
               Object_Stack->element->o_sflag &= ~ADAPTIVE_UV;) }
   | UV_STEPS fexper ',' fexper ',' fexper
      { ACTION(Object_Stack->element->o_uv_steps[0] = (int)$<flt>2;
               Object_Stack->element->o_uv_steps[1] = (int)$<flt>4;
               Object_Stack->element->o_uv_steps[2] = (int)$<flt>6;
               Object_Stack->element->o_sflag &= ~ADAPTIVE_UV;) }
   | U_STEPS fexper
      { ACTION(Object_Stack->element->o_uv_steps[0] = (int)$<flt>2;
               Object_Stack->element->o_sflag &= ~ADAPTIVE_UV;) }
   | V_STEPS fexper
      { ACTION(Object_Stack->element->o_uv_steps[1] = (int)$<flt>2;
               Object_Stack->element->o_sflag &= ~ADAPTIVE_UV;) }
   | W_STEPS fexper
      { ACTION(Object_Stack->element->o_uv_steps[2] = (int)$<flt>2;
               Object_Stack->element->o_sflag &= ~ADAPTIVE_UV;) }
   | UV_BOUNDS fexper ',' fexper ',' fexper ',' fexper
      { ACTION(Object_Stack->element->o_uv_bounds[0] = $<flt>2;
               Object_Stack->element->o_uv_bounds[1] = $<flt>4;
               Object_Stack->element->o_uv_bounds[2] = $<flt>6;
               Object_Stack->element->o_uv_bounds[3] = $<flt>8;) }
   ;

root_solver
   : ROOT_SOLVER FERRARI
      { ACTION(root_solver_action(Object_Stack->element, 0);) }
   | ROOT_SOLVER VIETA
      { ACTION(root_solver_action(Object_Stack->element, 1);) }
   | ROOT_SOLVER STURM
      { ACTION(root_solver_action(Object_Stack->element, 2);) }
   ;

object_decls
   : shape_decl
   | shape_decl object_modifier_decls
   ;

shape_decl
   : bezier
   | blob
   | box
   | cone
   | cylinder
   | cylindrical_height_field
   | cylindrical_height_fn
   | csg
   | disc
   | function
   | glyph
   | gridded
   | height_field
   | height_fn
   | hypertexture
   | lathe
   | light_object
   | nurb
   | parabola
   | parametric
   | polygon
   | polynomial
   | ppatch
   | raw
   | smooth_cheight_field
   | smooth_cheight_fn
   | smooth_height_field
   | smooth_height_fn
   | smooth_sheight_field
   | smooth_sheight_fn
   | sphere
   | spherical_height_field
   | spherical_height_fn
   | superq
   | sweep
   | torus
   ;

camera_exper
   : ANGLE fexper
     { ACTION(Eye.view_angle = degtorad($<flt>2/2.0 );) }
   | ANTIALIAS fexper
     { ACTION(antialias = (int)$<flt>2;
              if (antialias < 0 || antialias > 4)
                 error("Antialias value of %d is not between 0 and 4",
                       antialias);)}
   | ANTIALIAS_THRESHOLD fexper
     { ACTION(antialias_threshold = $<flt>2;) }
   | APERTURE fexper
     { ACTION(Eye.view_aperture = $<flt>2;) }
   | ASPECT fexper
     { ACTION(Eye.view_aspect = $<flt>2;) }
   | AT point
     { ACTION(VecCopy($<vec>2, Eye.view_at);) }
   | FOCAL_DISTANCE fexper
     { ACTION(Eye.view_focaldist = $<flt>2;) }
   | FROM point
     { ACTION(VecCopy($<vec>2, Eye.view_from);) }
   | HITHER fexper
     { ACTION(Eye.view_hither = $<flt>2;) }
   | IMAGE_FORMAT fexper
     { ACTION(if ((int)($<flt>2) == 0)
                 DepthRender = 0;
              else if ((int)($<flt>2) == 1) {
                 pixel_encoding = 0;
                 DepthRender = 1;
                 }
              else
                 error("image_format must be either 0 (normal) or 1 (depth)");
              ) }
   | IMAGE_WINDOW fexper ',' fexper ',' fexper ',' fexper
     { ACTION(Eye.view_x0 = (int) $<flt>2;
              Eye.view_y0 = (int) $<flt>4;
              Eye.view_xl = (int) $<flt>6;
              Eye.view_yl = (int) $<flt>8;) }
   | MAX_SAMPLES fexper
     { ACTION(maxsamples = (int)$<flt>2;
              if (maxsamples < 0)
                 error("maxsamples must be greater than 0");)}
   | MAX_TRACE_DEPTH fexper
     { ACTION(maxlevel = (int)$<flt>2;
              if (maxlevel < 1 || maxlevel > 63)
                 error("maxlevel must be between 1 and 63");)}
   | PIXEL_ENCODING fexper
     { ACTION(pixel_encoding = (int)$<flt>2;
              if (pixel_encoding != 0 && pixel_encoding != 1)
                 error("Pixel encoding of %d is not one of: 0 [none], 1 [RLE]",
                       pixel_encoding);) }
   | PIXELSIZE fexper
     { ACTION(pixelsize = (int)$<flt>2;
              if (pixelsize != 8 && pixelsize != 16 &&
                  pixelsize != 24 && pixelsize != 32)
                 error("Pixelsize of %d is not one of: 8, 16, 24, 32",
                       pixelsize);) }
   | RESOLUTION fexper ',' fexper
     { ACTION(Eye.view_xres = (int) $<flt>2;
              Eye.view_yres = (int) $<flt>4;) }
   | UP point
     { ACTION(VecCopy($<vec>2, Eye.view_up);) }
   | YON fexper
     { ACTION(Eye.view_yon = $<flt>2;) }
   ;

camera_expers
   : camera_exper
   | camera_expers camera_exper
   ;

camera
   : VIEWPOINT '{' camera_expers '}'
   ;

flare_options
   : flare_options flare_option
   |
   ;

flare_option
   : COLOR expression
      { ACTION(Set_Flare_Color($<exper>2);) }
   | COUNT fexper
      { ACTION(Set_Flare_Count($<flt>2);) }
   | SPACING fexper
      { ACTION(Set_Flare_Spacing($<flt>2);) }
   | SEED fexper
      { ACTION(Set_Flare_Seed((int)$<flt>2);) }
   | SIZE fexper ',' fexper
      { ACTION(Set_Flare_Size($<flt>2, $<flt>4);) }
   | CONCAVE fexper
      { ACTION(Set_Flare_Concave($<flt>2);) }
   | SPHERE fexper
      { ACTION(Set_Flare_Sphere($<flt>2);) }
   ;

light_modifier_decl
   : COLOR expression
      { ACTION(Set_Light_Color($<exper>2);) }
   | SPHERE point ',' fexper
      { ACTION(Translate_Light($<vec>2);
               Set_Light_Radius($<flt>4);) }
   | POLYGON fexper ',' fexper ',' fexper ',' fexper
      { ACTION(Set_Light_Polygon($<flt>2, $<flt>4, $<flt>6, $<flt>8);) }
   | NO_SHADOW
      { ACTION(Set_Light_Shadow(0);) }
   | FLARE
      { ACTION(Create_Lens_Flare();) }
      '{' flare_options '}'
   | transform
      { ACTION(Transform_Light($<trns>1);
               polyray_free($<trns>1);) }
   | ROTATE point
      { ACTION(Rotate_Light($<vec>2);) }
   | ROTATE point ',' fexper
      { ACTION(Rotate_Axis_Light($<vec>2, $<flt>4);) }
   | SHEAR fexper ',' fexper ',' fexper ',' fexper ','
           fexper ',' fexper
      { ACTION(Shear_Light($<flt>2, $<flt>4, $<flt>6,
                           $<flt>8, $<flt>10, $<flt>12);) }
   | TRANSLATE point
      { ACTION(Translate_Light($<vec>2);) }
   | SCALE point
      { ACTION(Scale_Light($<vec>2);) }
   ;

light_modifier_decls
   : light_modifier_decls light_modifier_decl
   |
   ;

depth_light_modifier
   : ANGLE fexper
      { ACTION(DepthLight1($<flt>2);) }
   | ASPECT fexper
      { ACTION(DepthLight2($<flt>2);) }
   | AT point
      { ACTION(DepthLight3($<vec>2);) }
   | COLOR expression
      { ACTION(DepthLight4($<exper>2);) }
   | DEPTH sexper
      { ACTION(DepthLight5($<name>2);
               polyray_free($<name>2);) }
   | FROM point
      { ACTION(DepthLight6($<vec>2);) }
   | HITHER fexper
     { ACTION(DepthLight9($<flt>2);) }
   | UP point
      { ACTION(DepthLight7($<vec>2);) }
   | NO_SHADOW
      { ACTION(Set_Light_Shadow(0);) }
   ;

depth_light_modifiers
   : depth_light_modifiers depth_light_modifier
   |
   ;

haze_statement
   : HAZE fexper ',' fexper ',' point
     { ACTION(haze_action($<flt>2, $<flt>4, $<vec>6);) }
   ;

light
   : LIGHT point ',' point 
     { ACTION($<lgt>$ = light_action1($<vec>2, $<vec>4);) }
   | LIGHT point 
     { ACTION($<lgt>$ = light_action2($<vec>2);) }
   | LIGHT NO_SHADOW point ',' point 
     { ACTION($<lgt>$ = light_action1($<vec>3, $<vec>5);
              Set_Light_Shadow(0);) }
   | LIGHT NO_SHADOW point 
     { ACTION($<lgt>$ = light_action2($<vec>3);
              Set_Light_Shadow(0);) }
   | SPOT_LIGHT point ',' point
     { ACTION($<lgt>$ = SetSpotLight(White, $<vec>2, $<vec>4, 10.0, 30, 45);) }
   | SPOT_LIGHT point ',' point ',' point ',' fexper ',' fexper ',' fexper
     { ACTION($<lgt>$ = SetSpotLight($<vec>2, $<vec>4, $<vec>6, $<flt>8,
                           $<flt>10, $<flt>12);) }
   | SPOT_LIGHT NO_SHADOW point ',' point
     { ACTION($<lgt>$ = SetSpotLight(White, $<vec>3, $<vec>5, 10.0, 30, 45);
              Set_Light_Shadow(0);) }
   | SPOT_LIGHT NO_SHADOW point ',' point ',' point ','
                          fexper ',' fexper ',' fexper
     { ACTION($<lgt>$ = SetSpotLight($<vec>3, $<vec>5, $<vec>7, $<flt>9,
                           $<flt>11, $<flt>13);
              Set_Light_Shadow(0);) }
   | TEXTURED_LIGHT '{'
     { ACTION($<lgt>$ = light_action3();) }
     light_modifier_decls '}'
     { ACTION($<lgt>$ = Current_Light;) }
   | DIRECTIONAL_LIGHT point
     { ACTION($<lgt>$ = light_action4($<vec>2);) }
   | DIRECTIONAL_LIGHT point ',' point
     { ACTION($<lgt>$ = light_action5($<vec>2, $<vec>4);) }
   | DIRECTIONAL_LIGHT NO_SHADOW point
     { ACTION($<lgt>$ = light_action4($<vec>3);
              Set_Light_Shadow(0);) }
   | DIRECTIONAL_LIGHT NO_SHADOW point ',' point
     { ACTION($<lgt>$ = light_action5($<vec>3, $<vec>5);
              Set_Light_Shadow(0);) }
   | DEPTHMAPPED_LIGHT '{'
     { ACTION($<lgt>$ = light_action6();) }
     depth_light_modifiers '}'
     { ACTION(DepthLight8();
              $<lgt>$ = Current_Light;) }
   ;

draw_statement
   : DRAW fexper ',' fexper ',' fexper ',' expression ',' expression
      { ACTION(draw_action($<flt>2, $<flt>4, (int)$<flt>6,
                           $<exper>8, $<exper>10);) }
   | POINT expression ',' expression
      { ACTION(draw_action(0.0, 0.0, 0, $<exper>2, $<exper>4);) }
   ;

background
   : BACKGROUND expression
     { ACTION(background_action($<exper>2);) }
   ;

surface_declaration
   : COLOR expression
      { ACTION(color_action(CurrentSurface, $<exper>2);) }
   | COLOR_MAP '(' map_entries ',' expression ')'
      { ACTION(color_map_action(CurrentSurface, $<cmap_entry>3, $<exper>5);) }
   | COLOR_MAP '(' map_entries ')'
      { ACTION(color_map_action(CurrentSurface, $<cmap_entry>3, NULL);) }
   | AMBIENT expression ',' expression
      { ACTION(ambient_action(CurrentSurface, $<exper>2, $<exper>4);) }
   | AMBIENT expression
      { ACTION(ambient_action(CurrentSurface, NULL, $<exper>2);) }
   | BRILLIANCE expression
      { ACTION(brilliance_action(CurrentSurface, $<exper>2);) }
   | BUMP_SCALE expression
      { ACTION(bump_scale_action(CurrentSurface, $<exper>2);) }
   | DIFFUSE expression ',' expression
      { ACTION(diffuse_action(CurrentSurface, $<exper>2, $<exper>4);) }
   | DIFFUSE expression
      { ACTION(diffuse_action(CurrentSurface, NULL, $<exper>2);) }
   | FREQUENCY expression
      { ACTION(frequency_action(CurrentSurface, $<exper>2);) }
   | LOOKUP_FUNCTION expression
      { ACTION(lookup_function_action(CurrentSurface, $<exper>2);) }
   | MICROFACET PHONG expression
      { ACTION(microfacet_action(CurrentSurface, PHONG, $<exper>3);) }
   | MICROFACET BLINN expression
      { ACTION(microfacet_action(CurrentSurface, BLINN, $<exper>3);) }
   | MICROFACET GAUSSIAN expression
      { ACTION(microfacet_action(CurrentSurface, GAUSSIAN, $<exper>3);) }
   | MICROFACET REITZ expression
      { ACTION(microfacet_action(CurrentSurface, REITZ, $<exper>3);) }
   | MICROFACET COOK expression
      { ACTION(microfacet_action(CurrentSurface, COOK, $<exper>3);) }
   | MICROFACET expression
      { ACTION(microfacet_action(CurrentSurface, PHONG, $<exper>2);) }
   | NORMAL expression
      { ACTION(normal_action(CurrentSurface, $<exper>2);) }
   | POSITION expression
      { ACTION(position_action(CurrentSurface, $<exper>2);) }
   | OCTAVES expression
      { ACTION(octaves_action(CurrentSurface, $<exper>2);) }
   | PHASE expression
      { ACTION(phase_action(CurrentSurface, $<exper>2);) }
   | POSITION_FUNCTION expression
      { ACTION(position_function_action(CurrentSurface, $<exper>2);) }
   | POSITION_SCALE expression
      { ACTION(position_scale_action(CurrentSurface, $<exper>2);) }
   | REFLECTION expression ',' expression
      { ACTION(reflection_action(CurrentSurface, $<exper>2, $<exper>4);) }
   | REFLECTION expression
      { ACTION(reflection_action(CurrentSurface, NULL, $<exper>2);) }
   | SPECULAR expression ',' expression
      { ACTION(specular_action(CurrentSurface, $<exper>2, $<exper>4);) }
   | SPECULAR expression
      { ACTION(specular_action(CurrentSurface, NULL, $<exper>2);) }
   | TRANSMISSION expression ',' expression ',' expression
      { ACTION(transmission_action(CurrentSurface, $<exper>2, $<exper>4,
                                   $<exper>6);) }
   | TRANSMISSION expression ',' expression
      { ACTION(transmission_action(CurrentSurface, NULL,
                                   $<exper>2, $<exper>4);) }
   | TURBULENCE expression
      { ACTION(turbulence_action(CurrentSurface, $<exper>2);) }
   ;

surface_declarations
   : surface_declarations surface_declaration
   |
   ;

surface
   : SURFACE '{'
      { ACTION(surface_action1();) }
     surface_declarations '}'
     { ACTION($<surf>$ = CurrentSurface;) }
   | SURFACE_SYM
      { ACTION(surface_action2($<name>1); $<surf>$ = CurrentSurface;) }
   | SURFACE_SYM '{'
      { ACTION(surface_action2($<name>1);) }
     surface_declarations '}'
      { ACTION($<surf>$ = CurrentSurface;) }
   ;

texture_map_element
   : '[' fexper ',' fexper ',' texture ',' texture ']'
      { ACTION($<text_map>$ =
               make_texture_map_entry($<flt>2, $<flt>4, $<text>6, $<text>8);) }
   ;

texture_map_elements
   : texture_map_element
      { ACTION($<text_map>$ = $<text_map>1;) }
   | texture_map_elements ',' texture_map_element
      { ACTION($<text_map>$ =
               texture_map_action2($<text_map>1, $<text_map>3);) }
   ; 

texture_fn_element
   : expression ',' texture
      { ACTION($<text_fn>$ = make_texture_fn_entry($<exper>1, $<text>3);) }
   ;

texture_fn_elements
   : texture_fn_element
      { ACTION($<text_fn>$ = $<text_fn>1;) }
   | texture_fn_elements ',' texture_fn_element
      { ACTION($<text_fn>$ = texture_fn_action2($<text_fn>1, $<text_fn>3);) }
   ; 

texture_map
   : TEXTURE_MAP_SYM
      { ACTION($<text_map>$ = texture_map_action1($<name>1);) }
   | TEXTURE_MAP '(' texture_map_elements ')'
      { ACTION($<text_map>$ = $<text_map>3;) }
   ;

texture_modifier_decls
   : texture_modifier_decls texture_modifier_decl
   |
   ;

texture_modifier_decl
   : transform
      { ACTION(if (Texture_Stack->element->t_trans == NULL)
                  Texture_Stack->element->t_trans = $<trns>1;
               else {
                  Compose_Transformations(Texture_Stack->element->t_trans,
                                          $<trns>1);
                  polyray_free($<trns>1);
                  }) }
   | ROTATE point
      { ACTION(TextureRotate(Texture_Stack->element, $<vec>2);) }
   | ROTATE point ',' fexper
      { ACTION(TextureAxisRotate(Texture_Stack->element, $<vec>2, $<flt>4);) }
   | SHEAR fexper ',' fexper ',' fexper ',' fexper ','
           fexper ',' fexper
      { ACTION(TextureShear(Texture_Stack->element, $<flt>2, $<flt>4,
                            $<flt>6, $<flt>8, $<flt>10, $<flt>12);) }
   | TRANSLATE point
      { ACTION(TextureTranslate(Texture_Stack->element, $<vec>2);) }
   | SCALE point
      { ACTION(TextureScale(Texture_Stack->element, $<vec>2);) }
   ;

texture_declarations
   : texture_declaration texture_modifier_decls
   ;

texture_declaration
   : surface
      { ACTION(create_plain(Texture_Stack->element, $<surf>1);) }
   | SPECIAL surface
      { ACTION(create_special(Texture_Stack->element, $<surf>2);) }
   | NOISE surface
      { ACTION(create_noise(Texture_Stack->element, $<surf>2);) }
   | CHECKER texture ',' texture
      { ACTION(create_checker(Texture_Stack->element, $<text>2, $<text>4);) }
   | HEXAGON texture ',' texture ',' texture
      { ACTION(create_hexagon(Texture_Stack->element,
                              $<text>2, $<text>4, $<text>6);) }
   | LAYERED texture_list
      { ACTION(create_layered(Texture_Stack->element, $<textlist>2);) }
   | INDEXED expression ',' texture_map
      { ACTION(create_indexed(Texture_Stack->element, $<exper>2,
                              $<text_map>4);) }
   | SUMMED texture_fn_elements
      { ACTION(create_summed(Texture_Stack->element, $<text_fn>2);) }
   ;

texture
   : TEXTURE '{'
      { ACTION(push_texture(texture_action1());) }
     texture_declarations '}'
      { ACTION($<text>$ = pop_texture();) }
   | TEXTURE_SYM
      { ACTION($<text>$ = texture_action2($<name>1);) }
   | TEXTURE_SYM '{'
      { ACTION(push_texture(texture_action2($<name>1));) }
     texture_modifier_decls '}'
      { ACTION($<text>$ = pop_texture();) }
   ;

texture_list
   : texture
      { ACTION($<textlist>$ = texture_list_action1($<text>1);) }
   | texture_list ',' texture
      { ACTION($<textlist>$ = texture_list_action2($<textlist>1, $<text>3);) }
   ;

transform_declaration
   : ROTATE point
      { ACTION(rotate_transform(Current_Transform, $<vec>2);) }
   | ROTATE point ',' fexper
      { ACTION(axis_rotate_transform(Current_Transform, $<vec>2, $<flt>4);) }
   | SCALE point
      { ACTION(scale_transform(Current_Transform, $<vec>2);) }
   | TRANSLATE point
      { ACTION(translate_transform(Current_Transform, $<vec>2);) }
   ;

transform_declarations
   : transform_declaration
   | transform_declarations transform_declaration
   ;

transform
   : TRANSFORM '{'
      { ACTION(Current_Transform = transform_action1();) }
     transform_declarations '}'
      { ACTION($<trns>$ = Current_Transform;) }
   | TRANSFORM_SYM
      { ACTION($<trns>$ = transform_action2($<name>1);) }
   | TRANSFORM_SYM '{'
      { ACTION(Current_Transform = transform_action2($<name>1);) }
     transform_declarations '}'
      { ACTION($<trns>$ = Current_Transform;) }
   ;

bezier_points
   : bezier_points ',' point
      { ACTION($<vecl>$ = add_bezier_point($<vecl>1, $<vec>3);) }
   | point
      { ACTION($<vecl>$ = add_bezier_point(NULL, $<vec>1);) }
   ;

bezier
   : BEZIER fexper ',' fexper ',' fexper ',' fexper ',' bezier_points
      { ACTION(MakeBezier(Object_Stack->element,
                          (int)$<flt>2, $<flt>4, (int)$<flt>6,
                          (int)$<flt>8, $<vecl>10);) }
   ;

blob
   : BLOB fexper ':'
     { ACTION(npoints = 0;) }
     blobelements
     { ACTION(MakeBlob(Object_Stack->element, $<flt>2,
                       blob_components, npoints, 1);
              blob_components = NULL; npoints = 0;) }
   ;

blobelements
   : blobelement
   | blobelements ',' blobelement
   ;

blobelement
   : fexper ',' fexper ',' point
      { ACTION(spherical_component_action($<vec>5, $<flt>1, $<flt>3);) }
   | SPHERE point ',' fexper ',' fexper
      { ACTION(spherical_component_action($<vec>2, $<flt>4, $<flt>6);) }
   | CYLINDER point ',' point ',' fexper ',' fexper
      { ACTION(cylindrical_component_action($<vec>2, $<vec>4,
                                            $<flt>6, $<flt>8);) }
   | PLANE point ',' fexper ',' fexper ',' fexper
      { ACTION(planar_component_action($<vec>2, $<flt>4,
                                       $<flt>6, $<flt>8);) }
   | TORUS point ',' point ',' fexper ',' fexper ',' fexper
      { ACTION(toroidal_component_action($<vec>2, $<vec>4,
                                         $<flt>6, $<flt>8, $<flt>10);) }
   ;

box
   : BOX point ',' point 
      { ACTION(MakeBox(Object_Stack->element, $<vec>2, $<vec>4);) }
   ;

cone
   : CONE point ',' fexper ',' point ',' fexper
      { ACTION(MakeCone(Object_Stack->element, $<vec>2,
                        $<flt>4, $<vec>6, $<flt>8);) }
   ;

csg
   :  { ACTION(ObjectDepth++;) }
     csg_tree
      { ACTION(ObjectDepth--;
               MakeCSG(Object_Stack->element, $<csgtree>2);) }
   ;

csg_tree
   : '(' csg_tree ')'
      { ACTION($<csgtree>$ = $<csgtree>2;) }
   | csg_tree '+' csg_tree
      { ACTION($<csgtree>$ =
                  make_csg_node(T_UNION, $<csgtree>1, $<csgtree>3);) }
   | csg_tree '-' csg_tree
      { ACTION($<csgtree>$ =
                  make_csg_node(T_INTERSECTION, $<csgtree>1,
                                make_csg_node(T_INVERSE, $<csgtree>3, NULL));) }
   | csg_tree '*' csg_tree
      { ACTION($<csgtree>$ =
                  make_csg_node(T_INTERSECTION, $<csgtree>1, $<csgtree>3);) }
   | '~' csg_tree
      { ACTION($<csgtree>$ =
                  make_csg_node(T_INVERSE, $<csgtree>2, NULL);) }
   | csg_tree '&' csg_tree
      { ACTION($<csgtree>$ =
                  make_csg_node(T_CLIP, $<csgtree>1, $<csgtree>3);) }
   | csg_tree '^' csg_tree
      { ACTION($<csgtree>$ =
                  make_csg_node(T_MERGE, $<csgtree>1, $<csgtree>3);) }
   | object
      { ACTION($<csgtree>$ =
                  make_csg_node(T_BASE_OBJECT, $<obj>1, NULL);) }
   ;

cylinder
   : CYLINDER point ',' point ',' fexper
      { ACTION(MakeCylinder(Object_Stack->element,
                            $<vec>2, $<vec>4, $<flt>6);) }
   ;

cylindrical_height_field
   : CHEIGHT_FIELD sexper
      { ACTION(MakeCylHeight(Object_Stack->element, $<name>2, 0,
                             1.0, 128.0);
        polyray_free($<name>2);) }
   | CHEIGHT_FIELD sexper ',' fexper ',' fexper
      { ACTION(MakeCylHeight(Object_Stack->element, $<name>2, 0,
                             $<flt>4, $<flt>6);
        polyray_free($<name>2);) }
   ;

cylindrical_height_fn
   : CHEIGHT_FN fexper ',' fexper ',' expression
      { ACTION(MakeCylHeightFn(Object_Stack->element, $<flt>2, $<flt>4,
                               $<exper>6, 0, 1.0, 128.0);) }
   | CHEIGHT_FN fexper ',' fexper ',' expression ','
                fexper ',' fexper
      { ACTION(MakeCylHeightFn(Object_Stack->element, $<flt>2, $<flt>4,
                               $<exper>6, 0, $<flt>8, $<flt>10);) }
   ;

disc
   : DISC point ',' point ',' fexper
      { ACTION(MakeDisc(Object_Stack->element,
                        $<vec>2, $<vec>4, 0.0, $<flt>6);) }
   | DISC point ',' point ',' fexper ',' fexper
      { ACTION(MakeDisc(Object_Stack->element,
                        $<vec>2, $<vec>4, $<flt>6, $<flt>8);) }
   ;

function
   : FUNCTION expression
      { ACTION(MakeFunction(Object_Stack->element, $<exper>2);) }
   ;

gridded
   : GRIDDED sexper ',' object_list
      { ACTION(MakeGrid(Object_Stack->element, $<name>2, $<objlist>4);
        polyray_free($<name>2);) }
   ;

object_list
   : object
      { ACTION(ostackptr ost =
                     polyray_malloc(sizeof(struct object_stack_struct));
                if (ost == NULL) error("Failed to allocate grid object");
                ost->element = $<obj>1;
                ost->next    = NULL;
                $<objlist>$  = ost;) }
   | object_list object
      { ACTION(ostackptr ost =
                     polyray_malloc(sizeof(struct object_stack_struct));
                if (ost == NULL) error("Failed to allocate grid object");
                ost->element = $<obj>2;
                ost->next    = $<objlist>1;
                $<objlist>$  = ost;) }
   ;

height_field
   : HEIGHT_FIELD sexper
      { ACTION(MakeHeight(Object_Stack->element, $<name>2, 0);
        polyray_free($<name>2);) }
   ;

height_fn
   : HEIGHT_FN fexper ',' fexper ','
               fexper ',' fexper ',' fexper ',' fexper ','
               expression
      { ACTION(MakeHeightFn(Object_Stack->element, (int)$<flt>2, (int)$<flt>4,
                            $<flt>6, $<flt>8, $<flt>10, $<flt>12,
                            $<exper>14, 0);) }
   | HEIGHT_FN fexper ',' fexper ',' expression
      { ACTION(MakeHeightFn(Object_Stack->element, (int)$<flt>2, (int)$<flt>4,
                            0.0, 1.0, 0.0, 1.0,
                            $<exper>6, 0);) }
   ;

hypertexture
   : HYPERTEXTURE expression
      { ACTION(MakeHypertexture(Object_Stack->element, $<exper>2);) }
   ;

lathe
   : LATHE fexper ',' point ',' fexper  ','
   { ACTION(npoints = (int)$<flt>6;
            plist = (fVec *)polyray_malloc((int)$<flt>6 * sizeof(fVec));
            if (plist == NULL) error("Failed to allocate lathe data\n");
            pl = plist;) }
   pointlist 
   { ACTION(MakeRevolve(Object_Stack->element, (int)$<flt>2,
                        $<vec>4, (int)$<flt>6, plist);) }
   ;

light_object
   : light
   { ACTION(MakeLight(Object_Stack->element, $<lgt>1);) }
   ;

nurb
   : NURB fexper ',' fexper ',' fexper ',' fexper ','
          expression ',' expression ',' expression
   { ACTION(MakeNurb(Object_Stack->element, (int)$<flt>2, (int)$<flt>4,
                     (int)$<flt>6, (int)$<flt>8, $<exper>10, $<exper>12,
                     $<exper>14);) }
   | NURB fexper ',' fexper ',' fexper ',' fexper ','
          expression
   { ACTION(MakeNurb(Object_Stack->element, (int)$<flt>2, (int)$<flt>4,
                     (int)$<flt>6, (int)$<flt>8, NULL, NULL,
                     $<exper>10);) }
   ;

parabola
   : PARABOLA point ',' point ',' fexper
      { ACTION(MakeParabola(Object_Stack->element,
                            $<vec>2, $<vec>4, $<flt>6);) }
   ;

parametric
   : PARAMETRIC expression
      { ACTION(MakeParametric(Object_Stack->element, $<exper>2);) }
   ;

polygon:
   POLYGON fexper  ','
   { ACTION(npoints = (int)$<flt>2;
            if (npoints < 3)
               error("polygons must have at least 3 sides\n");
            plist = (fVec *)polyray_malloc((int)$<flt>2 * sizeof(fVec)) ;
            if (plist == NULL) error("Failed to allocate polygon data\n");
            pl = plist;) }
   pointlist 
   { ACTION(MakePoly(Object_Stack->element, (int)$<flt>2, plist);) }
   ;

polynomial
   : POLYNOMIAL expression
      { ACTION(polynomial_action1($<exper>2, 1);) }
   ;

patch_vertex
   : point ',' point
      { ACTION(VecCopy($<vec>1, tri_vertex[npoints].pos);
               VecCopy($<vec>3, tri_vertex[npoints].norm);
               tri_vertex[npoints].u = PLY_HUGE;
               tri_vertex[npoints].v = PLY_HUGE;
               npoints++;) }
   | point ',' point UV fexper ',' fexper
      { ACTION(VecCopy($<vec>1, tri_vertex[npoints].pos);
               VecCopy($<vec>3, tri_vertex[npoints].norm);
               tri_vertex[npoints].u = $<flt>5;
               tri_vertex[npoints].v = $<flt>7;
               npoints++;) }
   ;

ppatch
   : PATCH
      { ACTION(npoints = 0;) }
     patch_vertex ',' patch_vertex ',' patch_vertex
      { ACTION(MakeTri(Object_Stack->element, tri_vertex);) }
   ;

raw
   : RAW sexper
      { ACTION(MakeRaw(Object_Stack->element, $<name>2, 0.0);
        polyray_free($<name>2);) }
   | RAW sexper ',' fexper
      { ACTION(MakeRaw(Object_Stack->element, $<name>2, $<flt>4);
        polyray_free($<name>2);) }
   ;

smooth_height_field
   : SMOOTH_HEIGHT_FIELD sexper
      { ACTION(MakeHeight(Object_Stack->element, $<name>2, 1);
        polyray_free($<name>2);) }
   ;

smooth_height_fn
   : SMOOTH_HEIGHT_FN fexper ',' fexper ','
               fexper ',' fexper ',' fexper ',' fexper ','
               expression
      { ACTION(MakeHeightFn(Object_Stack->element, (int)$<flt>2, (int)$<flt>4,
                            $<flt>6, $<flt>8, $<flt>10, $<flt>12,
                            $<exper>14, 1);) }
   | SMOOTH_HEIGHT_FN fexper ',' fexper ',' expression
      { ACTION(MakeHeightFn(Object_Stack->element, (int)$<flt>2, (int)$<flt>4,
                            0.0, 1.0, 0.0, 1.0,
                            $<exper>6, 1);) }
   ;

smooth_cheight_field
   : SMOOTH_CHEIGHT_FIELD sexper 
      { ACTION(MakeCylHeight(Object_Stack->element, $<name>2, 1, 1.0, 128.0);
        polyray_free($<name>2);) }
   | SMOOTH_CHEIGHT_FIELD sexper ',' fexper ',' fexper
      { ACTION(MakeCylHeight(Object_Stack->element, $<name>2, 1,
                             $<flt>4, $<flt>6);
        polyray_free($<name>2);) }
   ;

smooth_cheight_fn
   : SMOOTH_CHEIGHT_FN fexper ',' fexper ',' expression
      { ACTION(MakeCylHeightFn(Object_Stack->element, $<flt>2, $<flt>4,
                               $<exper>6, 1, 1.0, 128.0);) }
   | SMOOTH_CHEIGHT_FN fexper ',' fexper ',' expression ',' fexper ',' fexper
      { ACTION(MakeCylHeightFn(Object_Stack->element, $<flt>2, $<flt>4,
                               $<exper>6, 1, $<flt>8, $<flt>10);) }
   ;

smooth_sheight_field
   : SMOOTH_SHEIGHT_FIELD sexper
      { ACTION(MakeSphHeight(Object_Stack->element, $<name>2, 0,
                             1.0, 128.0);
        polyray_free($<name>2);) }
   | SMOOTH_SHEIGHT_FIELD sexper ',' fexper ',' fexper
      { ACTION(MakeSphHeight(Object_Stack->element, $<name>2, 1,
                             $<flt>4, $<flt>6);
        polyray_free($<name>2);) }
   ;

smooth_sheight_fn
   : SMOOTH_SHEIGHT_FN fexper ',' fexper ',' expression
      { ACTION(MakeSphHeightFn(Object_Stack->element, $<flt>2, $<flt>4,
                               $<exper>6, 1, 1.0, 128.0);) }
   | SMOOTH_SHEIGHT_FN fexper ',' fexper ',' expression ',' fexper ',' fexper
      { ACTION(MakeSphHeightFn(Object_Stack->element, $<flt>2, $<flt>4,
                               $<exper>6, 1, $<flt>8, $<flt>10);) }
   ;

sphere
   : SPHERE point ',' fexper 
      { ACTION(MakeSphere(Object_Stack->element,
                          $<vec>2, $<flt>4);) }
   ;

spherical_height_field
   : SHEIGHT_FIELD sexper
      { ACTION(MakeSphHeight(Object_Stack->element, $<name>2, 0, 1.0, 128.0);
        polyray_free($<name>2);) }
   | SHEIGHT_FIELD sexper ',' fexper ',' fexper
      { ACTION(MakeSphHeight(Object_Stack->element, $<name>2, 0,
                             $<flt>4, $<flt>6);
        polyray_free($<name>2);) }
   ;

spherical_height_fn
   : SHEIGHT_FN fexper ',' fexper ',' expression
      { ACTION(MakeSphHeightFn(Object_Stack->element, $<flt>2, $<flt>4,
                               $<exper>6, 0, 1.0, 128.0);) }
   | SHEIGHT_FN fexper ',' fexper ',' expression ',' fexper ',' fexper
      { ACTION(MakeSphHeightFn(Object_Stack->element, $<flt>2, $<flt>4,
                               $<exper>6, 0, $<flt>8, $<flt>10);) }
   ;

superq
   : SUPERQ fexper ',' fexper
      { ACTION(MakeSuperq(Object_Stack->element, $<flt>2, $<flt>4);) }
   ;

contour
   : CONTOUR fexper  ','
      { ACTION(npoints = (int)$<flt>2;
               if (npoints < 2)
                  error("contours must have at least 3 sides\n");
               plist = (fVec *)polyray_malloc(((int)$<flt>2 + 1) * sizeof(fVec));
               if (plist == NULL) error("Failed to allocate contour data\n");
               pl = plist;) }
     pointlist 
      { ACTION(if (gcount == 0) error("Too many contours for the glyph\n");
               cl->count = (int)$<flt>2;
               cl->points = plist;
               gcount--; cl++;) }
   ;

glyph_contours
   : contour
   | glyph_contours contour
   ;

glyph
   : GLYPH fexper
      { ACTION(gcount = (int)$<flt>2;
               if (gcount < 1)
                  error("Glyphs must have at least one contour");
               contours = polyray_malloc(gcount * sizeof(Contour));
               if (contours == NULL)
                  error("Failed to allocate glyph data");
               cl = contours;) }
     glyph_contours
      { ACTION(if (gcount != 0)
                  error("Wrong number of contours in glyph\n");
               MakeGlyph(Object_Stack->element, (int)$<flt>2, contours);) }
   ;

sweep
   : SWEEP fexper  ',' point ',' fexper ','
   { ACTION(npoints = (int)$<flt>6;
            plist = (fVec *)polyray_malloc((int)$<flt>6 * sizeof(fVec));
            if (plist == NULL) error("Failed to allocate sweep data\n");
            pl = plist;) }
   pointlist 
   { ACTION(MakeSweep(Object_Stack->element, (int)$<flt>2,
                      $<vec>4, (int)$<flt>6, plist);) }
   ;

torus
   : TORUS fexper ',' fexper ',' point ',' point 
      { ACTION(MakeTorus(Object_Stack->element, $<flt>2, $<flt>4,
                         $<vec>6, $<vec>8);) }
   ;

fexper
   : expression
      { ACTION(Flt ftmp; Vec vtmp; NODE_PTR tnode;
               if (eval_node(NULL, $<exper>1, &ftmp, vtmp, &tnode) == 1) {
                  deallocate_node($<exper>1);
                  $<flt>$ = ftmp;
                  }
               else {
                  error("Bad fexper expression\n");
                  }) }
   ;

point
   : expression
      { ACTION(Flt ftmp; Vec vtmp; NODE_PTR tnode;
               if (eval_node(NULL, $<exper>1, &ftmp, vtmp, &tnode) == 2) {
                  VecCopy(vtmp, $<vec>$);
                  deallocate_node($<exper>1);
                  }
               else {
                  error("Bad point expression\n");
                  }) }
   ;

sexper
   : expression
      { ACTION(char *stmp;
               if (create_string($<exper>1, &stmp)) {
                  deallocate_node($<exper>1);
                  $<name>$ = stmp;
                  }
               else {
                  error("Bad sexper expression\n");
                  }) }
   ;

pointlist
   : point
     { ACTION(if (npoints==0) error("Too many points for the polygon\n");
              VecCopy($<vec>1, (*pl)); npoints--; pl++;) }
   | pointlist ',' point
     { ACTION(if (npoints==0) error("Too many points for the polygon\n");
              VecCopy($<vec>3, (*pl)); npoints--; pl++;) }
   ;

expression
   : '(' expression ')'
      { ACTION($<exper>$ = $<exper>2;) }
   | '[' expression_list ']'
      { ACTION($<exper>$ = make_array_node($<elist>2);) }
   | '<' expression ',' expression '>'
      { ACTION($<exper>$ = make_vector3_node($<exper>2, $<exper>4,
                                             make_value_node(0.0));) }
   | '<' expression ',' expression ',' expression '>'
      { ACTION($<exper>$ = make_vector3_node($<exper>2, $<exper>4,
                                             $<exper>6);) }
   | '<' expression ',' expression ',' expression ',' expression '>'
      { ACTION($<exper>$ = make_vector4_node($<exper>2, $<exper>4,
                                             $<exper>6, $<exper>8);) }
   | expression '[' expression ']'
      { ACTION($<exper>$ = make_node(SUBSCRIPT_EXPER, $<exper>1, $<exper>3);) }
   | '(' conditional '?' expression ':' expression ')'
      { ACTION($<exper>$ = make_cond_node($<exper>2, $<exper>4, $<exper>6);) }
   | expression '^' expression
      { ACTION($<exper>$ = make_node(POWER_EXPER, $<exper>1, $<exper>3);) }
   | expression '%' expression
      { ACTION($<exper>$ = make_node(FMOD, $<exper>1, $<exper>3);) }
   | expression '*' expression
      { ACTION($<exper>$ = make_node(TIMES_EXPER, $<exper>1, $<exper>3);) }
   | expression '.' expression
      { $<exper>$ = make_node(DOT_EXPER, $<exper>1, $<exper>3); }
   | expression '/' expression
      { ACTION($<exper>$ = make_node(DIV_EXPER, $<exper>1, $<exper>3);) }
   | expression '+' expression
      { ACTION($<exper>$ = make_node(PLUS_EXPER, $<exper>1, $<exper>3);) }
   | expression '-' expression
      { ACTION($<exper>$ = make_node(MINUS_EXPER, $<exper>1, $<exper>3);) }
   | '-' expression %prec UMINUS
      { ACTION($<exper>$ = make_node(TIMES_EXPER, make_value_node(-1.0),
                                     $<exper>2);)}
   | '|' expression '|'
      { ACTION($<exper>$ = make_fn1_node(FABS, $<exper>2);) }
   | COLOR_MAP '(' map_entries ',' expression ')'
      { ACTION($<exper>$ = make_cmap_node($<cmap_entry>3, $<exper>5);) }
   | COLOR_MAP '(' map_entries ')'
      { ACTION($<exper>$ = make_cmap_node($<cmap_entry>3, NULL);) }
   | NOISE '(' expression ')'
      { ACTION($<exper>$ = make_node(NOISE, $<exper>3, NULL);) }
   | NOISE '(' expression ',' expression ')'
      { ACTION($<exper>$ = make_node(NOISE, $<exper>3, $<exper>5);) }
   | ROTATE '(' expression ',' expression ')'
      { ACTION($<exper>$ = make_fn3_node(ROTATE, $<exper>3, $<exper>5, NULL);) }
   | ROTATE '(' expression ',' expression ',' expression ')'
      { ACTION($<exper>$ = make_fn3_node(ROTATE, $<exper>3, $<exper>5,
                                         $<exper>7);) }
   | COLOR
      { ACTION($<exper>$ = make_node(COLOR, NULL, NULL);) }
   | FRAME
      { ACTION($<exper>$ = make_node(FRAME, NULL, NULL);) }
   | END_FRAME
      { ACTION($<exper>$ = make_node(END_FRAME, NULL, NULL);) }
   | START_FRAME
      { ACTION($<exper>$ = make_node(START_FRAME, NULL, NULL);) }
   | TOTAL_FRAMES
      { ACTION($<exper>$ = make_node(TOTAL_FRAMES, NULL, NULL);) }
   | TOKEN '(' expression_list ')'
      { ACTION($<exper>$ = check_term($<name>1, $<elist>3);)
        polyray_free($<name>1); }
   | TOKEN
      { ACTION($<exper>$ = check_term0($<name>1);)
        polyray_free($<name>1); }
   | NUM
      { ACTION($<exper>$ = make_value_node($<flt>1);) }
   | STRING
      { ACTION($<exper>$ = make_string_node($<name>1);)
               polyray_free($<name>1); }
   | EXPRESSION_SYM
      { ACTION($<exper>$ = exper_action($<name>1);) }
   ;

expression_list
   : expression
      { ACTION($<elist>$ = make_list_node($<exper>1);) }
   | expression_list ',' expression
      { ACTION($<elist>$ = expression_action1($<elist>1, $<exper>3);) }
   ;

conditional
   : '(' conditional ')'
      { ACTION($<exper>$ = $<exper>2;) }
   | expression '<' expression
      { ACTION($<exper>$ = make_node(LESS_EXPER, $<exper>1, $<exper>3);) }
   | expression '>' expression
      { ACTION($<exper>$ = make_node(GREATER_EXPER, $<exper>1, $<exper>3);) }
   | expression LTEQ_SYM expression
      { ACTION($<exper>$ = make_node(LTEQ_EXPER, $<exper>1, $<exper>3);) }
   | expression GTEQ_SYM expression
      { ACTION($<exper>$ = make_node(GTEQ_EXPER, $<exper>1, $<exper>3);) }
   | expression EQUAL_SYM expression
      { ACTION($<exper>$ = make_node(EQUAL_EXPER, $<exper>1, $<exper>3);) }
   | conditional AND_SYM conditional
      { ACTION($<exper>$ = make_node(AND_EXPER, $<exper>1, $<exper>3);) }
   | conditional OR_SYM conditional
      { ACTION($<exper>$ = make_node(OR_EXPER, $<exper>1, $<exper>3);) }
   | '!' conditional
      { ACTION($<exper>$ = make_node(NOT_EXPER, $<exper>1, NULL);) }
   ;

map_entry
   : '[' fexper ',' fexper ',' point ',' point ']'
      { ACTION($<cmap_entry>$ =
                 map_entry_action1($<flt>2, $<flt>4,
                                   $<vec>6, 0.0, $<vec>8, 0.0);) }
   | '[' fexper ',' fexper ',' point ',' fexper ',' point ',' fexper ']'
      { ACTION($<cmap_entry>$ =
                 map_entry_action1($<flt>2, $<flt>4,
                                   $<vec>6, $<flt>8,
                                   $<vec>10, $<flt>12);) }
   ;

map_entries
   : map_entry
      { ACTION($<cmap_entry>$ = $<cmap_entry>1;) }
   | map_entries map_entry
      { ACTION($<cmap_entry>$ = map_entry_action2($<cmap_entry>1,
                                                  $<cmap_entry>2);) }
   ;

frame_decl
   : end_frame_decl
   | start_frame_decl
   | total_frames_decl
   | frame_time_decl
   ; 

end_frame_decl
   : END_FRAME fexper
      { ACTION(end_frame = (int)$<flt>2;) }
   ;

start_frame_decl
   : START_FRAME fexper
      { ACTION(start_frame = (int)$<flt>2;
               if (!Parsed_Flag) current_frame = start_frame;) }
   ;

total_frames_decl
   : TOTAL_FRAMES fexper
      { ACTION(total_frames = (int)$<flt>2;) }
   ;

frame_time_decl
   : FRAME_TIME fexper
      { ACTION(frame_time = $<flt>2;) }
   ;

outfile
   : OUTFILE TOKEN
      { ACTION(if (!Parsed_Flag) {
                  strcpy(outfilebase, $<name>2);
                  filebaseflag = 1;
                  })
        polyray_free($<name>2); }
   | OUTFILE STRING
      { ACTION(if (!Parsed_Flag) {
                  strcpy(outfilebase, $<name>2);
                  filebaseflag = 1;
                  })
        polyray_free($<name>2); }
   ;

flush_statement
   : FILE_FLUSH fexper
      { ACTION(flush_action((int)$<flt>2);) }
   ;

system_call
   : SYSTEM '(' expression_list ')'
      { ACTION(evaluate_system_call($<elist>3);) }
   ;

statement
   : '{' elementlist '}'
   | element
   ;

if_else_part
   : ELSE
     { condition_flags[condition_depth] = 1-condition_flags[condition_depth]; }
     statement
   |
   ;

if_statement
   : IF '(' conditional ')'
      { Flt ftmp; Vec vtmp; NODE_PTR tnode;
        if (check_condition()) {
           if (eval_node(NULL, $<exper>3, &ftmp, vtmp, &tnode) == 1 &&
               ftmp != 0.0) {
              condition_flags[++condition_depth] = 1;
              }
           else
              condition_flags[++condition_depth] = 0;
           deallocate_node($<exper>3);
           }
        else
           condition_flags[++condition_depth] = 0; 
     }
     statement if_else_part
     { condition_depth--; }
   ;

%%
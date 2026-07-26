/* texture.cc

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
#include "defs3.h"
#include "io_ply.h"
#include "memory.h"
#include "polyray.tab.h"
#include "light.h"
#include "builder.h"
#include "eval.h"
#include "mfacet.h"
#include "runtime_state.h"
#include "symtab.h"
#include "vector.h"
#include "shade.h"
#include "texture.h"
#include "factory.h"

/* Deallocate all components of a special surface */
/** @brief Release all expression nodes owned by a parsed special surface.
 *  @param spec Special surface description to destroy.
 *  @return No return value.
 */
static void
delete_special0(Special_Surface *spec)
{
   /* Delete each component of the special surface */
   deallocate_node(spec->body_color);
   deallocate_node(spec->normal);
   deallocate_node(spec->position);
   deallocate_node(spec->Ka_color);
   deallocate_node(spec->Ka_scale);
   deallocate_node(spec->Kb_power);
   deallocate_node(spec->Kd_color);
   deallocate_node(spec->Kd_scale);
   deallocate_node(spec->Ks_color);
   deallocate_node(spec->Ks_scale);
   deallocate_node(spec->Kr_color);
   deallocate_node(spec->Kr_scale);
   deallocate_node(spec->Kt_color);
   deallocate_node(spec->Kt_scale);
   deallocate_node(spec->D_angle);
   deallocate_node(spec->ior);
   deallocate_node(spec->Position_fn);
   deallocate_node(spec->Pos_scale);
   deallocate_node(spec->Lookup_fn);
   deallocate_node(spec->Turbulence);
   deallocate_node(spec->Octaves);
   deallocate_node(spec->Frequency);
   deallocate_node(spec->Phase);
   deallocate_node(spec->Bump_scale);
   //polyray_free(spec);
   delete spec;
}

/* Evaluate each component of the special surface, placing
   the results into "surf". */
/** @brief Evaluate a special-surface description into a concrete render surface.
 *  @param subst   Substitution environment used for expression evaluation.
 *  @param insurf  Parsed special-surface definition.
 *  @param outsurf Output surface populated in place.
 *  @return No return value.
 */
static void
generate_surface(SUBST_PTR subst, Special_Surface *insurf, Surface *outsurf)
{
   #ifdef DEBUG_FN_CALLS
   smessage("texture::generate_surface");
   #endif
   NODE_PTR tmp;
   Flt ftemp = 0.17;
   Flt Kt_scale = 0.0;
   Vec body, tvec, vvec;
   int i;

   /* Determine the default color, for use when others are not defined.
      This is typically used so that calculations of ambient and diffuse
      colors do not have to be repeated. */
   if (insurf->body_color == nullptr)
      MakeVector(1.0, 1.0, 1.0, body)
   else if (eval_node(subst, insurf->body_color, &Kt_scale, body, &tmp) != 2) {
      serror("Bad vector in generate_surface\n");
      return;
   }

   /* Copy body color into PT and the alpha value into w */
   if (subst != nullptr) {
      VecCopy(body, subst->PT);
      subst->U[2] = Kt_scale;
      }

   /* Determining the ambient characteristics */
   if (insurf->Ka_color == nullptr)
      VecCopy(body, outsurf->Ka_color)
   else if (eval_node(subst, insurf->Ka_color,
                      &ftemp, vvec, &tmp) != 2) {
      serror("Bad Ka vector in generate_surface\n");
      return;
   }
   else
      VecCopy(vvec, outsurf->Ka_color)

   if (insurf->Ka_scale == nullptr)
      outsurf->Ka_scale = 0.1;
   else if (eval_node(subst, insurf->Ka_scale,
                      &ftemp, tvec, &tmp) != 1) {
      serror("Bad Ka scale in generate_surface\n");
      return;
      }
   else
      outsurf->Ka_scale = ftemp;

   /* Determine the brilliance power */
   if (insurf->Kb_power == nullptr)
      outsurf->Kb_power = 1.0;
   else if (eval_node(subst, insurf->Kb_power,
                      &ftemp, tvec, &tmp) != 1) {
      serror("Bad brilliance value in generate_surface\n");
      return;
   }
   else if (ftemp < 0.0) {
      swarning("Brilliance values less than 0 not allowed (reset to 0)\n");
      outsurf->Kb_power = 1.0;
      }
   else
      outsurf->Kb_power = ftemp;

   /* Determine the diffuse characteristics */
   if (insurf->Kd_color == nullptr)
      VecCopy(body, outsurf->Kd_color)
   else if (eval_node(subst, insurf->Kd_color, &ftemp,
                      vvec, &tmp) != 2) {
      serror("Bad Kd vector in generate_surface\n");
      return;
   }
   else
      VecCopy(vvec, outsurf->Kd_color)
   if (insurf->Kd_scale == nullptr)
      outsurf->Kd_scale = 0.8;
   else if (eval_node(subst, insurf->Kd_scale,
                      &ftemp, tvec, &tmp) != 1) {
      serror("Bad scale in generate_surface\n");
      return;
   }
   else
      outsurf->Kd_scale = ftemp;

   /* Determine the specular characteristics */
   if (insurf->Ks_color == nullptr)
      VecCopy(body, outsurf->Ks_color)
   else if (eval_node(subst, insurf->Ks_color,
                      &ftemp, vvec, &tmp) != 2) {
      serror("Bad Ks vector in generate_surface\n");
      return;
   }
   else
      VecCopy(vvec, outsurf->Ks_color)

   if (insurf->Ks_scale == nullptr)
      outsurf->Ks_scale = 0.0;
   else if (eval_node(subst, insurf->Ks_scale,
                      &ftemp, tvec, &tmp) != 1) {
      serror("Bad Ks scale in generate_surface\n");
      return;
   }
   else
      outsurf->Ks_scale = ftemp;

   /* Determine the reflection characteristics */
   if (insurf->Kr_color == nullptr)
      VecCopy(body, outsurf->Kr_color)
   else if (eval_node(subst, insurf->Kr_color,
                      &ftemp, vvec, &tmp) != 2) {
      serror("Bad vector in generate_surface\n");
      return;
   }
   else
      VecCopy(vvec, outsurf->Kr_color)
   if (insurf->Kr_scale == nullptr)
      outsurf->Kr_scale = 0.0;
   else if (eval_node(subst, insurf->Kr_scale,
                      &ftemp, tvec, &tmp) != 1) {
      serror("Bad Kr scale in generate_surface\n");
      return;
   }
   else
      outsurf->Kr_scale = ftemp;

   /* Determine the transmission characteristics */
   if (insurf->Kt_color == nullptr) {
#if 0
      if (Kt_scale > 0.0)
         /* If there was no transmission filter and an alpha value came
            from a color map, then assume the filter color is white. */
         MakeVector(1.0, 1.0, 1.0, outsurf->Kt_color)
      else
#endif
         /* Otherwise use the body color as the filter */
         VecCopy(body, outsurf->Kt_color)
      }
   else if (eval_node(subst, insurf->Kt_color, &ftemp,
                      vvec, &tmp) != 2) {
      serror("Bad Kt vector in generate_surface\n");
      return;
   }
   else
      VecCopy(vvec, outsurf->Kt_color)

   if (insurf->Kt_scale == nullptr)
      /* Either we got an alpha value from a color map or the scale
         is 0.  */
      outsurf->Kt_scale = Kt_scale;
   else if ((i = eval_node(subst, insurf->Kt_scale,
                           &ftemp, tvec, &tmp)) != 1) {
      if (i == 2) {
         /* We had a color rather than a scale */
         outsurf->Kt_scale = Kt_scale;
         VecCopy(tvec, outsurf->Kt_color)
         }
      else {
         serror("Bad Kt scale in generate_surface\n");
         return;
      }
      }
   else
      outsurf->Kt_scale = ftemp;

   /* Determine the index of refraction */
   if (insurf->ior == nullptr)
      outsurf->ior = 1.0;
   else if (eval_node(subst, insurf->ior, &ftemp, tvec, &tmp) != 1) {
      serror("Bad ior in generate_surface\n");
      return;
   }
   else
      outsurf->ior = ftemp;

   /* Determine the microfacet distribution */
   if (insurf->D_angle == nullptr)
      ftemp = 10.0;
   else if (eval_node(subst, insurf->D_angle, &ftemp, tvec, &tmp) != 1) {
      serror("Bad specular angle in generate_surface\n");
      return;
   }

   ftemp = PYM_PI * ftemp / 180.0;
   switch (insurf->D_type) {
   case PHONG:
       outsurf->D       = D_Phong;
       outsurf->D_coeff = D_Phong_Init(ftemp);
       break;
   case BLINN:
       outsurf->D       = D_Blinn;
       outsurf->D_coeff = D_Blinn_Init(ftemp);
       break;
   case GAUSSIAN:
       outsurf->D       = D_Gaussian;
       outsurf->D_coeff = D_Gaussian_Init(ftemp);
       break;
   case REITZ:
       outsurf->D       = D_Reitz;
       outsurf->D_coeff = D_Reitz_Init(ftemp);
       break;
   case COOK:
       outsurf->D       = D_Cook;
       outsurf->D_coeff = D_Cook_Init(ftemp);
       break;
   default:
      serror("Bad microfacet type in 'generate_surface'\n");
      return;
   }
}

/* Evaluate each component of the special surface, placing
   the results into "surf". */
/** @brief Build a noise-surface definition from a parsed special surface.
 *  @param insurf  Parsed special-surface definition.
 *  @param outsurf Output noise surface populated in place.
 *  @return No return value.
 */
static void
generate_noise_surface(Special_Surface *insurf, Noise_Surface *outsurf)
{
   NODE_PTR tmp;
   Flt ftemp;
   Vec tvec;
   Surface *local_surf = &outsurf->surf;

   generate_surface(nullptr, insurf, local_surf);

   if (insurf->body_color == nullptr)
      MakeVector(1.0, 1.0, 1.0, outsurf->body_color)
   else if (eval_node(nullptr, insurf->body_color, &ftemp,
                      outsurf->body_color, &tmp) != 2) {
      serror("Bad color vector in generate_noise_surface\n");
      return;
   }


   /* Determine the lookup function */
   if (insurf->Pos_scale == nullptr)
      outsurf->Pos_scale = 1.0;
   else if (eval_node(nullptr, insurf->Pos_scale,
                      &ftemp, tvec, &tmp) != 1) {
      serror("Bad position scaling value generate_noise_surface\n");
      return;
   }
   else
      outsurf->Pos_scale = ftemp;

   /* Determine the lookup function */
   if (insurf->Lookup_fn == nullptr)
      outsurf->Lookup_fn = 0;
   else if (eval_node(nullptr, insurf->Lookup_fn, &ftemp, tvec, &tmp) != 1 ||
            ftemp < 0.0 || ftemp > 4.0) {
      serror("Bad lookup function generate_noise_surface\n");
      return;
   }
   else
      outsurf->Lookup_fn = (int)ftemp;

   /* Determine the normal modifier function */
   if (insurf->normal == nullptr)
      outsurf->N_modifier = 0;
   else if (eval_node(nullptr, insurf->normal, &ftemp, tvec, &tmp) != 1 ||
            ftemp < 0.0 || ftemp > 3.0) {
      serror("Bad normal modifier function generate_noise_surface\n");
      return;
   }
   else
      outsurf->N_modifier = (int)ftemp;

   /* Octaves of noise to use */
   if (insurf->Octaves == nullptr)
      outsurf->Octaves = 1;
   else if (eval_node(nullptr, insurf->Octaves, &ftemp, tvec, &tmp) != 1 ||
            ftemp < 1.0) {
      serror("Bad Octaves value in generate_noise_surface\n");
      return;
   }
   else
      outsurf->Octaves = (int)ftemp;

   /* Frequency of ripple/wave */
   if (insurf->Frequency == nullptr)
      outsurf->Frequency = 1.0;
   else if (eval_node(nullptr, insurf->Frequency, &ftemp, tvec, &tmp) != 1 ||
            ftemp <= 0.0) {
      serror("Bad Frequency value in generate_noise_surface\n");
      return;
   }
   else
      outsurf->Frequency = ftemp;

   /* Phase offset for ripples */
   if (insurf->Phase == nullptr)
      outsurf->Phase = 0.0;
   else if (eval_node(nullptr, insurf->Phase, &ftemp, tvec, &tmp) != 1) {
      serror("Bad Phase value in generate_noise_surface\n");
      return;
   }
   else
      outsurf->Phase = ftemp;

   /* Amount of contribution of normal modifier */
   if (insurf->Bump_scale == nullptr)
      outsurf->Bump_scale = 1.0;
   else if (eval_node(nullptr, insurf->Bump_scale, &ftemp, tvec, &tmp) != 1) {
      serror("Bad Bump_scale value in generate_noise_surface\n");
      return;
   }
   else
      outsurf->Bump_scale = ftemp;

   /* Determine the position modifier function */
   if (insurf->Position_fn == nullptr)
      outsurf->Position_fn = 0;
   else if (eval_node(nullptr, insurf->Position_fn, &ftemp, tvec, &tmp) != 1 ||
            ftemp < 0.0 || ftemp > 5.0) {
      serror("Bad position modifier function in generate_noise_surface\n");
      return;
   }
   else
      outsurf->Position_fn = (int)ftemp;

   /* Amount of turbulence */
   if (insurf->Turbulence == nullptr)
      outsurf->Turbulence = 0.0;
   else if (eval_node(nullptr, insurf->Turbulence,
                      &ftemp, tvec, &tmp) != 1) {
      serror("Bad Turbulence value generate_noise_surface\n");
      return;
   }
   else
      outsurf->Turbulence = ftemp;

   outsurf->map = insurf->map;
}

/** @brief Evaluate a plain texture and return its stored surface.
 *  @param Eye   Active viewpoint.
 *  @param obj   Object being shaded.
 *  @param text  Texture instance being evaluated.
 *  @param W     World-space point.
 *  @param P     Object/texture-space point.
 *  @param N     Surface normal.
 *  @param I     Incoming ray direction.
 *  @param u     Texture u coordinate.
 *  @param v     Texture v coordinate.
 *  @param level Recursion/shading level.
 *  @return Pointer to the surface data owned by the texture.
 */
static Surface *
eval_plain(Viewpoint *Eye, Object *obj, Texture *text,
           Vec W, Vec P, Vec N, Vec I,
           float u, float v, int level)
{
   #ifdef DEBUG_FN_CALLS
   smessage("texture::eval_plain\n");
   #endif
   return (Surface *)(text->data);
}

/** @brief Free the resources owned by a plain texture.
 *  @param text Plain texture to destroy.
 *  @return No return value.
 */
static void
delete_plain(Texture *text)
{
   if ((Surface *)text->data != &RuntimeState::DefaultSurface)
      polyray_free(text->data);
}

/** @brief Initialize a plain texture from a special-surface description.
 *  @param texture Texture object to initialize.
 *  @param surf    Parsed special-surface description to consume.
 *  @return No return value.
 */
void
create_plain(Texture *texture, Special_Surface *surf)
{
   #ifdef DEBUG_FN_CALLS
   smessage("texture::create_plain\n");
   #endif
   /* Have to be able to evaluate all of the components of the parsed
      surface, and generate predefined surface values. */
   Surface *new_surf = (Surface *)polyray_malloc(sizeof(Surface));
   generate_surface(nullptr, surf, new_surf);
   delete_special0(surf);
   texture->type   = ShapeType::Plain;
   texture->eval   = eval_plain;
   texture->del    = delete_plain;
   texture->data   = new_surf;
}

/** @brief Evaluate a checker texture by dispatching to one of its child textures.
 *  @param Eye   Active viewpoint.
 *  @param obj   Object being shaded.
 *  @param text  Checker texture instance.
 *  @param W     World-space point.
 *  @param P     Object/texture-space point.
 *  @param N     Surface normal.
 *  @param I     Incoming ray direction.
 *  @param u     Texture u coordinate.
 *  @param v     Texture v coordinate.
 *  @param level Recursion/shading level.
 *  @return Surface returned by the selected child texture.
 */
static Surface *
eval_checker(Viewpoint *Eye, Object *obj, Texture *text,
             Vec W, Vec P, Vec N, Vec I,
             float u, float v, int level)
{
   int temp;
   Vec VP, VW;
   Checker *check = (Checker*)text->data;

   if (text->t_trans != nullptr) {
      /* Transform the point into texture space */
      InvTxVector1(VP, P, text->t_trans);
      InvTxVector1(VW, W, text->t_trans);
      }
   else {
      VecCopy(P, VP);
      VecCopy(W, VW);
      }
   temp = (int)floor(VP[0]-PLY_EPSILON) + (int)floor(VP[1]-PLY_EPSILON) +
          (int)floor(VP[2]-PLY_EPSILON);
/*
   if (check->repeat_flag) {
      VP[0] -= floor(VP[0]);
      VP[1] -= floor(VP[1]);
      VP[2] -= floor(VP[2]);
      }
*/
   if (temp & 1)
      return (check->text1->eval)(Eye, obj, check->text1, VW, VP, N, I, u, v, level);
   else
      return (check->text2->eval)(Eye, obj, check->text2, VW, VP, N, I, u, v, level);
}

/** @brief Free the resources owned by a checker texture.
 *  @param texture Checker texture to destroy.
 *  @return No return value.
 */
static void
delete_checker(Texture *texture)
{
   Checker *check = (Checker*)texture->data;
   TextureDelete(check->text1);
   TextureDelete(check->text2);
   polyray_free(check);
}

/** @brief Initialize a checker texture from two child textures.
 *  @param texture Texture object to initialize.
 *  @param text1   First child texture.
 *  @param text2   Second child texture.
 *  @return No return value.
 */
void
create_checker(Texture *texture, Texture *text1, Texture *text2)
{
   Checker *check = (Checker*)polyray_malloc(sizeof(Checker));
   if (check == nullptr) {
      serror("Failed to allocate a checker texture\n");
      return;
   }
   /* check->repeat_flag = Flag; */
   check->text1 = text1;
   check->text2 = text2;
   check->repeat_flag1 = 0;
   check->repeat_flag2 = 0;
   texture->type   = ShapeType::Checker;
   texture->eval   = eval_checker;
   texture->del    = delete_checker;
   texture->data   = check;
}

/** @brief Evaluate a hexagon texture by dispatching to one of three child textures.
 *  @param Eye   Active viewpoint.
 *  @param obj   Object being shaded.
 *  @param text  Hexagon texture instance.
 *  @param W     World-space point.
 *  @param P     Object/texture-space point.
 *  @param N     Surface normal.
 *  @param I     Incoming ray direction.
 *  @param u     Texture u coordinate.
 *  @param v     Texture v coordinate.
 *  @param level Recursion/shading level.
 *  @return Surface returned by the selected child texture.
 */
static Surface *
eval_hexagon(Viewpoint *Eye, Object *obj, Texture *text,
             Vec W, Vec P, Vec N, Vec I,
             float u, float v, int level)
{
   long temp, x0, x1, y0, y1, xh, yh;
   Flt x, y, xt, yt;
   Vec VP, VW;
   Hexagon *hex = (Hexagon*)text->data;
   if (text->t_trans != nullptr) {
      /* Transform the point into texture space */
      InvTxVector1(VP, P, text->t_trans);
      InvTxVector1(VW, W, text->t_trans);
      }
   else {
      VecCopy(P, VP);
      VecCopy(W, VW);
      }
   /* Scale to make everything fit */
   x = 2.0 * VP[0] / 3.0;
   y = 2.0 * VP[2] / sqrt(3.0);
   x0 = (long)floor(x); x1 = x0 + 1; xt = x - (Flt)x0;
   y0 = (long)floor(y); y1 = y0 + 1; yt = y - (Flt)y0;
   temp = x0 + y0;
   if (temp & 1) {
      /* Odd hex */
      if (xt < 0.333333)
         temp = 1;
      else if (xt > 0.666666)
         temp = 0;
      else if (yt > 3.0 * xt - 1.0)
         temp = 1;
      else
         temp = 0;
      if (temp) {
         xh = x0;
         yh = y1;
         }
      else {
         xh = x1;
         yh = y0;
         }
      }
   else {
      /* Even hex */
      if (xt < 0.333333)
         temp = 1;
      else if (xt > 0.666666)
         temp = 0;
      else if (yt < -3.0 * xt + 2.0)
         temp = 1;
      else
         temp = 0;
      if (temp) {
         xh = x0;
         yh = y0;
         }
      else {
         xh = x1;
         yh = y1;
         }
      }
/*
   if (hex->repeat_flag) {
      VP[0] = xt;
      VP[1] = 0.0;
      VP[2] = yt;
      }
*/
   temp = ((yh + 3 * xh) / 2) % 3;
   if (temp < 0) temp += 3;
   if (temp == 0)
      return (hex->text1->eval)(Eye, obj, hex->text1, VW, VP, N, I, u, v, level);
   else if (temp == 1)
      return (hex->text2->eval)(Eye, obj, hex->text2, VW, VP, N, I, u, v, level);
   else if (temp == 2)
      return (hex->text3->eval)(Eye, obj, hex->text3, VW, VP, N, I, u, v, level);
   else {
      serror("Bad hex index: %d\n", temp);
      return 0;
   }
}

/** @brief Free the resources owned by a hexagon texture.
 *  @param texture Hexagon texture to destroy.
 *  @return No return value.
 */
static void
delete_hexagon(Texture *texture)
{
   Hexagon *hex = (Hexagon*)texture->data;
   TextureDelete(hex->text1);
   TextureDelete(hex->text2);
   TextureDelete(hex->text3);
   polyray_free(hex);
}

/** @brief Initialize a hexagon texture from three child textures.
 *  @param texture Texture object to initialize.
 *  @param text1   First child texture.
 *  @param text2   Second child texture.
 *  @param text3   Third child texture.
 *  @return No return value.
 */
void
create_hexagon(Texture *texture, Texture *text1, Texture *text2, Texture *text3)
{
   Hexagon *hex = (Hexagon*)polyray_malloc(sizeof(Hexagon));
   if (hex == nullptr) {
      serror("Failed to allocate a hexagon texture\n");
      return;
   }
   /* hex->repeat_flag = Flag; */
   hex->text1 = text1;
   hex->text2 = text2;
   hex->text3 = text3;
   hex->repeat_flag1 = 0;
   hex->repeat_flag2 = 0;
   hex->repeat_flag3 = 0;
   texture->type   = ShapeType::Hexagon;
   texture->eval   = eval_hexagon;
   texture->del    = delete_hexagon;
   texture->data   = hex;
}

/** @brief Deep-copy the parsed expression fields of a special surface.
 *  @param old_spec Source special surface.
 *  @param new_spec Destination special surface.
 *  @return No return value.
 */
void
copy_special0(Special_Surface *old_spec, Special_Surface *new_spec)
{
   /* Copy each component of the special surface */
   new_spec->body_color  = copy_node(old_spec->body_color);
   new_spec->normal      = copy_node(old_spec->normal);
   new_spec->position    = copy_node(old_spec->position);
   new_spec->Ka_color    = copy_node(old_spec->Ka_color);
   new_spec->Ka_scale    = copy_node(old_spec->Ka_scale);
   new_spec->Kb_power    = copy_node(old_spec->Kb_power);
   new_spec->Kd_color    = copy_node(old_spec->Kd_color);
   new_spec->Kd_scale    = copy_node(old_spec->Kd_scale);
   new_spec->Ks_color    = copy_node(old_spec->Ks_color);
   new_spec->Ks_scale    = copy_node(old_spec->Ks_scale);
   new_spec->Kr_color    = copy_node(old_spec->Kr_color);
   new_spec->Kr_scale    = copy_node(old_spec->Kr_scale);
   new_spec->Kt_color    = copy_node(old_spec->Kt_color);
   new_spec->Kt_scale    = copy_node(old_spec->Kt_scale);
   new_spec->D_angle     = copy_node(old_spec->D_angle);
   new_spec->D_type      = old_spec->D_type;
   new_spec->ior         = copy_node(old_spec->ior);
   new_spec->Position_fn = copy_node(old_spec->Position_fn);
   new_spec->Pos_scale   = copy_node(old_spec->Pos_scale);
   new_spec->Lookup_fn   = copy_node(old_spec->Lookup_fn);
   new_spec->Turbulence  = copy_node(old_spec->Turbulence);
   new_spec->Octaves     = copy_node(old_spec->Octaves);
   new_spec->Frequency   = copy_node(old_spec->Frequency);
   new_spec->Phase       = copy_node(old_spec->Phase);
   new_spec->Bump_scale  = copy_node(old_spec->Bump_scale);
   new_spec->map = nullptr;
}

/** @brief Evaluate a special texture by running its expression-controlled modifiers.
 *  @param Eye   Active viewpoint.
 *  @param obj   Object being shaded.
 *  @param text  Special texture instance.
 *  @param W     World-space point.
 *  @param P     Object/texture-space point.
 *  @param N     Surface normal.
 *  @param I     Incoming ray direction.
 *  @param u     Texture u coordinate.
 *  @param v     Texture v coordinate.
 *  @param level Recursion/shading level.
 *  @return Pointer to the generated surface owned by the special texture.
 */
static Surface *
eval_special(Viewpoint *Eye, Object *obj, Texture *text,
             Vec W, Vec P, Vec N, Vec I,
             float u, float v, int level)
{
   #ifdef DEBUG_FN_CALLS
   smessage("texture::eval_special\n");
   #endif
   Special_Surface *special = (Special_Surface*)text->data;
   Surface *new_surf = &((*special).surf);
   struct subst_struct subst;
   Vec WV, PV, NV;
   Flt F;
   NODE_PTR tnode;

   if (text->t_trans != nullptr) {
      /* Apply texture transform */
      InvTxVector1(WV, W, text->t_trans);
      InvTxVector1(PV, P, text->t_trans);
      }
   else {
      VecCopy(W, WV);
      VecCopy(P, PV);
      }

   /* Build a substitution structure to evaluate the special texture with */
   VecCopy(PV, subst.P);
   MakeVector(0, 0, 0, subst.PT);
   MakeVector(u, v, level, subst.U);
   VecCopy(WV, subst.W);
   VecCopy(N, subst.N);
   VecCopy(I, subst.I);

   /* If there is a position modifying function then evaluate it */
   if (special->position != nullptr) {
      if (eval_node(&subst, special->position, &F, PV, &tnode) != 2) {
         serror("Bad position vector in eval_surface\n");
         return new_surf;
      }
      VecCopy(PV, subst.P);
      }

   /* If there is a normal modifying function then evaluate it */
   if (special->normal != nullptr) {
      if (eval_node(&subst, special->normal, &F, NV, &tnode) != 2) {
         serror("Bad normal vector in eval_surface: <%g, %g, %g>\n", NV);
         return new_surf;
      }
      VecNormalize(NV);
      VecCopy(NV, N);
      VecCopy(NV, subst.N);
      }

   generate_surface(&subst, special, new_surf);

   return new_surf;
}

/** @brief Free the resources owned by a special texture.
 *  @param texture Special texture to destroy.
 *  @return No return value.
 */
static void
delete_special(Texture *texture)
{
   delete_special0((Special_Surface *)texture->data);
}

/** @brief Initialize a special texture from prebuilt special-surface data.
 *  @param texture Texture object to initialize.
 *  @param data    Special-surface payload stored by the texture.
 *  @return No return value.
 */
void
create_special(Texture *texture, void *data)
{
   #ifdef DEBUG_FN_CALLS
   smessage("texture::create_special\n");
   #endif
   Special_Surface *spec = (Special_Surface *)data;
   texture->type   = ShapeType::Special;
   texture->eval   = eval_special;
   texture->del    = delete_special;
   texture->data   = data;
   memcpy(&spec->surf, &RuntimeState::DefaultSurface, sizeof(Surface));
}

/** @brief Evaluate a noise-driven procedural texture.
 *  @param Eye   Active viewpoint.
 *  @param obj   Object being shaded.
 *  @param text  Noise texture instance.
 *  @param W     World-space point.
 *  @param P     Object/texture-space point.
 *  @param N     Surface normal.
 *  @param I     Incoming ray direction.
 *  @param u     Texture u coordinate.
 *  @param v     Texture v coordinate.
 *  @param level Recursion/shading level.
 *  @return Pointer to the generated surface owned by the noise texture.
 */
static Surface *
eval_noise(Viewpoint *Eye, Object *obj, Texture *text,
           Vec W, Vec P, Vec N, Vec I,
           float u, float v, int level)
{
   Noise_Surface *noise_surf = (Noise_Surface*)text->data;
   Surface *new_surf = &noise_surf->surf;
   Vec WV, PV, ND, body;
   Flt ind, pos, nval, inter0, inter1;
   map_entries tmp = noise_surf->map;
   int cflag;

   if (text->t_trans != nullptr)
      InvTxVector1(PV, P, text->t_trans)
   else
      VecCopy(P, PV)
   VecCopy(W, WV);

   /* Modify the position value */
   switch (noise_surf->Position_fn) {
   case 1:
      pos = PV[0];
      break;
   case 2:
      pos = WV[0];
      break;
   case 3:
      pos = sqrt(PV[0] * PV[0] + PV[1] * PV[1]);
      break;
   case 4:
      pos = sqrt(PV[0] * PV[0] + PV[1] * PV[1] + PV[2] * PV[2]);
      break;
   case 5:
      pos = sqrt(PV[0] * PV[0] + PV[2] * PV[2]);
      if (pos < PLY_EPSILON)
         pos = 0;
      else {
         pos = acos(PV[0] / pos);
         if (PV[2] < 0) pos = TWO_PI - pos;
         pos = pos / TWO_PI;
         }
      break;
   default:
      pos = 0.0;
      break;
      }

   /* If there is a normal modifying function then evaluate it */
   switch (noise_surf->N_modifier) {
   case 1:
      /* Bumpy */
      dnoise3d(PV, ND, 2, 0.5, noise_surf->Frequency);
      ND[0] = 2.0 * (ND[0] - 0.5);
      ND[1] = 2.0 * (ND[1] - 0.5);
      ND[2] = 2.0 * (ND[2] - 0.5);
      VecAddS(noise_surf->Bump_scale, ND, N, N);
      VecNormalize(N);
      break;
   case 2:
      /* Rippled */
      ripples(PV, N, noise_surf->Frequency, noise_surf->Phase,
              noise_surf->Bump_scale);
      break;
   case 3:
      /* Dented */
      dnoise3d(PV, ND, 2, 0.5, noise_surf->Frequency);
      ND[0] = 2.0 * (ND[0] - 0.5);
      ND[1] = 2.0 * (ND[1] - 0.5);
      ND[2] = 2.0 * (ND[2] - 0.5);
      VecScale(noise_surf->Bump_scale, ND);
          nval =  fnoise(PV, 2, 0.5, noise_surf->Frequency);
      VecAddS(nval, ND, N, N);
      VecNormalize(N);
      break;
   default:
      break;
   }

   /* Evaluate the noise function for this surface */
   if (noise_surf->Turbulence != 0.0) {
      VecCopy(PV, WV);
      VecScale(noise_surf->Pos_scale, WV);
      nval = pos * noise_surf->Pos_scale +
             noise_surf->Turbulence * fnoise(WV, 2.0, 0.5, noise_surf->Octaves);
      }
   else
      nval = pos * noise_surf->Pos_scale;

   /* Evaluate the lookup function for this surface */
   switch (noise_surf->Lookup_fn) {
   case 0:
      ind = nval;
      break;
   case 1:
      ind = sawtooth(nval);
      break;
   case 2:
      ind = (sin(TWO_PI * nval) + 1.0) / 2.0;
      break;
   case 3:
      /* Ramp */
      ind = fmod(nval, 1.0);
      if (ind < 0) ind = 1 + ind;
      break;
   default:
      ind = nval;
      }

   /* Look up the color from the map */
   cflag = 0;
   while (tmp != nullptr && !cflag) {
      if (ind == tmp->p0) {
         VecCopy(tmp->v0, body);
         new_surf->Kt_scale = tmp->t0;
         cflag = 1;
         }
      else if (ind == tmp->p1) {
         VecCopy(tmp->v1, body);
         new_surf->Kt_scale = tmp->t1;
         cflag = 1;
         }
      else if (ind >= tmp->p0 && ind <= tmp->p1) {
         /* Found the correct entry in the color map - do
            a linear interpolation of values for final color. */
         inter0 = (ind - tmp->p0) / (tmp->p1 - tmp->p0);
         inter1 = (1 - inter0);
         body[0]  = inter0 * tmp->v1[0] + inter1 * tmp->v0[0];
         body[1]  = inter0 * tmp->v1[1] + inter1 * tmp->v0[1];
         body[2]  = inter0 * tmp->v1[2] + inter1 * tmp->v0[2];
         new_surf->Kt_scale = inter0 * tmp->t1 + inter1 * tmp->t0;
         cflag = 1;
         }
      else
         tmp = tmp->next;
      }
   if (!cflag) {
      VecCopy(noise_surf->body_color, body);
      VecCopy(noise_surf->body_color, new_surf->Kt_color);
      }

   /* Plop the color into the various components of the surface */
   VecCopy(body, new_surf->Ka_color);
   VecCopy(body, new_surf->Kd_color);
   /* This appears to be a bug, if the reflection component is
      specified then it doesn't get used:
   VecCopy(body, new_surf->Kr_color); */
   if (noise_surf->Kt_flag)
      VecCopy(body, new_surf->Kt_color);

   return new_surf;
}

/** @brief Free the resources owned by a noise texture.
 *  @param text Noise texture to destroy.
 *  @return No return value.
 */
static void
delete_noise(Texture *text)
{
   Noise_Surface *noise_surf = (Noise_Surface *)text->data;
   map_entries map, temp;
   /* The color map is the only dynamically allocated piece. */
   map = noise_surf->map;
   while (map != nullptr) {
      temp = map;
      map = map->next;
      delete temp;
      }
   polyray_free(noise_surf);
}

/** @brief Initialize a procedural noise texture from a parsed special surface.
 *  @param texture Texture object to initialize.
 *  @param surf    Parsed special-surface description to consume.
 *  @return No return value.
 */
void
create_noise(Texture *texture, Special_Surface *surf)
{
   Noise_Surface *new_surf =
      (Noise_Surface *)polyray_malloc(sizeof(Noise_Surface));
   memcpy(&new_surf->surf, &RuntimeState::DefaultSurface, sizeof(Surface));
   generate_noise_surface(surf, new_surf);
   delete_special0(surf);
   texture->type   = ShapeType::Noise;
   texture->eval   = eval_noise;
   texture->del    = delete_noise;
   texture->data   = new_surf;
}

/** @brief Evaluate a layered texture by compositing its child textures.
 *  @param Eye   Active viewpoint.
 *  @param obj   Object being shaded.
 *  @param text  Layered texture instance.
 *  @param W     World-space point.
 *  @param P     Object/texture-space point.
 *  @param N     Surface normal.
 *  @param I     Incoming ray direction.
 *  @param u     Texture u coordinate.
 *  @param v     Texture v coordinate.
 *  @param level Recursion/shading level.
 *  @return Pointer to the composited surface owned by the layered texture.
 */
static Surface *
eval_layered(Viewpoint *Eye, Object *obj, Texture *text,
             Vec W, Vec P, Vec N, Vec I,
             float u, float v, int level)
{
   #ifdef DEBUG_FN_CALLS
   smessage("texture::eval_layered\n");
   #endif
   Vec VP, color, temp_color;
   Flt surf_alpha, old_kt;
   Layered *layer = (Layered*)text->data;
   tstackptr texts;
   Surface *surf;
   Vec *LC;

   /* Transform the point into texture space */
   if (text->t_trans != nullptr)
      InvTxVector1(VP, P, text->t_trans)
   else
      VecCopy(P, VP)

   if (RuntimeState::Shadow_Test) {
      /* During shadowing, all we will do is determine the amount of
         transparency left at the bottom layer.  The only filtering will
         also only be from the bottom layer. */
      layer->surf.Ka_scale = 0.0;
      surf_alpha = 1.0;
      for (texts=layer->texts;texts->next!=nullptr;texts=texts->next) {
         surf = (texts->element->eval)(Eye, obj, texts->element, W, VP, N, I,
                                       u, v, level);
         surf_alpha *= surf->Kt_scale;
         }
      surf = (texts->element->eval)(Eye, obj, texts->element, W, VP, N, I,
                                    u, v, level);
      surf_alpha *= surf->Kt_scale;
      layer->surf.Kt_scale = surf_alpha;
      VecCopy(surf->Kt_color, layer->surf.Kt_color);
      return &layer->surf;
      }

   /* Get shadow information so we don't have to do it for every layer */
   LC = (Vec *)polyray_malloc(RuntimeState::nLights * sizeof(Vec));
   if (LC == nullptr) {
      serror("Failed to allocate space for light information\n");
      return nullptr;
   }
   else
      Get_Light_Colors(Eye, W, LC);

   /* The way we do the evaluation is by successive calls to "Shade" - as
      long as there is a little alpha left in the current texture, we keep
      evaluating.  The result is stored as a surface that only has an
      ambient component. */
   texts = layer->texts;
   surf = (texts->element->eval)(Eye, obj, texts->element,
                                 W, VP, N, I, u, v, level);
   surf_alpha = surf->Kt_scale;
   surf->Kt_scale = 0.0;
   ShadeSurface(Eye, obj, surf, level, 1.0, 1.0, I, W, N, color, LC);
   surf->Kt_scale = surf_alpha;
   VecScale((1.0 - surf_alpha), color);
   texts = texts->next;
   while (texts != nullptr && surf_alpha > 1.0e-3) {
      surf = (texts->element->eval)(Eye, obj, texts->element,
                                    W, VP, N, I, u, v, level);
      if (texts->next != nullptr) {
         old_kt = surf->Kt_scale;
         surf->Kt_scale = 0.0;
         ShadeSurface(Eye, obj, surf, level, 1.0, 1.0, I, W, N, temp_color, LC);
         surf->Kt_scale = old_kt;
         VecAddS(surf_alpha * (1.0 - old_kt), temp_color, color, color);
         surf_alpha *= old_kt;
         }
      else {
         ShadeSurface(Eye, obj, surf, level, 1.0, 1.0, I, W, N, temp_color, LC);
         VecAddS(surf_alpha, temp_color, color, color);
         }
      texts = texts->next;
      }
   layer->surf.Ka_scale = 1.0;
   layer->surf.Kt_scale = 0.0;
   VecCopy(color, layer->surf.Ka_color);
   polyray_free(LC);
   return &layer->surf;
}

/** @brief Free the resources owned by a layered texture.
 *  @param texture Layered texture to destroy.
 *  @return No return value.
 */
static void
delete_layered(Texture *texture)
{
   tstackptr temp, last;
   Layered *layer = (Layered*)texture->data;

   if (!layer->copy_flag) {
      /* Delete the component layers */
      temp = layer->texts;
      while (temp != nullptr) {
         TextureDelete(temp->element);
         last = temp;
         temp = temp->next;
         delete last;
         }
      }
   polyray_free(layer);
}

/** @brief Initialize a layered texture from a stack of child textures.
 *  @param texture Texture object to initialize.
 *  @param texts   Stack of child textures.
 *  @return No return value.
 */
void
create_layered(Texture *texture, tstackptr texts)
{
   #ifdef DEBUG_FN_CALLS
   smessage("texture::create_layered\n");
   #endif
   Layered *layer = (Layered*)polyray_malloc(sizeof(Layered));
   Surface *surf;

   if (layer == nullptr) {
      serror("Failed to allocate a layered texture\n");
      return;
   }
   layer->copy_flag = 0;
   layer->texts     = texts;
   texture->type    = ShapeType::Layered;
   texture->eval    = eval_layered;
   texture->del     = delete_layered;
   /* Initialize the surface in such a way that only the ambient contribution
      will ever be used. */
   surf = &layer->surf;
   MakeVector(0.0, 0.0, 0.0, surf->Ka_color);
   surf->Ka_scale = 1.0;
   surf->Kb_power = 1.0;
   MakeVector(0.0, 0.0, 0.0, surf->Kd_color);
   surf->Kd_scale = 0.0;
   MakeVector(0.0, 0.0, .0, surf->Ks_color);
   surf->Ks_scale = 0.0;
   MakeVector(0.0, 0.0, 0.0, surf->Kr_color);
   surf->Kr_scale = 0.0;
   MakeVector(0.0, 0.0, 0.0, surf->Kt_color);
   surf->Kt_scale = 0.0;
   surf->D = nullptr;
   surf->D_coeff = 1.0;
   surf->ior = 1.0;

   texture->data   = layer;
}

/** @brief Free all entries in a texture-map chain.
 *  @param map Head of the texture-map entry chain.
 *  @return No return value.
 */
void
delete_texture_map(texture_map_entries map)
{
   texture_map_entries temp1, temp2;
   for (temp1=map;temp1!=nullptr;) {
      TextureDelete(temp1->t0);
      TextureDelete(temp1->t1);
      temp2 = temp1;
      temp1 = temp1->next;
      delete temp2;
      }
}

/** @brief Evaluate an indexed texture by blending or selecting mapped child textures.
 *  @param Eye   Active viewpoint.
 *  @param obj   Object being shaded.
 *  @param tex   Indexed texture instance.
 *  @param W     World-space point.
 *  @param P     Object/texture-space point.
 *  @param N     Surface normal.
 *  @param I     Incoming ray direction.
 *  @param u     Texture u coordinate.
 *  @param v     Texture v coordinate.
 *  @param level Recursion/shading level.
 *  @return Surface selected or blended from the texture map.
 */
static Surface *
eval_indexed(Viewpoint *Eye, Object *obj, Texture *tex,
             Vec W, Vec P, Vec N, Vec I,
             float u, float v, int level)
{
   struct subst_struct subst;
   Vec WV, PV, NT, c0, c1;
   Flt F, inter0, inter1, cs0, cs1;
   NODE_PTR tnode;
   Texture *tlow, *thigh;
   Surface *surf;
   int cflag;
   texture_map_entries tmp;
   Indexed *text = (Indexed*)tex->data;

   /* Transform the point into texture space */
   if (tex->t_trans != nullptr) {
      /* Apply texture transform */
      InvTxVector1(WV, W, tex->t_trans);
      InvTxVector1(PV, P, tex->t_trans);
      }
   else {
      VecCopy(W, WV);
      VecCopy(P, PV);
      }

   /* Build a substitution structure to evaluate the special texture with */
   VecCopy(PV, subst.P);
   MakeVector(0, 0, 0, subst.PT);
   MakeVector(u, v, 0, subst.U);
   VecCopy(WV, subst.W);
   VecCopy(N, subst.N);
   VecCopy(I, subst.I);

   /* Evaluate the index function */
   if (eval_node(&subst, text->exper, &F, N, &tnode) != 1) {
      serror("Bad texture index function\n");
      return nullptr;
   }

   /* Determine which two textures are contributing to the overall texture. */
   cflag = 0;
   tmp = text->texts;
   while (tmp != nullptr && !cflag) {
      if (F == tmp->p0) {
         inter0 = 0.0;
         inter1 = 1.0;
         tlow   = tmp->t0;
         thigh  = tmp->t1;
         cflag = 1;
         }
      else if (F == tmp->p1) {
         inter0 = 1.0;
         inter1 = 0.0;
         tlow   = tmp->t0;
         thigh  = tmp->t1;
         cflag = 1;
         }
      else if (F >= tmp->p0 && F <= tmp->p1) {
         /* Found the correct entry in the color map - do
            a linear interpolation of values for final color. */
         inter0 = (F - tmp->p0) / (tmp->p1 - tmp->p0);
         inter1 = (1 - inter0);
         tlow   = tmp->t0;
         thigh  = tmp->t1;
         cflag = 1;
         }
      else
         tmp = tmp->next;
      }

   if (!cflag)
      /* Index is not in the texture map.  Return the default texture */
      return &RuntimeState::DefaultSurface;

   if (runtimeState::Shadow_Test) {
      /* During shadowing, all we will do is determine the amount of
         transparency left at the bottom layer.  The only filtering will
         also only be from the bottom layer. */
      if (inter1 == 1.0)
         surf = (tlow->eval)(Eye, obj, tlow, W, PV, N, I, u, v, level);
      else if (inter0 == 1.0)
         surf = (thigh->eval)(Eye, obj, thigh, W, PV, N, I, u, v, level);
      else {
         VecCopy(N, NT);
         surf = (tlow->eval)(Eye, obj, tlow, W, PV, NT, I, u, v, level);
         VecCopy(surf->Kt_color, c0);
         cs0 = surf->Kt_scale;
         VecCopy(N, NT);
         surf = (thigh->eval)(Eye, obj, thigh, W, PV, NT, I, u, v, level);
         VecCopy(surf->Kt_color, c1);
         cs1 = surf->Kt_scale;
         surf = &text->surf;
         surf->Ka_scale = 0.0;
         surf->Kt_scale = inter1 * cs0 + inter0 * cs1;
         VecComb(inter1, c0, inter0, c1, surf->Kt_color);
         }
      }
   else if (inter1 == 1.0)
      surf = (tlow->eval)(Eye, obj, tlow, W, PV, N, I, u, v, level);
   else if (inter0 == 1.0)
      surf = (thigh->eval)(Eye, obj, thigh, W, PV, N, I, u, v, level);
   else {
      VecCopy(N, NT);
      surf = (tlow->eval)(Eye, obj, tlow, W, PV, NT, I, u, v, level);
      ShadeSurface(Eye, obj, surf, level, 1.0, 1.0, I, W, NT, c0, nullptr);
      VecCopy(N, NT);
      surf = (thigh->eval)(Eye, obj, thigh, W, PV, NT, I, u, v, level);
      ShadeSurface(Eye, obj, surf, level, 1.0, 1.0, I, W, NT, c1, nullptr);
      surf = &text->surf;
      surf->Kt_scale = 0.0;
      surf->Ka_scale = 1.0;
      VecComb(inter1, c0, inter0, c1, surf->Ka_color);
      }

   return surf;
}

/** @brief Free the resources owned by an indexed texture.
 *  @param texture Indexed texture to destroy.
 *  @return No return value.
 */
static void
delete_indexed(Texture *texture)
{
   Indexed *text = (Indexed*)texture->data;

   if (!text->copy_flag) {
      delete_texture_map(text->texts);
      deallocate_node(text->exper);
      }
   polyray_free(text);
}

/** @brief Initialize an indexed texture from an expression and texture map.
 *  @param texture Texture object to initialize.
 *  @param exper   Expression that computes the texture index.
 *  @param texts   Texture-map entries used for lookup/interpolation.
 *  @return No return value.
 */
void
create_indexed(Texture *texture, NODE_PTR exper, texture_map_entries texts)
{
   Indexed *text = (Indexed*)polyray_malloc(sizeof(Indexed));
   Surface *surf;

   if (text == nullptr) {
      serror("Failed to allocate an indexed texture\n");
      return;
   }
   text->copy_flag = 0;
   text->exper     = exper;
   text->texts     = texts;
   texture->type   = ShapeType::Indexed;
   texture->eval   = eval_indexed;
   texture->del    = delete_indexed;
   texture->data   = text;

   /* Initialize the surface in such a way that only the ambient contribution
      will ever be used. */
   surf = &text->surf;
   MakeVector(0.0, 0.0, 0.0, surf->Ka_color);
   surf->Ka_scale = 1.0;
   surf->Kb_power = 1.0;
   MakeVector(0.0, 0.0, 0.0, surf->Kd_color);
   surf->Kd_scale = 0.0;
   MakeVector(0.0, 0.0, 0.0, surf->Ks_color);
   surf->Ks_scale = 0.0;
   MakeVector(0.0, 0.0, 0.0, surf->Kr_color);
   surf->Kr_scale = 0.0;
   MakeVector(0.0, 0.0, 0.0, surf->Kt_color);
   surf->Kt_scale = 0.0;
   surf->D = nullptr;
   surf->D_coeff = 1.0;
   surf->ior = 1.0;
}

/** @brief Free all entries in a texture-function chain.
 *  @param fns Head of the texture-function entry chain.
 *  @return No return value.
 */
static void
delete_texture_fns(texture_fn_entries fns)
{
   texture_fn_entries temp1, temp2;
   for (temp1=fns;temp1!=nullptr;) {
      deallocate_node(temp1->fn);
      TextureDelete(temp1->t0);
      temp2 = temp1;
      temp1 = temp1->next;
      delete temp2;
      }
}

/** @brief Evaluate a summed texture by accumulating weighted child textures.
 *  @param Eye   Active viewpoint.
 *  @param obj   Object being shaded.
 *  @param text  Summed texture instance.
 *  @param W     World-space point.
 *  @param P     Object/texture-space point.
 *  @param N     Surface normal.
 *  @param I     Incoming ray direction.
 *  @param u     Texture u coordinate.
 *  @param v     Texture v coordinate.
 *  @param level Recursion/shading level.
 *  @return Pointer to the accumulated surface owned by the summed texture.
 */
static Surface *
eval_summed(Viewpoint *Eye, Object *obj, Texture *text,
            Vec W, Vec P, Vec N, Vec I, float u, float v, int level)
{
   NODE_PTR n0;
   Vec p0, PV, WV, color, surf_color;
   Flt f0, surf_alpha;
   struct subst_struct subst;
   Summed *stex = (Summed*)text->data;
   texture_fn_entries texts;
   Surface *surf;
   Vec *LC;

   /* Transform the point into texture space */
   if (text->t_trans != nullptr) {
      /* Apply texture transform */
      InvTxVector1(WV, W, text->t_trans);
      InvTxVector1(PV, P, text->t_trans);
      }
   else {
      VecCopy(W, WV);
      VecCopy(P, PV);
      }

   /* Build a substitution structure to evaluate the special texture with */
   VecCopy(PV, subst.P);
   MakeVector(0, 0, 0, subst.PT);
   MakeVector(u, v, 0, subst.U);
   VecCopy(WV, subst.W);
   VecCopy(N, subst.N);
   VecCopy(I, subst.I);

   if (runtimeState::Shadow_Test) {
      /* Simple sum of all transparency components of each layer */
      surf_alpha = 1.0;
      MakeVector(0, 0, 0, surf_color);
      for (texts=stex->texts;texts->next!=nullptr;texts=texts->next) {
         /* Evaluate the index function */
         if (eval_node(&subst, texts->fn, &f0, p0, &n0) != 1) {
            serror("Bad texture sum function\n");
            return nullptr;
         }
         if (f0 > 0) {
            surf = (texts->t0->eval)(Eye, obj, texts->t0, WV, PV, N, I, u, v, level);
            surf_alpha += f0 * surf->Kt_scale;
            VecAddScaled(surf_color, f0, surf->Kt_color, surf_color);
            }
         }
      stex->surf.Ka_scale = 0.0;
      stex->surf.Kt_scale = surf_alpha;
      VecCopy(surf_color, stex->surf.Kt_color);
      return &stex->surf;
      }

   /* Get shadow information so we don't have to do it for every layer */
   LC = (Vec *)polyray_malloc(RuntimeState::nLights * sizeof(Vec));
   if (LC == nullptr) {
      serror("Failed to allocate space for light information\n");
      return nullptr;
   }
   else
      Get_Light_Colors(Eye, W, LC);

   /* The way we do the evaluation is by successive calls to "Shade" - as
      long as there is a little alpha left in the current texture, we keep
      evaluating.  The result is stored as a surface that only has an
      ambient component. */
   MakeVector(0, 0, 0, color);
   texts=stex->texts;
   while (texts != nullptr) {
      if (eval_node(&subst, texts->fn, &f0, p0, &n0) != 1) {
         serror("Bad texture sum function\n");
         return nullptr;
      }
      if (fabs(f0) > PLY_EPSILON) {
         surf = (texts->t0->eval)(Eye, obj, texts->t0, WV, PV, N, I, u, v, level);
         ShadeSurface(Eye, obj, surf, level, 1.0, 1.0, I, W, N, surf_color, LC);
         VecAddScaled(color, f0, surf_color, color);
         }
      texts = texts->next;
      }

   /* Create the final surface */
   stex->surf.Ka_scale = 1.0;
   stex->surf.Kt_scale = 0.0;
   VecCopy(color, stex->surf.Ka_color);
   polyray_free(LC);
   return &stex->surf;
}

/** @brief Free the resources owned by a summed texture.
 *  @param texture Summed texture to destroy.
 *  @return No return value.
 */
static void
delete_summed(Texture *texture)
{
   Summed *tex = (Summed*)texture->data;

   if (!tex->copy_flag)
      delete_texture_fns(tex->texts);
   polyray_free(tex);
}

/** @brief Initialize a summed texture from weighted child texture functions.
 *  @param texture Texture object to initialize.
 *  @param texts   Texture-function entries to accumulate.
 *  @return No return value.
 */
void
create_summed(Texture *texture, texture_fn_entries texts)
{
   Summed *tex = (Summed*)polyray_malloc(sizeof(Summed));
   Surface *surf;

   if (tex == nullptr) {
      serror("Failed to allocate a summed texture\n");
      return;
   }
   tex->copy_flag = 0;
   tex->texts     = texts;
   texture->type    = ShapeType::Summed;
   texture->eval    = eval_summed;
   texture->del     = delete_summed;
   /* Initialize the surface in such a way that only the ambient contribution
      will ever be used. */
   surf = &tex->surf;
   MakeVector(0.0, 0.0, 0.0, surf->Ka_color);
   surf->Ka_scale = 1.0;
   surf->Kb_power = 1.0;
   MakeVector(0.0, 0.0, 0.0, surf->Kd_color);
   surf->Kd_scale = 0.0;
   MakeVector(0.0, 0.0, 0.0, surf->Ks_color);
   surf->Ks_scale = 0.0;
   MakeVector(0.0, 0.0, 0.0, surf->Kr_color);
   surf->Kr_scale = 0.0;
   MakeVector(0.0, 0.0, 0.0, surf->Kt_color);
   surf->Kt_scale = 0.0;
   surf->D = nullptr;
   surf->D_coeff = 1.0;
   surf->ior = 1.0;

   texture->data   = tex;
}

/** @brief Copy a texture header and duplicate its transform when present.
 *  @param in_texture  Source texture.
 *  @param out_texture Destination texture.
 *  @return No return value.
 */
void
TextureCopy(Texture *in_texture, Texture *out_texture)
{
   /* Copy all the default stuff */
   memcpy(out_texture, in_texture, sizeof(Texture));

   /* Copy the transform. */
   if (in_texture->t_trans != nullptr) {
      Transform *tmptrn = FactoryTransform().release();
      if (tmptrn == nullptr) {
         serror("Failed to allocate a transform\n");
         return;
      }
      memcpy(tmptrn, in_texture->t_trans, sizeof(Transform));
      out_texture->t_trans = tmptrn;
      }

   /* Indicate that this is a copied texture */
   out_texture->copy_flag = 1;
}

/** @brief Destroy a texture and its transform when owned by this instance.
 *  @param texture Texture to destroy.
 *  @return No return value.
 */
void TextureDelete(Texture *texture)
{
#ifdef DEBUG
    smessage("texture::TextureDelete\n");
#endif
   if (!texture->copy_flag)
      (texture->del)(texture);
   if (texture->t_trans != nullptr) {
      delete texture->t_trans;
      texture->t_trans = nullptr;
   }
   delete texture;
}

/* Transformation functions for textures */
/** @brief Apply a shear transform to a texture.
 *  @param text Texture to modify.
 *  @param xy   X shear by Y.
 *  @param xz   X shear by Z.
 *  @param yx   Y shear by X.
 *  @param yz   Y shear by Z.
 *  @param zx   Z shear by X.
 *  @param zy   Z shear by Y.
 *  @return No return value.
 */
void TextureShear(Texture *text, Flt xy, Flt xz, Flt yx, Flt yz,
             Flt zx, Flt zy)
{
   Transform trans;
   Get_Shear_Transformation(&trans, xy, xz, yx, yz, zx, zy);
   if (text->t_trans == nullptr) 
      text->t_trans = Get_Transformation().release();
   Compose_Transformations(* text->t_trans, trans);
}

/** @brief Apply a translation transform to a texture.
 *  @param text   Texture to modify.
 *  @param Vector Translation vector.
 *  @return No return value.
 */
void TextureTranslate(Texture *text, Vec Vector)
{
   Transform trans;
   Get_Translation_Transformation(&trans, Vector);
   if (text->t_trans == nullptr) 
      text->t_trans = Get_Transformation().release();
   Compose_Transformations(* text->t_trans, trans);
}

/** @brief Apply Euler-angle rotation in degrees to a texture.
 *  @param text Texture to modify.
 *  @param v    Rotation angles in degrees.
 *  @return No return value.
 */
void TextureRotate(Texture *text, Vec v)
{
   Transform trans;
   Vec vt;
   VecCopy(v, vt);
   VecScale(PYM_PI/180.0, vt);
   Get_Rotation_Transformation(&trans, vt);
   if (text->t_trans == nullptr) 
      text->t_trans = Get_Transformation().release();
   Compose_Transformations(* text->t_trans, trans);
}

/** @brief Apply an axis-angle rotation in degrees to a texture.
 *  @param text Texture to modify.
 *  @param v    Rotation axis.
 *  @param ang  Rotation angle in degrees.
 *  @return No return value.
 */
void
TextureAxisRotate(Texture *text, Vec v, Flt ang)
{
   Transform trans;
   NuVec nv;
   nv[0]=v[0];nv[1]=v[1];nv[2]=v[2];
   Get_Rotate_Transform_CPP(trans, nv, PYM_PI * ang / 180.0);
   if (text->t_trans == nullptr) 
      text->t_trans = Get_Transformation().release();
   Compose_Transformations(* text->t_trans, trans);
}

/** @brief Apply a scaling transform to a texture.
 *  @param text   Texture to modify.
 *  @param Vector Scale factors along each axis.
 *  @return No return value.
 */
void TextureScale(Texture *text, Vec Vector)
{
   Transform trans;
   Get_Scaling_Transformation (&trans, Vector);
   if (text->t_trans == nullptr) 
      text->t_trans = Get_Transformation().release();
   Compose_Transformations(* text->t_trans, trans);
}

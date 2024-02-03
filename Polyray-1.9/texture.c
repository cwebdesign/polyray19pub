/* texture.c

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
#include "ytab.h"
#include "light.h"
#include "builder.h"
#include "eval.h"
#include "mfacet.h"
#include "symtab.h"
#include "vector.h"
#include "shade.h"
#include "texture.h"

/* Deallocate all components of a special surface */
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
   polyray_free(spec);
}

/* Evaluate each component of the special surface, placing
   the results into "surf". */
static void
generate_surface(SUBST_PTR subst, Special_Surface *insurf, Surface *outsurf)
{
   #ifdef DEBUG_FN_CALLS
   message("texture::generate_surface");
   #endif
   NODE_PTR tmp;
   Flt ftemp = 0.17;
   Flt Kt_scale = 0.0;
   Vec body, tvec, vvec;
   int i;

   /* Determine the default color, for use when others are not defined.
      This is typically used so that calculations of ambient and diffuse
      colors do not have to be repeated. */
   if (insurf->body_color == NULL)
      MakeVector(1.0, 1.0, 1.0, body)
   else if (eval_node(subst, insurf->body_color, &Kt_scale, body, &tmp) != 2) {
      error("Bad vector in generate_surface\n");
      return;
   }

   /* Copy body color into PT and the alpha value into w */
   if (subst != NULL) {
      VecCopy(body, subst->PT);
      subst->U[2] = Kt_scale;
      }

   /* Determining the ambient characteristics */
   if (insurf->Ka_color == NULL)
      VecCopy(body, outsurf->Ka_color)
   else if (eval_node(subst, insurf->Ka_color,
                      &ftemp, vvec, &tmp) != 2) {
      error("Bad Ka vector in generate_surface\n");
      return;
   }
   else
      VecCopy(vvec, outsurf->Ka_color)

   if (insurf->Ka_scale == NULL)
      outsurf->Ka_scale = 0.1;
   else if (eval_node(subst, insurf->Ka_scale,
                      &ftemp, tvec, &tmp) != 1) {
      error("Bad Ka scale in generate_surface\n");
      return;
      }
   else
      outsurf->Ka_scale = ftemp;

   /* Determine the brilliance power */
   if (insurf->Kb_power == NULL)
      outsurf->Kb_power = 1.0;
   else if (eval_node(subst, insurf->Kb_power,
                      &ftemp, tvec, &tmp) != 1) {
      error("Bad brilliance value in generate_surface\n");
      return;
   }
   else if (ftemp < 0.0) {
      warning("Brilliance values less than 0 not allowed (reset to 0)\n");
      outsurf->Kb_power = 1.0;
      }
   else
      outsurf->Kb_power = ftemp;

   /* Determine the diffuse characteristics */
   if (insurf->Kd_color == NULL)
      VecCopy(body, outsurf->Kd_color)
   else if (eval_node(subst, insurf->Kd_color, &ftemp,
                      vvec, &tmp) != 2) {
      error("Bad Kd vector in generate_surface\n");
      return;
   }
   else
      VecCopy(vvec, outsurf->Kd_color)
   if (insurf->Kd_scale == NULL)
      outsurf->Kd_scale = 0.8;
   else if (eval_node(subst, insurf->Kd_scale,
                      &ftemp, tvec, &tmp) != 1) {
      error("Bad scale in generate_surface\n");
      return;
   }
   else
      outsurf->Kd_scale = ftemp;

   /* Determine the specular characteristics */
   if (insurf->Ks_color == NULL)
      VecCopy(body, outsurf->Ks_color)
   else if (eval_node(subst, insurf->Ks_color,
                      &ftemp, vvec, &tmp) != 2) {
      error("Bad Ks vector in generate_surface\n");
      return;
   }
   else
      VecCopy(vvec, outsurf->Ks_color)

   if (insurf->Ks_scale == NULL)
      outsurf->Ks_scale = 0.0;
   else if (eval_node(subst, insurf->Ks_scale,
                      &ftemp, tvec, &tmp) != 1) {
      error("Bad Ks scale in generate_surface\n");
      return;
   }
   else
      outsurf->Ks_scale = ftemp;

   /* Determine the reflection characteristics */
   if (insurf->Kr_color == NULL)
      VecCopy(body, outsurf->Kr_color)
   else if (eval_node(subst, insurf->Kr_color,
                      &ftemp, vvec, &tmp) != 2) {
      error("Bad vector in generate_surface\n");
      return;
   }
   else
      VecCopy(vvec, outsurf->Kr_color)
   if (insurf->Kr_scale == NULL)
      outsurf->Kr_scale = 0.0;
   else if (eval_node(subst, insurf->Kr_scale,
                      &ftemp, tvec, &tmp) != 1) {
      error("Bad Kr scale in generate_surface\n");
      return;
   }
   else
      outsurf->Kr_scale = ftemp;

   /* Determine the transmission characteristics */
   if (insurf->Kt_color == NULL) {
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
      error("Bad Kt vector in generate_surface\n");
      return;
   }
   else
      VecCopy(vvec, outsurf->Kt_color)

   if (insurf->Kt_scale == NULL)
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
         error("Bad Kt scale in generate_surface\n");
         return;
      }
      }
   else
      outsurf->Kt_scale = ftemp;

   /* Determine the index of refraction */
   if (insurf->ior == NULL)
      outsurf->ior = 1.0;
   else if (eval_node(subst, insurf->ior, &ftemp, tvec, &tmp) != 1) {
      error("Bad ior in generate_surface\n");
      return;
   }
   else
      outsurf->ior = ftemp;

   /* Determine the microfacet distribution */
   if (insurf->D_angle == NULL)
      ftemp = 10.0;
   else if (eval_node(subst, insurf->D_angle, &ftemp, tvec, &tmp) != 1) {
      error("Bad specular angle in generate_surface\n");
      return;
   }

   ftemp = M_PI * ftemp / 180.0;
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
      error("Bad microfacet type in 'generate_surface'\n");
      return;
   }
}

/* Evaluate each component of the special surface, placing
   the results into "surf". */
static void
generate_noise_surface(Special_Surface *insurf, Noise_Surface *outsurf)
{
   NODE_PTR tmp;
   Flt ftemp;
   Vec tvec;
   Surface *local_surf = &outsurf->surf;

   generate_surface(NULL, insurf, local_surf);

   if (insurf->body_color == NULL)
      MakeVector(1.0, 1.0, 1.0, outsurf->body_color)
   else if (eval_node(NULL, insurf->body_color, &ftemp,
                      outsurf->body_color, &tmp) != 2) {
      error("Bad color vector in generate_noise_surface\n");
      return;
   }


   /* Determine the lookup function */
   if (insurf->Pos_scale == NULL)
      outsurf->Pos_scale = 1.0;
   else if (eval_node(NULL, insurf->Pos_scale,
                      &ftemp, tvec, &tmp) != 1) {
      error("Bad position scaling value generate_noise_surface\n");
      return;
   }
   else
      outsurf->Pos_scale = ftemp;

   /* Determine the lookup function */
   if (insurf->Lookup_fn == NULL)
      outsurf->Lookup_fn = 0;
   else if (eval_node(NULL, insurf->Lookup_fn, &ftemp, tvec, &tmp) != 1 ||
            ftemp < 0.0 || ftemp > 4.0) {
      error("Bad lookup function generate_noise_surface\n");
      return;
   }
   else
      outsurf->Lookup_fn = (int)ftemp;

   /* Determine the normal modifier function */
   if (insurf->normal == NULL)
      outsurf->N_modifier = 0;
   else if (eval_node(NULL, insurf->normal, &ftemp, tvec, &tmp) != 1 ||
            ftemp < 0.0 || ftemp > 3.0) {
      error("Bad normal modifier function generate_noise_surface\n");
      return;
   }
   else
      outsurf->N_modifier = (int)ftemp;

   /* Octaves of noise to use */
   if (insurf->Octaves == NULL)
      outsurf->Octaves = 1;
   else if (eval_node(NULL, insurf->Octaves, &ftemp, tvec, &tmp) != 1 ||
            ftemp < 1.0) {
      error("Bad Octaves value in generate_noise_surface\n");
      return;
   }
   else
      outsurf->Octaves = (int)ftemp;

   /* Frequency of ripple/wave */
   if (insurf->Frequency == NULL)
      outsurf->Frequency = 1.0;
   else if (eval_node(NULL, insurf->Frequency, &ftemp, tvec, &tmp) != 1 ||
            ftemp <= 0.0) {
      error("Bad Frequency value in generate_noise_surface\n");
      return;
   }
   else
      outsurf->Frequency = ftemp;

   /* Phase offset for ripples */
   if (insurf->Phase == NULL)
      outsurf->Phase = 0.0;
   else if (eval_node(NULL, insurf->Phase, &ftemp, tvec, &tmp) != 1) {
      error("Bad Phase value in generate_noise_surface\n");
      return;
   }
   else
      outsurf->Phase = ftemp;

   /* Amount of contribution of normal modifier */
   if (insurf->Bump_scale == NULL)
      outsurf->Bump_scale = 1.0;
   else if (eval_node(NULL, insurf->Bump_scale, &ftemp, tvec, &tmp) != 1) {
      error("Bad Bump_scale value in generate_noise_surface\n");
      return;
   }
   else
      outsurf->Bump_scale = ftemp;

   /* Determine the position modifier function */
   if (insurf->Position_fn == NULL)
      outsurf->Position_fn = 0;
   else if (eval_node(NULL, insurf->Position_fn, &ftemp, tvec, &tmp) != 1 ||
            ftemp < 0.0 || ftemp > 5.0) {
      error("Bad position modifier function in generate_noise_surface\n");
      return;
   }
   else
      outsurf->Position_fn = (int)ftemp;

   /* Amount of turbulence */
   if (insurf->Turbulence == NULL)
      outsurf->Turbulence = 0.0;
   else if (eval_node(NULL, insurf->Turbulence,
                      &ftemp, tvec, &tmp) != 1) {
      error("Bad Turbulence value generate_noise_surface\n");
      return;
   }
   else
      outsurf->Turbulence = ftemp;

   outsurf->map = insurf->map;
}

static Surface *
eval_plain(Viewpoint *Eye, Object *obj, Texture *text,
           Vec W, Vec P, Vec N, Vec I,
           float u, float v, int level)
{
   #ifdef DEBUG_FN_CALLS
   message("texture::eval_plain");
   #endif
   return (Surface *)(text->data);
}

static void
delete_plain(Texture *text)
{
   if ((Surface *)text->data != &DefaultSurface)
      polyray_free(text->data);
}

void
create_plain(Texture *texture, Special_Surface *surf)
{
   #ifdef DEBUG_FN_CALLS
   message("texture::create_plain");
   #endif
   /* Have to be able to evaluate all of the components of the parsed
      surface, and generate predefined surface values. */
   Surface *new_surf = (Surface *)polyray_malloc(sizeof(Surface));
   generate_surface(NULL, surf, new_surf);
   delete_special0(surf);
   texture->type   = T_PLAIN;
   texture->eval   = eval_plain;
   texture->del    = delete_plain;
   texture->data   = new_surf;
}

static Surface *
eval_checker(Viewpoint *Eye, Object *obj, Texture *text,
             Vec W, Vec P, Vec N, Vec I,
             float u, float v, int level)
{
   int temp;
   Vec VP, VW;
   Checker *check = text->data;

   if (text->t_trans != NULL) {
      /* Transform the point into texture space */
      InvTxVector1(VP, P, text->t_trans);
      InvTxVector1(VW, W, text->t_trans);
      }
   else {
      VecCopy(P, VP);
      VecCopy(W, VW);
      }
   temp = (int)floor(VP[0]-EPSILON) + (int)floor(VP[1]-EPSILON) +
          (int)floor(VP[2]-EPSILON);
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

static void
delete_checker(Texture *texture)
{
   Checker *check = texture->data;
   TextureDelete(check->text1);
   TextureDelete(check->text2);
   polyray_free(check);
}

void
create_checker(Texture *texture, Texture *text1, Texture *text2)
{
   Checker *check = polyray_malloc(sizeof(Checker));
   if (check == NULL) {
      error("Failed to allocate a checker texture\n");
      return;
   }
   /* check->repeat_flag = Flag; */
   check->text1 = text1;
   check->text2 = text2;
   check->repeat_flag1 = 0;
   check->repeat_flag2 = 0;
   texture->type   = T_CHECKER;
   texture->eval   = eval_checker;
   texture->del    = delete_checker;
   texture->data   = check;
}

static Surface *
eval_hexagon(Viewpoint *Eye, Object *obj, Texture *text,
             Vec W, Vec P, Vec N, Vec I,
             float u, float v, int level)
{
   long temp, x0, x1, y0, y1, xh, yh;
   Flt x, y, xt, yt;
   Vec VP, VW;
   Hexagon *hex = text->data;
   if (text->t_trans != NULL) {
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
      error("Bad hex index: %d\n", temp);
      return 0;
   }
}

static void
delete_hexagon(Texture *texture)
{
   Hexagon *hex = texture->data;
   TextureDelete(hex->text1);
   TextureDelete(hex->text2);
   TextureDelete(hex->text3);
   polyray_free(hex);
}

void
create_hexagon(Texture *texture, Texture *text1, Texture *text2, Texture *text3)
{
   Hexagon *hex = polyray_malloc(sizeof(Hexagon));
   if (hex == NULL) {
      error("Failed to allocate a hexagon texture\n");
      return;
   }
   /* hex->repeat_flag = Flag; */
   hex->text1 = text1;
   hex->text2 = text2;
   hex->text3 = text3;
   hex->repeat_flag1 = 0;
   hex->repeat_flag2 = 0;
   hex->repeat_flag3 = 0;
   texture->type   = T_HEXAGON;
   texture->eval   = eval_hexagon;
   texture->del    = delete_hexagon;
   texture->data   = hex;
}

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
   new_spec->map = NULL;
}

static Surface *
eval_special(Viewpoint *Eye, Object *obj, Texture *text,
             Vec W, Vec P, Vec N, Vec I,
             float u, float v, int level)
{
   #ifdef DEBUG_FN_CALLS
   message("texture::eval_special");
   #endif
   Special_Surface *special = text->data;
   Surface *new_surf = &((*special).surf);
   struct subst_struct subst;
   Vec WV, PV, NV;
   Flt F;
   NODE_PTR tnode;

   if (text->t_trans != NULL) {
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
   if (special->position != NULL) {
      if (eval_node(&subst, special->position, &F, PV, &tnode) != 2) {
         error("Bad position vector in eval_surface\n");
         return new_surf;
      }
      VecCopy(PV, subst.P);
      }

   /* If there is a normal modifying function then evaluate it */
   if (special->normal != NULL) {
      if (eval_node(&subst, special->normal, &F, NV, &tnode) != 2) {
         error("Bad normal vector in eval_surface: <%g, %g, %g>\n", NV);
         return new_surf;
      }
      VecNormalize(NV);
      VecCopy(NV, N);
      VecCopy(NV, subst.N);
      }

   generate_surface(&subst, special, new_surf);

   return new_surf;
}

static void
delete_special(Texture *texture)
{
   delete_special0((Special_Surface *)texture->data);
}

void
create_special(Texture *texture, void *data)
{
   #ifdef DEBUG_FN_CALLS
   message("texture::create_special");
   #endif
   Special_Surface *spec = (Special_Surface *)data;
   texture->type   = T_SPECIAL;
   texture->eval   = eval_special;
   texture->del    = delete_special;
   texture->data   = data;
   memcpy(&spec->surf, &DefaultSurface, sizeof(Surface));
}

static Surface *
eval_noise(Viewpoint *Eye, Object *obj, Texture *text,
           Vec W, Vec P, Vec N, Vec I,
           float u, float v, int level)
{
   Noise_Surface *noise_surf = text->data;
   Surface *new_surf = &noise_surf->surf;
   Vec WV, PV, ND, body;
   Flt ind, pos, nval, inter0, inter1;
   map_entries tmp = noise_surf->map;
   int cflag;

   if (text->t_trans != NULL)
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
      if (pos < EPSILON)
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
   while (tmp != NULL && !cflag) {
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

static void
delete_noise(Texture *text)
{
   Noise_Surface *noise_surf = text->data;
   map_entries map, temp;
   /* The color map is the only dynamically allocated piece. */
   map = noise_surf->map;
   while (map != NULL) {
      temp = map;
      map = map->next;
      polyray_free(temp);
      }
   polyray_free(noise_surf);
}

void
create_noise(Texture *texture, Special_Surface *surf)
{
   Noise_Surface *new_surf =
      (Noise_Surface *)polyray_malloc(sizeof(Noise_Surface));
   memcpy(&new_surf->surf, &DefaultSurface, sizeof(Surface));
   generate_noise_surface(surf, new_surf);
   delete_special0(surf);
   texture->type   = T_NOISE;
   texture->eval   = eval_noise;
   texture->del    = delete_noise;
   texture->data   = new_surf;
}

static Surface *
eval_layered(Viewpoint *Eye, Object *obj, Texture *text,
             Vec W, Vec P, Vec N, Vec I,
             float u, float v, int level)
{
   #ifdef DEBUG_FN_CALLS
   message("texture::eval_layered");
   #endif
   Vec VP, color, temp_color;
   Flt surf_alpha, old_kt;
   Layered *layer = text->data;
   tstackptr texts;
   Surface *surf;
   Vec *LC;

   /* Transform the point into texture space */
   if (text->t_trans != NULL)
      InvTxVector1(VP, P, text->t_trans)
   else
      VecCopy(P, VP)

   if (Shadow_Test) {
      /* During shadowing, all we will do is determine the amount of
         transparency left at the bottom layer.  The only filtering will
         also only be from the bottom layer. */
      layer->surf.Ka_scale = 0.0;
      surf_alpha = 1.0;
      for (texts=layer->texts;texts->next!=NULL;texts=texts->next) {
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
   LC = (Vec *)polyray_malloc(nLights * sizeof(Vec));
   if (LC == NULL) {
      error("Failed to allocate space for light information\n");
      return NULL;
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
   while (texts != NULL && surf_alpha > 1.0e-3) {
      surf = (texts->element->eval)(Eye, obj, texts->element,
                                    W, VP, N, I, u, v, level);
      if (texts->next != NULL) {
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

static void
delete_layered(Texture *texture)
{
   tstackptr temp, last;
   Layered *layer = texture->data;

   if (!layer->copy_flag) {
      /* Delete the component layers */
      temp = layer->texts;
      while (temp != NULL) {
         TextureDelete(temp->element);
         last = temp;
         temp = temp->next;
         polyray_free(last);
         }
      }
   polyray_free(layer);
}

void
create_layered(Texture *texture, tstackptr texts)
{
   #ifdef DEBUG_FN_CALLS
   message("texture::create_layered");
   #endif
   Layered *layer = polyray_malloc(sizeof(Layered));
   Surface *surf;

   if (layer == NULL) {
      error("Failed to allocate a layered texture\n");
      return;
   }
   layer->copy_flag = 0;
   layer->texts     = texts;
   texture->type    = T_LAYERED;
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
   surf->D = NULL;
   surf->D_coeff = 1.0;
   surf->ior = 1.0;

   texture->data   = layer;
}

void
delete_texture_map(texture_map_entries map)
{
   texture_map_entries temp1, temp2;
   for (temp1=map;temp1!=NULL;) {
      TextureDelete(temp1->t0);
      TextureDelete(temp1->t1);
      temp2 = temp1;
      temp1 = temp1->next;
      polyray_free(temp2);
      }
}

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
   Indexed *text = tex->data;

   /* Transform the point into texture space */
   if (tex->t_trans != NULL) {
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
      error("Bad texture index function\n");
      return NULL;
   }

   /* Determine which two textures are contributing to the overall texture. */
   cflag = 0;
   tmp = text->texts;
   while (tmp != NULL && !cflag) {
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
      return &DefaultSurface;

   if (Shadow_Test) {
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
      ShadeSurface(Eye, obj, surf, level, 1.0, 1.0, I, W, NT, c0, NULL);
      VecCopy(N, NT);
      surf = (thigh->eval)(Eye, obj, thigh, W, PV, NT, I, u, v, level);
      ShadeSurface(Eye, obj, surf, level, 1.0, 1.0, I, W, NT, c1, NULL);
      surf = &text->surf;
      surf->Kt_scale = 0.0;
      surf->Ka_scale = 1.0;
      VecComb(inter1, c0, inter0, c1, surf->Ka_color);
      }

   return surf;
}

static void
delete_indexed(Texture *texture)
{
   Indexed *text = texture->data;

   if (!text->copy_flag) {
      delete_texture_map(text->texts);
      deallocate_node(text->exper);
      }
   polyray_free(text);
}

void
create_indexed(Texture *texture, NODE_PTR exper, texture_map_entries texts)
{
   Indexed *text = polyray_malloc(sizeof(Indexed));
   Surface *surf;

   if (text == NULL) {
      error("Failed to allocate an indexed texture\n");
      return;
   }
   text->copy_flag = 0;
   text->exper     = exper;
   text->texts     = texts;
   texture->type   = T_INDEXED;
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
   surf->D = NULL;
   surf->D_coeff = 1.0;
   surf->ior = 1.0;
}

static void
delete_texture_fns(texture_fn_entries fns)
{
   texture_fn_entries temp1, temp2;
   for (temp1=fns;temp1!=NULL;) {
      deallocate_node(temp1->fn);
      TextureDelete(temp1->t0);
      temp2 = temp1;
      temp1 = temp1->next;
      polyray_free(temp2);
      }
}

static Surface *
eval_summed(Viewpoint *Eye, Object *obj, Texture *text,
            Vec W, Vec P, Vec N, Vec I, float u, float v, int level)
{
   NODE_PTR n0;
   Vec p0, PV, WV, color, surf_color;
   Flt f0, surf_alpha;
   struct subst_struct subst;
   Summed *stex = text->data;
   texture_fn_entries texts;
   Surface *surf;
   Vec *LC;

   /* Transform the point into texture space */
   if (text->t_trans != NULL) {
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

   if (Shadow_Test) {
      /* Simple sum of all transparency components of each layer */
      surf_alpha = 1.0;
      MakeVector(0, 0, 0, surf_color);
      for (texts=stex->texts;texts->next!=NULL;texts=texts->next) {
         /* Evaluate the index function */
         if (eval_node(&subst, texts->fn, &f0, p0, &n0) != 1) {
            error("Bad texture sum function\n");
            return NULL;
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
   LC = (Vec *)polyray_malloc(nLights * sizeof(Vec));
   if (LC == NULL) {
      error("Failed to allocate space for light information\n");
      return NULL;
   }
   else
      Get_Light_Colors(Eye, W, LC);

   /* The way we do the evaluation is by successive calls to "Shade" - as
      long as there is a little alpha left in the current texture, we keep
      evaluating.  The result is stored as a surface that only has an
      ambient component. */
   MakeVector(0, 0, 0, color);
   texts=stex->texts;
   while (texts != NULL) {
      if (eval_node(&subst, texts->fn, &f0, p0, &n0) != 1) {
         error("Bad texture sum function\n");
         return NULL;
      }
      if (fabs(f0) > EPSILON) {
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

static void
delete_summed(Texture *texture)
{
   Summed *tex = texture->data;

   if (!tex->copy_flag)
      delete_texture_fns(tex->texts);
   polyray_free(tex);
}

void
create_summed(Texture *texture, texture_fn_entries texts)
{
   Summed *tex = polyray_malloc(sizeof(Summed));
   Surface *surf;

   if (tex == NULL) {
      error("Failed to allocate a summed texture\n");
      return;
   }
   tex->copy_flag = 0;
   tex->texts     = texts;
   texture->type    = T_SUMMED;
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
   surf->D = NULL;
   surf->D_coeff = 1.0;
   surf->ior = 1.0;

   texture->data   = tex;
}

void
TextureCopy(Texture *in_texture, Texture *out_texture)
{
   /* Copy all the default stuff */
   memcpy(out_texture, in_texture, sizeof(Texture));

   /* Copy the transform. */
   if (in_texture->t_trans != NULL) {
      Transform *tmptrn = (Transform *)polyray_malloc(sizeof(Transform));
      if (tmptrn == NULL) {
         error("Failed to allocate a transform\n");
         return;
      }
      memcpy(tmptrn, in_texture->t_trans, sizeof(Transform));
      out_texture->t_trans = tmptrn;
      }

   /* Indicate that this is a copied texture */
   out_texture->copy_flag = 1;
}

void
TextureDelete(Texture *texture)
{
   if (!texture->copy_flag)
      (texture->del)(texture);
   if (texture->t_trans != NULL)
      polyray_free(texture->t_trans);
   polyray_free(texture);
}

/* Transformation functions for textures */
void
TextureShear(Texture *text, Flt xy, Flt xz, Flt yx, Flt yz,
             Flt zx, Flt zy)
{
   Transform trans;
   Get_Shear_Transformation(&trans, xy, xz, yx, yz, zx, zy);
   if (text->t_trans == NULL) text->t_trans = Get_Transformation();
   Compose_Transformations(text->t_trans, &trans);
}

void
TextureTranslate(Texture *text, Vec Vector)
{
   Transform trans;
   Get_Translation_Transformation(&trans, Vector);
   if (text->t_trans == NULL) text->t_trans = Get_Transformation();
   Compose_Transformations(text->t_trans, &trans);
}

void
TextureRotate(Texture *text, Vec v)
{
   Transform trans;
   Vec vt;
   VecCopy(v, vt);
   VecScale(M_PI/180.0, vt);
   Get_Rotation_Transformation(&trans, vt);
   if (text->t_trans == NULL) text->t_trans = Get_Transformation();
   Compose_Transformations(text->t_trans, &trans);
}

void
TextureAxisRotate(Texture *text, Vec v, Flt ang)
{
   Transform trans;

   Get_Rotate_Transform(&trans, v, M_PI * ang / 180.0);
   if (text->t_trans == NULL) text->t_trans = Get_Transformation();
   Compose_Transformations(text->t_trans, &trans);
}

void
TextureScale(Texture *text, Vec Vector)
{
   Transform trans;
   Get_Scaling_Transformation (&trans, Vector);
   if (text->t_trans == NULL) text->t_trans = Get_Transformation();
   Compose_Transformations(text->t_trans, &trans);
}

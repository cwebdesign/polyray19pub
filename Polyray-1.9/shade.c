/* shade.c

   Do the lighting equation

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
#include "light.h"
#include "trace.h"
#include "symtab.h"
#include "intersec.h"
#include "shade.h"
#include "vector.h"

Surface *
find_surface(Viewpoint *Eye, Object *obj, Texture *texture,
             Vec W, Vec N, Vec I, fVec U, int level)
{
   Surface *surf;
   Vec P;

   if (texture != NULL) {
      if (obj->o_trans != NULL)
         InvTxVector1(P, W, obj->o_trans)
      else
         VecCopy(W, P)
      surf = (texture->eval)(Eye, obj, texture, W, P, N, I, U[0], U[1], level);
      if (surf != NULL)
         return surf;
      else
         return &DefaultSurface;
      }
   else if (obj->o_texture != NULL) {
      if (obj->o_trans != NULL)
         InvTxVector1(P, W, obj->o_trans)
      else
         VecCopy(W, P)
      surf = (obj->o_texture->eval)
                (Eye, obj, obj->o_texture, W, P, N, I, U[0], U[1], level);
      if (surf != NULL)
         return surf;
      else
         return &DefaultSurface;
      }
   else if (obj->o_parent != NULL) {
      return find_surface(Eye, obj->o_parent, NULL, W, N, I, U, level);
      }
   else {
      return &DefaultSurface;
      }
}

void
ShadeSurface(Viewpoint *Eye, Object *obj, Surface *surf, int level,
             Flt weight, Flt ior, Vec I, Vec W, Vec N,
             Vec col, Vec *light_colors)
{
   Ray tray;
   Vec tcol;
   Vec V, L, NN, SV;
   Flt d, t, tmin, diff, spec, intensity, new_ior, radius;
   int i, j;
   Vec Kd_color, Ks_color, Kt_color, Kr_color;
   Flt Kd_scale, Ks_scale, Kt_scale, Kr_scale;
   Flt topacity;
   Vec light_pos, light_color;
   Light *light;

   VecCopy(I, V);
   VecNegate(V);
   VecNormalize(N);

   /* Ambient contribution */
   VecCopy(surf->Ka_color, col);
   VecScale(surf->Ka_scale, col);

   VecCopy(surf->Kd_color, Kd_color);
   VecCopy(surf->Ks_color, Ks_color);
   Kd_scale = surf->Kd_scale;
   Ks_scale = surf->Ks_scale;
   Kr_scale = surf->Kr_scale;
   Kt_scale = surf->Kt_scale;

   /* Calculate the contribution from reflected and refracted directions
      respectively */
   if (Kr_scale > 0.0 || Kt_scale > 0.0) {
      /* For the reflect/refract code to work, the normal must be
         oriented to point towards the direction the ray is coming from */
      VecCopy(surf->Kt_color, Kt_color);
      VecCopy(surf->Kr_color, Kr_color);
      VecCopy(N, NN);
      if (VecDot(I, NN) >= 0.0) {
         VecNegate(NN);
         }
      VecCopy(W, tray.P);
      /* Specular contributions from reflected direction (no opacity) */
      if ((Global_Shade_Flag & REFLECT_CHECK &&
           obj->o_sflag & REFLECT_CHECK) &&
          (t = Kr_scale * weight) > minweight) {
         SpecularDirection(V, NN, tray.D);
         Trace(Eye, level + 1, t, &tray, tcol, &topacity, ior, &nReflected);
     /*    for (i=0;i<3;i++) */
         for (i = 2; i>=0; i--) /* optimization from http://www.rt.e-technik.tu-darmstadt.de/~georg/djgpp/optimization.html */
             col[i] += Kr_scale * Kr_color[i] * tcol[i];
         }

      /* Specular contributions from transmitted direction */
      if ((Global_Shade_Flag & TRANSMIT_CHECK &&
           obj->o_sflag & TRANSMIT_CHECK) &&
          (t = surf->Kt_scale * weight) > minweight) {
         new_ior = surf->ior;
         if (ior == 1.0) {
            if (new_ior == 1.0) {
               /* No bending of light here */
               VecCopy(I, tray.D);
               Trace(Eye, level+1, t, &tray, tcol, &topacity, 1.0, &nRefracted);
               }
            else if (TransmissionDirection(1.0, new_ior, I, NN, tray.D))
               /* Refraction as the ray enters the object */
               Trace(Eye, level+1, t, &tray, tcol, &topacity, new_ior, &nRefracted);
            else {
               /* Total internal reflection */
               SpecularDirection(V, NN, tray.D);
               Trace(Eye, level+1, t, &tray, tcol, &topacity, ior, &nTIR);
               }
            }
         else if (TransmissionDirection(new_ior, 1.0, I, NN, tray.D))
            /* Refraction as the ray exits the object */
            Trace(Eye, level + 1, t, &tray, tcol, &topacity, 1.0, &nRefracted);
         else {
            /* Total internal reflection */
            SpecularDirection(V, NN, tray.D);
            Trace(Eye, level + 1, t, &tray, tcol, &topacity, ior, &nTIR);
            }
         /* for (i=0;i<3;i++)*/
         for (i = 2;i>=0;i--)
             col[i] += Kt_scale * Kt_color[i] * tcol[i];
         }
      }

   if (Kd_scale != 0.0 || Ks_scale != 0.0) {
      for (light=Lights,j=0;light!=NULL;light=light->next,j++) {
         VecCopy(W, tray.P);
//	printf("Light type is %d\n",light->type);
         intensity = Light_Color(light, W, light_color, light_pos, &radius);
         if (ABS(intensity) < EPSILON)
            continue;
         MakeVector(1.0, 1.0, 1.0, SV);
         nShadows++;
         #ifdef DEBUG
         printf("\nnShadows in shade=%ul\n",nShadows);
         printf("press enter\n");getchar();
         #endif
         VecSub(light_pos, W, L);
         t = VecNormalize(L);
         if ((d = VecDot(N, L)) <= 0) {
            if ((Global_Shade_Flag & NORMAL_CORRECT) &&
              (obj->o_sflag & NORMAL_CORRECT)) {
               d = -d;
               VecNegate(N);
               }
            else
               /* No contribution of diffuse from the backside */
               d = 0.0;
            }

         if (VecDot(N, V) < 0.0)
            if (!(Global_Shade_Flag & TWO_SIDED_SURFS) ||
                !(obj->o_sflag & TWO_SIDED_SURFS))
               continue;

         VecCopy(L, tray.D);
         if (Rendering_Method == SCAN_CONVERSION)
            /* The polygons can be pretty far from the real surface,
               add in a big interval before looking for intersections. */
            tmin = 0.1;
         else
            tmin = rayeps;
         if (light_colors != NULL || !(obj->o_sflag & SHADOW_CHECK) ||
             !light->flags || Shadow(Eye, light, &tray, tmin, t, radius, SV)) {
            if (Kd_scale != 0.0) {
               /* Diffuse contributions from light sources */
               diff = intensity * d * Kd_scale;
               /*for (i=0;i<3;i++)*/
               for (i=2;i>=0;i--)
                  if (light_colors)
                     col[i] += light_colors[j][i] * diff * Kd_color[i];
                  else
                     col[i] += SV[i] * diff * Kd_color[i] * light_color[i];
               }
            if (Ks_scale != 0.0) {
               /* Specular contributions from light sources */
               spec = surf->D(N, L, V, surf->D_coeff) * Ks_scale * intensity;
               /*for (i=0;i<3;i++)*/
               for (i=2;i>=0;i--)
                  if (light_colors)
                     col[i] += light_colors[j][i] * spec * Ks_color[i];
                  else
                     col[i] += SV[i] * spec * Ks_color[i] * light_color[i];
               }
            }
         }
      }
}

void
Shade(Viewpoint *Eye, Object *obj, Texture *texture, int level,
      Flt weight, Flt ior, Vec I, Vec W, Vec N, Vec U, Vec col) 
{
   Surface *surf;
   fVec U0;

   if (obj->o_type == T_HYPERTEXTURE) {
      VecCopy(U, col)
      }
   else {
      VecNormalize(N);
      VecCopy(U, U0);

      /* Get the pointer to the surface that needs shading */
      surf = find_surface(Eye, obj, texture, W, N, I, U0, level);

      ShadeSurface(Eye, obj, surf, level, weight, ior, I, W, N, col, NULL);
      }
}

/***********************************************************************
 * SpecularDirection(V, N, R)
 * 
 * Given a view vector V, and the normal N, calculate the 
 * direction of the reflected ray R.
 ***********************************************************************/
void
SpecularDirection(Vec V, Vec N, Vec R)
{
   Flt nv = 2.0 * VecDot(V, N);

   VecComb(nv, N, -1.0, V, R);
   VecNormalize(R);
}

/***********************************************************************
 * TransmissionDirection(m1, m2, I, N, T)
 *
 * calculates the direction of the transmitted ray
 ***********************************************************************/
int
TransmissionDirection(Flt n1, Flt n2, Vec I, Vec N, Vec T)
{
   Flt eta, c1, cs2;

   eta = n1/n2;
   c1 = -VecDot(I,N);
   cs2 = 1.0 - eta * eta*(1.0 - c1*c1);
   if (cs2 < 0.0) return 0;
   VecComb(eta, I, eta*c1-sqrt(cs2), N, T);
   return 1;
}


/* shade.cc

   Do the lighting equation

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
#include "light.h"
#include "trace.h"
#include "runtime_state.h"
#include "symtab.h"
#include "intersec.h"
#include "shade.h"
#include "vector.h"
#include "io_ply.h"

/** @brief Resolve the effective surface for shading at a point.
 *  @param Eye      Active viewpoint.
 *  @param obj      Object being shaded.
 *  @param texture  Optional texture override.
 *  @param W        World-space hit position.
 *  @param N        Surface normal at the hit point.
 *  @param I        Incident ray direction.
 *  @param U        Surface UV coordinates.
 *  @param level    Current recursion level.
 *  @return Pointer to the resolved surface, or the global default surface when none is found.
 */
Surface *find_surface(Viewpoint *Eye, Object *obj, Texture *texture,
             Vec W, Vec N, Vec I, const fVec U, const int level)
{
   #ifdef DEBUG_FN_CALLS
   smessage("shade::find_surface\n");
   #endif
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
         return &RuntimeState::DefaultSurface;
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
         return &RuntimeState::DefaultSurface;
      }
   else if (obj->o_parent != NULL) {
      return find_surface(Eye, obj->o_parent, NULL, W, N, I, U, level);
      }
   else {
      return &RuntimeState::DefaultSurface;
      }
}

/** @brief Evaluate the lighting equation for a resolved surface.
 *  @param Eye           Active viewpoint.
 *  @param obj           Object being shaded.
 *  @param surf          Surface properties to evaluate.
 *  @param level         Current recursion level.
 *  @param weight        Contribution weight for recursion termination.
 *  @param ior           Current index of refraction of the medium.
 *  @param I             Incident ray direction.
 *  @param W             World-space hit position.
 *  @param N             Surface normal at the hit point.
 *  @param col           Output accumulated color.
 *  @param light_colors  Optional per-light color overrides, or nullptr to use light colors directly.
 *  @return No return value.
 */
void
ShadeSurface(Viewpoint *Eye, Object *obj, Surface *surf, int level,
             Flt weight, Flt ior, Vec I, Vec W, Vec N,
             Vec col, Vec *light_colors)
    //we are returning a value in col. but using C interface still at this time
{
   #ifdef DEBUG_FN_CALLS
   smessage("shade::ShadeSurface\n");
   #endif
   Ray tray;
   NuVec tcol{};
   Vec V, L, NN, SV;
   Flt d, t, tmin, diff, spec, intensity, new_ior, radius;
   int i, j;
   Vec Kd_color, Ks_color, Kt_color, Kr_color;
   Flt Kd_scale, Ks_scale, Kt_scale, Kr_scale;
   Flt topacity;
   Vec light_pos, light_color;
   Light *light;

   VecCopy(I, V);
#ifdef DEBUG_FN_CALLS
   printf("\nBEFORE N-[0]=%f [1]=%f [2]=%f\n", N[0], N[1], N[2]);
   printf("BEFORE V-[0]=%f [1]=%f [2]=%f\n", V[0], V[1], V[2]);
#endif
   VecNegate(V);
   VecNormalize(N);
#ifdef DEBUG_FN_CALLS
   printf("AFTER N-[0]=%f [1]=%f [2]=%f\n", N[0], N[1], N[2]);
   printf("AFTER V-[0]=%f [1]=%f [2]=%f\n", V[0], V[1], V[2]);
 #endif

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
      if ((runtimeState::scene.Global_Shade_Flag & REFLECT_CHECK &&
           obj->o_sflag & REFLECT_CHECK) &&
          (t = Kr_scale * weight) > RuntimeState::settings.minweight) {
         SpecularDirection(V, NN, tray.D);
         Trace(Eye, level + 1, t, &tray, tcol, &topacity, ior, 
             &runtimeState::stats.nReflected);
        for (i=0;i<3;i++) 
         //for (i = 2; i>=0; i--) /* optimization from http://www.rt.e-technik.tu-darmstadt.de/~georg/djgpp/optimization.html */
             col[i] += Kr_scale * Kr_color[i] * tcol[i];
         }

      //std::cout << "SPEC col[0]=" << col[0] << " col[1]=" << col[1] << " col[2]=" << col[2] << "\n";
      /* Specular contributions from transmitted direction */
      if ((runtimeState::scene.Global_Shade_Flag & TRANSMIT_CHECK &&
           obj->o_sflag & TRANSMIT_CHECK) &&
          (t = surf->Kt_scale * weight) > runtimeState::settings.minweight) {
         new_ior = surf->ior;
         if (ior == 1.0) {
            if (new_ior == 1.0) {
               /* No bending of light here */
               VecCopy(I, tray.D);
               Trace(Eye, level+1, t, &tray, tcol, &topacity, 1.0, 
                   &runtimeState::stats.nRefracted);
               }
            else if (TransmissionDirection(1.0, new_ior, I, NN, tray.D))
               /* Refraction as the ray enters the object */
               Trace(Eye, level+1, t, &tray, tcol, &topacity, new_ior, 
                   &runtimeState::stats.nRefracted);
            else {
               /* Total internal reflection */
               SpecularDirection(V, NN, tray.D);
               Trace(Eye, level+1, t, &tray, tcol, &topacity, ior, 
                   &runtimeState::stats.nTIR);
               }
            }
         else if (TransmissionDirection(new_ior, 1.0, I, NN, tray.D))
            /* Refraction as the ray exits the object */
            Trace(Eye, level + 1, t, &tray, tcol, &topacity, 1.0, 
                &runtimeState::stats.nRefracted);
         else {
            /* Total internal reflection */
            SpecularDirection(V, NN, tray.D);
            Trace(Eye, level + 1, t, &tray, tcol, &topacity, ior, 
                &runtimeState::stats.nTIR);
            }
         for (i=0;i<3;i++)
             col[i] += Kt_scale * Kt_color[i] * tcol[i];
         }
      }

#ifdef DEBUG_FN_CALLS
   std::cout << "TOTINREFL col[0]=" << col[0] << " col[1]=" << col[1] << " col[2]=" << col[2] << "\n";
#endif
   if (Kd_scale != 0.0 || Ks_scale != 0.0) {
      for (light=RuntimeState::Lights,j=0;light!=NULL;light=light->next,j++) {
         VecCopy(W, tray.P);
//	printf("Light type is %d\n",light->type);
         intensity = Light_Color(light, W, light_color, light_pos, &radius);
         if (ABS(intensity) < PLY_EPSILON)
            continue;
         MakeVector(1.0, 1.0, 1.0, SV);
         runtimeState::stats.nShadows++;
         #ifdef DEBUG
         printf("\nnShadows in shade=%lu\n",runtimeState::stats.nShadows);
         printf("press enter\n");//getchar();
         #endif
         VecSub(light_pos, W, L);
         t = VecNormalize(L);
         if ((d = VecDot(N, L)) <= 0) {
            if ((runtimeState::scene.Global_Shade_Flag & NORMAL_CORRECT) &&
              (obj->o_sflag & NORMAL_CORRECT)) {
               d = -d;
               VecNegate(N);
               }
            else
               /* No contribution of diffuse from the backside */
               d = 0.0;
            }
         
#ifdef DEBUG_FN_CALLS
         std::cout << "N [0]=" << N[0] << " N[1]=" << N[1] << " [2]=" << N[2] << "\n";
#endif
         if (VecDot(N, V) < 0.0)
            if (!(runtimeState::scene.Global_Shade_Flag & TWO_SIDED_SURFS) ||
                !(obj->o_sflag & TWO_SIDED_SURFS))
               continue;

         VecCopy(L, tray.D);
         if (runtimeState::settings.Render_Method == rmode::SCAN_CONVERSION)
            /* The polygons can be pretty far from the real surface,
               add in a big interval before looking for intersections. */
            tmin = 0.1;
         else
            tmin = RuntimeState::rayeps;
         bool check = light_colors != nullptr || !(obj->o_sflag & SHADOW_CHECK);
         check = check || !light->flags;
         ShadRet SV{1, {1.0, 1.0, 1.0}};
         if (!check) SV = Shadow(Eye, light, &tray, tmin, t, radius);
         if (check || SV.i) {
            if (Kd_scale != 0.0) {
               /* Diffuse contributions from light sources */
               diff = intensity * d * Kd_scale;
               /*for (i=0;i<3;i++)*/
               for (i=2;i>=0;i--)
                  if (light_colors)
                     col[i] += light_colors[j][i] * diff * Kd_color[i];
                  else
                     col[i] += SV.ShadowVec[i] * diff * Kd_color[i] * light_color[i];
               }
#ifdef DEBUG_FN_CALLS      
            std::cout << "DIFF col[0]=" << col[0] << " col[1]=" << col[1] << " col[2]=" << col[2] << "\n";
#endif

            if (Ks_scale != 0.0) {
               /* Specular contributions from light sources */
               spec = surf->D(N, L, V, surf->D_coeff) * Ks_scale * intensity;
               /*for (i=0;i<3;i++)*/
               for (i=2;i>=0;i--)
                  if (light_colors)
                     col[i] += light_colors[j][i] * spec * Ks_color[i];
                  else
                     col[i] += SV.ShadowVec[i] * spec * Ks_color[i] * light_color[i];
               }
            }
         }
      }

   
#ifdef DEBUG_TESTS
   if (!AlmostEqualRelativeAndAbs(col[0], 0.1, PLY_EPSILON, PLY_EPSILON)) {
       std::cout << "col[0] failed\n";
       std::cout << "col[0]=" << col[0] << " col[1]=" << col[1] << " col[2]=" << col[2] << "\n";
       exit(4);
   }
#endif
}

/** @brief Resolve the surface and shade a point on an object.
 *  @param Eye      Active viewpoint.
 *  @param obj      Object being shaded.
 *  @param texture  Optional texture override.
 *  @param level    Current recursion level.
 *  @param weight   Contribution weight for recursion termination.
 *  @param ior      Current index of refraction of the medium.
 *  @param I        Incident ray direction.
 *  @param W        World-space hit position.
 *  @param N        Surface normal at the hit point.
 *  @param U        Surface UV coordinates.
 *  @param col      Output accumulated color.
 *  @return No return value.
 */
void
Shade(Viewpoint *Eye, Object *obj, Texture *texture, int level,
      Flt weight, Flt ior, Vec I, Vec W, Vec N, Vec U, Vec col)
{
   #ifdef DEBUG_FN_CALLS
   smessage("shade::shade\n");
   #endif
   Surface *surf;
   fVec U0;

#ifdef DEBUG_FN_CALLS
   printf("SHADE N-[0]=%f [1]=%f [2]=%f\n", N[0], N[1], N[2]);
   printf("SHADE I-[0]=%f [1]=%f [2]=%f\n", I[0], I[1], I[2]);
#endif
   if (obj->o_type == ShapeType::Hypertexture) {
      VecCopy(U, col)
      }
   else {
      VecNormalize(N);
#ifdef DEBUG_FN_CALLS
      printf("NORM N-[0]=%f [1]=%f [2]=%f\n", N[0], N[1], N[2]);
#endif
      VecCopy(U, U0);

      /* Get the pointer to the surface that needs shading */
      surf = find_surface(Eye, obj, texture, W, N, I, U0, level);
#ifdef DEBUG_FN_CALLS
      printf("SHADE N-[0]=%f [1]=%f [2]=%f\n", N[0], N[1], N[2]);
      printf("SHADE I-[0]=%f [1]=%f [2]=%f\n", I[0], I[1], I[2]);
#endif

      ShadeSurface(Eye, obj, surf, level, weight, ior, I /**/, W, N /**/, col, NULL);
      }
}

/***********************************************************************
 * SpecularDirection(V, N, R)
 *
 * Given a view vector V, and the normal N, calculate the
 * direction of the reflected ray R.
 ***********************************************************************/
/** @brief Compute the mirror-reflection direction for a view vector and normal.
 *  @param V View vector.
 *  @param N Surface normal.
 *  @param R Output reflected direction.
 *  @return No return value.
 */
void
SpecularDirection(Vec V, Vec N, Vec R)
{
   #ifdef DEBUG_FN_CALLS
   smessage("shade::SpecularDirection\n");
   #endif
   Flt nv = 2.0 * VecDot(V, N);

   VecComb(nv, N, -1.0, V, R);
   VecNormalize(R);
}

/***********************************************************************
 * TransmissionDirection(m1, m2, I, N, T)
 *
 * calculates the direction of the transmitted ray
 ***********************************************************************/
/** @brief Compute the refracted ray direction using Snell's law.
 *  @param n1 Index of refraction of the incident medium.
 *  @param n2 Index of refraction of the transmitted medium.
 *  @param I  Incident ray direction.
 *  @param N  Surface normal.
 *  @param T  Output transmitted direction.
 *  @return 1 when refraction is possible, 0 on total internal reflection.
 */
int
TransmissionDirection(Flt n1, Flt n2, Vec I, Vec N, Vec T)
{
   #ifdef DEBUG_FN_CALLS
   smessage("shade::TransmissionDirection\n");
   #endif
   Flt eta, c1, cs2;

   eta = n1/n2;
   c1 = -VecDot(I,N);
   cs2 = 1.0 - eta * eta*(1.0 - c1*c1);
   if (cs2 < 0.0) return 0;
   VecComb(eta, I, eta*c1-sqrt(cs2), N, T);
   return 1;
}

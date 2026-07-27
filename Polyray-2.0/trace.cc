/** @file trace.cc
 *  @brief Primary ray dispatch - intersection, shading, haze, and background evaluation.
 *
 *  Provides Trace() (production) and TraceTest() (test-instrumented variant).
 *  Both functions cast a ray into the scene, find the closest hit, and return
 *  an RGB colour, an opacity value, and the eye-to-hit distance.
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
#include "defs3.h"
#include "screen.h"
#include "trace.h"
#include "vector.h"
#include "io_ply.h"
#include "intersec.h"
#include "display.h"
#include "eval.h"
#include "shade.h"
#include "runtime_state.h"
#include "symtab.h"
#include "image.h"

/** @brief Cast a ray into the scene, shade the closest hit, and return its depth.
 *
 *  Recursion stops when @p level reaches RuntimeState::settings.maxlevel, at
 *  which point the background colour is used instead.  Primary rays (level 0)
 *  are clipped to the view frustum; secondary rays use rayeps and PLY_HUGE.
 *
 *  Haze is applied when Global_Haze > 0 and the hit distance exceeds
 *  Global_Haze_Start.  A procedural background expression, if set, is evaluated
 *  using normalised pixel coordinates and the ray direction.
 *
 *  @param Eye      Active viewpoint (provides frustum bounds and resolution).
 *  @param level    Current recursion depth; 0 for primary rays.
 *  @param weight   Contribution weight of this ray (used for early termination).
 *  @param ray      Ray to trace (origin @c P and direction @c D in world space).
 *  @param color    Output: RGB colour at the hit point, or background on a miss.
 *  @param opacity  Output: 1.0 on a hit or secondary miss; 0.0 on a primary miss
 *                  (signals a transparent background pixel to the compositing layer).
 *  @param ior      Index of refraction of the medium the ray is currently travelling through.
 *  @param nr       Input/output: running count of rays traced; incremented once per call.
 *  @return         Distance from the eye to the hit point, or PLY_HUGE on a miss.
 */
float Trace(Viewpoint *Eye, int level, Flt weight, Ray *ray,
      NuVec& color, Flt *opacity, Flt ior, unsigned long *nr)
{
#ifdef DEBUG
    //smessage("trace::Trace\n");
#endif
   Isect hit;
   Vec C, U;
   Flt ftemp, hither, yon, c0, c1;
   float depth;
   struct subst_struct subst;
   int i, hit_flag, old_level = runtimeState::recursion_depth;
   NODE_PTR tnode;

   //unsigned long basenr = *nr;
   hit_flag = 0;

   /* After a certain amount of recursion go to background. */
   runtimeState::recursion_depth = level;
   if (level < runtimeState::settings.maxlevel) {
      (*nr)++;

      /* If we are tracing a primary ray then clip the view frustrum */
      if (level == 0) {
         hither = Eye->view_hither;
         yon    = Eye->view_yon;
         }
      else {
         hither = RuntimeState::rayeps;
         yon    = PLY_HUGE;
         }

      /* Should the sampling be random, or not? */
      if (IntersectCPP(Eye, RuntimeState::scene.Root, ray, hither, yon, &hit))
         hit_flag = 1;
      }

   if (hit_flag) {
      VecSub(Eye->view_from, hit.W, C);
      depth = sqrt(VecDot(C, C));
      }
   else
      depth = PLY_HUGE;

   /* If we hit something then shade it, otherwise use the background */
   if (runtimeState::settings.DepthRender == 1) {
      if (hit_flag) {
         VecSub(Eye->view_from, hit.W, C);
         quantize_depth(depth, color.data(), opacity);
         }
      else
         quantize_depth((runtimeState::settings.pixelsize == 32 ? PLY_HUGE : 256.0), color.data(), opacity);
      }
   else if (hit_flag) {
      VecCopy(hit.U, U); /* Convert from fVec to Vec */
      Shade(Eye, hit.obj, hit.texture, level, weight, ior, ray->D, hit.W, hit.N, U, C);
      if (RuntimeState::Global_Haze > 0.0 && RuntimeState::Global_Haze < 1.0 &&
          hit.isect_t > RuntimeState::Global_Haze_Start) {
         c0 = pow(RuntimeState::Global_Haze, (hit.isect_t - RuntimeState::Global_Haze_Start));
         c1 = 1.0 - c0;
         VecComb(c0, C, c1, RuntimeState::Global_Haze_Color, color);
         }
      else {
         VecCopy(C, color);
         }
      *opacity = 1.0;
      }
   else if (RuntimeState::Background != nullptr) {
      c0 = (RuntimeState::current_col >= Eye->view_xres ? Eye->view_xres - 1 : RuntimeState::current_col);
      c1 = (RuntimeState::current_row >= Eye->view_yres ? Eye->view_yres - 1 : RuntimeState::current_row);
      subst.P[0] = c0 / (Flt)Eye->view_xres;
      subst.P[1] = 0.0;
      subst.P[2] = (Flt)(Eye->view_yres - c1 - 1) / (Flt)Eye->view_yres;
      MakeVector(0, 0, 0, subst.PT);
      MakeVector(subst.P[0], subst.P[2], level, subst.U);
      MakeVector(0.0, 0.0, 0.0, subst.W);
      VecCopy(ray->D, subst.N);
      MakeVector(0.0, 0.0, 0.0, subst.I);
      if ((i = eval_node(&subst, RuntimeState::Background, &ftemp, color.data(), &tnode)) != 2)
         serror("Unresolved background expression\n");
      *opacity = (level == 0 ? 0.0 : 1.0);
      }
   else {
      VecCopy(RuntimeState::BackgroundColor, color);
      *opacity = (level == 0 ? 0.0 : 1.0);
      }

   runtimeState::recursion_depth = old_level;

   //std::cout << "nR inc=" << *nr-basenr<< "\n";//1st time was equal to 1. running test0.pi
   
   return depth;
}

/** @brief Expected colour x 100000 reference values for TraceTest regression checks. */
static float colarr[] = { 37531.738f, 39093.214f, 40720.031f, 42382.175f, 44072.371f };
/** @brief Cyclic index into @c colarr, advanced each time a hit is verified. */
static int colorindx = 0;
/** @brief Last valid index into @c colarr (length - 1). */
const int maxindx = 4;

/** @brief Test-instrumented variant of Trace() with debug output and colour assertions.
 *
 *  Identical logic to Trace() but, when compiled with @c DEBUG_TEST defined,
 *  validates near/far clip distances and compares the first colour channel
 *  against the expected values in @c colarr[] using AlmostEqualRelativeAndAbs().
 *  Terminates via @c exit(4) on assertion failure.
 *
 *  Used exclusively by the GTest unit test suite; not called in production builds.
 *
 *  @param Eye      Active viewpoint.
 *  @param level    Current recursion depth; 0 for primary rays.
 *  @param weight   Contribution weight of this ray.
 *  @param ray      Ray to trace.
 *  @param color    Output: RGB colour result.
 *  @param opacity  Output: 1.0 on a hit or secondary miss; 0.0 on a primary miss.
 *  @param ior      Index of refraction of the current medium.
 *  @param nr       Input/output: running ray count; incremented once per call.
 *  @return         Distance from the eye to the hit point, or PLY_HUGE on a miss.
 */
float TraceTest(Viewpoint* Eye, int level, Flt weight, Ray* ray,
    NuVec& color, Flt* opacity, Flt ior, unsigned long* nr)
{
#ifdef DEBUG_FN_CALLS
    smessage("trace::TraceTest\n");
#endif
    Isect hit;
    Vec C, U;
    Flt ftemp, hither, yon, c0, c1;
    float depth;
    struct subst_struct subst;
    int i, hit_flag, old_level = runtimeState::recursion_depth;
    NODE_PTR tnode;


    hit_flag = 0;
#ifdef DEBUG_FN_CALLS
    std::cout << "TRACE: Ray.P=" << ray->P[0] << "," << ray->P[1] << "," << ray->P[2] << "\n";
    std::cout << "TRACE: Ray.D=" << ray->D[0] << "," << ray->D[1] << "," << ray->D[2] << "\n";
    if (runtimeState::scene.Root.slab_root != nullptr) {
        std::cout << "Root.slab_root->o_type-"
            << std::to_underlying(runtimeState::scene.Root.slab_root->o_type) << "\n";//heap use after free , here on Linux
        std::cout << "Root.slab_root->o_bnd.lower_left=" << runtimeState::scene.Root.slab_root->o_bnd.lower_left[0] << ","
            << runtimeState::scene.Root.slab_root->o_bnd.lower_left[1] << "," << runtimeState::scene.Root.slab_root->o_bnd.lower_left[2] << "\n";
    }
#endif


    /* After a certain amount of recursion go to background. */
    runtimeState::recursion_depth = level;
    if (level < runtimeState::settings.maxlevel) {
        (*nr)++;

        /* If we are tracing a primary ray then clip the view frustrum */
        if (level == 0) {
            hither = Eye->view_hither;
            yon = Eye->view_yon;
        }
        else {
            hither = RuntimeState::rayeps;
            yon = PLY_HUGE;
        }
#ifdef DEBUG_FN_CALLS
        std::cout << "hither=" << hither << " yon=" << yon << "\n";
        std::cout << "Ray=<" << ray->P[0] << "," << ray->P[1] << "," << ray->P[2] << ">\n";
#endif

        /* Should the sampling be random, or not? */
        if (IntersectCPP(Eye, RuntimeState::scene.Root, ray, hither, yon, &hit))
            hit_flag = 1;
    }

    if (hit_flag) {
        VecSub(Eye->view_from, hit.W, C);
        depth = sqrt(VecDot(C, C));
    }
    else
        depth = PLY_HUGE;

    /* If we hit something then shade it, otherwise use the background */
    if (runtimeState::settings.DepthRender == 1) {
        if (hit_flag) {
            VecSub(Eye->view_from, hit.W, C);
            quantize_depth(depth, color.data(), opacity);
        }
        else
            quantize_depth((runtimeState::settings.pixelsize == 32 ? PLY_HUGE : 256.0), 
                color.data(), opacity);
    }
    else if (hit_flag) {
#ifdef DEBUG_FN_CALLS
        std::cout << "hit_flag=" << hit_flag << "\n";
        std::cout<<"hit.U ="<<hit.U[0]<<" "<<hit.U[1]<<" "<<hit.U[2]<<"\n";
#endif      
        VecCopy(hit.U, U); /* Convert from fVec to Vec. U=hit.U */
        Shade(Eye, hit.obj, hit.texture, level, weight, ior, ray->D, hit.W, hit.N, U, C);
        if (RuntimeState::Global_Haze > 0.0 && RuntimeState::Global_Haze < 1.0 &&
            hit.isect_t > RuntimeState::Global_Haze_Start) {
            c0 = pow(RuntimeState::Global_Haze, (hit.isect_t - RuntimeState::Global_Haze_Start));
            c1 = 1.0 - c0;
            VecComb(c0, C, c1, RuntimeState::Global_Haze_Color, color);
        }
        else {
            VecCopy(C, color);
        }
        *opacity = 1.0;
    }
    else if (RuntimeState::Background != nullptr) {
        c0 = (RuntimeState::current_col >= 
            Eye->view_xres ? Eye->view_xres - 1 : RuntimeState::current_col);
        c1 = (RuntimeState::current_row >= 
            Eye->view_yres ? Eye->view_yres - 1 : RuntimeState::current_row);
        subst.P[0] = c0 / (Flt)Eye->view_xres;
        subst.P[1] = 0.0;
        subst.P[2] = (Flt)(Eye->view_yres - c1 - 1) / (Flt)Eye->view_yres;
        MakeVector(0, 0, 0, subst.PT);
        MakeVector(subst.P[0], subst.P[2], level, subst.U);
        MakeVector(0.0, 0.0, 0.0, subst.W);
        VecCopy(ray->D, subst.N);
        MakeVector(0.0, 0.0, 0.0, subst.I);
        if ((i = eval_node(&subst, RuntimeState::Background, &ftemp, 
            color.data(), &tnode)) != 2)
            serror("Unresolved background expression\n");
        *opacity = (level == 0 ? 0.0 : 1.0);
    }
    else {
        VecCopy(RuntimeState::BackgroundColor, color);
        *opacity = (level == 0 ? 0.0 : 1.0);
    }

    runtimeState::recursion_depth = old_level;
   
#ifdef DEBUG_TESTS
     if (! AlmostEqualRelativeAndAbs(hither, 0.001000, PLY_EPSILON, PLY_EPSILON)) {
         std::cout << "hither failed\n";
         std::cout << "hither=" << hither << " yon=" << yon << "\n";
         std::cout << "hit_flag=" << hit_flag << "\n";
         exit(4);
    }
     if (!AlmostEqualRelativeAndAbs(yon, 1000000.0, PLY_EPSILON, PLY_EPSILON)) {
         std::cout << "yon failed\n";
         std::cout << "hither=" << hither << " yon=" << yon << "\n";
         std::cout << "hit_flag=" << hit_flag << "\n";
         exit(4);
     }
     if (hit_flag != 0) {
         std::cout << "hit_flag isnt 0#" << "\n";
         std::cout << "hither=" << hither << ", yon=" << yon << ", hit_flag=" << hit_flag << "\n";
         std::cout << "color[0]=" << color[0] << " [1]=" << color[1] << " [2]=" << color[2] << "\n";

         std::cout << "EPSILON=" << (PLY_EPSILON * 2) << "\n";
         std::cout << "colorindx=" << colorindx << "\n";
         float compcol = colarr[colorindx++];
         if (colorindx > maxindx) colorindx = 0;

         if (!AlmostEqualRelativeAndAbs(color[0] * 100000, compcol, PLY_EPSILON,PLY_EPSILON)) {
             std::cout << "color isnt expected\n";
             exit(4);
         }
     }

   
    std::cout << "TEST WORKED TraceTest\n";
#endif
    return depth;
}
/* trace.c

   Step through each base object and test for intersections.

  Polyray � MIT Licensed Revival
  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Copyright (C) 1999-2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the �Software�), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#include "defs.h"
#include "screen.h"
#include "trace.h"
#include "vector.h"
#include "io.h"
#include "intersec.h"
#include "display.h"
#include "eval.h"
#include "shade.h"
#include "symtab.h"
#include "image.h"

float
Trace(Viewpoint *Eye, int level, Flt weight, Ray *ray,
      Vec color, Flt *opacity, Flt ior, unsigned long *nr) 
{
   Isect hit;
   Vec C, U;
   Flt ftemp, hither, yon, c0, c1;
   float depth;
   struct subst_struct subst;
   int i, hit_flag, old_level = recursion_depth;
   NODE_PTR tnode;

   hit_flag = 0;

   /* After a certain amount of recursion go to background. */
   recursion_depth = level;
   if (level < maxlevel) {
      (*nr)++;

      /* If we are tracing a primary ray then clip the view frustrum */
      if (level == 0) {
         hither = Eye->view_hither;
         yon    = Eye->view_yon;
         }
      else {
         hither = rayeps;
         yon    = PLY_HUGE;
         }

      /* Should the sampling be random, or not? */
      if (Intersect(Eye, &Root, ray, hither, yon, &hit))
         hit_flag = 1;
      }

   if (hit_flag) {
      VecSub(Eye->view_from, hit.W, C);
      depth = sqrt(VecDot(C, C));
      }
   else
      depth = PLY_HUGE;

   /* If we hit something then shade it, otherwise use the background */
   if (DepthRender == 1) {
      if (hit_flag) {
         VecSub(Eye->view_from, hit.W, C);
         quantize_depth(depth, color, opacity);
         }
      else
         quantize_depth((pixelsize == 32 ? PLY_HUGE : 256.0), color, opacity);
      }
   else if (hit_flag) {
      VecCopy(hit.U, U); /* Convert from fVec to Vec */
      Shade(Eye, hit.obj, hit.texture, level, weight, ior, ray->D, hit.W, hit.N, U, C);
      if (Global_Haze > 0.0 && Global_Haze < 1.0 &&
          hit.isect_t > Global_Haze_Start) {
         c0 = pow(Global_Haze, (hit.isect_t - Global_Haze_Start));
         c1 = 1.0 - c0;
         VecComb(c0, C, c1, Global_Haze_Color, color);
         }
      else {
         VecCopy(C, color);
         }
      *opacity = 1.0;
      }
   else if (Background != NULL) {
      c0 = (current_col >= Eye->view_xres ? Eye->view_xres - 1 : current_col);
      c1 = (current_row >= Eye->view_yres ? Eye->view_yres - 1 : current_row);
      subst.P[0] = c0 / (Flt)Eye->view_xres;
      subst.P[1] = 0.0;
      subst.P[2] = (Flt)(Eye->view_yres - c1 - 1) / (Flt)Eye->view_yres;
      MakeVector(0, 0, 0, subst.PT);
      MakeVector(subst.P[0], subst.P[2], level, subst.U);
      MakeVector(0.0, 0.0, 0.0, subst.W);
      VecCopy(ray->D, subst.N);
      MakeVector(0.0, 0.0, 0.0, subst.I);
      if ((i = eval_node(&subst, Background, &ftemp, color, &tnode)) != 2)
         error("Unresolved background expression\n");
      *opacity = (level == 0 ? 0.0 : 1.0);
      }
   else {
      VecCopy(BackgroundColor, color);
      *opacity = (level == 0 ? 0.0 : 1.0);
      }

   recursion_depth = old_level;

   return depth;
}

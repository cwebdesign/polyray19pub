/* alight.cc

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
#include "memory.h"
#include "bound.h"
#include "vector.h"
#include "intersec.h"
#include "runtime_state.h"
#include "symtab.h"
#include "shade.h"
#include "roots.h"
#include "alight.h"

#if 0
/* Currently unused */
/* Ensure that we hit inside the area light */
/**
 * @brief Test whether a shadow ray intersects the polygonal area light.
 *
 * @param alight Polygonal area-light data being sampled.
 * @param ray Shadow ray to test against the light plane and polygon.
 * @param mindist Minimum acceptable intersection distance along the ray.
 * @param W Receives the intersection point when the test succeeds.
 * @param U Receives the projected in-light coordinates when the test succeeds.
 * @param N Receives the light normal when the test succeeds.
 * @return `1` when the ray hits inside the polygon bounds, otherwise `0`.
 */
static int AlightHitPoint(PolyAlightData *alight, Ray *ray, Flt mindist,
               Vec W, Vec U, Vec N)
{
   int u, v;
   Flt n, d, t;
   Vec V;

   d = VecDot(ray->D, alight->normal);
   if (fabs(d) < EPSILON) return 0;
   n = VecDot(ray->P, alight->normal) + alight->d;
   t = -n / d;
   if (t < mindist) return 0;
   VecAddScaled(ray->P, t, ray->D, V);
   u = alight->u;
   v = alight->v;

   if (Inside_Polygon(V[u], V[v], alight->npoints, alight->points, u, v)) {
      VecCopy(V, W)
      VecCopy(alight->normal, N)
      MakeVector(V[u], V[v], 0, U)
      return 1;
      }
   else
      return 0;
}
#endif

/**
 * @brief Return the squared distance between two vectors.
 *
 * @param a First vector.
 * @param b Second vector.
 * @return Squared Euclidean distance between `a` and `b`.
 */
static Flt VecDist2(Vec a, Vec b)
{
   Vec t;
   VecSub(a, b, t);
   return VecDot(t, t);
}

#if 0
/* Currently unused */
/**
 * @brief Release dynamic storage owned by a polygon area light.
 *
 * @param alight Polygonal area-light data whose buffers are being freed.
 */
static void deallocate_polygon_light(PolyAlightData *alight)
{
   polyray_free(alight->nbuf);
   polyray_free(alight->obuf);
   polyray_free(alight->sbuf1);
   polyray_free(alight->sbuf2);
   polyray_free(alight->points);
}
#endif

/**
 * @brief Build a shadow ray toward a sampled point on the area light.
 *
 * @param alight Polygonal area-light data that controls jittering.
 * @param from Shadow ray origin.
 * @param at Sample target point on the light.
 * @param udelta Light-space sample spacing along the U basis.
 * @param vdelta Light-space sample spacing along the V basis.
 * @param ray Receives the constructed ray origin and normalized direction.
 * @param tmax Receives the distance from `from` to `at`.
 * @return Always returns `1`.
 */
static int make_jittered_ray(PolyAlightData *alight, Vec from, Vec at,
                          Flt udelta, Flt vdelta, Ray *ray, Flt *tmax)
{
   Flt ulen, vlen;
   Vec delta_at;

   if (alight->jitter != 0) {
      /* Create a jittered offset */
      ulen = udelta * (polyray_random() * 2.0 - 1.0);
      vlen = vdelta * (polyray_random() * 2.0 - 1.0);

      VecComb(alight->jitter * ulen, alight->ubasis,
              alight->jitter * vlen, alight->vbasis,
              delta_at);
      VecAdd(delta_at, at, at);
      }

   VecCopy(from, ray->P)
   VecSub(at, from, ray->D)
   *tmax = VecNormalize(ray->D);

#if 0
printf("xfrom: <%g,%g,%g>, dir: <%g,%g,%g>\n",
       ray->P[0], ray->P[1], ray->P[2],
       ray->D[0], ray->D[1], ray->D[2]);
#endif
   return 1;
}

/* Filt is the color reaching the point in question from our light */
/**
 * @brief Trace one area-light sample and accumulate transmission filtering.
 *
 * @param eye Current viewpoint used for intersection and shading queries.
 * @param light Light whose blocker cache is consulted and updated.
 * @param alight Polygonal area-light data for jittered sampling.
 * @param from Shadow ray origin.
 * @param at Sample target point on the light.
 * @param udelta Light-space sample spacing along the U basis.
 * @param vdelta Light-space sample spacing along the V basis.
 * @param tmin Minimum intersection distance for occluder tests.
 * @param Filt Receives the remaining light color after occlusion filtering.
 * @return `1` when the sample ray is processed, otherwise `0`.
 */
static int single_pixel(Viewpoint *eye, Light *light, PolyAlightData *alight,
             Vec from, Vec at, Flt udelta, Flt vdelta, Flt tmin,
    NuVec& Filt)
{
   Vec fcolor;
   Flt light_left, fscale, t, tmax;
   Ray ray;
   Surface *surf;
   Object *cobj;
   Isect thit;

   /* Jitter the ray a little bit? */
   if (!make_jittered_ray(alight, from, at, udelta, vdelta, &ray, &tmax)) {
      /* The sample point wasn't in the light polygon.  */
      return 0;
      }
/* printf("Sample: <%g,%g,%g>\n", at[0], at[1], at[2]); */

   /* Since there may be several semi-transparent occluding
      surfaces between the initial point and the light, we
      loop until we either are completely blocked or we get
      very close to the light. */
   MakeVector(1.0, 1.0, 1.0, Filt);
   int cnt = 0;          /* Haven't started looking towards the light yet */
   light_left = 1.0; /* Start by assuming all of the light gets through */
   while (light_left > SMALL && tmax > SMALL && cnt < 10) {
      thit.flag = 0;
      if (light != nullptr && cnt == 0) {
         runtimeState::stats.totalShadows++;
         cobj = Get_Light_Blocker(light, runtimeState::recursion_depth);
         if (cobj != nullptr &&
            find_object_intersections(eye, cobj, &ray, tmin,
                                      tmax, &thit)) {
             runtimeState::stats.totalShadowCaches++;
            light_left = 0.0;
            break;
            }
         }

      if (Intersect(eye, &RuntimeState::scene.Root, &ray, tmin, tmax, &thit)) {
          /* See if there is any color left after this hit */
         if (thit.obj->o_type == ShapeType::Hypertexture) {
            /* Special case, the color and opacity have already
               been calculated for us.  */
            fscale = thit.U[0];
            MakeVector(1, 1, 1, fcolor)
            t = thit.U[1];
            }
         else if ((runtimeState::scene.Global_Shade_Flag & TRANSMIT_CHECK) &&
                  (thit.obj->o_sflag & TRANSMIT_CHECK)) {
            t = thit.isect_t;
            VecNormalize(thit.N);
            surf = find_surface(eye, thit.obj, thit.texture, thit.W,
                                thit.N, ray.D, thit.U, 7);
            if (surf->Kt_scale == 0.0) {
               /* Completely blocked */
               if (cnt == 0 && light != nullptr)
                  Set_Light_Blocker(light, runtimeState::recursion_depth, thit.obj);
               light_left = 0.0;
               break;
               }
            VecCopy(surf->Kt_color, fcolor)
            fscale = surf->Kt_scale;
            }
         else {
            /* If this is the first hit in the loop then we can
               cache this object as the blocker of the light */
            if (cnt == 0 && light != nullptr)
               Set_Light_Blocker(light, runtimeState::recursion_depth, thit.obj);
            light_left = 0.0;
            break;
            }

         /* Reduce the amount of light by the filter color associated
            with the thing it hit */
         for (int i=0;i<3;++i)
            Filt[i] *= fscale * fcolor[i];
         light_left = VecDot(Filt, Filt);

         /* Move up a little closer to the light */
         VecCopy(thit.W, ray.P);
         tmax = tmax - t;
         cnt++;
         }
      else {
         if (cnt == 0 && light != nullptr)
            Set_Light_Blocker(light, runtimeState::recursion_depth, nullptr);
         break;
         }
      }

   /* See if there is an early out based on complete occlusion by
      the light source */
   if (light_left < SMALL)
      MakeVectorZero(Filt); /* MakeVector(0.0, 0.0, 0.0, Filt) */


   /* It was a valid sample ray */
   return 1;
}

/**
 * @brief Adaptively subdivide an area-light sample cell and average its result.
 *
 * @param eye Current viewpoint used for visibility checks.
 * @param light Light whose blocker cache is consulted during sampling.
 * @param alight Polygonal area-light data controlling adaptive sampling.
 * @param from Shadow ray origin.
 * @param udelta Light-space sample spacing along the U basis.
 * @param vdelta Light-space sample spacing along the V basis.
 * @param tmin Minimum intersection distance for occluder tests.
 * @param sample_points Corner sample positions for the current cell.
 * @param corner_colors Corner sample colors for the current cell.
 * @param color Receives the averaged color for the cell.
 * @param depth Current adaptive subdivision depth.
 */
static void throw_rays(Viewpoint *eye, Light *light, PolyAlightData *alight,
           Vec from, Flt udelta, Flt vdelta, Flt tmin,
           Vec sample_points[4], Vec corner_colors[4],
           Vec color, int depth)
{
    std::array<NuVec,9> color_grid;
    Vec new_color[4], avg_color;
   Vec sample_grid[9], new_samples[4];

   if ((depth < alight->adaptive_depth) &&
       ((depth == 1 && alight->adaptive_depth > 2) ||
        (VecDist2(corner_colors[0],corner_colors[1]) > runtimeState::settings.antialias_threshold) ||
        (VecDist2(corner_colors[0],corner_colors[2]) > runtimeState::settings.antialias_threshold) ||
        (VecDist2(corner_colors[1],corner_colors[3]) > runtimeState::settings.antialias_threshold) ||
        (VecDist2(corner_colors[2],corner_colors[3]) > runtimeState::settings.antialias_threshold))) {
      /* Reduce jitter size as we subdivide light size */
      udelta *= 0.5;
      vdelta *= 0.5;

      /* Difference between corners is sufficiently different to force
         oversampling */
      VecCopy(corner_colors[0], color_grid[0]);
      VecCopy(corner_colors[1], color_grid[2]);
      VecCopy(corner_colors[2], color_grid[6]);
      VecCopy(corner_colors[3], color_grid[8]);

      VecCopy(sample_points[0], sample_grid[0]);
      VecCopy(sample_points[1], sample_grid[2]);
      VecCopy(sample_points[2], sample_grid[6]);
      VecCopy(sample_points[3], sample_grid[8]);

      /* Compute and trace the five new points */
      VecAdd(sample_points[0], sample_points[1], sample_grid[1])
      VecScale(0.5, sample_grid[1]);
      single_pixel(eye, light, alight, from, sample_grid[1],
                   udelta, vdelta, tmin, color_grid[1]);

      VecAdd(sample_points[0], sample_points[2], sample_grid[3])
      VecScale(0.5, sample_grid[3]);
      single_pixel(eye, light, alight, from, sample_grid[3],
                   udelta, vdelta, tmin, color_grid[3]);

      VecAdd(sample_points[0], sample_points[3], sample_grid[4])
      VecScale(0.5, sample_grid[4]);
      single_pixel(eye, light, alight, from, sample_grid[4],
                   udelta, vdelta, tmin, color_grid[4]);

      VecAdd(sample_points[1], sample_points[3], sample_grid[5])
      VecScale(0.5, sample_grid[5]);
      single_pixel(eye, light, alight, from, sample_grid[5],
                   udelta, vdelta, tmin, color_grid[5]);

      VecAdd(sample_points[2], sample_points[3], sample_grid[7])
      VecScale(0.5, sample_grid[7]);
      single_pixel(eye, light, alight, from, sample_grid[7],
                   udelta, vdelta, tmin, color_grid[7]);

      /* Now that we have the full 3x3 matrix of samples, we recurse into
         four 2x2 boxes that represent the subpixels of this pixel */
      VecCopy(color_grid[0], new_color[0]);
      VecCopy(color_grid[1], new_color[1]);
      VecCopy(color_grid[3], new_color[2]);
      VecCopy(color_grid[4], new_color[3]);
      VecCopy(sample_grid[0], new_samples[0]);
      VecCopy(sample_grid[1], new_samples[1]);
      VecCopy(sample_grid[3], new_samples[2]);
      VecCopy(sample_grid[4], new_samples[3]);
      throw_rays(eye, light, alight, from, udelta, vdelta, tmin,
                 new_samples, new_color, avg_color,
                 depth + 1);
      VecCopy(avg_color, color);

      VecCopy(color_grid[1], new_color[0]);
      VecCopy(color_grid[2], new_color[1]);
      VecCopy(color_grid[4], new_color[2]);
      VecCopy(color_grid[5], new_color[3]);
      VecCopy(sample_grid[1], new_samples[0]);
      VecCopy(sample_grid[2], new_samples[1]);
      VecCopy(sample_grid[4], new_samples[2]);
      VecCopy(sample_grid[5], new_samples[3]);
      throw_rays(eye, light, alight, from, udelta, vdelta, tmin,
                 new_samples, new_color, avg_color,
                 depth + 1);
      VecAdd(avg_color, color, color);

      VecCopy(color_grid[3], new_color[0]);
      VecCopy(color_grid[4], new_color[1]);
      VecCopy(color_grid[6], new_color[2]);
      VecCopy(color_grid[7], new_color[3]);
      VecCopy(sample_grid[3], new_samples[0]);
      VecCopy(sample_grid[4], new_samples[1]);
      VecCopy(sample_grid[6], new_samples[2]);
      VecCopy(sample_grid[7], new_samples[3]);
      throw_rays(eye, light, alight, from, udelta, vdelta, tmin,
                 new_samples, new_color, avg_color,
                 depth + 1);
      VecAdd(avg_color, color, color);

      VecCopy(color_grid[4], new_color[0]);
      VecCopy(color_grid[5], new_color[1]);
      VecCopy(color_grid[7], new_color[2]);
      VecCopy(color_grid[8], new_color[3]);
      VecCopy(sample_grid[4], new_samples[0]);
      VecCopy(sample_grid[5], new_samples[1]);
      VecCopy(sample_grid[7], new_samples[2]);
      VecCopy(sample_grid[8], new_samples[3]);
      throw_rays(eye, light, alight, from, udelta, vdelta, tmin,
                 new_samples, new_color, avg_color,
                 depth + 1);
      VecAdd(avg_color, color, color);

      VecScale(0.25, color);
      }
   else {
      /* Average the corners */
      VecCopy(corner_colors[0], color);
      VecAdd(corner_colors[1], color, color);
      VecAdd(corner_colors[2], color, color);
      VecAdd(corner_colors[3], color, color);
      VecScale(0.25, color);
      }
}

/**
 * @brief Sample a polygonal area light and return the accumulated light color.
 *
 * @param eye Current viewpoint used for visibility checks.
 * @param light Light whose blocker cache is consulted during sampling.
 * @param alight Polygonal area-light data describing the emitter.
 * @param tmin Minimum intersection distance for shadow rays.
 * @param test_point Point being illuminated.
 * @param total_light Receives the averaged transmitted light color.
 * @return `1` when any light reaches the point, otherwise `0`.
 */
int PolygonLight(Viewpoint *eye, Light *light, PolyAlightData *alight,
             Flt tmin, Vec test_point, Vec total_light)
{
   int valid_samples;
   Flt udelta, vdelta, cdelta;
   Vec udeltav, vdeltav;
   std::vector<NuVec> nbuf, obuf, tmp;
   std::vector<NuVec> sbuf1, sbuf2;
   Vec from, avg_color, corner_colors[4];
   Vec corner_points[4], sample1, sample2;

   VecCopy(test_point, from);

   nbuf = alight->nbuf;
   obuf = alight->obuf;
   sbuf1 = alight->sbuf1;
   sbuf2 = alight->sbuf2;

   /* Set up variables used to step across the light */
   udelta = (alight->ures < 2 ? 1.0 : 1.0 / (Flt)alight->ures);
   vdelta = (alight->ures < 2 ? 1.0 : 1.0 / (Flt)alight->vres);
   VecCopy(alight->ubasis, udeltav)
   VecScale(udelta, udeltav)
   VecCopy(alight->vbasis, vdeltav)
   VecScale(vdelta, vdeltav)

   udelta = VecLen(udeltav);
   vdelta = VecLen(vdeltav);

   /* Set up the containers for sampling information */
   /*MakeVector(0, 0, 0, total_light)*/
   MakeVectorZero(total_light);
   valid_samples = 0;
   VecCopy(alight->lower_left, sample1)

#if 0
printf("Sample1: <%g,%g,%g>\n",
       sample1[0], sample1[1], sample1[2]);
printf("udeltav: <%g,%g,%g>, vdeltav: <%g,%g,%g>\n",
       udeltav[0], udeltav[1], udeltav[2],
       vdeltav[0], vdeltav[1], vdeltav[2]);
#endif
   /* Walk through the sample points on the light */
   for (int u=0;u<=alight->ures;++u) {
      VecCopy(sample1, sample2)
      for (int v=0;v<=alight->vres;++v) {
         VecCopy(sample2, sbuf1[v])
#if 0
printf("from: <%g,%g,%g>, at: <%g,%g,%g>\n",
       from[0], from[1], from[2],
       sample2[0], sample2[1], sample2[2]);
#endif

         single_pixel(eye, light, alight, from, sample2,
                      udelta, vdelta, tmin, nbuf[v]);
         VecAdd(vdeltav, sample2, sample2)
         }
      if (u > 0) {
         for (int i=0;i<alight->vres;++i) {
            VecCopy(obuf[i],   corner_colors[0]);
            VecCopy(obuf[i+1], corner_colors[1]);
            VecCopy(nbuf[i],   corner_colors[2]);
            VecCopy(nbuf[i+1], corner_colors[3]);
            VecCopy(sbuf1[i],   corner_points[0]);
            VecCopy(sbuf1[i+1], corner_points[1]);
            VecCopy(sbuf2[i],   corner_points[2]);
            VecCopy(sbuf2[i+1], corner_points[3]);
            throw_rays(eye, light, alight, from, udelta, vdelta,
                       tmin, corner_points, corner_colors,
                       avg_color, 0);
            VecAdd(avg_color, total_light, total_light);
            valid_samples++;
            }
         }
      /* Roll the buffers */
      tmp =  obuf;  obuf =  nbuf;  nbuf = tmp;
      tmp = sbuf1; sbuf1 = sbuf2; sbuf2 = tmp;
      VecAdd(udeltav, sample1, sample1)
      }

   /* Figure out the final color */
   if (valid_samples > 0) {
      cdelta = 1.0 / (Flt)valid_samples;
      VecScale(cdelta, total_light)
      }
   else
      MakeVectorZero(total_light); /* MakeVector(0, 0, 0, total_light) */
#if 0
printf("Total light: <%g,%g,%g>\n",
       total_light[0], total_light[1], total_light[2]);
#endif
   if (total_light[0] == 0.0 && total_light[1] == 0.0 &&
       total_light[2] == 0.0)
      return 0;
   else
      return 1;
}

/* screen.cc

   Step through all rays, checking them against all base objects for
   intersections.

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

#include <vector>
#include <span>
#include <stdexcept>

#include "defs3.h"
#include "vector.h"
#include "io_ply.h"
#include "screen.h"
#include "trace.h"
#include "display.h"
#include "memory.h"
#include "runtime_state.h"
#include "symtab.h"
#include "image.h"
#include "unixcompat.h"
#include "util.h"

#if defined(__linux)
/** @brief Stub keyboard-read helper for Linux builds that do not use interactive abort keys.
 *  @return Always returns 0.
 */
static int getch(void)
{
return 0;
}
#endif

/** @brief Compute the squared distance between two vectors.
 *  @param a First vector.
 *  @param b Second vector.
 *  @return Squared Euclidean distance between @p a and @p b.
 */
static Flt VecDist2(Vec a, Vec b)
{
   Vec t;
   VecSub(a, b, t);
   return VecDot(t, t);
}

/** @brief Jitter a ray origin across the lens aperture for depth-of-field sampling.
 *  @param from       Camera origin.
 *  @param viewvec    Forward viewing vector.
 *  @param upvec      Camera up vector.
 *  @param focaldist  Distance to the focal plane.
 *  @param aperture   Aperture radius controlling blur amount.
 *  @param ray        Ray updated in place with a jittered origin and refocused direction.
 *  @return No return value.
 */
static void focal_blur(Vec from, Vec viewvec, Vec upvec,
           Flt focaldist, Flt aperture, Ray *ray)
{
   Flt xlen, ylen;
   Vec scrni, scrnj;
   Vec aperture_inc;

   /* Create a jittered offset */
   xlen = polyray_random() * 2.0 - 1.0;
   ylen = polyray_random() * 2.0 - 1.0;

   VecCross(upvec, viewvec, scrni);
   (void)VecNormalize(scrni);
   VecCross(viewvec, scrni, scrnj);
   (void)VecNormalize(scrnj);
   VecComb(aperture * xlen, scrni,
           aperture * ylen, scrnj, aperture_inc);
   VecAdd(aperture_inc, from, ray->P);
   VecScale(focaldist, ray->D);
   VecSub(ray->D, aperture_inc, ray->D);
   (void)VecNormalize(ray->D);
}

/** @brief Trace one pixel sample, optionally averaging multiple depth-of-field rays.
 *  @param Eye        Active viewpoint.
 *  @param from       Camera origin.
 *  @param viewvec    Forward viewing vector.
 *  @param upvec      Camera up vector.
 *  @param focaldist  Distance to the focal plane.
 *  @param aperture   Aperture radius; zero disables focal blur.
 *  @param ray        Working ray structure.
 *  @param depth      Current adaptive supersampling depth.
 *  @param color      Output sampled color.
 *  @param opacity    Output sampled opacity.
 *  @return No return value.
 */
#ifndef TESTING
static 
#endif 
void single_pixel(Viewpoint *Eye, Vec from, Vec viewvec, Vec upvec,
             Flt focaldist, Flt aperture, Ray *ray,
             int depth, Vec color, Flt *opacity)
{
   int sample_count, j;
   Vec avg_color, D;
   Flt avg_opacity;

   if (aperture == 0) {
      NuVec color_nv{ color[0], color[1], color[2] };
      Trace(Eye, 0, 1.0, ray, color_nv, opacity, 1.0,
          &runtimeState::stats.nRays);
      assignNuVecToFVec(color_nv, color);
      }
   else {
      VecCopy(ray->D, D);
      /* MakeVector(0.0, 0.0, 0.0, color); */
      MakeVectorZero(color);
      *opacity = 0.0;
      sample_count = 1 + (runtimeState::settings.maxsamples / (1 << depth));
      for (j=0;j<sample_count;j++) {
         VecCopy(D, ray->D);
         if (j > 0)
            focal_blur(from, viewvec, upvec, focaldist, aperture, ray);
         NuVec avg_color_nv{};
         Trace(Eye, 0, 1.0, ray, avg_color_nv, &avg_opacity, 1.0,
             &runtimeState::stats.nRays);
         assignNuVecToFVec(avg_color_nv, avg_color);
         VecAdd(color, avg_color, color);
         *opacity += avg_opacity;
         }
      VecScale((1.0 / (Flt)sample_count), color);
      *opacity /= (Flt)sample_count;
      }
}

/** @brief Recursively supersample a pixel region using corner colors and opacities.
 *  @param Eye           Active viewpoint.
 *  @param from          Camera origin.
 *  @param viewvec       Forward viewing vector.
 *  @param upvec         Camera up vector.
 *  @param rightvec      Camera right vector.
 *  @param focaldist     Distance to the focal plane.
 *  @param aperture      Aperture radius controlling blur amount.
 *  @param ray           Working ray structure.
 *  @param xlen          Left sample position in normalized screen space.
 *  @param ylen          Top sample position in normalized screen space.
 *  @param xdelta        Horizontal subpixel span.
 *  @param ydelta        Vertical subpixel span.
 *  @param corner_colors Corner sample colors in raster order.
 *  @param corner_opacs  Corner sample opacities in raster order.
 *  @param color         Output averaged color.
 *  @param opacity       Output averaged opacity.
 *  @param depth         Current recursion depth.
 *  @param limit         Maximum recursion depth.
 *  @return No return value.
 */
#ifndef TESTING
static 
#endif
void throw_rays(Viewpoint *Eye, Vec from, Vec viewvec, Vec upvec, Vec rightvec,
           Flt focaldist, Flt aperture, Ray *ray,
           Flt xlen, Flt ylen, Flt xdelta, Flt ydelta,
           Vec corner_colors[4], Flt corner_opacs[4],
           Vec color, Flt *opacity,
           int depth, int limit)
{
   Flt avg_opacity;
   Vec D, color_grid[9], new_color[4], avg_color;
   Flt opac_grid[9], new_opac[4];

   if ((depth < limit) &&
       ((depth == 1 && limit > 2) ||
        (VecDist2(corner_colors[0],corner_colors[1]) > runtimeState::settings.antialias_threshold) ||
        (VecDist2(corner_colors[0],corner_colors[2]) > runtimeState::settings.antialias_threshold) ||
        (VecDist2(corner_colors[1],corner_colors[3]) > runtimeState::settings.antialias_threshold) ||
        (VecDist2(corner_colors[2],corner_colors[3]) > runtimeState::settings.antialias_threshold))) {
      /* Difference between corners is sufficiently different to force
         oversampling */
      VecCopy(corner_colors[0], color_grid[0]);
      VecCopy(corner_colors[1], color_grid[2]);
      VecCopy(corner_colors[2], color_grid[6]);
      VecCopy(corner_colors[3], color_grid[8]);
      opac_grid[0] = corner_opacs[0];
      opac_grid[2] = corner_opacs[1];
      opac_grid[6] = corner_opacs[2];
      opac_grid[8] = corner_opacs[3];
      xdelta *= 0.5;
      ydelta *= 0.5;

      /* Compute and trace the five new points */
      xlen += xdelta;
      VecComb(xlen, rightvec, ylen, upvec, D);
      VecAdd(D, viewvec, D);
      VecNormalize(D);
      VecCopy(D, ray->D);
      single_pixel(Eye, from, viewvec, upvec, focaldist, aperture, ray,
                   depth, color_grid[1], &opac_grid[1]);

      xlen -= xdelta;
      ylen += ydelta;
      VecComb(xlen, rightvec, ylen, upvec, D);
      VecAdd(D, viewvec, D);
      VecNormalize(D);
      VecCopy(D, ray->D);
      single_pixel(Eye, from, viewvec, upvec, focaldist, aperture, ray,
                   depth, color_grid[3], &opac_grid[3]);

      xlen += xdelta;
      VecComb(xlen, rightvec, ylen, upvec, D);
      VecAdd(D, viewvec, D);
      VecNormalize(D);
      VecCopy(D, ray->D);
      single_pixel(Eye, from, viewvec, upvec, focaldist, aperture, ray,
                   depth, color_grid[4], &opac_grid[4]);

      xlen += xdelta;
      VecComb(xlen, rightvec, ylen, upvec, D);
      VecAdd(D, viewvec, D);
      VecNormalize(D);
      VecCopy(D, ray->D);
      single_pixel(Eye, from, viewvec, upvec, focaldist, aperture, ray,
                   depth, color_grid[5], &opac_grid[5]);

      xlen -= xdelta;
      ylen += ydelta;
      VecComb(xlen, rightvec, ylen, upvec, D);
      VecAdd(D, viewvec, D);
      VecNormalize(D);
      VecCopy(D, ray->D);
      single_pixel(Eye, from, viewvec, upvec, focaldist, aperture, ray,
                   depth, color_grid[7], &opac_grid[7]);

      /* Now that we have the full 3x3 matrix of samples, we recurse into
         four 2x2 boxes that represent the subpixels of this pixel */
      VecCopy(color_grid[0], new_color[0]);
      new_opac[0] = opac_grid[0];
      VecCopy(color_grid[1], new_color[1]);
      new_opac[1] = opac_grid[1];
      VecCopy(color_grid[3], new_color[2]);
      new_opac[2] = opac_grid[3];
      VecCopy(color_grid[4], new_color[3]);
      new_opac[3] = opac_grid[4];
      xlen -= xdelta;
      ylen -= 2.0*ydelta;
      throw_rays(Eye, from, viewvec, upvec, rightvec,
                 focaldist, aperture, ray,
                 xlen, ylen, xdelta, ydelta,
                 new_color, new_opac,
                 avg_color, &avg_opacity,
                 depth + 1, limit);
      VecCopy(avg_color, color);
      *opacity = avg_opacity;

      VecCopy(color_grid[1], new_color[0]);
      new_opac[0] = opac_grid[1];
      VecCopy(color_grid[2], new_color[1]);
      new_opac[1] = opac_grid[2];
      VecCopy(color_grid[4], new_color[2]);
      new_opac[2] = opac_grid[4];
      VecCopy(color_grid[5], new_color[3]);
      new_opac[3] = opac_grid[5];
      xlen += xdelta;
      throw_rays(Eye, from, viewvec, upvec, rightvec,
                 focaldist, aperture, ray,
                 xlen, ylen, xdelta, ydelta,
                 new_color, new_opac,
                 avg_color, &avg_opacity,
                 depth + 1, limit);
      VecAdd(avg_color, color, color);
      *opacity += avg_opacity;

      VecCopy(color_grid[3], new_color[0]);
      new_opac[0] = opac_grid[3];
      VecCopy(color_grid[4], new_color[1]);
      new_opac[1] = opac_grid[4];
      VecCopy(color_grid[6], new_color[2]);
      new_opac[2] = opac_grid[6];
      VecCopy(color_grid[7], new_color[3]);
      new_opac[3] = opac_grid[7];
      xlen -= xdelta;
      ylen += ydelta;
      throw_rays(Eye, from, viewvec, upvec, rightvec,
                 focaldist, aperture, ray,
                 xlen, ylen, xdelta, ydelta,
                 new_color, new_opac,
                 avg_color, &avg_opacity,
                 depth + 1, limit);
      VecAdd(avg_color, color, color);
      *opacity += avg_opacity;

      VecCopy(color_grid[4], new_color[0]);
      new_opac[0] = opac_grid[4];
      VecCopy(color_grid[5], new_color[1]);
      new_opac[1] = opac_grid[5];
      VecCopy(color_grid[7], new_color[2]);
      new_opac[2] = opac_grid[7];
      VecCopy(color_grid[8], new_color[3]);
      new_opac[3] = opac_grid[8];
      xlen += xdelta;
      throw_rays(Eye, from, viewvec, upvec, rightvec,
                 focaldist, aperture, ray,
                 xlen, ylen, xdelta, ydelta,
                 new_color, new_opac,
                 avg_color, &avg_opacity,
                 depth + 1, limit);
      VecAdd(avg_color, color, color);
      *opacity += avg_opacity;

      VecScale(0.25, color);
      *opacity *= 0.25;
      }
   else {
      /* Average the corners */
      VecCopy(corner_colors[0], color);
      VecAdd(corner_colors[1], color, color);
      VecAdd(corner_colors[2], color, color);
      VecAdd(corner_colors[3], color, color);
      VecScale(0.25, color);
      *opacity = 0.25 * (corner_opacs[0] + corner_opacs[1] +
                         corner_opacs[2] + corner_opacs[3]);
      }
}


/** @brief Render scanlines without adaptive antialiasing.
 *  @param eye      Active viewpoint.
 *  @param viewvec  Forward viewing vector.
 *  @param rightvec Camera right vector.
 *  @param ystart   First scanline to render, inclusive.
 *  @param yend     Last scanline to render, exclusive.
 *  @return No return value.
 */
#ifndef TESTING
static 
#endif
void Scan(Viewpoint *eye, Vec viewvec, Vec rightvec, int ystart, int yend)
{
#ifdef DEBUG
    smessage("screen::Scan\n");
#endif
   Ray ray;
   int x, y, j;
   Flt xlen, ylen;
   Vec D, color, avg;
   Flt opacity, avg_opacity;
   float z=0;
#ifdef unix
   char tmp[100];
#endif

   /* First figure out how the row and column counters are
      incremented. */
   VecCopy(eye->view_from, ray.P);

   for (y=ystart;y<yend;y++) {
       RuntimeState::current_row = y;
      ylen = 1.0 - ((Flt)(2 * y) / (Flt)eye->view_yres);
      for (x=0;x<eye->view_xres;x++) {
          RuntimeState::current_col = x;
         xlen = ((Flt)(2 * x) / (Flt)eye->view_xres) - 1.0;
         VecComb(xlen, rightvec, ylen, eye->view_up, D);
         VecAdd(D, viewvec, D);
         VecNormalize(D);
         if (eye->view_aperture == 0) {
            VecCopy(D, ray.D);
            NuVec color_nv = toNuVec(color);
            z = Trace(eye, 0, 1.0, &ray, color_nv, &opacity, 1.0,
                &runtimeState::stats.nRays);
            assignNuVecToFVec(color_nv, color);
            }
         else {
            /* MakeVector(0.0, 0.0, 0.0, color); */
            MakeVectorZero1(color);
            opacity = 0.0;
            for (j= 0;j< runtimeState::settings.maxsamples;j++) {
               VecCopy(D, ray.D);
               focal_blur(eye->view_from, viewvec, eye->view_up,
                          eye->view_focaldist, eye->view_aperture, &ray);
               NuVec avg_nv{};
               z = Trace(eye, 0, 1.0, &ray, avg_nv, &avg_opacity, 1.0,
                   &RuntimeState::stats.nRays);
               assignNuVecToFVec(avg_nv, avg);
               VecAdd(color, avg, color);
               }
            VecScale((1.0 / (Flt)RuntimeState::settings.maxsamples), color);
            opacity += avg_opacity / (Flt)RuntimeState::settings.maxsamples;
            }
         Put_Pixel(eye, x, y, color, opacity);
         if (z < ZBuffer_Read(eye, x, y))
            ZBuffer_Write(eye, x, y, z);

         if (Global::Display_Flag)
            display_plot(x, y, color);
#if !defined( _WINDOWS )
         if ((RuntimeState::Check_Abort_Flag == 1) && _kbhit()) {
#if defined( MAC )||defined(linux)||defined(__illumos__)
            RuntimeState::Abort_Flag = 1;
#else
            RuntimeState::Abort_Flag = getch();
#endif
            return;
            }
#endif
         if (RuntimeState::settings.tickflag == 3)
            status((char*)"\r[%d, %d]     ", y, x);
         }
      if ((RuntimeState::Check_Abort_Flag == 2) && _kbhit()) {
#if defined( MAC )||defined(__illumos__)
            RuntimeState::Abort_Flag = 1;
#else
            RuntimeState::Abort_Flag = getch();
#endif
         return;
         }
      if (RuntimeState::settings.tickflag == 2)
         status((char*)"\r%d ", y);
      }
#ifdef unix
      if (start_frame!=end_frame)
        sprintf(tmp,"F%d/%d, L%d/%d", current_frame-start_frame,
                end_frame-start_frame, y,eye->view_yres);
      else
        sprintf(tmp,"L%d/%d",y,eye->view_yres);
      //SpecialStatus(tmp);
#endif
}

/** @brief Render scanlines with adaptive supersampling and optional depth-of-field blur.
 *  @param eye      Active viewpoint.
 *  @param viewvec  Forward viewing vector.
 *  @param rightvec Camera right vector.
 *  @param maxdepth Maximum adaptive subdivision depth.
 *  @param ystart   First scanline to render, inclusive.
 *  @param yend     Last scanline to render, inclusive.
 *  @return No return value.
 */
#ifndef TESTING
static 
#endif
void FilterScan(Viewpoint* eye, Vec viewvec, Vec rightvec, int maxdepth,
    int ystart, int yend)
{
#ifdef DEBUG_FN_CALLS
    smessage("screen::FilterScan ystart=%d,yend=%d\n", ystart, yend);
#endif

#ifdef DEBUG_FN_CALLS
    if (RuntimeState::scene.Root.slab_root != nullptr) {
        std::cout << "Root.slab_root->o_type=" << 
        std::to_underlying(RuntimeState::scene.Root.slab_root->o_type) << "\n";
        printf("Root.slab_root->o_bnd.lower_left=%f,%f,%f\n", RuntimeState::scene.Root.slab_root->o_bnd.lower_left[0], \
            RuntimeState::scene.Root.slab_root->o_bnd.lower_left[1], RuntimeState::scene.Root.slab_root->o_bnd.lower_left[2]);
    }
#endif
    // Ensure these variables are scoped appropriately in your function
    Ray ray;
    int x, y, i, j;
    Flt xlen, ylen;
    Flt xdelta, ydelta;

    Flt opacity, avg_opacity, corner_opacs[4];
    Vec D, color, avg_color, corner_colors[4];
    float z = 0;

    const size_t buffer_size = eye->view_xres + 1;

    // 1. Memory Owners: Vectors handle allocation, cleanup, and type safety automatically.
    std::vector<Vec> vec_pool_a(buffer_size);
    std::vector<Vec> vec_pool_b(buffer_size);
    std::vector<Flt> flt_pool_a(buffer_size);
    std::vector<Flt> flt_pool_b(buffer_size);

    // 2. Non-owning views (spans) replace the raw pointers for lightning-fast row swapping.
    std::span<Vec> nbuf = vec_pool_a;
    std::span<Flt> onbuf = flt_pool_a;

    std::span<Vec> obuf{};   // Replaces NULL initialization
    std::span<Flt> oobuf{};

    VecCopy(eye->view_from, ray.P);

    ydelta = -2.0 / static_cast<Flt>(eye->view_yres);
    xdelta = 2.0 / static_cast<Flt>(eye->view_xres);
    ylen = 1.0 - (2.0 * ystart / static_cast<Flt>(eye->view_yres));

    for (y = ystart; y <= yend; y++, ylen += ydelta) {
        runtimeState::current_row = y;

        for (x = 0, xlen = -1.0; x <= eye->view_xres; x++, xlen += xdelta) {
            runtimeState::current_col = x;
            VecComb(xlen, rightvec, ylen, eye->view_up, D);
            VecAdd(D, viewvec, D);
            VecNormalize(D);

            if (eye->view_aperture == 0) {
                VecCopy(D, ray.D);
#ifdef DEBUG_TESTS
                NuVec color_nv_test = toNuVec(color);
                z = TraceTest(eye, 0, 1.0, &ray, color_nv_test, &opacity, 1.0, &runtimeState::stats.nRays);
                assignNuVecToFVec(color_nv_test, color);
#else
                NuVec color_nv = toNuVec(color);
                z = Trace(eye, 0, 1.0, &ray, color_nv, &opacity, 1.0, &runtimeState::stats.nRays);
                assignNuVecToFVec(color_nv, color);
#endif
            }
            else {
#ifdef DEBUG
                smessage("bpoint10\n");
#endif
                MakeVectorZero1(color);
                opacity = 0.0;
                for (j = 0; j < runtimeState::settings.maxsamples; j++) {
                    VecCopy(D, ray.D);
                    focal_blur(eye->view_from, viewvec, eye->view_up, eye->view_focaldist, eye->view_aperture, &ray);
                    NuVec avg_color_nv{};
                    z = Trace(eye, 0, 1.0, &ray, avg_color_nv, &avg_opacity, 1.0, &runtimeState::stats.nRays);
                    assignNuVecToFVec(avg_color_nv, avg_color);
                    VecAdd(color, avg_color, color);
                    opacity += avg_opacity;
                }
                VecScale((1.0 / static_cast<Flt>(runtimeState::settings.maxsamples)), color);
                opacity /= static_cast<Flt>(runtimeState::settings.maxsamples);
            }

            if (z < ZBuffer_Read(eye, x, y)) {
                ZBuffer_Write(eye, x, y, z);
            }

            VecCopy(color, nbuf[x]);
            onbuf[x] = opacity;

            if (Global::Display_Flag && x < eye->view_xres && y < eye->view_yres) {
                display_plot(x, y, color);
            }

#if !defined( _WINDOWS )
            if ((runtimeState::Check_Abort_Flag == 1) && _kbhit()) {
#if defined( MAC ) || defined(__illumos__)
                runtimeState::Abort_Flag = 1;
#else
                runtimeState::Abort_Flag = getch();
#endif
                goto end_of_scan;
            }
#endif
            if (runtimeState::settings.tickflag == 3) {
                status((char*)"\r[%d, %d]     ", y, x);
            }
        }

        if (!obuf.empty()) { // Checked if initialized
            xlen = -1.0;
            for (i = 0; i < eye->view_xres; i++, xlen += xdelta) {
                runtimeState::current_col = i;
                VecCopy(obuf[i], corner_colors[0]);
                VecCopy(obuf[i + 1], corner_colors[1]);
                VecCopy(nbuf[i], corner_colors[2]);
                VecCopy(nbuf[i + 1], corner_colors[3]);
                corner_opacs[0] = oobuf[i];
                corner_opacs[1] = oobuf[i + 1];
                corner_opacs[2] = onbuf[i];
                corner_opacs[3] = onbuf[i + 1];

                throw_rays(eye, eye->view_from, viewvec, eye->view_up,
                    rightvec, eye->view_focaldist, eye->view_aperture, &ray,
                    xlen, ylen - ydelta, xdelta, ydelta,
                    corner_colors, corner_opacs,
                    avg_color, &avg_opacity, 0, maxdepth);

                if (Global::Display_Flag) {
                    display_plot(i, y - 1, avg_color);
                }
#if !defined( _WINDOWS )
                if ((runtimeState::Check_Abort_Flag == 1) && _kbhit()) {
#if defined( MAC ) || defined(__illumos__)
                    runtimeState::Abort_Flag = 1;
#else
                    runtimeState::Abort_Flag = getch();
#endif
                    goto end_of_scan;
                }
#endif
                Put_Pixel(eye, i, y - 1, avg_color, avg_opacity);
            }

            // C++ Modern Swap: Instantly updates the span endpoints without copying data blocks.
            std::swap(obuf, nbuf);
            std::swap(oobuf, onbuf);

        }
        else {
            /* First scan line configuration */
            obuf = nbuf;
            oobuf = onbuf;

            // Points the modern active buffers to the pre-allocated pool B
            nbuf = vec_pool_b;
            onbuf = flt_pool_b;
        }

        if (runtimeState::settings.tickflag == 2) {
            status((char*)"\r%d ", y);
        }
        if ((runtimeState::Check_Abort_Flag == 2) && _kbhit()) {
#if defined( MAC ) || defined(__illumos__)
            runtimeState::Abort_Flag = 1;
#else
            runtimeState::Abort_Flag = getch();
#endif
            goto end_of_scan;
        }
    }


end_of_scan:

}

/** @brief Render a vertical range of the image for the current viewpoint.
 *  @param eye      Active viewpoint.
 *  @param y_start  First scanline to render.
 *  @param y_end    Last scanline limit passed to the chosen scan routine.
 *  @return No return value.
 */
void Screen(Viewpoint *eye, int y_start, int y_end)
{
#ifdef DEBUG
    smessage("screen::Screen ystart=%d yend=%d\n",y_start,y_end);
#endif
   Vec viewvec, rightvec;
   Flt frustrumheight;
   Flt frustrumwidth;

   /* Calculate the "up" vector and ensure that it is perpendicular
      to the eye vector.  */
   VecNormalize(eye->view_up);
   VecSub(eye->view_at, eye->view_from, viewvec);

   if (eye->view_focaldist == -1.0) {
#ifdef DEBUG
       smessage("bpoint8 focal =-1\n");
#endif
       /* If the focal distance hasn't been set yet, then default to the
          distance from the eye to the point of interest */
       eye->view_focaldist = VecNormalize(viewvec);
#ifdef DEBUG
       smessage("bpoint9\n");
#endif
   }
   else {
#ifdef DEBUG
       smessage("bpoint8 VecNormalize\n");
#endif
       (void)VecNormalize(viewvec);
   }
   VecCross(eye->view_up, viewvec, rightvec);
   VecNormalize(rightvec);
   VecCross(viewvec, rightvec, eye->view_up);
   VecNormalize(eye->view_up);

   /* Calculate the height of the view frustrum in world coordinates.
      and then scale the right and up vectors appropriately. */
   frustrumheight = ((Flt)tan(eye->view_angle));
   frustrumwidth = eye->view_aspect * frustrumheight;
   VecScale(frustrumheight, eye->view_up);
   VecScale(frustrumwidth, rightvec);
#ifdef DEBUG
   smessage("bpoint10\n");
#endif

   /* Now go render the image.  The routine called is based on how
      much antialiasing needs to be performed on the image.  There is
      no antialiasing allowed for depth renders - it screws up the
      final result */
   if (RuntimeState::settings.antialias == 0 || RuntimeState::settings.DepthRender == 1)
      Scan(eye, viewvec, rightvec, y_start, y_end);
   else
      FilterScan(eye, viewvec, rightvec, RuntimeState::settings.antialias - 1, y_start, y_end);
}

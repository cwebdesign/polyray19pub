/* screen.c

   Step through all rays, checking them against all base objects for
   intersections.

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
#include "vector.h"
#include "io.h"
#include "screen.h"
#include "trace.h"
#include "display.h"
#include "memory.h"
#include "io.h"
#include "symtab.h"
#include "image.h"
#include "unixcompat.h"

static Flt
VecDist2(Vec a, Vec b)
{
   Vec t;
   VecSub(a, b, t);
   return VecDot(t, t);
}

static void
focal_blur(Vec from, Vec viewvec, Vec upvec,
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

static void
single_pixel(Viewpoint *Eye, Vec from, Vec viewvec, Vec upvec,
             Flt focaldist, Flt aperture, Ray *ray,
             int depth, Vec color, Flt *opacity)
{
   int sample_count, j;
   Vec avg_color, D;
   Flt avg_opacity;

   if (aperture == 0)
      Trace(Eye, 0, 1.0, ray, color, opacity, 1.0, &nRays);
   else {
      VecCopy(ray->D, D);
      /* MakeVector(0.0, 0.0, 0.0, color); */
      MakeVectorZero(color);
      *opacity = 0.0;
      sample_count = 1 + (maxsamples / (1 << depth));
      for (j=0;j<sample_count;j++) {
         VecCopy(D, ray->D);
         if (j > 0)
            focal_blur(from, viewvec, upvec, focaldist, aperture, ray);
         Trace(Eye, 0, 1.0, ray, avg_color, &avg_opacity, 1.0, &nRays);
         VecAdd(color, avg_color, color);
         *opacity += avg_opacity;
         }
      VecScale((1.0 / (Flt)sample_count), color);
      *opacity /= (Flt)sample_count;
      }
}

static void
throw_rays(Viewpoint *Eye, Vec from, Vec viewvec, Vec upvec, Vec rightvec,
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
        (VecDist2(corner_colors[0],corner_colors[1]) > antialias_threshold) ||
        (VecDist2(corner_colors[0],corner_colors[2]) > antialias_threshold) ||
        (VecDist2(corner_colors[1],corner_colors[3]) > antialias_threshold) ||
        (VecDist2(corner_colors[2],corner_colors[3]) > antialias_threshold))) {
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

static void
Scan(Viewpoint *eye, Vec viewvec, Vec rightvec, int ystart, int yend)
{
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
      current_row = y;
      ylen = 1.0 - ((Flt)(2 * y) / (Flt)eye->view_yres);
      for (x=0;x<eye->view_xres;x++) {
         current_col = x;
         xlen = ((Flt)(2 * x) / (Flt)eye->view_xres) - 1.0;
         VecComb(xlen, rightvec, ylen, eye->view_up, D);
         VecAdd(D, viewvec, D);
         VecNormalize(D);
         if (eye->view_aperture == 0) {
            VecCopy(D, ray.D);
            z = Trace(eye, 0, 1.0, &ray, color, &opacity, 1.0, &nRays);
            }
         else {
            /* MakeVector(0.0, 0.0, 0.0, color); */
            MakeVectorZero1(color);
            opacity = 0.0;
            for (j= 0;j<maxsamples;j++) {
               VecCopy(D, ray.D);
               focal_blur(eye->view_from, viewvec, eye->view_up,
                          eye->view_focaldist, eye->view_aperture, &ray);
               z = Trace(eye, 0, 1.0, &ray, avg, &avg_opacity, 1.0, &nRays);
               VecAdd(color, avg, color);
               }
            VecScale((1.0 / (Flt)maxsamples), color);
            opacity += avg_opacity / (Flt)maxsamples;
            }
         Put_Pixel(eye, x, y, color, opacity);
         if (z < ZBuffer_Read(eye, x, y))
            ZBuffer_Write(eye, x, y, z);

         if (Display_Flag)
            display_plot(x, y, color);
#if !defined( _WINDOWS )
         if ((Check_Abort_Flag == 1) && _kbhit()) {
#if defined( MAC )
            Abort_Flag = 1;
#else
            Abort_Flag = getch();
#endif
            return;
            }
#endif
         if (tickflag == 3)
            status("\r[%d, %d]     ", y, x);
         }
      if ((Check_Abort_Flag == 2) && _kbhit()) {
#if defined( MAC )
            Abort_Flag = 1;
#else
            Abort_Flag = getch();
#endif
         return;
         }
      if (tickflag == 2)
         status("\r%d ", y);
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

static void
FilterScan(Viewpoint *eye, Vec viewvec, Vec rightvec, int maxdepth,
           int ystart, int yend)
{
   Ray ray;
   int x, y, i, j;
   Flt xlen, ylen;
   Flt xdelta, ydelta;
   Vec *nbuf, *obuf, *tmp;
   Flt *onbuf, *oobuf, *otmp;
   Flt opacity, avg_opacity, corner_opacs[4];
   Vec D, color, avg_color, corner_colors[4];
   float z=0;

   /* allocate enough memory for the filter buffer */
   nbuf  = (Vec *)polyray_malloc((eye->view_xres + 1) * sizeof(Vec));
   onbuf = (Flt *)polyray_malloc((eye->view_xres + 1) * sizeof(Flt));
   if (nbuf == NULL || onbuf == NULL)
      error("Failed to allocate filter buffer");

   /* Initialize the second line buffers */
   obuf  = NULL; oobuf = NULL;

   VecCopy(eye->view_from, ray.P);

   ydelta = -2.0 / (Flt)eye->view_yres;
   xdelta =  2.0 / (Flt)eye->view_xres;
   ylen   = 1.0 - (2.0 * ystart / (Flt)eye->view_yres);

   for (y=ystart;y<=yend;y++,ylen+=ydelta) {
      current_row = y;
      for (x=0,xlen=-1.0;x<=eye->view_xres;x++,xlen+=xdelta) {
         current_col = x;
         VecComb(xlen, rightvec, ylen, eye->view_up, D);
         VecAdd(D, viewvec, D);
         VecNormalize(D);
         if (eye->view_aperture == 0) {
            VecCopy(D, ray.D);
            z = Trace(eye, 0, 1.0, &ray, color, &opacity, 1.0, &nRays);
            }
         else {
            /* MakeVector(0.0, 0.0, 0.0, color); */
            MakeVectorZero1(color);
            opacity = 0.0;
            for (j=0;j<maxsamples;j++) {
               VecCopy(D, ray.D);
               focal_blur(eye->view_from, viewvec, eye->view_up,
                          eye->view_focaldist, eye->view_aperture, &ray);
               z = Trace(eye, 0, 1.0, &ray, avg_color, &avg_opacity, 1.0, &nRays);
               VecAdd(color, avg_color, color);
               opacity += avg_opacity;
               }
            VecScale((1.0 / (Flt)maxsamples), color);
            opacity /= (Flt)maxsamples;
            }
         if (z < ZBuffer_Read(eye, x, y))
            ZBuffer_Write(eye, x, y, z);
         VecCopy(color, nbuf[x]);
         onbuf[x] = opacity;
         if (Display_Flag && x < eye->view_xres && y < eye->view_yres)
            display_plot(x, y, color);
#if !defined( _WINDOWS )
         if ((Check_Abort_Flag == 1) && _kbhit()) {
#if defined( MAC )
            Abort_Flag = 1;
#else
            Abort_Flag = getch();
#endif
            goto end_of_scan;
            }
#endif
         if (tickflag == 3)
            status("\r[%d, %d]     ", y, x) ;
         }
      if (obuf != NULL) {
         xlen = -1.0;
         for (i=0;i<eye->view_xres;i++,xlen+=xdelta) {
            current_col = i;
            VecCopy(obuf[i],   corner_colors[0]);
            VecCopy(obuf[i+1], corner_colors[1]);
            VecCopy(nbuf[i],   corner_colors[2]);
            VecCopy(nbuf[i+1], corner_colors[3]);
            corner_opacs[0] = oobuf[i];
            corner_opacs[1] = oobuf[i+1];
            corner_opacs[2] = onbuf[i];
            corner_opacs[3] = onbuf[i+1];
            throw_rays(eye, eye->view_from, viewvec, eye->view_up,
                       rightvec, eye->view_focaldist,
                       eye->view_aperture, &ray,
                       xlen, ylen-ydelta, xdelta, ydelta,
                       corner_colors, corner_opacs,
                       avg_color, &avg_opacity, 0, maxdepth);

            if (Display_Flag)
               display_plot(i, y-1, avg_color);
#if !defined( _WINDOWS )
            if ((Check_Abort_Flag == 1) && _kbhit()) {
#if defined( MAC )
               Abort_Flag = 1;
#else
               Abort_Flag = getch();
#endif
               goto end_of_scan;
               }
#endif
            Put_Pixel(eye, i, y-1, avg_color, avg_opacity);
            }
         /* Roll the buffers */
          tmp =  obuf;  obuf =  nbuf;  nbuf =  tmp;
         otmp = oobuf; oobuf = onbuf; onbuf = otmp;
         }
      else {
         /*  first scan line, set it up wierdly... */
         obuf  = nbuf;
         oobuf = onbuf;
         nbuf  = (Vec *)polyray_malloc((eye->view_xres + 1) * sizeof(Vec));
         onbuf = (Flt *)polyray_malloc((eye->view_xres + 1) * sizeof(Flt));
         if (nbuf == NULL || onbuf == NULL)
            error("Failed to allocate second filter buffer");
         }
      if (tickflag == 2)
         status("\r%d ", y) ;
      if ((Check_Abort_Flag == 2) && _kbhit()) {
#if defined( MAC )
         Abort_Flag = 1;
#else
         Abort_Flag = getch();
#endif
         goto end_of_scan;
         }
      }

end_of_scan:
   if (obuf != NULL) polyray_free(obuf);
   if (oobuf != NULL) polyray_free(oobuf);
   if (nbuf != NULL) polyray_free(nbuf);
   if (onbuf != NULL) polyray_free(onbuf);
}

void
Screen(Viewpoint *eye, int y_start, int y_end)
{
   Vec viewvec, rightvec;
   Flt frustrumheight;
   Flt frustrumwidth;

   /* Calculate the "up" vector and ensure that it is perpendicular
      to the eye vector.  */
   VecNormalize(eye->view_up);
   VecSub(eye->view_at, eye->view_from, viewvec);

   if (eye->view_focaldist == -1.0)
      /* If the focal distance hasn't been set yet, then default to the
         distance from the eye to the point of interest */
      eye->view_focaldist = VecNormalize(viewvec);
   else
      (void)VecNormalize(viewvec);
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

   /* Now go render the image.  The routine called is based on how
      much antialiasing needs to be performed on the image.  There is
      no antialiasing allowed for depth renders - it screws up the
      final result */
   if (antialias == 0 || DepthRender == 1)
      Scan(eye, viewvec, rightvec, y_start, y_end);
   else
      FilterScan(eye, viewvec, rightvec, antialias - 1, y_start, y_end);
}

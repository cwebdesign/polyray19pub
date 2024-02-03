/* scan.c

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
#include "memory.h"
#include "io.h"
#include "display.h"
#include "bound.h"
#include "scan.h"
#include "subdiv.h"
#include "vector.h"
#include "eval.h"
#include "csg.h"
#include "symtab.h"
#include "shade.h"
#include "image.h"

static void
copy_vertex(Vertex *v1, Vertex *v2)
{
   v2->w = v1->w;
   v2->S[2] = v1->S[2];
   VecCopy(v1->S, v2->S);
   VecCopy(v1->W, v2->W);
   if (Rendering_Method == GOURAD_SHADE)
      VecCopy(v1->P, v2->P)
   else if (Rendering_Method == SCAN_CONVERSION) {
      VecCopy(v1->N, v2->N);
      VecCopy(v1->U, v2->U);
      }
}

static void
calculate_vertex_delta(Vertex *lp, Vertex *rp,
                       Vertex *dx, float dt)
{
   dx->w = (rp->w - lp->w) * dt;
   dx->S[2] = (rp->S[2] - lp->S[2]) * dt;
   VecSub(rp->W, lp->W, dx->W);
   VecScale(dt, dx->W);
   if (Rendering_Method == GOURAD_SHADE) {
      VecSub(rp->P, lp->P, dx->P);
      VecScale(dt, dx->P);
      }
   else if (Rendering_Method == SCAN_CONVERSION) {
      VecSub(rp->N, lp->N, dx->N);
      VecScale(dt, dx->N);
      VecSub(rp->U, lp->U, dx->U);
      VecScale(dt, dx->U);
      }
}

static void
add_vertex_delta(Vertex *pt, float frac, Vertex *dp)
{
   pt->w += frac * dp->w;
   pt->S[2] += frac * dp->S[2];
   VecAddScaled(pt->W, frac, dp->W, pt->W);
   if (Rendering_Method == GOURAD_SHADE)
      VecAddScaled(pt->P, frac, dp->P, pt->P)
   else if (Rendering_Method == SCAN_CONVERSION) {
      VecAddScaled(pt->N, frac, dp->N, pt->N);
      VecAddScaled(pt->U, frac, dp->U, pt->U);
      }
}

static void
add_vertex_delta1(Vertex *pt, Vertex *dp)
{
   pt->w += dp->w;
   pt->S[2] += dp->S[2];
   VecAdd(pt->W, dp->W, pt->W);
   if (Rendering_Method == GOURAD_SHADE)
      VecAdd(pt->P, dp->P, pt->P)
   else if (Rendering_Method == SCAN_CONVERSION) {
      VecAdd(pt->N, dp->N, pt->N);
      VecAdd(pt->U, dp->U, pt->U);
      }
}

/* called at each pixel by poly_scan.  Returns 1 if the pixel was drawn,
   returns the distance to the pixel in depth. */
static int
pixelproc(Viewpoint *eye, Object *obj, Texture *tex,
          int x, int y, Vertex *pt, int edge_flag)
{
   float sz;
   Flt opacity;
   Vec C, color;
   Flt q, c0, c1;
   Vec L, N, P, W, U;
   Surface *surf;
   static Vec White = {1, 1, 1}, Black = {0, 0, 0};

   if ((Check_Abort_Flag == 1) && _kbhit())
      longjmp(abort_environ, 1);

   opacity = 1.0;

   /* Unhomogonize (curdle?) the object coordinates */
   q = 1.0 / pt->w;

#ifdef SIMPLE_DEPTH
   sz = q * pt->S[2];
#else
   VecCopy(pt->W, W);
   VecScale(q, W);
   VecSub(W, eye->view_from, L);
   sz = VecNormalize(L);
#endif

   if (sz > ZBuffer_Read(eye, x, y))
      return 0; /* Farther than an existing point */

#ifdef SIMPLE_DEPTH
   VecCopy(pt->W, W);
   VecScale(q, W);
#endif

   /* Do CSG checks on this point */
   if ((obj->o_parent != NULL) && !Inside_CSG_Node(obj->o_csg_tree, W))
      return 0; /* Not inside CSG */

   /* Check for dithering */
   if (obj->o_dither >= 0.0 && obj->o_dither < polyray_random())
      return 0; /* Dithered out of existence */

   if (Rendering_Method == SCAN_CONVERSION) {
      VecCopy(pt->U, U);
      VecScale(q, U);
      }
   else if (Rendering_Method == GOURAD_SHADE) {
      VecCopy(pt->P, P);
      VecScale(q, P);
      }

   if (DepthRender) {
#ifdef SIMPLE_DEPTH
      VecSub(W, eye->view_from, L);
      sz = VecNormalize(L);
#endif
      quantize_depth(sz, color, &opacity);
      }
   else {
      /* compute ambient, diffuse, and specular coeffs */
      if (edge_flag) {
#if defined( MAC )
         MakeVector(0, 0, 0, color)
#else
         MakeVector(1, 1, 1, color)
#endif
         }
      else if (Rendering_Method == WIRE_FRAME ||
               Rendering_Method == HIDDEN_LINE) {
         /* Wireframe gets rendered in black and white */
#if defined( MAC )
         MakeVector(1, 1, 1, color)
#else
         MakeVector(0, 0, 0, color)
#endif
         }
      else if (Rendering_Method == GOURAD_SHADE) {
         /* Gourad shading keeps the color in the P variable */
         VecCopy(P, color);
         }
      else {
#ifdef SIMPLE_DEPTH
         /* Get the direction from the eye to this point */
         VecSub(W, eye->view_from, L);
         sz = VecNormalize(L);
#endif
         /* unitize the normal vector */
         VecCopy(pt->N, N);
         if (VecNormalize(N) < EPSILON)
            error("Bad normal: <%g, %g, %g> on triangle\n");

         MakeVector(0.0, 0.0, 0.0, color);

         if (obj->o_sflag & TRANSMIT_CHECK) {
            fVec U0;

            VecCopy(U, U0)
            surf = find_surface(eye, obj, tex, W, N, L, U0, 1);
            /* If there are no color contributions from the surface
               (e.g., ambient, diffuse, specular, or reflection) and
               it is transparent then we can completely omit this
               pixel. */
            if ((fabs(surf->Kt_scale - 1.0) < EPSILON) &&
                (fabs(surf->Ka_scale) < EPSILON ||
                 VecClose(surf->Ka_color, Black)) &&
                (fabs(surf->Kd_scale) < EPSILON ||
                 VecClose(surf->Kd_color, Black)) &&
                (fabs(surf->Ks_scale) < EPSILON ||
                 VecClose(surf->Ks_color, Black)) &&
                VecClose(surf->Kt_color, White) &&
                (!obj->o_sflag & REFLECT_CHECK ||
                 !Global_Shade_Flag & REFLECT_CHECK ||
                 fabs(surf->Kr_scale) < EPSILON ||
                 VecClose(surf->Kr_color, Black)))
               /* Point is completely transparent, no point in
                  writing it out. */
               return 0;
            ShadeSurface(eye, obj, surf, 0, 1.0, 1.0, L, W, N, C, NULL);
            }
         else
               Shade(eye, obj, tex, 0, 1.0, 1.0, L, W, N, U, C);

         if (Global_Haze > 0.0 && Global_Haze < 1.0 &&
             sz > Global_Haze_Start) {
            c0 = pow(Global_Haze, (sz - Global_Haze_Start));
            c1 = 1.0 - c0;
            VecComb(c0, C, c1, Global_Haze_Color, color);
            }
         else {
            VecCopy(C, color);
            }
         }
      }

   /* Write the color to the output file */
   if (File_Generation_Flag)
      Put_Pixel(eye, x, y, color, opacity);

   /* If the display is active then draw the pixel on screen */
   if (Display_Flag) display_plot(x, y, color);

   /* Save the current depth */
   ZBuffer_Write(eye, x, y, sz);
   return 1;
}

static int
edgepixel(Viewpoint *eye, int x, int y)
{
   Vec color;

   if (x < 0 || x > eye->view_xres ||
       y < win.y0 || y > win.y1)
      return 0;

#if defined( MAC )
   MakeVector(0, 0, 0, color)
#else
   MakeVector(1, 1, 1, color)
#endif

   /* Write the color to the output file */
   if (File_Generation_Flag)
      Put_Pixel(eye, x, y, color, 1.0);

   /* If the display is active then draw the pixel on screen */
   if (Display_Flag) display_plot(x, y, color);

   return 1;
}

static void
compute_line_values(Vertex *v1, Vertex *v2,
                    int *x1, int *x2, int *y1, int *y2,
                    int *dx, int *dy, int *sx, int *sy,
                    float *dtdx, float *dtdy)
{
   /* Calculate the various bounds to do the line draw */
   *x1 = v1->S[0];
   *x2 = v2->S[0];
   *y1 = v1->S[1];
   *y2 = v2->S[1];
   *dx = *x2 - *x1;
   *dy = *y2 - *y1;
   *sx = SGN(*dx);
   *sy = SGN(*dy);
   *dtdx = (*dx == 0 ? 0.0 : (float)*sx / (float)*dx);
   *dtdy = (*dy == 0 ? 0.0 : (float)*sy / (float)*dy);
}

static void
edge_scan(Viewpoint *eye, Object *obj, Poly *p)
{
   int i, n, x, y, x1, y1, x2, y2;
   int ax, ay, sx, sy, dx, dy, d1;
   float dtdx, dtdy;
   Vertex l, r, dxp, dyp, pt;
   Vertex *lp, *rp, *tp;

   lp = &l;
   rp = &r;

   /* Set up first vertex */
   n = p->n-1;
   copy_vertex(&(p->vertices[n]), rp);
   rp->S[1] -= 0.5;
   for (i=0; i<=n; i++) {
      /* Draw the line between the last point and the current point */

      /* Step to the next edge */
      tp = rp; rp = lp; lp = tp;

      copy_vertex(&(p->vertices[i]), rp);
      rp->S[1] -= 0.5;

      /* Calculate the various bounds to do the line draw */
      compute_line_values(lp, rp, &x1, &x2, &y1, &y2, &dx, &dy,
                          &sx, &sy, &dtdx, &dtdy);

      x = x1; y = y1;
      ax = ABS(dx) << 1;
      ay = ABS(dy) << 1;

      /* Calculate the deltas between the two vertices */
      calculate_vertex_delta(lp, rp, &dxp, dtdx);
      calculate_vertex_delta(lp, rp, &dyp, dtdy);

      copy_vertex(lp, &pt);
      if (y >= win.y0 && y <= win.y1 && x <= win.x1)
         pixelproc(eye, obj, NULL, x, y, &pt, 1);

      if (ax > ay) {
         /* x dominant */
         d1 = ay - (ax >> 1);
         for (;;) {
            if (x == x2) break;
            if (d1 >= 0) {
               y += sy;
               d1 -= ax;
               }

            add_vertex_delta1(&pt, &dxp);
            x += sx;
            d1 += ay;

            if (y >= win.y0 && y <= win.y1 && x <= win.x1)
               pixelproc(eye, obj, NULL, x, y, &pt, 1);
            }
         }
      else {
         /* y dominant */
         d1 = ax - (ay >> 1);
         for (;;) {
            if (y == y2) break;
            add_vertex_delta1(&pt, &dyp);
            if (d1 >= 0) {
               x += sx;
               d1 -= ay;
               }
            y += sy;
            d1 += ax;

            if (y >= win.y0 && y <= win.y1 && x <= win.x1)
               pixelproc(eye, obj, NULL, x, y, &pt, 1);
            }
         }
      }
}

static void
poly_scan(Viewpoint *eye, Object *obj, Texture *tex, Poly *p)
{
   int i, li, ri, y, ly, ry, top=0, rem, w;
   int n, x, lx, rx;
   int xedge_flag, yedge_flag, edge_flag;
   int xindex, yindex;
   Flt ymin, ymax, xmin, xmax;
   Flt dy, dx, frac;
   Vertex l, r, dl, dr, pt, dp;
   Vertex *le, *re;

   /* Determine the bounds of the polygon.  While we
      are at it, we determine the index of the top
      vertex */
   n = p->n;
   xmin = ymin =  PLY_HUGE;
   xmax = ymax = -PLY_HUGE;
   for (i=0; i<n; i++) {
      dx = p->vertices[i].S[0];
      dy = p->vertices[i].S[1];
      if (dx < xmin) xmin = dx;
      if (dx > xmax) xmax = dx;
      if (dy > ymax) ymax = dy;
      if (dy < ymin) { ymin = dy; top = i; }
      }
   xmin = ceil(xmin - 0.5);
   if (xmin < win.x0) xmin = win.x0;
   xmax = floor(xmax - 0.5);
   if (xmax > win.x1) xmax = win.x1;
   ymin = ceil(ymin - 0.5);
   if (ymin < win.y0) ymin = win.y0;
   ymax = floor(ymax + 0.5);
   if (ymax > win.y1) ymax = win.y1;

   /* Set up the edge management arrays */
   for (i=0,w=xmax-xmin;i<=w;i++)
      eye->edgey[i] = ymax+2;
   for (i=0,w=ymax-ymin;i<=w;i++)
      eye->edgex[i] = xmax+2;

   li = ri = top; /* left and right vertex indices */
   rem = n;       /* number of vertices remaining */
   y = ymin;      /* current scan line */
   yindex = 0;    /* Index into x edge limit table */
   ly = ry = y-1; /* lower end of left & right edges */

   while (rem > 0) {
      /* scan in y, activating new edges on left & right as scan
         line passes over new vertices */

      /* advance left edge? */
      while (ly <= y && rem > 0) {
         rem--;
         i = li-1; /* step ccw down left side */
         if (i < 0) i = n-1;

         dy = p->vertices[i].S[1] - p->vertices[li].S[1];
         dy = (dy == 0.0 ? 1.0 : 1.0 / dy);
         frac = y + 0.5 - p->vertices[li].S[1];

         dl.S[0] = (p->vertices[i].S[0] - p->vertices[li].S[0]) * dy;
         calculate_vertex_delta(&p->vertices[li], &p->vertices[i], &dl, dy);

         copy_vertex(&p->vertices[li], &l);
         l.S[0] = p->vertices[li].S[0] + dl.S[0] * frac;/* Must be after copy */
         add_vertex_delta(&l, frac, &dl);

         ly = floor(p->vertices[i].S[1] + 0.5);
         li = i;
         }

      /* advance right edge? */
      while (ry <= y && rem > 0) {
         rem--;
         i = ri+1;              /* step cw down right edge */
         if (i >= n) i = 0;
         dy = p->vertices[i].S[1] - p->vertices[ri].S[1];
         dy = (dy == 0.0 ? 1.0 : 1.0 / dy);
         frac = y + 0.5 - p->vertices[ri].S[1];

         dr.S[0] = (p->vertices[i].S[0] - p->vertices[ri].S[0]) * dy;
         calculate_vertex_delta(&p->vertices[ri], &p->vertices[i], &dr, dy);

         copy_vertex(&p->vertices[ri], &r);
         r.S[0] = p->vertices[ri].S[0] + dr.S[0] * frac;/* Must be after copy */
         add_vertex_delta(&r, frac, &dr);

         ry = floor(p->vertices[i].S[1] + 0.5);
         ri = i;
         }

      while (y<ly && y<ry) {
         /* do scanlines till end of l or r edge */
         if (y >= win.y0 && y <= win.y1) {
            if (l.S[0] <= r.S[0]) {
               le = &l; re = &r;
               }
            else {
               le = &r; re = &l;
               }
            lx = ceil(le->S[0] - 0.5);
            if (lx < win.x0) lx = win.x0;
            rx = floor(re->S[0] - 0.5);
            if (rx > win.x1) rx = win.x1;
            if (lx <= rx) {
               dx = re->S[0] - le->S[0];
               dx = (dx == 0.0 ? 1.0 : 1.0 / dx);
               frac = lx + 0.5 - le->S[0];

               calculate_vertex_delta(le, re, &dp, dx);
               copy_vertex(le, &pt);
               add_vertex_delta(&pt, frac, &dp);

               for (xindex=lx-xmin,x=lx; x<=rx; xindex++,x++) {
                  /* scan in x, generating pixels */
                  yedge_flag = (y<eye->edgey[xindex] || x==lx || x==rx ? 1 : 0);
                  xedge_flag = (x < eye->edgex[yindex] ? 1 : 0);
                  edge_flag = (Rendering_Method == HIDDEN_LINE &&
                               (xedge_flag || yedge_flag) ? 1 : 0);
                  if (pixelproc(eye, obj, tex, x, y, &pt, edge_flag)) {
                     eye->edgey[xindex] = y;
                     eye->edgex[yindex] = x;
                     }
                  add_vertex_delta1(&pt, &dp);
                 }
               }
            }
         y++;
         yindex++;

         r.S[0] += dr.S[0];
         add_vertex_delta1(&r, &dr);

         l.S[0] += dl.S[0];
         add_vertex_delta1(&l, &dl);
         }
      }

   if (Rendering_Method == HIDDEN_LINE) {
      lx = xmin; rx = xmax;
      for (xindex=0,x=lx;x<=rx;xindex++,x++)
         if (eye->edgey[xindex] <= ymax)
            edgepixel(eye, x, eye->edgey[xindex]);
      lx = ymin; rx = y-1;
      for (yindex=0,y=lx;y<=rx;yindex++,y++)
         if (eye->edgex[yindex] <= xmax)
            edgepixel(eye, eye->edgex[yindex], y);
      }
}

static void
poly_outline(Poly *p)
{
   int i, sx0, sx1, sy0, sy1;
   Vertex *v = p->vertices;
#if defined( MAC ) || defined( _WINDOWS )
   static Vec BLACK  = { 0.0, 0.0, 0.0 };
#else
   static Vec WHITE  = { 1.0, 1.0, 1.0 };
#endif

   if ((Check_Abort_Flag == 1) && _kbhit())
      longjmp(abort_environ, 1);

   sx0 = (Flt)v[p->n-1].S[0];
   sy0 = (Flt)v[p->n-1].S[1];
   for (i=0;i<p->n;i++) {
      sx1 = (Flt)v[i].S[0];
      sy1 = (Flt)v[i].S[1];
#if defined( MAC ) || defined( _WINDOWS )
      display_line(sx0, sy0, sx1, sy1, BLACK);
#else
      display_line(sx0, sy0, sx1, sy1, WHITE);
#endif
      sx0 = sx1;
      sy0 = sy1;
      }
}

/* Determine the screen extent of a polygon */
#if 0
static void
poly_size(Viewpoint *eye, Poly *poly, int *x, int *y)
{
   int i, r, x0, x1, y0, y1;
   int sx, sy;
   Flt w;
   Poly p;
   Vertex *v;
   Transform *tx = eye->WS;

   p.n = poly->n;
   for (i=0;i<poly->n;i++)
      p.vertices[i] = poly->vertices[i];

   /* transform vertices from world space to homogeneous screen space */
   for (i=0; i<p.n; i++) {
      v = &p.vertices[i];
      v->w = v->W[0] * tx->matrix[0][3] +
             v->W[1] * tx->matrix[1][3] +
             v->W[2] * tx->matrix[2][3] +
                       tx->matrix[3][3];
      fTxVec(v->S, v->W, tx);
      }

   if ((r = poly_clip_to_box(&p, &box)) == POLY_CLIP_OUT) {
      *x = 0; *y = 0;
      return;
      }

   /* do homogeneous division of screen position, object position */
   for (i=0; i<p.n; i++) {
      v = &p.vertices[i];
      w = 1.0 / v->w;
      VecScale(w, v->S);
      }

   v = &p.vertices[0];
   x0 = x1 = v[0].S[0];
   y0 = y1 = v[0].S[1];
   for (i=1;i<p.n;i++) {
      sx = v[i].S[0];
      sy = v[i].S[1];
      if (sx < x0) x0 = sx;
      if (sx > x1) x1 = sx;
      if (sy < y0) y0 = sy;
      if (sy > y1) y1 = sy;
      }
   *x = x1 - x0;
   *y = y1 - y0;
}
#endif

void
BboxScreenSize(Viewpoint *eye, bbox_info *bbox, int *x, int *y)
{
   int i, j, x0, x1, y0, y1;
   int sx, sy, maxx, maxy;
   fVec lower_left, lengths;
   fVec W, S;
   Flt w;
   Transform *tx = eye->WS;

   VecCopy(bbox->lower_left, lower_left);
   VecCopy(bbox->lengths, lengths);
   x0 = y0 =  PLY_HUGE;
   x1 = y1 = -PLY_HUGE;

   maxx = eye->view_xres;
   maxy = eye->view_yres;

   /* First see if the eye is either inside the bounding box or
      perhaps in front of it. */
   for (i=0,j=0;i<3;i++)
      if (eye->view_from[i] > lower_left[i] &&
          eye->view_from[i] < lower_left[i] + lengths[i])
         j++;
   if (j == 3) {
      /* bounding box covers the entire field of view */
      *x = maxx;
      *y = maxy;
      return;
      }

   /* Transform each corner of the bounding box and see where it
      goes */
   for (i=1;i<8;i++) {
      VecCopy(lower_left, W);
      W[0] += ((i & 1) ? lengths[0] : 0.0);
      W[1] += ((i & 2) ? lengths[1] : 0.0);
      W[2] += ((i & 4) ? lengths[2] : 0.0);
      fTxVec(S, W, tx);
      w = W[0] * tx->matrix[0][3] + W[1] * tx->matrix[1][3] +
          W[2] * tx->matrix[2][3] + tx->matrix[3][3];
      w = 1.0 / w;
      VecScale(w, S);
      sx = S[0];
      if (sx < 0) sx = 0;
      if (sx > maxx) sx = maxx;
      sy = S[1];
      if (sy < 0) sy = 0;
      if (sy > maxy) sy = maxy;
      if (sx < x0) x0 = sx;
      if (sx > x1) x1 = sx;
      if (sy < y0) y0 = sy;
      if (sy > y1) y1 = sy;
      }

   /* Now chop the boundaries against the image window */
   y0 = MAX(y0, win.y0);
   y1 = MIN(y1, win.y1);

   *x = x1 - x0;
   *y = y1 - y0;
}

/* W0 must be ok and W1 the one we want to move.  The resulting
   position of W1 is returned in Wres */
#define MAX_CSG_SUBDIVISIONS 10
static void
csg_subdivide_loop(Object *obj, Vec W0, Vec W1, Vec Wres)
{
   int i;
   Vec tW0, tW1, Wmid;

   VecCopy(W0, tW0)
   VecCopy(W1, tW1)
   for (i=0;i<MAX_CSG_SUBDIVISIONS;i++) {
      VecAdd(tW0, tW1, Wmid)
      VecScale(0.5, Wmid)
      if (Inside_CSG_Node(obj->o_csg_tree, Wmid)) {
         VecCopy(Wmid, tW0)
         }
      else {
         VecCopy(Wmid, tW1)
         }
      }
   VecCopy(Wmid, Wres)
}

static void
emit_raw_triangle(Vec W0, Vec W1, Vec W2)
{
   printf("%.4g %.4g %.4g ",  W0[0], W0[1], W0[2]);
   printf("%.4g %.4g %.4g ",  W1[0], W1[1], W1[2]);
   printf("%.4g %.4g %.4g\n", W2[0], W2[1], W2[2]);
}

/* Use binary subdivision to more closely approximate where a triangle
   enters and exits CSG.  This function returns 1 if there was any
   adjustment needed to the triangle legs. */
static short
slice_csg_triangle(Object *obj, Vec W0, Vec W1, Vec W2,
                   short flag0, short flag1, short flag2,
                   int depth)
{
   Vec tW0, tW1, tW2;

   if (!flag0) {
      if (!flag1) {
         csg_subdivide_loop(obj, W2, W0, tW0);
         csg_subdivide_loop(obj, W2, W1, tW1);
         emit_raw_triangle(tW0, tW1, W2);
         return 1;
         }
      else if (!flag2) {
         csg_subdivide_loop(obj, W1, W0, tW0);
         csg_subdivide_loop(obj, W1, W2, tW2);
         emit_raw_triangle(tW0, W1, tW2);
         return 1;
         }
      else {
         csg_subdivide_loop(obj, W1, W0, tW0);
         csg_subdivide_loop(obj, W2, W0, tW1);
         emit_raw_triangle(W1, tW0, tW1);
         emit_raw_triangle(W1, tW1, W2);
         return 1;
         }
      }
   else if (!flag1) {
      if (!flag2) {
         csg_subdivide_loop(obj, W0, W1, tW1);
         csg_subdivide_loop(obj, W0, W2, tW2);
         emit_raw_triangle(W0, tW1, tW2);
         return 1;
         }
      else {
         csg_subdivide_loop(obj, W0, W1, tW0);
         csg_subdivide_loop(obj, W2, W1, tW1);
         emit_raw_triangle(W0, tW0, tW1);
         emit_raw_triangle(W0, tW1, W2);
         return 1;
         }
      }
   else if (!flag2) {
      csg_subdivide_loop(obj, W0, W2, tW0);
      csg_subdivide_loop(obj, W1, W2, tW1);
      emit_raw_triangle(W0, W1, tW1);
      emit_raw_triangle(W0, tW1, tW0);
      return 1;
      }
   else if (depth == 0) {
      emit_raw_triangle(W0, W1, W2);
      return 1;
      }
   else
      return 0;
}

static short
check_leg_lengths(Vec W0, Vec W1, Vec W2, Flt tolerance)
{
   Vec LegLen;
   short divide_flag = 0;

   VecSub(W0, W1, LegLen)
   if (fabs(LegLen[0]) > tolerance ||
       fabs(LegLen[1]) > tolerance ||
       fabs(LegLen[2]) > tolerance)
      divide_flag = 1;
   VecSub(W0, W2, LegLen)
   if (fabs(LegLen[0]) > tolerance ||
       fabs(LegLen[1]) > tolerance ||
       fabs(LegLen[2]) > tolerance)
      divide_flag = 1;
   VecSub(W1, W2, LegLen)
   if (fabs(LegLen[0]) > tolerance ||
       fabs(LegLen[1]) > tolerance ||
       fabs(LegLen[2]) > tolerance)
      divide_flag = 1;
   return divide_flag;
}

/* Use binary subdivision to more closely approximate where a triangle
   enters and exits CSG */
static short
subdiv_triangle(Object *obj, Vec W0, Vec W1, Vec W2, int depth)
{
   short f0, f1, f2, f3;
   Vec mW0, mW1, mW2;

   /* Check to see if this triangle is entirely within CSG */
   f0 = Inside_CSG_Node(obj->o_csg_tree, W0);
   f1 = Inside_CSG_Node(obj->o_csg_tree, W1);
   f2 = Inside_CSG_Node(obj->o_csg_tree, W2);

   if (!check_leg_lengths(W0, W1, W2, csg_leg_tolerance) ||
       depth > csg_subdivision_depth) {
      return slice_csg_triangle(obj, W0, W1, W2, f0, f1, f2, depth);
      }

   /* We need to further subdivide the triangle based on the
      leg lengths of the triangle */
   VecAdd(W0, W1, mW0) VecAdd(W1, W2, mW1) VecAdd(W2, W0, mW2)
   VecScale(0.5, mW0) VecScale(0.5, mW1) VecScale(0.5, mW2)

   f0 = subdiv_triangle(obj,  W0, mW0, mW2, depth+1);
   f1 = subdiv_triangle(obj, mW0,  W1, mW1, depth+1);
   f2 = subdiv_triangle(obj, mW0, mW1, mW2, depth+1);
   f3 = subdiv_triangle(obj, mW1,  W2, mW2, depth+1);

   if (f0 + f1 + f2 + f3 == 0) {
      if (depth == 0) {
         emit_raw_triangle(W0, W1, W2);
         return 1;
         }
      else
         /* The printing will be done at a higher level */
         return 0;
      }
   else {
      if (!f0) emit_raw_triangle( W0, mW0, mW2);
      if (!f1) emit_raw_triangle(mW0,  W1, mW1);
      if (!f2) emit_raw_triangle(mW0, mW1, mW2);
      if (!f3) emit_raw_triangle(mW1,  W2, mW2);
      return 1;
      }
}

/* The polygon has to be convex before this point is reached
   (in fact, it will be either three or four sided) */
static void
poly_raw_output(Object *obj, Poly *p)
{
   int i, j;
   Vertex *v[3];
   Vec W0, W1, W2;

   if ((Check_Abort_Flag == 1) && _kbhit())
      longjmp(abort_environ, 1);

   v[0] = &p->vertices[0];
   for (i=1;i<p->n-1;i++) {
      v[1] = &p->vertices[i];
      v[2] = &p->vertices[i+1];
      if (Rendering_Method == CSG_TRIANGLES && obj->o_parent != NULL) {
         VecCopy(v[0]->W, W0)
         VecCopy(v[1]->W, W1)
         VecCopy(v[2]->W, W2)
         subdiv_triangle(obj, W0, W1, W2, 0);
         }
      else {
         for (j=0;j<3;j++)
            printf("%.4g %.4g %.4g ", v[j]->W[0], v[j]->W[1], v[j]->W[2]);
         if (Rendering_Method == UV_TRIANGLES) {
            for (j=0;j<3;j++)
               printf("%.4g %.4g %.4g ", v[j]->N[0], v[j]->N[1], v[j]->N[2]);
            for (j=0;j<3;j++)
               printf("%.4g %.4g ", v[j]->U[0], v[j]->U[1]);
            }
         printf("\n");
         }
      }
}

static void
poly_obj_output(BinTree *Root, Object *obj, Poly *p)
{
   TriangleObject *tri_obj;
   fVec *verts, *norms, *uvals;
   Vec P1, P2, N;
   int **out_verts;
   int i, u_axis, v_axis;
   int new_n, old_n, out_n;
   bbox_info box;

   new_n = p->n;
   old_n = (obj->o_vertices == NULL ? 0 : obj->o_vertices->n);
   verts = (fVec *)polyray_malloc((new_n + old_n) * sizeof(fVec));
   norms = (fVec *)polyray_malloc((new_n + old_n) * sizeof(fVec));
   uvals = (fVec *)polyray_malloc((new_n + old_n) * sizeof(fVec));
   if (verts == NULL || norms == NULL || uvals == NULL)
      error("Insufficient polygon memory");

   if (obj->o_vertices == NULL) {
      obj->o_vertices = (ObjectVertices *)
                        polyray_malloc(sizeof(ObjectVertices));
      obj->o_vertices->n = new_n;
      }
   else {
      /* Copy any old vertices into the new list */
      for (i=0;i<old_n;i++) {
         VecCopy(obj->o_vertices->V[i], verts[i]);
         if (obj->o_vertices->N != NULL)
            VecCopy(obj->o_vertices->N[i], norms[i])
         if (obj->o_vertices->U != NULL)
            VecCopy(obj->o_vertices->U[i], uvals[i])
         }
      polyray_free(obj->o_vertices->V);
      if (obj->o_vertices->N != NULL)
         polyray_free(obj->o_vertices->N);
      if (obj->o_vertices->U != NULL)
         polyray_free(obj->o_vertices->U);
      }

   /* Copy the new vertices into the list */
   for (i=old_n;i<old_n+new_n-1;i++) {
      VecCopy(p->vertices[i].W, verts[i]);
      if (obj->o_vertices->N != NULL)
         VecCopy(p->vertices[i].N, norms[i]);
      if (obj->o_vertices->U != NULL)
         VecCopy(p->vertices[i].U, uvals[i]);
      }
   obj->o_vertices->V = verts;
   obj->o_vertices->N = norms;
   obj->o_vertices->U = uvals;

   /* Allocate space to hold the intermediate polygon stacks */
   out_verts = (int **)polyray_malloc((new_n - 2) * sizeof(int *));
   if (out_verts == NULL)
      error("Insufficient memory to split polygon");
   for (i=0;i<new_n-2;i++) {
      out_verts[i] = (int *)polyray_malloc(3 * sizeof(int));
      if (out_verts[i] == NULL)
         error("Insufficient memory to split polygon");
      }

   /* Calculate the normal by giving various cross products */
   VecSub(verts[old_n+1], verts[old_n], P1);
   VecSub(verts[old_n+2], verts[old_n], P2);
   VecCross(P1, P2, N);
   if (fabs(N[0]) >= fabs(N[1]) && fabs(N[0]) >= fabs(N[2])) {
      u_axis = 1;
      v_axis = 2;
      }
   else if (fabs(N[1]) >= fabs(N[0]) && fabs(N[1]) >= fabs(N[2])) {
      u_axis = 0;
      v_axis = 2;
      }
   else {
      u_axis = 0;
      v_axis = 1;
      }

   /* Slice the polygon into triangles */
   Split_Polygon(new_n, &verts[old_n], u_axis, v_axis,
                 &out_n, out_verts);

   /* Now add the triangles to the list of objects */
   for (i=0;i<out_n;i++) {
      tri_obj = (TriangleObject *)polyray_malloc(sizeof(TriangleObject));
      if (tri_obj == NULL)
         error("Insufficient polygon memory");
      tri_obj->o_type = T_POLYGON;
      tri_obj->o_parent = obj;
      tri_obj->o_vert[0] = out_verts[i][0] + old_n;
      tri_obj->o_vert[1] = out_verts[i][1] + old_n;
      tri_obj->o_vert[2] = out_verts[i][2] + old_n;
      tri_obj->o_texture = obj->o_texture;
      if (calc_triangle_bounds(tri_obj, &box)) {
         /* Now add this triangle object to the root */
         VecCopy(box.lower_left, tri_obj->o_bnd.lower_left);
         VecCopy(box.lengths, tri_obj->o_bnd.lengths);
         Root->members.list = push_object(Root->members.list,
                                          (Object *)tri_obj);
         Root->members.count++;
         }
      else
         polyray_free(tri_obj);
      }

   /* Free the temporary polygon storage */
   for (i=0;i<new_n-2;i++)
      polyray_free(out_verts[i]);
   polyray_free(out_verts);
}

void
scan_convert(Viewpoint *eye, BinTree *Root, Object *obj, Texture *tex, Poly *p)
{
   int i, r;
   Vertex *v;
   Flt w;
   Vec L, C, W, N, U;
   Transform *tx;

   if (Rendering_Method == RAW_TRIANGLES ||
       Rendering_Method == CSG_TRIANGLES ||
       Rendering_Method == UV_TRIANGLES) {
      poly_raw_output(obj, p);
      return;
      }
   else if (Rendering_Method == MESH_CONVERSION) {
      poly_obj_output(Root, obj, p);
      return;
      }
   else if (eye == NULL)
      /* No eye transformation - this should never happen */
      error("No perspective transformation for scan conversion");

   tx = eye->WS;

   /* transform vertices from world space to homogeneous screen space */
   for (i=0; i<p->n; i++) {
      v = &p->vertices[i];
      v->w = v->W[0] * tx->matrix[0][3] +
             v->W[1] * tx->matrix[1][3] +
             v->W[2] * tx->matrix[2][3] +
                       tx->matrix[3][3];
      fTxVec(v->S, v->W, tx);
/* Backface culling - seems to speed things up around 15%. */
/*
TxNormal(N, v->N, tx);
if (N[2] < 0) {
   return;
   }
*/
      }

   if ((r = poly_clip_to_box(p, &box)) == POLY_CLIP_OUT)
      return;

   /* do homogeneous division of screen position, object position */
   for (i=0; i<p->n; i++) {
      v = &p->vertices[i];
      w = 1.0 / v->w;
      v->w = w;
      VecScale(w, v->S);
      if (Rendering_Method == GOURAD_SHADE) {
         /* Put the color into P */
         VecSub(v->W, eye->view_from, L);
         VecNormalize(L);
         VecCopy(v->W, W);
         VecCopy(v->N, N);
         VecCopy(v->U, U);
         Shade(eye, obj, NULL, 0, 1.0, 1.0, L, W, N, U, C);
         VecCopy(C, v->P);
         VecScale(w, v->P);
         }
      else if (Rendering_Method == SCAN_CONVERSION) {
         VecScale(w, v->P);
         VecScale(w, v->U);
         }
      VecScale(w, v->W);
      }

   if (Rendering_Method == WIRE_FRAME && !File_Generation_Flag)
      poly_outline(p);
   else if (Rendering_Method == WIRE_FRAME)
      edge_scan(eye, obj, p);
   else /* SCAN_CONVERSION, GOURAD_SHADE, HIDDEN_LINE */
      poly_scan(eye, obj, tex, p);
}

void
render_prim(Viewpoint *eye, BinTree *Root, Object *pobj, Object *obj)
{
   int i;
   CompositeObject *cobj;
   TriangleObject *tobj;
   fVec *V, *N, *U, fB0, B1, N0;// B0 conflicts on macos
   float d;
   Poly P;
   Poly *Polygon = &P;
   N0[0]=N0[1]=N0[2]=0.0;//cm initing N0

   switch (obj->o_type) {
      case T_CSG:
      case T_BEZIER:
      case T_BLOB:
      case T_BOX:
      case T_CONE:
      case T_CYLINDER:
      case T_CYL_HEIGHT_FIELD:
      case T_DISC:
      case T_FUNCTION:
      case T_GLYPH:
      case T_GRIDDED:
      case T_HEIGHT_FIELD:
      case T_HYPERTEXTURE:
      case T_NURB:
      case T_PARABOLA:
      case T_PARAMETRIC:
      case T_POLY:
      case T_POLYNOMIAL:
      case T_RAW_TRIANGLES:
      case T_REVOLVE:
      case T_SPHERE:
      case T_SPH_HEIGHT_FIELD:
      case T_SUPERQ:
      case T_SWEEP:
      case T_TORUS:
      case T_TRI:
         obj->o_procs->render(eye, Root, obj);
         break;
      case T_COMPOSITE:
         cobj = (CompositeObject *)obj;
         for (i=0;i<cobj->c_size;i++)
            render_prim(eye, Root, pobj, cobj->c_object[i]);
         break;
      case T_POLYGON:
         tobj = (TriangleObject *)obj;
         if (pobj == NULL || pobj->o_type != T_RAW_TRIANGLES)
            /* Triangles created by MESH_CONVERSION have their parents
               set correctly and don't need it forced. Triangles read
               in from a raw file and used in a define statement will
               have the parent set to the defined object rather than
               the instantiated object. */
            pobj = tobj->o_parent;
         i = pobj->o_type;
         V = pobj->o_vertices->V;
         U = pobj->o_vertices->U;
         N = pobj->o_vertices->N;
         Polygon->n = 3;
         if (N == NULL) {
            /* Calculate the normal to the triangle */
            VecSub(V[tobj->o_vert[1]], V[tobj->o_vert[0]], fB0);
            VecSub(V[tobj->o_vert[2]], V[tobj->o_vert[0]], B1);
            VecCross(fB0, B1, N0);
            d = VecDot(N0, N0);
            d = (d < EPSILON ? 1.0 : 1.0 / d);
            VecScale(d, N0);
            }
         for (i=0;i<3;i++) {
            VecCopy(V[tobj->o_vert[i]], Polygon->vertices[i].W);
            VecCopy(V[tobj->o_vert[i]], Polygon->vertices[i].P);
            if (U == NULL)
               VecCopy(V[tobj->o_vert[i]], Polygon->vertices[i].U)
            else
               VecCopy(U[tobj->o_vert[i]], Polygon->vertices[i].U)
            if (N == NULL)
               VecCopy(N0, Polygon->vertices[i].N)
            else
               VecCopy(N[tobj->o_vert[i]], Polygon->vertices[i].N)
            }

         if (pobj->o_type == T_RAW_TRIANGLES &&
             pobj->o_trans != NULL)
            for (i=0;i<3;i++) {
               fTxVec(Polygon->vertices[i].W, Polygon->vertices[i].P,
                      pobj->o_trans);
               fTxNormal(Polygon->vertices[i].N, Polygon->vertices[i].N,
                         pobj->o_trans);
               fVecNormalize(Polygon->vertices[i].N);
               }

         scan_convert(eye, Root, pobj, tobj->o_texture, Polygon);
         break;
      default:
         break;
      }
}

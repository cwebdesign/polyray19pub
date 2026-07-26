/* scan.cc

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
#include "io_ply.h"
#include "display.h"
#include "bound.h"
#include "scan.h"
#include "subdiv.h"
#include "vector.h"
#include "eval.h"
#include "csg.h"
#include "runtime_state.h"
#include "symtab.h"
#include "shade.h"
#include "image.h"
#include "enqueue.h"
#include "factory.h"
#include "vec3.h"
#include "image.h"
#include "util.h"

using namespace openpolyray;

#ifndef TESTING
static
#endif
/** @brief Copy the interpolated fields needed for scan conversion between vertices.
 *  @param v1 Source vertex.
 *  @param v2 Destination vertex.
 *  @return No return value.
 */
void copy_vertex(Vertex* v1, Vertex* v2)
{
    /// <summary>
    /// w, S and W are copied. 
    /// for Gouraud Shading P is copied too
    /// for Scan Conversion N and U are copied too
    /// </summary>
    /// <param name="v1">src</param>
    /// <param name="v2">dest</param>
   v2->w = v1->w;
   v2->S[2] = v1->S[2];
   VecCopy(v1->S, v2->S);
   VecCopy(v1->W, v2->W);
   if (runtimeState::settings.Render_Method == rmode::GOURAD_SHADE)
      VecCopy(v1->P, v2->P)
   else if (runtimeState::settings.Render_Method == rmode::SCAN_CONVERSION) {
      VecCopy(v1->N, v2->N);
      VecCopy(v1->U, v2->U);
      }
}

#ifndef TESTING
static
#endif
/** @brief Compute per-step vertex deltas for interpolation between two vertices.
 *  @param lp      Left or start vertex.
 *  @param rp      Right or end vertex.
 *  @param out_dx  Output delta vertex.
 *  @param dt      Reciprocal step count or interpolation increment.
 *  @return No return value.
 */
void calculate_vertex_delta(Vertex* lp, Vertex* rp, Vertex* out_dx, float dt)
{
    out_dx->w = (rp->w - lp->w) * dt;
    out_dx->S[2] = (rp->S[2] - lp->S[2]) * dt;
   VecSub(rp->W, lp->W, out_dx->W);
   VecScale(dt, out_dx->W);
   if (runtimeState::settings.Render_Method == rmode::GOURAD_SHADE) {
      VecSub(rp->P, lp->P, out_dx->P);
      VecScale(dt, out_dx->P);
      }
   else if (runtimeState::settings.Render_Method == rmode::SCAN_CONVERSION) {
      VecSub(rp->N, lp->N, out_dx->N);
      VecScale(dt, out_dx->N);
      VecSub(rp->U, lp->U, out_dx->U);
      VecScale(dt, out_dx->U);
      }
}

#ifndef TESTING
static
#endif
/** @brief Advance a vertex by a fractional multiple of a delta vertex.
 *  @param pt    Vertex updated in place.
 *  @param frac  Fractional scale applied to @p dp.
 *  @param dp    Delta vertex to apply.
 *  @return No return value.
 */
void add_vertex_delta(Vertex* pt, float frac, Vertex* dp)
{
   pt->w += frac * dp->w;
   pt->S[2] += frac * dp->S[2];
   VecAddScaled(pt->W, frac, dp->W, pt->W);
   if (runtimeState::settings.Render_Method == rmode::GOURAD_SHADE)
      VecAddScaled(pt->P, frac, dp->P, pt->P)
   else if (runtimeState::settings.Render_Method == rmode::SCAN_CONVERSION) {
      VecAddScaled(pt->N, frac, dp->N, pt->N);
      VecAddScaled(pt->U, frac, dp->U, pt->U);
      }
}

#ifndef TESTING
static
#endif
/** @brief Advance a vertex by one full delta step.
 *  @param pt Vertex updated in place.
 *  @param dp Delta vertex to apply.
 *  @return No return value.
 */
void add_vertex_delta1(Vertex* pt, Vertex* dp)
{
   pt->w += dp->w;
   pt->S[2] += dp->S[2];
   VecAdd(pt->W, dp->W, pt->W);
   if (runtimeState::settings.Render_Method == rmode::GOURAD_SHADE)
      VecAdd(pt->P, dp->P, pt->P)
   else if (runtimeState::settings.Render_Method == rmode::SCAN_CONVERSION) {
      VecAdd(pt->N, dp->N, pt->N);
      VecAdd(pt->U, dp->U, pt->U);
      }
}

// W = true world-space position of pt
static void unhomogenize(const Vertex* pt, Flt q, NuVec& W) {
    VecCopy(pt->W, W);
    VecScale(q, W);
}

// W = true world-space position of pt
static void unhomogenize_vec3(const Vertex* pt, Flt q, Vec3& W) {
    W = Vec3(pt->W);
    VecCopy(pt->W, W);
    VecScale(q, W);
}

static void OutputColour(Viewpoint* eye, int x, int y, Vec& color, Flt opacity)
{
    // Write the color to the output file 
    if (RuntimeState::File_Generation_Flag)
        Put_Pixel(eye, x, y, color, opacity);

    /* If the display is active then draw the pixel on screen */
    if (Global::Display_Flag) display_plot(x, y, color);
}

/* If there are no color contributions from the surface
   (e.g., ambient, diffuse, specular, or reflection) and it is
   transparent then the pixel can be completely omitted. */
static bool isInvisiblePixel(Surface* surf, Vec& Black, Vec& White, Object* obj)
{
    return (fabs(surf->Kt_scale - 1.0) < PLY_EPSILON) &&
        (fabs(surf->Ka_scale) < PLY_EPSILON ||
            VecClose(surf->Ka_color, Black)) &&
        (fabs(surf->Kd_scale) < PLY_EPSILON ||
            VecClose(surf->Kd_color, Black)) &&
        (fabs(surf->Ks_scale) < PLY_EPSILON ||
            VecClose(surf->Ks_color, Black)) &&
        VecClose(surf->Kt_color, White) &&
        (!(obj->o_sflag & REFLECT_CHECK) ||
            !(runtimeState::scene.Global_Shade_Flag & REFLECT_CHECK) ||
            fabs(surf->Kr_scale) < PLY_EPSILON ||
            VecClose(surf->Kr_color, Black));
}


/* called at each pixel by poly_scan.  Returns 1 if the pixel was drawn,
   returns the distance to the pixel in depth. */
/** @brief Shade and write a single scan-converted pixel.
 *  @param eye       Active viewpoint.
 *  @param obj       Object owning the polygon.
 *  @param tex       Optional texture override.
 *  @param x         Pixel x coordinate.
 *  @param y         Pixel y coordinate.
 *  @param pt        Interpolated per-pixel vertex data.
 *  @param edge_flag Non-zero when the pixel lies on a polygon edge.
 *  @return 1 when the pixel is emitted, 0 when it is rejected.
 */
#ifndef TESTING
static
#endif
int pixelproc(Viewpoint *eye, Object *obj, Texture *tex,
          int x, int y, Vertex *pt, int edge_flag)
{
   float sz;
   Flt opacity;
   Vec C;
   Flt c0, c1;
   Vec L, N, P, U;
   Surface *surf;
   static Vec White = {1, 1, 1}, Black = {0, 0, 0};

   if ((runtimeState::Check_Abort_Flag == 1) && _kbhit())
       throw std::runtime_error("ABORT");

   opacity = 1.0;

   /* Unhomogonize (curdle?) the object coordinates */
   Flt q = 1.0 / pt->w;

   NuVec W;
#ifdef SIMPLE_DEPTH
   sz = q * pt->S[2];
#else
   unhomogenize(pt, q, W);
   VecSub(W, eye->view_from, L);
   sz = VecNormalize(L);
#endif

   if (sz > ZBuffer_Read(eye, x, y))
      return 0; /* Farther than an existing point */

#ifdef SIMPLE_DEPTH
   VecCopy(pt->W, W);
   VecScale(q, W);
#endif
   
   NuVec colour{0,0,0};
   Vec cW;
   assignNuVecToFVec(W, cW);//lets use Vec again for now
   /* Do CSG checks on this point */
   if ((obj->o_parent != nullptr) && !Inside_CSG_Node(obj->o_csg_tree, cW))
      return 0; /* Not inside CSG */

   /* Check for dithering */
   if (obj->o_dither >= 0.0 && obj->o_dither < polyray_random())
      return 0; /* Dithered out of existence */

   if (runtimeState::settings.Render_Method == rmode::SCAN_CONVERSION) {
      VecCopy(pt->U, U);
      VecScale(q, U);
      }
   else if (runtimeState::settings.Render_Method == rmode::GOURAD_SHADE) {
      VecCopy(pt->P, P);
      VecScale(q, P);
      }

   if (runtimeState::settings.DepthRender) {
#ifdef SIMPLE_DEPTH
      VecSub(W, eye->view_from, L);
      sz = VecNormalize(L);
#endif
      quantize_depth_CPP(sz, colour, opacity);
      }
   else {
      /* compute ambient, diffuse, and specular coeffs */
      if (edge_flag) {
#if defined( MAC )
         colour={0,0,0};//MakeVector(0, 0, 0, color)
#else
         colour={1,1,1};//MakeVector(1, 1, 1, color)
#endif
         }
      else if (runtimeState::settings.Render_Method == rmode::WIRE_FRAME ||
               runtimeState::settings.Render_Method == rmode::HIDDEN_LINE) {
         /* Wireframe gets rendered in black and white */
#if defined( MAC )
         colour={1,1,1};//MakeVector(1, 1, 1, color)
#else
         colour={0,0,0};
#endif
         }
      else if (runtimeState::settings.Render_Method == rmode::GOURAD_SHADE) {
         /* Gouraud shading keeps the colour in the P variable */
         VecCopy(P, colour);         
         }
      else {
#ifdef SIMPLE_DEPTH
         /* Get the direction from the eye to this point */
         VecSub(W, eye->view_from, L);
         sz = VecNormalize(L);
#endif
         /* unitize the normal vector */
         VecCopy(pt->N, N);
         if (VecNormalize(N) < PLY_EPSILON)
            serror("Bad normal: <%g, %g, %g> on triangle\n");

         colour={ 0.0, 0.0, 0.0};

         if (obj->o_sflag & TRANSMIT_CHECK) {
            fVec U0;
            VecCopy(U, U0)
            surf = find_surface(eye, obj, tex, cW, N, L, U0, 1);
            if (isInvisiblePixel(surf, Black, White, obj)) return 0;
            ShadeSurface(eye, obj, surf, 0, 1.0, 1.0, L, cW, N, C, nullptr);
            }
         else
               Shade(eye, obj, tex, 0, 1.0, 1.0, L, cW, N, U, C);

         if (RuntimeState::Global_Haze > 0.0 && RuntimeState::Global_Haze < 1.0 &&
             sz > RuntimeState::Global_Haze_Start) {
            c0 = pow(RuntimeState::Global_Haze, (sz - RuntimeState::Global_Haze_Start));
            c1 = 1.0 - c0;
            VecComb(c0, C, c1, RuntimeState::Global_Haze_Color, colour);
            }
         else {
            VecCopy(C, colour);
            }
         }
      }

   Vec color;
   VecCopy(colour,colour);
   assignNuVecToFVec(colour, color);//lets use Vec again for now
   OutputColour(eye, x, y, color, opacity);

   // Save the current depth
   ZBuffer_Write(eye, x, y, sz);
   return 1;
}

/* called at each pixel by poly_scan.  Returns 1 if the pixel was drawn,
   returns the distance to the pixel in depth. */
/** @brief Testable wrapper around pixelproc logic used under the TESTING build.
 *  @param eye       Active viewpoint.
 *  @param obj       Object owning the polygon.
 *  @param tex       Optional texture override.
 *  @param x         Pixel x coordinate.
 *  @param y         Pixel y coordinate.
 *  @param pt        Interpolated per-pixel vertex data.
 *  @param edge_flag Non-zero when the pixel lies on a polygon edge.
 *  @return 1 when the pixel would be emitted, 0 otherwise.
 */
#ifndef TESTING
static
#endif
int pixelproctesting(Viewpoint* eye, Object* obj, Texture* tex, int x, int y, Vertex* pt, int edge_flag)
{
    float sz;
    Flt opacity;
    Vec C, color;
    Flt q, c0, c1;
    Vec L, N, P, W, U;
    Surface* surf;
    static Vec White = { 1, 1, 1 }, Black = { 0, 0, 0 };

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

    
#ifdef SIMPLE_DEPTH
    VecCopy(pt->W, W);
    VecScale(q, W);
#endif

    /* Do CSG checks on this point */
    if ((obj->o_parent != nullptr) && !Inside_CSG_Node(obj->o_csg_tree, W))
        return 0; /* Not inside CSG */
  
    /* Check for dithering */
    if (obj->o_dither >= 0.0 && obj->o_dither < polyray_random())
        return 0; /* Dithered out of existence */
  
    if (runtimeState::settings.Render_Method == rmode::SCAN_CONVERSION) {
        VecCopy(pt->U, U);
        VecScale(q, U);
    }
    else if (runtimeState::settings.Render_Method == rmode::GOURAD_SHADE) {
        VecCopy(pt->P, P);
        VecScale(q, P);
    }
    
    if (runtimeState::settings.DepthRender) {
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
        else if (runtimeState::settings.Render_Method == rmode::WIRE_FRAME ||
            runtimeState::settings.Render_Method == rmode::HIDDEN_LINE) {
            /* Wireframe gets rendered in black and white */
#if defined( MAC )
            MakeVector(1, 1, 1, color)
#else
            MakeVector(0, 0, 0, color)
#endif
        }
        else if (runtimeState::settings.Render_Method == rmode::GOURAD_SHADE) {
            /* Gouraud shading keeps the color in the P variable */
            VecCopy(P, color);
        }
        else {
        //    std::cout << "ppt5\n" << std::flush;
#ifdef SIMPLE_DEPTH
            /* Get the direction from the eye to this point */
            VecSub(W, eye->view_from, L);
            sz = VecNormalize(L);
#endif
            /* unitize the normal vector */
            VecCopy(pt->N, N);
            if (VecNormalize(N) < PLY_EPSILON)
                serror("Bad normal: <%g, %g, %g> on triangle\n");

            MakeVector(0.0, 0.0, 0.0, color);

            if (obj->o_sflag & TRANSMIT_CHECK) {
                fVec U0;

                VecCopy(U, U0);
                surf = find_surface(eye, obj, tex, W, N, L, U0, 1);
                if (isInvisiblePixel(surf, Black, White, obj)) return 0;
                ShadeSurface(eye, obj, surf, 0, 1.0, 1.0, L, W, N, C, nullptr);
            }
            else
                Shade(eye, obj, tex, 0, 1.0, 1.0, L, W, N, U, C);

            if (RuntimeState::Global_Haze > 0.0 && RuntimeState::Global_Haze < 1.0 &&
                sz > RuntimeState::Global_Haze_Start) {
                c0 = pow(RuntimeState::Global_Haze, (sz - RuntimeState::Global_Haze_Start));
                c1 = 1.0 - c0;
                VecComb(c0, C, c1, RuntimeState::Global_Haze_Color, color);
            }
            else {
                VecCopy(C, color);
            }
        }
    }

    // Save the current depth 
    ZBuffer_Write(eye, x, y, sz);
    return 1;
}

/** @brief Emit a highlighted edge pixel for wireframe or hidden-line rendering.
 *  @param eye Active viewpoint.
 *  @param x   Pixel x coordinate.
 *  @param y   Pixel y coordinate.
 *  @return 1 when the edge pixel is inside the output window and emitted, 0 otherwise.
 */
#ifndef TESTING
static
#endif
int edgepixel(Viewpoint* eye, int x, int y)
{
   if (x < 0 || x > eye->view_xres-1 ||
       y < win.y0 || y > win.y1)
      return 0;
  Vec color;
#if defined( MAC )
   MakeVector(0, 0, 0, color)
#else
   MakeVector(1, 1, 1, color)
#endif

   /* Write the color to the output file */
   if (RuntimeState::File_Generation_Flag)
      Put_Pixel(eye, x, y, color, 1.0);

   /* If the display is active then draw the pixel on screen */
   if (Global::Display_Flag) display_plot(x, y, color);

   return 1;
}

/** @brief Precompute integer and incremental values for line rasterization.
 *  @param v1    First endpoint.
 *  @param v2    Second endpoint.
 *  @param x1    Output first x coordinate.
 *  @param x2    Output second x coordinate.
 *  @param y1    Output first y coordinate.
 *  @param y2    Output second y coordinate.
 *  @param dx    Output x delta.
 *  @param dy    Output y delta.
 *  @param sx    Output x step sign.
 *  @param sy    Output y step sign.
 *  @param dtdx  Output interpolation increment per x step.
 *  @param dtdy  Output interpolation increment per y step.
 *  @return No return value.
 */
#ifndef TESTING
static
#endif
void compute_line_values(Vertex* v1, Vertex* v2,
    int* x1, int* x2, int* y1, int* y2,
    int* dx, int* dy, int* sx, int* sy,
    float* dtdx, float* dtdy)
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


/** @brief Rasterize the outline of a polygon into edge pixels.
 *  @param eye Active viewpoint.
 *  @param obj Object owning the polygon.
 *  @param p   Polygon to rasterize.
 *  @return No return value.
 */
#ifndef TESTING
static
#endif
void edge_scan(Viewpoint* eye, Object* obj, Poly* p)
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
   for (i=0; i<=n; ++i) {
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
         pixelproc(eye, obj, nullptr, x, y, &pt, 1);

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
               pixelproc(eye, obj, nullptr, x, y, &pt, 1);
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
               pixelproc(eye, obj, nullptr, x, y, &pt, 1);
            }
         }
      }
}



/** @brief Testable wrapper around edge_scan used under the TESTING build.
 *  @param eye Active viewpoint.
 *  @param obj Object owning the polygon.
 *  @param p   Polygon to rasterize.
 *  @return No return value.
 */
#ifndef TESTING
static
#endif 
void edge_scantesting(Viewpoint* eye, Object* obj, Poly* p)
{
    int i, n, x, y, x1, y1, x2, y2;
    int ax, ay, sx, sy, dx, dy, d1;
    float dtdx, dtdy;
    Vertex l, r, dxp, dyp, pt;
    Vertex* lp, * rp, * tp;

    lp = &l;
    rp = &r;

    /* Set up first vertex */
    n = p->n - 1;
    copy_vertex(&(p->vertices[n]), rp);
    rp->S[1] -= 0.5;
    for (i = 0; i <= n; i++) {
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
            pixelproctesting(eye, obj, nullptr, x, y, &pt, 1);

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
                    pixelproc(eye, obj, nullptr, x, y, &pt, 1);
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
                    pixelproc(eye, obj, nullptr, x, y, &pt, 1);
            }
        }
    }
}


/** @brief Fill a convex polygon by scan-converting horizontal spans.
 *  @param eye Active viewpoint.
 *  @param obj Object owning the polygon.
 *  @param tex Optional texture override.
 *  @param p   Polygon to fill.
 *  @return No return value.
 */
#ifndef TESTING
static 
#endif
void poly_scan(Viewpoint *eye, Object *obj, Texture *tex, Poly *p)
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
   for (i=0; i<n; ++i) {
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
   for (i=0,w=xmax-xmin;i<=w;++i)
      eye->edgey[i] = ymax+2;
   for (i=0,w=ymax-ymin;i<=w;++i)
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
                  edge_flag = (runtimeState::settings.Render_Method == rmode::HIDDEN_LINE &&
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

   if (runtimeState::settings.Render_Method == rmode::HIDDEN_LINE) {
      lx = xmin; rx = xmax;
      for (xindex=0,x=lx;x<=rx;++xindex,++x)
         if (eye->edgey[xindex] <= ymax)
            edgepixel(eye, x, eye->edgey[xindex]);
      lx = ymin; rx = y-1;
      for (yindex=0,y=lx;y<=rx;++yindex,++y)
         if (eye->edgex[yindex] <= xmax)
            edgepixel(eye, eye->edgex[yindex], y);
      }
}


/** @brief Draw the display-only outline of a polygon.
 *  @param p Polygon to outline.
 *  @return No return value.
 */
#ifndef TESTING
static 
#endif
void poly_outline(Poly *p)
{
   int i, sx0, sx1, sy0, sy1;
   Vertex *v = p->vertices;
#if defined( MAC ) || defined( _WINDOWS )
   static Vec BLACK  = { 0.0, 0.0, 0.0 };
#else
   static Vec WHITE  = { 1.0, 1.0, 1.0 };
#endif

   if ((RuntimeState::Check_Abort_Flag == 1) && _kbhit())
      throw std::runtime_error("ABORT");

   sx0 = (Flt)v[p->n-1].S[0];
   sy0 = (Flt)v[p->n-1].S[1];
   for (i=0;i<p->n;++i) {
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
   Transform *tx = eye->WS.get();

   p.n = poly->n;
   for (i=0;i<poly->n;++i)
      p.vertices[i] = poly->vertices[i];

   /* transform vertices from world space to homogeneous screen space */
   for (i=0; i<p.n; ++i) {
      v = &p.vertices[i];
      v->w = v->W[0] * tx->matrix[0][3] +
             v->W[1] * tx->matrix[1][3] +
             v->W[2] * tx->matrix[2][3] +
                       tx->matrix[3][3];
      fTxVec(v->S, v->W, tx);
      }

   if ((r = poly_clip_to_box(&p, &pbox)) == POLY_CLIP_OUT) {
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

/** @brief Estimate the screen-space extent of a bounding box.
 *  @param eye   Active viewpoint.
 *  @param bbox  Bounding box in world space.
 *  @param x     Output width in screen pixels.
 *  @param y     Output height in screen pixels.
 *  @return No return value.
 */
void BboxScreenSize(Viewpoint *eye, bbox_info *bbox, int *x, int *y)
{
   int j, x0, x1, y0, y1;
   int sx, sy, maxx, maxy;
   fVec lower_left, lengths;
   fVec W, S;
   Flt w;
   Transform *tx = eye->WS.get();

   VecCopy(bbox->lower_left, lower_left);
   VecCopy(bbox->lengths, lengths);
   x0 = y0 =  PLY_HUGE;
   x1 = y1 = -PLY_HUGE;

   maxx = eye->view_xres;
   maxy = eye->view_yres;

   /* First see if the eye is either inside the bounding box or
      perhaps in front of it. */
   j = 0;
   for (int i=0;i<3;++i)
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
   for (int i=0;i<8;++i) {
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
   y1 = PLY_MIN(y1, win.y1);

   *x = x1 - x0;
   *y = y1 - y0;
}

/* W0 must be ok and W1 the one we want to move.  The resulting
   position of W1 is returned in Wres */
constexpr int MAX_CSG_SUBDIVISIONS = 10;

/** @brief Binary-search a CSG boundary point along a triangle edge.
 *  @param obj  Object whose CSG tree is queried.
 *  @param W0   Edge endpoint known to be inside.
 *  @param W1   Edge endpoint to move toward the boundary.
 *  @param Wres Output point approximating the boundary crossing.
 *  @return No return value.
 */
#ifndef TESTING
static
#endif
void csg_subdivide_loop(Object *obj, Vec W0, Vec W1, Vec Wres)
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

/** @brief Emit a single raw triangle to stdout.
 *  @param W0 First vertex in world space.
 *  @param W1 Second vertex in world space.
 *  @param W2 Third vertex in world space.
 *  @return No return value.
 */
#ifndef TESTING
static
#endif
void emit_raw_triangle(Vec W0, Vec W1, Vec W2)
{
   printf("%.4g %.4g %.4g ",  W0[0], W0[1], W0[2]);
   printf("%.4g %.4g %.4g ",  W1[0], W1[1], W1[2]);
   printf("%.4g %.4g %.4g\n", W2[0], W2[1], W2[2]);
}

/* Use binary subdivision to more closely approximate where a triangle
   enters and exits CSG.  This function returns 1 if there was any
   adjustment needed to the triangle legs. */
/** @brief Slice a triangle against CSG membership using per-vertex inside flags.
 *  @param obj    Object whose CSG tree is queried.
 *  @param W0     First triangle vertex.
 *  @param W1     Second triangle vertex.
 *  @param W2     Third triangle vertex.
 *  @param flag0  Inside flag for @p W0.
 *  @param flag1  Inside flag for @p W1.
 *  @param flag2  Inside flag for @p W2.
 *  @param depth  Current subdivision depth.
 *  @return 1 when output triangles are emitted, 0 when higher-level subdivision should continue.
 */
#ifndef TESTING
static
#endif
short slice_csg_triangle(Object *obj, Vec W0, Vec W1, Vec W2,
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
      if (!flag2) {
         csg_subdivide_loop(obj, W1, W0, tW0);
         csg_subdivide_loop(obj, W1, W2, tW2);
         emit_raw_triangle(tW0, W1, tW2);
         return 1;
      }
      
      csg_subdivide_loop(obj, W1, W0, tW0);
      csg_subdivide_loop(obj, W2, W0, tW1);
      emit_raw_triangle(W1, tW0, tW1);
      emit_raw_triangle(W1, tW1, W2);
      return 1;
      
   }//if(!flag0)
   if (!flag1) {
      if (!flag2) {
         csg_subdivide_loop(obj, W0, W1, tW1);
         csg_subdivide_loop(obj, W0, W2, tW2);
         emit_raw_triangle(W0, tW1, tW2);
         return 1;
      }
      
      csg_subdivide_loop(obj, W0, W1, tW0);
      csg_subdivide_loop(obj, W2, W1, tW1);
      emit_raw_triangle(W0, tW0, tW1);
      emit_raw_triangle(W0, tW1, W2);
      return 1;
      
   }//if(!flag1)
   if (!flag2) {
      csg_subdivide_loop(obj, W0, W2, tW0);
      csg_subdivide_loop(obj, W1, W2, tW1);
      emit_raw_triangle(W0, W1, tW1);
      emit_raw_triangle(W0, tW1, tW0);
      return 1;
      }
   if (depth == 0) {
      emit_raw_triangle(W0, W1, W2);
      return 1;
      }   
   return 0;
}


/** @brief Check whether any triangle edge exceeds the configured CSG subdivision tolerance.
 *  @param W0         First triangle vertex.
 *  @param W1         Second triangle vertex.
 *  @param W2         Third triangle vertex.
 *  @param tolerance  Maximum allowed component-wise edge delta.
 *  @return Non-zero when further subdivision is required, zero otherwise.
 */
#ifndef TESTING
static
#endif
short check_leg_lengths(Vec W0, Vec W1, Vec W2, Flt tolerance)
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
/** @brief Recursively subdivide a triangle before raw CSG emission.
 *  @param obj   Object whose CSG tree is queried.
 *  @param W0    First triangle vertex.
 *  @param W1    Second triangle vertex.
 *  @param W2    Third triangle vertex.
 *  @param depth Current subdivision depth.
 *  @return Non-zero when this branch emitted output, zero otherwise.
 */
#ifndef TESTING
static
#endif
short subdiv_triangle(Object *obj, Vec W0, Vec W1, Vec W2, int depth)
{
   short f0, f1, f2, f3;
   Vec mW0, mW1, mW2;

   /* Check to see if this triangle is entirely within CSG */
   f0 = Inside_CSG_Node(obj->o_csg_tree, W0);
   f1 = Inside_CSG_Node(obj->o_csg_tree, W1);
   f2 = Inside_CSG_Node(obj->o_csg_tree, W2);

   if (!check_leg_lengths(W0, W1, W2, RuntimeState::settings.csg_leg_tolerance) ||
       depth > RuntimeState::settings.csg_subdivision_depth) {
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

/** @brief Emit a convex polygon as raw triangle output.
 *  @param obj Object owning the polygon.
 *  @param p   Polygon to triangulate and emit.
 *  @return No return value.
 */
#ifndef TESTING
static
#endif
void poly_raw_output(Object *obj, Poly *p)
{
   int i, j;
   Vertex *v[3];
   Vec W0, W1, W2;

   if ((RuntimeState::Check_Abort_Flag == 1) && _kbhit())
      throw std::runtime_error("ABORT");

   v[0] = &p->vertices[0];
   for (i=1;i<p->n-1;++i) {
      v[1] = &p->vertices[i];
      v[2] = &p->vertices[i+1];
      if (runtimeState::settings.Render_Method == rmode::CSG_TRIANGLES && obj->o_parent != nullptr) {
         VecCopy(v[0]->W, W0)
         VecCopy(v[1]->W, W1)
         VecCopy(v[2]->W, W2)
         subdiv_triangle(obj, W0, W1, W2, 0);
         }
      else {
         for (j=0;j<3;++j)
            printf("%.4g %.4g %.4g ", v[j]->W[0], v[j]->W[1], v[j]->W[2]);
         if (runtimeState::settings.Render_Method == rmode::UV_TRIANGLES) {
            for (j=0;j<3;++j)
               printf("%.4g %.4g %.4g ", v[j]->N[0], v[j]->N[1], v[j]->N[2]);
            for (j=0;j<3;++j)
               printf("%.4g %.4g ", v[j]->U[0], v[j]->U[1]);
            }
         printf("\n");
         }
      }
}


/** @brief Convert a polygon into mesh triangles appended to a BinTree.
 *  @param Root Output BinTree receiving generated triangle objects.
 *  @param obj  Parent object receiving expanded vertex arrays.
 *  @param p    Polygon to split into triangles.
 *  @return No return value.
 */
#ifndef TESTING
static
#endif
void poly_obj_output(BinTree& Root, Object* obj, Poly* p)
{
   TriangleObject *tri_obj;
   fVec *verts, *norms, *uvals;
   Vec P1, P2, N;
   int u_axis, v_axis;
   int new_n, old_n, out_n;
   bbox_info box;
   
   if (obj->o_vertices != nullptr) {
       Ensures(obj->o_vertices->N != nullptr);
       Ensures(obj->o_vertices->U != nullptr);
       Ensures(obj->o_vertices->V != nullptr);
   }
   new_n = p->n;// p has valid vertices 0 to new_n-1
   old_n = (obj->o_vertices == nullptr ? 0 : obj->o_vertices->n);
   verts = (fVec *)polyray_malloc((new_n + old_n) * sizeof(fVec));
   norms = (fVec *)polyray_malloc((new_n + old_n) * sizeof(fVec));
   uvals = (fVec *)polyray_malloc((new_n + old_n) * sizeof(fVec));
   if (verts == nullptr || norms == nullptr || uvals == nullptr)
      serror("Insufficient polygon memory");

   if (obj->o_vertices == nullptr) {
      obj->o_vertices = (ObjectVertices *)
                        polyray_malloc(sizeof(ObjectVertices));
      obj->o_vertices->n = 0;
      obj->o_vertices->N = nullptr;
      obj->o_vertices->U = nullptr;
      obj->o_vertices->V = nullptr;
      }
   else {
      /* Copy any old vertices into the new list */
      for (int i=0;i<old_n;++i) {
         VecCopy(obj->o_vertices->V[i], verts[i]);
         VecCopy(obj->o_vertices->N[i], norms[i])
         VecCopy(obj->o_vertices->U[i], uvals[i])
      }
      polyray_free(obj->o_vertices->V);
      if (obj->o_vertices->N != nullptr)
         polyray_free(obj->o_vertices->N);
      if (obj->o_vertices->U != nullptr)
         polyray_free(obj->o_vertices->U);
   }

   // Copy the new vertices into the list 
   for (int i=old_n;i<old_n+new_n;++i) {
       if (i-old_n > new_n - 1) serror("out of range access p\n");
      VecCopy(p->vertices[i-old_n].W, verts[i]);
      VecCopy(p->vertices[i-old_n].N, norms[i]);
      VecCopy(p->vertices[i-old_n].U, uvals[i]);
      }
   obj->o_vertices->n += new_n;//updated no of vertices
   obj->o_vertices->V = verts;
   obj->o_vertices->N = norms;
   obj->o_vertices->U = uvals;

   // Allocate space to hold the intermediate polygon stacks
   std::vector<std::array<int,3>> out_storage(new_n - 2);
   std::vector<int*> out_verts(new_n - 2);
   for (int i=0;i<new_n-2;i++)
      out_verts[i] = out_storage[i].data();

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
                 out_n, out_verts.data());

   /* Now add the triangles to the list of objects */
   for (int i=0;i<out_n;++i) {
      tri_obj = FactoryTriangleObject();
      tri_obj->o_type = ShapeType::Polygon;
      tri_obj->o_parent = obj;
      tri_obj->o_vert[0] = out_verts[i][0] + old_n;
      tri_obj->o_vert[1] = out_verts[i][1] + old_n;
      tri_obj->o_vert[2] = out_verts[i][2] + old_n;
      tri_obj->o_texture = obj->o_texture;
      //tri_obj->o_trans = nullptr;
      if (calc_triangle_bounds(*tri_obj, &box)) {
         /* Now add this triangle object to the root */
         VecCopy(box.lower_left, tri_obj->o_bnd.lower_left);
         VecCopy(box.lengths, tri_obj->o_bnd.lengths);
         Root.members.list = push_object(Root.members.list,
                                          (Object *)tri_obj);
#ifdef DEBUG_FN_CALLS
         printf("tri_obj->o_bnd.lower_left = %f %f %f\n",
				tri_obj->o_bnd.lower_left[0],
				tri_obj->o_bnd.lower_left[1],
				tri_obj->o_bnd.lower_left[2]);
             
#endif

         Root.members.count++;
         }
      else
         delete tri_obj;
      }
}

/** @brief Convert a polygon into the active render or export representation.
 *  @param eye  Active viewpoint; required for screen-space rendering.
 *  @param Root Scene or mesh output tree.
 *  @param obj  Object owning the polygon.
 *  @param tex  Optional texture override.
 *  @param p    Polygon to process.
 *  @return No return value.
 */
void scan_convert(Viewpoint *eye, BinTree *Root, Object *obj, Texture *tex, Poly *p)
{   
   Vertex *v;
   Flt w;
   Vec L, C, W, N, U;
   Transform *tx;
   #ifdef DEBUG_FN_CALLS
   smessage("scan::scan_convert\n");
   #endif
   if (runtimeState::settings.Render_Method == rmode::RAW_TRIANGLES ||
       runtimeState::settings.Render_Method == rmode::CSG_TRIANGLES ||
       runtimeState::settings.Render_Method == rmode::UV_TRIANGLES) {
      poly_raw_output(obj, p);
      return;
   }
   if (runtimeState::settings.Render_Method == rmode::MESH_CONVERSION) {
      poly_obj_output(*Root, obj, p);
      return;
   }
   if (eye == nullptr)
      /* No eye transformation - this should never happen */
      serror("No perspective transformation for scan conversion");

   tx = eye->WS.get();

   /* transform vertices from world space to homogeneous screen space */
   for (int i=0; i<p->n; ++i) {
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
   auto r = poly_clip_to_box(p, &pbox);
   if (r == PolyClip::PolyClipOut)
      return;

   /* do homogeneous division of screen position, object position */
   for (int i=0; i<p->n; ++i) {
      v = &p->vertices[i];
      w = 1.0 / v->w;
      v->w = w;
      VecScale(w, v->S);
      if (runtimeState::settings.Render_Method == rmode::GOURAD_SHADE) {
         /* Put the color into P */
         VecSub(v->W, eye->view_from, L);
         VecNormalize(L);
         VecCopy(v->W, W);
         VecCopy(v->N, N);
         VecCopy(v->U, U);
         Shade(eye, obj, nullptr, 0, 1.0, 1.0, L, W, N, U, C);
         VecCopy(C, v->P);
         VecScale(w, v->P);
         }
      else if (runtimeState::settings.Render_Method == rmode::SCAN_CONVERSION) {
         VecScale(w, v->P);
         VecScale(w, v->U);
         }
      VecScale(w, v->W);
      }

   if (runtimeState::settings.Render_Method == rmode::WIRE_FRAME && !RuntimeState::File_Generation_Flag)
      poly_outline(p);
   else if (runtimeState::settings.Render_Method == rmode::WIRE_FRAME)
      edge_scan(eye, obj, p);
   else /* SCAN_CONVERSION, GOURAD_SHADE, HIDDEN_LINE */
      poly_scan(eye, obj, tex, p);
}


/** @brief Render or further dispatch a primitive object according to its type.
 *  @param eye  Active viewpoint.
 *  @param Root Scene acceleration tree or mesh output tree.
 *  @param pobj Parent object context for nested triangle rendering.
 *  @param obj  Primitive object to render.
 *  @return No return value.
 */
void render_prim(Viewpoint *eye, BinTree *Root, Object *pobj, Object *obj)
{
   #ifdef DEBUG_FN_CALLS
   smessage("scan::render_prim\n");
   #endif
   CompositeObject *cobj;
   TriangleObject *tobj;
   fVec *V, *N, *U, fB0, B1, N0;// B0 conflicts on macos
   float d;
   Poly P;
   Poly *Polygon = &P;
   N0[0]=N0[1]=N0[2]=0.0;//cm initing N0

   switch (obj->o_type) {
      case ShapeType::Csg:
      case ShapeType::Bezier:
      case ShapeType::Blob:
      case ShapeType::Box:
      case ShapeType::Cone:
      case ShapeType::Cylinder:
      case ShapeType::Cyl_Height_Field:
      case ShapeType::Disc:
      case ShapeType::Function:
      case ShapeType::Glyph:
      case ShapeType::Gridded:
      case ShapeType::Height_Field:
      case ShapeType::Hypertexture:
      case ShapeType::Nurb:
      case ShapeType::Parabola:
      case ShapeType::Parametric:
      case ShapeType::Poly:
      case ShapeType::Polynomial:
      case ShapeType::Raw_Triangles:
      case ShapeType::Revolve:
      case ShapeType::Sphere:
      case ShapeType::Sph_Height_Field:
      case ShapeType::SuperQ:
      case ShapeType::Sweep:
      case ShapeType::Torus:
      case ShapeType::Tri:
         obj->o_procs->render(eye, Root, obj);
         break;
      case ShapeType::Composite:
         cobj = (CompositeObject *)obj;
         for (int i=0;i<cobj->c_size;++i)
            render_prim(eye, Root, pobj, cobj->c_object[i]);//std::vector
            //render_prim(eye, Root, pobj, cobj->c_object[i]);//Object*
         break;
      case ShapeType::Polygon:
         tobj = (TriangleObject *)obj;
         if (pobj == nullptr || pobj->o_type != ShapeType::Raw_Triangles)
            /* Triangles created by MESH_CONVERSION have their parents
               set correctly and don't need it forced. Triangles read
               in from a raw file and used in a define statement will
               have the parent set to the defined object rather than
               the instantiated object. */
            pobj = tobj->o_parent;
         //ShapeType i = pobj->o_type;
         V = pobj->o_vertices->V;
         U = pobj->o_vertices->U;
         N = pobj->o_vertices->N;
         Polygon->n = 3;
         if (N == nullptr) {
            /* Calculate the normal to the triangle */
            VecSub(V[tobj->o_vert[1]], V[tobj->o_vert[0]], fB0);
            VecSub(V[tobj->o_vert[2]], V[tobj->o_vert[0]], B1);
            VecCross(fB0, B1, N0);
            d = VecDot(N0, N0);
            d = (d < PLY_EPSILON ? 1.0 : 1.0 / d);
            VecScale(d, N0);
            }
         for (int i=0;i<3;++i) {
            VecCopy(V[tobj->o_vert[i]], Polygon->vertices[i].W);
            VecCopy(V[tobj->o_vert[i]], Polygon->vertices[i].P);
            if (U == nullptr)
               VecCopy(V[tobj->o_vert[i]], Polygon->vertices[i].U)
            else
               VecCopy(U[tobj->o_vert[i]], Polygon->vertices[i].U)
            if (N == nullptr)
               VecCopy(N0, Polygon->vertices[i].N)
            else
               VecCopy(N[tobj->o_vert[i]], Polygon->vertices[i].N)
            }

         if (pobj->o_type == ShapeType::Raw_Triangles &&
             pobj->o_trans != nullptr)
            for (int i=0;i<3;++i) {
               fTxVec(Polygon->vertices[i].W, Polygon->vertices[i].P,
                      pobj->o_trans);
               auto ret = fTxNormal(Polygon->vertices[i].N,
                         pobj->o_trans);
               //Polygon->vertices[i].N = ret;
               assignNuVecToFVec(ret, Polygon->vertices[i].N);
               fVecNormalize(Polygon->vertices[i].N);
               }

         scan_convert(eye, Root, pobj, tobj->o_texture, Polygon);
         break;
      default:
         break;
      }
}

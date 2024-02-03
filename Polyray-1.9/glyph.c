/* glyph.c

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
#include "intersec.h"
#include "symtab.h"
#include "scan.h"
#include "subdiv.h"
#include "vector.h"
#include "glyph.h"
#include "roots.h"

/* Structure to hold points in a contour */
typedef struct cpoints_struct *cpointPtr;
struct cpoints_struct {
   float x, y;
   cpointPtr next, last;
   };

typedef struct t_glyphdata {
   int count;         /* Number of contours */
   Contour *contours; /* Actual contours */
   } GlyphData;

void GlyphRender(Viewpoint *, BinTree *, Object *);
int GlyphIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int GlyphInside(Object *obj, Vec P);
void GlyphDelete(Object *object);

#if 0
static Vec Red = {1, 0, 0};

static void
tx_point(Object *obj, Vec P, Vec W, Vec S)
{
   Flt w;
   Transform *tx = Eye.WS;
   Vec N;

   TxVector(W, P, obj->o_trans)
   w = W[0] * tx->matrix[0][3] +
       W[1] * tx->matrix[1][3] +
       W[2] * tx->matrix[2][3] +
              tx->matrix[3][3];
   TxVec(S, W, tx);
   w = 1.0 / w;
   VecScale(w, S);
}

static void
draw_contour(Object *obj, cpointPtr contour, Vec color)
{
   int sx0, sx1, sy0, sy1;
   Vec P, W, S;
   cpointPtr cpoint;
   extern void display_line(int, int, int, int, Vec);

   MakeVector(contour->x, contour->y, 0, P);
   tx_point(obj, P, W, S);
   sx0 = (Flt)S[0];
   sy0 = (Flt)S[1];
   for (cpoint = contour->next;
        cpoint != contour;
        cpoint = cpoint->next) {
   if ((Check_Abort_Flag != 0) && _kbhit())
      longjmp(abort_environ, 1);

      MakeVector(cpoint->x, cpoint->y, 0, P);
      tx_point(obj, P, W, S);
      sx1 = (Flt)S[0];
      sy1 = (Flt)S[1];
      display_line(sx0, sy0, sx1, sy1, color);
      sx0 = sx1;
      sy0 = sy1;
      }
   MakeVector(cpoint->x, cpoint->y, 0, P);
   tx_point(obj, P, W, S);
   sx1 = (Flt)S[0];
   sy1 = (Flt)S[1];
   display_line(sx0, sy0, sx1, sy1, color);
}
#endif

ObjectProcs GlyphProcs = {
   GlyphRender,
   NULL,
   GenericInitialize,
   GlyphIntersect,
   GlyphInside,
   GenericCopy,
   GlyphDelete,
   };

/* Split a contour into triangles. */
static void
Split_Contour(Viewpoint *eye, BinTree *Root, Object *obj,
              int count, cpointPtr *contour_points, int *flags)
{
   fVec *verts;
   int **out_verts;
   Vertex *vertex;
   int i, j, k;
   int out_n, npoints;
   cpointPtr cpoint0;
   Poly Polygon;
   Vec P, N;
   jmp_buf temp_environ;
   int internal_abort;

   /* Can't split a NULL vertex list */
   if (contour_points == NULL)
      return;

   memcpy(temp_environ, abort_environ, sizeof(jmp_buf));

   for (i=0;i<count;i++) {
      npoints = (int)fabs(flags[i]);

      /* Allocate space to hold the intermediate polygon stacks */
      verts = (fVec *)polyray_malloc(npoints * sizeof(fVec));
      out_verts = (int **)polyray_malloc((npoints - 2) * sizeof(int *));
      if (verts == NULL || out_verts == NULL)
         error("Insufficient memory for glyph polygons");
      for (j=0;j<npoints-2;j++) {
         out_verts[j] = (int *)polyray_malloc(3 * sizeof(int));
         if (out_verts[j] == NULL)
            error("Insufficient memory for glyph polygons");
         }

      cpoint0 = contour_points[i];
      for (j=0;j<npoints;j++,cpoint0=cpoint0->next)
         MakeVector(cpoint0->x, cpoint0->y, 0.0, verts[j])

      Split_Polygon(npoints, verts, 0, 1, &out_n, out_verts);

      if (setjmp(abort_environ) == 0) {
         /* Now output the triangles that we generated */
         for (j=0;j<out_n;j++) {
            /* Bottom layer */
            Polygon.n = 3;
            for (k=0;k<3;k++) {
               vertex = &Polygon.vertices[k];
               VecCopy(verts[out_verts[j][k]], P);
               P[2] = 0;
               MakeVector(0, 0, -1, N);
               VecCopy(P, vertex->P);
               VecCopy(P, vertex->U);
               if (obj->o_trans) {
                  TxVector(P, P, obj->o_trans);
                  TxNormal(N, N, obj->o_trans);
                  }
               VecCopy(P, vertex->W);
               VecCopy(N, vertex->N);
               }
            scan_convert(eye, Root, obj, NULL, &Polygon);

            /* Top layer */
            Polygon.n = 3;
            for (k=0;k<3;k++) {
               vertex = &Polygon.vertices[k];
               VecCopy(verts[out_verts[j][k]], P);
               P[2] = 1.0;
               MakeVector(0, 0, 1, N);
               VecCopy(P, vertex->P);
               VecCopy(P, vertex->U);
               if (obj->o_trans) {
                  TxVector(P, P, obj->o_trans);
                  TxNormal(N, N, obj->o_trans);
                  }
               VecCopy(P, vertex->W);
               VecCopy(N, vertex->N);
               }
            scan_convert(eye, Root, obj, NULL, &Polygon);
            }
         internal_abort = 0;
         }
      else
         internal_abort = 1;

#if 0
draw_contour(obj, *contour_points, Red);
#endif
      /* Free the temporary polygon storage */
      for (j=0;j<npoints-2;j++)
         polyray_free(out_verts[j]);
      polyray_free(out_verts);
      polyray_free(verts);

      if (internal_abort) {
         memcpy(abort_environ, temp_environ, sizeof(jmp_buf));
         longjmp(abort_environ, 1);
         }
      }
   memcpy(abort_environ, temp_environ, sizeof(jmp_buf));
}

static int
Point_Segment_Test(Flt x, Flt y, Flt x0, Flt y0, Flt x1, Flt y1)
{
   int ri, qi, rj, qj;
   Flt m, b;

   qi = ri = qj = rj = 0;
   if (y0 == y1)
      return 0;
   if (y0 < y) qi = 1;
   if (y1 < y) qj = 1;
   if (qi == qj) return 0;
   if (x0 > x) ri = 1;
   if (x1 > x) rj = 1;
   if (ri & rj) return 1;
   if ((ri | rj) == 0) return 0;
   m = (y1 - y0) / (x1 - x0);
   b = (y1 - y) - m * (x1 - x);
   if ((-b/m) >= 0.0) return 1;
   return 0;
}

/* Test to see if "point" is inside the splined polygon "points". */
static int
Inside_Glyph(GlyphData *glyph, Flt x, Flt y)
{
   Contour *contours;
   int i, crossings;

   crossings = 0;
   for (i=0,contours=glyph->contours;i<glyph->count;i++,contours++)
      crossings += Inside_Contour(x, y, 3, contours->count+1,
                                  contours->points);

   return (crossings & 1);
}

/* Returns the distance to z = 0 in t0, and the distance to z = 1 in
   t1.  These distances are to the the bottom and top surfaces of the glyph.
   The distances are set to -1 if there is no hit */
static void
GetZeroOneHits(GlyphData *glyph, Vec P, Vec D, Flt *t0, Flt *t1)
{
   Flt x0, y0, t;

   *t0 = -1.0;
   *t1 = -1.0;

   /* Are we parallel to the x-y plane? */
   if (fabs(D[2]) < EPSILON)
      return;

   /* Solve: P[2] + t * D[2] = 0 */
   t = -P[2] / D[2];
   x0 = P[0] + t * D[0];
   y0 = P[1] + t * D[1];
   if (Inside_Glyph(glyph, x0, y0))
      *t0 = t;

   /* Solve: P[2] + t * D[2] = 1 */
   t += (1.0 / D[2]);
   x0 = P[0] + t * D[0];
   y0 = P[1] + t * D[1];
   if (Inside_Glyph(glyph, x0, y0))
      *t1 = t;
}

/*
Solving for a linear glyph of a non-linear curve can be performed by
projecting the ray onto the x-y plane, giving a parametric equation
for the ray as:

   x = x0 + x1 t, y = y0 + y1 t

Eliminating t from the above gives the implicit equation:

   y1 x - x1 y - (x0 y1 - y0 x1) = 0.

Substituting a parametric equation for x and y gives:

   y1 x(s) - x1 y(s) - (x0 y1 - y0 x1) = 0.

which can be written as

   a x(s) + b y(s) + c = 0,

where a = y1, b = -x1, c = (y0 x1 - x0 y1).

For piecewise quadratics, the parametric equations will have
the forms:

   x(s) = (1-s)^2 P0(x) + 2 s (1 - s) P1(x) + s^2 P2(x)
   y(s) = (1-s)^2 P0(y) + 2 s (1 - s) P1(y) + s^2 P2(y)

where P0 is the first defining vertex of the spline, P1 is the second,
P2 is the third.  Using the substitutions:

   xt2 = x0 - 2 x1 + x2, xt1 = 2 * (x1 - x0), xt0 = x0;
   yt2 = y0 - 2 y1 + y2, yt1 = 2 * (y1 - y0), yt0 = y0;

the equations can be written as:

   x(s) = xt2 s^2 + xt1 s + xt0,
   y(s) = yt2 s^2 + yt1 s + yt0.

Substituting and multiplying out gives the following equation in s:

   s^2 * (a*xt2 + b*yt2) +
   s   * (a*xt1 + b*yt1) + 
         c + a*xt0 + b*yt0

This is then solved using the quadratic formula.  Any solutions
of s that are between 0 and 1 (inclusive) are valid solutions.
*/ 
int
GlyphIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
               Flt mindist, Flt maxdist, Isect *hit)
{
   Contour *contours;
   int i, j, k, l, npoints;
   Flt t, t0, t1, z;
   Flt a, b, c, C[3], S[2];
   Vec P, PP, D, N;
   Flt x0, x1, x2, y0, y1, y2;
   Flt xt0, xt1, xt2, yt0, yt1, yt2;
   Flt d0, d1;
   int Flag = 0, dirflag = 0;
   GlyphData *glyph = (GlyphData *)obj->o_data;

   /* Now transform to canonical glyph space */
   VecCopy(ray->P, P);
   VecCopy(ray->D, D);

   GetZeroOneHits(glyph, P, D, &t0, &t1);
   if (t0 > mindist && t0 <= maxdist) {
      VecAddScaled(P, t0, D, PP);
      MakeVector(0, 0, -1, N);
      Insert_Hit(obj, PP, N, t0, PP, hit);
      Flag = 1;
      }
   if (t1 > mindist && t1 <= maxdist) {
      VecAddScaled(P, t1, D, PP);
      MakeVector(0, 0, 1, N);
      Insert_Hit(obj, PP, N, t1, PP, hit);
      Flag = 1;
      }

   if (fabs(D[0]) < EPSILON)
      if (fabs(D[1]) < EPSILON)
         /* This means the ray is moving parallel to the walls of the glyph */
         return Flag;
      else
         dirflag = 0;
   else
      dirflag = 1;

   a = D[1];
   b = -D[0];
   c = (P[1] * D[0] - P[0] * D[1]);

   for (i=0,contours=glyph->contours;i<glyph->count;i++,contours++) {
      x0 = contours->points[0][0];
      y0 = contours->points[0][1];
      npoints = contours->count;
      for (j=1;j<=npoints;j++) {
         x1 = contours->points[j][0];
         y1 = contours->points[j][1];
         if (contours->points[j][2] == 0.0) {
            /* Linear glyph */
            d0 = (x1 - x0);
            d1 = (y1 - y0);
            t0 = d1 * D[0] - d0 * D[1];
            if (fabs(t0) < EPSILON)
               /* No possible intersection */
               goto end_line_test;
            t = (D[0] * (P[1] - y0) - D[1] * (P[0] - x0)) / t0;
            if (t < 0.0 || t > 1.0)
               goto end_line_test;
            if (dirflag)
               t = ((x0 + t * d0) - P[0]) / D[0];
            else
               t = ((y0 + t * d1) - P[1]) / D[1];
            z  = P[2] + t * D[2];

            if (z >= 0.0 && z <= 1.0 && t > mindist && t <= maxdist) {
               VecAddScaled(P, t, D, PP);
               MakeVector(d1, -d0, 0.0, N);
               Insert_Hit(obj, PP, N, t, PP, hit);
               Flag = 1;
               }
end_line_test:
            x0 = x1;
            y0 = y1;
            }
         else {
            /* Quadratic glyph of some sort (no fear of having j walk past the
               end of "points", cause we allocated an extra slot at the end
               and made sure it's on the contour) */
            x2 = contours->points[j+1][0];
            y2 = contours->points[j+1][1];
            if (contours->points[j+1][2] != 0.0) {
               /* Parabola with far end floating - readjust the far end
                  so that it is on the curve.  (In the correct place too.) */
               x2 = 0.5 * (x1 + x2);
               y2 = 0.5 * (y1 + y2);
               }

            /* Make the interpolating quadrics */
            xt2 = x0 - 2.0 * x1 + x2;
            xt1 = 2.0 * (x1 - x0);
            xt0 = x0;
            yt2 = y0 - 2.0 * y1 + y2;
            yt1 = 2.0 * (y1 - y0);
            yt0 = y0;

            C[0] = a * xt2 + b * yt2;
            C[1] = a * xt1 + b * yt1;
            C[2] = a * xt0 + b * yt0 + c;

            k = solve_quadratic(C, S, 0.0, 1.0);

            for (l=0;l<k;l++) {
               if (dirflag) {
                  t0 = S[l] * S[l] * xt2 + S[l] * xt1 + xt0;
                  t = (t0 - P[0]) / D[0];
                  }
               else {
                  t0 = S[l] * S[l] * yt2 + S[l] * yt1 + yt0;
                  t = (t0 - P[1]) / D[1];
                  }
               if (t > mindist && t <= maxdist) {
                  VecAddScaled(P, t, D, PP);
                  if (PP[2] >= 0.0 && PP[2] <= 1.0) {
                     MakeVector(2.0 * S[l] * yt2 + yt1, -2.0 * S[l] * xt2 - xt1,
                                0.0, N);
                     Insert_Hit(obj, PP, N, t, PP, hit);
                     Flag = 1;
                     }
                  }
               }

            x0 = x2;
            y0 = y2;
            }
         }
      }

   return Flag;
}

int
GlyphInside(Object *obj, Vec Pos)
{
   Vec P;
   GlyphData *glyph = (GlyphData *)obj->o_data;

   /* Transform the ray into the lathes space */
   InvTxVector1(P, Pos, obj->o_trans)

   /* See if the point is above, below, or inside the glyph */
   if ((P[2] > -EPSILON) && (P[2] < 1.0+EPSILON) &&
       Inside_Glyph(glyph, P[0], P[1]))
      return 1;
   else
      return 0;
}

Object *
MakeGlyph(Object *object, int count, Contour *contours)
{
   int i, j, k;
   int npoints;
   Flt t;
   fVec *points;
   Vec mins, maxs;
   GlyphData *glyph;

   object->o_type  = T_GLYPH;
   object->o_procs = &GlyphProcs;
   object->o_uv_steps[0] = 8;
   object->o_uv_steps[1] = 1;

   /* Attempt to allocate memory for this primitive */
   if ((glyph = (GlyphData *)polyray_malloc(sizeof(GlyphData))) == NULL)
      error("Failed to allocate glyph data\n");
   glyph->count = count;
   glyph->contours = contours;

   for (i=0;i<count;i++) {
      npoints = contours[i].count;
      points = contours[i].points;

      /* Copy the first point of the contour into the last position
         (assumes that the parser allocated the extra slot) */
      points[0][2] = 0.0; /* Force first point of contour onto the curve */
      VecCopy(points[0], points[npoints]);

      /* Compute bounding information */
      for (j=0;j<npoints;j++)
         if (i == 0 && j == 0) {
            VecCopy(points[0], mins);
            VecCopy(mins, maxs);
            }
         else
            for (k=0;k<2;k++) {
               t = points[j][k];
               if (t < mins[k]) mins[k] = t;
               if (t > maxs[k]) maxs[k] = t;
               }
      }
   mins[2] = 0.0;
   maxs[2] = 1.0;
   VecCopy(mins, object->o_bnd.lower_left);
   VecSub(maxs, mins, object->o_bnd.lengths);
   object->o_data = (void *)glyph;
   return object;
}

void
GlyphDelete(Object *object)
{
   int i;
   GlyphData *glyph = (GlyphData *)object->o_data;
   if (object->o_copy == 0) {
      for (i=0;i<glyph->count;i++)
         polyray_free(glyph->contours[i].points);
      polyray_free(glyph->contours);
      polyray_free(object->o_data);
      }
}

static void
Linear_Contour_Evaluater(Object *obj, Flt x0, Flt x1, Flt y0, Flt y1,
                         Flt u, Flt v, Vertex *vert)
{
   Flt x, y;
   Vec P, N;

   /* Calculate position and normal information */
   x = x0 + u * (x1 - x0);
   y = y0 + u * (y1 - y0);
   MakeVector(x, y, v, P);
   MakeVector(y1 - y0, x0 - x1, 0.0, N);

   /* Transform into world space */
   VecCopy(P, vert->P);
   if (obj->o_trans) {
      TxVector(P, P, obj->o_trans);
      TxNormal(N, N, obj->o_trans);
      }
   VecNormalize(N);
   VecCopy(P, vert->W);
   VecCopy(N, vert->N);
}

static void
Quadratic_Contour_Evaluater(Object *obj, Flt xt0, Flt xt1, Flt xt2,
                            Flt yt0, Flt yt1, Flt yt2,
                            Flt u, Flt v, Vertex *vert)
{
   Vec P, N;
   Flt x, y, dx, dy;

   /* Calculate position and normal information */
   x = xt2 * u * u + xt1 * u + xt0;
   y = yt2 * u * u + yt1 * u + yt0;

   dx = 2.0 * xt2 * u + xt1;
   dy = 2.0 * yt2 * u + yt1;

   MakeVector(x, y, v, P);
   MakeVector(dy,-dx, 0.0, N);

   VecCopy(P, vert->P);
   if (obj->o_trans) {
      TxVector(P, P, obj->o_trans);
      TxNormal(N, N, obj->o_trans);
      }
   VecNormalize(N);
   VecCopy(P, vert->W);
   VecCopy(N, vert->N);
}

static cpointPtr
alloc_cpoint(float x, float y)
{
   cpointPtr result = polyray_malloc(sizeof(struct cpoints_struct));
   if (result == NULL)
      error("Failed to allocate contour information");
   result->x = x;
   result->y = y;
   result->next = NULL;
   result->last = NULL;
   return result;
}

/* Push a new point into the ring of points on this contour */
static void
Push_Point(cpointPtr *contour_points, int i, Flt x, Flt y)
{
   cpointPtr cpoint, temp_point1, temp_point2;

   temp_point1 = contour_points[i];
   cpoint = alloc_cpoint(x, y);
   if (temp_point1 == NULL) {
      /* List totally empty */
      cpoint->next = NULL;
      cpoint->last = NULL;
      }
   else if (temp_point1->last == NULL) {
      /* Only one thing on the list */
      temp_point1->last = cpoint;
      temp_point1->next = cpoint;
      cpoint->next = temp_point1;
      cpoint->last = temp_point1;
      }
   else {
      /* Multiple points on the list */
      temp_point2 = temp_point1->last;
      temp_point1->last = cpoint;
      cpoint->next = temp_point1;
      cpoint->last = temp_point2;
      temp_point2->next = cpoint;
      }
   contour_points[i] = cpoint;
}

/* Insert a new point into the ring of points on this contour.  
   Note that cpoints must be non-NULL for this to work.  */
static void
Insert_Point(cpointPtr cpoints, Flt x, Flt y)
{
   cpointPtr cpoint, temp_point;

   cpoint = alloc_cpoint(x, y);
   if (cpoints->last == NULL) {
      cpoints->last = cpoint;
      cpoints->next = cpoint;
      cpoint->last = cpoints;
      cpoint->next = cpoints;
      }
   else {
      temp_point = cpoints->last;
      cpoints->last = cpoint;
      cpoint->next = cpoints;
      cpoint->last = temp_point;
      temp_point->next = cpoint;
      }
}

static cpointPtr
Rightmost(cpointPtr contour)
{
   cpointPtr cpoint, right;
   Flt max_x;

   max_x = contour->x;
   right = contour;
   for (cpoint = contour->next;
        cpoint != contour;
        cpoint = cpoint->next)
      if (cpoint->x > max_x) {
         max_x = cpoint->x;
         right = cpoint;
         }
   return right;
}

static void
ReverseContour(cpointPtr contour)
{
   cpointPtr root, next;

   if (contour == NULL || contour->next == NULL)
      return;

   root = contour;
   do {
      next = contour->next;
      contour->next = contour->last;
      contour->last = next;
      contour = next;
      } while (contour != root);
}

/* Compute the angle between two 2D line segments (the segments MUST
   have been normalized before calling this routine. Positive angles
   are counter-clockwise.  */
static Flt
SegmentAngle(Flt dx0, Flt dy0, Flt dx1, Flt dy1)
{
   Flt t0, ang, z;

   t0 = dx0 * dx1 + dy0 * dy1;
   ang = acos(t0);
   z = dx0 * dy1 - dx1 * dy0;
   if (z > 0) ang = -ang;
   return ang;
}

/* Determine the total number of times the contour winds around.
   It should be either +1 or -1.  Any other value means that it
   self-intersects.  The +1 and -1 aren't guarantees that the contour
   doesn't self-intersect, it's a necessary but not sufficient test. */
static Flt
WindingNumber(cpointPtr contour)
{
   cpointPtr root;
   Flt x0, x1, x2, y0, y1, y2;
   Flt dx0, dx1, dy0, dy1, l0;
   Flt total_angle;

   if (contour == NULL || contour->next == NULL ||
       contour->last == contour->next)
      /* If there are less than 3 points in the contour,
         we can't do much with it. */
      return 0;
   root = contour->next->next;

   x0 = contour->x;
   y0 = contour->y;
   contour = contour->next;
   x1 = contour->x;
   y1 = contour->y;
   contour = contour->next;
   x2 = contour->x;
   y2 = contour->y;

   dx0 = x1 - x0;
   dy0 = y1 - y0;
   l0 = sqrt(dx0 * dx0 + dy0 * dy0);
   if (fabs(l0) < EPSILON)
      /* Problem, segment length is zero */
      return 0;
   dx0 /= l0;
   dy0 /= l0;

   dx1 = x2 - x1;
   dy1 = y2 - y1;
   l0 = sqrt(dx1 * dx1 + dy1 * dy1);
   if (fabs(l0) < EPSILON)
      return 0;
   dx1 /= l0;
   dy1 /= l0;

   total_angle = SegmentAngle(dx0, dy0, dx1, dy1);

   for (contour = contour->next; contour != root;
        contour = contour->next) {
      x0 = x1; y0 = y1;
      x1 = x2; y1 = y2;
      dx0 = dx1; dy0 = dy1;
      x2 = contour->x;
      y2 = contour->y;
      dx1 = x2 - x1;
      dy1 = y2 - y1;
      l0 = sqrt(dx1 * dx1 + dy1 * dy1);
      if (fabs(l0) < EPSILON)
         return 0;
      dx1 /= l0;
      dy1 /= l0;
      total_angle += SegmentAngle(dx0, dy0, dx1, dy1);
      }
   total_angle = 0.5 * total_angle / M_PI;
   return total_angle;
}

static void
Create_Point_Contours(Object *obj, cpointPtr *contour_points, int *flags)
{
   int i, j, k, u_steps, npoints;
   Flt x, y, x0, y0, x1, y1, x2, y2;
   Flt xt0, xt1, xt2;
   Flt yt0, yt1, yt2;
   Flt u, delta_u;
   Contour *contours;
   int count;
   GlyphData *glyph = (GlyphData *)obj->o_data;

   contours = glyph->contours;
   count = glyph->count;

   /* Determine step sizes along a contour */
   u_steps = obj->o_uv_steps[0];
   delta_u = 1.0 / (Flt)u_steps;

   for (i=0;i<count;i++,contours++) {
      /* We start by creating the contours */
      x0 = contours->points[0][0];
      y0 = contours->points[0][1];
      npoints = contours->count;
      contour_points[i] = NULL;
      Push_Point(contour_points, i, x0, y0);
      flags[i] = 1;
      for (j=1;j<npoints;j++) {
         x1 = contours->points[j][0];
         y1 = contours->points[j][1];
         if (contours->points[j][2] == 0.0) {
            /* Linear glyph */
            Push_Point(contour_points, i, x1, y1);
            flags[i]++;

            /* Move to next vertex in the contour */
            x0 = x1;
            y0 = y1;
            }
         else {
            /* Quadratic glyph of some sort (no fear of having j walk past the
               end of "points", cause we allocated an extra slot at the end
               and made sure it's on the contour) */
            x2 = contours->points[j+1][0];
            y2 = contours->points[j+1][1];
            if (contours->points[j+1][2] != 0.0) {
               /* Parabola with far end floating - readjust the far end
                  so that it is on the curve.  (In the correct place too.) */
               x2 = 0.5 * (x1 + x2);
               y2 = 0.5 * (y1 + y2);
               }

            /* Make the interpolating quadrics */
            xt2 = x0 - 2.0 * x1 + x2;
            xt1 = 2.0 * (x1 - x0);
            xt0 = x0;
            yt2 = y0 - 2.0 * y1 + y2;
            yt1 = 2.0 * (y1 - y0);
            yt0 = y0;

            /* Move along the quadratic, using the number of steps
               defined by v_steps. */
            for (k=1,u=delta_u;k<u_steps;k++,u+=delta_u) {
               /* Create each point along the parabola */
               x = xt2 * u * u + xt1 * u + xt0;
               y = yt2 * u * u + yt1 * u + yt0;
               Push_Point(contour_points, i, x, y);
               flags[i]++;
               }

            x0 = x2;
            y0 = y2;
            }
         }

      /* Now see if this contour is clockwise or counterclockwise */
      flags[i] *= ((WindingNumber(contour_points[i]) < 0) ? -1.0 : 1.0);
      }
}

static void
Contour_Orientations(int count, cpointPtr *contour_points, int *flags)
{
   int i, j, k, npoints;
   int crossings, inside_flag;
   Flt x, y, x0, y0, x1, y1;
   cpointPtr cpoint;

   /* What we have now is a set of contours in the form of complex polygons.
      We need to now determine which polygons are inside and which are outside.
      All inside polygons need to be connected to other polygons. */
   for (i=0;i<count;i++) {
      x = contour_points[i]->x;
      y = contour_points[i]->y;

      /* Find the right most point */
      cpoint = Rightmost(contour_points[i]);

      /* Move the rightmost point so that it is the first one on the list */
      contour_points[i] = cpoint;

      /* Figure out if this is an inside or outside contour.  If this
         is an inside contour, then keep track of the closest contour
         to the right of it. */
      inside_flag = 0;
      crossings = 0;
      for (j=0;j<count;j++) {
         /* Don't check a contour against itself */
         if (i == j)
            continue;

         /* Walk through each line segment in the other contour, looking
            for ones that are to the right of the test point and have a
            span in y that covers the test point */
         cpoint = contour_points[j];
         npoints = fabs(flags[j]);
         x0 = cpoint->x;
         y0 = cpoint->y;
         cpoint = cpoint->next;
         for (k=0;k<npoints;cpoint=cpoint->next,k++) {
            x1 = cpoint->x;
            y1 = cpoint->y;
            crossings += Point_Segment_Test(x, y, x0, y0, x1, y1);
            x0 = x1;
            y0 = y1;
            }
         }
      if (crossings & 1)
         inside_flag = 1;

      /* If the current contour is inside another one then connect it
         to the one that is closest (whether it's an inside or outside
         contour itself). */
      if (inside_flag) {
         if (flags[i] < 0) {
            /* Interior contours must be counterclockwise, reverse it */
            ReverseContour(contour_points[i]);
            flags[i] = -flags[i];
            }
         }
      else {
         if (flags[i] > 0) {
            /* Exterior contours must be clockwise, reverse it. */
            ReverseContour(contour_points[i]);
            flags[i] = -flags[i];
            }
         }
      }
}

static void
Concatenate_Contours(int *count, cpointPtr *contour_points, int *flags)
{
   int c, i, j, k, npoints;
   int contour_index;
   cpointPtr cpoint0, cpoint1;
   cpointPtr contour_point0, contour_point1;
   Flt x, y, x0, y0, x1, y1;
   Flt xd, yd, xt, t, t_min;

   c = *count;
   for (i=0;i<c;) {
      if (flags[i] < 0) {
         /* Don't process an exterior contour */
         i++;
         continue;
         }

      /* Coordinates of rightmost point on this contour */
      x = contour_points[i]->x;
      y = contour_points[i]->y;

      t_min = PLY_HUGE;
      
      /* Find the closest line segment to the right */
      contour_point0 = NULL;
      for (j=0;j<c;j++) {
         /* Don't check a contour against itself */
         if (i == j)
            continue;

         /* Walk through each line segment in the other contour, looking
            for ones that are to the right of the test point and have a
            span in y that covers the test point */
         cpoint0 = contour_points[j];
         npoints = fabs(flags[j]);
         x0 = cpoint0->x;
         y0 = cpoint0->y;
         cpoint0 = cpoint0->next;
         for (k=0;k<npoints;cpoint0=cpoint0->next,k++) {
            x1 = cpoint0->x;
            y1 = cpoint0->y;
            if (Point_Segment_Test(x, y, x0, y0, x1, y1)) {
               /* Determine cut point on the connecting segment */
               t = (y - y0) / (y1 - y0);
               xt = (x0 + t * (x1 - x0)) - x;
               if (xt < t_min) {
                  xd = xt + x;
                  yd = y0 + t * (y1 - y0);
                  t_min = xt;
                  contour_index = j;
                  contour_point0 = cpoint0;
                  }
               }
            x0 = x1;
            y0 = y1;
            }
         }

      if (contour_point0 != NULL) {
         /* We found one to collapse this contour into */
         Insert_Point(contour_point0, xd, yd);
         Insert_Point(contour_point0, xd, yd);
         contour_point1 = contour_point0->last;
         contour_point0 = contour_point1->last;

         Insert_Point(contour_points[i], x, y);
         cpoint0 = contour_points[i];
         cpoint1 = cpoint0->last;

         cpoint0->last = contour_point0;
         contour_point0->next = cpoint0;
         cpoint1->next = contour_point1;
         contour_point1->last = cpoint1;

         contour_points[contour_index] = Rightmost(contour_points[i]);

         if (flags[contour_index] > 0)
            /* Attached two interior contours */
            flags[contour_index] += flags[i] + 3;
         else
            /* Attached an interior to an exterior */
            flags[contour_index] -= flags[i] + 3;

         /* Move all the rest of the contours back one in the
            array */
         for (j=i,c--;j<c;j++) {
            contour_points[j] = contour_points[j+1];
            flags[j] = flags[j+1];
            }
         }
      else {
         /* This is bad - we had an inside contour that we couldn't
            find anything to attach it to.  Guess we have to just
            ignore it. */
         i++;
         }
      }

   /* Update the total number of contours after the collapse */
   *count = c;
}

static void
Deallocate_Contours(int count, cpointPtr *contour_points, int *flags)
{
   int i;
   cpointPtr cpoint, contour, root;

   for (i=0;i<count;i++) {
      contour = root = contour_points[i];
      do {
         cpoint = contour;
         contour = contour->next;
         polyray_free(cpoint);
         } while (contour != root);
      }
   polyray_free(contour_points);
   polyray_free(flags);
}

static void
Render_Glyph_Faces(Viewpoint *eye, BinTree *Root, Object *obj)
{
   cpointPtr *contour_points;
   GlyphData *glyph = (GlyphData *)obj->o_data;
   int *flags=NULL;
   int count = glyph->count;
   jmp_buf temp_environ;
   int internal_abort;

   memcpy(temp_environ, abort_environ, sizeof(jmp_buf));

   if (setjmp(abort_environ) == 0) {
      /* Allocate memory for the contour lists */
      contour_points = polyray_malloc(glyph->count * sizeof(cpointPtr));
      flags = polyray_malloc(count * sizeof(int));
      if (contour_points == NULL || flags == NULL)
         error("Failed to allocate contour information");

      /* Turn the contour arrays into doubly linked lists of contour vertices.
         When we are done, the array 'contour_points' will hold the lists, and
         the array 'flags' will contain the number of points in each list as
         well as +/- indicating the orientation of the contour */
      Create_Point_Contours(obj, contour_points, flags);

      /* Determine which contours are inside and which are outside.  If any
         of the contours are in the wrong orientation (inside must be clockwise,
         outside must be counterclockwise) then this routine corrects it. */
      Contour_Orientations(count, contour_points, flags);

      /* Start concatenating the inside contours to their immediate neighbors.
         Continue until there are no more inside contours. */
      Concatenate_Contours(&count, contour_points, flags);

      /* At this point we should have the minimum set of contours, with no
         contours interior to any others. These contours can now be subdivided
         into triangles and spit out.  */
      Split_Contour(eye, Root, obj, count, contour_points, flags);
      internal_abort = 0;
      }
   else
      internal_abort = 1;

   /* Now that we are all done with the contour, we need to free up
      all the memory we allocated along the way. */
   Deallocate_Contours(count, contour_points, flags);

   memcpy(abort_environ, temp_environ, sizeof(jmp_buf));
   if (internal_abort)
      longjmp(abort_environ, 1);
}

static void
Render_Glyph_Sides(Viewpoint *eye, BinTree *Root, Object *obj)
{
   int i, j, k, l, npoints;
   Flt x0, x1, x2, y0, y1, y2;
   Flt xt0, xt1, xt2;
   Flt yt0, yt1, yt2;
   Poly Polygon;
   int u_steps, v_steps;
   Flt u, v, delta_u, delta_v;
   GlyphData *glyph = (GlyphData *)obj->o_data;
   Contour *contours;

   /* Determine step sizes along a contour */
   u_steps = obj->o_uv_steps[0];
   v_steps = obj->o_uv_steps[1];
   delta_u = 1.0 / (Flt)u_steps;
   delta_v = 1.0 / (Flt)v_steps;

   for (i=0,contours=glyph->contours;i<glyph->count;i++,contours++) {
      x0 = contours->points[0][0];
      y0 = contours->points[0][1];
      npoints = contours->count;
      for (j=1;j<=npoints;j++) {
         x1 = contours->points[j][0];
         y1 = contours->points[j][1];
         if (contours->points[j][2] == 0.0) {
            /* Linear glyph */
            for (k=0,u=0.0;k<u_steps;k++,u+=delta_u)
               for (l=0,v=0.0;l<v_steps;l++,v+=delta_v) {
                  Polygon.n = 4;
                  Linear_Contour_Evaluater(obj, x0, x1, y0, y1,
                                           u, v, &Polygon.vertices[0]);
                  Linear_Contour_Evaluater(obj, x0, x1, y0, y1,
                                           u, v+delta_v, &Polygon.vertices[1]);
                  Linear_Contour_Evaluater(obj, x0, x1, y0, y1,
                                           u+delta_u, v+delta_v, &Polygon.vertices[2]);
                  Linear_Contour_Evaluater(obj, x0, x1, y0, y1,
                                           u+delta_u, v, &Polygon.vertices[3]);
                  scan_convert(eye, Root, obj, NULL, &Polygon);
                  }

            /* Move to next vertex in the contour */
            x0 = x1;
            y0 = y1;
            }
         else {
            /* Quadratic glyph of some sort (no fear of having j walk past the
               end of "points", cause we allocated an extra slot at the end
               and made sure it's on the contour) */
            x2 = contours->points[j+1][0];
            y2 = contours->points[j+1][1];
            if (contours->points[j+1][2] != 0.0) {
               /* Parabola with far end floating - readjust the far end
                  so that it is on the curve.  (In the correct place too.) */
               x2 = 0.5 * (x1 + x2);
               y2 = 0.5 * (y1 + y2);
               }

            /* Make the interpolating quadrics */
            xt2 = x0 - 2.0 * x1 + x2;
            xt1 = 2.0 * (x1 - x0);
            xt0 = x0;
            yt2 = y0 - 2.0 * y1 + y2;
            yt1 = 2.0 * (y1 - y0);
            yt0 = y0;

            /* Move along the quadratic, using the number of steps
               defined by v_steps. */
            for (k=0,u=0.0;k<u_steps;k++,u+=delta_u) {
               for (l=0,v=0.0;l<v_steps;l++,v+=delta_v) {
                  Polygon.n = 4;
                  Quadratic_Contour_Evaluater(obj, xt0, xt1, xt2, yt0, yt1, yt2,
                                              u, v, &Polygon.vertices[0]);
                  Quadratic_Contour_Evaluater(obj, xt0, xt1, xt2, yt0, yt1, yt2,
                                              u, v+delta_v, &Polygon.vertices[1]);
                  Quadratic_Contour_Evaluater(obj, xt0, xt1, xt2, yt0, yt1, yt2,
                                              u+delta_u, v+delta_v, &Polygon.vertices[2]);
                  Quadratic_Contour_Evaluater(obj, xt0, xt1, xt2, yt0, yt1, yt2,
                                              u+delta_u, v, &Polygon.vertices[3]);
                  scan_convert(eye, Root, obj, NULL, &Polygon);
                  }
               }

            x0 = x2;
            y0 = y2;
            }
         }
      }
}

void
GlyphRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   /* Render the sides of the glyph */
   Render_Glyph_Sides(eye, Root, obj);

   /* Render the top and bottom faces of the glyph */
   Render_Glyph_Faces(eye, Root, obj);
}


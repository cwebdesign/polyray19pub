/*
  revolve.c

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
#include "vector.h"
#include "bound.h"
#include "roots.h"
#include "cone.h"
#include "revolve.h"

/* This is a placeholder for primitive data */
typedef struct t_revolvedata {
   int npoints;
   fVec *points;
   short int itype, Sturm_Flag;
   Transform trans;
   } RevolveData;

void RevolveRender(Viewpoint *, BinTree *, Object *);
int RevolveIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int RevolveInside(Object *obj, Vec P);
void RevolveDelete(Object *object);

ObjectProcs RevolveProcs = {
   RevolveRender,
   NULL,
   GenericInitialize,
   RevolveIntersect,
   RevolveInside,
   GenericCopy,
   RevolveDelete,
   };

#define DIST_LOW  (0.0 - EPSILON)
#define DIST_HIGH (1.0 + EPSILON)

static int
checkout_lathe_hit(Object *obj, Flt t, Flt s, Vec P, Vec D, Flt dist,
                   int itype, Flt xt1, Flt xt2, Flt yt1, Flt yt2,
                   Flt nmin, Flt nmax, Isect *hit)
{
   Flt dx, dy;
   Vec PP, PP1, V0, V1, N;
   RevolveData *revolve = (RevolveData *)obj->o_data;

   if (t > nmin && t <= nmax) {
      VecAddScaled(P, t, D, PP);
      if (itype == 1) {
         dx = xt1;
         dy = yt1;
         }
      else {
         dx = 2.0 * xt2 * s + xt1;
         dy = 2.0 * yt2 * s + yt1;
         }
      MakeVector(PP[0], PP[1], 0.0, V0);
      VecNormalize(V0);
      MakeVector(0.0, 0.0, 1.0, V1);
      VecComb(dy, V0,-dx, V1, N);
      InvTxNormal(N, N, &revolve->trans);
      InvTxVector1(PP1, PP, &revolve->trans);
      Insert_Hit(obj, PP1, N, t/dist, PP1, hit);
      return 1;
      }
   else
      return 0;
}

#if 0
/* Find the closest that the ray [P, D] gets to the z-axis between
   the values of z0 and z1, subject to minimum and maximum distance
   constraints along the ray.  This is useful for checking to see
   if the */
static Flt
closest_approach(Vec P, Vec D, Flt z0, Flt z1, Flt mindist, Flt maxdist)
{
   if (fabs(D[2]) < EPSILON) {
      /* Special case, not going up or down in z */
      if (P[2] < z0 || P[2] > z1)
         return PLY_HUGE;
      else {
         }
      }
   else if (1.0 - fabs(D[2]) < EPSILON) {
      /* Special case, parallel to z-axis */
      return sqrt(P[0] * P[0] + P[1] * P[1]);
      }
   else {
      /* General case, skew lines */
      }
}
#endif

static int
intersect_two_point_cylinder(Object *obj, Flt x0, Flt y0, Flt x1, Flt y1,
                             Flt a, Flt b, Flt c, Flt d,
                             Vec P, Vec D, Flt dist, Flt nmin, Flt nmax,
                             Isect *hit)
{
   int i, j, Flag;
   Flt l, v, t;
   Flt C[3], S[2];

   x1 = x1 - x0;
   y1 = y1 - y0;
   l  = sqrt(x1 * x1 + y1 * y1);
   if (l < EPSILON)
      return 0;
   x1 /= l;
   y1 /= l;
   C[0] = (a * x1 * x1 + b * y1 * y1);
   C[1] = (2 * a * x0 * x1 + 2 * b * y0 * y1 + c * y1);
   C[2] = a * x0 * x0 + b * y0 * y0 + c * y0 + d;

   j = solve_quadratic(C, S, 0, l);

   Flag = 0;
   for (i=0;i<j;i++) {
      if (fabs(D[2]) > EPSILON) {
         v = y0 + S[i] * y1;
         t = (v - P[2]) / D[2];
         if (checkout_lathe_hit(obj, t, S[i], P, D, dist, 1, x1, 0, y1, 0,
                                nmin, nmax, hit))
            Flag = 1;
         }
      else {
         v = x0 + S[i] * x1;
         b = -(P[0] * D[0] + P[1] * D[1]);
         d = b * b - (P[0] * P[0] + P[1] * P[1]) + v * v;
         if (d < 0.0)
            continue;
         d = sqrt(d);
         t = b - d;
         if (checkout_lathe_hit(obj, t, S[i], P, D, dist, 1, x1, 0, y1, 0,
                                nmin, nmax, hit))
            Flag = 1;
         t = b + d;
         if (checkout_lathe_hit(obj, t, S[i], P, D, dist, 1, x1, 0, y1, 0,
                                nmin, nmax, hit))
            Flag = 1;
         }
      }

   return Flag;
}

static int
intersect_three_point_cylinder(Object *obj, Flt x0, Flt y0, Flt x1, Flt y1,
                               Flt x2, Flt y2, Flt a, Flt b, Flt c, Flt d,
                               Vec P, Vec D, Flt dist, Flt nmin, Flt nmax,
                               Isect *hit)
{
   RevolveData *revolve = (RevolveData *)obj->o_data;
   Flt xt0, xt1, xt2, yt0, yt1, yt2;
   int i, j, k, Flag;
   Flt l, v, t;
   Flt C[5], S[4];

   /* Make the interpolating quadrics */
   xt2 = x0 - 2.0 * x1 + x2;
   xt1 = 2.0 * (x1 - x0);
   xt0 = x0;
   yt2 = y0 - 2.0 * y1 + y2;
   yt1 = 2.0 * (y1 - y0);
   yt0 = y0;

   Flag = 0;

   if (fabs(D[2]) < EPSILON) {
      /* Find the point of intersection of the ray with a cylinder
         that is along the z-axis.  This is a special case for
         when the ray is not moving up or down the z-axis. */
      if (fabs(y2 - y0) < EPSILON)
         return 0;
      C[0] = yt2;
      C[1] = yt1;
      C[2] = yt0 - P[2];
      j = solve_quadratic(C, S, 0, 1);
      for (i=0;i<j;i++) {
         l = S[i];
         v = l * (l * xt2 + xt1) + xt0;
         b = -(P[0] * D[0] + P[1] * D[1]);
         d = b * b - (P[0] * P[0] + P[1] * P[1]) + v * v;
         if (d < 0.0)
            return 0;
         d = sqrt(d);
         t = b - d;
         if (checkout_lathe_hit(obj, t, l, P, D, dist, 2, xt1, xt2, yt1, yt2,
                                nmin, nmax, hit))
            Flag = 1;
         t = b + d;
         if (checkout_lathe_hit(obj, t, l, P, D, dist, 2, xt1, xt2, yt1, yt2,
                                nmin, nmax, hit))
            Flag = 1;
         }
      return Flag;
      }
   else {
      /* Calculate the coefficients of the rotated quadrics */
      C[4] = d + a*xt0*xt0 + c*yt0 + b*yt0*yt0;
      C[3] = (2*a*xt0*xt1 + c*yt1 + 2*b*yt0*yt1);
      C[2] = (a*xt1*xt1 + 2*a*xt0*xt2 + b*yt1*yt1 + c*yt2 + 2*b*yt0*yt2);
      C[1] = (2*a*xt1*xt2 + 2*b*yt1*yt2);
      C[0] = (a*xt2*xt2 + b*yt2*yt2);

      if (revolve->Sturm_Flag == 0)
         /* Use Ferrari's method */
         j = solve_quartic(C, S, DIST_LOW, DIST_HIGH);
      else if (revolve->Sturm_Flag == 1)
         /* Use Vieta's method */
         j = solve_quartic1(C, S, DIST_LOW, DIST_HIGH);
      else
         /* Sturm sequences */
         j = bounded_polysolve(4, C, S, DIST_LOW, DIST_HIGH);

      for (k=0;k<j;k++) {
         v = S[k] * (S[k] * yt2 + yt1) + yt0;
         t = (v - P[2]) / D[2];
         if (checkout_lathe_hit(obj, t, S[k], P, D, dist, 2, xt1, xt2, yt1, yt2,
                                nmin, nmax, hit))
            Flag = 1;
         }
      return Flag;
      }
}

/*
The formula for determining where a ray intersects a line segment that
has been revolved about the z-axis is derived from the following:

   a = -z1^2,
   b = x1^2 + y1^2
   c = 2 (z1 (x0 x1 + y0 y1) - z0 b)
   d = z0^2 b -2 z0 z1 (x0 x1 + y0 y1) + z1^2 (x0^2 + y0^2)

The implicit equation in terms of the line parameter "s" will
then be:
   
   a x(s)^2 + b y(s)^2 + c y(s) + d = 0

There are currently two ways to make the surface.  The first is a simple
revolution of the line segments connecting each vertex around the axis.
The second way is to generate quadratic spline curves that smoothly
approximate the surface.

1) Generating simple line segments to connect the vertices, and
expanding terms leads to:

   s^2 * (a x1^2 + b y1^2) +
   s   * (2 a x0 x1 + c y1 + 2 b y0 y1) +
         a x0^2 + b y0^2 + c y0 + d

Happily this is a quadratic and can be easily solved.

2) Generating a piecewise quadratic approximation to the surface gives:

   x(s) = (1-s)^2 P0(x) + 2 s (1 - s) P1(x) + s^2 P2(x)

   x(s) = s^2 (P0(x) - 2 P1(x) + P2(x)) +
          s   2 (P1(x) - P0(x)) +
          P0(x)

Using a similar equation for y(s), and making the substitutions:

   xt2 = x0 - 2 x1 + x2, xt1 = 2 * (x1 - x0), xt0 = x0;
   yt2 = y0 - 2 y1 + y2, yt1 = 2 * (y1 - y0), yt0 = y0;

followed by a substitution of x(s) and y(s) into the implicit
equation above gives the quartic:

  s^4 * (a*xt2*xt2 + b*yt2*yt2) +
  s^3 * (2*a*xt1*xt2 + 2*b*yt1*yt2) +
  s^2 * (a*xt1*xt1 + 2*a*xt0*xt2 + b*yt1*yt1 + c*yt2 + 2*b*yt0*yt2) +
  s   * (2*a*xt0*xt1 + c*yt1 + 2*b*yt0*yt1) +
        d + a*xt0*xt0 + c*yt0 + b*yt0*yt0;

All solutions of this quartic that lie between 0 and 1 (inclusive) will
lie on the corresponding part of the surface.
*/
int
RevolveIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                 Flt mindist, Flt maxdist, Isect *hit)
{
   RevolveData *revolve = (RevolveData *)obj->o_data;
   int i, npoints, Flag, itype;
   Flt dist;
   Vec P, D;
   Flt x0, x1, x2, y0, y1, y2;
   Flt a, b, c, d;
   Flt nmin, nmax;
   int z0;
   fVec *points;

   Flag = 0;
   itype   = revolve->itype;
   points  = revolve->points;
   npoints = revolve->npoints;

   /* Now transform to canonical revolve space */
   TxVector(P, ray->P, &revolve->trans);
   TxDirection(D, ray->D, &revolve->trans);
   dist = VecNormalize(D);
   nmin = dist * mindist;
   nmax = dist * maxdist;

   a = -D[2] * D[2];
   b = D[0] * D[0] + D[1] * D[1];
   c = 2 * (D[2] * (P[0] * D[0] + P[1] * D[1]) - P[2] * b);
   d = P[2] * P[2] * b - 2 * P[2] * D[2] * (P[0] * D[0] + P[1] * D[1]) -
       a * (P[0] * P[0] + P[1] * P[1]);

   x0 = points[0][0];
   y0 = points[0][1];
   z0 = 0; /* First point must be on-curve */
   for (i=1;i<npoints;i++) {
      /* Grab the control vertices */
      x1 = revolve->points[i][0];
      y1 = revolve->points[i][1];

      /* Last point must be on curve, all others may float. */
      if (itype == 1 || (itype == 3 && points[i][2] == 0.0)) {
         /* Straight line segment */
         if (intersect_two_point_cylinder(obj, x0, y0, x1, y1, a, b, c, d,
                                          P, D, dist, nmin, nmax, hit))
            Flag = 1;
         x0 = x1;
         y0 = y1;
         }
      else {
         x2 = points[i + 1][0];
         y2 = points[i + 1][1];

         if ((i < npoints-2) && (itype == 2 || points[i+1][2] != 0.0)) {
            /* Parabola with far end floating - readjust the far end
               so that it is on the curve.  (In the correct place too.) */
            x2 = 0.5 * (x1 + x2);
            y2 = 0.5 * (y1 + y2);
            }

         /* Parabolic segment */
         if (intersect_three_point_cylinder(obj, x0, y0, x1, y1, x2, y2,
                                            a, b, c, d, P, D, dist,
                                            nmin, nmax, hit))
            Flag = 1;
         if (i == npoints-2)
            break;
         x0 = x2;
         y0 = y2;
         }
      }
   return Flag;
}

int
RevolveInside(Object *obj, Vec Pos)
{
   Vec P;
   Flt x, y;
   RevolveData *revolve = (RevolveData *)obj->o_data;

   /* Transform the ray into the lathes space */
   InvTxVector1(P, Pos, obj->o_trans)

   /* Transform to canonical revolve space */
   TxVector(P, P, &revolve->trans);
   
   /* Project the point onto the x-y plane and see if it
      is inside the sweep polygon */
   x = sqrt(P[0]*P[0] + P[1] * P[1]);
   y = P[2];

   return Inside_Contour(x, y, revolve->itype, revolve->npoints,
                         revolve->points);
}

Object *
MakeRevolve(Object *obj, int itype, Vec axis, int npoints, fVec *points)
{
   int i;
   Flt len, t;
   Vec bot, mins, maxs;
   RevolveData *revolve;

   obj->o_type  = T_REVOLVE;
   obj->o_procs = &RevolveProcs;
   obj->o_uv_steps[0] = 8;
   if (itype == 1)
      obj->o_uv_steps[1] = 1;
   else
      obj->o_uv_steps[1] = 8;

   /* Must be at least two points */
   if (npoints < 2)
      error("Bad surface of revolution, must be at least 2 points\n");

#if 0
   /* Step through the points to see that no line segment
      crosses the y-axis */
   for (i=0;i<npoints-1;i++)
      if (points[i][0] * points[i+1][0] < 0)
         warning("Bad surface of revolution, segments should not cross y-axis\n");
#endif

   if (npoints == 2) {
      /* Quietly turn this surface into a cone */
      MakeVector(0.0, points[0][1], 0.0, mins);
      MakeVector(0.0, points[1][1], 0.0, maxs);
      return MakeCone(obj, mins, points[0][0],
                      maxs, points[1][0]);
      }

   /* Attempt to allocate memory for this primitive */
   if ((revolve = (RevolveData *)polyray_malloc(sizeof(RevolveData))) == NULL)
      error("Failed to allocate revolve data\n");
   revolve->itype = itype;
   revolve->Sturm_Flag = 1; /* Default to the method of Vieta */

   /* Find the axis and axis length */
   len = VecNormalize(axis);
   if (len < EPSILON)
      error("Degenerate lathe axis\n");
   revolve->npoints = npoints;
   revolve->points = (fVec *)polyray_malloc((npoints+1) * sizeof(fVec));
   if (revolve->points == NULL)
      error("Failed to allocate polygon data\n");
   for (i=0;i<npoints;i++)
      VecCopy(points[i], revolve->points[i]);
   VecCopy(points[0], revolve->points[npoints]);

   /* The first and last vertices are fixed */
   revolve->points[0][2] = 0.0;
   revolve->points[npoints-1][2] = 0.0;

   MakeVector(0.0, 0.0, 0.0, bot);
   Get_Coordinate_Transform(&revolve->trans, bot, axis, 1.0, 1.0);

   /* Compute bounding information */
   mins[1] = mins[0] = MIN(points[0][0], -points[0][0]);
   maxs[1] = maxs[0] = MAX(points[0][0], -points[0][0]);
   maxs[2] = mins[2] = points[0][1];
   for (i=1;i<npoints;i++) {
      t = points[i][0];
      if (t < mins[0]) {
         mins[0] = t;
         mins[1] = t;
         }
      if (-t < mins[0]) {
         mins[0] = -t;
         mins[1] = -t;
         }
      if (t > maxs[0]) {
         maxs[0] = t;
         maxs[1] = t;
         }
      if (-t > maxs[0]) {
         maxs[0] = -t;
         maxs[1] = -t;
         }
      t = points[i][1];
      if (t < mins[2]) mins[2] = t;
      if (t > maxs[2]) maxs[2] = t;
      }
   VecCopy(mins, obj->o_bnd.lower_left);
   VecSub(maxs, mins, obj->o_bnd.lengths);
   recompute_inverse_bbox(&obj->o_bnd, &revolve->trans);
   obj->o_data = (void *)revolve;
   polyray_free(points);
   return obj;
}

void
Set_Lathe_Solver(Object *obj, int Sturm_Flag)
{
   RevolveData *revolve = (RevolveData *)obj->o_data;
   revolve->Sturm_Flag = Sturm_Flag;
}

void
RevolveDelete(Object *obj)
{
   RevolveData *revolve = (RevolveData *)obj->o_data;
   if (obj->o_copy == 0) {
      polyray_free(revolve->points);
      polyray_free(obj->o_data);
      }
}

static void
Linear_Evaluater(Object *obj, Flt dx, Flt dy, Flt x0, Flt y0,
                 Flt x1, Flt y1, Flt u, Flt v, Vertex *vert)
{
   RevolveData *revolve = (RevolveData *)obj->o_data;
   Vec P, N, v0, v1;
   Flt x, y, theta = TWO_PI * u;

   x = (1.0 - v) * x0 + v * x1;
   y = (1.0 - v) * y0 + v * y1;

   MakeVector( x*cos(theta), x*sin(theta), y, P);
   MakeVector(P[0], P[1], 0, v0);
   VecNormalize(v0);
   MakeVector(0.0, 0.0, 1.0, v1);
   VecComb(dy, v0,-dx, v1, N);

   InvTxVector(P, P, &revolve->trans);
   InvTxNormal(N, N, &revolve->trans);

   VecCopy(P, vert->P);
   VecCopy(P, vert->U);
   if (obj->o_trans) {
      TxVector(P, P, obj->o_trans);
      TxNormal(N, N, obj->o_trans);
      }
   VecNormalize(N);
   VecCopy(P, vert->W);
   VecCopy(N, vert->N);
}

static void
Quadratic_Evaluater(Object *obj, Flt xt0, Flt yt0, Flt xt1, Flt yt1,
                    Flt xt2, Flt yt2, Flt u, Flt v, Vertex *vert)
{
   RevolveData *revolve = (RevolveData *)obj->o_data;
   Vec P, N;
   Flt x, y, dx, dy;
   Flt theta = TWO_PI * u;

   /* Calculate position and normal information */
   x = xt2 * v * v + xt1 * v + xt0;
   y = yt2 * v * v + yt1 * v + yt0;

   dx = 2.0 * xt2 * v + xt1;
   dy = 2.0 * yt2 * v + yt1;

   MakeVector(x*cos(theta),x*sin(theta), y, P);
   MakeVector(dy*cos(theta), dy*sin(theta),-dx, N);

   InvTxVector(P, P, &revolve->trans);
   InvTxNormal(N, N, &revolve->trans);

   VecCopy(P, vert->P);
   VecCopy(P, vert->U);
   if (obj->o_trans) {
      TxVector(P, P, obj->o_trans);
      TxNormal(N, N, obj->o_trans);
      }
   VecNormalize(N);
   VecCopy(P, vert->W);
   VecCopy(N, vert->N);
}

void
RevolveRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   Flt x0, x1, x2, y0, y1, y2;
   Flt len, dx, dy;
   Flt xt0, xt1, xt2;
   Flt yt0, yt1, yt2;
   int j, k, l, u_steps, v_steps;
   Flt u, v, delta_u, delta_v;
   Poly Polygon;
   RevolveData *revolve = (RevolveData *)obj->o_data;
   int npoints = revolve->npoints;
   int itype   = revolve->itype;
   fVec *points = revolve->points;

   u_steps = obj->o_uv_steps[0];
   v_steps = obj->o_uv_steps[1];
   delta_u = 1.0 / (Flt)u_steps;
   delta_v = 1.0 / (Flt)v_steps;

   x0 = points[0][0];
   y0 = points[0][1];

   /* If this is a type 2 lathe surface, then we have to prevent overrun of
      the point array during evaluation */
   if (itype == 2)
      npoints--;

   for (j=1;j<npoints;j++) {
      x1 = points[j][0];
      y1 = points[j][1];
      if (itype == 1 || (itype == 3 && points[j][2] == 0.0)) {
         dx = (x1 - x0);
         dy = (y1 - y0);
         len = sqrt(dx*dx+dy*dy);
         if (len < EPSILON)
            continue;
         dx /= len;
         dy /= len;
         /* Linear cross section */
         for (k=0,u=0.0;k<u_steps;k++,u+=delta_u)
            for (l=0,v=0.0;l<v_steps;l++,v+=delta_v) {
               Polygon.n = 4;
               Linear_Evaluater(obj, dx, dy, x0, y0, x1, y1,
                                u, v, &Polygon.vertices[0]);
               Linear_Evaluater(obj, dx, dy, x0, y0, x1, y1,
                                u, v+delta_v, &Polygon.vertices[1]);
               Linear_Evaluater(obj, dx, dy, x0, y0, x1, y1,
                                u+delta_u, v+delta_v, &Polygon.vertices[2]);
               Linear_Evaluater(obj, dx, dy, x0, y0, x1, y1,
                                u+delta_u, v, &Polygon.vertices[3]);
               scan_convert(eye, Root, obj, NULL, &Polygon);
               }

         /* Move to next vertex in the contour */
         x0 = x1;
         y0 = y1;
         }
      else {
         /* Quadratic cross section */
         x2 = points[j+1][0];
         y2 = points[j+1][1];

         if ((j < npoints-2) && (itype == 2 || points[j+1][2] != 0.0)) {
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
               Quadratic_Evaluater(obj, xt0, yt0, xt1, yt1, xt2, yt2,
                                   u, v, &Polygon.vertices[0]);
               Quadratic_Evaluater(obj, xt0, yt0, xt1, yt1, xt2, yt2,
                                   u, v+delta_v, &Polygon.vertices[1]);
               Quadratic_Evaluater(obj, xt0, yt0, xt1, yt1, xt2, yt2,
                                   u+delta_u, v+delta_v, &Polygon.vertices[2]);
               Quadratic_Evaluater(obj, xt0, yt0, xt1, yt1, xt2, yt2,
                                   u+delta_u, v, &Polygon.vertices[3]);
               scan_convert(eye, Root, obj, NULL, &Polygon);
               }
            }

         x0 = x2;
         y0 = y2;
         }
      }
}

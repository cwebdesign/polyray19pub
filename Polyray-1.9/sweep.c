/*
  sweep.c

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
#include "scan.h"
#include "vector.h"
#include "bound.h"
#include "symtab.h"
#include "roots.h"
#include "sweep.h"

/* This is a placeholder for primitive data */
typedef struct t_sweepdata {
   Vec axis;
   int itype, npoints;
   fVec *points;
   Transform trans;
   } SweepData;

void SweepRender(Viewpoint *, BinTree *, Object *);
int SweepIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int SweepInside(Object *obj, Vec P);
void SweepDelete(Object *object);

ObjectProcs SweepProcs = {
   SweepRender,
   NULL,
   GenericInitialize,
   SweepIntersect,
   SweepInside,
   GenericCopy,
   SweepDelete,
   };

#define DIST_LOW  (0.0 - EPSILON)
#define DIST_HIGH (1.0 + EPSILON)

static int
SweepIntersect1(Object *obj, Ray *ray, Flt mindist, Flt maxdist,
                Isect *hit)
{
   int i, npoints, Flag = 0;
   Flt dist, nmin;
   Flt t, t0, z;
   Vec P, PP, D, N;
   Flt u0, u1, v0, v1;
   Flt d0, d1;
   int dirflag = 0;
   SweepData *sweep = (SweepData *)obj->o_data;

   /* Now transform to canonical sweep space */
   TxVector(P, ray->P, &sweep->trans);
   TxDirection(D, ray->D, &sweep->trans);
   dist = VecNormalize(D);
   nmin = mindist * dist;
   npoints = sweep->npoints;

   if (fabs(D[0]) < EPSILON)
      if (fabs(D[1]) < EPSILON)
         /* This means the ray is moving parallel
            to the walls of the sweep surface */
         return 0;
      else
         dirflag = 0;
   else
      dirflag = 1;

   for (i=0;i<npoints-1;i++) {
      u0 = sweep->points[i][0];
      v0 = sweep->points[i][1];
      u1 = sweep->points[i+1][0];
      v1 = sweep->points[i+1][1];

      d0 = (u1 - u0);
      d1 = (v1 - v0);
      t0 = d1 * D[0] - d0 * D[1];
      if (fabs(t0) < EPSILON)
         /* No possible intersection */
         continue;
      t = (D[0] * (P[1] - v0) - D[1] * (P[0] - u0)) / t0;
      if (t < DIST_LOW || t > DIST_HIGH)
         continue;
      if (dirflag)
         t = ((u0 + t * d0) - P[0]) / D[0];
      else
         t = ((v0 + t * d1) - P[1]) / D[1];
      z  = P[2] + t * D[2];

      if (z >= DIST_LOW && z <= DIST_HIGH &&
          t > nmin && t <= dist * maxdist) {
         VecAddScaled(P, t, D, PP);
         MakeVector(d1, -d0, 0.0, N);
         InvTxNormal(N, N, &sweep->trans);
         InvTxVector(PP, PP, &sweep->trans);
         Insert_Hit(obj, PP, N, t/dist, P, hit);
         Flag = 1;
         }
      }
   return Flag;
}


/*
Solving for a linear sweep of a non-linear curve can be performed by
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
static int
SweepIntersect2(Object *obj, Ray *ray, Flt mindist, Flt maxdist,
                Isect *hit)
{
   int i, j, k, npoints, Flag = 0;
   Flt dist, nmin, nmax;
   Vec P, PP, D, N;
   Flt x0, x1, y0, y1, x2, y2, u, v, t;
   Flt xt0, xt1, xt2, yt0, yt1, yt2;
   Flt a, b, c, C[3], S[2];
   int dirflag = 0;
   SweepData *sweep = (SweepData *)obj->o_data;

   /* Now transform to canonical sweep space */
   TxVector(P, ray->P, &sweep->trans);
   TxDirection(D, ray->D, &sweep->trans);
   dist = VecNormalize(D);
   nmin = mindist * dist;
   nmax = maxdist * dist;
   npoints = sweep->npoints;

   if (fabs(D[0]) < EPSILON)
      if (fabs(D[1]) < EPSILON)
         /* This means the ray is moving parallel
            to the walls of the sweep surface */
         return 0;
      else
         dirflag = 0;
   else
      dirflag = 1;

   a = D[1];
   b = -D[0];
   c = (P[1] * D[0] - P[0] * D[1]);

   for (i=0;i<npoints-2;i++) {
      x0 = sweep->points[i][0];
      y0 = sweep->points[i][1];
      x1 = sweep->points[i+1][0];
      y1 = sweep->points[i+1][1];
      x2 = sweep->points[i+2][0];
      y2 = sweep->points[i+2][1];

      if ((i > 0) && (sweep->itype == 2 || sweep->points[i+1][2] != 0.0)) {
         x0 = (x0 + x1) / 2.0;
         y0 = (y0 + y1) / 2.0;
         }
      if ((i < npoints-3) &&
          (sweep->itype == 2 || sweep->points[i+1][2] != 0.0)) {
         x2 = (x1 + x2) / 2.0;
         y2 = (y1 + y2) / 2.0;
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

      j = solve_quadratic(C, S, DIST_LOW, DIST_HIGH);

      for (k=0;k<j;k++) {
         if (dirflag) {
            u = S[k] * S[k] * xt2 + S[k] * xt1 + xt0;
            t = (u - P[0]) / D[0];
            }
         else {
            v = S[k] * S[k] * yt2 + S[k] * yt1 + yt0;
            t = (v - P[1]) / D[1];
            }
         if (t > nmin && t <= nmax) {
            VecAddScaled(P, t, D, PP);
            if (PP[2] >= 0.0 && PP[2] <= 1.0) {
               MakeVector(PP[0], PP[1], 0.0, N);
               InvTxNormal(N, N, &sweep->trans);
               InvTxVector(PP, PP, &sweep->trans);
               Insert_Hit(obj, PP, N, t/dist, P, hit);
               Flag = 1;
               }
            }
         }
      }
   return Flag;
}

int
SweepIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
               Flt mindist, Flt maxdist, Isect *hit)
{
   SweepData *sweep = (SweepData *)obj->o_data;
   int Flag;

   if (sweep->itype == 1)
      Flag = SweepIntersect1(obj, ray, mindist, maxdist, hit);
   else
      Flag = SweepIntersect2(obj, ray, mindist, maxdist, hit);
   return Flag;
}

int
SweepInside(Object *obj, Vec Pos)
{
   Vec P;
   SweepData *sweep = (SweepData *)obj->o_data;

   /* Transform the ray into the lathes space */
   InvTxVector1(P, Pos, obj->o_trans);

   /* Transform to canonical sweep space */
   TxVector(P, P, &sweep->trans);
   
   /* See if the point is either above or below the sweep */
   if (P[2] < DIST_LOW || P[2] > DIST_HIGH)
      return 0;

   /* Project the point onto the x-y plane and see if it
      is inside the sweep polygon */
   return Inside_Contour(P[0], P[1], sweep->itype,
                         sweep->npoints, sweep->points);
}

Object *
MakeSweep(Object *object, int itype, Vec axis, int npoints, fVec *points)
{
   int i, j;
   Flt len, t;
   Vec bot, mins, maxs;
   SweepData *sweep;

   object->o_type  = T_SWEEP;
   object->o_procs = &SweepProcs;
   if (itype == 1)
      object->o_uv_steps[0] = 1;
   else if (itype == 2 || itype == 3)
      object->o_uv_steps[0] = 8;
   else
      error("Only linear (type 1) and quadratic (type 2/3) sweeps allowed");
   object->o_uv_steps[1] = 1;

   /* Attempt to allocate memory for this primitive */
   if ((sweep = (SweepData *)polyray_malloc(sizeof(SweepData))) == NULL)
      error("Failed to allocate sweep data\n");
   sweep->itype = itype;

   /* Find the axis and axis length */
   len = VecNormalize(axis);
   if (len < EPSILON)
      error("Degenerate sweep\n");
   sweep->npoints = npoints;
   sweep->points = (fVec *)polyray_malloc((npoints+1) * sizeof(fVec));
   if (sweep->points == NULL)
      error("Insufficient memory for sweep");
   for (i=0;i<npoints;i++)
      VecCopy(points[i], sweep->points[i])
   VecCopy(points[0], sweep->points[npoints]);
   polyray_free(points);
   MakeVector(0.0, 0.0, 0.0, bot);
   Get_Coordinate_Transform(&sweep->trans, bot, axis, 1.0, len);

   /* Compute bounding information */
   VecCopy(sweep->points[0], mins);
   VecCopy(mins, maxs);
   for (i=1;i<npoints;i++)
      for (j=0;j<2;j++) {
         t = sweep->points[i][j];
         if (t < mins[j]) mins[j] = t;
         if (t > maxs[j]) maxs[j] = t;
         }
   mins[2] = 0.0;
   maxs[2] = 1.0;
   VecCopy(mins, object->o_bnd.lower_left);
   VecSub(maxs, mins, object->o_bnd.lengths);
   recompute_inverse_bbox(&object->o_bnd, &sweep->trans);
   object->o_data = (void *)sweep;
   return object;
}

void
SweepDelete(Object *object)
{
   SweepData *sweep = (SweepData *)object->o_data;
   if (object->o_copy == 0) {
      polyray_free(sweep->points);
      polyray_free(object->o_data);
      }
}

static void
Sweep_Evaluater2(Object *obj, int i, Flt u, Flt v, Vertex *vert)
{
   Vec P, N;
   Flt x0, x1, x2, y0, y1, y2, x, y, dx, dy;
   Flt xt2, xt1, xt0, yt2, yt1, yt0;
   SweepData *sweep = (SweepData *)obj->o_data;
   int npoints = sweep->npoints;

   /* Calculate the points */
   x0 = sweep->points[i][0];
   y0 = sweep->points[i][1];
   x1 = sweep->points[i+1][0];
   y1 = sweep->points[i+1][1];
   x2 = sweep->points[i+2][0];
   y2 = sweep->points[i+2][1];

   if ((i > 0) && (sweep->itype == 2 || sweep->points[i+1][2] != 0.0)) {
      x0 = (x0 + x1) / 2.0;
      y0 = (y0 + y1) / 2.0;
      }
   if ((i < npoints-3) && (sweep->itype == 2 || sweep->points[i+1][2] != 0.0)) {
      x2 = (x1 + x2) / 2.0;
      y2 = (y1 + y2) / 2.0;
      }

   /* Make the interpolating quadrics */
   xt2 = x0 - 2.0 * x1 + x2;
   xt1 = 2.0 * (x1 - x0);
   xt0 = x0;
   yt2 = y0 - 2.0 * y1 + y2;
   yt1 = 2.0 * (y1 - y0);
   yt0 = y0;

   /* Calculate position and normal information */
   x = (xt2 * u + xt1) * u + xt0;
   y = (yt2 * u + yt1) * u + yt0;

   dx = 2.0 * xt2 * u + xt1;
   dy = 2.0 * yt2 * u + yt1;

   MakeVector(x, y, v, P);
   MakeVector(dy,-dx, 0.0, N);
   InvTxVector(P, P, &sweep->trans);
   InvTxNormal(N, N, &sweep->trans);

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
SweepRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   int i, j, k;
   Vec N, Pos[4];
   SweepData *sweep = (SweepData *)obj->o_data;
   Poly Polygon;
   int npoints = sweep->npoints;
   int u_steps, v_steps;
   Flt u, v, delta_u, delta_v;

   if (sweep->itype == 1)
      for (i=0;i<npoints-1;i++) {
         /* Calculate the points */
         Pos[0][0] = Pos[3][0] = sweep->points[i+1][0];
         Pos[0][1] = Pos[3][1] = sweep->points[i+1][1];
         Pos[1][0] = Pos[2][0] = sweep->points[i][0];
         Pos[1][1] = Pos[2][1] = sweep->points[i][1];
         Pos[0][2] = Pos[1][2] = 0.0;
         Pos[2][2] = Pos[3][2] = 1.0;

         /* Compute the normal */
         N[0] = Pos[0][1] - Pos[1][1];
         N[1] = Pos[1][0] - Pos[0][0];
         N[2] = 0.0;

         for (j=0;j<4;j++) {
            InvTxVector(Pos[j], Pos[j], &sweep->trans);
            VecCopy(Pos[j], Polygon.vertices[j].P);
            VecCopy(Pos[j], Polygon.vertices[j].U);
            }
         InvTxNormal(N, N, &sweep->trans);

         if (obj->o_trans) {
            for (j=0;j<4;j++)
               TxVector(Pos[j], Pos[j], obj->o_trans);
            TxNormal(N, N, obj->o_trans);
            }
         VecNormalize(N);

         Polygon.n = 4;
         for (j=0;j<4;j++) {
            VecCopy(Pos[j], Polygon.vertices[j].W);
            VecCopy(N, Polygon.vertices[j].N);
            }
         scan_convert(eye, Root, obj, NULL, &Polygon);
         }
   else {
      u_steps = obj->o_uv_steps[0];
      v_steps = obj->o_uv_steps[1];
      delta_u = 1.0 / (Flt)u_steps;
      delta_v = 1.0 / (Flt)v_steps;
      for (i=0;i<npoints-2;i++) {
         /* Dump out polygons */
         for (j=0,u=0.0;j<u_steps;j++,u+=delta_u) {
            for (k=0,v=0.0;k<v_steps;k++,v+=delta_v) {
               Polygon.n = 4;
               Sweep_Evaluater2(obj, i,
                                u, v, &Polygon.vertices[0]);
               Sweep_Evaluater2(obj, i,
                                u, v+delta_v, &Polygon.vertices[1]);
               Sweep_Evaluater2(obj, i,
                                u+delta_u, v+delta_v, &Polygon.vertices[2]);
               Sweep_Evaluater2(obj, i,
                                u+delta_u, v, &Polygon.vertices[3]);
               scan_convert(eye, Root, obj, NULL, &Polygon);
               }
            }
         }
      }
}

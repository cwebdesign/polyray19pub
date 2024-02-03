/* cylinder.c

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
#include "intersec.h"
#include "symtab.h"
#include "scan.h"
#include "vector.h"
#include "bound.h"
#include "cylinder.h"

/* This is a placeholder for primitive data */
typedef struct t_conedata {
   short int closed;
   Vec bot, top;
   Flt radius;
   Transform trans;
   } CylData;

void Cylinder_Evaluater(Object *, float, float, Vertex *);
int CylIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int CylNormal(CylData *cyl, Vec Pos, Vec N);
void CylUV(Vec Pos, Vec D, Flt t, Flt *u, Flt * v);
int CylInside(Object *obj, Vec Pos);

ObjectProcs CylProcs = {
   GenericRender,
   Cylinder_Evaluater,
   GenericInitialize,
   CylIntersect,
   CylInside,
   GenericCopy,
   GenericDelete,
   };

int
CylNormal(CylData *cyl, Vec Pos, Vec N)
{
   Vec P;
   /* Calculate the normal in cyl space */
   TxVector(P, Pos, &cyl->trans);
   P[2] = 0.0;
   InvTxNormal(N, P, &cyl->trans);
   return 1;
}

void
CylUV(Vec Pos, Vec D, Flt t, Flt *u, Flt * v)
{
   Flt x, len, theta;
   Vec P;
   
   VecAddScaled(Pos, t, D, P);
   len = sqrt(P[0] * P[0] + P[1] * P[1]);
   /* Make sure this vector is on the unit cylinder. */
   if (len < EPSILON)
      theta = 0;
   else {
      x  = P[0] / len;
      if (P[1] == 0.0)
         if (x > 0)
            theta = 0.0;
         else
            theta = M_PI;
      else {
         theta = acos(x);
         if (P[1] < 0.0)
            theta = (2.0 * M_PI) - theta;
         }
      }
   *u = theta / (2.0 * M_PI);
   *v = P[2];
}

static int
check_cylinder_hit(Object *obj, CylData *cyl,
                   Ray *ray, Vec P, Vec D, Flt t,
                   Flt nmin, Flt nmax, Flt dist, Isect *hit)
{
   Vec PP, N, U;
   Flt u, v;

   if (t >= nmin && t <= nmax) {
      if ((Global_Shade_Flag & UV_CHECK) &&
          (obj->o_sflag & UV_CHECK)) {
         CylUV(P, D, t, &u, &v);
         MakeVector(u, v, 0, U);
         }
      else
         VecCopy(P, U);
      t /= dist;
      VecAddScaled(ray->P, t, ray->D, PP);
      CylNormal(cyl, PP, N);
      return Insert_Hit(obj, PP, N, t, U, hit);
      }
   else
      return 0;
}

int
CylIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
             Flt mindist, Flt maxdist, Isect *hit)
{
   Flt t1, t2, a, b, c;
   Flt disc, x, y, z, dist, nmin, nmax;
   Vec P, D;
   int Flag = 0;
   CylData *cyl = (CylData *)obj->o_data;

   /* Now transform to canonical cylinder space */
   TxVector(P, ray->P, &cyl->trans);
   TxDirection(D, ray->D, &cyl->trans);
   dist = VecNormalize(D);
   nmin = mindist * dist;
   nmax = maxdist * dist;

   /* Do some simple exception testing */
   if (P[2] > 1.0 && D[2] > 0.0)
      return 0;
   if (P[2] < 0.0 && D[2] < 0.0)
      return 0;
   b = P[0] * D[0] + P[1] * D[1];
   c = P[0] * P[0] + P[1] * P[1] - 1.0;
   if (c > 0.0 && b > 0.0)
      /* Ray starts outside the cylinder, and continues
         away from it. */
      return 0;

   if (cyl->closed && fabs(D[2]) > 0.0) {
      /* Look for intersections with cylinder caps */
      t1 = -P[2] / D[2]; /* Intersection with z=0 plane */
      x = (P[0] + t1 * D[0]);
      y = (P[1] + t1 * D[1]);
      if ((x * x + y * y <= 1.0) &&
          check_cylinder_hit(obj, cyl, ray, P, D, t1,
                             nmin, nmax, dist, hit))
         Flag = 1;
      t1 += 1.0 / D[2];  /* Intersections with z=1 plane */
      x = (P[0] + t1 * D[0]);
      y = (P[1] + t1 * D[1]);
      if ((x * x + y * y <= 1.0) &&
          check_cylinder_hit(obj, cyl, ray, P, D, t1,
                             nmin, nmax, dist, hit))
         Flag = 1;
      }

   /* Look for intersections with the cylinder walls */
   a = D[0] * D[0] + D[1] * D[1];
   if (a < EPSILON)
      /* Ray goes straight up and down, can't hit walls */
      return Flag;

   disc = b * b - a * c;
   if (disc < 0.0) return Flag;
   disc = sqrt(disc);
   t1 = (-b + disc) / a;
   t2 = (-b - disc) / a;
   z = P[2] + t1 * D[2];
   if (z >= 0.0 && z <= 1.0 &&
       check_cylinder_hit(obj, cyl, ray, P, D, t1,
                          nmin, nmax, dist, hit))
      Flag = 1;
   z = P[2] + t2 * D[2];
   if (z >= 0.0 && z <= 1.0 &&
       check_cylinder_hit(obj, cyl, ray, P, D, t2,
                          nmin, nmax, dist, hit))
      Flag = 1;

   return Flag;
}

int
CylInside(Object *obj, Vec Pos)
{
   /* For csg purposes, treat the cylinder as if it were
      capped at each end */
   Vec P;
   Flt w2;
   CylData *cyl = (CylData *)obj->o_data;

   InvTxVector1(P, Pos, obj->o_trans)

   /* Transform to canonical cone space */
   TxVector(P, P, &cyl->trans);
   w2 = P[0] * P[0] + P[1] * P[1];

   return ((w2 < 1.0 && P[2] > 0.0 && P[2] < 1.0) ? 1 : 0);
}

Object *
MakeCylinder(Object *object, Vec bot, Vec top, Flt radius)
{
   Vec axis;
   Flt len;
   CylData *cyl;

   object->o_type = T_CYLINDER;
   object->o_procs = &CylProcs;
   object->o_uv_steps[0] = 16;
   object->o_uv_steps[1] =  2;

   /* Attempt to allocate memory for this primitive */
   if ((cyl = (CylData *)polyray_malloc(sizeof(CylData))) == NULL)
      error("Failed to allocate cylinder data\n");

   /* Process the primitive specific information */
   if (radius < EPSILON)
      error("Degenerate Cylinder\n");
   /* Find the axis and axis length */
   VecSub(top, bot, axis);
   len = VecNormalize(axis);
   if (len < EPSILON)
      error("Degenerate cyl\n");
   VecCopy(bot, cyl->bot);
   VecCopy(top, cyl->top);
   cyl->radius = radius;
   VecNegate(bot);
   Get_Coordinate_Transform(&cyl->trans, bot, axis, radius, len);
   cyl->closed = 0;

   /* Compute bounding information */
   MakeVector(-1.0, -1.0, 0.0, object->o_bnd.lower_left);
   MakeVector(2.0, 2.0, 1.0, object->o_bnd.lengths);
   recompute_inverse_bbox(&object->o_bnd, &cyl->trans);
   object->o_data = (void *)cyl;
   return object;
}

void
Cylinder_Evaluater(Object *obj, float u, float v, Vertex *vert)
{
   Vec P, N, v0, v1;
   CylData *cyl = (CylData *)obj->o_data;
   Flt theta = TWO_PI * u;

   MakeVector(u, v, 0.0, vert->U);
   MakeVector( cos(theta), sin(theta), v, P);
   MakeVector(-sin(theta), cos(theta), 0.0, v0);
   MakeVector(0.0, 0.0, 1.0, v1);
   VecCross(v0, v1, N);
   InvTxVector(P, P, &cyl->trans);
   InvTxNormal(N, N, &cyl->trans);

   VecCopy(P, vert->P);
   if (obj->o_trans) {
      TxVector(P, P, obj->o_trans);
      TxNormal(N, N, obj->o_trans);
      }
   VecNormalize(N);
   VecCopy(P, vert->W);
   VecCopy(N, vert->N);
}

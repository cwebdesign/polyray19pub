/* disc.c

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
#include "disc.h"

/* This is a placeholder for primitive data */
typedef struct t_discdata {
   Vec center;
   Vec normal;
   Flt iradius, oradius;
   Flt iradius2, oradius2, d;
   } DiscData;

void Disc_Evaluater(Object *, float, float, Vertex *);
int DiscIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int DiscInside(Object *, Vec);
void DiscUV(Vec P, Vec N, Flt r0, Flt r1, Flt *u, Flt *v);

ObjectProcs DiscProcs = {
   GenericRender,
   Disc_Evaluater,
   GenericInitialize,
   DiscIntersect,
   DiscInside,
   GenericCopy,
   GenericDelete,
   };

void
DiscUV(Vec P, Vec N, Flt r0, Flt r1, Flt *u, Flt *v)
{
   Flt len, theta;
   Flt x, y;
   Vec v1, v2;

   /* Find vectors orthogonal to the axis */
   if (N[0] != 0.0) {
      MakeVector(-N[1], N[0], 0.0, v1);
      }
   else {
      MakeVector(0.0, -N[2], N[1], v1);
      }
   VecNormalize(v1);
   VecCross(N, v1, v2);
   VecNormalize(v2);

   x = VecDot(P, v1);
   y = VecDot(P, v2);

   len = sqrt(x * x + y * y);
   if (len == 0.0)
      theta = 0;
   else {
      if (y == 0.0)
         if (x > 0)
            theta = 0.0;
         else
            theta = M_PI;
      else {
         theta = acos(x / len);
         if (y < 0.0) theta = 2.0 * M_PI - theta;
         }
      }
   *u = theta / (2.0 * M_PI);
   *v = (len - r0) / (r1 - r0);
}

int
DiscIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
              Flt mindist, Flt maxdist, Isect *hit)
{
   Vec P, pos, U;
   Flt denom, dist, u, v;
   DiscData *disc = (DiscData *)obj->o_data;

   /* Do the normal ray-plane intersection */
   denom = VecDot(disc->normal, ray->D);
   if (fabs(denom) < EPSILON)
      return 0;
   dist = -(VecDot(disc->normal, ray->P) + disc->d) / denom;
   if (dist < mindist || dist > maxdist)
      return 0;

   /* Find distance between the intersection point of the discs plane
      and the center of the disc */
   VecAddScaled(ray->P, dist, ray->D, P);
   VecSub(P, disc->center, pos);
   denom = VecDot(pos, pos);
   if (denom <= disc->oradius2 && denom >= disc->iradius2) {
      DiscUV(pos, disc->normal, disc->iradius, disc->oradius, &u, &v);
      MakeVector(u, v, 0, U);
      Insert_Hit(obj, P, disc->normal, dist, U, hit);
      return 1;
      }
   return 0;
}

int
DiscInside(Object *obj, Vec Pos)
{
   Vec P;
   DiscData *disc = obj->o_data;
   Flt n;

   InvTxVector1(P, Pos, obj->o_trans)

   n = VecDot(P, disc->normal) + disc->d;
   return (n < 0 ? 1 : 0);
}

Object *
MakeDisc(Object *object, Vec c, Vec n, Flt ir, Flt or)
{
   DiscData *disc;

   object->o_type = T_DISC;
   object->o_procs = &DiscProcs;
   object->o_uv_steps[0] = 32;
   object->o_uv_steps[1] = 4;

   if (or < EPSILON || ir < 0 || VecNormalize(n) < EPSILON)
      error("Degenerate disc.\n");

   /* Attempt to allocate memory for this primitive */
   if ((disc = (DiscData *)polyray_malloc(sizeof(DiscData))) == NULL)
      error("Failed to allocate disc data\n");

   /* Set up the primitive specific information based on the
      input parameters */
   disc->iradius  = ir;
   disc->oradius  = or;
   disc->iradius2 = ir * ir;
   disc->oradius2 = or * or;
   VecNormalize(n);
   VecCopy(c, disc->center);
   VecCopy(n, disc->normal);
   disc->d = -VecDot(c, n);

   /* Compute bounding information - these bounds are really shitty */
   MakeVector(c[0]-or, c[1]-or, c[2]-or, object->o_bnd.lower_left);
   MakeVector(2.0 * or, 2.0 * or, 2.0 * or, object->o_bnd.lengths);

   object->o_data = (void *)disc;

   return object;
}

void
Disc_Evaluater(Object *obj, float u, float v, Vertex *vert)
{
   Flt theta, radius;
   Vec P, N;
   Vec v1, v2, v3, c;
   Flt r0, r1;
   DiscData *disc = (DiscData *)obj->o_data;

   MakeVector(u, v, 0.0, vert->U);

   VecCopy(disc->center, c);
   r0 = disc->iradius;
   r1 = disc->oradius;
   radius = v * (r1 - r0) + r0;

   /* Find vectors orthogonal to the axis */
   VecCopy(disc->normal, N);
   if (N[0] != 0.0) {
      MakeVector(-N[1], N[0], 0.0, v1);
      }
   else {
      MakeVector(0.0, -N[2], N[1], v1);
      }
   VecNormalize(v1);
   VecCross(N, v1, v2);
   VecNormalize(v2);

   /* Height and angle */
   theta  = TWO_PI * u;

   VecComb(cos(theta), v1, sin(theta), v2, v3);
   VecAddScaled(c, radius, v3, P);

   VecCopy(P, vert->P);
   if (obj->o_trans) {
      TxVector(P, P, obj->o_trans);
      TxNormal(N, N, obj->o_trans);
      }
   VecNormalize(N);
   VecCopy(P, vert->W);
   VecCopy(N, vert->N);
}

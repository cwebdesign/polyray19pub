/* parabola.c

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
#include "parabola.h"

/* Info needed to define a parabola. */
typedef struct t_parabdata {
   short int closed;
   Vec top, bot;
   Flt radius;
   Transform trans;
   } ParabolaData;


void ParabolaEvaluater(Object *, float, float, Vertex *);
int ParabolaIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int ParabolaNormal(ParabolaData *parabola, Vec Pos, Vec N);
void ParabolaUV(Vec Pos, Vec D, Flt t, Flt *u, Flt * v);
int ParabolaInside(Object *obj, Vec P);

ObjectProcs ParabolaProcs = {
   GenericRender,
   ParabolaEvaluater,
   GenericInitialize,
   ParabolaIntersect,
   ParabolaInside,
   GenericCopy,
   GenericDelete,
   };

int
ParabolaNormal(ParabolaData *parabola, Vec Pos, Vec N)
{
   Vec P;
   TxVector(P, Pos, &parabola->trans);
   P[2] = -0.5;
   InvTxNormal(N, P, &parabola->trans);
   return 1;
}

void
ParabolaUV(Vec Pos, Vec D, Flt t, Flt *u, Flt * v)
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
   *v = 1.0 - sqrt(ABS(P[2]));
}

int
ParabolaIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                  Flt mindist, Flt maxdist, Isect *hit)
{
   Flt t1, t2, a, b, c, u, v;
   Flt disc, zpos, dist, nmin, nmax;
   Vec P, PP, D, N, U;
   int Flag = 0;
   ParabolaData *parabola = (ParabolaData *)obj->o_data;

   /* Now transform to canonical parabola space */
   TxVector(P, ray->P, &parabola->trans);
   TxDirection(D, ray->D, &parabola->trans);
   dist = VecNormalize(D);
   nmin = mindist * dist;
   nmax = maxdist * dist;

   a = D[0] * D[0] + D[1] * D[1];
   b = D[0] * P[0] + D[1] * P[1] - D[2]/2.0;
   c = P[0] * P[0] + P[1] * P[1] - P[2];

   if (fabs(a) < EPSILON) {
      if (fabs(b) < EPSILON)
         /* No possible intersection */
         return 0;
      /* One intersection */
      t1 = -0.5 * c / b;
      zpos = P[2] + t1 * D[2];
      if (t1 < nmin || zpos > 1.0 || zpos < 0.0 || t1 > nmax)
         return 0;
      ParabolaUV(P, D, t1, &u, &v);
      t1 /= dist;
      VecAddScaled(ray->P, t1, ray->D, P);
      ParabolaNormal(parabola, P, N);
      MakeVector(u, v, 0, U);
      Insert_Hit(obj, P, N, t1, U, hit);
      return 1;
      }
   else {
      disc = b * b - a * c;
      if (disc < 0.0) return 0;
      disc = sqrt(disc);
      t1 = (-b + disc) / a;
      t2 = (-b - disc) / a;
      zpos = P[2] + t1 * D[2];
      if (t1 >= nmin && zpos >= 0.0 && zpos <= 1.0 && t1 <= nmax) {
         ParabolaUV(P, D, t1, &u, &v);
         t1 /= dist;
         VecAddScaled(ray->P, t1, ray->D, PP);
         ParabolaNormal(parabola, PP, N);
         MakeVector(u, v, 0, U);
         Insert_Hit(obj, PP, N, t1, U, hit);
         Flag = 1;
         }
      zpos = P[2] + t2 * D[2];
      if (t2 >= nmin && zpos >= 0.0 && zpos <= 1.0 && t2 <= nmax) {
         ParabolaUV(P, D, t2, &u, &v);
         t2 /= dist;
         VecAddScaled(ray->P, t2, ray->D, PP);
         ParabolaNormal(parabola, PP, N);
         MakeVector(u, v, 0, U);
         Insert_Hit(obj, PP, N, t2, U, hit);
         Flag = 1;
         }
      return Flag;
      }
}

int
ParabolaInside(Object *obj, Vec Pos)
{
   /* For csg purposes, treat the parabola as if it were
      capped at each end */
   Vec P;
   Flt w2, z2;
   ParabolaData *parabola = (ParabolaData *)obj->o_data;

   InvTxVector1(P, Pos, obj->o_trans)

   /* Transform to canonical parabola space */
   TxVector(P, P, &parabola->trans);
   w2 = P[0] * P[0] + P[1] * P[1];
   z2 = P[2];
   return (w2 < z2 && P[2] < 1.0 ? 1 : 0);
}

Object *
MakeParabola(Object *object, Vec bot, Vec top, Flt radius)
{
   Vec axis;
   Flt len;
   ParabolaData *parabola;

   object->o_type = T_PARABOLA;
   object->o_procs = &ParabolaProcs;
   object->o_uv_steps[0] = 32;
   object->o_uv_steps[1] = 16;

   /* Attempt to allocate memory for this primitive */
   if ((parabola =
         (ParabolaData *)polyray_malloc(sizeof(ParabolaData))) == NULL)
      error("Failed to allocate parabola data\n");

   /* Process the primitive specific information */
   if (radius < EPSILON)
      error("Degenerate parabola\n");
   /* Find the axis and axis length */
   VecSub(top, bot, axis);
   len = VecNormalize(axis);
   if (len < EPSILON)
      error("Degenerate parabola\n");
   VecCopy(bot, parabola->bot);
   VecCopy(top, parabola->top);
   parabola->radius = radius;
   VecNegate(bot);
   Get_Coordinate_Transform(&parabola->trans, bot, axis, radius, len);

   /* Compute bounding information */
   MakeVector(-1.0, -1.0, 0.0, object->o_bnd.lower_left);
   MakeVector(2.0, 2.0, 1.0, object->o_bnd.lengths);
   recompute_inverse_bbox(&object->o_bnd, &parabola->trans);

   object->o_data = (void *)parabola;
   return object;
}

void
ParabolaEvaluater(Object *obj, float u, float v, Vertex *vert)
{
   Flt theta, vt = 1.0 - v;
   Vec v0, v1, P, N;
   ParabolaData *par = (ParabolaData *)obj->o_data;

   MakeVector(u, v, 0.0, vert->U);

   theta  = TWO_PI * u;
   MakeVector(vt * cos(theta), vt * sin(theta), vt * vt, P);

   if (vt > EPSILON) {
      MakeVector(-vt * sin(theta), vt * cos(theta), 0.0, v0);
      MakeVector(cos(theta), sin(theta), 2.0 * vt, v1);
      VecCross(v0, v1, N);
      }
   else {
      MakeVector(0.0, 0.0, -1.0, N);
      }

   InvTxVector(P, P, &par->trans);
   InvTxNormal(N, N, &par->trans);

   VecCopy(P, vert->P);
   if (obj->o_trans) {
      TxVector(P, P, obj->o_trans);
      TxNormal(N, N, obj->o_trans);
      }
   VecNormalize(N);
   VecCopy(P, vert->W);
   VecCopy(N, vert->N);
}


/* cone.c

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
#include "cone.h"
#include "cylinder.h"

/* This is a placeholder for primitive data */
typedef struct t_conedata {
   short int closed;
   Vec top, bot;
   Flt trad, brad;
   Flt dist;
   Transform trans;
   } ConeData;

void Cone_Evaluater(Object *obj, float u, float v, Vertex *vert);
int ConeIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                  Flt mindist, Flt maxdist, Isect *hit);
int ConeNormal(ConeData *cone, Vec Pos, Vec N);
void ConeUV(Vec P, Vec D, Flt t, Flt d, Flt *u, Flt *v);
int ConeInside(Object *obj, Vec P);

ObjectProcs ConeProcs = {
   GenericRender,
   Cone_Evaluater,
   GenericInitialize,
   ConeIntersect,
   ConeInside,
   GenericCopy,
   GenericDelete,
   };

int
ConeNormal(ConeData *cone, Vec Pos, Vec N)
{
   Vec P;
   TxVector(P, Pos, &cone->trans);
   P[2] = -P[2];
   InvTxNormal(N, P, &cone->trans);
   return 1;
}

void
ConeUV(Vec Pos, Vec D, Flt t, Flt d, Flt *u, Flt * v)
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
   *u = 1.0 - theta / (2.0 * M_PI);
   *v = 1.0 - (P[2] - d) / (1.0 - d);
}

static int
check_cone_hit(Object *obj, ConeData *cone,
               Ray *ray, Vec P, Vec D, Flt t,
               Flt nmin, Flt nmax, Flt dist, Isect *hit)
{
   Vec PP, N, U;
   Flt u, v;

   if (t >= nmin && t <= nmax) {
      if ((Global_Shade_Flag & UV_CHECK) &&
          (obj->o_sflag & UV_CHECK)) {
         ConeUV(P, D, t, cone->dist, &u, &v);
         MakeVector(u, v, 0, U);
         }
      else
         VecCopy(P, U);
      t /= dist;
      VecAddScaled(ray->P, t, ray->D, PP);
      ConeNormal(cone, PP, N);
      return Insert_Hit(obj, PP, N, t, U, hit);
      }
   else
      return 0;
}

int
ConeIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
              Flt mindist, Flt maxdist, Isect *hit)
{
   Flt t1, t2, a, b, c;
   Flt disc, zpos, dist, nmin, nmax;
   Vec P, D;
   int Flag = 0;
   ConeData *cone = (ConeData *)obj->o_data;

   /* Now transform to canonical cone space */
   TxVector(P, ray->P, &cone->trans);
   TxDirection(D, ray->D, &cone->trans);
   dist = VecNormalize(D);
   nmin = mindist * dist;
   nmax = maxdist * dist;

   a = D[0] * D[0] + D[1] * D[1] - D[2] * D[2];
   b = D[0] * P[0] + D[1] * P[1] - D[2] * P[2];
   c = P[0] * P[0] + P[1] * P[1] - P[2] * P[2];

   if (fabs(a) < 1.0e-20) {
      if (fabs(b) < 1.0e-20)
         /* No possible intersection */
         return 0;
      /* One intersection */
      t1 = -0.5 * c / b;
      zpos = P[2] + t1 * D[2];
      if (zpos >= cone->dist && zpos <= 1.0 &&
          check_cone_hit(obj, cone, ray, P, D, t1,
                         nmin, nmax, dist, hit))
         return 1;
      else
         return 0;
      }
   else {
      disc = b * b - a * c;
      if (disc < 0.0) return 0;
      disc = sqrt(disc);
      t1 = (-b + disc) / a;
      t2 = (-b - disc) / a;
      zpos = P[2] + t1 * D[2];
      if (zpos >= cone->dist && zpos <= 1.0 &&
          check_cone_hit(obj, cone, ray, P, D, t1,
                         nmin, nmax, dist, hit))
         Flag = 1;
      zpos = P[2] + t2 * D[2];
      if (zpos >= cone->dist && zpos <= 1.0 &&
          check_cone_hit(obj, cone, ray, P, D, t2,
                         nmin, nmax, dist, hit))
         Flag = 1;
      }
   return Flag;
}

int
ConeInside(Object *obj, Vec Pos)
{
   /* For csg purposes, treat the cone as if it were
      capped at each end */
   Vec P;
   Flt w2, z2;
   ConeData *cone = (ConeData *)obj->o_data;

   InvTxVector1(P, Pos, obj->o_trans)

   /* Transform to canonical cone space */
   TxVector(P, P, &cone->trans);
   w2 = P[0] * P[0] + P[1] * P[1];
   z2 = P[2] * P[2];
   return ((w2 < z2 && P[2] > cone->dist && P[2] < 1.0) ? 1 : 0);
}

Object *
MakeCone(Object *object, Vec bot, Flt brad, Vec top, Flt trad)
{
   Flt cottheta, lprime, tlen, len, tmpf;
   Vec axis, base, tmpv;
   ConeData *cone;

   object->o_type  = T_CONE;
   object->o_procs = &ConeProcs;
   object->o_uv_steps[0] = 16;
   object->o_uv_steps[1] = 2;

   /* Attempt to allocate memory for this primitive */
   if ((cone = (ConeData *)polyray_malloc(sizeof(ConeData))) == NULL)
      error("Failed to allocate cone data\n");

   /* Store the parameters of this cone */
   VecCopy(bot, cone->bot);
   cone->brad = brad;
   VecCopy(top, cone->top);
   cone->trad = trad;

   /* By default cones have open ends */
   cone->closed = 0;

   /* Process the primitive specific information */
   if(trad < brad) {
      /* Want the bigger end at the top */
      VecCopy(bot, tmpv);
      VecCopy(top, bot);
      VecCopy(tmpv, top);
      tmpf = brad;
      brad = trad;
      trad = tmpf;
      }
   else if (equal(trad, brad)) {
      /* Quietly change this cone into a cylinder */
      return MakeCylinder(object, bot, top, trad);
      }
   /* Find the axis and axis length */
   VecSub(top, bot, axis);
   len = VecNormalize(axis);
   if (len < EPSILON)
      error("Degenerate cone\n");
   /* Determine alignment */
   cottheta = len / (trad - brad);
   lprime = brad * cottheta;
   base[0] = lprime * axis[0];
   base[1] = lprime * axis[1];
   base[2] = lprime * axis[2];
   VecSub(base, bot, base);
   tlen = lprime + len;
   cone->dist = lprime / tlen;
   Get_Coordinate_Transform(&cone->trans, base, axis, trad, tlen);

   /* Compute bounding information */
   MakeVector(-1.0, -1.0, cone->dist, object->o_bnd.lower_left);
   MakeVector(2.0, 2.0, 1.0-cone->dist, object->o_bnd.lengths);
   recompute_inverse_bbox(&object->o_bnd, &cone->trans);

   object->o_data = (void *)cone;
   return object;
}

void
Cone_Evaluater(Object *obj, float u, float v, Vertex *vert)
{
   Vec P, N, v0, v1;
   ConeData *cone = (ConeData *)obj->o_data;
   Flt vt, theta;

   vt = cone->dist + (1.0 - v) * (1.0 - cone->dist);
   theta = TWO_PI * (1.0 - u);
   MakeVector(u, v, 0.0, vert->U);

   MakeVector(vt * cos(theta), vt * sin(theta), vt, P);
   if (vt > EPSILON) {
      MakeVector(-vt * sin(theta), vt * cos(theta), 0.0, v0);
      MakeVector(cos(theta), sin(theta), 1.0, v1);
      VecCross(v0, v1, N);
      }
   else {
      MakeVector(0.0, 0.0, -1.0, N);
      }
   InvTxVector(P, P, &cone->trans);
   InvTxNormal(N, N, &cone->trans);

   VecCopy(P, vert->P);
   if (obj->o_trans) {
      TxVector(P, P, obj->o_trans);
      TxNormal(N, N, obj->o_trans);
      }
   VecNormalize(N);
   VecCopy(P, vert->W);
   VecCopy(N, vert->N);
}

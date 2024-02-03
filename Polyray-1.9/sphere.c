/* sphere.c

   Processing for sphere primitives

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
#include "sphere.h"

typedef struct t_spheredata {
   Vec sph_center;
   Flt sph_radius;
   Flt sph_radius2;
   } SphereData;

void Ellipse_Evaluater(Object *, float, float, Vertex *);
int SphereIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int SphereInside(Object *obj, Vec P);
void SphereUV(Vec Pos, Vec C, Flt *u, Flt *v);

ObjectProcs SphereProcs = {
   GenericRender,
   Ellipse_Evaluater,
   GenericInitialize,
   SphereIntersect,
   SphereInside,
   GenericCopy,
   GenericDelete,
   };

static int
SphereNormal(SphereData *sp, Vec P, Vec N)
{
   VecSub(P, sp->sph_center, N);
   return 1;
}

void
SphereUV(Vec Pos, Vec C, Flt *u, Flt *v)
{
   Flt len, phi, theta, scp;
   Flt x, y, z;
   Vec P;

   VecSub(Pos, C, P);
   /* Make sure this vector is on the unit sphere. */
   len = VecLen(P);
   if (len < EPSILON) {
/*      *u = 0.0;
      *v = 0.0; */
      *u = *v = 0.0;
      return;
      }
   else {
      scp = RECIPROCAL(len);
      x = P[0] * scp; // / len;
      y = P[1] * scp; // / len;
      z = P[2] * scp; // / len;
      /* try scp = 1/len; x=scp*P[0];y=scp*P[1];z=scp*P[2]; */
      }
   /* Determine its angle from the x-z plane. */
   phi = asin(y);
   
   /* Determine its angle from the point (1, 0, 0) in the x-z plane. */
   len = sqrt(x * x + z * z);
#if 1
   theta = atan2(z, x) + M_PI;
#else
   if (len == 0.0) {
      /* This point is at one of the poles. Any value of xcoord will be ok...*/
      theta = 0;
      }
   else {
      if (z == 0.0)
         if (x > 0)
            theta = 0.0;
         else
            theta = M_PI;
      else {
         theta = acos(x / len);
         if (z < 0.0) theta = 2.0 * M_PI - theta;
         }
      }
#endif
   *u = 1.0 - theta / (2.0 * M_PI);
   *v = 0.5 + (phi / M_PI);
}

int
SphereIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                Flt mindist, Flt maxdist, Isect *hit)
{
   Flt b, disc, t, u, v;
   Vec V, P, N, U;
   SphereData *sp = (SphereData *)obj->o_data;
   int Flag = 0;

   VecSub(sp->sph_center, ray->P, V);
   b = VecDot(V, ray->D);
   disc = b * b - VecDot(V, V) + sp->sph_radius2;
   if (disc < 0.0) return 0;
   disc = sqrt(disc);
   t = b - disc ;
   if (t > mindist && t < maxdist) {
      VecAddScaled(ray->P, t, ray->D, P);
      if ((Global_Shade_Flag & UV_CHECK) &&
          (obj->o_sflag & UV_CHECK)) {
         SphereUV(P, sp->sph_center, &u, &v);
         MakeVector(u, v, 0, U);
         }
      else
         VecCopy(P, U);
      SphereNormal(sp, P, N);
      Insert_Hit(obj, P, N, t, U, hit);
      Flag = 1;
      }
   t = b + disc;
   if (t > mindist && t < maxdist) {
      VecAddScaled(ray->P, t, ray->D, P);
      if ((Global_Shade_Flag & UV_CHECK) &&
          (obj->o_sflag & UV_CHECK)) {
         SphereUV(P, sp->sph_center, &u, &v);
         MakeVector(u, v, 0, U);
         }
      else
         VecCopy(P, U);
      SphereNormal(sp, P, N);
      Insert_Hit(obj, P, N, t, U, hit);
      Flag = 1;
      }
   return Flag;
}

int
SphereInside(Object *obj, Vec Pos)
{
   SphereData *sp;
   Vec P, D;
   Flt d;

   InvTxVector1(P, Pos, obj->o_trans)
   sp = (SphereData *)obj->o_data;
   VecSub(P, sp->sph_center, D);
   d = VecDot(D, D);
   return ((d < sp->sph_radius2) ? 1 : 0);
}

Object *
MakeSphere(Object *object, Vec pos, Flt radius)
{
   int i ;
   SphereData *sp;

   object->o_type = T_SPHERE;
   object->o_procs = &SphereProcs;
   object->o_uv_steps[0] = 32;
   object->o_uv_steps[1] = 16;

   sp = (SphereData *)polyray_malloc(sizeof(SphereData));
   if (sp == NULL)
      error("Failed to allocate sphere data\n");
   VecCopy(pos, sp->sph_center);
   sp->sph_radius = radius;
   sp->sph_radius2 = radius * radius;

   /* Compute bounding information */
//   for (i=0;i<3;i++) {
   for (i=3; i--; ) {
      object->o_bnd.lower_left[i] = sp->sph_center[i] - sp->sph_radius;
      object->o_bnd.lengths[i] = 2.0 * sp->sph_radius;
      }

   object->o_data = (void *)sp;
   return object;
}

void
Ellipse_Evaluater(Object *obj, float u, float v, Vertex *vert)
{
   Flt theta, phi;
   Flt radius;
   Vec P, N;
   SphereData *sp = (SphereData *)obj->o_data;

   MakeVector(u, v, 0.0, vert->U);

   radius = sp->sph_radius;

   theta = (TWO_PI * v - M_PI) / 2.0;
   phi   = TWO_PI * (1.0 - u);

   N[0] = cos(phi) * cos(theta);
   N[1] = sin(theta);
   N[2] = sin(phi) * cos(theta);
   VecAddScaled(sp->sph_center, radius, N, P);

   VecCopy(P, vert->P);
   if (obj->o_trans) {
      TxVector(P, P, obj->o_trans);
      TxNormal(N, N, obj->o_trans);
      }
   VecNormalize(N);
   VecCopy(P, vert->W);
   VecCopy(N, vert->N);
}


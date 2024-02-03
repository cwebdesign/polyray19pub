/*
  poly.c

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
#include "poly.h"
#include "vector.h"
#include "roots.h"

typedef struct t_polydata {
   int poly_npoints;
   fVec *poly_point;
   fVec poly_normal;
   float poly_d;
   short poly_u, poly_v;
   } PolyData;

void PolyRender(Viewpoint *, BinTree *, Object *);
int PolyIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int PolyInside(Object *, Vec);
void PolyDelete(Object *);

ObjectProcs PolyProcs = {
   PolyRender,
   NULL,
   GenericInitialize,
   PolyIntersect,
   PolyInside,
   GenericCopy,
   PolyDelete,
   };

/***********************************************************************
 * PolyIntersect(obj, ray, hit)
 *
 * returns 1 if we hit the polygon, with the hit information in hit.
 * Uses a version of Jordan's theorem to determine whether the point
 * is inside the polygon.
 * The variable "crosses" will count the number of times that we
 * cross the boundary of the curve.  If it is odd, we are inside.
 *
 ***********************************************************************/
int
PolyIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
              Flt mindist, Flt maxdist, Isect *hit)
{
   int u, v;
   Flt n, d, t;
   Vec V, U, N;
   PolyData *pd = (PolyData *) obj->o_data;

   d = VecDot(ray->D, pd->poly_normal);
   if (fabs(d) < EPSILON) return 0;
   n = VecDot(ray->P, pd->poly_normal) + pd->poly_d;
   t = -n / d;
   if (t < mindist || t > maxdist)
      return 0;
   VecAddScaled(ray->P, t, ray->D, V);
   u = pd->poly_u;
   v = pd->poly_v;

   if (Inside_Polygon(V[u], V[v], pd->poly_npoints, pd->poly_point, u, v)) {
      MakeVector(V[u], V[v], 0, U);
      VecCopy(pd->poly_normal, N);
      Insert_Hit(obj, V, N, t, U, hit);
      return 1;
      }
   else
      return 0;
}

int
PolyInside(Object *obj, Vec Pos)
{
   Vec P;
   PolyData *pd;
   Flt n;

   InvTxVector1(P, Pos, obj->o_trans)

   pd = (PolyData *)obj->o_data;
   n = VecDot(P, pd->poly_normal) + pd->poly_d;
   return (n < 0 ? 1 : 0);
}

Object *
MakePoly(Object *object, int npoints, fVec *points)
{
   PolyData * pd;
   Vec P1, P2, N, mins, maxs;
   Flt t;
   int i, j;

   object->o_type = T_POLY;
   object->o_procs = &PolyProcs;

   pd = (PolyData *)polyray_malloc(sizeof(PolyData));
   if (pd == NULL)
      error("Failed to allocate polygon data\n");
   pd->poly_npoints = npoints;
   pd->poly_point = (fVec *)polyray_malloc(npoints * sizeof(fVec));
   if (pd->poly_point == NULL)
      error("Failed to allocate polygon data\n");
   for (i=0;i<npoints;i++)
      VecCopy(points[i], pd->poly_point[i])
   polyray_free(points);

   /* calculate the normal by giving various cross products */
   VecSub(pd->poly_point[1], pd->poly_point[0], P1);
   VecSub(pd->poly_point[2], pd->poly_point[0], P2);

   VecCross(P1, P2, N);
   VecNormalize(N);
   VecCopy(N, pd->poly_normal);

   if (fabs(pd->poly_normal[0]) >= fabs(pd->poly_normal[1])
      && fabs(pd->poly_normal[0]) >= fabs(pd->poly_normal[2])) {
      pd->poly_u = 1;
      pd->poly_v = 2;
      }
   else if (fabs(pd->poly_normal[1]) >= fabs(pd->poly_normal[0])
      && fabs(pd->poly_normal[1]) >= fabs(pd->poly_normal[2])) {
      pd->poly_u = 0;
      pd->poly_v = 2;
      }
   else {
      pd->poly_u = 0;
      pd->poly_v = 1;
      }

   pd->poly_d = -VecDot(pd->poly_normal, pd->poly_point[0]);

   /* Compute bounding information */
   for (i=0;i<3;i++) {
      mins[i] = pd->poly_point[0][i] - EPSILON;
      maxs[i] = pd->poly_point[0][i] + EPSILON;
      }
   for (i=1;i<npoints;i++) {
      for (j=0;j<3;j++) {
         t = pd->poly_point[i][j];
         if (t < mins[j]) mins[j] = t;
         if (t > maxs[j]) maxs[j] = t;
         }
      }
   VecCopy(mins, object->o_bnd.lower_left);
   VecSub(maxs, mins, object->o_bnd.lengths);

   object->o_data = (void *)pd;
   return object;
}

void PolyDelete(Object *object)
{
   PolyData *pd = (PolyData *)object->o_data;
   if (object->o_copy == 0) {
      polyray_free(pd->poly_point);
      polyray_free(object->o_data);
      }
}

void
PolyRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   fVec *verts;
   int **out_verts;
   Vertex *vertex;
   int i, j;
   int out_n, npoints;
   Poly Polygon;
   Vec P, N;
   PolyData *pol = (PolyData *)obj->o_data;

   npoints = pol->poly_npoints;

   /* Allocate space to hold the intermediate polygon stacks */
   verts = (fVec *)polyray_malloc(npoints * sizeof(fVec));
   out_verts = (int **)polyray_malloc((npoints - 2) * sizeof(int *));
   if (verts == NULL || out_verts == NULL)
      error("Insufficient memory to split polygon");
   for (i=0;i<npoints;i++)
      VecCopy(pol->poly_point[i], verts[i])
   for (i=0;i<npoints-2;i++) {
      out_verts[i] = (int *)polyray_malloc(3 * sizeof(int));
      if (out_verts[i] == NULL)
         error("Insufficient memory to split polygon");
      }

   Split_Polygon(npoints, verts, pol->poly_u, pol->poly_v,
                 &out_n, out_verts);

   /* Now output the triangles that we generated */
   for (i=0;i<out_n;i++) {
      Polygon.n = 3;
      for (j=0;j<3;j++) {
         vertex = &Polygon.vertices[j];
         VecCopy(verts[out_verts[i][j]], P);
         VecCopy(pol->poly_normal, N);
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

   /* Free the temporary polygon storage */
   for (i=0;i<npoints-2;i++)
      polyray_free(out_verts[i]);
   polyray_free(out_verts);
   polyray_free(verts);
}

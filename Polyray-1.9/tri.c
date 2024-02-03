/* tri.c

  Processing for triangles that have associated vertex normals

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
#include "tri.h"

int InvertMatrix(fVec in[3], fVec out[3]);

typedef struct t_triangledata {
   fVec tri_P[3];
   fVec tri_N[3];
   float tri_u[3];
   float tri_v[3];
   fVec tri_bb[3];
   } TriData;

void TriRender(Viewpoint *, BinTree *, Object *);
int TriIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int TriInside(Object *, Vec);

ObjectProcs TriProcs = {
   TriRender,
   NULL,
   GenericInitialize,
   TriIntersect,
   TriInside,
   GenericCopy,
   GenericDelete,
   };

int
TriIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
             Flt mindist, Flt maxdist, Isect *hit)
{
   TriData *td = (TriData *)obj->o_data;
   Flt n, d, dist;
   Flt r, s, t;
   Flt a, b, u, v;
   Vec P, Q, N, U;

   /*
    * The matrix td -> tri_bb transforms vectors in the world 
    * space into a space with the following properties.
    *
    * 1.  The sides of the triangle are coincident with the
    *     x and y axis, and have unit length.
    * 2.  The normal to the triangle is coincident with the 
    *     z axis.
    *
    */

   /*
    * d is the slope with respect to the z axis.  If d is zero, then
    * the ray is parallel to the plane of the polygon, and we count 
    * it as a miss...
    */
   d = VecDot(ray->D, td->tri_bb[2]);
   if (fabs(d) < EPSILON)
      return 0;

   /*
    * Q is a vector from the eye to the triangles "origin" vertex.
    * n is then set to be the distance of the tranformed eyepoint
    * to the plane in the polygon.
    * Together, n and d allow you to find the distance to the polygon, 
    * which is merely n / d.
    */
   VecSub(td->tri_P[0], ray->P, Q);
   n = VecDot(Q, td->tri_bb[2]);
   dist = n / d;

   if (dist < mindist) 
      return 0 ;
   
   /* 
    * Q is the point we hit.  Find its position relative to the
    * origin of the triangle.
    */
   VecAddScaled(ray->P, dist, ray->D, P);
   VecSub(P, td -> tri_P[0], Q);

   a = VecDot(Q, td->tri_bb[0]);
   b = VecDot(Q, td->tri_bb[1]);

   if (a < -EPSILON || b < -EPSILON || a + b > 1.0+EPSILON)
      return 0;
   
   r = 1.0 - a - b;
   s = a;
   t = b;

   if (dist > mindist && dist < maxdist) {
      MakeVector(0.0, 0.0, 0.0, N);
      VecAddS(r, td->tri_N[0], N, N);
      VecAddS(s, td->tri_N[1], N, N);
      VecAddS(t, td->tri_N[2], N, N);
      VecNormalize(N);
      u = r * td->tri_u[0] + s * td->tri_u[1] + t * td->tri_u[2];
      v = r * td->tri_v[0] + s * td->tri_v[1] + t * td->tri_v[2];
      MakeVector(u, v, 0, U);
      Insert_Hit(obj, P, N, dist, U, hit);
      return 1;
      }
   else
      return 0;
}

int
TriInside(Object *obj, Vec P)
{
   return 0;
}

Object *
MakeTri(Object *object, UVVert *tri_vert)
{
   TriData *td;
   Vec maxs, mins;
   int i;
   Flt len;
   fVec B[3];

   object->o_type = T_TRI ;
   object->o_procs = &TriProcs;

   td = (TriData *)polyray_malloc(sizeof(TriData)) ;
   if (td == NULL)
      error("Failed to allocate triangle data\n");

   /* Copy the points into the patch */
   for (i=0;i<3;i++) {
      VecCopy(tri_vert[i].pos,  td->tri_P[i]);
      fVecNormalize(tri_vert[i].norm);
      VecCopy(tri_vert[i].norm, td->tri_N[i]);
      td->tri_u[i] = tri_vert[i].u;
      td->tri_v[i] = tri_vert[i].v;
      }

   /* Correct the u/v if necessary */
   if (td->tri_u[0] == PLY_HUGE) td->tri_u[0] = 0.0;
   if (td->tri_v[0] == PLY_HUGE) td->tri_v[0] = 0.0;
   if (td->tri_u[1] == PLY_HUGE) td->tri_u[1] = 1.0;
   if (td->tri_v[1] == PLY_HUGE) td->tri_v[1] = 0.0;
   if (td->tri_u[2] == PLY_HUGE) td->tri_u[2] = 0.0;
   if (td->tri_v[2] == PLY_HUGE) td->tri_v[2] = 1.0;

   /*
    * construct the inverse of the matrix...
    * | P1 |
    * | P2 |
    * | N  |
    * and store it in td -> tri_bb[]
    */
   
   VecSub(td->tri_P[1], td->tri_P[0], B[0]);
   VecSub(td->tri_P[2], td->tri_P[0], B[1]);
   VecCross(B[0], B[1], B[2]);

   len = sqrt(VecDot(B[2], B[2]));
   if (len == 0.0)
      MakeVector(1.0, 0.0, 0.0, B[2])
   else {
      len = 1.0 / len;
      VecScale(len, B[2]);
      }

   InvertMatrix(B, td->tri_bb);

   /* Compute bounding information */
   for (i=0;i<3;i++) {
      mins[i] = MIN(td->tri_P[0][i], MIN(td->tri_P[1][i], td->tri_P[2][i]));
      maxs[i] = MAX(td->tri_P[0][i], MAX(td->tri_P[1][i], td->tri_P[2][i]));
      }
   VecCopy(mins, object->o_bnd.lower_left);
   VecSub(maxs, mins, object->o_bnd.lengths);
   for (i=0;i<3;i++) {
      object->o_bnd.lower_left[i] -= EPSILON + EPSILON;
      object->o_bnd.lengths[i] += 4.0 * EPSILON;
      }
   object->o_data = (void *)td;
   return object;
}

int
InvertMatrix(fVec in[3], fVec out[3])
{
   int i, j;
   Flt det;

   out[0][0] =  (in[1][1] * in[2][2] - in[1][2] * in[2][1]);
   out[1][0] = -(in[0][1] * in[2][2] - in[0][2] * in[2][1]);
   out[2][0] =  (in[0][1] * in[1][2] - in[0][2] * in[1][1]);

   out[0][1] = -(in[1][0] * in[2][2] - in[1][2] * in[2][0]);
   out[1][1] =  (in[0][0] * in[2][2] - in[0][2] * in[2][0]);
   out[2][1] = -(in[0][0] * in[1][2] - in[0][2] * in[1][0]);

   out[0][2] =  (in[1][0] * in[2][1] - in[1][1] * in[2][0]);
   out[1][2] = -(in[0][0] * in[2][1] - in[0][1] * in[2][0]);
   out[2][2] =  (in[0][0] * in[1][1] - in[0][1] * in[1][0]);
   
   det = in[0][0] * in[1][1] * in[2][2] +
         in[0][1] * in[1][2] * in[2][0] +
         in[0][2] * in[1][0] * in[2][1] -
         in[0][2] * in[1][1] * in[2][0] -
         in[0][0] * in[1][2] * in[2][1] -
         in[0][1] * in[1][0] * in[2][2];

   if (fabs(det) < EPSILON)
      return 0;

   det = 1 / det;

   for (i=0;i<3;i++)
      for (j=0;j<3;j++)
         out[i][j] *= det;
   return 1;
}

void
TriRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   int i;
   Vec P, N;
   TriData *tri = (TriData *)obj->o_data;
   Poly Polygon;
   Object *tobj;

   Polygon.n = 3;
   for (i=0;i<3;i++) {
      P[0] = tri->tri_P[i][0];
      P[1] = tri->tri_P[i][1];
      P[2] = tri->tri_P[i][2];
      N[0] = tri->tri_N[i][0];
      N[1] = tri->tri_N[i][1];
      N[2] = tri->tri_N[i][2];

      VecCopy(P, Polygon.vertices[i].P);
      MakeVector(tri->tri_u[i], tri->tri_v[i], 0.0,
                 Polygon.vertices[i].U);

      tobj = obj;
      if (tobj->o_trans) {
         TxVector(P, P, obj->o_trans);
         TxNormal(N, N, obj->o_trans);
         }

      VecCopy(P, Polygon.vertices[i].W);
      VecCopy(N, Polygon.vertices[i].N);
      }
   scan_convert(eye, Root, obj, NULL, &Polygon);
}

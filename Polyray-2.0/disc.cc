/* disc.cc

  Polyray - MIT Licensed Revival
  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Copyright (C) 1999-2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", (C), WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#include "defs3.h"
#include "memory.h"
#include "io_ply.h"
#include "intersec.h"
#include "symtab.h"
#include "scan.h"
#include "vector.h"
#include "bound.h"
#include "disc.h"
#include "factory.h"



void Disc_Evaluator(Object *, Flt, Flt, Vertex *);
int DiscIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int DiscInside(Object *, Vec);
void DiscUV(Vec P, Vec N, Flt r0, Flt r1, Flt *u, Flt *v);

openpolyray::dispatch::ObjectProcs DiscProcs = {
   .render = GenericRender,
   .evaluate = Disc_Evaluator,
   .initialize = GenericInitialize,
   .intersect = DiscIntersect,
   .inside = DiscInside,
   .copy = GenericCopy,
   .del = DiscDelete,
   };

 /**
 * @brief Release the heap payload owned by a box primitive.
 *
 * @param object Disc object whose `DiscData` payload should be destroyed.
 * @return No return value.
 */
void DiscDelete(Object *object)
{
   if (object->o_copy != 0 || object->o_data == nullptr)
      return;

   delete static_cast<DiscData *>(object->o_data);
   object->o_data = nullptr;
}

/**
 * Compute disc texture coordinates for a point relative to the disc center.
 * @param P Point on the disc expressed relative to the disc center.
 * @param N Unit disc normal.
 * @param r0 Inner disc radius.
 * @param r1 Outer disc radius.
 * @param u Receives the computed angular texture coordinate.
 * @param v Receives the computed radial texture coordinate.
 * @return No return value.
 */
void DiscUV(Vec P, Vec N, Flt r0, Flt r1, Flt *u, Flt *v)
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
            theta = PYM_PI;
      else {
         theta = acos(x / len);
         if (y < 0.0) theta = 2.0 * PYM_PI - theta;
         }
      }
   *u = theta / (2.0 * PYM_PI);
   *v = (len - r0) / (r1 - r0);
}

/**
 * Intersect a ray with a disc primitive.
 * @param Eye Viewpoint issuing the ray.
 * @param obj Disc object being tested.
 * @param ray Ray to intersect.
 * @param mindist Minimum valid hit distance.
 * @param maxdist Maximum valid hit distance.
 * @param hit Intersection list that receives accepted hits.
 * @return `1` when a disc hit is recorded, otherwise `0`.
 */
int DiscIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
              Flt mindist, Flt maxdist, Isect *hit)
{
   Vec P, pos, U;
   Flt denom, dist, u, v;
   //DiscData *
   auto disc = (DiscData *)obj->o_data;

   /* Do the normal ray-plane intersection */
   denom = VecDot(disc->normal, ray->D);
   if (fabs(denom) < PLY_EPSILON)
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

/**
 * Test whether a point lies on the negative side of the disc plane.
 * @param obj Disc object being queried.
 * @param Pos World-space point to test.
 * @return `1` when the point is inside according to the disc half-space test, otherwise `0`.
 */
int DiscInside(Object *obj, Vec Pos)
{
   Vec P;
   DiscData *disc = (DiscData*)obj->o_data;
   Flt n;

   InvTxVector1(P, Pos, obj->o_trans)

   n = VecDot(P, disc->normal) + disc->d;
   return (n < 0 ? 1 : 0);
}

/**
 * Initialize an object as a disc primitive.
 * @param object Object being configured as a disc.
 * @param c Disc center.
 * @param n Disc normal, normalized in place.
 * @param ir Inner radius.
 * @param _or Outer radius.
 * @return `object` configured as a disc primitive.
 */
Object *MakeDisc(Object *object, Vec c, Vec n, Flt ir, Flt _or)
{

   object->o_type = ShapeType::Disc;
   object->o_procs = &DiscProcs;
   object->o_uv_steps[0] = 32;
   object->o_uv_steps[1] = 4;

   if (_or < PLY_EPSILON || ir < 0 || VecNormalize(n) < PLY_EPSILON)
      serror("Degenerate disc.\n");

   // Attempt to allocate memory for this primitive 
   auto disc = FactoryDiscData();

   /* Set up the primitive specific information based on the
      input parameters */
   disc->iradius  = ir;
   disc->oradius  = _or;
   disc->iradius2 = ir * ir;
   disc->oradius2 = _or * _or;
   VecNormalize(n);
   VecCopy(c, disc->center);
   VecCopy(n, disc->normal);
   disc->d = -VecDot(c, n);

   /* Compute bounding information - these bounds are really shitty */
   MakeVector(c[0]-_or, c[1]-_or, c[2]-_or, object->o_bnd.lower_left);
   MakeVector(2.0 * _or, 2.0 * _or, 2.0 * _or, object->o_bnd.lengths);

   object->o_data = (void *)disc;

   return object;
}

/**
 * Sample a disc surface point and normal from parametric UV coordinates.
 * @param obj Disc object being evaluated.
 * @param u Angular parameter on the disc surface.
 * @param v Radial parameter on the disc surface.
 * @param vert Receives the sampled position, normal, and texture coordinates.
 * @return No return value.
 */
void Disc_Evaluator(Object *obj, Flt u, Flt v, Vertex *vert)
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

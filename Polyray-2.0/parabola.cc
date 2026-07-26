/** @file parabola.cc
 *  @brief Paraboloid primitive intersection, normal, UV, and tessellation.
 *
 *  The canonical paraboloid is the surface x^2 + y^2 = z for z in [0, 1].
 *  All intersection and normal computations transform the ray into this
 *  canonical space via the pre-computed Transform stored in ParabolaData.
 *
 *  Polyray  MIT Licensed Revival
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
#include "io_ply.h"
#include "memory.h"
#include "intersec.h"
#include "symtab.h"
#include "scan.h"
#include "vector.h"
#include "bound.h"
#include "parabola.h"
#include "factory.h"




void ParabolaEvaluator(Object *, Flt, Flt, Vertex *);
int ParabolaIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int ParabolaNormal(ParabolaData *parabola, Vec Pos, Vec N);
void ParabolaUV(Vec Pos, Vec D, Flt t, Flt *u, Flt * v);
int ParabolaInside(Object* obj, Vec P);

openpolyray::dispatch::ObjectProcs ParabolaProcs = {
   .render = GenericRender,
   .evaluate = ParabolaEvaluator,
   .initialize = GenericInitialize,
   .intersect = ParabolaIntersect,
   .inside = ParabolaInside,
   .copy = GenericCopy,
   .del = ParabolaDelete,
   };

/**
 * @brief Release the heap payload owned by a paraboloid primitive.
 *
 * @param object Paraboloid object whose `ParabolaData` payload should be freed.
 * @return No return value.
 */
void ParabolaDelete(Object *object)
{
   if (object->o_copy != 0 || object->o_data == nullptr)
      return;

   delete static_cast<ParabolaData *>(object->o_data);
   object->o_data = nullptr;
}

/** @brief Compute the outward surface normal of a paraboloid at a world-space point.
 *
 *  Transforms @p Pos into canonical paraboloid space, evaluates the analytic
 *  gradient (x, y, ?1/2), then maps it back to world space via the inverse transpose.
 *  @param parabola  Paraboloid shape data containing the canonical transform.
 *  @param Pos       World-space point on the surface.
 *  @param N         Output: un-normalised world-space normal.
 *  @return          Always 1.
 */
int ParabolaNormal(ParabolaData *parabola, Vec Pos, Vec N)
{
   Vec P;
   TxVector(P, Pos, &parabola->trans);
   P[2] = -0.5;
   InvTxNormal(N, P, &parabola->trans);
   return 1;
}

/** @brief Compute UV texture coordinates for a point on the paraboloid.
 *
 *  Evaluates the canonical-space hit point as @c Pos + t*D, then inverts the
 *  parameterisation used by ParabolaEvaluator():
 *  - @c u is the azimuthal angle about the +Z axis, normalised to [0, 1)
 *  - @c v = 1 - sqrt(z), so @c v = 0 at the base rim and @c v = 1 at the apex
 *  @param Pos  Ray origin in canonical paraboloid space.
 *  @param D    Ray direction in canonical paraboloid space.
 *  @param t    Canonical-space ray parameter at the hit point.
 *  @param u    Output: azimuthal texture coordinate in [0, 1).
 *  @param v    Output: height texture coordinate in [0, 1].
 */
void
ParabolaUV(Vec Pos, Vec D, Flt t, Flt *u, Flt * v)
{
   Flt x, len, theta;
   Vec P;
   
   VecAddScaled(Pos, t, D, P);
   len = sqrt(P[0] * P[0] + P[1] * P[1]);
   /* Make sure this vector is on the unit cylinder. */
   if (len < PLY_EPSILON)
      theta = 0;
   else {
      x  = P[0] / len;
      if (P[1] == 0.0)
         if (x > 0)
            theta = 0.0;
         else
            theta = PYM_PI;
      else {
         theta = acos(x);
         if (P[1] < 0.0)
            theta = (2.0 * PYM_PI) - theta;
         }
      }
   *u = theta / (2.0 * PYM_PI);
   *v = 1.0 - sqrt(ABS(P[2]));
}

/** @brief Find ray-paraboloid intersections and record hit records.
 *
 *  Transforms the ray into canonical paraboloid space (x^2 + y^2 = z, z in [0,1])
 *  and solves the resulting quadratic.  Both roots are tested and inserted
 *  via Insert_Hit() when they fall within [mindist, maxdist] and on the surface.
 *  @param Eye      Active viewpoint (unused directly; passed through).
 *  @param obj      Paraboloid object.
 *  @param ray      Incoming ray in world space.
 *  @param mindist  Minimum valid ray parameter (near clip).
 *  @param maxdist  Maximum valid ray parameter (far clip).
 *  @param hit      Output: intersection accumulator.
 *  @return         1 if at least one valid intersection was found, 0 otherwise.
 */
int ParabolaIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
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

   if (fabs(a) < PLY_EPSILON) {
      if (fabs(b) < PLY_EPSILON)
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

/** @brief Test whether a world-space point lies inside the paraboloid volume.
 *
 *  For CSG purposes the paraboloid is treated as a capped solid: the interior
 *  is the region where x^2 + y^2 < z and z < 1 in canonical space.
 *  @param obj  Paraboloid object (provides transform and shape data).
 *  @param Pos  World-space point to test.
 *  @return     1 if the point is inside the capped paraboloid, 0 otherwise.
 */
int ParabolaInside(Object *obj, Vec Pos)
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

/** @brief Initialise an Object as a paraboloid primitive.
 *
 *  Builds the canonical-space transform from the axis defined by @p bot and
 *  @p top and the base @p radius, computes the bounding box, and attaches
 *  the ParabolaData to the object.
 *  @param object  Pre-allocated Object to configure.
 *  @param bot     Centre of the base circle (wide end) in world space.
 *  @param top     Apex point (narrow tip) in world space.
 *  @param radius  Radius of the base circle; must be > PLY_EPSILON.
 *  @return        @p object configured as T_PARABOLA.
 */
Object *MakeParabola(Object *object, Vec bot, Vec top, Flt radius)
{   
   ParabolaData *parabola;

   object->o_type = ShapeType::Parabola;
   object->o_procs = &ParabolaProcs;
   object->o_uv_steps[0] = 32;
   object->o_uv_steps[1] = 16;

   parabola = FactoryParabolaData();
  

   /* Process the primitive specific information */
   if (radius < PLY_EPSILON)
      serror("Degenerate parabola\n");
   /* Find the axis and axis length */
   NuVec axis;
   VecSub(top, bot, axis);
   Flt len = VecNuNormalize(axis);
   if (len < PLY_EPSILON)
      serror("Degenerate parabola\n");
   VecCopy(bot, parabola->bot);
   VecCopy(top, parabola->top);
   parabola->radius = radius;
   VecNegate(bot);
   Vec tempbot;
   tempbot[0] = bot[0]; tempbot[1] = bot[1]; tempbot[2] = bot[2];
   Get_Coordinate_TransformCPP(parabola->trans, tempbot, axis, radius, len);

   /* Compute bounding information */
   MakeVector(-1.0, -1.0, 0.0, object->o_bnd.lower_left);
   MakeVector(2.0, 2.0, 1.0, object->o_bnd.lengths);
   recompute_inverse_bbox(&object->o_bnd, &parabola->trans);

   object->o_data = (void *)parabola;
   return object;
}

/** @brief Evaluate a parametric point and normal on the paraboloid surface.
 *
 *  Maps (u, v) in [0,1]^2 to the paraboloid:
 *  - @c u controls the azimuthal angle theta = 2*pi*u
 *  - @c v controls the height; vt = 1-v, so the surface point is
 *    (vt*cos(theta), vt*sin(theta), vt^2) in canonical space.
 *  At v = 1 (the apex) the normal defaults to (0, 0, -1).
 *  @param obj   Paraboloid object.
 *  @param u     Azimuthal parametric coordinate in [0, 1].
 *  @param v     Height parametric coordinate in [0, 1] (0 = base rim, 1 = apex).
 *  @param vert  Output vertex receiving object-space position, world-space
 *               position, normal, and UV coordinates.
 */
void ParabolaEvaluator(Object *obj, Flt u, Flt v, Vertex *vert)
{
   Flt theta, vt = 1.0 - v;
   Vec v0, v1, P, N;
   ParabolaData *par = (ParabolaData *)obj->o_data;

   MakeVector(u, v, 0.0, vert->U);

   theta  = TWO_PI * u;
   MakeVector(vt * cos(theta), vt * sin(theta), vt * vt, P);

   if (vt > PLY_EPSILON) {
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

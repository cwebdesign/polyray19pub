/* superq.cc

  Processing for superquadric primitives

  Note: values of n and e that are close to degenerate (e.g., below around
  0.1) appear to give the root solver fits.  Note sure quite where the problem
  lays just yet.

  Polyray MIT Licensed Revival
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
#include "superq.h"
#include "factory.h"

int SuperQIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int SuperQInside(Object *obj, Vec P);

openpolyray::dispatch::ObjectProcs SuperQProcs = {
   .render = GenericRender,
   .evaluate = SuperQ_Evaluator,
   .initialize = GenericInitialize,
   .intersect = SuperQIntersect,
   .inside = SuperQInside,
   .copy = GenericCopy,
   .del = SuperQDelete,
   };

/**
 * @brief Release the heap payload owned by a SuperQ primitive.
 *
 * @param object Box object whose `SuperQData` payload should be destroyed.
 * @return No return value.
 */
void SuperQDelete(Object* object)
{
    if (object->o_copy != 0 || object->o_data == nullptr)
        return;

    delete static_cast<SuperQData*>(object->o_data);
    object->o_data = nullptr;
}




/** @brief Build a superquadric primitive from its shape exponents.
 *  @param object Object to initialize.
 *  @param n      Vertical superquadric exponent.
 *  @param e      Horizontal superquadric exponent.
 *  @return The initialized superquadric object.
 */
Object *MakeSuperq(Object *object, Flt n, Flt e)
{
   SuperQData *sq;

   object->o_type = ShapeType::SuperQ;
   object->o_procs = &SuperQProcs;
   object->o_uv_steps[0] = 64;
   object->o_uv_steps[1] = 32;

   sq = FactorySuperQData();
   if (n < PLY_EPSILON) n = PLY_EPSILON;
   if (e < PLY_EPSILON) e = PLY_EPSILON;
   sq->n = n;
   sq->e = e;

   /* Compute bounding information */
   for (int i=0;i<3;i++) {
      object->o_bnd.lower_left[i] = -1.0001;
      object->o_bnd.lengths[i] = 2.0002;
      }

   object->o_data = (void *)sq;
   return object;
}

static Flt sqbox[2][3] = {{-1,-1,-1}, { 1, 1, 1}};
constexpr int SQPLANECOUNT = 15;
static Flt sqplanes[SQPLANECOUNT][4] =
   {{1, 0, 0,-1}, {1, 0, 0, 0}, {1, 0, 0, 1},
    {0, 1, 0,-1}, {0, 1, 0, 0}, {0, 1, 0, 1},
    {0, 0, 1,-1}, {0, 0, 1, 0}, {0, 0, 1, 1},
    {1, 1, 0, 0}, {1,-1, 0, 0},
    {1, 0, 1, 0}, {1, 0,-1, 0},
    {0, 1, 1, 0}, {0, 1,-1, 0}};

/* Compare two slabs. */
/** @brief Compare two scalar distances for qsort ordering.
 *  @param in_a Pointer to the first distance value.
 *  @param in_b Pointer to the second distance value.
 *  @return Negative when @p in_a is smaller, zero when equal, positive otherwise.
 */
static int compdists(void const *in_a, void const *in_b)
{
   Flt a, b;

   a = *((Flt *)in_a);
   b = *((Flt *)in_b);

  if (a < b)
      return -1;
   else if (a == b)
      return 0;
   else
      return 1;
}

/* Find all the places where the ray intersects the set of
   subdividing planes through the superquadric.  Return the
   number of valid hits (within the bounding box).  */
/** @brief Collect candidate ray distances from the superquadric subdivision planes.
 *  @param ray      Ray to test.
 *  @param cnt      Initial number of distances already present in @p dists.
 *  @param dists    Distance buffer updated in place and sorted on return.
 *  @param mindist  Minimum valid distance.
 *  @param maxdist  Maximum valid distance.
 *  @return Number of valid candidate distances stored in @p dists.
 */
static int find_ray_plane_points(Ray *ray, int cnt, Flt *dists, Flt mindist, Flt maxdist)
{
   int i;
   Flt t, d;

   /* Since min and max dist are the distance to two of
      the bounding planes we are considering, there is
      a high probablity of missing them due to round
      off error.  Therefore we adjust min and max. */
   t = PLY_EPSILON * (maxdist - mindist);
   mindist -= t;
   maxdist += t;

   /* Check the sets of planes that cut apart the superquadric */
   for (i=0;i<SQPLANECOUNT;i++) {
      d = (ray->D[0] * sqplanes[i][0] +
           ray->D[1] * sqplanes[i][1] +
           ray->D[2] * sqplanes[i][2]);
      if (fabs(d) < PLY_EPSILON)
         /* Can't possibly get a hit for this combination
            of ray and plane. */
         continue;
      t = (sqplanes[i][3] -
           (ray->P[0] * sqplanes[i][0] +
            ray->P[1] * sqplanes[i][1] +
            ray->P[2] * sqplanes[i][2])) / d;
      if (t >= mindist && t <= maxdist)
         dists[cnt++] = t;
      }

   /* Sort the results for further processing */
   qsort((char *)(dists), cnt, sizeof(Flt), compdists);

   return cnt;
}

/* Evaluate the superquadric equation at a point in space.
   Note that both n and e must be greater than 0 for this to work */
/** @brief Evaluate the implicit superquadric function at a point.
 *  @param P Point in object space.
 *  @param n Vertical superquadric exponent.
 *  @param e Horizontal superquadric exponent.
 *  @return Signed implicit-function value; zero lies on the surface.
 */
static Flt eval_superq(Vec P, Flt n, Flt e)
{
   Flt v;

   v = pow((pow(fabs(P[0]), 2.0 / e) +
            pow(fabs(P[1]), 2.0 / e)), e / n) +
       pow(fabs(P[2]), 2.0 / n) - 1.0;
   return v;
}

/* Find the normal vector to superquadric surface */
/** @brief Compute the geometric normal of a superquadric at a point.
 *  @param n Vertical superquadric exponent.
 *  @param e Horizontal superquadric exponent.
 *  @param P Point on the surface in object space.
 *  @param N Output normal vector.
 *  @return No return value.
 */
static void SuperQNormal(Flt n, Flt e, Vec P, Vec N)
{
   Flt x, y, z;
   Flt ix, iy, iz;

   x = fabs(P[0]);
   y = fabs(P[1]);
   z = fabs(P[2]);

   ix = SGN(P[0]); if (ix == 0) ix = 1;
   iy = SGN(P[1]); if (iy == 0) iy = 1;
   iz = SGN(P[2]); if (iz == 0) iz = 1;

   N[0] = ix * pow(x,(-1 + 2 / e)) *
          pow((pow(x, 2/e) + pow(y, 2/e)), (-1 + e/n));
   N[1] = iy * pow(y,(-1 + 2 / e)) *
          pow((pow(x, 2/e) + pow(y, 2/e)), (-1 + e/n));
   N[2] = iz *  pow(z,(-1 + 2 / n));
   VecNormalize(N);
}

constexpr Flt SQPLY_EPSILON = 1.0e-10;
constexpr int MAX_SQ_ITERATIONS = 20;

/* Home in on the root of a superquadric using a combination of
   secant and bisection methods.  This routine requires that
   the sign of the function be different at P0 and P1, it will
   fail drastically if this isn't the case.  */
/** @brief Refine a superquadric root bracket using secant and bisection steps.
 *  @param n   Vertical superquadric exponent.
 *  @param e   Horizontal superquadric exponent.
 *  @param v0  Implicit-function value at @p tP0.
 *  @param tP0 Near endpoint of the bracket.
 *  @param v1  Implicit-function value at @p tP1.
 *  @param tP1 Far endpoint of the bracket.
 *  @param P   Output point on the surface.
 *  @return No return value.
 */
static void solve_sq_hit1(Flt n, Flt e, Flt v0, Vec tP0, Flt v1, Vec tP1, Vec P)
{
   Vec P0, P1, P2, P3;
   Flt x, v, v2, v3;
   int i;

   VecCopy(tP0, P0)
   VecCopy(tP1, P1)

   /* The sign of v0 and v1 changes between P0 and P1, this
      means there is an intersection point in there somewhere. */
   for (i=0;i<MAX_SQ_ITERATIONS;i++) {
      if (fabs(v0) < SQPLY_EPSILON) {
         /* Near point is close enough to an intersection - just
            use it. */
         v = v0;
         VecCopy(P0, P);
         break;
         }
      else if (fabs(v1) < SQPLY_EPSILON) {
         /* Far point is close enough to an intersection */
         v = v1;
         VecCopy(P1, P);
         break;
         }
      else {
         /* Look at the chord connecting P0 and P1 */
         x = fabs(v0) / fabs(v1 - v0); /* Assume a line between the points */
         VecSub(P1, P0, P2);
         VecAddScaled(P0, x, P2, P2);
         v2 = eval_superq(P2, n, e);

         /* Look at the midpoint between P0 and P1 */
         VecSub(P1, P0, P3);
         VecAddScaled(P0, 0.5, P3, P3);
         v3 = eval_superq(P3, n, e);

         if (v0 * v2 > 0.0) {
            if (v1 * v2 > 0.0) {
               /* This should be impossible, since v0 and v1
                  were opposite signs, v2 must be either 0 or
                  opposite in sign to either v0 or v1 */
               serror("internal failure in function solve_sq_hit1: %d, %g, %g, %g", i, v0, v1, v2);
               }
            else if (v0 * v3 > 0.0) {
               if (x < 0.5) {
                  v0 = v3;
                  VecCopy(P3, P0)
                  }
               else {
                  v0 = v2;
                  VecCopy(P2, P0);
                  }
               }
            else {
               /* We can move both ends */
               v0 = v2;
               VecCopy(P2, P0)
               v1 = v3;
               VecCopy(P3, P1)
               }
            }
         else if (v0 * v3 > 0.0) {
            /* We can move both ends */
            v0 = v3;
            VecCopy(P3, P0)
            v1 = v2;
            VecCopy(P2, P1)
            }
         else if (x < 0.5) {
            v1 = v2;
            VecCopy(P2, P1);
            }
         else {
            v1 = v3;
            VecCopy(P3, P1);
            }
         }
      }

   if (i == MAX_SQ_ITERATIONS) {
      /* The loop never quite closed in on the result - just
         use the point closest to zero.  This really shouldn't happen
         since the max number of iterations is enough to converge
         with straight bisection.  */
      if (fabs(v0) < fabs(v1)) {
         v = v0;
         VecCopy(P0, P);
         }
      else {
         v = v1;
         VecCopy(P1, P);
         }
      }
}

/* Try to find the root of a superquadric using Newtons method.  */
/** @brief Search for a missed superquadric root between sampled distances.
 *  @param n   Vertical superquadric exponent.
 *  @param e   Horizontal superquadric exponent.
 *  @param P   Ray origin in object space.
 *  @param D   Ray direction in object space.
 *  @param t0  Start distance of the interval.
 *  @param P0  Point at @p t0.
 *  @param v0  Implicit-function value at @p P0.
 *  @param t1  End distance of the interval.
 *  @param t   Output hit distance.
 *  @param Q   Output hit point.
 *  @return 1 when a root is found, 0 otherwise.
 */
static int check_sq_hit2(Flt n, Flt e, Vec P, Vec D,
              Flt t0, Vec P0, Flt v0, Flt t1,
              Flt *t, Vec Q)
{
   Flt dt0, dt1, v1, deltat, maxdelta;
   Vec P1;
   int i;

   dt0 = t0;
   dt1 = t0 + 0.0001 * (t1 - t0);
   maxdelta = t1 - t0;
   for (i=0;dt0<t1 && i<MAX_SQ_ITERATIONS;i++) {
      VecAddScaled(P, dt1, D, P1)
      v1 = eval_superq(P1, n, e);
      if (v0 * v1 < 0) {
         /* Found a crossing point, go back and
            use normal root solving */
         solve_sq_hit1(n, e, v0, P0, v1, P1, Q);
         VecSub(Q, P, P0);
         *t = sqrt(VecDot(P0, P0));
         return 1;
         }
      else if (fabs(v1) < SQPLY_EPSILON) {
         VecAddScaled(P, dt1, D, Q)
         *t = dt1;
         return 1;
         }
      else if ((v0 > 0 && v1 > v0) ||
               (v0 < 0 && v1 < v0)) {
         /* We definitely failed */
         break;
         }
      else if (v1 == v0)
         break;
      else
         deltat = v1 * (dt1 - dt0) / (v1 - v0);
      if (fabs(deltat) > maxdelta)
         break;
      v0 = v1;
      dt0 = dt1;
      dt1 -= deltat;
      }

   return 0;
}

/* Find the intersection of a ray with a superquadric */
/** @brief Intersect a ray with a superquadric primitive.
 *  @param Eye      Active viewpoint, or nullptr for non-primary rays.
 *  @param obj      Superquadric object to test.
 *  @param ray      Ray in object space.
 *  @param mindist  Minimum valid hit distance.
 *  @param maxdist  Maximum valid hit distance.
 *  @param hit      Intersection accumulator.
 *  @return 1 when a valid hit is inserted, 0 otherwise.
 */
int SuperQIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                Flt mindist, Flt maxdist, Isect *hit)
{
   Vec N, P, D, P0, P1, P2, P3;
   Flt dists[SQPLANECOUNT+2];
   Flt n, e, t, t1, t2, v0, v1;
   int i, cnt;
   SuperQData *sq = (SuperQData *)obj->o_data;

   VecCopy(ray->P, P)
   VecCopy(ray->D, D)
   t1 = mindist;
   t2 = maxdist;
   if (!determine_start(P, D, sqbox, &t1, &t2))
      return 0;
   n = sq->n;
   e = sq->e;

   cnt = 0;
   /* If the ray is inside the superquadric bounding
      box then we include the start/end point of the ray
      as one of the subdividing points. */
   if (t1 == mindist)
      dists[cnt++] = t1;
   if (t2 == maxdist)
      dists[cnt++] = t2;

   cnt = find_ray_plane_points(ray, cnt, dists, t1, t2);

   if (cnt <= 1)
      return 0;

   VecAddScaled(P, dists[0], D, P0)
   v0 = eval_superq(P0, n, e);
   if (fabs(v0) < SQPLY_EPSILON) {
      SuperQNormal(n, e, P0, N);
      if (Insert_Hit(obj, P0, N, dists[0], P0, hit))
         return 1;
      }
   for (i=1;i<cnt;i++) {
      VecAddScaled(P, dists[i], D, P1)
      v1 = eval_superq(P1, n, e);
      if (fabs(v1) < SQPLY_EPSILON) {
         SuperQNormal(n, e, P1, N);
         if (Insert_Hit(obj, P1, N, dists[i], P1, hit))
            return 1;
         }
      else if (v0 * v1 < 0.0) {
         /* Opposite signs, there must be a root between */
         solve_sq_hit1(n, e, v0, P0, v1, P1, P2);
         VecSub(P2, P, P3);
         t = sqrt(VecDot(P3, P3));
         SuperQNormal(n, e, P2, N);
         if (Insert_Hit(obj, P2, N, t, P2, hit))
            return 1;
         }
      else if (check_sq_hit2(n, e, P, D, dists[i-1], P0, v0,
                             dists[i], &t, P2)) {
         /* Although there was no sign change, we may actually
            be approaching the surface.  In this case, we are
            being fooled by the shape of the surface into thinking
            there isn't a root between sample points. */
         SuperQNormal(n, e, P2, N);
         if (Insert_Hit(obj, P2, N, t, P2, hit))
            return 1;
         else
            break;
         }
      v0 = v1;
      VecCopy(P1, P0)
      }

   /* No valid hits were found */
   return 0;
}

/** @brief Test whether a point lies inside a superquadric.
 *  @param obj Superquadric object to test.
 *  @param Pos Point in world space.
 *  @return 1 when the point is inside, 0 otherwise.
 */
int SuperQInside(Object *obj, Vec Pos)
{
   SuperQData *sq;
   Vec P;
   Flt d;

   InvTxVector1(P, Pos, obj->o_trans)
   sq = (SuperQData *)obj->o_data;
   d = eval_superq(P, sq->n, sq->e);
   return (d < 0 ? 1 : 0);
}

/** @brief Evaluate a parametric point and normal on the superquadric surface.
 *  @param obj   Superquadric object being sampled.
 *  @param u     Horizontal parametric coordinate.
 *  @param v     Vertical parametric coordinate.
 *  @param vert  Output vertex receiving position, UV, world position, and normal.
 *  @return No return value.
 */
void SuperQ_Evaluator(Object *obj, Flt u, Flt v, Vertex *vert)
{
   Flt n, e;
   Flt icu, isu, icv, isv;
   Flt cu, su, cv, sv;
   Vec P, N;
   SuperQData *sq = (SuperQData *)obj->o_data;

   n = sq->n;
   e = sq->e;

   MakeVector(u, v, 0.0, vert->U);

   v = (TWO_PI * v - PYM_PI) / 2.0;
   u = TWO_PI * (1.0 - u);

   cu  = cos(u);   su  = sin(u);
   cv  = cos(v);   sv  = sin(v);
   icu = SGN(cu);  isu = SGN(su);
   icv = SGN(cv);  isv = SGN(sv);
   cu  = fabs(cu); su  = fabs(su);
   cv  = fabs(cv); sv  = fabs(sv);

   P[0] = pow(cv, n) * pow(cu, e) * icv * icu;
   P[1] = pow(cv, n) * pow(su, e) * icv * isu;
   P[2] = pow(sv, n) * isv;

   if ((e < 2 || n < 2) &&
       (cu < PLY_EPSILON || su < PLY_EPSILON ||
        cv < PLY_EPSILON || sv < PLY_EPSILON))
      MakeVector(cu * cv, su * cv, sv, N)
   else {
      N[0] = pow(cv, 2.0-n) * pow(cu, 2.0-e) * icv * icu;
      N[1] = pow(cv, 2.0-n) * pow(su, 2.0-e) * icv * isu;
      N[2] = pow(sv, 2.0-n) * isv;
      }

   VecCopy(P, vert->P);
   if (obj->o_trans) {
      TxVector(P, P, obj->o_trans);
      TxNormal(N, N, obj->o_trans);
      }
   VecNormalize(N);
   VecCopy(P, vert->W);
   VecCopy(N, vert->N);
}

/*
  torus.cc

  Polyray MIT Licensed Revival
  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Copyright (C) 1999-2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the 
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the 
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "defs3.h"
#include "io_ply.h"
#include "memory.h"
#include "intersec.h"
#include "scan.h"
#include "vector.h"
#include "bound.h"
#include "runtime_state.h"
#include "symtab.h"
#include "roots.h"
#include "torus.h"
#include "factory.h"



void Torus_Evaluator(Object *, Flt, Flt, Vertex *);
int TorusIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int TorusNormal(TorusData *torus, Vec Pos, Vec N);
int TorusInside(Object *, Vec);


openpolyray::dispatch::ObjectProcs TorusProcs = {
   .render = GenericRender,
   .evaluate = Torus_Evaluator,
   .initialize = GenericInitialize,
   .intersect = TorusIntersect,
   .inside = TorusInside,
   .copy = GenericCopy,
   .del = TorusDelete,
   };

    /**
 * @brief Release the heap payload owned by a torus primitive.
 *
 * @param object Disc object whose `TorusData` payload should be destroyed.
 * @return No return value.
 */
void TorusDelete(Object *object)
{
   if (object->o_copy != 0 || object->o_data == nullptr)
      return;

   delete static_cast<TorusData *>(object->o_data);
   object->o_data = nullptr;
}


/** @brief Compute the surface normal for a point in canonical torus space.
 *  @param torus Torus data describing the primitive.
 *  @param P     Point in canonical torus space.
 *  @param N     Output normal vector.
 *  @return Always returns `1` after writing the transformed normal.
 */
int TorusNormal(TorusData *torus, Vec P, Vec N)
{
   Flt t, r0;

   /* Calculate the normal in torus space

      The function is: (x^2+y^2+z^2-(a^2+b^2))^2-4*a^2*(b^2-z^2)

      The partial derivatives are:
           x*(x^2+y^2+z^2-(a^2+b^2))
           y*(x^2+y^2+z^2-(a^2+b^2))
           z*(x^2+y^2+z^2-(a^2+b^2)) + 2*a^2*z
   */

   r0 = torus->r0;
   t = VecDot(P, P) - (r0 + torus->r1);
   N[0] = P[0] * t;
   N[1] = P[1] * t;
   N[2] = P[2] * (t + 2.0 * r0);

   InvTxNormal(N, N, &torus->trans);
   return 1;
}

/* Translate a point from canonical torus space to u,v coordinates */
/** @brief Convert a canonical torus-space point to parametric UV coordinates.
 *  @param P  Point in canonical torus space.
 *  @param r0 Major torus radius.
 *  @param u  Output u coordinate.
 *  @param v  Output v coordinate.
 *  @return No return value.
 */
static void
TorusUV(Vec P, Flt r0, Flt *u, Flt *v)
{
   Flt len, phi, theta;
   Flt x, y, z;

   x = P[0];
   y = P[1];
   z = P[2];

   /* Determine its angle from the x-axis. */
   len = sqrt(x * x + y * y);
   if (len == 0.0)
      return;
   
   if (y == 0.0)
      if (x > 0)
        theta = 0.0;
      else
        theta = PYM_PI;
      else {
       theta = acos(x / len);
       if (y < 0.0) theta = 2.0 * PYM_PI - theta;
      }
   

   /* Now rotate about the y-axis to get the point P into the x-z plane. */
   x = len - r0;
   len = sqrt(x * x + z * z);
   phi = acos(x / len);
   if (z < 0.0) phi = 2.0 * PYM_PI - phi;

   /* Determine the parametric coordinates. */
   *u = theta / (2.0 * PYM_PI);
   *v = phi / (2.0 * PYM_PI) - 0.5;
   if (*v < 0.0) *v += 1.0;
}

/** @brief Intersect a ray with a torus primitive.
 *  @param Eye      Active viewpoint, or nullptr for non-primary rays.
 *  @param obj      Torus object to test.
 *  @param ray      Ray in world space.
 *  @param mindist  Minimum valid hit distance.
 *  @param maxdist  Maximum valid hit distance.
 *  @param hit      Intersection accumulator.
 *  @return 1 when at least one valid hit is inserted, 0 otherwise.
 */
int TorusIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
               Flt mindist, Flt maxdist, Isect *hit)
{
   Flt coeff[5], Depths[4];
   Flt dist, nmin, nmax;
   Vec P, D, PP, N, U;
   int i, hitcnt, Flag = 0;
   TorusData *torus = (TorusData *)obj->o_data;
   Flt t1, t2, t3, t4, t5, t6, u=0, v=0;

   /* Now transform to canonical torus space */
   TxVector(P, ray->P, &torus->trans);
   TxDirection(D, ray->D, &torus->trans);
   dist = VecNormalize(D);
   nmin = mindist * dist;
   nmax = maxdist * dist;

   t1 = VecDot(D, P);
   t2 = VecDot(P, P);
   t4 = torus->r0;
   t5 = torus->r1;
   t3 = t4 + t5;
   t6 = t2 - t3;

   coeff[0] = 1.0;
   coeff[1] = 4.0 * t1;
   coeff[2] = 2.0 * t6 + 4.0 * (t1 * t1 + t4 * D[2] * D[2]);
   coeff[3] = 4.0 * t1 * t6 + 8.0 * t4 * P[2] * D[2];
   coeff[4] = t6 * t6 - 4.0 * t4 * (t5 - P[2] * P[2]);

   if (torus->Sturm_Flag == 2) /* Sturm sequences */
      hitcnt = bounded_polysolve(4, coeff, Depths, nmin, nmax);
   else if (torus->Sturm_Flag == 1) /* Use Vieta's method */
      hitcnt = solve_quartic1(coeff, Depths, nmin, nmax);
   else /* Use Ferrari's method */
      hitcnt = solve_quartic(coeff, Depths, nmin, nmax);

   for (i=0;i<hitcnt;i++) {
      VecAddScaled(P, Depths[i], D, PP);
      TorusNormal(torus, PP, N);
      if ((runtimeState::scene.Global_Shade_Flag & UV_CHECK) &&
          (obj->o_sflag & UV_CHECK)) {
         TorusUV(PP, sqrt(torus->r0), &u, &v);
         MakeVector(u, v, 0, U);
         }
      else
         VecCopy(P, U);
      t1 = Depths[i]/dist;
      VecAddScaled(ray->P, t1, ray->D, PP);
      Insert_Hit(obj, PP, N, t1, U, hit);
      Flag = 1;
      }
   return Flag;
}

/** @brief Test whether a point lies inside a torus primitive.
 *  @param obj Torus object to test.
 *  @param Pos Point in world space.
 *  @return 1 when the point is inside the torus, 0 otherwise.
 */
int TorusInside(Object *obj, Vec Pos)
{
   Vec P;
   Flt w, r02, r12;
   TorusData *torus = (TorusData *)obj->o_data;

   /* The formula for a Torus in canonical form is:

         (x^2+y^2+z^2-(a^2+b^2))^2-4*a^2*(b^2-z^2)

   First transform to canonical torus space */

   InvTxVector1(P, Pos, obj->o_trans);
   TxVector(P, P, &torus->trans);

   /* Now see if it is inside the torus. */
   r02 = torus->r0;
   r12 = torus->r1;
   w = (VecDot(P, P) - r02 - r12);
   w = w * w - 4.0 * r02 * (r12 - P[2] * P[2]);
   return (w < PLY_EPSILON ? 1 : 0);
}

/** @brief Build a torus primitive from major/minor radii, center, and axis.
 *  @param object Object to initialize.
 *  @param r0     Major radius.
 *  @param r1     Minor radius.
 *  @param center Torus center.
 *  @param dir    Torus axis direction.
 *  @return The initialized torus object.
 */
Object *MakeTorus(Object *object, Flt r0, Flt r1, Vec center, Vec dir)
{
   Flt len;
   TorusData *torus;

   object->o_type = ShapeType::Torus;
   object->o_procs = &TorusProcs;
   object->o_uv_steps[0] = 32;
   object->o_uv_steps[1] = 16;

   /* Attempt to allocate memory for this primitive */
   torus = FactoryTorusData();

   /* Find the axis and axis length */
   len = VecNormalize(dir);
   if (len < PLY_EPSILON)
      serror("Bad direction for torus\n");
   VecCopy(center, torus->center);
   VecCopy(dir, torus->dir);
   torus->r0 = r0 * r0;
   torus->r1 = r1 * r1;
   torus->Sturm_Flag = 0;
   VecNegate(center);
   Vec tempcenter;
   tempcenter[0] = center[0]; tempcenter[1] = center[1]; tempcenter[2] = center[2];
   NuVec tempdir;
   tempdir[0] = dir[0]; tempdir[1] = dir[1]; tempdir[2] = dir[2];
   Get_Coordinate_TransformCPP((torus->trans), tempcenter, tempdir, 1.0, 1.0);

   MakeVector(-r0-r1, -r0-r1, -r1, object->o_bnd.lower_left);
   MakeVector(2.0*(r0+r1), 2.0*(r0+r1), 2.0*r1, object->o_bnd.lengths);
   recompute_inverse_bbox(&object->o_bnd, &torus->trans);
   object->o_data = (void *)torus;
   return object;
}

/** @brief Select the quartic root solver used for torus intersections.
 *  @param obj         Torus object to update.
 *  @param Sturm_Flag  Solver selector value.
 *  @return No return value.
 */
void Set_Torus_Solver(Object *obj, int Sturm_Flag)
{
   TorusData *torus = static_cast<TorusData *>(obj->o_data);
   torus->Sturm_Flag = Sturm_Flag;
}

/** @brief Evaluate a torus parametrically and produce a shaded vertex sample.
 *  @param obj  Torus object to evaluate.
 *  @param u    Parametric u coordinate.
 *  @param v    Parametric v coordinate.
 *  @param vert Output vertex populated with position, normal, and UV data.
 *  @return No return value.
 */
void Torus_Evaluator(Object *obj, Flt u, Flt v, Vertex *vert)
{
   Flt theta, phi;
   Vec v0, v1, P, N;
   Flt r0, r1;
   TorusData *torus = (TorusData *)obj->o_data;

   MakeVector(u, v, 0.0, vert->U);

   r0 = sqrt(torus->r0);
   r1 = sqrt(torus->r1);

   theta = TWO_PI * (v - 0.5);
   phi   = TWO_PI * u;

   /* Compute the position of the point */
   MakeVector((r0 + r1 * cos(theta)) * cos(phi),
              (r0 + r1 * cos(theta)) * sin(phi),
              r1 * sin(theta), P);

   /* Compute the normal at that point */
   MakeVector(r1*sin(theta)*cos(phi),
              r1*sin(theta)*sin(phi),
              -r1*cos(theta), v0);
   MakeVector(-(r0+r1*cos(theta))*sin(phi),
              (r0+r1*cos(theta))*cos(phi),
              0.0, v1);
   VecCross(v0, v1, N);
   InvTxVector(P, P, &torus->trans);
   InvTxNormal(N, N, &torus->trans);

   VecCopy(P, vert->P);
   if (obj->o_trans) {
      TxVector(P, P, obj->o_trans);
      TxNormal(N, N, obj->o_trans);
      }
   VecCopy(P, vert->W);
   VecCopy(N, vert->N);
}

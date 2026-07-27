/* cone.cc

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
#include "runtime_state.h"
#include "symtab.h"
#include "scan.h"
#include "vector.h"
#include "bound.h"
#include "cone.h"
#include "cylinder.h"
#include "factory.h"

/* This is a placeholder for primitive data */

void Cone_Evaluator(Object *obj, Flt u, Flt v, Vertex *vert);
int ConeIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                  Flt mindist, Flt maxdist, Isect *hit);
int ConeNormal(ConeData* cone,  Vec Pos, Vec& N);
void ConeUV(const Vec P, const Vec D, Flt t, Flt d, Flt *u, Flt *v);
int ConeInside(Object *obj, Vec P);

openpolyray::dispatch::ObjectProcs ConeProcs = {
   .render = GenericRender,
   .evaluate = Cone_Evaluator,
   .initialize = GenericInitialize,
   .intersect = ConeIntersect,
   .inside = ConeInside,
   .copy = GenericCopy,
   .del = ConeDelete,
   };

/**
 * @brief Release the heap payload owned by a cone primitive.
 *
 * @param object Cone object whose `ConeData` payload should be destroyed.
 * @return No return value.
 */
void ConeDelete(Object* object)
{
    if (object->o_copy != 0 || object->o_data == nullptr)
        return;

    delete static_cast<ConeData*>(object->o_data);
    object->o_data = nullptr;
}

/**
 * Calculate the surface normal for a point on a cone.
 * @param cone Cone definition that provides the canonical-space transform.
 * @param Pos Surface position in world space.
 * @param N Receives the transformed outward normal.
 * @return `1` after computing the normal.
 */
int ConeNormal(ConeData *cone,  Vec Pos, Vec& N)
{
#ifdef DEBUG_FN_CALLS
    smessage("cone::ConeNormal\n");
#endif
   Vec P;
   TxVector(P, Pos, &cone->trans);
   P[2] = -P[2];
   InvTxNormal(N, P, &cone->trans);
   return 1;
}

/**
 * Compute cone texture coordinates for a ray hit in canonical cone space.
 * @param Pos Ray origin in canonical cone space.
 * @param D Normalized ray direction in canonical cone space.
 * @param t Hit distance along `D`.
 * @param d Cone base offset stored in `cone->dist`.
 * @param u Receives the computed horizontal texture coordinate.
 * @param v Receives the computed vertical texture coordinate.
 * @return No return value.
 */
void ConeUV(const Vec Pos, const Vec D, Flt t, Flt d, Flt *u, Flt * v)
{
#ifdef DEBUG_FN_CALLS
    smessage("cone::ConeUV\n");
#endif
   Flt x, len, theta=0.0f;
   Vec P;
   
   VecAddScaled(Pos, t, D, P);
   len = std::sqrt(P[0] * P[0] + P[1] * P[1]);
   /* Make sure this vector is on the unit cylinder. */
   if (len < PLY_EPSILON)
         theta=0;
     else {
      x  = P[0] / len;
      //if (P[1] == 0.0)
      if (POLYRAYequal(P[1],0.0))
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
      
   *u = 1.0 - theta / (2.0 * PYM_PI);
   *v = 1.0 - (P[2] - d) / (1.0 - d);
}

/**
 * Validate a candidate cone hit and insert it into the hit list when valid.
 * @param obj Cone object owning the intersection.
 * @param cone Cone data used for normal and UV evaluation.
 * @param ray Original world-space ray.
 * @param P Ray origin in canonical cone space.
 * @param D Normalized ray direction in canonical cone space.
 * @param t Candidate hit distance in canonical cone space.
 * @param nmin Minimum valid hit distance in canonical cone space.
 * @param nmax Maximum valid hit distance in canonical cone space.
 * @param dist Scale factor between canonical-space and world-space ray lengths.
 * @param hit Intersection list that receives the hit when accepted.
 * @return Result from `Insert_Hit` when the hit is valid, otherwise `0`.
 */
static int check_cone_hit(Object *obj, ConeData *cone,
               Ray *ray, Vec& P, Vec& D, Flt t,
               Flt nmin, Flt nmax, Flt dist, Isect *hit)
{
#ifdef DEBUG_FN_CALLS
    smessage("cone::check_cone_hit\n");
#endif
   Vec PP, N, U;
   Flt u, v;

   if (t >= nmin && t <= nmax) {
      if ((runtimeState::scene.Global_Shade_Flag & UV_CHECK) &&
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

/**
 * Intersect a ray with a cone primitive.
 * @param Eye Viewpoint issuing the ray.
 * @param obj Cone object being tested.
 * @param ray Ray to intersect.
 * @param mindist Minimum valid hit distance.
 * @param maxdist Maximum valid hit distance.
 * @param hit Intersection list that receives accepted hits.
 * @return `1` when at least one cone hit is recorded, otherwise `0`.
 */
int ConeIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
              Flt mindist, Flt maxdist, Isect *hit)
{
#ifdef DEBUG_FN_CALLS
    std::cout<<("cone::ConeIntersect\n");//instead of smessage
#endif
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
      if (fabs(b) < 1.0e-20) // No possible intersection 
         return 0;
      /* One intersection */
#ifdef DEBUG
      //smessage("one intersection\n");
#endif
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
#ifdef DEBUG
      std::cout << "disc=" << disc << "\n";
#endif
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

/**
 * Test whether a world-space point lies inside the truncated cone volume.
 * @param obj Cone object being queried.
 * @param Pos World-space point to test.
 * @return `1` when `Pos` lies inside the capped cone volume, otherwise `0`.
 */
int ConeInside(Object *obj, Vec Pos)
{
#ifdef DEBUG_FN_CALLS
    smessage("cone::ConeInside\n");    
#endif
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

/**
 * Initialize an object as a cone primitive.
 * @param object Object being configured as a cone.
 * @param bot Bottom endpoint of the cone axis.
 * @param brad Radius at `bot`.
 * @param top Top endpoint of the cone axis.
 * @param trad Radius at `top`.
 * @return `object` configured as a cone, or a cylinder object when the radii match.
 */
Object *MakeCone(Object *object, Vec bot, Flt brad, Vec top, Flt trad)
{
#ifdef DEBUG_FN_CALLS
    smessage("cone::MakeCone\n");
#endif
   Flt cottheta, lprime, tlen, len, tmpf;
   Vec base, tmpv;

#ifdef DEBUG_FN_CALLS
   printf("bot=[%f,%f,%f]\n", bot[0], bot[1], bot[2]);
   printf("top=[%f,%f,%f]\n", top[0], top[1], top[2]);
   printf("brad=%f, trad=%f\n", brad, trad);
#endif
   
   object->o_type  = ShapeType::Cone;
   object->o_procs = &ConeProcs;
   object->o_uv_steps[0] = 16;
   object->o_uv_steps[1] = 2;

   /* Attempt to allocate memory for this primitive */
   ConeData *cone = FactoryConeData();
   
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
   else {
       if (POLYRAYequal(trad, brad)) {
           /* Quietly change this cone into a cylinder */
           return MakeCylinder(object, bot, top, trad);
       }
   }
   /* Find the axis and axis length */
   NuVec axis;
   VecSub(top, bot, axis);
   len = VecNuNormalize(axis);
#ifdef DEBUG_FN_CALLS
   printf("len=%f\n", len);
#endif

   if (len < PLY_EPSILON)
      serror("Degenerate cone\n");
   /* Determine alignment */
   cottheta = len / (trad - brad);
   lprime = brad * cottheta;
   base[0] = lprime * axis[0];
   base[1] = lprime * axis[1];
   base[2] = lprime * axis[2];
#ifdef DEBUG_FN_CALLS
   printf("base=[%f,%f,%f]\n", base[0], base[1], base[2]);
#endif
   VecSub(base, bot, base);
   tlen = lprime + len;
   cone->dist = lprime / tlen;
   Get_Coordinate_TransformCPP(cone->trans, base, axis, trad, tlen);
#ifdef DEBUG_FN_CALLS
   printf("tlen=%f,lprime=%f,cottheta=%f\n", tlen, lprime, cottheta);
#endif



   /* Compute bounding information */
#ifdef DEBUG_FN_CALLS
   printf("compute bounding information: cone->dist=%f\n", cone->dist);
#endif
   //object->o_bnd = FactoryBBOXINFO();
   MakeVector(-1.0, -1.0, cone->dist, object->o_bnd.lower_left);
   MakeVector(2.0, 2.0, 1.0-cone->dist, object->o_bnd.lengths);
   recompute_inverse_bbox(&object->o_bnd, &cone->trans);

   object->o_data = (void *)cone;
   return object;
}

/**
 * Sample a cone surface point and normal from parametric UV coordinates.
 * @param obj Cone object being evaluated.
 * @param u Horizontal parameter on the cone surface.
 * @param v Vertical parameter on the cone surface.
 * @param vert Receives the sampled position, normal, and texture coordinates.
 * @return No return value.
 */
void
Cone_Evaluator(Object *obj, Flt u, Flt v, Vertex *vert)
{
#ifdef DEBUG_FN_CALLS
    smessage("cone::Cone_Evaluater\n");
#endif
   Vec P, N, v0, v1;
   ConeData *cone = (ConeData *)obj->o_data;
   Flt vt, theta;

   vt = cone->dist + (1.0 - v) * (1.0 - cone->dist);
   theta = TWO_PI * (1.0 - u);
   MakeVector(u, v, 0.0, vert->U);

   MakeVector(vt * cos(theta), vt * sin(theta), vt, P);
   if (vt > PLY_EPSILON) {
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

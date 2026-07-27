/* bezier.cc

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

#include <memory>
#include <map>

#include "defs3.h"
#include "bezier.h"
#include "io_ply.h"
#include "memory.h"
#include "builder.h"
#include "eval.h"
#include "vector.h"
#include "factory.h"
#include "symtab.h"
#include "polyray.tab.h"

/* Basis matrices for: Bezier, B-Spline, Catmull-Rom, and Hermite.  Note that
   for the first three, the vectors P0 - P3 are used.  For the last (Hermite)
   two vectors P0, P3 and two tangents R0, R3 are used.  */
static NuMatrix4 BZM = {{
    {-1, 3,-3, 1}, { 3,-6, 3, 0},
    {-3, 3, 0, 0}, { 1, 0, 0, 0}
}};
static NuMatrix4 BSM = {{
    {-1, 3,-3, 1}, { 3,-6, 3, 0},
    {-3, 0, 3, 0}, { 1, 4, 1, 0}
}};
static NuMatrix4 CRM = {{
    {-1, 3,-3, 1}, { 2,-5, 4,-1},
    {-1, 0, 1, 0}, { 0, 2, 0, 0}
}};
static NuMatrix4 HRM = {{
    { 2,-2, 1, 1}, {-3, 3,-2,-1},
    { 0, 0, 1, 0}, { 1, 0, 0, 0}
}};


/* Representation of a bicubic patch is:
      Q(t) = S . M . G . Mt . Tt
   Where:
      S = [s^3, s^2, s, 1]
      T = [t^3, t^2, t, 1]
      M = Basis matrix from above (Mt is the transpose)
      G = [P0, P1, P2, P3] (or for Hermite, [P0, P3, R0, R3])
*/




static void BezierEvaluator(Object *, Flt, Flt, Vertex *);
static int BezierIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                           Flt mindist, Flt maxdist, Isect *hit);
static int BezierInside(Object *obj, Vec P);

static openpolyray::dispatch::ObjectProcs BezierProcs = {
   .render = GenericRender,
   .evaluate = BezierEvaluator,
   .initialize = GenericInitialize,
   .intersect = BezierIntersect,
   .inside = BezierInside,
   .copy = GenericCopy,
   .del = BezierDelete,
   };

/**
 * @brief Release the heap payload owned by a bezier primitive.
 *
 * @param object Box object whose `BezierData` payload should be destroyed.
 * @return No return value.
 */
void BezierDelete(Object* object)
{
    if (object->o_copy != 0 || object->o_data == nullptr)
        return;

    delete static_cast<BezierData*>(object->o_data);
    object->o_data = nullptr;
}


static void NurbEvaluator(Object *, Flt, Flt, Vertex *);

static openpolyray::dispatch::ObjectProcs NurbProcs = {
   .render = GenericRender,
   .evaluate = NurbEvaluator,
   .initialize = GenericInitialize,
   .intersect = BezierIntersect,
   .inside = BezierInside,
   .copy = GenericCopy,
   .del = NurbDelete,
   };

/**
 * @brief Report whether a point lies inside a Bezier patch volume.
 *
 * @param obj Bezier object being queried.
 * @param P Point to test.
 * @return Always returns `0` because Bezier patches are not treated as CSG solids.
 */
static int BezierInside(Object *obj, Vec P)
{
   /* Not a csg primitive */
   return 0;
}

/* Determine the normal at a single coordinate point (u, v) on a bezier patch */
/**
 * @brief Determine the surface point and normal for a bicubic patch sample.
 *
 * @param shape Bezier patch data containing the control mesh and basis type.
 * @param u0 Parametric coordinate along the U direction.
 * @param v0 Parametric coordinate along the V direction.
 * @param P Receives the evaluated surface point.
 * @param N Receives the normalized surface normal.
 * @return No return value.
 */
static void BezierNormal(BezierData *shape, Flt u0, Flt v0, Vec P, Vec N)
{
   Flt t;
   Flt u[4], v[4], du[4], dv[4];
   Flt um[4], vm[4], dum[4], dvm[4];
   int i, j;
   Vec U, V;

   u[0]  = 1.0; du[0] = 0.0;
   v[0]  = 1.0; dv[0] = 0.0;
   for (i=1;i<4;i++) {
      u[i]   = u[i-1] * u0;
      v[i]   = v[i-1] * v0;
      du[i]  =  i * u[i-1];
      dv[i]  =  i * v[i-1];
      }

   /* Now evaluate a Bezier based on it's control points */
   MakeVector(0, 0, 0, P);
   MakeVector(0, 0, 0, U);
   MakeVector(0, 0, 0, V);
   for (i=0;i<4;i++) {
      um[i] = 0.0;
      vm[i] = 0.0;
      dum[i] = 0.0;
      dvm[i] = 0.0;
      for (j=0;j<4;j++) {
         if (shape->patch_type >= 0 &&
             shape->patch_type <= 3) {
            um[i] += u[3-j] * BZM[j][i];
            //printf("um[%d] now=%f BZM[j][i]=%f\n",i,um[i],BZM[j][i]);
            vm[i] += v[3-j] * BZM[i][j];
            //printf("v[3-j]=%f BZM[i][j]=%f vm[%d]=%f\n",v[3-j],BZM[i][j],i,vm[i]);
            //printf("du[%d]=%f\n",3-j,du[3-j]);
            dum[i] += du[3-j]* BZM[j][i];
            //printf("dum[%d] now = %f",i,dum[i]);
            dvm[i] += dv[3-j] * BZM[i][j];
            }
         else if (shape->patch_type == 4) {
            um[i] += u[3-j] * BSM[j][i] / 6;
            vm[i] += v[3-j] * BSM[i][j] / 6;
            dum[i] += du[3-j] * BSM[j][i] / 6;
            dvm[i] += dv[3-j] * BSM[i][j] / 6;
            }
         else if (shape->patch_type == 5) {
            um[i] += u[3-j] * CRM[j][i] / 2;
            vm[i] += v[3-j] * CRM[i][j] / 2;
            dum[i] += du[3-j] * CRM[j][i] / 2;
            dvm[i] += dv[3-j] * CRM[i][j] / 2;
            }
         else {
            um[i] += u[3-j] * HRM[j][i];
            vm[i] += v[3-j] * HRM[i][j];
            dum[i] += du[3-j] * HRM[j][i];
            dvm[i] += dv[3-j] * HRM[i][j];
            }
         }
      }

   for (i=0;i<4;i++) {
      for (j=0;j<4;j++) {
         t = um[i] * vm[j];
         //printf("control %d,%d: t=%f %f %f %f %f %f\n",i,j,t,um[i],vm[j],dum[i],vm[i],dvm[j]);
         //printf("um[%d]=%f vm[%d]=%f t=%f\n",i,um[i],j,vm[j],t);
         //printf("P before=%f,%f,%f\n",P[0],P[1],P[2]);
         VecAddScaled(P, t, shape->Control_Points[i][j], P);
         //printf("P after=%f,%f,%f\n",P[0],P[1],P[2]);
         t = dum[i] * vm[j];
         //printf("dum[%d]=%f vm[%d]=%f t2=%f\n",i,dum[i],j,vm[j],t);
         //printf("U before=%f,%f,%f\n",U[0],U[1],U[2]);
         VecAddScaled(U, t, shape->Control_Points[i][j], U);
         //printf("U after=%f,%f,%f\n",U[0],U[1],U[2]);
         t = um[i] * dvm[j];
         VecAddScaled(V, t, shape->Control_Points[i][j], V);
         }
      }

   VecCross(U, V, N);
   //VecCopy(V,N);
   VecNormalize(N);
}

/**
 * @brief Exercise Bezier patch normal evaluation with synthetic control points.
 *
 * @return No return value.
 */
void BezierNormalTest(void)
{
    BezierData* test1 = FactoryBezierData();
    for (int pt=0;pt<=5;pt++) {
       test1->patch_type=pt;
       for (int i=0;i<4;i++)
         for (int j=0;j<4;j++)
         {
           test1->Control_Points[i][j][0]=i*j+4;
           test1->Control_Points[i][j][1]=i*20;
           test1->Control_Points[i][j][2]=j*2;
         }
       Flt u0=0.0;Flt v0=0.0;
       Vec P,N;
       BezierNormal(test1,u0,v0,P,N);
       //printf("N=%f,%f,%f\n",N[0],N[1],N[2]);
//         std::cout<<"N="<<N[0]<<","<<N[1]<<","<<N[2]<<std::endl;
   }

   delete test1;
}

/**
 * @brief Initialize a Bezier patch object from a 4x4 control mesh.
 *
 * @param object Object being initialized as a Bezier primitive.
 * @param type Patch basis type.
 * @param flatness Flatness hint supplied by the caller.
 * @param usteps Tessellation step count along the U direction.
 * @param vsteps Tessellation step count along the V direction.
 * @param points List of 16 control points describing the patch.
 * @return `object` configured as a Bezier patch.
 */
Object *MakeBezier(Object *object, int type, Flt flatness,
           int usteps, int vsteps, VList *points)
{
   Vec mins, maxs;
   Flt t;
   BezierData *Bezier;

   object->o_type = ShapeType::Bezier;
   object->o_procs = &BezierProcs;
   object->o_uv_steps[0] = usteps;
   object->o_uv_steps[1] = vsteps;

   /* Attempt to allocate memory for this primitive */
   if (points->count != 16)
      serror("Must be 16 points on a Bezier patch, there are only: %d\n",
            points->count);
   Bezier = FactoryBezierData();
   if (Bezier  == nullptr)
      serror("Failed to allocate Bezier data\n");
//   for (i=0;i<16;i++) {
   for (int i=16;i--;) {
      VecCopy(points->points[i], Bezier->Control_Points[i/4][i%4]);
      }
   Bezier->patch_type = type;

   VecCopy(points->points[0], mins);
   VecCopy(mins, maxs);
   for (int i=1;i<16;i++)
      for (int j=0;j<3;j++) {
         t = points->points[i][j];
         if (t < mins[j]) mins[j] = t;
         if (t > maxs[j]) maxs[j] = t;
         }
   VecCopy(mins, object->o_bnd.lower_left);
   VecSub(maxs, mins, object->o_bnd.lengths);
   polyray_free(points->points);
   polyray_free(points);
   object->o_data = (void *)Bezier;

   return object;
}

/**
 * @brief Stub intersection handler for a Bezier patch object.
 *
 * @param Eye Viewpoint issuing the ray.
 * @param obj Bezier object being tested.
 * @param ray Ray to test.
 * @param mindist Minimum valid hit distance.
 * @param maxdist Maximum valid hit distance.
 * @param hit Receives intersection data if a hit were found.
 * @return Always returns `0` because tessellated triangles handle intersections.
 */
static int BezierIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                Flt mindist, Flt maxdist, Isect *hit)
{
   /* Should never get here - the bezier patch is represented as
      a collection of triangles, each of which is handled by
      the high level intersection routine. */
   return 0;
}

/* Evaluate a single coordinate point (u, v) on a bezier patch. */
/**
 * @brief Evaluate a Bezier patch at one parametric coordinate.
 *
 * @param obj Bezier object being sampled.
 * @param u0 Parametric coordinate along the U direction.
 * @param v0 Parametric coordinate along the V direction.
 * @param vert Receives the evaluated position, normal, and UV data.
 * @return No return value.
 */
static void BezierEvaluator(Object *obj, Flt u0, Flt v0, Vertex *vert)
{
   Vec P, N;
   BezierData *shape = (BezierData *)obj->o_data;

   BezierNormal(shape, u0, v0, P, N);

   VecCopy(P, vert->P);
   if (obj->o_trans) {
      TxVector(P, P, obj->o_trans);
      TxNormal(N, N, obj->o_trans);
      }
   VecNormalize(N);
   VecCopy(P, vert->W);
   MakeVector(u0, v0, 0.0, vert->U);
   VecCopy(N, vert->N);
}

/**
 * @brief Initialize a NURB patch object from knot vectors and control points.
 *
 * @param object Object being initialized as a NURB primitive.
 * @param norder Order of the basis in the U direction.
 * @param npts Number of control points in the U direction.
 * @param morder Order of the basis in the V direction.
 * @param mpts Number of control points in the V direction.
 * @param nknots Optional knot vector expression for the U direction.
 * @param mknots Optional knot vector expression for the V direction.
 * @param ctlpts Control mesh expression for the patch.
 * @return `object` configured as a NURB patch.
 */
Object *MakeNurb(Object *object, int norder, int npts, int morder, int mpts,
         NODE_PTR nknots, NODE_PTR mknots, NODE_PTR ctlpts)
{
   int i, j, haveknots=0;
   Flt fval;
   Vec vval;
   NODE_PTR nval, ntemp1, ntemp2;
   NurbData *ndata;

   object->o_type = ShapeType::Nurb;
   object->o_procs = &NurbProcs;

   /* Set the uv bounds to reflect the bounds of the knot values (or of
      the control mesh if no knot vector was supplied */
   object->o_uv_bounds[0] = 0.0;
   object->o_uv_bounds[1] = npts - norder + 1;
   object->o_uv_bounds[2] = 0.0;
   //Potential signed integer overflow in NURB arithmetic if input values are extreme/unvalidated:
   object->o_uv_bounds[3] = mpts - morder + 1;
   object->o_uv_steps[0] = npts * 2;
   object->o_uv_steps[1] = mpts * 2;

   /* Attempt to allocate memory for this primitive */
   ndata = FactoryNurbData();
   if (ndata == nullptr)
      serror("Failed to allocate NURB data\n");

   /* Do some type checking on the input parameters */
   ndata->rat_flag = 0;
   ndata->norder = norder;
   ndata->morder = morder;
   ndata->npts = npts;
   ndata->mpts = mpts;
   ndata->nknots = norder + npts;
   ndata->mknots = morder + mpts;
   ndata->nknotvec = (float *)polyray_malloc(ndata->nknots * sizeof(float));
   ndata->mknotvec = (float *)polyray_malloc(ndata->mknots * sizeof(float));
   ndata->nbasis  = (float *)polyray_malloc(ndata->nknots * sizeof(float));
   ndata->ndbasis = (float *)polyray_malloc(ndata->nknots * sizeof(float));
   ndata->mbasis  = (float *)polyray_malloc(ndata->mknots * sizeof(float));
   ndata->mdbasis = (float *)polyray_malloc(ndata->mknots * sizeof(float));
   if (ndata->nknotvec == nullptr || ndata->mknotvec == nullptr ||
       ndata->nbasis == nullptr || ndata->ndbasis == nullptr ||
       ndata->mbasis == nullptr || ndata->mdbasis == nullptr)
      serror("Failed to allocate NURB data");

   /* Count the number of elements in each knot vector */

   /* First verify that the knot vector for the u direction has the right
      number of elements */
   if (nknots == NULL)
      haveknots = 0;
   else if (nknots->exper_type != ARRAY)
      serror("Knot vector for NURB must be an array");
   else {
      auto ltemp1=std::get<LIST_PTR>(nknots->exper_data);
      for (i=0;ltemp1!=nullptr;ltemp1=ltemp1->next,i++) ;

      if (i != ndata->nknots) {
         swarning("First knot vector has %d entries and should have %d, ",
                 i, ndata->nknots);
         haveknots = 0;
         }
      else
         haveknots = 1;
      }

   /* If there isn't a valid knot vector in the v direction, create an
      open uniform knot vector */
   if (haveknots) {
      auto ltemp1=std::get<LIST_PTR>(nknots->exper_data);
      for (i=0;ltemp1!=nullptr;ltemp1=ltemp1->next,++i) {
         if (eval_node(nullptr, ltemp1->element, &fval, vval, &nval) != 1)
            serror("Knot value isn't a floating point number");
         ndata->nknotvec[i] = fval;
         }
      }
   else {
      ndata->nknotvec[0] = 0.0;
      for (i=1;i<ndata->nknots;++i)
         if (i >= norder && i < npts + 1)
            ndata->nknotvec[i] = ndata->nknotvec[i-1] + 1;
         else
            ndata->nknotvec[i] = ndata->nknotvec[i-1];
      }

   /* Next, verify that the knot vector for the v direction has the right
      number of elements */
   if (mknots == nullptr)
      /* Create uniform knot vector for this direction */
      haveknots = 0;
   else if (mknots->exper_type != ARRAY)
      serror("Knot vector for NURB must be an array");
   else {
      auto ltemp1=std::get<LIST_PTR>(mknots->exper_data);
      for (j=0;ltemp1!=NULL;ltemp1=ltemp1->next,j++) ;
      if (j != ndata->mknots) {
         swarning("Second knot vector has %d entries and should have %d, ",
                 j, ndata->mknots);
         haveknots = 0;
         }
      else
         haveknots = 1;
      }

   /* If there isn't a valid knot vector in the v direction, create an
      open uniform knot vector */
   if (haveknots) {
      auto ltemp1=std::get<LIST_PTR>(mknots->exper_data);
      for (i=0;ltemp1!=nullptr;ltemp1=ltemp1->next,i++) {
         if (eval_node(nullptr, ltemp1->element, &fval, vval, &nval) != 1)
            serror("Knot value isn't a floating point number");
         ndata->mknotvec[i] = fval;
         }
      }
   else {
      ndata->mknotvec[0] = 0.0;
      for (i=1;i<ndata->mknots;i++)
         if (i >= morder && i < mpts + 1)
            ndata->mknotvec[i] = ndata->mknotvec[i-1] + 1;
         else
            ndata->mknotvec[i] = ndata->mknotvec[i-1];
      }

   /* Build the array of control points */
   if (ctlpts == nullptr || ctlpts->exper_type != ARRAY)
      serror("NURB control points must be a square array");
   auto ltemp1=std::get<LIST_PTR>(ctlpts->exper_data);
   ndata->Control_Points = (fourvec **)polyray_malloc(npts * sizeof(fourvec *));
   for (i=0;i<npts && ltemp1!=nullptr;i++,ltemp1=ltemp1->next) {
      ndata->Control_Points[i] = (fourvec *)
                                 polyray_malloc(mpts * sizeof(fourvec));
      if (ndata->Control_Points[i] == nullptr)
         serror("Failed to allocate NURB data");
      ntemp1 = ltemp1->element;
      if (ntemp1 == nullptr || ntemp1->exper_type != ARRAY)
         serror("NURB rows must be arrays of vectors");
      auto ltemp2=std::get<LIST_PTR>(ntemp1->exper_data);
      for (j=0;j<mpts&&ltemp2!=nullptr;j++,ltemp2=ltemp2->next) {
         /* Each entry in the control mesh must be either a vector or a
            or a vector with a homogenous component */
         ntemp1 = ltemp2->element;
         if ((ntemp1->exper_type != VECTOR_EXPER &&
              ntemp1->exper_type != VEC_EXPER) ||
             eval_node(NULL, ntemp1, &fval, vval, &nval) != 2) {
            smessage("NURB entries must be vectors, found(%d):\n",
                    ntemp1->exper_type);
            show_node(ntemp1);
            smessage("\n");
            serror("");
            }
         VecCopy(vval, ndata->Control_Points[i][j]);

         /* See if this is a homogenous vector */
         if (ntemp1->exper_type == VECTOR_EXPER) {
            auto vec=std::get<vvarr>(ntemp1->exper_data);
            ntemp2 = vec[3];
            if (ntemp2 != nullptr) {
               if (eval_node(nullptr, ntemp2, &fval, vval, &nval) != 1) {
                  smessage("Bad homogenous component of NURB entry (%d,%d):\n",
                          i, j);
                  smessage("Value = ");
                  show_node(ntemp1);
                  smessage("\n");
                  show_node(ntemp2);
                  smessage("\n");
                  fval = 1.0;
                  }
               ndata->rat_flag = 1;
               }
            else
               fval = 1.0;
            }
         else
            fval = 1.0;
         ndata->Control_Points[i][j][3] = fval;
         }
      if (j < mpts)
         serror("Too few column entries in NURB control mesh at row %d", i);
      if (ltemp2 != nullptr)
         serror("Too many column entries in NURB control mesh at row %d", i);
      }
   if (i < npts)
      serror("Too few row entries in NURB control mesh");
   if (ltemp1 != nullptr)
      serror("Too many row entries in NURB control mesh");

   /* Now go deallocate the input data */
   deallocate_node(nknots);
   deallocate_node(mknots);
   deallocate_node(ctlpts);

   /* Set the data part of the object to point to the NURB info */
   object->o_data = ndata;
   return object;
}

/**
 * @brief Initialize a NURB patch object with a trim-loop payload.
 *
 * The trim payload is currently parsed and shape-validated as an array of
 * loops, where each loop is expected to be an array. The trim curves are not
 * yet applied during intersection; this entry point exists to reserve the
 * syntax and provide a dedicated implementation path for the upcoming feature.
 *
 * @param object Object being initialized as a trimmed NURB primitive.
 * @param norder Order of the basis in the U direction.
 * @param npts Number of control points in the U direction.
 * @param morder Order of the basis in the V direction.
 * @param mpts Number of control points in the V direction.
 * @param nknots Optional knot vector expression for the U direction.
 * @param mknots Optional knot vector expression for the V direction.
 * @param ctlpts Control mesh expression for the patch.
 * @param trimloops Trim-loop payload expression.
 * @return `object` configured as a NURB patch.
 */
Object *MakeNurbTrimmed(Object *object, int norder, int npts, int morder, int mpts,
         NODE_PTR nknots, NODE_PTR mknots, NODE_PTR ctlpts, NODE_PTR trimloops)
{
   if (trimloops == nullptr || trimloops->exper_type != ARRAY)
      serror("NURB2 trim loops must be an array");

   auto loops = std::get<LIST_PTR>(trimloops->exper_data);
   for (int loop_index = 0; loops != nullptr; loops = loops->next, loop_index++) {
      if (loops->element == nullptr || loops->element->exper_type != ARRAY)
         serror("NURB2 trim loop %d must be an array", loop_index);
      }

   swarning("NURB2 trim curves are parsed but not applied yet\n");
   deallocate_node(trimloops);
   return MakeNurb(object, norder, npts, morder, mpts, nknots, mknots, ctlpts);
}

/**
 * @brief Release dynamic storage owned by a NURB patch object.
 *
 * @param obj NURB object whose owned buffers are being freed.
 * @return No return value.
 */
void NurbDelete(Object *obj)
{
   int i;
   NurbData *ndata = (NurbData *)obj->o_data;

   /* Only delete the memory if this is the original */
   if (obj->o_copy != 0) return;

   for (i=0;i<ndata->npts;++i)
      polyray_free(ndata->Control_Points[i]);
   polyray_free(ndata->Control_Points);
   polyray_free(ndata->nknotvec);
   polyray_free(ndata->mknotvec);
   polyray_free(ndata->nbasis);
   polyray_free(ndata->ndbasis);
   polyray_free(ndata->mbasis);
   polyray_free(ndata->mdbasis);

   delete ndata;
}

/**
 * @brief Compute B-spline basis values and first derivatives at one parameter.
 *
 * @param c Basis order.
 * @param t Parametric coordinate to evaluate.
 * @param npts Number of control points influenced by the basis.
 * @param x Knot vector.
 * @param basis Receives the basis function values.
 * @param dbasis Receives the basis function derivatives.
 * @return No return value.
 */
static void NurbDBasis(int c, Flt t, int npts, float *x,
           float *basis, float *dbasis)
{
   int i, k, nplusc;
   //float b1=0.0, b2=0.0, f1=0.0, f2=0.0, f3=0.0, f4=0.0;//cm251021 init all these to 0.0
   float b1, b2, f1, f2, f3, f4;//old version
   float numer, denom;

   nplusc = npts + c;

   for (i=0;i<nplusc;i++) {
      basis[i]  = 0.0;
      dbasis[i] = 0.0;
      }

   /* Calculate the first order basis functions */
   for (i=0;i<nplusc-1;i++)
      if (t >= x[i] && t < x[i+1])
         basis[i] = 1.0;
      else
         basis[i] = 0.0;
   if (t == x[nplusc-1])
      basis[npts-1] = 1.0; //Separate related bug: possible OOB index if npts <= 0 at bezier.cc (line 528) (basis[npts-1]).

   /* Calculate higher order basis functions and their derivatives */
   for (k=2;k<=c;k++) {
      for (i=0;i<nplusc-k;i++) {
         /* Calculate the basis function */
         if (basis[i] != 0.0) {
            numer = (t - x[i]) * basis[i];
            denom = x[i+k-1] - x[i];
            if (denom == 0.0)
               if (numer == 0.0)
                  b1 = 1.0;
               else
                  serror("Bad division: %g / %g\n", numer, denom);
            else
               b1 = numer / denom;
            }
         else
            b1 = 0.0;
         if (basis[i+1] != 0.0) {
            numer = (x[i+k] - t) * basis[i+1];
            denom = x[i+k] - x[i+1];
            if (denom == 0.0)
               if (numer == 0.0)
                  b2 = 1.0;
               else
                  serror("Bad division: %g / %g\n", numer, denom);
            else
               b2 = numer / denom;
            }
         else
            b2 = 0.0;

         /* Calculate the first derivative */
         if (basis[i] != 0.0) {
            denom = x[i+k-1] - x[i];
            if (denom == 0.0)
               serror("Bad division: %g / %g\n", basis[i], denom);
            f1 = basis[i] / denom;
            }
         else
            f1 = 0.0;
         if (basis[i+1] != 0.0) {
            denom = x[i+k] - x[i+1];
            if (denom == 0.0)
               serror("Bad division: %g / %g\n", basis[i+k], denom);
            f2 = -basis[i+1] / denom;
            }
         else
            f2 = 0.0;
         if (dbasis[i] != 0.0) {
            numer = (t - x[i]) * dbasis[i];
            denom = x[i+k-1] - x[i];
            if (denom == 0.0)
               if (numer == 0.0)
                  f3 = 1.0;
               else
                  serror("Bad division: %g / %g\n", numer, denom);
            else
               f3 = numer / denom;
            }
         else
            f3 = 0.0;
         if (dbasis[i+1] != 0.0) {
            numer = (x[i+k] - t) * dbasis[i+1];
            denom = x[i+k] - x[i+1];
            if (denom == 0.0)
               if (numer == 0.0)
                  f4 = 1.0;
               else
                  serror("Bad division: %g / %g\n", numer, denom);
            else
               f4 = numer / denom;
            }
         else
            f4 = 0.0;

         /* Save the results for this level */
         basis[i]  = b1 + b2;
         dbasis[i] = f1 + f2 + f3 + f4;
         }
      }
}

/* Determine the normal at a single coordinate point (u, v) on a bezier patch */
/**
 * @brief Determine the surface point and normal for a NURB patch sample.
 *
 * @param nurb NURB patch data containing knot vectors and control points.
 * @param u0 Parametric coordinate along the U direction.
 * @param v0 Parametric coordinate along the V direction.
 * @param P Receives the evaluated surface point.
 * @param N Receives the normalized surface normal.
 * @return No return value.
 */
static void NurbNormal(NurbData *nurb, Flt u0, Flt v0, Vec P, Vec N)
{
   float *nbasis, *ndbasis, *mbasis, *mdbasis;
   Flt t, homog;
   Flt D, Du, Dv;
   int i, j, nplusc, mplusc;
   Vec U, V, Nu, Nv;

   /* Calculate the basis functions */
   nplusc  = nurb->npts + nurb->norder;
   mplusc  = nurb->mpts + nurb->morder;
   nbasis  = nurb->nbasis;
   ndbasis = nurb->ndbasis;
   mbasis  = nurb->mbasis;
   mdbasis = nurb->mdbasis;

   NurbDBasis(nurb->norder, u0, nurb->npts, nurb->nknotvec, nbasis, ndbasis);
   NurbDBasis(nurb->morder, v0, nurb->mpts, nurb->mknotvec, mbasis, mdbasis);

   /* Now evaluate for this point */
   MakeVector(0, 0, 0, P);
   MakeVector(0, 0, 0, U);
   MakeVector(0, 0, 0, V);

   /* Check for a rational component */
   if (nurb->rat_flag) {
      MakeVector(0, 0, 0, Nu);
      MakeVector(0, 0, 0, Nv);

      D  = 0.0; Du = 0.0; Dv = 0.0;
      for (i=0;i<nurb->npts;i++)
         if (nbasis[i] != 0.0 || ndbasis[i] != 0.0)
            for (j=0;j<nurb->mpts;j++)
               if (mbasis[j] != 0.0 || mdbasis[j] != 0.0) {
                  /* Calculate denominator of the rational basis functions */
                  homog = nurb->Control_Points[i][j][3];
                  D  += homog * nbasis[i] * mbasis[j];
                  Du += homog * ndbasis[i] * mbasis[j];
                  Dv += homog * nbasis[i] * mdbasis[j];

                  /* Calculate the numerators of the rational basis functions */
                  t = homog * nbasis[i] * mbasis[j];
                  VecAddScaled(P, t, nurb->Control_Points[i][j], P);
                  t = homog * ndbasis[i] * mbasis[j];
                  VecAddScaled(Nu, t, nurb->Control_Points[i][j], Nu);
                  t = homog * nbasis[i] * mdbasis[j];
                  VecAddScaled(Nv, t, nurb->Control_Points[i][j], Nv);
                  }

      /* Now perform the final scaling and sums */
      D = 1.0 / D;
      VecScale(D, P);

      VecCopy(P, U);
      VecScale(D, U);
      VecScale(Du, U);
      VecScale(D, Nu);
      VecSub(Nu, U, U);

      VecCopy(P, V);
      VecScale(D, V);
      VecScale(Dv, V);
      VecScale(D, Nv);
      VecSub(Nv, V, V);
      }
   else {
      for (i=0;i<nurb->npts;i++)
         for (j=0;j<nurb->mpts;j++) {
            t = nbasis[i] * mbasis[j];
            VecAddScaled(P, t, nurb->Control_Points[i][j], P);
            t = ndbasis[i] * mbasis[j];
            VecAddScaled(U, t, nurb->Control_Points[i][j], U);
            t = nbasis[i] * mdbasis[j];
            VecAddScaled(V, t, nurb->Control_Points[i][j], V);
            }
      }
   VecCross(U, V, N);
   VecNormalize(N);
}

/* Evaluate a single coordinate point (u, v) on a bezier patch. */
/**
 * @brief Evaluate a NURB patch at one parametric coordinate.
 *
 * @param obj NURB object being sampled.
 * @param u0 Parametric coordinate along the U direction.
 * @param v0 Parametric coordinate along the V direction.
 * @param vert Receives the evaluated position, normal, and UV data.
 * @return No return value.
 */
static void NurbEvaluator(Object *obj, Flt u0, Flt v0, Vertex *vert)
{
   Vec P, N;
   NurbData *nurb = (NurbData *)obj->o_data;

   NurbNormal(nurb, u0, v0, P, N);

   VecCopy(P, vert->P);
   if (obj->o_trans) {
      TxVector(P, P, obj->o_trans);
      TxNormal(N, N, obj->o_trans);
      }
   VecNormalize(N);
   VecCopy(P, vert->W);
   MakeVector(u0, v0, 0.0, vert->U);
   VecCopy(N, vert->N);
}

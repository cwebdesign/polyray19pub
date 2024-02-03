/* bezier.c

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
#include "bezier.h"
#include "io.h"
#include "memory.h"
#include "builder.h"
#include "eval.h"
#include "vector.h"
#include "symtab.h"
#include "ytab.h"

/* Basis matrices for: Bezier, B-Spline, Catmull-Rom, and Hermite.  Note that
   for the first three, the vectors P0 - P3 are used.  For the last (Hermite)
   two vectors P0, P3 and two tangents R0, R3 are used.  */
static Matrix BZM = {{-1, 3,-3, 1}, { 3,-6, 3, 0},
                     {-3, 3, 0, 0}, { 1, 0, 0, 0}};
static Matrix BSM = {{-1, 3,-3, 1}, { 3,-6, 3, 0},
                     {-3, 0, 3, 0}, { 1, 4, 1, 0}};
static Matrix CRM = {{-1, 3,-3, 1}, { 2,-5, 4,-1},
                     {-1, 0, 1, 0}, { 0, 2, 0, 0}};
static Matrix HRM = {{ 2,-2, 1, 1}, {-3, 3,-2,-1},
                     { 0, 0, 1, 0}, { 1, 0, 0, 0}};

/* Representation of a bicubic patch is:
      Q(t) = S . M . G . Mt . Tt
   Where:
      S = [s^3, s^2, s, 1]
      T = [t^3, t^2, t, 1]
      M = Basis matrix from above (Mt is the transpose)
      G = [P0, P1, P2, P3] (or for Hermite, [P0, P3, R0, R3])
*/

/* This is a placeholder for primitive data */
typedef struct t_bezierdata {
   int patch_type;
   fVec Control_Points[4][4];
   } BezierData;

typedef float fourvec[4];

typedef struct t_nurbdata {
   int rat_flag;              /* Does this patch have any rational vertices */
   int norder, npts, nknots;
   int morder, mpts, mknots;
   float *nknotvec, *mknotvec;
   float *nbasis, *ndbasis;
   float *mbasis, *mdbasis;
   fourvec **Control_Points;
   } NurbData;


static void BezierEvaluater(Object *, float, float, Vertex *);
static int BezierIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                           Flt mindist, Flt maxdist, Isect *hit);
static int BezierInside(Object *obj, Vec P);

static ObjectProcs BezierProcs = {
   GenericRender,
   BezierEvaluater,
   GenericInitialize,
   BezierIntersect,
   BezierInside,
   GenericCopy,
   GenericDelete,
   };

static void NurbEvaluater(Object *, float, float, Vertex *);
static void NurbDelete(Object *);

static ObjectProcs NurbProcs = {
   GenericRender,
   NurbEvaluater,
   GenericInitialize,
   BezierIntersect,
   BezierInside,
   GenericCopy,
   NurbDelete,
   };

static int
BezierInside(Object *obj, Vec P)
{
   /* Not a csg primitive */
   return 0;
}

/* Determine the normal at a single coordinate point (u, v) on a bezier patch */
static void
BezierNormal(BezierData *shape, Flt u0, Flt v0, Vec P, Vec N)
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

void BezierNormalTest(void)
{
   BezierData *test1 = (BezierData *)polyray_malloc(sizeof(BezierData));
   for (int i=0;i<=5;i++) {
       test1->patch_type=i;
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
       printf("N=%f,%f,%f\n",N[0],N[1],N[2]);
//         std::cout<<"N="<<N[0]<<","<<N[1]<<","<<N[2]<<std::endl;
   }

}

Object *
MakeBezier(Object *object, int type, Flt flatness,
           int usteps, int vsteps, VList *points)
{
   int i, j;
   Vec mins, maxs;
   Flt t;
   BezierData *Bezier;

   object->o_type = T_BEZIER;
   object->o_procs = &BezierProcs;
   object->o_uv_steps[0] = usteps;
   object->o_uv_steps[1] = vsteps;

   /* Attempt to allocate memory for this primitive */
   if (points->count != 16)
      error("Must be 16 points on a Bezier patch, there are only: %d\n",
            points->count);
   if ((Bezier = (BezierData *)polyray_malloc(sizeof(BezierData))) == NULL)
      error("Failed to allocate Bezier data\n");
//   for (i=0;i<16;i++) {
   for (i=16;i--;) {
      VecCopy(points->points[i], Bezier->Control_Points[i/4][i%4]);
      }
   Bezier->patch_type = type;

   VecCopy(points->points[0], mins);
   VecCopy(mins, maxs);
   for (i=1;i<16;i++)
      for (j=0;j<3;j++) {
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

static int
BezierIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                Flt mindist, Flt maxdist, Isect *hit)
{
   /* Should never get here - the bezier patch is represented as
      a collection of triangles, each of which is handled by
      the high level intersection routine. */
   return 0;
}

/* Evaluate a single coordinate point (u, v) on a bezier patch. */
static void
BezierEvaluater(Object *obj, float u0, float v0, Vertex *vert)
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

Object *
MakeNurb(Object *object, int norder, int npts, int morder, int mpts,
         NODE_PTR nknots, NODE_PTR mknots, NODE_PTR ctlpts)
{
   int i, j, haveknots;
   Flt fval;
   Vec vval;
   NODE_PTR nval, ntemp1, ntemp2;
   LIST_PTR ltemp1, ltemp2;
   NurbData *ndata;

   object->o_type = T_NURB;
   object->o_procs = &NurbProcs;

   /* Set the uv bounds to reflect the bounds of the knot values (or of
      the control mesh if no knot vector was supplied */
   object->o_uv_bounds[0] = 0.0;
   object->o_uv_bounds[1] = npts - norder + 1;
   object->o_uv_bounds[2] = 0.0;
   object->o_uv_bounds[3] = mpts - morder + 1;
   object->o_uv_steps[0] = npts * 2;
   object->o_uv_steps[1] = mpts * 2;

   /* Attempt to allocate memory for this primitive */
   if ((ndata = (NurbData *)polyray_malloc(sizeof(NurbData))) == NULL)
      error("Failed to allocate NURB data\n");

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
   if (ndata->nknotvec == NULL || ndata->mknotvec == NULL ||
       ndata->nbasis == NULL || ndata->ndbasis == NULL ||
       ndata->mbasis == NULL || ndata->mdbasis == NULL)
      error("Failed to allocate NURB data");

   /* Count the number of elements in each knot vector */

   /* First verify that the knot vector for the u direction has the right
      number of elements */
   if (nknots == NULL)
      haveknots = 0;
   else if (nknots->exper_type != ARRAY)
      error("Knot vector for NURB must be an array");
   else {
      ltemp1 = nknots->exper_data.array;
      for (i=0;ltemp1!=NULL;ltemp1=ltemp1->next,i++) ;
      if (i != ndata->nknots) {
         warning("First knot vector has %d entries and should have %d, ",
                 i, ndata->nknots);
         haveknots = 0;
         }
      else
         haveknots = 1;
      }

   /* If there isn't a valid knot vector in the v direction, create an
      open uniform knot vector */
   if (haveknots) {
      ltemp1 = nknots->exper_data.array;
      for (i=0;ltemp1!=NULL;ltemp1=ltemp1->next,i++) {
         if (eval_node(NULL, ltemp1->element, &fval, vval, &nval) != 1)
            error("Knot value isn't a floating point number");
         ndata->nknotvec[i] = fval;
         }
      }
   else {
      ndata->nknotvec[0] = 0.0;
      for (i=1;i<ndata->nknots;i++)
         if (i >= norder && i < npts + 1)
            ndata->nknotvec[i] = ndata->nknotvec[i-1] + 1;
         else
            ndata->nknotvec[i] = ndata->nknotvec[i-1];
      }

   /* Next, verify that the knot vector for the v direction has the right
      number of elements */
   if (mknots == NULL)
      /* Create uniform knot vector for this direction */
      haveknots = 0;
   else if (mknots->exper_type != ARRAY)
      error("Knot vector for NURB must be an array");
   else {
      ltemp1 = mknots->exper_data.array;
      for (j=0;ltemp1!=NULL;ltemp1=ltemp1->next,j++) ;
      if (j != ndata->mknots) {
         warning("Second knot vector has %d entries and should have %d, ",
                 j, ndata->mknots);
         haveknots = 0;
         }
      else
         haveknots = 1;
      }

   /* If there isn't a valid knot vector in the v direction, create an
      open uniform knot vector */
   if (haveknots) {
      ltemp1 = mknots->exper_data.array;
      for (i=0;ltemp1!=NULL;ltemp1=ltemp1->next,i++) {
         if (eval_node(NULL, ltemp1->element, &fval, vval, &nval) != 1)
            error("Knot value isn't a floating point number");
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
   if (ctlpts == NULL || ctlpts->exper_type != ARRAY)
      error("NURB control points must be a square array");
   ltemp1 = ctlpts->exper_data.array;
   ndata->Control_Points = (fourvec **)polyray_malloc(npts * sizeof(fourvec *));
   for (i=0;i<npts && ltemp1!=NULL;i++,ltemp1=ltemp1->next) {
      ndata->Control_Points[i] = (fourvec *)
                                 polyray_malloc(mpts * sizeof(fourvec));
      if (ndata->Control_Points[i] == NULL)
         error("Failed to allocate NURB data");
      ntemp1 = ltemp1->element;
      if (ntemp1 == NULL || ntemp1->exper_type != ARRAY)
         error("NURB rows must be arrays of vectors");
      ltemp2 = ntemp1->exper_data.array;
      for (j=0;j<mpts&&ltemp2!=NULL;j++,ltemp2=ltemp2->next) {
         /* Each entry in the control mesh must be either a vector or a
            or a vector with a homogenous component */
         ntemp1 = ltemp2->element;
         if ((ntemp1->exper_type != VECTOR_EXPER &&
              ntemp1->exper_type != VEC_EXPER) ||
             eval_node(NULL, ntemp1, &fval, vval, &nval) != 2) {
            message("NURB entries must be vectors, found(%d):\n",
                    ntemp1->exper_type);
            show_node(ntemp1);
            message("\n");
            error("");
            }
         VecCopy(vval, ndata->Control_Points[i][j]);

         /* See if this is a homogenous vector */
         if (ntemp1->exper_type == VECTOR_EXPER) {
            ntemp2 = ntemp1->exper_data.vec[3];
            if (ntemp2 != NULL) {
               if (eval_node(NULL, ntemp2, &fval, vval, &nval) != 1) {
                  message("Bad homogenous component of NURB entry (%d,%d):\n",
                          i, j);
                  message("Value = ");
                  show_node(ntemp1);
                  message("\n");
                  show_node(ntemp2);
                  message("\n");
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
         error("Too few column entries in NURB control mesh at row %d", i);
      if (ltemp2 != NULL)
         error("Too many column entries in NURB control mesh at row %d", i);
      }
   if (i < npts)
      error("Too few row entries in NURB control mesh");
   if (ltemp1 != NULL)
      error("Too many row entries in NURB control mesh");

   /* Now go deallocate the input data */
   deallocate_node(nknots);
   deallocate_node(mknots);
   deallocate_node(ctlpts);

   /* Set the data part of the object to point to the NURB info */
   object->o_data = ndata;
   return object;
}

static void
NurbDelete(Object *obj)
{
   int i;
   NurbData *ndata = (NurbData *)obj->o_data;

   /* Only delete the memory if this is the original */
   if (obj->o_copy != 0) return;

   for (i=0;i<ndata->npts;i++)
      polyray_free(ndata->Control_Points[i]);
   polyray_free(ndata->Control_Points);
   polyray_free(ndata->nknotvec);
   polyray_free(ndata->mknotvec);
   polyray_free(ndata->nbasis);
   polyray_free(ndata->ndbasis);
   polyray_free(ndata->mbasis);
   polyray_free(ndata->mdbasis);

   polyray_free(ndata);
}

static void
NurbDBasis(int c, Flt t, int npts, float *x,
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
      basis[npts-1] = 1.0;

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
                  error("Bad division: %g / %g\n", numer, denom);
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
                  error("Bad division: %g / %g\n", numer, denom);
            else
               b2 = numer / denom;
            }
         else
            b2 = 0.0;

         /* Calculate the first derivative */
         if (basis[i] != 0.0) {
            denom = x[i+k-1] - x[i];
            if (denom == 0.0)
               error("Bad division: %g / %g\n", basis[i], denom);
            f1 = basis[i] / denom;
            }
         else
            f1 = 0.0;
         if (basis[i+1] != 0.0) {
            denom = x[i+k] - x[i+1];
            if (denom == 0.0)
               error("Bad division: %g / %g\n", basis[i+k], denom);
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
                  error("Bad division: %g / %g\n", numer, denom);
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
                  error("Bad division: %g / %g\n", numer, denom);
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
static void
NurbNormal(NurbData *nurb, Flt u0, Flt v0, Vec P, Vec N)
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
static void
NurbEvaluater(Object *obj, float u0, float v0, Vertex *vert)
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

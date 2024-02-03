/* function.c

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
#include "memory.h"
#include "io.h"
#include "intersec.h"
#include "symtab.h"
#include "mcube.h"
#include "vector.h"
#include "builder.h"
#include "bound.h"
#include "function.h"
#include "eval.h"

#define MAX_FN_ITERATIONS 20

typedef struct {
   int flag;            /* Set to 0 if the object hasn't been initialized */
   NODE_PTR fn;         /* Symbolic function */
   Vec deltas;          /* Size of each voxel */
   int sizes[3];        /* Number of sides of the containing box */
   } FunctionData;

void FunctionRender(Viewpoint *, BinTree *, Object *);
int FunctionIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int FunctionInside(Object *, Vec);
void FunctionCopy(Object *, Object *);
void FunctionDelete(Object *);

ObjectProcs FunctionProcs = {
   FunctionRender,
   NULL,
   GenericInitialize,
   FunctionIntersect,
   FunctionInside,
   GenericCopy,
   FunctionDelete,
   };

static int
FunctionNormal(Object *obj, Vec P, Vec N)
{
   int i;
   Flt fval;
   Vec vval;
   struct subst_struct subst, *sp;
   FunctionData *FnData = (FunctionData *)obj->o_data;
   NODE_PTR exper = FnData->fn;

   /* Solve for the partial with respect to x, then y, then z */
   sp = &subst;
   VecCopy(P, subst.P);
   MakeVector(1, 0, 0, subst.PT);
   i = eval_node_dx(sp, exper, &fval, vval);
   N[0] = (i == 1 ? fval : vval[0]);
   MakeVector(0, 1, 0, subst.PT);
   i = eval_node_dx(sp, exper, &fval, vval);
   N[1] = (i == 1 ? fval : vval[1]);
   MakeVector(0, 0, 1, subst.PT);
   i = eval_node_dx(sp, exper, &fval, vval);
   N[2] = (i == 1 ? fval : vval[2]);

   /* Make the gradient point out of the function */
   VecScale(-1.0, N);
   (void)VecNormalize(N);

   return 1;
}

static void
InitializeFunction(Object *obj)
{
   FunctionData *FnData = (FunctionData *)obj->o_data;

   FnData->flag = 1;

   FnData->sizes[0] = obj->o_uv_steps[0];
   FnData->sizes[1] = obj->o_uv_steps[1];
   FnData->sizes[2] = obj->o_uv_steps[2];

   FnData->deltas[0] = obj->o_bnd.lengths[0] / (Flt)(FnData->sizes[0] - 1);
   FnData->deltas[1] = obj->o_bnd.lengths[1] / (Flt)(FnData->sizes[1] - 1);
   FnData->deltas[2] = obj->o_bnd.lengths[2] / (Flt)(FnData->sizes[2] - 1);
}

void
Compute_Step_Values(Vec deltas, int sizes[3], Vec D,
                    int steps[3], int highs[3], float dx[3])
{
   int i, tsizes[3];

   for (i=0;i<3;i++)
      tsizes[i] = sizes[i] - 1;

   /* Set the step direction, step size, and exit condition
      for each axis */
   for (i=0;i<3;i++)
      if (D[i] < 0.0) {
         steps[i] = -1;
         highs[i] = -1;
         dx[i] = -deltas[i] / D[i];
         }
      else if (D[i] > 0.0) {
         steps[i] = 1;
         highs[i] = tsizes[i];
         dx[i] = deltas[i] / D[i];
         }
      else {
         steps[i] = 0;
         highs[i] = -1;
         dx[i] = 0.0;
         }
}

void
Compute_DDA_Start(Vec deltas, int sizes[3], bbox_info *bbox,
                  Vec hitpos, Vec D,
                  int x[3], Flt fx[3])
{
   Flt offset;
   int i;

   /* Figure out what voxel we are starting at */
   for (i=0;i<3;i++) {
      x[i] = (hitpos[i] - bbox->lower_left[i]) / deltas[i];
      if (x[i] >= sizes[i])
         x[i] = sizes[i] - 1;
      }

   /* Set the initial difference values along each axis */
   for (i=0;i<3;i++) {
      offset = (x[i] * deltas[i]) + bbox->lower_left[i];
      if (D[i] < 0.0)
         fx[i] = (offset - hitpos[i]) / D[i];
      else if (D[i] > 0.0)
         fx[i] = ((offset + deltas[i]) - hitpos[i]) / D[i];
      else
         fx[i] = PLY_HUGE;
      }
}

static int
check_fn_hit(Object *obj, Ray *ray, Isect *hit,
             Flt mindist, Flt maxdist,
             Vec Low, Vec High, Flt *lastf)
{
   FunctionData *FnData = (FunctionData *)obj->o_data;
   struct subst_struct subst, *sp;
   NODE_PTR fn, tempn;
   Vec N, P, P0, P1, P2, vval;
   Flt d, v, x, v0, v1, v2;
   int i, Flag = 0;

   fn = FnData->fn;
   sp = &subst;

   VecCopy(Low, P0);
   VecCopy(High, P1);

   /* See if there is an intersection between v0 and v1 */
   if (*lastf == -PLY_HUGE) {
      VecCopy(P0, subst.P);
      MakeVector(0, 0, 0, subst.PT);
      if (eval_node(sp, fn, &v0, vval, &tempn) != 1)
         return 0;
      }
   else
      v0 = *lastf;

   VecCopy(P1, subst.P);
   if (eval_node(sp, fn, &v1, vval, &tempn) != 1)
      return 0;
   *lastf = v1;

   if (v0 * v1 > 0.0)
      /* Same sign at both ends - no intersection possible */
      return 0;

   /* The sign of v0 and v1 changes between P0 and P1, this
      means there is an intersection point in there somewhere.
      We now use binary search to close in on it. */
   for (i=0;i<MAX_FN_ITERATIONS;i++) {
      if (fabs(v0) < EPSILON) {
         /* Near point is close enough to an intersection - just
            use it. */
         v = v0;
         VecCopy(P0, P);
         break;
         }
      else if (fabs(v1) < EPSILON) {
         /* Far point is close enough to an intersection */
         v = v1;
         VecCopy(P1, P);
         break;
         }
      else {
         /* Continue bisecting */
         x = fabs(v0) / fabs(v1 - v0); /* Assume a line between the points */
         VecSub(P1, P0, P2);
         VecAddScaled(P0, x, P2, P2);
         VecCopy(P2, subst.P);
         if (eval_node(sp, fn, &v2, vval, &tempn) != 1)
            return 0;
         if (v0 * v2 > 0.0)
            if (v1 * v2 > 0.0) {
               /* This should be impossible, since v0 and v1
                  were opposite signs, v2 must be either 0 or
                  opposite in sign to either v0 or v1 */
               error("internal failure in function bisection");
               }
            else {
               v0 = v2;
               VecCopy(P2, P0);
               }
         else {
            v1 = v2;
            VecCopy(P2, P1);
            }
         }
      }

   if (i == MAX_FN_ITERATIONS) {
      /* The loop never quite closed in on the result - just
         use the point closest to zero. */
      if (fabs(v0) < fabs(v1)) {
         v = v0;
         VecCopy(P0, P);
         }
      else {
         v = v1;
         VecCopy(P1, P);
         }
      }

   VecSub(P, ray->P, P0);
   d = sqrt(VecDot(P0, P0));

   /* See if it is a valid intersection */
   if (d > mindist && d < maxdist) {
      FunctionNormal(obj, P, N);
      if (Insert_Hit(obj, P, N, d, P, hit))
         Flag = 1;
      }

   return Flag;
}

int
FunctionIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                  Flt mindist, Flt maxdist, Isect *hit)
{
   FunctionData *FnData = (FunctionData *)obj->o_data;
   NODE_PTR exper;
   Flt mind, maxd;
   int i, index, Flag;
   int x[3], steps[3], highs[3];
   float dx[3];
   bbox_info bbox;
   Flt fx[3], lastf, boundbox[2][3];
   Vec P, D, hitpos, pDX[3], nxp[3];

   exper = FnData->fn;
   VecCopy(ray->P, P);
   VecCopy(ray->D, D);

   /* See if this function has been initialized yet */
   if (FnData->flag == 0)
      InitializeFunction(obj);

   /* Find where we hit the bounding box around the height field */
   mind = mindist;
   maxd = maxdist;
   VecCopy(obj->o_bnd.lower_left, bbox.lower_left);
   VecCopy(obj->o_bnd.lengths, bbox.lengths);
   recompute_inverse_bbox(&bbox, obj->o_trans);
   VecCopy(bbox.lower_left, boundbox[0]);
   VecAdd(boundbox[0], bbox.lengths, boundbox[1]);
   if (determine_start(P, D, boundbox, &mind, &maxd))
      VecAddScaled(P, mind, D, hitpos)
   else
      return 0;

   Compute_Step_Values(FnData->deltas, FnData->sizes,
                       D, steps, highs, dx);
   for (i=0;i<3;i++) {
      VecCopy(D, pDX[i])
      VecScale(dx[i], pDX[i]);
      }

   Compute_DDA_Start(FnData->deltas, FnData->sizes, &obj->o_bnd,
                     hitpos, D, x, fx);
   for (i=0;i<3;i++)
      VecAddScaled(hitpos, fx[i], D, nxp[i]);

   /* Now walk the voxels using a 3D-DDA */
   Flag = 0;
   lastf = -PLY_HUGE;
   for (;;) {
      /* Determine which direction to step */
      if ((fx[0] < fx[1]) && (fx[0] < fx[2]))
         index = 0;
      else if (fx[1] < fx[2])
         index = 1;
      else
         index = 2;

      /* Check for an intersection between "hitpos" and "nxp" */
      if (check_fn_hit(obj, ray, hit, mindist, maxdist,
                       hitpos, nxp[index], &lastf)) {
            Flag = 1;
            break;
            }

      x[index] += steps[index];
      if ((maxdist < fx[index]) || (x[index] == highs[index]))
         break;
      fx[index] += dx[index];
      VecCopy(nxp[index], hitpos);
      VecAdd(nxp[index], pDX[index], nxp[index]);
      }

   return Flag;
}

Object *
MakeFunction(Object *object, NODE_PTR data)
{
   FunctionData *FnData = (FunctionData *)object->o_data;

   object->o_type = T_FUNCTION;
   object->o_procs = &FunctionProcs;
   FnData = (FunctionData *)polyray_malloc(sizeof(FunctionData));
   if (FnData == NULL)
      error("Failed to allocate function information");
   FnData->fn = data;
   FnData->flag  = 0;
   object->o_data = FnData;
   MakeVector(-1, -1, -1, object->o_bnd.lower_left);
   MakeVector( 2,  2,  2, object->o_bnd.lengths);
   object->o_uv_steps[0] = 20;
   object->o_uv_steps[1] = 20;
   object->o_uv_steps[2] = 20;
   return object;
}

void
FunctionDelete(object)
   Object *object;
{
   FunctionData *FnData = (FunctionData *)object->o_data;

   /* Only delete the memory if this is the original */
   if (object->o_copy != 0)
      return;

   /* Free the symbolic function */
   deallocate_node(FnData->fn);

   /* Free the function structure itself */
   polyray_free(FnData);
}

int
FunctionInside(Object *obj, Vec Pos)
{
   Vec vval;
   NODE_PTR tempn;
   Vec P;
   Flt Result;
   struct subst_struct subst;
   FunctionData *FnData = (FunctionData *)obj->o_data;

   /* See if this function has been initialized yet */
   if (FnData->flag == 0)
      InitializeFunction(obj);

   /* Transform the point into function space */
   InvTxVector1(P, Pos, obj->o_trans)
   VecCopy(P, subst.P);
   MakeVector(0, 0, 0, subst.PT);
   eval_node(&subst, FnData->fn, &Result, vval, &tempn);
   return (Result < 0 ? 1 : 0);
}

static Flt
function_value(Object *obj, Vec Pos)
{
   struct subst_struct subst;
   Vec P;
   Flt v1;
   Vec vval;
   NODE_PTR tempn;
   FunctionData *FnData = (FunctionData *)obj->o_data;

   if (obj->o_trans)
      InvTxVector1(P, Pos, obj->o_trans)
   else
      VecCopy(Pos, P);
   VecCopy(P, subst.P);
   MakeVector(0, 0, 0, subst.PT);

   eval_node(&subst, FnData->fn, &v1, vval, &tempn);
   return v1;
}

void
FunctionRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   MarchCubes(eye, Root, obj->o_uv_steps[0], obj->o_uv_steps[1],
              obj->o_uv_steps[2], &(obj->o_bnd),
              0.0, function_value,
              FunctionNormal, obj);
              /* NULL, obj); */
}

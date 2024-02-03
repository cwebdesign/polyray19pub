/* parametr.c

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
#include "parametr.h"
#include "io.h"
#include "memory.h"
#include "eval.h"
#include "vector.h"
#include "symtab.h"
#include "builder.h"

/* This is a placeholder for primitive data */
typedef struct t_parametricdata {
   NODE_PTR formula;
   } ParametricData;

static void ParametricEvaluater(Object *, float, float, Vertex *);
static int ParametricIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
static int ParametricInside(Object *obj, Vec P);
static void ParametricDelete(Object *);

static ObjectProcs ParametricProcs = {
   GenericRender,
   ParametricEvaluater,
   GenericInitialize,
   ParametricIntersect,
   ParametricInside,
   GenericCopy,
   ParametricDelete,
   };

void
ParametricDelete(Object *object)
{
   ParametricData *shape = (ParametricData *)object->o_data;

   /* Only delete the memory if this is the original */
   if (object->o_copy != 0)
      return;

   /* Free the symbolic function */
   deallocate_node(shape->formula);

   /* Free the function structure itself */
   polyray_free(shape);
}

static int
ParametricInside(Object *obj, Vec P)
{
   /* There are a lot of things that can be expressed as parametric
      equations that are closed - we really ought to use Jordan's rule. */
/*CM*/
/* Use Jordans rule for inside/outside testing but how?
   Ray ray;
   InvTxVector1(ray.P, P, obj->o_trans)  ray.P -- start location of ray
   MakeVector(0.12345, 0.98765, 0.57392, ray.D);  ray.D -- direction of ray
   VecNormalize(ray.D);
 END CM*/
   return 0;
}

Object *
MakeParametric(Object *object, NODE_PTR formula)
{
   ParametricData *shape;

   object->o_type = T_PARAMETRIC;
   object->o_procs = &ParametricProcs;
   object->o_uv_steps[0] = 32;
   object->o_uv_steps[1] = 32;
   object->o_uv_steps[2] = 32;
   object->o_uv_bounds[0] = 0.0;
   object->o_uv_bounds[1] = 1.0;
   object->o_uv_bounds[2] = 0.0;
   object->o_uv_bounds[3] = 1.0;

   if ((shape = (ParametricData *)
                polyray_malloc(sizeof(ParametricData))) == NULL)
      error("Failed to allocate Parametric data\n");
   shape->formula = formula;
   object->o_data = (void *)shape;

#if 0
   /* Create a bag of triangles... */
   Uniform_Subdivide(Viewpoint *eye, BinTree *Root, Object *obj)
#endif

   return object;
}

static int
ParametricIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                    Flt mindist, Flt maxdist, Isect *hit)
{
   /* This object should be represented as a big bunch of triangles.
      Code should never get here... */
   return 0;
}

/* Evaluate a single coordinate point (u, v) on a bezier patch. */
static void
ParametricEvaluater(Object *obj, float u0, float v0, Vertex *vert)
{
   int i;
   Flt fval;
   Vec P, N, Nu, Nv;
   NODE_PTR nval;
   struct subst_struct subst, *sp;
   ParametricData *shape = (ParametricData *)obj->o_data;

   /* Set default values for the evaluation structure */
   sp = &subst;
   reset_subst(sp);
   MakeVector(u0, v0, 0, subst.U);

   if (eval_node(&subst, shape->formula, &fval, P, &nval) != 2)
      error("Non vector formula for parametric patch");

   VecCopy(P, subst.P);
   MakeVector(1, 0, 0, subst.UT);
   i = eval_node_dx(sp, shape->formula, &N[0], Nu);
   MakeVector(0, 1, 0, subst.UT);
   i = eval_node_dx(sp, shape->formula, &N[1], Nv);

   VecCross(Nu, Nv, N);

   /* Make the gradient point out of the function */
   VecScale(-1.0, N);
   (void)VecNormalize(N);

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

#if 0
/* Recursively descend into the tree of raw triangles and spit them out */
static void
ParametricRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   ostackptr objs;
   ParametricData *raw = obj->o_data;

   for (objs=raw->objs.members.list;objs!=NULL;objs=objs->next)
      render_prim(eye, Root, obj, objs->element);
}
#endif

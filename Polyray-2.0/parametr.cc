/** @file parametr.cc
 *  @brief Parametric surface primitive - expression-driven tessellation.
 *
 *  A parametric surface is defined by a vector-valued expression f(u, v)
 *  supplied at parse time.  The object has no analytic intersection routine;
 *  it is rendered exclusively by adaptive UV subdivision into triangles via
 *  GenericRender / ParametricEvaluator.
 *
 *  Polyray - MIT Licensed Revival
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
#include "parametr.h"
#include "io_ply.h"
#include "memory.h"
#include "eval.h"
#include "vector.h"
#include "symtab.h"
#include "builder.h"
#include "factory.h"



static void ParametricEvaluator(Object *, Flt, Flt, Vertex *);
static int ParametricIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
static int  ParametricInside(Object *obj, Vec P);


static openpolyray::dispatch::ObjectProcs ParametricProcs = {
   .render = GenericRender,
   .evaluate = ParametricEvaluator,
   .initialize = GenericInitialize,
   .intersect = ParametricIntersect,
   .inside = ParametricInside,
   .copy = GenericCopy,
   .del = ParametricDelete,
   };

/** @brief Free parametric-surface-specific data owned by @p object.
 *
 *  Skips deallocation when o_copy is non-zero (shared copy).
 *  Otherwise deallocates the formula expression tree and the ParametricData struct.
 *  @param object  Parametric surface object to clean up.
 */
void ParametricDelete(Object *obj)
{
   ParametricData *shape = static_cast<ParametricData*>(obj->o_data);

   /* Only delete the memory if this is the original */
   if (obj->o_copy != 0)
      return;

   /* Free the symbolic function */
   deallocate_node(shape->formula);

   /* Free the function structure itself */
   delete static_cast<ParametricData*>(obj->o_data);
   obj->o_data = nullptr;
}

/** @brief Test whether a point lies inside the parametric surface volume.
 *
 *  Not implemented - always returns 0.  A correct implementation would
 *  require Jordan's ray-crossing rule applied to the tessellated mesh.
 *  @param obj  Parametric surface object (unused).
 *  @param P    World-space point to test (unused).
 *  @return     Always 0 (outside).
 */
static int ParametricInside(Object *obj, Vec P)
{
   /* There are a lot of things that can be expressed as parametric
      equations that are closed - we really ought to use Jordan's rule. */
/*CM*/
//todo: fix parametric equation to use Jordan's rule
/* Use Jordans rule for inside/outside testing but how?
   Ray ray;
   InvTxVector1(ray.P, P, obj->o_trans)  ray.P -- start location of ray
   MakeVector(0.12345, 0.98765, 0.57392, ray.D);  ray.D -- direction of ray
   VecNormalize(ray.D);
 END CM*/
   return 0;
}

/** @brief Initialise an Object as a parametric surface primitive.
 *
 *  Attaches @p formula as the surface definition, sets UV bounds to [0,1]x[0,1]
 *  with 32x32 tessellation steps, and marks the object as T_PARAMETRIC.
 *  No bounding box is computed here; it is determined during tessellation.
 *  @param object   Pre-allocated Object to configure.
 *  @param formula  Vector-valued expression f(u,v) defining the surface;
 *                  ownership transferred to the object.
 *  @return         @p object configured as T_PARAMETRIC.
 */
Object *MakeParametric(Object *object, NODE_PTR formula)
{
   
   object->o_type = ShapeType::Parametric;
   object->o_procs = &ParametricProcs;
   object->o_uv_steps[0] = 32;
   object->o_uv_steps[1] = 32;
   object->o_uv_steps[2] = 32;
   object->o_uv_bounds[0] = 0.0;
   object->o_uv_bounds[1] = 1.0;
   object->o_uv_bounds[2] = 0.0;
   object->o_uv_bounds[3] = 1.0;

   ParametricData* shape = FactoryParametricData();
   shape->formula = formula;
   object->o_data = (void *)shape;

#if 0
   /* Create a bag of triangles... */
   Uniform_Subdivide(Viewpoint *eye, BinTree *Root, Object *obj)
#endif

   return object;
}

/** @brief Ray-parametric-surface intersection (stub - always returns 0).
 *
 *  Parametric surfaces are represented entirely as tessellated triangles;
 *  direct ray intersection is not implemented and this function should
 *  never be reached during normal rendering.
 *  @param Eye      Active viewpoint (unused).
 *  @param obj      Parametric surface object (unused).
 *  @param ray      Incoming ray (unused).
 *  @param mindist  Minimum valid ray parameter (unused).
 *  @param maxdist  Maximum valid ray parameter (unused).
 *  @param hit      Intersection accumulator (unused).
 *  @return         Always 0.
 */
static int ParametricIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                    Flt mindist, Flt maxdist, Isect *hit)
{
   /* This object should be represented as a big bunch of triangles.
      Code should never get here... */
   return 0;
}

/** @brief Evaluate a point and normal on the parametric surface at (u0, v0).
 *
 *  Evaluates the stored formula f(u,v) to obtain the surface position @c P,
 *  then uses eval_node_dx() to compute the partial derivatives df/du and
 *  df/dv.  The surface normal is their cross product, negated so it points
 *  outward, and normalised.
 *  @param obj   Parametric surface object.
 *  @param u0    Horizontal parametric coordinate in [0, 1].
 *  @param v0    Vertical parametric coordinate in [0, 1].
 *  @param vert  Output vertex receiving object-space position (@c P),
 *               world-space position (@c W), surface normal (@c N),
 *               and UV coordinates (@c U).
 */
static void ParametricEvaluator(Object *obj, Flt u0, Flt v0, Vertex *vert)
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
      serror("Non vector formula for parametric patch");

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

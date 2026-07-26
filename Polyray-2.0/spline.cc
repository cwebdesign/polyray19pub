/* spline.cc

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
#include "defs3.h" //spline struct has been moved to defs3.h
#include "polyray.tab.h"
#include "memory.h"
#include "io_ply.h"
#include "builder.h"
#include "factory.h"
#include "vector.h"
#include "eval.h"
#include "spline.h"

/* Type definition for spline data only needs to be visible to this module */
//no longer!

/* Calculate the value of a spline that passes
   through a set of control points.  The type of the spline can be either
   a linear fit (polyline) between control points or a parabolically blended
   cubic  The set of control points is passed in as well as a parameter
   value for what position on the spline is needed.

   The parameter value for the spline is from 0 - 1.  Since this value is
   for the entire spline, it is allocated to a particular part of the
   spline by looking at a chord length approximation between control
   points.

   After that is done, the new value is used as a parameter for
   the individual cubic spline that goes through the four control points
   that surround it.

   The parameters have the following meaning:

      spline_type     0) Cubic spline interpolating all control points
                      1) Cubic spline that loops back from the last control
                         point to the first.
                      2) Linear interpolation between control points
                      3) Linear interpolation with loop from last to first
                      4) Cubic spline interpolating all control points, each
                         control point contains a fourth value stored in the
                         local ctl_params that defines the parameter distance
                         between a control point and the next control point.
                      5) Quadratic Bezier spline - the control parameters
                         indicate if each point is on or off the curve.

      t               The point along the entire splined path (between 0 for the
                      first control point and 1 for the last control point)

      num_ctl_points  The total number of control points being passed

      ctl_points      An array of control points (each of which has three
                      components, <x,y,z>).

      ctl_params      An array of control parameters (effectively the distance
                      to the next control point)

      spline_value    The resulting point on the curve

*/
/** @brief Evaluate a spline at a normalized parameter value.
 *  @param spline_type    Spline interpolation mode.
 *  @param t              Normalized parameter in the spline domain.
 *  @param num_ctl_points Number of control points.
 *  @param ctl_points     Control point array.
 *  @param dist           Total parameter distance across the spline.
 *  @param ctl_params     Per-segment parameter distances.
 *  @param ctl_derivs     Control-point derivative array for Hermite-style splines.
 *  @param spline_value   Output point on the spline.
 *  @return No return value.
 */
static void evaluate_spline(int spline_type, float t, int num_ctl_points,
                 Vec *ctl_points, Flt dist, Flt *ctl_params,
                 Vec *ctl_derivs, Vec spline_value)
{
   int i;
   Flt t0, t1, t2, t3;
   Flt d0, d1, d2, d3;
   Flt st;
   Vec P0, P1, P2, P3;

   /* Handle special cases for low numbers of control points */
   if (num_ctl_points <= 0) {
      /* No points - just return <0,0,0> */
      MakeVector(0, 0, 0, spline_value)
      return;
      }
   else if (num_ctl_points == 1) {
      /* Single point */
      VecCopy(ctl_points[0], spline_value)
      return;
      }

   /* Wrap the parameter value into the range 0 -> 1 */
   if (spline_type == 1 || spline_type == 3) {
      t = fmod(t, 1.0);
      if (t < 0.0)
         t += 1.0;
      }
   else {
      if (t < 0.0)
         t = 0.0;
      else if (t > 1.0)
         t = 1.0;
      }

   if (num_ctl_points == 2 && spline_type < 4) {
      /* Straight line between two points, easy to interpolate. */
      if (t <= 0)
         VecCopy(ctl_points[0], spline_value)
      else if (t >= 1)
         VecCopy(ctl_points[1], spline_value)
      else {
         t0 = t;
         t1 = 1.0 - t;
         VecComb(t0, ctl_points[0], t1, ctl_points[1], spline_value)
         }
      return;
      }

   /* Handle two special cases: t == 0 and t == 1 */
   if (t <= 0.0) {
      /* t is at the beginning of the spline. */
      VecCopy(ctl_points[0], spline_value)
      return;
      }
   else if (t >= 1.0) {
      /* t is at the end of the spline */
      VecCopy(ctl_points[num_ctl_points-1], spline_value)
      return;
      }

   /* dist now has the entire length of the path (in terms of the distances
      between control points.  chord_dist has the distances between each
      control point and the next control point.  From here, we figure out
      which two control points the value of t should be between.  */
   for (i=0,d0=t*dist,d1=0;i<=num_ctl_points-1;i++) {
      d1 += ctl_params[i];
      if (d0 <= d1)
         /* Found the right spot */
         break;
      }
   if (i == num_ctl_points)
      serror("Failed to properly interpolate spline at %g\n", t);

   /* t lies between control point i and i+1.  Now figure out what fraction
      of the distance between these control points it is at.  */
   st = 1.0 - (d1 - d0) / ctl_params[i];

   if (spline_type == 0 || spline_type == 1) {
      /* Figure out the coefficients of the cubic spline that passes through
         the control points: i-2, i-1, i, i+1.  There are a few special cases:
         i == 0, i == 1, and i == num_ctl_points-1.  The first happens when
         t == 0 and should already have been handled as a special case.  The
         second and last happen when t is inside either the first or last
         segment of the spline and need some extra attention. */
      if (spline_type == 1) {
         VecCopy(ctl_points[i  ], P1)
         if (i == 0) {
            VecCopy(ctl_points[num_ctl_points-1], P0)
            VecCopy(ctl_points[i+1], P2)
            VecCopy(ctl_points[i+2], P3)
            }
         else if (i == num_ctl_points-2) {
            VecCopy(ctl_points[i-1], P0)
            VecCopy(ctl_points[i+1], P2)
            VecCopy(ctl_points[0], P3)
            }
         else if (i == num_ctl_points-1) {
            VecCopy(ctl_points[i-1], P0)
            VecCopy(ctl_points[0], P2)
            VecCopy(ctl_points[1], P3)
            }
         else {
            VecCopy(ctl_points[i-1], P0)
            VecCopy(ctl_points[i+1], P2)
            VecCopy(ctl_points[i+2], P3)
            }
         }
      else {
         VecCopy(ctl_points[i  ], P1)
         VecCopy(ctl_points[i+1], P2)
         if (i == 0) {
            /* Make P0 by extending the first segment backwards */
            VecSub(P2, P1, P0)
            VecSub(P1, P0, P0)
            /* No need to adjust P3 */
            VecCopy(ctl_points[i+2], P3)
            }
         else if (i == num_ctl_points-2) {
            /* Make P3 by extending the third segment forwards */
            VecSub(P2, P1, P3)
            VecAdd(P2, P3, P3)
            /* No need to adjust P0 */
            VecCopy(ctl_points[i-1], P0)
            }
         else {
            /* No special processing for P0 or P3 */
            VecCopy(ctl_points[i-1], P0)
            VecCopy(ctl_points[i+2], P3)
            }
         }

      /* Now we have the four control points for the piece of the spline we
         are really interested in.  From these we now calculate the coefficients
         of the cubic spline that passes through P1 and P2, and is wiggled by
         P0 and P3.  An alternative to calculating alpha and beta based on
         chord length is to simply set both to 0.5 (not quite as fair a spline,
         but often avoids unwanted cusps in the spline) */
      t1 = st; t2 = t1 * st; t3 = t2 * st; /* 1st, 2nd, and 3rd power of st */
      d0 =       -t3 + 2.0 * t2 - t1;
      d1 =  3.0 * t3 - 5.0 * t2 +     2.0;
      d2 = -3.0 * t3 + 4.0 * t2 + t1;
      d3 =        t3 -       t2;

      for (i=0;i<3;i++)
         /* Figure out the splined values of each component - the 0.5 is
            applied equally to d0-d3, so instead of 4 multiplications, it
            is used once here (which is only 3 multiplications). */
         spline_value[i] = 0.5 * (d0 * P0[i] + d1 * P1[i] +
                                  d2 * P2[i] + d3 * P3[i]);
      }
   else if (spline_type == 2 || spline_type == 3) {
      /* Do a linear interpolation between the two control points */
      if (spline_type == 3) {
         VecCopy(ctl_points[i], P0)
         if (i == num_ctl_points-1)
            VecCopy(ctl_points[0], P1)
         else
            VecCopy(ctl_points[i+1], P1)
         }
      else {
         VecCopy(ctl_points[i], P0)
         VecCopy(ctl_points[i+1], P1)
         }
      VecSub(P1, P0, P2);
      VecAddS(st, P2, P0, spline_value)
      }
   else {
      /* t1 is the fraction of distance st */
      t1 = st;
      t2 = t1 * t1;
      t3 = t2 * t1;

      VecCopy(ctl_points[i],   P0)
      VecCopy(ctl_derivs[i],   P2)
      if (spline_type == 4 && i == num_ctl_points - 1)
         serror("Went too far in the spline...");
      VecCopy(ctl_points[i+1], P1)
      VecCopy(ctl_derivs[i+1], P3)
      
      d0 =  2.0 * t3 - 3.0 * t2 + 1.0;
      d1 = -2.0 * t3 + 3.0 * t2;
      d2 = (t3 - 2.0 * t2 + t1) * ctl_params[i];
      d3 = (t3 - t2) * ctl_params[i];
      
      for (i=0;i<3;i++)
         spline_value[i] = d0 * P0[i] + d1 * P1[i] +
                           d2 * P2[i] + d3 * P3[i];
      }

   /* All done - wasn't that easy? */
}

/* Solve a tridiagonal system of equations.
   This is used for determing the set of coefficients needed
   for evaluation of a cubic spline.  The meaning of the
   parameters is:
      a - lower diagonal values
      b - center diagonal values
      c - upper diagonal values
      r - rhs values
      u - solution values

   The tridiagonal system looks like:
      | b0 c0  0  ...                      |   | u0     |   | r0     |
      | a1 b1 c1  ...                      |   | u1     |   | r1     |
      |           ...                      | * |  ...   | = |  ...   |
      |           ... a(n-2) b(n-2) c(n-2) |   | u(n-2) |   | r(n-2) |
      |           ... 0      a(n-1) b(n-1) |   | u(n-1) |   | r(n-1) |

*/
/** @brief Solve a tridiagonal linear system for vector-valued unknowns.
 *  @param a Lower diagonal coefficients.
 *  @param b Main diagonal coefficients.
 *  @param c Upper diagonal coefficients.
 *  @param r Right-hand-side vectors.
 *  @param u Output solution vectors.
 *  @param n System size.
 *  @return No return value.
 */
static void tridiag(Flt *a, Flt *b, Flt *c, Vec *r, Vec *u, int n)
{
   int i, j;
   float beta;
   Flt *gamma;

   gamma = (Flt*)polyray_malloc(n * sizeof(Flt));

   for (j=0;j<3;j++) {
      if (b[0] == 0.0)
         serror("Bad spline system\n");
      beta = b[0];
      u[0][j] = r[0][j] / beta;
      for (i=1;i<n;i++) {
         gamma[i] = c[i-1] / beta;
         beta = b[i] - a[i] * gamma[i];
         if (beta == 0.0)
            serror("Bad spline system\n");
         u[i][j] = (r[i][j] - a[i] * u[i-1][j]) / beta;
         }

      for (i=n-2;i>=0;i--)
         u[i][j] -= gamma[i+1] * u[i+1][j];
      }

   polyray_free(gamma);
}

/* For spline type 4 we need to calculate the natural gradients through
   each control point.  The gradients (tangents) are given for the first
   and last points.  This routine solves for the ones in the middle. */
/** @brief Compute interior control-point derivatives for type-4 splines.
 *  @param spn Spline node whose derivative array is updated in place.
 *  @return No return value.
 */
static void calculate_gradients(spline_node *spn)
{
   int i, n, spline_type;
   Flt x, y, t;
   Vec P, P0, P1, *ctl_points;
   Flt *a, *b, *c;
   Vec *r;

   spline_type = spn->spline_type;
   if (spline_type != 4)
      return;
   n = spn->ctl_count;

   a = (Flt*)polyray_malloc(n * sizeof(Flt));
   b = (Flt*)polyray_malloc(n * sizeof(Flt));
   c = (Flt*)polyray_malloc(n * sizeof(Flt));
   r = (Vec*)polyray_malloc(n * sizeof(Vec));

   /* Lower boundary condition for tridiagonal matrix */
   a[0] = 0;
   b[0] = 1;
   c[0] = 0;

   /* Create the entries along the diagonal */
   for (i=1;i<n-1;i++) {
      a[i] = spn->ctl_params[i];
      c[i] = spn->ctl_params[i-1];
      b[i] = 2.0 * (a[i] + c[i]);
      }

   /* Upper boundary condition for tridiagonal matrix */
   a[n-1] = 0;
   b[n-1] = 1;
   c[n-1] = 0;

   /* Boundary conditions on result array */
   VecCopy(spn->ctl_derivs[0], r[0]);
   VecCopy(spn->ctl_derivs[n-1], r[n-1]);

   /* Fill in the intermediate values for result array */
   ctl_points = spn->ctl_points;
   for (i=1;i<n-1;i++) {
      VecSub(ctl_points[i+1], ctl_points[i  ], P0);
      VecSub(ctl_points[i  ], ctl_points[i-1], P1);
      x = spn->ctl_params[i];
      y = spn->ctl_params[i+1];
      if (x == 0 || y == 0)
         t = 1.0;
      else
         t = 3.0 / (x * y);
      x = x * x;
      y = y * y;
      VecComb(x, P0, y, P1, P);
      VecScale(t, P);
      VecCopy(P, r[i]);
      }

   tridiag(a, b, c, r, spn->ctl_derivs, n);

   polyray_free(a);
   polyray_free(b);
   polyray_free(c);
   polyray_free(r);

/*
message("Spline entries:\n");
for (i=0;i<n;i++)
   message("   P: <%g,%g,%g>, P': <%g,%g,%g>\n",
           spn->ctl_points[i][0],
           spn->ctl_points[i][1],
           spn->ctl_points[i][2],
           spn->ctl_derivs[i][0],
           spn->ctl_derivs[i][1],
           spn->ctl_derivs[i][2]);
*/
}

/* Evaluate the expression spline(param, array, type). */
/** @brief Build a spline expression node from parameter, control-point, and optional parameter arrays.
 *  @param type        Optional node specifying the spline type.
 *  @param param       Parameter expression node.
 *  @param ctl_points  Array node containing control-point vectors.
 *  @param ctl_params  Optional array node containing per-segment parameter distances.
 *  @return Newly allocated SPLINE expression node.
 */
NODE_PTR make_spline_node(NODE_PTR type, NODE_PTR param,
                 NODE_PTR ctl_points, NODE_PTR ctl_params)
{
   int i, j, len1, len2;
   Flt tempf, t, dist;
   Vec tempv;
   NODE_PTR node, tarray, tempn;
   LIST_PTR list;
   spline_node *spn;

   node = FactoryNODEPTR();
   spn  = (spline_node*)polyray_malloc(sizeof(spline_node));
   if (node == nullptr || spn == nullptr)
      serror("Failed to allocate a spline node\n");
   node->exper_type = SPLINE;
   node->exper_data = spn;
   node->left  = nullptr;
   node->right = nullptr;

   spn->spline_type = 0;
   spn->copy_flag   = 0;
   spn->param       = param;
   spn->ctl_count   = 0;
   spn->ctl_points  = nullptr;
   spn->ctl_params  = nullptr;
   spn->ctl_derivs  = nullptr;

   /* Determine the type of the spline */
   if (type == nullptr)
      spn->spline_type = 0;
   else if ((eval_node(nullptr, type, &t, tempv, &tempn) == 1) &&
            t >= 0 && t <= 4) {
      spn->spline_type = (int)t;
      deallocate_node(type);
      }
   else
      serror("First arg to Spline must a valid spline type [0 - 4]");

   if (ctl_points != nullptr && ctl_points->exper_type == ARRAY) {
      tarray = ctl_points;

      /* Determine the length of the array */
      for (len1=0,list=std::get<LIST_PTR>(tarray->exper_data);
           list != nullptr;
           len1++, list = list->next)
         /* void */;

      if ((spn->spline_type == 4) && len1 < 4)
            serror("Type 4 splines must have at least 2 control points and 2 derivatives");

      spn->ctl_count  = len1;
      if (spn->spline_type == 4)
         spn->ctl_count -= 2;
      spn->ctl_points = (Vec*)polyray_malloc(len1 * sizeof(Vec));
      spn->ctl_derivs = (Vec*)polyray_malloc(len1 * sizeof(Vec));
      spn->ctl_params = (Flt*)polyray_malloc(len1 * sizeof(Flt));

/* message("Spline length: %d\n", spn->ctl_count); */
      /* Walk through the array and set the values of the spline
         control point */
      for (i=0,list=std::get<LIST_PTR>(tarray->exper_data);i<len1;i++,list=list->next) {
         if (eval_node(nullptr, list->element, &tempf, tempv, &tempn) != 2)
            serror("Control points in spline must be vectors");
         if (spn->spline_type == 4) {
            if (i == 0)
               VecCopy(tempv, spn->ctl_derivs[0])
            else if (i == len1 - 1)
               VecCopy(tempv, spn->ctl_derivs[spn->ctl_count - 1])
            else
               VecCopy(tempv, spn->ctl_points[i-1])
            }
         else
            VecCopy(tempv, spn->ctl_points[i])
         }
      deallocate_node(ctl_points);

      /* See if the user entered parameter values for the control points */
      if (ctl_params != nullptr) {
         if (ctl_params->exper_type != ARRAY)
            serror("Spline parameters must be an array");
         tarray = ctl_params;

         /* Determine the length of the array */
         for (len2=0,list=std::get<LIST_PTR>(tarray->exper_data);
              list != nullptr;
              len2++, list = list->next)
            /* void */;

         if (spn->ctl_count != len2)
            serror("Spline must have same # of params(%d) as control points(%d)",
                  len2, len1);

         /* Walk through the array and set the values of the spline
            control point */
         list=std::get<LIST_PTR>(tarray->exper_data);
         for (i=0,dist=0.0;i<len2;i++,list=list->next) {
            j = eval_node(nullptr, list->element, &tempf, tempv, &tempn);
            if (j != 1)
               serror("Spline parameters %d is not a float (%d)", i, j);
            if (i == len2 - 1 && spn->spline_type == 4)
               spn->ctl_params[i] = 0.0;
            else
               spn->ctl_params[i] = tempf;
            dist += spn->ctl_params[i];
            }
         spn->param_dist = dist;
         deallocate_node(ctl_params);
         }
      else {
         /* No parameters for the control points, so we will use chord
            length approximation for all odd spline types and a nice matrix
            solution for some others */
         if (spn->spline_type == 4)
            len2 = len1 - 2;
         else
            len2 = len1;
         for (i=0,dist=0.0;i<len2-1;i++) {
            VecSub(spn->ctl_points[i+1], spn->ctl_points[i], tempv);
            spn->ctl_params[i] = VecLen(tempv);
            dist += spn->ctl_params[i];
            }

         /* If this is a periodic spline then we need one at the end
            for the parameter from the last control point back to the
            first control point */
         if (spn->spline_type & 1) {
            /* Add in the distance from the last control point to the first */
            VecSub(spn->ctl_points[0], spn->ctl_points[i], tempv);
            spn->ctl_params[i] = VecLen(tempv);
            if (spn->ctl_params[i] == 0.0)
               spn->ctl_params[i] = 1.0;
            dist += spn->ctl_params[i];
            }
         else
            spn->ctl_params[i] = 0.0;
         spn->param_dist = dist;
         }

      /* Precalculate the gradients at each control point. */
      if (spn->spline_type == 4)
         calculate_gradients(spn);
      }
   else
      serror("Spline must have an array of control points\n");

   return node;
}

/* Evaluate the expression spline(param, array, type). */
/** @brief Create a shallow copy of spline-node metadata for expression copying.
 *  @param spline_data Source spline node data.
 *  @return Newly allocated spline_node sharing the underlying arrays and parameter node.
 */
spline_node *copy_spline_node(spline_node *spline_data)
{
   spline_node *new_spn;
   spline_node *spn = (spline_node *)spline_data;

   new_spn = (spline_node*)polyray_malloc(sizeof(spline_node));
   if (new_spn == nullptr)
      serror("Failed to allocate a spline node\n");
   new_spn->spline_type = 0;
   new_spn->copy_flag   = 1;
   new_spn->param       = spn->param;
   new_spn->ctl_count   = spn->ctl_count;
   new_spn->ctl_points  = spn->ctl_points;
   new_spn->ctl_params  = spn->ctl_params;
   new_spn->ctl_derivs  = spn->ctl_derivs;

   return new_spn;
}

/** @brief Emit a textual representation of a spline node.
 *  @param node Spline expression node to display.
 *  @return No return value.
 */
void show_spline_node(NODE_PTR node)
{
   smessage("spline()");
}

/** @brief Release the resources owned by a spline expression node.
 *  @param node Spline expression node to deallocate.
 *  @return No return value.
 */
void deallocate_spline_node(NODE_PTR node)
{
   spline_node *spline;

   spline = std::get<spline_node*>(node->exper_data);

   if (!spline->copy_flag) {
      deallocate_node(spline->param);
      polyray_free(spline->ctl_points);
      polyray_free(spline->ctl_params);
      polyray_free(spline->ctl_derivs);
      }
   polyray_free(spline);
}

/* Evaluate the expression spline(param, array, type). */
/** @brief Evaluate a spline expression into a vector result.
 *  @param subst Active substitution/environment for expression evaluation.
 *  @param node  Spline expression node.
 *  @param vval  Output vector value.
 *  @return 2 on successful vector evaluation, 0 otherwise.
 */
int eval_spline(SUBST_PTR subst, NODE_PTR node, Vec vval)
{
   spline_node *spn;
   Flt t;
   Vec tempv;
   NODE_PTR tempn;
   
   spn = std::get<spline_node*>(node->exper_data);
   if (eval_node(subst, spn->param, &t, tempv, &tempn) == 1) {
      /* Evaluate the spline */
      evaluate_spline(spn->spline_type, t, spn->ctl_count, spn->ctl_points,
                      spn->param_dist, spn->ctl_params, spn->ctl_derivs, vval);
      return 2;
      }
   else
      return 0;
}


/* polynom.c

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
#include "io.h"
#include "memory.h"
#include "intersec.h"
#include "symtab.h"
#include "mcube.h"
#include "vector.h"
#include "bound.h"
#include "roots.h"
#include "polynom.h"

typedef struct t_polynomialdata {
   int Order, Sturm_Flag;
   Flt *Coeffs;
   } PolynomialData;
#define POLY PolynomialData

#define COEFF_LIMIT 1.0e-20
#define LEFT_MARGIN 0
#define RIGHT_MARGIN 72

void PolynomialRender(Viewpoint *, BinTree *, Object *obj);
int PolynomialIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int PolynomialInside(Object *, Vec);
void PolynomialDelete(Object *);

ObjectProcs PolynomialProcs = {
   PolynomialRender,
   NULL,
   GenericInitialize,
   PolynomialIntersect,
   PolynomialInside,
   GenericCopy,
   PolynomialDelete,
   };

/* Buffers for manipulating big polynomial equations */
static Flt eqn_v[3][MAX_POLYNOMIAL_ORDER+1];
static Flt eqn_vt[3][MAX_POLYNOMIAL_ORDER+1];

#if 0
static void
show_poly(char *label, int Order, Flt *Coeffs)
{
   int i,j,k,term,cnt,col,f;
   char plyout[40];

   f   = 0;
   cnt = 0;
   message("%s", label);
   col = strlen(label);
   term = 0;
   for (i=Order;i>=0;i--)
      for (j=Order-i;j>=0;j--)
         for (k=Order-(i+j);k>=0;k--) {
            if (Coeffs[term] != 0.0) {
               if (f)
                  sprintf(&plyout[0], " + %.5g", Coeffs[term]);
               else
                  sprintf(&plyout[0], "%.5g", Coeffs[term]);
               cnt = strlen(&plyout[0]);
               if (i == 1)
                  sprintf(&plyout[cnt], "x");
               else if (i > 1)
                  sprintf(&plyout[cnt], "x^%d", i);
               cnt = strlen(&plyout[0]);
               if (j == 1)
                  sprintf(&plyout[cnt], "y");
               else if (j > 1)
                  sprintf(&plyout[cnt], "y^%d", j);
               cnt = strlen(&plyout[0]);
               if (k == 1)
                  sprintf(&plyout[cnt], "z");
               else if (k > 1)
                  sprintf(&plyout[cnt], "z^%d", k);
               cnt = strlen(&plyout[0]);
               f = 1;
               if (col + cnt > RIGHT_MARGIN) {
                  message("\n");
                  col=LEFT_MARGIN;
                  }
               message("%s", &plyout[0]);
               col += cnt;
               }
            term++;
            }
}
#endif

/* For speedup of low order polynomials, expand out the terms
   involved in evaluating the poly. */
static Flt
evaluate_linear(Vec P, Flt *a)
{
   return (a[0] * P[0]) + (a[1] * P[1]) + (a[2] * P[2]) + a[3];
}

/* Intersection of a ray and a quadratic */
static Flt
evaluate_quadratic(Vec P, Flt *a)
{
   Flt x, y, z;
   x  = P[0]; y = P[1]; z  = P[2];
   return  a[0] * x * x + a[1] * x * y + a[2] * x * z +
           a[3] * x     + a[4] * y * y + a[5] * y * z +
           a[6] * y     + a[7] * z * z + a[8] * z     +
           a[9];
}

Object *
MakePolynomial(Object *object, int Order, Flt *Coeffs, int solver)
{
   PolynomialData *sp ;

   object->o_type = T_POLYNOMIAL;
   object->o_procs = &PolynomialProcs;
   sp = (PolynomialData *)polyray_malloc(sizeof(PolynomialData)) ;
   if (sp == NULL)
      error("Failed to allocate polynomial data\n");
   sp->Order = Order;
   sp->Sturm_Flag = solver;
   sp->Coeffs = Coeffs;
   object->o_data = (void *)sp;
   object->o_uv_steps[0] = 20;
   object->o_uv_steps[1] = 20;
   object->o_uv_steps[2] = 20;
   return object;
}

void
Set_Polynomial_Solver(Object *obj, int Sturm_Flag)
{
   PolynomialData *ply = (PolynomialData *)obj->o_data;
   ply->Sturm_Flag = Sturm_Flag;
}

void
PolynomialDelete(Object *object)
{
   PolynomialData *sp = (PolynomialData *)object->o_data;
   if (object->o_copy == 0) {
      polyray_free(sp->Coeffs);
      polyray_free(object->o_data);
      }
}

static Flt
inside(Vec ipoint, int Order, Flt *Coeffs)
{
   Flt x[MAX_POLYNOMIAL_ORDER+1], y[MAX_POLYNOMIAL_ORDER+1];
   Flt z[MAX_POLYNOMIAL_ORDER+1], c, Result;
   int i, j, k, term;
   x[0] = 1.0;       y[0] = 1.0;       z[0] = 1.0;
   x[1] = ipoint[0]; y[1] = ipoint[1]; z[1] = ipoint[2];
   for (i=2;i<=Order;i++) {
      x[i] = x[1] * x[i-1];
      y[i] = y[1] * y[i-1];
      z[i] = z[1] * z[i-1];
      }
   Result = 0.0;
   term = 0;
   for (i=Order;i>=0;i--)
      for (j=Order-i;j>=0;j--)
         for (k=Order-(i+j);k>=0;k--) {
            if ((c = Coeffs[term]) != 0.0)
               Result += c * x[i] * y[j] * z[k];
            term++;
            }
   return Result;
}

/* Intersection of a ray and an arbitrary polynomial function */
static int
rayeqn(Ray *ray, POLY *Shape, Flt *Depths,
       Flt mindist, Flt maxdist)
{
   Flt eqn[MAX_POLYNOMIAL_ORDER+1];
   Flt t[3][MAX_POLYNOMIAL_ORDER+1];
   Vec P, D;
   int h, i, j, k, i1, j1, k1, term;
   int offset, Order;
   Flt val, *Coeffs;
   
   Order = Shape->Order;
   Coeffs = Shape->Coeffs;

   /* First we calculate the values of the individual powers
      of x, y, and z as they are represented by the ray */
   VecCopy(ray->P, P);
   VecCopy(ray->D, D);
   for (i=0;i<3;i++) {
      eqn_v[i][0]  = 1.0;
      eqn_v[i][1]  = P[i];
      eqn_vt[i][0] = 1.0;
      eqn_vt[i][1] = D[i];
      }
   for (i=2;i<=Order;i++)
      for (j=0;j<3;j++) {
         eqn_v[j][i]  = eqn_v[j][1] * eqn_v[j][i-1];
         eqn_vt[j][i] = eqn_vt[j][1] * eqn_vt[j][i-1];
         }

   for (i=0;i<=Order;i++)
      eqn[i] = 0.0;

   /* Now walk through the terms of the polynomial.  As we go
      we substitute the ray equation for each of the variables. */
   term = 0;
   for (i=Order;i>=0;i--) {
      for (h=0;h<=i;h++)
         t[0][h] = binomial(i, h) *
                   eqn_vt[0][i-h] *
                   eqn_v[0][h];
      for (j=Order-i;j>=0;j--) {
         for (h=0;h<=j;h++)
            t[1][h] = binomial(j, h) *
                      eqn_vt[1][j-h] *
                      eqn_v[1][h];
         for (k=Order-(i+j);k>=0;k--) {
            if (Coeffs[term] != 0) {
               for (h=0;h<=k;h++)
                  t[2][h] = binomial(k, h) *
                            eqn_vt[2][k-h] *
                            eqn_v[2][h];

               /* Multiply together the three polynomials */
               offset = Order - (i + j + k);
               for (i1=0;i1<=i;i1++)
                  for (j1=0;j1<=j;j1++)
                     for (k1=0;k1<=k;k1++) {
                        h = offset + i1 + j1 + k1;
                        val = Coeffs[term];
                        val *= t[0][i1];
                        val *= t[1][j1];
                        val *= t[2][k1];
                        eqn[h] += val;
                        }
               }
            term++;
            }
         }
      }
   for (i=0,j=Order;i<=Order;i++)
      if (eqn[i] != 0.0)
         break;
      else
         j--;

   if (j > 4 || Shape->Sturm_Flag == 2)
      return bounded_polysolve(j, &eqn[i], Depths, mindist, maxdist);
   else if (j == 4)
      if (Shape->Sturm_Flag == 1)
          return solve_quartic1(&eqn[i], Depths, mindist, maxdist);
      else
          return solve_quartic(&eqn[i], Depths, mindist, maxdist);
   else if (j==3)
      return solve_cubic(&eqn[i], Depths, mindist, maxdist);
   else if (j==2)
      return solve_quadratic(&eqn[i], Depths, mindist, maxdist);
   else
      return 0;
}

int
PolynomialInside(Object *obj, Vec Pos)
{
   POLY *Shape = (POLY *)obj->o_data;
   Flt Result;
   Vec P;

   InvTxVector1(P, Pos, obj->o_trans)

   if (Shape->Order == 1)
      Result = evaluate_linear(P, Shape->Coeffs);
   else if (Shape->Order == 2)
      Result = evaluate_quadratic(P, Shape->Coeffs);
   else
      Result = inside(P, Shape->Order, Shape->Coeffs);
   return (Result < 0 ? 1 : 0);
}

/* Normal to a polynomial */
static void
pnormal(Vec Result, int Order, Flt *Coeffs, Vec Intersection_Point)
{
   int i, j, k, term;
   Flt *a, val;
   Flt x[MAX_POLYNOMIAL_ORDER+1];
   Flt y[MAX_POLYNOMIAL_ORDER+1];
   Flt z[MAX_POLYNOMIAL_ORDER+1];

   x[0] = 1.0; y[0] = 1.0; z[0] = 1.0;
   x[1] = Intersection_Point[0];
   y[1] = Intersection_Point[1];
   z[1] = Intersection_Point[2];
   for (i=2;i<=Order;i++) {
      x[i] = Intersection_Point[0] * x[i-1];
      y[i] = Intersection_Point[1] * y[i-1];
      z[i] = Intersection_Point[2] * z[i-1];
      }
   a = Coeffs;
   MakeVector(0, 0, 0, Result);
   term = 0;
   for (i=Order;i>=0;i--)
      for (j=Order-i;j>=0;j--)
         for (k=Order-(i+j);k>=0;k--) {
            if (i >= 1) {
               val = x[i-1] * y[j] * z[k];
               Result[0] += i * a[term] * val;
               }
            if (j >= 1) {
               val = x[i] * y[j-1] * z[k];
               Result[1] += j * a[term] * val;
               }
            if (k >= 1) {
               val = x[i] * y[j] * z[k-1];
               Result[2] += k * a[term] * val;
               }
            term++;
            }
}

/* Intersection of a ray and a quadratic */
static int
intersect_linear(Ray *ray, POLY *Shape, Flt *Depths,
                 Flt mindist, Flt maxdist)
{
   Flt t0, t1, *a = Shape->Coeffs;

   t0 = VecDot(a, ray->P);
   t1 = VecDot(a, ray->D);
   if (fabs(t1) < EPSILON)
      return 0;
   Depths[0] = -(a[3] + t0) / t1;
   if (Depths[0] > mindist && Depths[0] < maxdist)
      return 1;
   else
      return 0;
}

/* Intersection of a ray and a quadratic */
static int
intersect_quadratic(Ray *ray, POLY *Shape, Flt *Depths,
                    Flt mindist, Flt maxdist)
{
   Flt x,y,z,x2,y2,z2;
   Flt xx,yy,zz,xx2,yy2,zz2;
   Flt *a, ac, bc, cc, d, t, q;

   x  = ray->P[0]; y  = ray->P[1]; z  = ray->P[2];
   xx = ray->D[0]; yy = ray->D[1]; zz = ray->D[2];
   x2 = x * x;  y2 = y * y;  z2 = z * z;
   xx2 = xx * xx;  yy2 = yy * yy;  zz2 = zz * zz;
   a = Shape->Coeffs;

   /*
      Determine the coefficients of t^n, where the line is represented
      as (x,y,z) + (xx,yy,zz)*t.
   */
   ac = (a[0]*xx2 + a[1]*xx*yy + a[2]*xx*zz + a[4]*yy2 + a[5]*yy*zz +
        a[7]*zz2);
   bc = (2*a[0]*x*xx + a[1]*(x*yy + xx*y) + a[2]*(x*zz + xx*z) +
        a[3]*xx + 2*a[4]*y*yy + a[5]*(y*zz + yy*z) + a[6]*yy +
        2*a[7]*z*zz + a[8]*zz);
   cc = a[0]*x2 + a[1]*x*y + a[2]*x*z + a[3]*x + a[4]*y2 +
        a[5]*y*z + a[6]*y + a[7]*z2 + a[8]*z + a[9];

   if (fabs(ac) < COEFF_LIMIT) {
      if (fabs(bc) < COEFF_LIMIT)
         return 0;
      t = -cc / bc;
      if (t > mindist && t < maxdist) {
         Depths[0] = t;
         return 1;
         }
      else
         return 0;
      }

   /* Solve the quadratic formula & return results that are
      within the correct interval. */
   d = bc * bc - 4.0 * ac * cc;
   if (d < 0.0) return 0;
   d = sqrt(d);
   bc = -bc;
   t = 2.0 * ac;
   q = (bc + d) / t;
   if (q > mindist && q < maxdist) {
      Depths[0] = q;
      q = (bc - d) / t;
      if (q > mindist && q < maxdist) {
         Depths[1] = q;
         return 2;
         }
      return 1;
      }
   q = (bc - d) / t;
   if (q > mindist && q < maxdist) {
      Depths[0] = q;
      return 1;
      }
   return 0;
}

/* Normal to a quartic */
static void
quartic_normal(Vec Result, POLY *Shape, Vec Intersection_Point)
{
   Flt *a,x,y,z,x2,y2,z2,x3,y3,z3;
   int Order = Shape->Order;

   a = Shape->Coeffs;
   x = Intersection_Point[0];
   y = Intersection_Point[1];
   z = Intersection_Point[2];
/* message("Normal at <%lg, %lg, %lg>\n", x, y, z); */
   if (Order == 1) {
      /* Linear partial derivatives */
      Result[0] = a[0];
      Result[1] = a[1];
      Result[2] = a[2];
      }
   else if (Order == 2) {
      /* Quadratic partial derivatives */
      Result[0] = 2*a[0]*x+a[1]*y+a[2]*z+a[3];
      Result[1] = a[1]*x+2*a[4]*y+a[5]*z+a[6];
      Result[2] = a[2]*x+a[5]*y+2*a[7]*z+a[8];
      }
   else if (Order == 3) {
      x2 = x * x;  y2 = y * y;  z2 = z * z;
      /* Cubic partial derivatives */
      Result[0] = 3*a[0]*x2 + 2*x*(a[1]*y + a[2]*z + a[3]) + a[4]*y2 +
                  y*(a[5]*z + a[6]) + a[7]*z2 + a[8]*z + a[9];
      Result[1] = a[1]*x2 + x*(2*a[4]*y + a[5]*z + a[6]) + 3*a[10]*y2 +
                  2*y*(a[11]*z + a[12]) + a[13]*z2 + a[14]*z + a[15];
      Result[2] = a[2]*x2 + x*(a[5]*y + 2*a[7]*z + a[8]) + a[11]*y2 +
                  y*(2*a[13]*z + a[14]) + 3*a[16]*z2 + 2*a[17]*z + a[18];
      }
   else {
      /* Quartic partial derivatives */
      x2 = x * x;  y2 = y * y;  z2 = z * z;
      x3 = x * x2; y3 = y * y2; z3 = z * z2;
      Result[0] = 4*a[ 0]*x3+3*x2*(a[ 1]*y+a[ 2]*z+a[ 3])+
                  2*x*(a[ 4]*y2+y*(a[ 5]*z+a[ 6])+a[ 7]*z2+a[ 8]*z+a[ 9])+
                  a[10]*y3+y2*(a[11]*z+a[12])+y*(a[13]*z2+a[14]*z+a[15])+
                  a[16]*z3+a[17]*z2+a[18]*z+a[19];
      Result[1] = a[ 1]*x3+x2*(2*a[ 4]*y+a[ 5]*z+a[ 6])+
                  x*(3*a[10]*y2+2*y*(a[11]*z+a[12])+a[13]*z2+a[14]*z+a[15])+
                  4*a[20]*y3+3*y2*(a[21]*z+a[22])+2*y*(a[23]*z2+a[24]*z+a[25])+
                  a[26]*z3+a[27]*z2+a[28]*z+a[29];
      Result[2] = a[ 2]*x3+x2*(a[ 5]*y+2*a[ 7]*z+a[ 8])+
                  x*(a[11]*y2+y*(2*a[13]*z+a[14])+3*a[16]*z2+2*a[17]*z+a[18])+
                  a[21]*y3+y2*(2*a[23]*z+a[24])+y*(3*a[26]*z2+2*a[27]*z+a[28])+
                  4*a[30]*z3+3*a[31]*z2+2*a[32]*z+a[33];
      }
}

int
PolynomialIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                    Flt mindist, Flt maxdist, Isect *hit)
{
   Flt Depths[MAX_POLYNOMIAL_ORDER];
   Vec P, N ;
   PolynomialData *shape = (PolynomialData *)obj->o_data;
   int cnt, i;
   int Flag = 0;

   if (shape->Order == 1)
      cnt = intersect_linear(ray, shape, Depths, mindist, maxdist);
   else if (shape->Order == 2)
      cnt = intersect_quadratic(ray, shape, Depths, mindist, maxdist);
   else
      cnt = rayeqn(ray, shape, Depths, mindist, maxdist);

   for (i=0;i<cnt;i++) {
      /* Insert the hit  */
      VecAddScaled(ray->P, Depths[i], ray->D, P);
      if (shape->Order <= 4)
         quartic_normal(N, shape, P);
      else
         pnormal(N, shape->Order, shape->Coeffs, P);
      Insert_Hit(obj, P, N, Depths[i], P, hit);
      Flag = 1;
      }
   return Flag;
}

static Flt
polynomial_value(Object *obj, Vec Pos)
{
   Vec P;
   POLY *Shape = (POLY *)obj->o_data;

   /* Bring the point into this objects space */
   if (obj->o_trans != NULL)
      InvTxVector1(P, Pos, obj->o_trans)
   else
      VecCopy(Pos, P)

   if (Shape->Order == 1)
      return evaluate_linear(P, Shape->Coeffs);
   else if (Shape->Order == 2)
      return evaluate_quadratic(P, Shape->Coeffs);
   else
      return inside(P, Shape->Order, Shape->Coeffs);
}

static int
polynomial_gradient(Object *obj, Vec P, Vec N)
{
   PolynomialData *shape = (PolynomialData *)obj->o_data;

   if (shape->Order <= 4)
      quartic_normal(N, shape, P);
   else
      pnormal(N, shape->Order, shape->Coeffs, P);
   return 1;
}

void
PolynomialRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   MarchCubes(eye, Root, obj->o_uv_steps[0], obj->o_uv_steps[1],
              obj->o_uv_steps[2], &(obj->o_bnd),
              0.0, polynomial_value,
              polynomial_gradient, obj);

}

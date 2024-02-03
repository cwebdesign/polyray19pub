/* roots.c

  Code to solve for the roots of polynomials.

  Input polynomials should always have the coefficient of the highest
  power first.

  Copyright (C) 1993, Alexander Enzmann, All rights reserved.

  You may not distribute this software, in whole or in part,
  without the express consent of the authors.

  There is no warranty or other guarantee of fitness of this software
  for any purpose.  It is provided solely "as is".

*/
#include "defs.h"
#include "vector.h"
#include "roots.h"
#include "io.h"

#undef EPSILON

#define EPSILON 1.0e-10
#define MAX_ITERATIONS 50
#define COEFF_LIMIT 1.0e-20
#define POLISH_EPSILON 1.0e-5

#define FUDGE_FACTOR1 1.0e11
#define FUDGE_FACTOR2 -1.0e-5
#define FUDGE_FACTOR3 1.0e-7

#define MAX_STURM_ORDER 17
typedef struct p {
   int ord;
   LFlt coef[MAX_STURM_ORDER+1];
   } polynomial;

#define BINOMSIZE 40

/* The following table contains the binomial coefficients up to 15 */
int binomials[15][15] =
  {{  1,  0,  0,  0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0},
   {  1,  1,  0,  0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0},
   {  1,  2,  1,  0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0},
   {  1,  3,  3,  1,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0},
   {  1,  4,  6,  4,   1,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0},
   {  1,  5, 10, 10,   5,   1,   0,   0,   0,   0,   0,  0,  0,  0,  0},
   {  1,  6, 15, 20,  15,   6,   1,   0,   0,   0,   0,  0,  0,  0,  0},
   {  1,  7, 21, 35,  35,  21,   7,   1,   0,   0,   0,  0,  0,  0,  0},
   {  1,  8, 28, 56,  70,  56,  28,   8,   1,   0,   0,  0,  0,  0,  0},
   {  1,  9, 36, 84, 126, 126,  84,  36,   9,   1,   0,  0,  0,  0,  0},
   {  1, 10, 45,120, 210, 252, 210, 120,  45,  10,   1,  0,  0,  0,  0},
   {  1, 11, 55,165, 330, 462, 462, 330, 165,  55,  11,  1,  0,  0,  0},
   {  1, 12, 66,220, 495, 792, 924, 792, 495, 220,  66, 12,  1,  0,  0},
   {  1, 13, 78,286, 715,1287,1716,1716,1287, 715, 286, 78, 13,  1,  0},
   {  1, 14, 91,364,1001,2002,3003,3432,3003,2002,1001,364, 91, 14,  1}};

/* Remove all factors of i from n. */
static int
factor_out(int n, int i, int *c, int *s)
{
   while (!(n % i)) {
      n /= i;
      s[(*c)++] = i;
      }
   return n;
}

/* Find all prime factors of n. (Note that n must be less than 2^15 */
static void
factor1(int n, int *c, int *s)
{
   int i,k;
   /* First factor out any 2s */
   n = factor_out(n, 2, c, s);
   /* Now any odd factors */
   k = sqrt(n) + 1;
   for (i=3;n>1 && i<=k;i+=2)
      if (!(n%i)) {
         n = factor_out(n, i, c, s);
         k = sqrt(n)+1;
         }
   if (n>1)
      s[(*c)++] = n;
}

/* Calculate the binomial coefficent of n,r. */
long
binomial(int n, int r)
{
   int h,i,j,k,l;
   unsigned long result;
   static int stack1[BINOMSIZE], stack2[BINOMSIZE];
   if (n<0 || r<0 || r>n)
      result = 0L;
   else if (r==n)
      result = 1L;
   else if (r < 15 && n < 15)
      result = (long)binomials[n][r];
   else {
      j = 0;
      for (i=r+1;i<=n;i++)
         stack1[j++] = i;
      for (i=2;i<=(n-r);i++) {
         h = 0;
         factor1(i, &h, stack2);
         for (k=0;k<h;k++) {
            for (l=0;l<j;l++)
               if (!(stack1[l] % stack2[k])) {
                  stack1[l] /= stack2[k];
                  goto l1;
                  }
            warning("Failed to factor %d from: ", stack1[k]);
            l1:;
            }
         }
      result=1;
      for (i=0;i<j;i++)
         result *= stack1[i];
      }
   return result;
}

/* Calculate the modulus of u(x) / v(x) leaving it in r, it
    returns 0 if r(x) is a constant.
    note: this function assumes the leading coefficient of v 
     is 1 or -1 */
static int
modp(polynomial *u, polynomial *v, polynomial *r)
{
   int i, k, j;

   for (i=0;i<u->ord;i++)
      r[i] = u[i];

   if (v->coef[v->ord] < 0.0) {
      for (k = u->ord - v->ord - 1; k >= 0; k -= 2)
         r->coef[k] = -r->coef[k];
      for (k = u->ord - v->ord; k >= 0; k--)
         for (j = v->ord + k - 1; j >= k; j--)
            r->coef[j] = -r->coef[j] - r->coef[v->ord + k] * v->coef[j - k];
      }
   else {
      for (k = u->ord - v->ord; k >= 0; k--)
         for (j = v->ord + k - 1; j >= k; j--)
            r->coef[j] -= r->coef[v->ord + k] * v->coef[j - k];
      }

   k = v->ord - 1;
   while (k >= 0 && fabs(r->coef[k]) < COEFF_LIMIT) {
      r->coef[k] = 0.0;
      k--;
      }
   r->ord = (k < 0) ? 0 : k;
   return(r->ord);
}

/* Build the sturmian sequence for a polynomial */
static int
buildsturm(int ord, polynomial *sseq)
{
   int i;
   LFlt f, *fp, *fc;
   polynomial *sp;

   sseq[0].ord = ord;
   sseq[1].ord = ord - 1;

   /* calculate the derivative and normalize the leading coefficient. */
   f = fabs(sseq[0].coef[ord] * ord);
   fp = sseq[1].coef;
   fc = sseq[0].coef + 1;
   for (i = 1; i <= ord; i++)
      *fp++ = *fc++ * i / f;

   /* construct the rest of the Sturm sequence */
   for (sp = sseq + 2;modp(sp - 2, sp - 1, sp); sp++) {
      /* reverse the sign and normalize */
      f = -fabs(sp->coef[sp->ord]);
      for (fp = &sp->coef[sp->ord]; fp >= sp->coef; fp--)
         *fp /= f;
      }
   sp->coef[0] = -sp->coef[0];   /* reverse the sign */

   return(sp - sseq);
}

/* Evaluate a polynomial at a specific point */
static LFlt
polyeval(LFlt x, int n, LFlt *Coeffs)
{
   int i;
   LFlt *tcoef;
   LFlt val;

   tcoef = &Coeffs[n];
   val = *tcoef--;
   for (i=n-1;i>=0;i--,tcoef--)
      val = val * x + *tcoef;
   return val;
}

/* Return the number of sign changes in the Sturm sequence in
   sseq at the value a. */
static int
numchanges(int np, polynomial *sseq, LFlt a)
{
   int changes;
   LFlt f, lf;
   polynomial *s;
   changes = 0;
   lf = polyeval(a, sseq[0].ord, sseq[0].coef);
   for (s = sseq + 1; s <= sseq + np; s++) {
      f = polyeval(a, s->ord, s->coef);
      if (lf == 0.0 || lf * f < 0)
         changes++;
      lf = f;
      }
   return(changes);
}


/* Close in on a root by using regula-falsa */
static int
regula_falsa(int order, LFlt *coef, LFlt a, LFlt b, Flt *val)
{
   int its;
   LFlt fa, fb, x, fx, lfx;

   fa = polyeval(a, order, coef);
   fb = polyeval(b, order, coef);

   if (fa * fb > 0.0)
      return 0;

   if (fabs(fa) < COEFF_LIMIT) {
      *val = a;
      return 1;
      }

   if (fabs(fb) < COEFF_LIMIT) {
      *val = b;
      return 1;
      }

   lfx = fa;

   for (its = 0; its < MAX_ITERATIONS; its++) {
      x = (fb * a - fa * b) / (fb - fa);
      fx = polyeval(x, order, coef);

      if (fabs(x) > EPSILON) {
         if (fabs(fx / x) < EPSILON) {
            *val = x;
            return 1;
            }
         }
      else if (fabs(fx) < EPSILON) {
         *val = x;
         return 1;
         }

      if (fa < 0)
         if (fx < 0) {
            a = x;
            fa = fx;
            if ((lfx * fx) > 0)
               fb /= 2;
            }
         else {
            b = x;
            fb = fx;
            if ((lfx * fx) > 0)
               fa /= 2;
            }
      else if (fx < 0) {
         b = x;
         fb = fx;
         if ((lfx * fx) > 0)
            fa /= 2;
         }
      else {
         a = x;
         fa = fx;
         if ((lfx * fx) > 0)
            fb /= 2;
         }
      if (fabs(b-a) < EPSILON) {
         /* Check for underflow in the domain */
         *val = x;
         return 1;
         }
      lfx = fx;
      }
   return 0;
}

/* Use a bisection based on the sturm sequence for the polynomial
   described in sseq to isolate intervals in which roots occur,
   the roots are returned in the roots array in order of magnitude.

Note: This routine has one severe bug: When the interval containing the
      root [min, max] has a root at one of its endpoints, as well as one
      within the interval, the root at the endpoint will be returned rather
      than the one inside. */
static int
sbisect(int np, polynomial *sseq, LFlt min, LFlt max,
        int atmin, int atmax, Flt *roots)
{
   LFlt  mid;
   int  n1, n2, its, atmid;

   if ((atmin - atmax) == 1) {
      /* first try using regula-falsa to find the root.  */
      if (regula_falsa(sseq->ord, sseq->coef, min, max, roots))
         return 1;
      else {
         /* That failed, so now find it by bisection */
         for (its = 0; its < MAX_ITERATIONS; its++) {
            mid = (min + max) / 2;
            atmid = numchanges(np, sseq, mid);
            if (fabs(mid) > EPSILON) {
               if (fabs((max - min) / mid) < EPSILON) {
                  roots[0] = mid;
                  return 1;
                  }
               }
            else if (fabs(max - min) < EPSILON) {
               roots[0] = mid;
               return 1;
               }
            if ((atmin - atmid) == 0)
               min = mid;
            else
               max = mid;
            }
         /* Bisection took too long - just return what we got */
         roots[0] = mid;
         return 1;
         }
      }

   /* There is more than one root in the interval.
      Bisect to find new intervals */
   for (its = 0; its < MAX_ITERATIONS; its++) {
      mid = (min + max) / 2;
      atmid = numchanges(np, sseq, mid);
      n1 = atmin - atmid;
      n2 = atmid - atmax;
      if (n1 != 0 && n2 != 0) {
         n1 = sbisect(np, sseq, min, mid, atmin, atmid, roots);
         n2 = sbisect(np, sseq, mid, max, atmid, atmax, &roots[n1]);
         return n1 + n2;
         }
      if (n1 == 0)
         min = mid;
      else
         max = mid;
      }

   /* Took too long to bisect.  Just return what we got. */
   roots[0] = mid;
   return 1;
}

/*
   Solve the linear equation:
      x[0] * x + x[1] = 0.
*/
int
solve_linear(Flt *x, Flt *y, Flt mindist, Flt maxdist)
{
   Flt a, b, q;
   a = x[0];
   b = -x[1];
   if (fabs(a) < COEFF_LIMIT)
      return 0;
   else {
      q = b / a;
      if (q >= mindist && q <= maxdist) {
         y[0] = q;
         return 1;
         }
      else
         return 0;
      }
}

/*
   Solve the quadratic equation:
      x[0] * x^2 + x[1] * x + x[2] = 0.

   The value returned by this function is the number of real roots.
   The roots themselves are returned in y[0], y[1].
*/
int
solve_quadratic(Flt *x, Flt *y, Flt mindist, Flt maxdist)
{
   Flt d, t, a, b, c, q;

   a = x[0];
   b = -x[1];
   c = x[2];
   if (fabs(a) < COEFF_LIMIT) {
      if (fabs(b) < COEFF_LIMIT) {
         return 0;
         }
      q = c / b;
      if (q >= mindist && q <= maxdist) {
         y[0] = q;
         return 1;
         }
      else {
         return 0;
         }
      }
   d = b * b - 4.0 * a * c;
   if (d < -EPSILON) {
      return 0;
      }
   else if (fabs(d) <= EPSILON) {
      q = 0.5 * b / a;
      if (q >= mindist && q <= maxdist) {
         y[0] = q;
         return 1;
         }
      return 0;
      }
   d = sqrt(d);
   t = 2.0 * a;
   q = (b + d) / t;
   if (q >= mindist && q <= maxdist) {
      y[0] = q;
      q = (b - d) / t;
      if (q >= mindist && q <= maxdist) {
         y[1] = q;
         return 2;
         }
      return 1;
      }
   q = (b - d) / t;
   if (q >= mindist && q <= maxdist) {
      y[0] = q;
      return 1;
      }
   return 0;
}

/*
   Solve the cubic equation:

      x[0] * x^3 + x[1] * x^2 + x[2] * x + x[3] = 0.

   The result of this function is an integer that tells how many real
   roots exist.  Determination of how many are distinct is up to the
   process that calls this routine.  The roots that exist are stored
   in (y[0], y[1], y[2]).

   Note: this function relies very heavily on trigonometric functions and
   the square root function.  If an alternative solution is found that does
   not rely on transcendentals this code will be replaced.
*/
int
solve_cubic(Flt *x, Flt *y, Flt mindist, Flt maxdist)
{
   Flt Q, R, Q3, R2, sQ, d, an, theta;
   Flt A2, a0, a1, a2, a3;
   int i;

   a0 = x[0];
   if (fabs(a0) < COEFF_LIMIT) {
      return solve_quadratic(&x[1], y, mindist, maxdist);
      }
   else if (a0 != 1.0) {
      a1 = x[1] / a0;
      a2 = x[2] / a0;
      a3 = x[3] / a0;
      }
   else {
      a1 = x[1];
      a2 = x[2];
      a3 = x[3];
      }
   A2 = a1 * a1;
   Q = (A2 - 3.0 * a2) / 9.0;
   R = (2.0 * A2 * a1 - 9.0 * a1 * a2 + 27.0 * a3) / 54.0;
   Q3 = Q * Q * Q;
   R2 = R * R;
   d = Q3 - R2;
   an = a1 / 3.0;
   i = 0;
   if (d >= 0.0) {
      /* Three real roots. */
      d = R / sqrt(Q3);
      theta = acos(d) / 3.0;
      sQ = -2.0 * sqrt(Q);
      d = sQ * cos(theta) - an;
      if (d > mindist && d < maxdist)
         y[i++] = d;
      d = sQ * cos(theta + TWO_PI_3) - an;
      if (d > mindist && d < maxdist)
         y[i++] = d;
      d = sQ * cos(theta + TWO_PI_43) - an;
      if (d > mindist && d < maxdist)
         y[i++] = d;
      return i;
      }
   else {
      sQ = pow(sqrt(R2 - Q3) + fabs(R), 1.0 / 3.0);
      if (R < 0)
         d = (sQ + Q / sQ) - an;
      else
         d = -(sQ + Q / sQ) - an;
      if (d > mindist && d < maxdist)
         y[i++] = d;
      return i;
      }
}

/* Test to see if any coeffs are more than 6 orders of magnitude
   larger than the smallest */
static int
difficult_coeffs(int n, Flt *x)
{
   int i;
   Flt *p, t, biggest, smallest;

   biggest = smallest = fabs(x[0]);
   for (i=1,p=x+1;i<=n;i++,p++) {
      t = fabs(*p);
      if (t > biggest)
         biggest = t;
      if (t < smallest)
         smallest = t;
      }
   
   /* Everything is zero no sense in doing any more */
   if (biggest == 0.0)
      return 0;

/*   if (biggest / smallest > FUDGE_FACTOR1) {*/
   if (biggest > FUDGE_FACTOR1*smallest) {
      /* Try simply setting all very small coefficients to zero */
      for (i=0,p=x;i<=n;i++,p++) {
         t = fabs(*p);
         if (fabs(t) < POLISH_EPSILON && t/biggest < EPSILON)
            *p = 0.0;
         }
      return 1;
      }

   return 0;
}

/* This is an adaptation of the method of Lodovico Ferrari (Circa 1731). */
int
solve_quartic(Flt *x, Flt *results, Flt mindist, Flt maxdist)
{
   Flt cubic[4], roots[3];
   Flt a0, a1, y, d1, x1, t1, t2;
   Flt c0, c1, c2, c3, c4, d2, q1, q2;
   int i;

   /* See if the constant term has vanished */
   y = fabs(x[4]);
   if (y < COEFF_LIMIT) {
      if (0 > mindist && 0 < maxdist) {
         results[0] = 0.0;
         return 1 + solve_cubic(x, &results[1], mindist, maxdist);
         }
      else
         return solve_cubic(x, results, mindist, maxdist);
      }
#if 0
   else if (fabs(x[1]) < EPSILON && fabs(x[3]) < EPSILON) {
      /* This is a quadratic in x^2.  Solve it as a special case. */
      cubic[0] = x[0];
      cubic[1] = x[2];
      cubic[2] = x[4];
      i = 0;
      j = solve_quadratic(cubic, roots, -PLY_HUGE, PLY_HUGE);
      if (j > 0) {
         if (roots[0] <= 0.0) {
            y = sqrt(-roots[0]);
            if (y >= mindist && y <= maxdist)
               results[i++] = y;
            if (-y >= mindist && -y <= maxdist)
               results[i++] = -y;
            }
         if (j > 1) {
            if (roots[1] <= 0.0) {
               y = sqrt(-roots[1]);
               if (y >= mindist && y <= maxdist)
                  results[i++] = y;
               if (-y >= mindist && -y <= maxdist)
                  results[i++] = -y;
               }
            }
         }
      return i;
      }
#endif

   c0 = x[0];
   if (fabs(c0) < COEFF_LIMIT)
      return solve_cubic(&x[1], results, mindist, maxdist);

   if (difficult_coeffs(4, x))
      return bounded_polysolve(4, x, results, mindist, maxdist);

   /* Make sure the quartic has a leading coefficient of 1.0 */
   if (c0 != 1.0) {
      c1 = x[1] / c0;
      c2 = x[2] / c0;
      c3 = x[3] / c0;
      c4 = x[4] / c0;
      }
   else {
      c1 = x[1];
      c2 = x[2];
      c3 = x[3];
      c4 = x[4];
      }

   /* The first step is to take the original equation:

         x^4 + b*x^3 + c*x^2 + d*x + e = 0

      and rewrite it as:

         x^4 + b*x^3 = -c*x^2 - d*x - e,

      adding (b*x/2)^2 + (x^2 + b*x/2)y + y^2/4 to each side gives a
      perfect square on the lhs:

         (x^2 + b*x/2 + y/2)^2 = (b^2/4 - c + y)x^2 + (b*y/2 - d)x + y^2/4 - e

      By choosing the appropriate value for y, the rhs can be made a perfect
      square also.  This value is found when the rhs is treated as a quadratic
      in x with the discriminant equal to 0.  This will be true when:

         (b*y/2 - d)^2 - 4.0 * (b^2/4 - c*y)*(y^2/4 - e) = 0, or

         y^3 - c*y^2 + (b*d - 4*e)*y - b^2*e + 4*c*e - d^2 = 0.

      This is called the resolvent of the quartic equation.  */

   a0 = 4.0 * c4;
   cubic[0] = 1.0;
   cubic[1] = -1.0 * c2;
   cubic[2] = c1 * c3 - a0;
   cubic[3] = a0 * c2 - c1 * c1 * c4 - c3 * c3;
   i = solve_cubic(&cubic[0], &roots[0], -PLY_HUGE, PLY_HUGE);
   if (i > 0)
      y = roots[0];
   else
      return 0;

   /* What we are left with is a quadratic squared on the lhs and a
      linear term on the right.  The linear term has one of two signs,
      take each and add it to the lhs.  The form of the quartic is now:

         a' = b^2/4 - c + y,    b' = b*y/2 - d, (from rhs quadritic above)

         (x^2 + b*x/2 + y/2) = +sqrt(a'*(x + 1/2 * b'/a')^2), and
         (x^2 + b*x/2 + y/2) = -sqrt(a'*(x + 1/2 * b'/a')^2).

      By taking the linear term from each of the right hand sides and
      adding to the appropriate part of the left hand side, two quadratic
      formulas are created.  By solving each of these the four roots of
      the quartic are determined.
   */
   i = 0;
   a0 = c1 / 2.0;
   a1 = y / 2.0;

   t1 = a0 * a0 - c2 + y;

   if (t1 < 0.0) {
      if (t1 > FUDGE_FACTOR2)
         t1 = 0.0;
      else
         /* First Special case, a' < 0 means all roots are complex. */
         return 0;
      }
   if (t1 < FUDGE_FACTOR3) {
      /* Second special case, the "x" term on the right hand side above
         has vanished.  In this case:
                (x^2 + b*x/2 + y/2) = +sqrt(y^2/4 - e), and
                (x^2 + b*x/2 + y/2) = -sqrt(y^2/4 - e).  */
      t2 = a1 * a1 - c4;
      if (t2 < 0.0)
         return 0;
      x1 = 0.0;
      d1 = sqrt(t2);
      }
   else {
      x1 = sqrt(t1);
      d1 = 0.5 * (a0 * y - c3) / x1;
      }

   /* Solve the first quadratic */
   i = 0;
   q1 = -a0 - x1;
   q2 = a1 + d1;
   d2 = q1 * q1 - 4.0 * q2;
   if (d2 >= 0.0) {
      d2 = sqrt(d2);
      y = 0.5 * (q1 + d2);
      if (y > mindist && y < maxdist)
         results[i++] = y;
      y = 0.5 * (q1 - d2);
      if (y > mindist && y < maxdist)
         results[i++] = y;
      }
   /* Solve the second quadratic */
   q1 = x1 - a0;
   q2 = a1 - d1;
   d2 = q1 * q1 - 4.0 * q2;
   if (d2 >= 0.0) {
      d2 = sqrt(d2);
      y = 0.5 * (q1 + d2);
      if (y > mindist && y < maxdist)
         results[i++] = y;
      y = 0.5 * (q1 - d2);
      if (y > mindist && y < maxdist)
         results[i++] = y;
      }
   return i;
}

/* Solve a quartic using the method of Francois Vieta (Circa 1735) */
int
solve_quartic1(Flt *x, Flt *results, Flt mindist, Flt maxdist)
{
   Flt cubic[4], roots[3];
   Flt c12, y, z, p, q, q1, q2, r, d1, d2;
   Flt c0, c1, c2, c3, c4;
   int i;

   /* See if the high order term has vanished */
   c0 = x[0];
   if (fabs(c0) < COEFF_LIMIT)
      return solve_cubic(&x[1], results, mindist, maxdist);

   /* See if the constant term has vanished */
   y = fabs(x[4]);
   if (y < COEFF_LIMIT) {
      if (0 > mindist && 0 < maxdist) {
         results[0] = 0.0;
         return 1 + solve_cubic(x, &results[1], mindist, maxdist);
         }
      else
         return solve_cubic(x, results, mindist, maxdist);
      }
#if 0
   else if (fabs(x[1]) < EPSILON && fabs(x[3]) < EPSILON) {
      /* This is a quadratic in x^2.  Solve it as a special case. */
      cubic[0] = x[0];
      cubic[1] = x[2];
      cubic[2] = x[4];
      i = 0;
      j = solve_quadratic(cubic, roots, -PLY_HUGE, PLY_HUGE);
      if (j > 0) {
         if (roots[0] >= 0.0) {
            y = sqrt(roots[0]);
            if (y >= mindist && y <= maxdist)
               results[i++] = y;
            if (-y >= mindist && -y <= maxdist)
               results[i++] = -y;
            }
         if (j > 1) {
            if (roots[1] >= 0.0) {
               y = sqrt(roots[1]);
               if (y >= mindist && y <= maxdist)
                  results[i++] = y;
               if (-y >= mindist && -y <= maxdist)
                  results[i++] = -y;
               }
            }
         }
      return i;
      }
#endif

   if (difficult_coeffs(4, x))
      return bounded_polysolve(4, x, results, mindist, maxdist);

   /* Make sure the quartic has a leading coefficient of 1.0 */
   if (c0 != 1.0) {
      c1 = x[1] / c0;
      c2 = x[2] / c0;
      c3 = x[3] / c0;
      c4 = x[4] / c0;
      }
   else {
      c1 = x[1];
      c2 = x[2];
      c3 = x[3];
      c4 = x[4];
      }

   /* Compute the cubic resolvant */
   c12 = c1 * c1;
   p = -0.375 * c12 + c2;
   q = 0.125 * c12 * c1 - 0.5 * c1 * c2 + c3;
   r = -0.01171875 * c12 * c12 + 0.0625 * c12 * c2 - 0.25 * c1 * c3 + c4;

   cubic[0] = 1.0;
   cubic[1] = -0.5 * p;
   cubic[2] = -r;
   cubic[3] = 0.5 * r * p - 0.125 * q * q;
   i = solve_cubic(cubic, roots, -PLY_HUGE, PLY_HUGE);
   if (i > 0)
      z = roots[0];
   else
      return 0;

   d1 = 2.0 * z - p;

   if (d1 < 0.0) {
      if (d1 > -EPSILON)
         d1 = 0.0;
      else
         return 0;
      }
   if (d1 < EPSILON) {
      d2 = z * z - r;
      if (d2 < 0.0)
         return 0;
      d2 = sqrt(d2);
      }
   else {
      d1 = sqrt(d1);
      d2 = 0.5 * q / d1;
      }

   /* Set up useful values for the quadratic factors */
   q1 = d1 * d1;
   q2 = -0.25 * c1;
   i = 0;

   /* Solve the first quadratic */
   p = q1 - 4.0 * (z - d2);
   if (p == 0) {
      y = -0.5 * d1 - q2;
      if (y > mindist && y < maxdist)
         results[i++] = y;
      }
   else if (p > 0) {
      p = sqrt(p);
      y = -0.5 * (d1 + p) + q2;
      if (y > mindist && y < maxdist)
         results[i++] = y;
      y = -0.5 * (d1 - p) + q2;
      if (y > mindist && y < maxdist)
         results[i++] = y;
      }
   /* Solve the second quadratic */
   p = q1 - 4.0 * (z + d2);
   if (p == 0) {
      y = 0.5 * d1 - q2;
      if (y > mindist && y < maxdist)
         results[i++] = y;
      }
   else if (p > 0) {
      p = sqrt(p);
      y = 0.5 * (d1 + p) + q2;
      if (y > mindist && y < maxdist)
         results[i++] = y;
      y = 0.5 * (d1 - p) + q2;
      if (y > mindist && y < maxdist)
         results[i++] = y;
      }
   return i;
}

/* Root solver based on the Sturm sequences for a polynomial. */
int
bounded_polysolve(int order, Flt *Coeffs, Flt *roots,
                  Flt mindist, Flt maxdist)
{
   polynomial sseq[MAX_STURM_ORDER+1];
   int i, j, n, nroots, np, atmin, atmax;
extern int current_row, current_col;

   if (order >= MAX_STURM_ORDER)
      error("Polynomials of order %d are too complex, max order is: %d\n",
            order, MAX_STURM_ORDER-1);

   /* Perform deflation based on significantly different orders of magnitude
      in the coefficients. */
   (void)difficult_coeffs(order, Coeffs);

   /* Look to see if the polynomial is ok by examining the high
      order terms in the poly.  If we find any zeros, then we
      solve for the polynomial that has a non-zero term. */
   for (j=order;j>0;j--)
      if (Coeffs[order-j] != 0)
         break;

   n = 0;
   if (Coeffs[order] == 0.0) {
      /* Zero root, deflate the polynomial prior to solving */
      if (0 >= mindist && 0 <= maxdist) {
         roots[0] = 0.0;
         n = 1;
         }
      order -= 1;
      j -= 1;
      }

   if (j == 3)
      return solve_cubic(&Coeffs[order-j], &roots[n], mindist, maxdist);
   else if (j == 2)
      return solve_quadratic(&Coeffs[order-j], &roots[n], mindist, maxdist);
   else if (j == 1)
      return solve_linear(&Coeffs[order-j], &roots[n], mindist, maxdist);

   /* Put the coefficients into the top of the stack. */
   for (i=0;i<=order;i++) {
      sseq[0].coef[order-i] = Coeffs[i];
      }

   /* Build the Sturm sequence */
   np = buildsturm(j, &sseq[0]);

   /* Get the total number of visible roots within the interval */
   atmin = numchanges(np, sseq, mindist);
   atmax = numchanges(np, sseq, maxdist);
   nroots = atmin - atmax;

   if (nroots <= 0) return 0;

   /* perform the bisection. */
   return sbisect(np, sseq, mindist, maxdist, atmin, atmax, &roots[n]);
}

/* Test to see if "point" is inside the polygon "points". */
int
Inside_Polygon(Flt x, Flt y, int n, fVec *points, int u, int v)
{
   int qi, ri, qj, rj;
   int crossings, i, j;
   Flt b, m;

   crossings = 0;
   for (i=0;i<n;i++) {
      j = (i + 1) % n;
      qi = ri = qj = rj = 0;
      if (points[i][v] == points[j][v]) continue;
      if (points[i][v] < y) qi = 1;
      if (points[j][v] < y) qj = 1;
      if (qi == qj) continue;
      if (points[i][u] < x) ri = 1;
      if (points[j][u] < x) rj = 1;
      if (ri&rj) { crossings++; continue; }
      if ((ri|rj) == 0) continue;
      m = (points[j][v] - points[i][v]) / (points[j][u] - points[i][u]);
      b = (points[j][v] - y) - m * (points[j][u] - x);
      if ((-b / m) < EPSILON) crossings++;
      }
   if (crossings & 1)
      return 1;
   else
      return 0;
}

/* Test to see if "point" is inside a quadratic Bezier contour. */
int
Inside_Contour(Flt x, Flt y, int itype, int n, fVec *points)
{
   int qi, ri, qj, rj;
   int crossings, i, j, k;
   Flt b, m, x0, y0, x1, y1, x2, y2;
   Flt t, xc, xt[3], yt[3], roots[2];

   crossings = 0;
   x0 = points[0][0];
   y0 = points[0][1];
   for (i=1;i<n;i++) {
      /* Grab the control vertices */
      x1 = points[i][0];
      y1 = points[i][1];

      /* Last point must be on curve, all others may float. */
      if (itype == 1 || (itype == 3 && points[i][2] == 0.0)) {
         /* Straight line segment */
         qi = ri = qj = rj = 0;
         if (y0 == y1) goto next_segment;
         if (y0 < y) qi = 1;
         if (y1 < y) qj = 1;
         if (qi == qj) goto next_segment;
         if (x0 > x) ri = 1;
         if (x1 > x) rj = 1;
         if (ri&rj) { crossings++; goto next_segment; }
         if ((ri|rj) == 0) goto next_segment;
         m = (y1 - y0) / (x1 - x0);
         b = (y1 - y) - m * (x1 - x);
         if ((b / m) < EPSILON) { crossings++; }
next_segment:
         x0 = x1; y0 = y1;
         }
      else {
         /* The i+1 below won't fail as long as the
            z coordinate of the last control point is 0. */
         x2 = points[i+1][0];
         y2 = points[i+1][1];

         if ((itype == 2 && i < n-2) || (itype == 3 && points[i+1][2] != 0.0)) {
            /* Parabola with far end floating - readjust the far end
               so that it is on the curve.  (In the correct place too.) */
            x2 = 0.5 * (x1 + x2);
            y2 = 0.5 * (y1 + y2);
            }
         /* only test crossing when y is in the range */
         /* this also helps saving some computations  */
         if (((y0 < y) && (y1 < y) && (y2 < y)) ||
             ((y0 > y) && (y1 > y) && (y2 > y)))
            goto l0;


         /* Make the interpolating quadrics */
         yt[0] = y0 - 2.0 * y1 + y2;
         yt[1] = 2.0 * (y1 - y0);
         yt[2] = y0 - y;

#if 1
         /* Figure out where the quadratic intersects
            the x-axis */
         j = solve_quadratic(yt, roots, 0.0, 1.0);

         /* there are still situations that end points */
         /* may be counted twice.                      */
         for (k = 0; k < j; k++) {
            /* if the root is very close to the starting */
            /* point, check if it really intersects the  */
            /* curve segment.                            */
            if (roots[k] <= EPSILON) {
               /* if y actually is not in range, */
               /* discard the root.              */
               if (((y <= y0) && (y < y1)) ||
                   ((y >= y0) && (y > y1))) {
                  j--;
                  if (j > k)
                     roots[k] = roots[k+1];
                  continue;
                  }
               }
             /* if the root is very close to the ending  */
             /* point, check if it really intersects the */
             /* curve segment.                           */
             else if (roots[k] >= (1.0 - EPSILON)) {
                /* if y actually is not in range, */
                /* discard the root.              */
                if (((y < y2) && (y < y1)) ||
                    ((y > y2) && (y > y1))) {
                   j--;
                   if (j > k)
                      if (k<=0) roots[k] = roots[k+1];//check CM
                   continue;
                   }
                }
              }
#else
         /* Figure out where the quadratic intersects
            the x-axis */
         j = solve_quadratic(yt, roots, EPSILON, 1.0);
#endif
         if (j > 0) {
            xt[0] = x0 - 2.0 * x1 + x2;
            xt[1] = 2.0 * (x1 - x0);
            xt[2] = x0;
            t = roots[0];
            xc = (xt[0] * t + xt[1]) * t + xt[2];
            if (xc > x) { crossings++; }
            if (j > 1) {
               t = roots[1];
               xc = (xt[0] * t + xt[1]) * t + xt[2];
               if (xc > x) { crossings++; }
               }
            }
l0:

         /* Set up for next segment/arc */
         x0 = x2;
         y0 = y2;
         }
      }

   return (crossings & 1);
}

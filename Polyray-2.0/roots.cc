/** @file roots.cc
 *  @brief Polynomial root solvers (linear through degree-17 via Sturm sequences).
 *
 *  Input polynomials must have the coefficient of the highest power first.
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

#include <vector>

#include "defs3.h"
#include "vector.h"
#include "roots.h"
#include "io_ply.h"

constexpr Flt PLY_EPSILON2 = 1.0e-10;
constexpr Flt MAX_ITERATIONS=50;
constexpr Flt COEFF_LIMIT=1.0e-20;
constexpr Flt POLISH_PLY_EPSILON=1.0e-5;

#define FUDGE_FACTOR1 1.0e11
#define FUDGE_FACTOR2 -1.0e-5
#define FUDGE_FACTOR3 1.0e-7

#define MAX_STURM_ORDER 17
/** @brief Polynomial in the Sturm sequence, stored in ascending-degree order. */
typedef struct p {
   int ord;                        /**< Degree of the polynomial. */
   LFlt coef[MAX_STURM_ORDER+1];  /**< Coefficients coef[0]...coef[ord], ascending degree. */
   } polynomial;

constexpr int BINOMSIZE = 40;

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

/** @brief Remove all factors of @p i from @p n, recording each in @p s.
 *  @param n        Integer to factor.
 *  @param factor   Prime factor to extract.
 *  @param c        Running factor count; incremented for each extracted factor.
 *  @param factors  Output array; each extracted factor is appended at s[(c)++].
 *  @return   @p    n with all factors of @p i divided out.
 */
static int factor_out(int n, int factor, int& c, int *factors)
{
   while (!(n % factor)) {
      n /= factor;
      factors[(c)++] = factor;
      }
   return n;
}

static int factor_out_to_vector(int n, int factor, std::vector<int>& factors)
{
    while (n % factor == 0) {
        n /= factor;
        factors.push_back(factor);
    }
    return n;
}

//That version is clearer and safer, but it would ripple into factor1() and binomial() because they currently rely on fixed C arrays.So the practical modernization path is :
//
//Only move to std::vector<int> if you're ready to refactor the surrounding helpers too.
//One more small improvement : if invalid input is possible, guard factor <= 1 to avoid a bad loop.

#ifdef TESTING
int factor_out_test(int n, int i, int& c, int *s)
{
   return factor_out(n, i, c, s);
}
#endif

//convert our int * to a std::vector<int> 
static std::vector<int> processArray(int* arr, size_t size) {
    // One-liner that creates and returns the vector
    return std::vector<int>(arr, arr + size);
}

/** @brief Decompose @p n into its prime factors.
 *  @param n             Positive integer to factor; must be less than 2^15.
 *  @param factor_count  Running factor count; incremented for each prime factor found.
 *  @param s  Output array; prime factors are appended starting at s[*c].
 */
static void factor1(int n, int& factor_count, std::vector<int>& factors_vec
//    int *factors
)
{
   const auto start_size = factors_vec.size();
   /* First factor out any 2s */
   n = factor_out_to_vector(n, 2, factors_vec);
   /* Now any odd factors */
   int k = std::sqrt(n) + 1;
   for (int i=3;n>1 && i<=k;i+=2)
      if (!(n%i)) {
         n = factor_out_to_vector(n, i, factors_vec);
         k = std::sqrt(n)+1;
         }
   if (n>1)
      factors_vec.push_back( n );
   factor_count = static_cast<int>(factors_vec.size() - start_size);
}

/** @brief Compute the binomial coefficient C(@p n, @p r).
 *
 *  Uses a precomputed table for n < 15 and r < 15; falls back to prime
 *  factorisation for larger values.
 *  @param n  Total number of items (n >= 0).
 *  @param r  Items chosen (0 <= r <= n).
 *  @return   C(n, r), or 0 if the arguments are out of range.
 */
long binomial(int n, int r)
{
   int h,i,k,l;
   unsigned long result;
   std::vector<int> stack1 {};
   if (n<0 || r<0 || r>n)
      result = 0L;
   else if (r==n)
      result = 1L;
   else if (r < 15 && n < 15)
      result = (long)binomials[n][r];
   else {
      std::vector<int> stack2;
      for (i=r+1;i<=n;i++)
          stack1.push_back(i);
      for (i=2;i<=(n-r);i++) {
         h = 0;
         stack2.clear();
         factor1(i, h, stack2);
         for (k=0;k<h;k++) {
            for (l=0;l< static_cast<int>(stack1.size()) ;l++)
               if (!(stack1[l] % stack2[k])) {
                  stack1[l] /= stack2[k];
                  goto l1;
                  }
            swarning("Failed to factor %d from: ", stack2[k]);
            l1:;
            }
         }
      result=1;
      for (i=0;i< static_cast<int>(stack1.size()) ;i++)
         result *= stack1[i];
      }
   return result;
}

/** @brief Compute the polynomial remainder r(x) = u(x) mod v(x).
 *
 *  @note Assumes the leading coefficient of @p v is 1 or -1.
 *  @param u  Dividend polynomial.
 *  @param v  Divisor polynomial (leading coefficient must be +/-1).
 *  @param r  Output: remainder polynomial u mod v.
 *  @return   Degree of the remainder; 0 when r(x) is a constant.
 */
static int modp(polynomial *u, polynomial *v, polynomial *r)
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

/** @brief Build the Sturm sequence for a polynomial.
 *
 *  Fills @p sseq[0] with the input polynomial, @p sseq[1] with its
 *  normalised derivative, and @p sseq[2...] with successive remainders.
 *  @param ord   Degree of the polynomial in sseq[0].
 *  @param sseq  Array of at least (ord+1) polynomial slots; sseq[0].coef
 *               must be pre-filled with the input coefficients.
 *  @return      Length of the Sturm sequence (index of the last element).
 */
static int buildsturm(int ord, polynomial *sseq)
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

/** @brief Evaluate a degree-@p n polynomial at @p x using Horner's method.
 *  @param x       Evaluation point.
 *  @param n       Degree of the polynomial.
 *  @param Coeffs  Coefficient array in ascending-degree order (Coeffs[0] = constant term).
 *  @return        Value of the polynomial at @p x.
 */
static LFlt polyeval(LFlt x, int n, LFlt *Coeffs)
{   
   LFlt *tcoef;
   LFlt val;

   tcoef = &Coeffs[n];
   val = *tcoef--;
   for (int i=n-1;i>=0;i--,tcoef--)
      val = val * x + *tcoef;
   return val;
}

/** @brief Count sign changes in the Sturm sequence evaluated at @p a.
 *
 *  The number of distinct real roots in (a, b) equals the difference between
 *  the sign-change counts at the two endpoints.
 *  @param np    Length of the Sturm sequence (as returned by buildsturm()).
 *  @param sseq  Sturm sequence array.
 *  @param a     Evaluation point.
 *  @return      Number of sign changes in sseq[0..np] at @p a.
 */
static int numchanges(int np, polynomial *sseq, LFlt a)
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


/** @brief Refine a root in [@p a, @p b] using the regula-falsa method.
 *
 *  Falls back to bisection if regula-falsa does not converge within
 *  MAX_ITERATIONS steps.
 *  @param order  Degree of the polynomial.
 *  @param coef   Polynomial coefficients in ascending-degree order.
 *  @param a      Left bracket; must satisfy f(a) * f(b) <= 0.
 *  @param b      Right bracket.
 *  @param val    Output: refined root value when a root is found.
 *  @return       1 if a root was found and written to @p val, 0 otherwise.
 */
static int regula_falsa(int order, LFlt *coef, LFlt a, LFlt b, Flt *val)
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

      if (fabs(x) > PLY_EPSILON2) {
         if (fabs(fx / x) < PLY_EPSILON) {
            *val = x;
            return 1;
            }
         }
      else if (fabs(fx) < PLY_EPSILON) {
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
      if (fabs(b-a) < PLY_EPSILON2) {
         /* Check for underflow in the domain */
         *val = x;
         return 1;
         }
      lfx = fx;
      }
   return 0;
}

/** @brief Refine a root in [@p a, @p b] using a guarded Newton-Raphson method.
 *
 *  Uses the midpoint as the initial guess, keeps iterates inside the bracket,
 *  and returns 0 when the derivative is too small or a Newton step becomes
 *  unsafe so the caller can fall back to a more conservative method.
 *
 *  @param order  Degree of the polynomial.
 *  @param coef   Polynomial coefficients in ascending-degree order.
 *  @param a      Left bracket; must satisfy f(a) * f(b) <= 0.
 *  @param b      Right bracket.
 *  @param val    Output: refined root value when a root is found.
 *  @return       1 if a root was found and written to @p val, 0 otherwise.
 */
static int newton_raphson_guarded(int order, LFlt *coef, LFlt a, LFlt b, Flt *val)
{
   LFlt fa, fb, x, fx, dfx, next_x;

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

   x = (a + b) / 2.0;

   for (int its = 0; its < MAX_ITERATIONS; its++) {
      /* Simultaneous Horner evaluation of f(x) and f'(x). */
      fx = coef[order];
      dfx = 0.0;
      for (int i = order - 1; i >= 0; i--) {
         dfx = dfx * x + fx;
         fx = fx * x + coef[i];
         }

      if (fabs(x) > PLY_EPSILON2) {
         if (fabs(fx / x) < PLY_EPSILON) {
            *val = x;
            return 1;
            }
         }
      else if (fabs(fx) < PLY_EPSILON) {
         *val = x;
         return 1;
         }

      /* Flat derivative: let the caller fall back to regula-falsa/bisection. */
      if (fabs(dfx) < COEFF_LIMIT)
         return 0;

      next_x = x - fx / dfx;

      /* Keep Newton bracket-safe. */
      if (next_x <= a || next_x >= b)
         return 0;

      if (fabs(b - a) < PLY_EPSILON2) {
         *val = next_x;
         return 1;
         }

      if (fa < 0.0) {
         if (fx < 0.0) {
            a = x;
            fa = fx;
            }
         else {
            b = x;
            fb = fx;
            }
         }
      else if (fx < 0.0) {
         b = x;
         fb = fx;
         }
      else {
         a = x;
         fa = fx;
         }

      x = next_x;
      }

   return 0;
}
/** @brief Isolate and refine all roots of a polynomial in [@p min, @p max].
 *
 *  Uses the Sturm sequence to count roots in sub-intervals, then recurses
 *  and hands off to regula_falsa() for refinement.
 *
 *  @note Known limitation: when [min, max] contains a root at an endpoint
 *        as well as an interior root, the endpoint root is returned instead
 *        of the interior one.
 *  @param np     Length of the Sturm sequence.
 *  @param sseq   Sturm sequence array.
 *  @param min    Left bound of the search interval.
 *  @param max    Right bound of the search interval.
 *  @param atmin  Sign-change count at @p min (from numchanges()).
 *  @param atmax  Sign-change count at @p max (from numchanges()).
 *  @param roots  Output array; found roots are written here in order of magnitude.
 *  @return       Number of roots found and written to @p roots.
 */
static int sbisect(int np, polynomial *sseq, LFlt min, LFlt max,
        int atmin, int atmax, Flt *roots)
{
   LFlt  mid;
   int  n1, n2,atmid;

   //fix here!
   if ((atmin - atmax) == 1) {
      /* First try guarded Newton-Raphson, then fall back to regula-falsa. */
      if (newton_raphson_guarded(sseq->ord, sseq->coef, min, max, roots))
         return 1;
      else if (regula_falsa(sseq->ord, sseq->coef, min, max, roots))
         return 1;
      else {
         /* That failed, so now find it by bisection */
         for (int its = 0; its < MAX_ITERATIONS; its++) {
            mid = (min + max) / 2;
            atmid = numchanges(np, sseq, mid);
            if (fabs(mid) > PLY_EPSILON) {
               if (fabs((max - min) / mid) < PLY_EPSILON2) {
                  roots[0] = mid;
                  return 1;
                  }
               }
            else if (fabs(max - min) < PLY_EPSILON2) {
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
   for (int its = 0; its < MAX_ITERATIONS; its++) {
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

/** @brief Solve the linear equation x[0]*t + x[1] = 0.
 *  @param x        Coefficients: x[0] is the leading term, x[1] the constant.
 *  @param y        Output: root written to y[0] when found.
 *  @param mindist  Lower bound of the valid root range (inclusive).
 *  @param maxdist  Upper bound of the valid root range (inclusive).
 *  @return         Number of real roots in [mindist, maxdist]: 0 or 1.
 */
int solve_linear(Flt *x, Flt *y, Flt mindist, Flt maxdist)
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

/** @brief Solve the quadratic equation x[0]*t^2 + x[1]*t + x[2] = 0.
 *
 *  Degenerates gracefully to solve_linear() when x[0] is negligible.
 *  @param x        Coefficients in descending-degree order.
 *  @param y        Output: up to two roots written to y[0] and y[1].
 *  @param mindist  Lower bound of the valid root range (inclusive).
 *  @param maxdist  Upper bound of the valid root range (inclusive).
 *  @return         Number of real roots in [mindist, maxdist]: 0, 1, or 2.
 */
int solve_quadratic(Flt *x, Flt *y, Flt mindist, Flt maxdist)
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
   if (d < -PLY_EPSILON2) {
      return 0;
      }
   else if (fabs(d) <= PLY_EPSILON2) {
      q = 0.5 * b / a;
      if (q >= mindist && q <= maxdist) {
         y[0] = q;
         return 1;
         }
      return 0;
      }
   d = std::sqrt(d);
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

/** @brief Solve the cubic equation x[0]*t^3 + x[1]*t^2 + x[2]*t + x[3] = 0.
 *
 *  Uses Cardano's trigonometric method.  Degenerates gracefully to
 *  solve_quadratic() when x[0] is negligible.
 *
 *  @note The caller is responsible for determining which roots are distinct.
 *  @param x        Coefficients in descending-degree order.
 *  @param y        Output: up to three roots written to y[0]...y[2].
 *  @param mindist  Lower bound of the valid root range (exclusive).
 *  @param maxdist  Upper bound of the valid root range (exclusive).
 *  @return         Number of real roots in (mindist, maxdist): 0-3.
 */
int solve_cubic(Flt *x, Flt *y, Flt mindist, Flt maxdist)
{
   Flt Q, R, Q3, R2, sQ, d, an, theta;
   Flt A2, a0, a1, a2, a3;
   
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
   int i = 0;
   if (d >= 0.0) {
      /* Three real roots. */
      d = R / sqrt(Q3);
      theta = acos(d) / 3.0;
      sQ = -2.0 * sqrt(Q);
      d = sQ * cos(theta) - an;
      if (d > mindist && d < maxdist)
         y[i++] = d;
      d = sQ * cos(theta + PYTWO_PI_3) - an;
      if (d > mindist && d < maxdist)
         y[i++] = d;
      d = sQ * cos(theta + PYTWO_PI_43) - an;
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

/** @brief Detect and mitigate ill-conditioned polynomial coefficients.
 *
 *  If the largest coefficient exceeds the smallest by more than FUDGE_FACTOR1,
 *  very small coefficients are zeroed in place to improve numerical stability.
 *  @param n  Degree of the polynomial.
 *  @param x  Coefficient array (descending-degree order); modified in place.
 *  @return   1 if coefficients were ill-conditioned (and potentially zeroed), 0 otherwise.
 */
static int difficult_coeffs(int n, Flt *x)
{
   Flt t, biggest, smallest;

   biggest = smallest = fabs(x[0]);
   for (int i=1; i<=n; i++) {
      t = fabs(x[i]);
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
      for (int i=0; i<=n; i++) {
         t = fabs(x[i]);
         if (fabs(t) < POLISH_PLY_EPSILON && t/biggest < PLY_EPSILON2)
            x[i] = 0.0;
         }
      return 1;
      }

   return 0;
}

#ifdef TESTING
void factor1_test(int n, int& c, int *s)
{
   std::vector<int> factors_vec;
   factor1(n, c, factors_vec);
   for (int i = 0; i < c; ++i)
      s[i] = factors_vec[i];
}

int modp_test(int u_ord, const LFlt *u_coef, int v_ord, const LFlt *v_coef,
              int *r_ord, LFlt *r_coef)
{
   polynomial u = {};
   polynomial v = {};
   polynomial r = {};

   u.ord = u_ord;
   v.ord = v_ord;
   for (int i = 0; i <= u_ord; ++i)
      u.coef[i] = u_coef[i];
   for (int i = 0; i <= v_ord; ++i)
      v.coef[i] = v_coef[i];

   int ret = modp(&u, &v, &r);
   *r_ord = r.ord;
   for (int i = 0; i <= r.ord; ++i)
      r_coef[i] = r.coef[i];
   return ret;
}

int buildsturm_test(int ord, const LFlt *coef, int *derived_ord, LFlt *derived_coef)
{
   polynomial sseq[MAX_STURM_ORDER + 1] = {};
   for (int i = 0; i <= ord; ++i)
      sseq[0].coef[i] = coef[i];

   int ret = buildsturm(ord, sseq);
   *derived_ord = sseq[1].ord;
   for (int i = 0; i <= sseq[1].ord; ++i)
      derived_coef[i] = sseq[1].coef[i];
   return ret;
}

LFlt polyeval_test(LFlt x, int n, const LFlt *Coeffs)
{
   return polyeval(x, n, const_cast<LFlt *>(Coeffs));
}

int numchanges_test(int ord, const LFlt *coef, LFlt a)
{
   polynomial sseq[MAX_STURM_ORDER + 1] = {};
   for (int i = 0; i <= ord; ++i)
      sseq[0].coef[i] = coef[i];
   int np = buildsturm(ord, sseq);
   return numchanges(np, sseq, a);
}

int regula_falsa_test(int order, const LFlt *coef, LFlt a, LFlt b, Flt *val)
{
   LFlt local[MAX_STURM_ORDER + 1] = {};
   for (int i = 0; i <= order; ++i)
      local[i] = coef[i];
   return regula_falsa(order, local, a, b, val);
}

int newton_raphson_guarded_test(int order, const LFlt *coef, LFlt a, LFlt b, Flt *val)
{
   LFlt local[MAX_STURM_ORDER + 1] = {};
   for (int i = 0; i <= order; ++i)
      local[i] = coef[i];
   return newton_raphson_guarded(order, local, a, b, val);
}

int sbisect_test(int ord, const LFlt *coef, LFlt min, LFlt max, Flt *roots)
{
   polynomial sseq[MAX_STURM_ORDER + 1] = {};
   for (int i = 0; i <= ord; ++i)
      sseq[0].coef[i] = coef[i];
   int np = buildsturm(ord, sseq);
   int atmin = numchanges(np, sseq, min);
   int atmax = numchanges(np, sseq, max);
   return sbisect(np, sseq, min, max, atmin, atmax, roots);
}

int difficult_coeffs_test(int n, Flt *x)
{
   return difficult_coeffs(n, x);
}
#endif

/** @brief Solve the quartic x[0]*t^4+x[1]*t^3+x[2]*t^2+x[3]*t+x[4]=0 (Ferrari, 1731).
 *
 *  Reduces to a cubic resolvent and solves two derived quadratics.
 *  Falls back to bounded_polysolve() when coefficients are ill-conditioned.
 *  Degenerates to solve_cubic() when x[0] or x[4] is negligible.
 *  @param x        Coefficients in descending-degree order.
 *  @param results  Output: up to four roots written to results[0]...results[3].
 *  @param mindist  Lower bound of the valid root range (exclusive).
 *  @param maxdist  Upper bound of the valid root range (exclusive).
 *  @return         Number of real roots in (mindist, maxdist): 0-4.
 */
int solve_quartic(Flt *x, Flt *results, Flt mindist, Flt maxdist)
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
   else if (fabs(x[1]) < PLY_EPSILON2 && fabs(x[3]) < PLY_EPSILON2) {
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

/** @brief Solve the quartic x[0]*t^4+...+x[4]=0 using Vieta's method (1735).
 *
 *  Alternative quartic solver via a cubic resolvant and two quadratic factors.
 *  Falls back to bounded_polysolve() when coefficients are ill-conditioned.
 *  Degenerates to solve_cubic() when x[0] or x[4] is negligible.
 *  @param x        Coefficients in descending-degree order.
 *  @param results  Output: up to four roots written to results[0]...results[3].
 *  @param mindist  Lower bound of the valid root range (exclusive).
 *  @param maxdist  Upper bound of the valid root range (exclusive).
 *  @return         Number of real roots in (mindist, maxdist): 0-4.
 */
int solve_quartic1(Flt *x, Flt *results, Flt mindist, Flt maxdist)
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
   else if (fabs(x[1]) < PLY_EPSILON2 && fabs(x[3]) < PLY_EPSILON2) {
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
      if (d1 > -PLY_EPSILON2)
         d1 = 0.0;
      else
         return 0;
      }
   if (d1 < PLY_EPSILON) {
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

/** @brief Solve a polynomial of arbitrary degree using Sturm sequences.
 *
 *  Builds the Sturm sequence, counts roots in [mindist, maxdist], then
 *  calls sbisect() to isolate and refine each root.  Degenerates to the
 *  closed-form solvers (cubic/quadratic/linear) for low-degree polynomials.
 *  @param order    Degree of the polynomial; must be less than MAX_STURM_ORDER.
 *  @param Coeffs   Coefficients in descending-degree order.
 *  @param roots    Output array; found roots are written here.
 *  @param mindist  Lower bound of the search interval.
 *  @param maxdist  Upper bound of the search interval.
 *  @return         Number of real roots found in [mindist, maxdist].
 */
int bounded_polysolve(int order, Flt *Coeffs, Flt *roots,
                  Flt mindist, Flt maxdist)
{
   polynomial sseq[MAX_STURM_ORDER+1];
   int i, j, n, nroots, np, atmin, atmax;
extern int current_row, current_col;

   if (order >= MAX_STURM_ORDER)
      serror("Polynomials of order %d are too complex, max order is: %d\n",
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

/** @brief Test whether the 2-D point (@p x, @p y) lies inside a planar polygon.
 *
 *  Uses a ray-casting crossing-number test.
 *  @param x       X coordinate of the query point (in the @p u axis).
 *  @param y       Y coordinate of the query point (in the @p v axis).
 *  @param n       Number of vertices in the polygon.
 *  @param points  Array of @p n 3-D vertices defining the polygon.
 *  @param u       Axis index (0, 1, or 2) used as the horizontal axis.
 *  @param v       Axis index (0, 1, or 2) used as the vertical axis.
 *  @return        1 if the point is inside the polygon, 0 otherwise.
 */
int Inside_Polygon(Flt x, Flt y, int n, fVec *points, int u, int v)
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
      if ((-b / m) < PLY_EPSILON2) crossings++;
      }
   if (crossings & 1)
      return 1;
   else
      return 0;
}

/** @brief Test whether the 2-D point (@p x, @p y) lies inside a quadratic Bezier contour.
 *
 *  Uses a crossing-number test that handles straight-line and curved segments.
 *  @param x       X coordinate of the query point.
 *  @param y       Y coordinate of the query point.
 *  @param itype   Contour encoding: 1 = all straight-line segments;
 *                 2 = all quadratic arcs (last segment on-curve);
 *                 3 = mixed - the z coordinate of each control point
 *                     distinguishes on-curve (z==0) from off-curve (z!=0).
 *  @param n       Number of control points in @p points.
 *  @param points  Array of @p n 3-D control-point coordinates (x, y, z).
 *  @return        1 if the point is inside the contour (odd crossing count), 0 otherwise.
 */
int Inside_Contour(Flt x, Flt y, int itype, int n, fVec *points)
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
         if ((b / m) < PLY_EPSILON) { crossings++; }
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
            if (roots[k] <= PLY_EPSILON2) {
               /* if y actually is not in range, */
               /* discard the root.              */
               if (((y <= y0) && (y < y1)) ||
                   ((y >= y0) && (y > y1))) {
                  j--;
                  if (j > k)
                     //roots[k] = roots[k+1];
                     //fix for error: Reading invalid data from 'roots':
                     if (k < 2 /*num_roots*/ - 1) {
                            roots[k] = roots[k+1];
                     }
                  continue;
                  }
               }
             /* if the root is very close to the ending  */
             /* point, check if it really intersects the */
             /* curve segment.                           */
             else if (roots[k] >= (1.0 - PLY_EPSILON2)) {
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
         j = solve_quadratic(yt, roots, PLY_EPSILON, 1.0);
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

/** @brief Test whether the 2-D point (@p x, @p y) lies inside a quadratic Bezier contour.
 *
 *  Uses a crossing-number test that handles straight-line and curved segments.
 *  @param x       X coordinate of the query point.
 *  @param y       Y coordinate of the query point.
 *  @param itype   Contour encoding: 1 = all straight-line segments;
 *                 2 = all quadratic arcs (last segment on-curve);
 *                 3 = mixed - the z coordinate of each control point
 *                     distinguishes on-curve (z==0) from off-curve (z!=0).
 *  @param n       Number of control points in @p points.
 *  @param points  Vector of @p n 3-D control-point coordinates (x, y, z).
 *  @return        1 if the point is inside the contour (odd crossing count), 0 otherwise.
 */
int Inside_Contour_CPP(Flt x, Flt y, int itype,
    int n, const std::vector<NuVec>& points)
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
         if ((b / m) < PLY_EPSILON) { crossings++; }
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
            if (roots[k] <= PLY_EPSILON2) {
               /* if y actually is not in range, */
               /* discard the root.              */
               if (((y <= y0) && (y < y1)) ||
                   ((y >= y0) && (y > y1))) {
                  j--;
                  if (j > k)
                     //roots[k] = roots[k+1];
                     //fix for error: Reading invalid data from 'roots':
                     if (k < 2 /*num_roots*/ - 1) {
                            roots[k] = roots[k+1];
                     }
                  continue;
                  }
               }
             /* if the root is very close to the ending  */
             /* point, check if it really intersects the */
             /* curve segment.                           */
             else if (roots[k] >= (1.0 - PLY_EPSILON2)) {
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
         j = solve_quadratic(yt, roots, PLY_EPSILON, 1.0);
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

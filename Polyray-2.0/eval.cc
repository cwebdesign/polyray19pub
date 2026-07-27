/** @file eval.cc
 *  @brief Parse-tree evaluator - simplifies/optimises expression trees using substitution values.
 *
 *  Polyray - MIT Licensed Revival
 *  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
 *  Copyright (C) 1999-2026, Clyde Meli, All rights reserved.

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

#include <iostream>
#include <numbers>
#include "defs3.h"

#include "polyray.tab.h"
#include "eval.h"
#include "io_ply.h"
#include "vector.h"
#include "image.h"
#include "intersec.h"
#include "trace.h"
#include "runtime_state.h"
#include "symtab.h"
#include "builder.h"
#include "spline.h"
#include "ripple.h"

//#if \_\_INTELLISENSE\_\_
#include "vec3.h"
//import "vec3.h";
//#endif



// Noise generator variables 
constexpr Flt LARGE         = 100000.0;
constexpr int HASH_SIZE     = 65535;
constexpr int DEFAULT_CYCLES = 1;
constexpr Flt pi_23 = 2.0944;



//#define spline(x) ((3.0 - 2.0 * (x)) * (x) * (x))
static int mult_table_size = 5;
static unsigned long mult_table[] = { 99137, 113713, 55237, 994472, 441973 };



openpolyray::Ripple rip;

/** @brief Perlin's improved (2002) fade/smoothstep - replaces the old s_curve macro.
 *
 *  Polynomial: 6t^5 - 15t^4 + 10t^3.
 *
 *  @param t  Input value in [0, 1].
 *  @return   Smoothed interpolation weight.
 */
inline Flt fade(Flt t) {

    return t * t * t * (t * (t * 6 - 15) + 10);
}

/** @brief Linear interpolation between two values.
 *  @param t  Blend factor in [0, 1].
 *  @param a  Value at t = 0.
 *  @param b  Value at t = 1.
 *  @return   Interpolated value a + t*(b-a).
 */
inline Flt lerp(Flt t, Flt a, Flt b)
{
    return a + t * (b - a);
};

/** @brief Dot product of gradient vector @p g with displacement (x, y, z).
 *  @param g  Gradient vector.
 *  @param x  X component of displacement.
 *  @param y  Y component of displacement.
 *  @param z  Z component of displacement.
 *  @return   Scalar dot product.
 */
inline Flt dot(const Vec& g, Flt x, Flt y, Flt z)
{
    return g[0] * x + g[1] * y + g[2] * z;
};



/** @brief Initialise the global ripple state. */
#ifndef TESTING
static
#endif
void init_ripples(void)
{
    rip.init_ripples();
}


/** @brief Perturb normal @p N with a ripple effect at point @p P.
 *  @param P      Surface point.
 *  @param N      Surface normal (modified in place).
 *  @param freq   Ripple frequency.
 *  @param phase  Phase offset.
 *  @param scale  Displacement magnitude.
 */
void ripples(Vec P, Vec N, Flt freq, Flt phase, Flt scale)
{
    openpolyray::Vec3 N3=rip.ripples(P, N, freq, phase, scale);
    VecCopy(N3, N); // N = N3;
}

/** @brief Evaluate a RIPPLE expression node into a vector result.
 *  @param subst  Surface substitution data (position, normal, etc.).
 *  @param node   Parse-tree node carrying ripple parameters.
 *  @param vval   Output vector (ripple displacement).
 *  @return       2 on success, 0 on error.
 */
static int eval_ripple(SUBST_PTR subst, NODE_PTR node, Vec vval)
{
   int i;
   Flt len, val, freq, phase;
   Vec P, Center;
   Flt ftmp1;
   Vec vval1;
   NODE_PTR tnode1;

   auto vec=std::get<vvarr>(node->exper_data);
   if (eval_node(subst, vec[0], &ftmp1, P, &tnode1) != 2) {
      if (subst == nullptr)
         return 0;
      else {
         serror("ripple coordinate must be vector");
         return 0;
      }
   }

   if (vec[1] == nullptr &&
       vec[2] == nullptr &&
       vec[3]) {
      /* Evaluate ripple with just the position */
      MakeVector(0, 0, 0, Center)
      freq  = 1.0;
      phase = 0.0;
      }
   else {
      i = eval_node(subst, vec[1], &ftmp1, Center, &tnode1);
      if (i != 2) {
         if (subst == nullptr)
            return 0;
         else {
            serror("ripple center must be vector");
            return 0;
         }
      }
      i = eval_node(subst, vec[2], &freq, vval1, &tnode1);
      if (i != 1) {
         serror("ripple frequency must be a floating point value");
         return 0;
      }
      i = eval_node(subst, vec[3], &phase, vval1, &tnode1);
      if (i != 1) {
         serror("ripple phase must be a floating point value");
         return 0;
      }
      }

   VecSub(P, Center, vval);
   len = VecDot(vval, vval);
   if (len < PLY_EPSILON)
      len = 1.0;
   else
      len = sqrt(len);
   val = cos(len * freq + phase) / len;
   VecScale(val, vval);

   return 2;
}


/** @brief Testable variant of eval_ripple with hard-coded input coordinates.
 *  @param subst  Surface substitution data.
 *  @param node   Parse-tree node (used for right/left branches only).
 *  @param vval   Output ripple displacement vector.
 *  @return       2 on success, 0 on error.
 */
#ifndef TESTING
static
#endif
int eval_ripplemock(SUBST_PTR subst, NODE_PTR node, Vec& vval)
{
    int i;
    Flt len, val, freq, phase;
    Vec P{ 1.0,2.0,3.0 }, Center{ 0,0,0 };
    Flt ftmp1;
    Vec vval1;
    NODE_PTR tnode1;

    //if (eval_node(subst, vec0, &ftmp1, P, &tnode1) != 2) {
    if (2 != 2) {
        //if (eval_node(subst, NULL /*for test case*/, &ftmp1, P, &tnode1) != 2) {
        if (subst == NULL)
            return 0;
        else {
            serror("ripple coordinate must be vector");
            return 0;
        }
    }

    //  if (node->vvec()[1] == NULL &&
     //     node->vvec()[2] == NULL &&
     //     node->vvec()[3])
    if (1) {
        /* Evaluate ripple with just the position */
        MakeVector(0, 0, 0, Center)
            freq = 1.0;
        phase = 0.0;
    }
    else {
        auto vec = std::get<vvarr>(node->exper_data);
        
        i = eval_node(subst, vec[1], &ftmp1, Center, &tnode1);
        if (i != 2) {
            if (subst == NULL)
                return 0;
            else {
                serror("ripple center must be vector");
                return 0;
            }
        }
        i = eval_node(subst, vec[2], &freq, vval1, &tnode1);
        if (i != 1) {
            serror("ripple frequency must be a floating point value");
            return 0;
        }
        i = eval_node(subst, vec[3], &phase, vval1, &tnode1);
        if (i != 1) {
            serror("ripple phase must be a floating point value");
            return 0;
        }
    }

    VecSub(P, Center, vval);
    len = VecDot(vval, vval);
    if (len < PLY_EPSILON)
        len = 1.0;
    else
        len = sqrt(len);
    val = cos(len * freq + phase) / len;
    VecScale(val, vval);

    return 2;
}


/** @brief Map three lattice coordinates to a pseudo-random scalar in [0, 1).
 *  @param x  X lattice index.
 *  @param y  Y lattice index.
 *  @param z  Z lattice index.
 *  @return   Hashed floating-point value in [0, 1).
 */
#ifndef TESTING
static
#endif
Flt hash3d(unsigned long x, unsigned long y, unsigned long z)
{
   unsigned long K, Kt;

   K = x | y | z;
   Kt = 0;
   for (int i=0;i<mult_table_size;i++)
      Kt = ((Kt + K) * mult_table[i]) % HASH_SIZE;
      /* Kt = ((Kt + K) * mult_table[i]) & 0xffff; */ /* HASH_SIZE = 65535 */
   Flt result = (Flt)Kt / (Flt)(HASH_SIZE - 1);
   return result;
}

/** @brief Trilinearly interpolated lattice noise at point @p P.
 *  @param P  3-D sample point.
 *  @return   Noise value in [0, 1].
 */
#ifndef TESTING
static
#endif
Flt flt_noise(Vec P)
{
   unsigned long ix, iy, iz, jx, jy, jz;
   Flt sx0, sy0, sz0, sx1, sy1, sz1;
   Flt t, result;
   Vec Pt;

   Pt[0] = P[0] + LARGE;
   Pt[1] = P[1] + LARGE;
   Pt[2] = P[2] + LARGE;
   ix = Pt[0];  iy = Pt[1];  iz = Pt[2];
   jx = ix + 1; jy = iy + 1; jz = iz + 1;

   /* Compute cubic interpolated influences of surrounding
      lattice points */
   t = Pt[0] - ix; sx0 = fade(t); sx1 = 1.0 - sx0;
   t = Pt[1] - iy; sy0 = fade(t); sy1 = 1.0 - sy0;
   t = Pt[2] - iz; sz0 = fade(t); sz1 = 1.0 - sz0;
   ix=(ix & 0x03ffL) << 20; iy=(iy & 0x03ffL) << 10; iz&=0x03ffL;
   jx=(jx & 0x03ffL) << 20; jy=(jy & 0x03ffL) << 10; jz&=0x03ffL;
   result = (((hash3d(ix, iy, iz) * sz1 +
               hash3d(ix, iy, jz) * sz0) * sy1) +
             ((hash3d(ix, jy, iz) * sz1 +
               hash3d(ix, jy, jz) * sz0) * sy0)) * sx1 +
            (((hash3d(jx, iy, iz) * sz1 +
               hash3d(jx, iy, jz) * sz0) * sy1) +
             ((hash3d(jx, jy, iz) * sz1 +
               hash3d(jx, jy, jz) * sz0) * sy0)) * sx0;
   return result;
}

constexpr int Perlin_B      = 256;

static int Perlin_p[Perlin_B + Perlin_B + 2];
static Vec Perlin_g[Perlin_B + Perlin_B + 2];
static int Perlin_Flag = 1;

struct PerlinNoiseSetupResult {
    int b0;
    int b1;
    Flt r0;
    Flt r1;
};

inline PerlinNoiseSetupResult perlin_noise_setup(const Vec P, int axis)
{
    const Flt t = P[axis] + static_cast<Flt>(10000.0231);
    const auto base = static_cast<long>(t);
    const int b0 = static_cast<int>(base) & (Perlin_B - 1);

    return {
        b0,
        (b0 + 1) & (Perlin_B - 1),
        t - static_cast<Flt>(base),
        t - static_cast<Flt>(base) - static_cast<Flt>(1.0)
    };
}

/** @brief Seed the Perlin gradient table and permutation array (runs once). */
#ifndef TESTING
static
#endif
void Perlin_init(void)
{
   int i, j, k;
   Vec v;
   Flt s;

   /* Create an array of random gradient vectors uniformly on the unit sphere */
   srand(1);
   for (i=0;i<Perlin_B;i++) {
      do {
         for (j=0;j<3;j++)
            v[j] = (Flt)((rand() % (Perlin_B << 1)) - Perlin_B) / (Flt)Perlin_B;
         s = VecNormalize(v);
         } while (s > 1.0);
      VecCopy(v, Perlin_g[i]);
      }
   /* Create a pseudorandom permutation of [1..B] */
   for (i=0;i<Perlin_B;i++)
      Perlin_p[i] = i;
   for (i=Perlin_B;i>0;i-=2) {
      k = Perlin_p[i];
      j = rand() % Perlin_B;
      Perlin_p[i] = Perlin_p[j];
      Perlin_p[j] = k;
      }
   /* Extend g and p arrays to allow for faster indexing */
   for (i=0;i<Perlin_B+2;i++) {
      Perlin_p[Perlin_B + i] = Perlin_p[i];
      VecCopy(Perlin_g[i], Perlin_g[Perlin_B+i])
      }
}

inline Flt perlin_at(const Flt* q, Flt rx, Flt ry, Flt rz)
{
    return rx * q[0] + ry * q[1] + rz * q[2];
}


/** @brief Classic Perlin gradient noise at point @p P.
 *  @param P  3-D sample point.
 *  @return   Noise value approximately in [0, 1].
 */
#ifndef TESTING
static
#endif
Flt Perlin_noise(Vec P)
{
   int b00, b10, b01, b11;
   Flt *q, sx, sy, sz, a, b, c, d, t, u, v;
   int i, j;

   if (Perlin_Flag) {
      Perlin_Flag = 0;
      Perlin_init();
      }

   auto [bx0, bx1, rx0, rx1] = perlin_noise_setup(P, 0);
   
   auto [by0, by1, ry0, ry1] = perlin_noise_setup(P, 1);
  
   auto [bz0, bz1, rz0, rz1] = perlin_noise_setup(P, 2);

   i = Perlin_p[bx0];
   j = Perlin_p[bx1];

   b00 = Perlin_p[i + by0];
   b10 = Perlin_p[j + by0];
   b01 = Perlin_p[i + by1];
   b11 = Perlin_p[j + by1];

//#define at(rx,ry,rz) (rx * q[0] + ry * q[1] + rz * q[2]) //todo:replace macros here
  

   sx = fade(rx0);
   sy = fade(ry0);
   sz = fade(rz0);

   q = Perlin_g[b00 + bz0]; u = perlin_at(q,rx0, ry0, rz0);
   q = Perlin_g[b10 + bz0]; v = perlin_at(q,rx1, ry0, rz0);
   a = lerp(sx, u, v);

   q = Perlin_g[b01 + bz0]; u = perlin_at(q,rx0, ry1, rz0);
   q = Perlin_g[b11 + bz0]; v = perlin_at(q,rx1, ry1, rz0);
   b = lerp(sx, u, v);

   c = lerp(sy, a, b);

   q = Perlin_g[b00 + bz1]; u = perlin_at(q, rx0, ry0, rz1);
   q = Perlin_g[b10 + bz1]; v = perlin_at(q, rx1, ry0, rz1);
   a = lerp(sx, u, v);

   q = Perlin_g[b01 + bz1]; u = perlin_at(q, rx0, ry1, rz1);
   q = Perlin_g[b11 + bz1]; v = perlin_at(q, rx1, ry1, rz1);
   b = lerp(sx, u, v);

   d = lerp(sy, a, b);

   return (0.75 * lerp(sz, c, d) + 0.5);
}

/** @brief Fractal (absolute-value turbulence) noise - sum of |Perlin| octaves.
 *  @param P            3-D sample point.
 *  @param pos_scale    Spatial scale multiplier applied between octaves.
 *  @param noise_scale  Amplitude multiplier applied between octaves.
 *  @param octaves      Number of octaves to accumulate.
 *  @return             Normalised turbulence value in [0, 1].
 */
Flt fnoise(Vec P, Flt pos_scale, Flt noise_scale, int octaves)
{
   Vec Pt;
   Flt result = 0.0;
   Flt scale  = 1.0;
   Flt magnitude = 0.0;   

   if (octaves <= 0)
      return 0.0;
   VecCopy(P, Pt);
   for (int i=0;i<octaves;i++) {
     result += scale * fabs(Perlin_noise(Pt) - 0.5);
     magnitude += scale;
     if (i < octaves-1) {
        VecScale(pos_scale, Pt)
        scale *= noise_scale;
        }
     }
   result /= 0.5 * magnitude;
   return result;
}

/** @brief Multi-octave lattice noise (chaos/turbulence).
 *  @param P            3-D sample point.
 *  @param pos_scale    Spatial scale multiplier between octaves.
 *  @param noise_scale  Amplitude multiplier between octaves.
 *  @param octaves      Number of octaves to sum.
 *  @return             Normalised noise value in [0, 1].
 */
Flt Kaos(Vec P, Flt pos_scale, Flt noise_scale, int octaves)
{
   Vec Pt;
   Flt result = 0.0;
   Flt scale  = 1.0;
   Flt magnitude = 0.0;
   
   if (octaves <= 0)
      return 0.0;
   VecCopy(P, Pt);
   for (int i=0;i<octaves;i++) {
     result += scale * flt_noise(Pt);
     magnitude += scale;
     if (i < octaves-1) {
        VecScale(pos_scale, Pt)
        scale *= noise_scale;
        }
     }
   result /= magnitude;
   return result;
}

/** @brief Directional (vector) chaos noise - three offset Perlin noise values.
 *  @param P            3-D sample point.
 *  @param R            Output vector set to the normalised noise gradient.
 *  @param pos_scale    Spatial scale multiplier between octaves.
 *  @param noise_scale  Amplitude multiplier between octaves.
 *  @param octaves      Number of octaves to sum.
 */
void
dKaos(Vec P, Vec R, Flt pos_scale, Flt noise_scale, int octaves)
{
   Vec Pt;
   Flt scale, magnitude;
   
   /* MakeVector(0.0, 0.0, 0.0, R); */
   MakeVectorZero(R);
   if (octaves <= 0)
      return;
   for (int j=0;j<3;j++) {
      for (int i=0;i<3;i++)
         Pt[i] = P[(i+j)%3];
      scale = noise_scale;
      magnitude = 0.0;
      R[j] = 0.0;
      for (int i=0;i<octaves;i++) {
         R[j] += scale * Perlin_noise(Pt);
         magnitude += scale;
         if (i < octaves-1) {
            VecScale(pos_scale, Pt)
            scale *= noise_scale;
            }
         }
      R[j] /= 0.5 * magnitude;
      }
   VecNormalize(R);
}


/** @brief Directional lattice noise - three offset flt_noise values.
 *  @param P            3-D sample point.
 *  @param R            Output normalised displacement vector.
 *  @param pos_scale    Spatial scale multiplier between octaves.
 *  @param noise_scale  Amplitude multiplier between octaves.
 *  @param octaves      Number of octaves to sum.
 */
void dnoise3d(Vec P, Vec R, Flt pos_scale, Flt noise_scale, int octaves)
{
   Vec Pt;
   Flt scale, magnitude;

   /* MakeVector(0.0, 0.0, 0.0, R);*/
   MakeVectorZero(R);
   if (octaves <= 0)
      return;
   for (int j=0;j<3;j++) {
      for (int i=0;i<3;i++)
         Pt[i] = P[(i+j)%3];
      scale = noise_scale;
      magnitude = 0.0;
      R[j] = 0.0;
      for (int i=0;i<octaves;i++) {
         R[j] += scale * flt_noise(Pt);
         magnitude += scale;
         if (i < octaves-1) {
            VecScale(pos_scale, Pt)
            scale *= noise_scale;
            }
         }
      R[j] /= 0.5 * magnitude;
      }
   VecNormalize(R);
}

/** @brief Simulate Brownian motion by deflecting a particle from @p start.
 *  @param start   Starting position.
 *  @param cycles  Number of random push iterations.
 *  @param scale   Per-axis push magnitude.
 *  @param end     Output: final position after all pushes.
 */
#ifndef TESTING
static
#endif
void brownian_motion(Vec start, int cycles, Vec scale, Vec end)//we return a value in end. need to change this!
{
   /* Deflect a particle starting at "start" by random pushes
      of size "scale".  Repeat "cycles" times */
   Flt rnd;

   VecCopy(start, end);
   for (int i=0;i<cycles;i++)
      for (int j=0;j<3;j++) {
         rnd = 2.0 * polyray_random() - 1.0;
         end[j] += rnd * scale[j];
         }

}

/** @brief Testable variant of brownian_motion using a fixed displacement.
 *  @param start   Starting position.
 *  @param cycles  Number of push iterations.
 *  @param scale   Per-axis push magnitude.
 *  @param end     Output: final deflected position.
 */
#ifndef TESTING
static
#endif
void brownian_motionmock(Vec start, int cycles, Vec scale, Vec end)
//we return a value in end. need to change this!
{
    /* Deflect a particle starting at "start" by random pushes
       of size "scale".  Repeat "cycles" times */

    VecCopy(start, end);
    for (int i = 0; i < cycles; i++)
        for (int j = 0; j < 3; j++) {
            Flt rnd = 2.0 * 1.0 - 1.0;
            end[j] += rnd * scale[j];
        }
}

/** @brief Evaluate an FBM (fractal Brownian motion) expression node.
 *  @param subst  Surface substitution data.
 *  @param node   Parse-tree node carrying start and scale sub-expressions.
 *  @param vval   Output displacement vector.
 *  @return       2 on success, 0 on error.
 */
static int
eval_brownian_motion(SUBST_PTR subst, NODE_PTR node, Vec vval)
{
   NODE_PTR tnode;
   Flt fscale=0.0;
   Vec start{ 0,0,0 }, vscale{ 0,0,0 };
   int stype=0;

   if (eval_node(subst, node->left, &fscale, start, &tnode) == 2) {
      if (node->right == nullptr)
         MakeVector(0.1, 0.1, 0.1, vscale)
      else {
         stype = eval_node(subst, node->right, &fscale, vscale,
                           &tnode);
         if (stype == 1) {
            MakeVector(fscale, fscale, fscale, vscale);
            }
         else if (stype != 2)
            return 0;
         }

      brownian_motion(start, DEFAULT_CYCLES, vscale, vval);
      return 2;
      }
   else
      return 0;
}

/** @brief Testable variant of eval_brownian_motion.
 *  @param subst  Surface substitution data.
 *  @param node   Parse-tree node.
 *  @param vval   Output displacement vector.
 *  @return       2 on success, 0 on error.
 */
#ifndef TESTING
static
#endif
int eval_brownian_motionmock(SUBST_PTR subst, NODE_PTR node, Vec& vval)
{
    NODE_PTR tnode;
    Flt fscale=0;
    Vec start{ 0,0,0 }, vscale{ 0,0,0 };
    int stype=0;

    if (eval_node(subst, node, &fscale, start, &tnode) == 2) {
        if (node->right == NULL)
            MakeVector(0.1, 0.1, 0.1, vscale)
        else {
            stype = eval_node(subst, node->right, &fscale, vscale,
                &tnode);
            if (stype == 1) {
                MakeVector(fscale, fscale, fscale, vscale);
            }
            else if (stype != 2)
                return 0;
        }

        brownian_motionmock(start, DEFAULT_CYCLES, vscale, vval);
        return 2;
    }
    else
        return 0;
}

/** @brief Triangular sawtooth wave with period 1 - output in [0, 1].
 *  @param x  Input value (any real).
 *  @return   Sawtooth value in [0, 1].
 */
Flt sawtooth(Flt x)
{
   Flt y;

   if (x >= 0.0)
      x = x - floor(x);
   else
      x = 1 + x + floor(ABS(x));
   if (x >= 0.5)
      y = 2.0 * (1.0 - x);
   else
      y = 2.0 * x;
   return y;
}

/** @brief Ramp function - fractional part of @p x mapped to [0, 1).
 *  @param x  Input value.
 *  @return   Value in [0, 1), or 0 if |x| < epsilon.
 */
Flt
ramp(Flt x)
{
   if (fabs(x) < PLY_EPSILON)
      return 0.0;

   Flt y = fmod(x, 1.0);
   if (y < 0.0)
      y += 1.0;

   return y;
}

/** @brief Associated Legendre polynomial P_l^m(x) (from Numerical Recipes).
 *  @param l  Degree (non-negative integer).
 *  @param m  Order (0 <= m <= l).
 *  @param x  Evaluation point in [-1, 1].
 *  @return   P_l^m(x), or 0 if parameters are out of range.
 */
/* Legendre polynomial from "Numerical Recipes", Press, et. al. */
#ifndef TESTING
static
#endif
Flt legendre(int l, int m, Flt x)
{
   Flt fact, pll=0, pmm, pmmp1, somx2;   

   if (m < 0 || m > l || fabs(x) > 1.0)
      return 0.0;
   pmm = 1.0;
   if (m > 0) {
      somx2 = sqrt((1.0 - x) * (1.0 + x));
      fact = 1.0;
      for (int i=1;i<=m;i++) {
         pmm *= -fact * somx2;
         fact += 2.0;
         }
      }
   if (l == m)
      return pmm;
   else {
      pmmp1 = x * (2 * m + 1) * pmm;
      if (l == (m + 1))
         return pmmp1;
      else {
         for (int ll = m + 2; ll <= l; ll++) {
            pll = (x * (2 * ll - 1) * pmmp1 - (ll + m - 1) * pmm) / (ll - m);
            pmm = pmmp1;
            pmmp1 = pll;
            }
         return pll;
         }
      }
}

/** @brief Map a unit-sphere point to (u, v) texture coordinates.
 *  @param P  Point on the unit sphere.
 *  @param u  Output: horizontal texture coordinate in [0, 1].
 *  @param v  Output: vertical texture coordinate in [0, 1].
 *  @return   1 on success, 0 if @p P is too close to the origin.
 */
/* Map a point (x, y, z) on a sphere of radius 1 to a 2-d image. (Or is it the
   other way around?) */
int spherical_imagemap(Vec P, Flt *u, Flt *v)
{
   Flt len, phi, theta;
   Flt x, y, z;

   /* Make sure this vector is on the unit sphere. */
   len = VecLen(P);
   if (len < PLY_EPSILON) {
      swarning("Bad point: <%g, %g, %g>, dist = %g in spherical map\n",
              P[0], P[1], P[2], len);
      return 0;
      }
   else {
      x = P[0] / len;
      y = P[1] / len;
      z = P[2] / len;
      }
   /* Determine its angle from the x-z plane. */
   phi = asin(y);

   /* Determine its angle from the point (1, 0, 0) in the x-z plane. */
   len = sqrt(x * x + z * z);
   if (len == 0.0) {
      /* This point is at one of the poles. Any value of xcoord will be ok...*/
      theta = 0;
      }
   else {
      if (z == 0.0)
         if (x > 0)
            theta = 0.0;
         else
            theta = PYM_PI;
      else {
         theta = acos(x / len);
         if (z < 0.0) theta = 2.0 * PYM_PI - theta;
         }
      }
   *u = theta / (2.0 * PYM_PI);
   *v = 0.5 + (phi / PYM_PI);
   return 1;
}

/** @brief Map a unit-cylinder surface point to (u, v) texture coordinates.
 *
 *  The cylinder has radius 1, height 1, and its axis along the Y axis.
 *
 *  @param P  Point on the cylinder surface.
 *  @param u  Output: azimuthal coordinate in [0, 1].
 *  @param v  Output: height coordinate (P[1]).
 *  @return   1 always.
 */
/* Map a point (x, y, z) on a cylinder of radius 1, height 1, that has its
   axis of symmetry along the y-axis to the square [0,1]x[0,1]. */
#ifndef TESTING
static
#endif
int cylindrical_imagemap(Vec P, Flt* u, Flt* v)
{
   Flt x, len, theta;

   len = sqrt(P[0] * P[0] + P[2] * P[2]);
   /* Make sure this vector is on the unit cylinder. */
   if (len < PLY_EPSILON)
      theta = 0;
   else {
      x  = P[0] / len;
      if (P[2] == 0.0)
         if (x > 0)
            theta = 0.0;
         else
            theta = PYM_PI;
      else {
         theta = acos(x);
         if (P[2] < 0.0)
            theta = (2.0 * PYM_PI) - theta;
         }
      }
   *u = theta / (2.0 * PYM_PI);
   *v = P[1];
   return 1;
}

/** @brief Compute (u, v) texture coordinates from a surface point for a given map type.
 *  @param map_type  One of PLANAR_IMAGEMAP, SPHERICAL_IMAGEMAP, CYLINDRICAL_IMAGEMAP, etc.
 *  @param vleft     Surface point in object space.
 *  @param u         Output: horizontal texture coordinate.
 *  @param v         Output: vertical texture coordinate.
 */
#ifndef TESTING
static
#endif
void calculate_uv(int map_type, Vec vleft, Flt* u, Flt* v)
{
   if (map_type == PLANAR_IMAGEMAP ||
       map_type == PLANAR_BUMPMAP ||
       map_type == HEIGHT_MAP ||
       map_type == INDEXED_MAP) {
      /* Planar image maps and planar height maps */
      *u = vleft[0];
      *v = vleft[2];
      }
   else if (map_type == SPHERICAL_IMAGEMAP ||
            map_type == SPHERICAL_INDEXED ||
            map_type == SPHERICAL_BUMPMAP) {
      if (!spherical_imagemap(vleft, u, v)) {
         *u = 0;
         *v = 0;
         }
      }
   else if (map_type == CYLINDRICAL_IMAGEMAP ||
            map_type == CYLINDRICAL_INDEXED ||
            map_type == CYLINDRICAL_BUMPMAP) {
      if (!cylindrical_imagemap(vleft, u, v)) {
         *u = 0;
         *v = 0;
         }
      }
   else {
      serror("Unsupported image map type: %d\n", map_type);
      *u = 0;
      *v = 0;
   }
}

/** @brief Evaluate an image-map expression node and return a colour/height/index.
 *  @param map_type  Map projection type (PLANAR, SPHERICAL, CYLINDRICAL, HEIGHT, INDEXED).
 *  @param subst     Surface substitution data.
 *  @param node      Parse-tree node; left child holds the sample point, right the wrap flag.
 *  @param fval      Output: scalar result (alpha or height).
 *  @param vval      Output: colour vector.
 *  @return          2 for colour, 1 for scalar, 0 on error.
 */
static int
eval_imagemap(int map_type, SUBST_PTR subst, NODE_PTR node,
              Flt *fval, Vec vval)
{
   Flt fleft;
   Flt u, v;
   Vec vleft{ 0,0,0 };
   int rflag;
   auto param=std::get<NODE_PTR>(node->exper_data);
   NODE_PTR tnode, map = param;

   *fval = 0; //fval was uninitialised in Polyray!
   if (subst == nullptr)
      return 0;
   else if ((map->exper_type == IMAGE) &&
       (eval_node(subst, node->left, &fleft, vleft, &tnode) == 2)) {
      rflag = (node->right == nullptr ? 0 : 1);
      calculate_uv(map_type, vleft, &u, &v);
      switch (map_type) {
      case PLANAR_IMAGEMAP:
      case SPHERICAL_IMAGEMAP:
      case CYLINDRICAL_IMAGEMAP: {
         auto image=std::get<Img*>(map->exper_data);
         lookup_image_color(image, u, v, rflag, fval, vval);
         return 2;
      }
      case HEIGHT_MAP: {
         auto image=std::get<Img*>(map->exper_data);
         lookup_height(image, u, v, rflag, fval);
         return 1;
      }
      case INDEXED_MAP:
      case SPHERICAL_INDEXED:
      case CYLINDRICAL_INDEXED: {
         auto image=std::get<Img*>(map->exper_data);
         lookup_index(image, u, v, rflag, fval);
         return 1;
      }
      default:
         serror("Unknown map type");
         return 0;
         }
      }
   else {
      serror("Left node is %d, not an image node\n", node->left->exper_type);
      return 0;
      }
}


/** @brief Testable variant of eval_imagemap with pre-supplied image and coordinates.
 *  @param map_type  Map projection type.
 *  @param subst     Surface substitution data.
 *  @param node      Parse-tree node (right used for wrap flag).
 *  @param nodeleft  Unused left node placeholder.
 *  @param image     Image to sample directly.
 *  @param fval      Output: scalar result.
 *  @param vval      Output: colour vector.
 *  @return          2 always.
 */
#ifndef TESTING
static
#endif
int eval_imagemapmock(int map_type, SUBST_PTR subst, NODE_PTR node, NODE_PTR nodeleft, Img* image, Flt* fval, Vec& vval)
{ //extreme mock, hope its working
    Flt fleft;
    Flt u, v;
    Vec vleft{ 1.0,2.0,3.0 };
    int rflag;
    NODE_PTR tnode;// , map = node->vparam();

    *fval = 0;
    rflag = (node->right == NULL ? 0 : 1);
    calculate_uv(map_type, vleft, &u, &v);    
    lookup_image_color(image, u, v, rflag, fval, vval);
    return 2;
}

/** @brief Evaluate a bump-map expression node and return a perturbed normal.
 *  @param map_type  Map projection type (PLANAR, SPHERICAL, or CYLINDRICAL bump).
 *  @param subst     Surface substitution data (must include surface normal N).
 *  @param node      Parse-tree node; left: sample point, right: optional scale.
 *  @param vval      Output: perturbed surface normal (normalised).
 *  @return          2 on success, 0 on error.
 */
#ifndef TESTING
static
#endif
int eval_bumpmap(int map_type, SUBST_PTR subst, NODE_PTR node, Vec vval)
{
   Flt len, u, v, deltau, deltav;
   Flt height0, height1, height2;
   Vec N, N1, N2, A, B, C;
   Vec V0, V1, bump_scale;
   int i;
   NODE_PTR tnode;

   auto map=std::get<NODE_PTR>(node->exper_data);
   auto image=std::get<Img*>(map->exper_data);

   if (subst == nullptr)
      return 0;
   else if ((map->exper_type != IMAGE) ||
            (eval_node(subst, node->left, &u, V0, &tnode) != 2))
      return 0;

   /* Determine the magnitude of the bump map */
   if (node->right != nullptr) {
      i = eval_node(subst, node->right, &u, V1, &tnode);
      if (i == 1)
         MakeVector(u, u, u, bump_scale)
      else if (i == 2)
         VecCopy(V1, bump_scale)
      else
         MakeVector(1, 1, 1, bump_scale)
      }
   else
      MakeVector(1, 1, 1, bump_scale)

   calculate_uv(map_type, V0, &u, &v);
   deltau = 1.0 / (Flt)image->width;
   deltav = (image->scanline_order & 0x20 ? -1.0 : 1.0) / (Flt)image->length;

   if (lookup_height(image, u, v, 1, &height0) == 0)
      return 0;
   if (lookup_height(image, u+deltau, v, 1, &height1) == 0)
      return 0;
   if (lookup_height(image, u, v+deltav, 1, &height2) == 0)
      return 0;

#if 0
   height0 = bump_scale[0] * (height0 + 128.0) / 255.0;
   height1 = bump_scale[1] * (height1 + 128.0) / 255.0;
   height2 = bump_scale[2] * (height2 + 128.0) / 255.0;
#else
   height0 = (height0 + 128.0) / 255.0;
   height1 = (height1 + 128.0) / 255.0;
   height2 = (height2 + 128.0) / 255.0;
#endif

   /* Find the relative amounts of deflected normal */
   VecCopy(subst->N, N);
   MakeVector(1, height1-height0, 0, A);
   MakeVector(0, height2-height0, 1, B);
   VecCross(A, B, N1);
   VecNormalize(N1);

   /* Calculate two vectors that are orthogonal to the original normal,
      making the basic assumption that <0, 1, 0> is where the normal
      is starting from.  */
   VecCopy(N, B);
   MakeVector(0, 1, 0, N2);
   VecCross(B, N2, A);
   len = sqrt(VecDot(A, A));
   if (fabs(len) < PLY_EPSILON) {
      /* The original normal is exactly along <0, 1, 0>.  We need to choose
         a different set of orthogonal vectors */
      MakeVector(1, 0, 0, A);
      if (fabs(N[1] - 1.0) < PLY_EPSILON)
         MakeVector(0, 1, 0, B)
      else
         MakeVector(0,-1, 0, B)
      }
   else {
      len = 1.0 / len;
      VecScale(len, A);
      }

   /* Now calculate the resulting bumped normal */
   VecCross(A, B, C);
   VecNormalize(C);
#if 0
   VecScale(N1[0], A);
   VecScale(N1[1], B);
   VecScale(N1[2], C);
#else
   VecScale(bump_scale[0] * N1[0], A);
   VecScale(bump_scale[1] * N1[1], B);
   VecScale(bump_scale[2] * N1[2], C);

#endif
   VecAdd(A, B, vval);
   VecAdd(vval, C, vval);
   VecNormalize(vval);

   return 2;
}

/** @brief Evaluate an environment (cube) map lookup.
 *  @param subst  Surface substitution data.
 *  @param node   Parse-tree node; left: direction vector, right: ENVIRONMENT node.
 *  @param fval   Output: scalar (alpha).
 *  @param vval   Output: colour from the cube-map face.
 *  @return       2 on success, 0 on error.
 */
static int eval_environment_map(SUBST_PTR subst, NODE_PTR node,
                     Flt *fval, Vec vval)
{
   Flt x, y, z, ax, ay, az;
   Flt u, v, r, len;
   int i;
   Vec D;
   NODE_PTR tnode, map = node->right;

   if (subst == nullptr)
      return 0;
   else if ((map->exper_type == ENVIRONMENT) &&
            (eval_node(subst, node->left, &len, D, &tnode) == 2)) {
      if ((len = VecNormalize(D)) < PLY_EPSILON) {
         /* Bad point, right at the environment's center */
         MakeVector(1.0, 1.0, 1.0, vval);
         return 2;
         }
      x = D[0];
      y = D[1];
      z = D[2];

      /* Determine which face of the environment map, and how to access the
         corresponding image */
      ax = ABS(x); ay = ABS(y); az = ABS(z);
      if (ax > ay && ax > az)
         if (x > 0) {
            /* Right */
            u = -z;
            v = y;
            r = 1.0 / x;
            i = 0;
            }
         else {
            /* Left */
            u = z;
            v = y;
            r = -1.0 / x;
            i = 1;
            }
      else if (ay > az)
         if (y > 0) {
            /* Top */
            u = x;
            v = -z;
            r = 1.0 / y;
            i = 2;
            }
         else {
            /* Bottom */
            u = x;
            v = z;
            r = -1.0 / y;
            i = 3;
            }
      else if (z > 0) {
         /* Back */
         u = x;
         v = y;
         r = 1.0 / z;
         i = 4;
         }
      else {
         /* Front */
         u = -x;
         v = y;
         r = -1.0 / z;
         i = 5;
         }

      /* Scale back into the image space */
      u = 0.5 * (r * u + 1);
      v = 0.5 * (r * v + 1);
      auto images=std::get<Img**>(map->exper_data);
      (void)lookup_image_color(images[i], u, v, 0, fval, vval);
      return 2;
      }
   else {
      serror("Bad environment node\n");
      return 0;
      }
}

/** @brief Testable variant of eval_environment_map with hard-coded direction.
 *  @param subst  Surface substitution data.
 *  @param node   Parse-tree node (right child used for ENVIRONMENT type check).
 *  @param fval   Output: synthetic scalar value.
 *  @param vval   Output: synthetic colour vector.
 *  @return       2 on success, 0 on error.
 */
#ifndef TESTING
static
#endif
int eval_environment_mapmock(SUBST_PTR subst, NODE_PTR node, Flt* fval, Vec vval)
{
    Flt x, y, z, ax, ay, az;
    Flt u, v, r, len;
    int i;
    Vec D;
    NODE_PTR tnode, map = node->right;

    if (subst == nullptr)
        return 0;
    else if ((map->exper_type == ENVIRONMENT) ) {
        auto vec = std::get<NuVec>(node->exper_data);
        VecCopy(vec, D);
        
        if ((len = VecNormalize(D)) < PLY_EPSILON) {
            /* Bad point, right at the environment's center */
            MakeVector(1.0, 1.0, 1.0, vval);
            return 2;
        }
        x = D[0];
        y = D[1];
        z = D[2];

        /* Determine which face of the environment map, and how to access the
           corresponding image */
        ax = ABS(x); ay = ABS(y); az = ABS(z);
        if (ax > ay && ax > az)
            if (x > 0) {
                /* Right */
                u = -z;
                v = y;
                r = 1.0 / x;
                i = 0;
            }
            else {
                /* Left */
                u = z;
                v = y;
                r = -1.0 / x;
                i = 1;
            }
        else if (ay > az)
            if (y > 0) {
                /* Top */
                u = x;
                v = -z;
                r = 1.0 / y;
                i = 2;
            }
            else {
                /* Bottom */
                u = x;
                v = z;
                r = -1.0 / y;
                i = 3;
            }
        else if (z > 0) {
            /* Back */
            u = x;
            v = y;
            r = 1.0 / z;
            i = 4;
        }
        else {
            /* Front */
            u = -x;
            v = y;
            r = -1.0 / z;
            i = 5;
        }

        /* Scale back into the image space */
        u = 0.5 * (r * u + 1);
        v = 0.5 * (r * v + 1);
        MakeVector((Flt)r / 255.0, (Flt)u / 255.0, (Flt)v / 255.0, vval);
        *fval = 1.0 - (Flt)r / 255.0;

        return 2;
    }
    else {
        serror("Bad environment node\n");
        return 0;
    }
}

/** @brief Evaluate a PLUS_EXPER node (float+float or vector+vector).
 *  @param subst  Surface substitution data.
 *  @param node   Parse-tree node with left and right operands.
 *  @param fval   Output: scalar sum (float case).
 *  @param vval   Output: vector sum (vector case).
 *  @return       1 for float result, 2 for vector result, 0 on type mismatch.
 */
/* Simplify additive terms. */
static int eval_plus(SUBST_PTR subst, NODE_PTR node, Flt* fval, Vec vval)
{
    Flt fleft, fright;
    Vec vleft, vright;
    NODE_PTR tnode;

    switch (eval_node(subst, node->left, &fleft, vleft, &tnode)) {
    case 1:
        switch (eval_node(subst, node->right, &fright, vright,
            &tnode)) {
        case 1:
            *fval = fleft + fright;
            return 1;
        case 2:
            /* Tried to add a float to a vector */
            return 0;
        default:
            /* Invalid result */
            return 0;
        }
    case 2:
        switch (eval_node(subst, node->right, &fright, vright,
            &tnode)) {
        case 1:
            /* Tried to add a float to a vector */
            return 0;
        case 2:
            *fval = fleft + fright;
            VecAdd(vleft, vright, vval);
            return 2;
        default:
            /* Invalid result */
            return 0;
        }
    default:
        return 0;
    }
}


/** @brief Testable variant of eval_plus using inline node data.
 *  @param subst  Surface substitution data.
 *  @param node   Parse-tree node.
 *  @param fval   Output: scalar sum.
 *  @param vval   Output: vector sum.
 *  @return       1 always.
 */
/* Simplify additive terms. */
#ifndef TESTING
static
#endif
int eval_plusmock(SUBST_PTR subst, NODE_PTR node, Flt* fval, Vec vval)
{
   Flt fleft, fright;
   Vec vleft, vright;
   NODE_PTR tnode;

   auto vec = std::get<NuVec>(node->exper_data);

   auto vec2 = std::get<NuVec>(node->left->exper_data);
   
   *fval = vec2[0] + vec[0];
   //*fval = fleft + fright;
   VecAdd(vec, vec2, vval);
   return 1;

}


/** @brief Evaluate a MINUS_EXPER node (float-float or vector-vector).
 *  @param subst  Surface substitution data.
 *  @param node   Parse-tree node with left and right operands.
 *  @param fval   Output: scalar difference.
 *  @param vval   Output: vector difference.
 *  @return       1 for float, 2 for vector, 0 on type mismatch.
 */
static int
eval_minus(SUBST_PTR subst, NODE_PTR node, Flt *fval, Vec vval)
{
   Flt fleft, fright;
   Vec vleft, vright;
   NODE_PTR tnode;

   switch (eval_node(subst, node->left, &fleft, vleft, &tnode)) {
      case 1:
      switch (eval_node(subst, node->right, &fright, vright,
                        &tnode)) {
         case 1:
            *fval = fleft - fright;
            return 1;
         case 2:
            /* Tried to add a float to a vector */
            return 0;
         default:
            /* Invalid result */
            return 0;
         }
      case 2:
      switch (eval_node(subst, node->right, &fright, vright,
                        &tnode)) {
         case 1:
            /* Tried to add a float to a vector */
            return 0;
         case 2:
            *fval = fleft - fright;
            VecSub(vleft, vright, vval);
            return 2;
         default:
            /* Invalid result */
            return 0;
         }
      default:
      return 0;
      }
}

/** @brief Testable variant of eval_minus using inline node data.
 *  @param subst  Surface substitution data.
 *  @param node   Parse-tree node.
 *  @param fval   Output: scalar difference.
 *  @param vval   Output: vector difference.
 *  @return       1 always.
 */
#ifndef TESTING
static
#endif
int eval_minusmock(SUBST_PTR subst, NODE_PTR node, Flt* fval, Vec vval)
{
    Flt fleft, fright;
    Vec vleft, vright;
    NODE_PTR tnode;

    auto vec = std::get<NuVec>(node->exper_data);

    auto vec2 = std::get<NuVec>(node->left->exper_data);

    *fval = vec2[0] - vec[0];
    //*fval = fleft + fright;
    VecSub(vec, vec2, vval);
    return 1;

}

/** @brief Evaluate a TIMES_EXPER node (float*float, float*vector, or vector cross-product).
 *  @param subst  Surface substitution data.
 *  @param node   Parse-tree node with left and right operands.
 *  @param fval   Output: scalar product (float*float case).
 *  @param vval   Output: scaled or cross-product vector.
 *  @return       1 for float, 2 for vector, 0 on type mismatch.
 */
static int eval_times(SUBST_PTR subst, NODE_PTR node, Flt *fval, Vec vval)
{
   Flt fleft, fright;
   Vec vleft, vright;
   NODE_PTR tnode;

   switch (eval_node(subst, node->left, &fleft, vleft, &tnode)) {
      case 1:
      switch (eval_node(subst, node->right, &fright, vright, &tnode)) {
         case 1:
            *fval = fleft * fright;
            return 1;
         case 2:
            VecCopy(vright, vval);
            VecScale(fleft, vval);
            return 2;
         default:
            /* Invalid result */
            return 0;
         }
      case 2:
      switch (eval_node(subst, node->right, &fright, vright, &tnode)) {
         case 1:
            VecCopy(vleft, vval);
            VecScale(fright, vval);
            return 2;
         case 2:
            VecCross(vleft, vright, vval);
            return 2;
         default:
            /* Invalid result */
            return 0;
         }
      default:
      return 0;
      }
}

/** @brief Testable variant of eval_times using inline node data.
 *  @param subst  Surface substitution data.
 *  @param node   Parse-tree node.
 *  @param fval   Output: scalar product.
 *  @param vval   Output: cross-product vector.
 *  @return       2 always.
 */
#ifndef TESTING
static
#endif
int eval_timesmock(SUBST_PTR subst, NODE_PTR node, Flt* fval, Vec vval)
{
    Vec vleft, vright;

    auto vec = std::get<NuVec>(node->exper_data);

    auto vec2 = std::get<NuVec>(node->left->exper_data);

    *fval = vec2[0] * vec[0];
    //*fval = fleft + fright;
    VecCross(vec, vec2, vval);
    
    return 2;

}


/** @brief Evaluate a DIV_EXPER node (float/float or vector/float).
 *  @param subst  Surface substitution data.
 *  @param node   Parse-tree node with left (dividend) and right (divisor).
 *  @param fval   Output: scalar quotient.
 *  @param vval   Output: scaled vector (vector/float case).
 *  @return       1 for float, 2 for vector, 0 on error or division by zero.
 */
static int eval_div(SUBST_PTR subst, NODE_PTR node, Flt *fval, Vec vval)
{
   Flt fleft, fright;
   Vec vleft, vright;
   NODE_PTR tnode;

   switch (eval_node(subst, node->left, &fleft, vleft, &tnode)) {
      case 1:
      switch (eval_node(subst, node->right, &fright, vright, &tnode)) {
         case 1:
            *fval = fleft / fright;
            return 1;
         default:
            // Invalid result 
            return 0;
         }
      case 2:
      switch (eval_node(subst, node->right, &fright, vright, &tnode)) {
         case 1:
            if (fright != 0.0) {
               VecCopy(vleft, vval);
               VecScale(1.0/fright, vval);
               return 2;
               }
            else {
               smessage("Division by 0 in eval_div\n");
               return 0;
               }
         case 2:
            // Vector/vector division is undefined in Polyray expressions. 
            return 0;
         default:
            // Invalid result 
            return 0;
         }
      default:
      return 0;
      }
}

/** @brief Testable variant of eval_div using inline node data.
 *  @param subst  Surface substitution data.
 *  @param node   Parse-tree node.
 *  @param fval   Output: scalar quotient.
 *  @param vval   Output: result vector.
 *  @return       2 on success, 0 if divisor is zero.
 */
#ifndef TESTING
static
#endif
int eval_divmock(SUBST_PTR subst, NODE_PTR node, Flt* fval, Vec vval)
{
    Flt f1, f2;
    Vec vleft, vright;
    NODE_PTR tnode;

    auto vec = std::get<NuVec>(node->exper_data);

    auto vec2 = std::get<NuVec>(node->left->exper_data);
    MakeVector(vec[0] * vec2[0], vec[1] * vec2[1],
        vec[2] * vec2[2], vval);

    f1 = vec2[0]; f2 = vec[0];
    if (f2 == 0.0) return 0;
    *fval = f1 / f2;

    VecCross(vec, vec2, vval);

    if (f2 != 0.0)
        *fval = f1 / f2;
    else
        *fval = 0.0;


    return 2;

}

/** @brief Sign-preserving power function - handles negative bases and integer exponents.
 *  @param in     Base value.
 *  @param power  Exponent.
 *  @return       in^power with appropriate sign handling.
 */
#ifndef TESTING
static
#endif
Flt xpow(Flt in, Flt power)
{
   Flt temp, result;
   int even, test;

   if (in == 0.0)
      result = 0.0;
   else if (power == 0.0)
      result = 1.0;
   else if (power == 1.0)
      result = in;
   else if (power < 0.0)
      if (in == 0.0)
         result = 0.0;
      else if (in < 0.0)
         result = -pow(ABS(in), power);
      else
         result = pow(in, power);
   else {
      test = (int)power;
      temp = (Flt)test;
      if (temp == power) {
         /* Integer power - can deal with these */
         even = !(test%2);
         if (even)
            result = pow(ABS(in), power);
         else
            result = -pow(ABS(in), power);
         }
      else {
         if (in < 0.0)
            result = -pow(ABS(in), power);
         else
            result = pow(in, power);
         }
      }
   return result;
}

/** @brief Evaluate a POWER_EXPER node (float^float or component-wise vector product).
 *  @param subst  Surface substitution data.
 *  @param node   Parse-tree node; left: base, right: exponent.
 *  @param fval   Output: scalar power result.
 *  @param vval   Output: component-wise product (vector^vector case).
 *  @return       1 for float, 2 for vector, 0 on error.
 */
static int eval_power(SUBST_PTR subst, NODE_PTR node, Flt *fval, Vec vval)
{
   int i;
   Flt fleft, fright;
   Vec vleft, vright;
   NODE_PTR tnode;

   if ((i = eval_node(subst, node->left, &fleft, vleft, &tnode)) == 1) {
      if (eval_node(subst, node->right, &fright, vright, &tnode) == 1) {
         *fval = pow(fleft, fright);
         return 1;
         }
      else
         return 0;
      }
   else if (i == 2 &&
            eval_node(subst, node->right, &fright, vright, &tnode) == 2) {
      /* Not really a power, it's an external product -
         <x0,y0,z0> ^ <x1,y1,z1> = <x0*x1,y0*y1,z0*z1> */
      vval[0] = vleft[0] * vright[0];
      vval[1] = vleft[1] * vright[1];
      vval[2] = vleft[2] * vright[2];
      return 2;
      }
   else
      return 0;
}

//UNIT_STATIC(int eval_powermock(SUBST_PTR subst, NODE_PTR node, Flt* fval, Vec vval);)
/** @brief Testable variant of eval_power using inline node data.
 *  @param subst  Surface substitution data.
 *  @param node   Parse-tree node.
 *  @param fval   Output: scalar power.
 *  @param vval   Output: component-wise product vector.
 *  @return       1 always.
 */
#ifndef TESTING
static
#endif
int eval_powermock(SUBST_PTR subst, NODE_PTR node, Flt* fval, Vec vval)
{
    Flt fleft, fright;
    Vec vleft, vright;
    NODE_PTR tnode;

    auto vec = std::get<NuVec>(node->exper_data);

    auto vec2 = std::get<NuVec>(node->left->exper_data);

    *fval = pow(vec[0],vec2[0]);
    /* Not really a power, it's an external product -
         <x0,y0,z0> ^ <x1,y1,z1> = <x0*x1,y0*y1,z0*z1> */
    vval[0] = vec[0] * vec2[0];
    vval[1] = vec[1] * vec2[1];
    vval[2] = vec[2] * vec2[2];
    return 1;
}
/** @brief Look up and linearly interpolate a colour from a colour-map list.
 *  @param cmap           Linked list of colour-map entries.
 *  @param default_color  Fallback colour used when no entry matches.
 *  @param index          Scalar index value to look up.
 *  @param color          Output: interpolated RGB colour.
 *  @param alpha          Output: interpolated alpha/opacity value.
 *  @return               1 always (fallback used when no entry matches).
 */
int
eval_colormap(map_entries cmap, Vec default_color,
              Flt index, Vec color, Flt *alpha)
{
   map_entries temp;
   Flt inter0, inter1;

   temp = cmap;
   while (temp != nullptr) {
      if (index == temp->p0) {
         VecCopy(temp->v0, color);
         *alpha = temp->t0;
         return 1;
         }
      else if (index == temp->p1) {
         VecCopy(temp->v1, color);
         *alpha = temp->t1;
         return 1;
         }
      else if (index >= temp->p0 && index <= temp->p1) {
         /* Found the correct entry in the color map - do
            a linear interpolation of values for final color. */
         inter0 = (index - temp->p0) / (temp->p1 - temp->p0);
         inter1 = (1 - inter0);
         color[0] = inter0 * temp->v1[0] + inter1 * temp->v0[0];
         color[1] = inter0 * temp->v1[1] + inter1 * temp->v0[1];
         color[2] = inter0 * temp->v1[2] + inter1 * temp->v0[2];
         *alpha   = inter0 * temp->t1    + inter1 * temp->t0;
         return 1;
         }
      else
         temp = temp->next;
      }

   /* If we got here, then there was no appropriate entry in
      the color map.  Use the default if it exists, or black
      if no default */
   VecCopy(default_color, color);
   return 1;
}

/** @brief Inner implementation of colour-map evaluation (called by evaluateColourMap).
 *  @param subst          Surface substitution data.
 *  @param node           Parse-tree subscript node.
 *  @param cindex         Output: evaluated index scalar.
 *  @param vtmp           Scratch vector.
 *  @param tnode          Output: trailing node pointer.
 *  @param temp           Output: matched colour-map entry pointer.
 *  @param default_color  Output: default colour from the map node.
 *  @param ftmp           Scratch float.
 *  @param vval           Output: resulting colour vector.
 *  @param fval           Output: resulting alpha scalar.
 *  @param retFlag        Output: true when an early return value is produced.
 *  @return               2 on colour result, 0 on error, or unspecified when retFlag is false.
 */
static int doevaluateColourMap(SUBST_PTR subst, NODE_PTR node, Flt& cindex, Vec& vtmp,
    NODE_PTR* tnode, map_entries& temp, Vec& default_color,
    Flt& ftmp, NuVec& vval, Flt* fval, bool& retFlag)
{
    retFlag = true;
    if (eval_node(subst, node->right, &cindex, vtmp, tnode) == 1) {
        /* We have a good configuration, walk the list of values and
        find the resulting start and end vectors */
        temp = std::get<map_entries>(node->left->exper_data);
        /*MakeVector(0.0, 0.0, 0.0, default_color);*/
        MakeVectorZero1(default_color);
        Vec localvval;
        if (node->left->left != nullptr)
            if (eval_node(subst, node->left->left, &ftmp, localvval, tnode) == 2)
                VecCopy(localvval, default_color)
        
        auto cmap = std::get<map_entries>(node->left->exper_data);
        if (eval_colormap(cmap, default_color, cindex,
            localvval, fval)) {
            VecCopy(localvval, vval);
            return 2;
        }
        VecCopy(localvval, vval);
    }
    else
        return 0;
    retFlag = false;
    return {};
}

#ifdef TESTING
/** @brief Testable variant of doevaluateColourMap with no eval_node call on the index.
 *  @param subst          Surface substitution data.
 *  @param node           Parse-tree subscript node.
 *  @param cindex         Output: colour-map index.
 *  @param vtmp           Scratch vector.
 *  @param tnode          Output: trailing node pointer.
 *  @param temp           Output: matched colour-map entry pointer.
 *  @param default_color  Output: default colour from the map node.
 *  @param ftmp           Scratch float.
 *  @param vval           Output: resulting colour vector.
 *  @param fval           Output: resulting alpha scalar.
 *  @param retFlag        Output: true when an early return value is produced.
 *  @return               2 on colour result, 0 on error.
 */
int doevaluateColourMapmock(SUBST_PTR subst, NODE_PTR node, Flt& cindex, Vec& vtmp,
    NODE_PTR* tnode, map_entries& temp, Vec& default_color,
    Flt& ftmp, NuVec& vval, Flt* fval, bool& retFlag)
{
    retFlag = true;
        /* We have a good configuration, walk the list of values and
        find the resulting start and end vectors */
        temp = std::get<map_entries>(node->left->exper_data);
        /*MakeVector(0.0, 0.0, 0.0, default_color);*/
        MakeVectorZero1(default_color);
        Vec localvval;
        if (node->left->left != nullptr)
            if (eval_node(subst, node->left->left, &ftmp, localvval, tnode) == 2)
                VecCopy(localvval, default_color)

        auto cmap = std::get<map_entries>(node->left->exper_data);
        if (eval_colormap(cmap, default_color, cindex,
            localvval, fval)) {
            VecCopy(localvval, vval);
            return 2;
        }
        VecCopy(localvval, vval);

    retFlag = false;
    return {};
}
#endif

/** @brief Dispatch colour-map evaluation (real or test stub).
 *  @param subst          Surface substitution data.
 *  @param node           Parse-tree subscript node.
 *  @param cindex         Output: colour-map index.
 *  @param vtmp           Scratch vector.
 *  @param tnode          Output: trailing node pointer.
 *  @param temp           Output: matched entry pointer.
 *  @param default_color  Output: default colour.
 *  @param ftmp           Scratch float.
 *  @param vval           Output: colour vector.
 *  @param fval           Output: alpha scalar.
 *  @param retFlag        Output: early-return flag.
 *  @return               2 on colour result, 0 on error.
 */
static int evaluateColourMap(SUBST_PTR subst, NODE_PTR node, Flt& cindex, Vec& vtmp,
    NODE_PTR* tnode, map_entries& temp, Vec& default_color,
    Flt& ftmp, NuVec& vval, Flt* fval, bool& retFlag)
{
#ifdef TESTING
    NuVec localvval{ 255,0,0};
    VecCopy(localvval, vval);
    *fval = 1.0;
    return 2;
#else
    return doevaluateColourMap(subst, node, cindex, vtmp, tnode, temp, default_color, ftmp, vval, fval, retFlag);
#endif
}

/** @brief Inner implementation: walk an ARRAY node and evaluate the indexed element.
 *  @param subst   Surface substitution data.
 *  @param node    Parse-tree node; left: ARRAY, right: index expression.
 *  @param ftmp    Output: evaluated index as float.
 *  @param vtmp    Scratch vector.
 *  @param tnode   Output: trailing node pointer.
 *  @param list    Output: list pointer advanced to the selected element.
 *  @param fval    Output: scalar result.
 *  @param vval    Output: vector result.
 *  @return        Result type from eval_node, or 0 on index out of range.
 */
static int doevaluateArray(SUBST_PTR subst, NODE_PTR node, Flt& ftmp, Vec& vtmp, NODE_PTR* tnode, LIST_PTR& list, Flt* fval, Vec vval)
{
    int i = 0;
    if (eval_node(subst, node->right, &ftmp, vtmp, tnode) == 1) {
        for (list = std::get<LIST_PTR>(node->left->exper_data), i = ftmp;
            i > 0 && list != nullptr;
            i--, list = list->next);
        if (i == 0 && list != nullptr)
            return eval_node(subst, list->element, fval, vval,
                tnode);
        else {
            serror("Array index (%d) out of range\n", (int)ftmp);
            return 0;
        }
    }
    else
        return 0;
}

/** @brief Dispatch array evaluation (real or test stub).
 *  @param subst   Surface substitution data.
 *  @param node    Parse-tree node.
 *  @param ftmp    Output: index float.
 *  @param vtmp    Scratch vector.
 *  @param tnode   Output: trailing node pointer.
 *  @param list    Output: advanced list pointer.
 *  @param fval    Output: scalar result.
 *  @param vval    Output: vector result.
 *  @return        Result type, or 1 in test mode.
 */
static int evaluateArray(SUBST_PTR subst, NODE_PTR node, Flt& ftmp, Vec& vtmp, NODE_PTR* tnode, LIST_PTR& list, Flt* fval, Vec vval)
{
#ifdef TESTING
    NuVec localvval{ 255,0,0 };
    VecCopy(localvval, vval);
    *fval = 1.0;
    return 1;
#else
    return doevaluateArray(subst, node, ftmp, vtmp, tnode, list, fval, vval);
#endif
}

/** @brief Inner implementation: evaluate a SUBSCRIPT_EXPER node (vector component or array index).
 *  @param subst    Surface substitution data.
 *  @param node     Parse-tree node; left: base expression, right: index.
 *  @param tnode    Output: trailing node pointer.
 *  @param ftmp     Output: evaluated index as float.
 *  @param fval     Output: scalar result (component value).
 *  @param tarray   Output: array node pointer (ARRAY case).
 *  @param list     Output: advanced list pointer (ARRAY case).
 *  @param vval     Output: vector result.
 *  @param retFlag  Output: true when an early return value is produced.
 *  @return         1 for scalar, 2 for vector, 0 on error.
 */
static int doevaluateSubscriptExpression(SUBST_PTR subst, NODE_PTR node, NODE_PTR* tnode, Flt& ftmp, Flt* fval, NODE_PTR& tarray, LIST_PTR& list, Vec vval, bool& retFlag)
{
    retFlag = true;
    Flt fleft;
    Vec vleft;
    int i = eval_node(subst, node->left, &fleft, vleft, tnode);
    if (i == 1) {
        serror("Attempted to take a subscript of a float");
        return 0;
    }
    else if (i == 2) {
        Vec vtmp;
        if (eval_node(subst, node->right, &ftmp, vtmp, tnode) == 1) {
            i = (int)ftmp;
            if (i >= 0 && i < 3) {
                *fval = vleft[i];
                return 1;
            }
            else if (i == 3) {
                *fval = fleft;
                return 1;
            }
            else
                return 0;
        }
        else
            /* Bad subscript */
            return 0;
    }
    else if (i == 3 && (*tnode)->exper_type == ARRAY) {
        Vec vtmp;
        tarray = (*tnode);
        if (eval_node(subst, node->right, &ftmp, vtmp, tnode) == 1) {
            for (list = std::get<LIST_PTR>(tarray->exper_data), i = ftmp;
                i > 0 && list != nullptr;
                i--, list = list->next);
            if (i == 0 && list != nullptr)
                return eval_node(subst, list->element, fval, vval,
                    tnode);
            else {
                serror("Array index (%d) out of range\n", (int)ftmp);
                return 0;
            }
        }
        else
            return 0;
        /* serror("Non-integer subscript in array\n"); */
    }
    retFlag = false;
    return {};
}

/** @brief Dispatch subscript-expression evaluation (real or test stub).
 *  @param subst    Surface substitution data.
 *  @param node     Parse-tree node.
 *  @param tnode    Output: trailing node pointer.
 *  @param ftmp     Output: index float.
 *  @param fval     Output: scalar result.
 *  @param tarray   Output: array node.
 *  @param list     Output: advanced list pointer.
 *  @param vval     Output: vector result.
 *  @param retFlag  Output: early-return flag.
 *  @return         1 in test mode; real result otherwise.
 */
static int evaluateSubscriptExpression(SUBST_PTR subst, NODE_PTR node, NODE_PTR* tnode, Flt& ftmp, Flt* fval, NODE_PTR& tarray, LIST_PTR& list, Vec vval, bool& retFlag)
{
#ifdef TESTING
    NuVec localvval{ 255,0,0 };
    VecCopy(localvval, vval);
    *fval = 1.0;
    std::cout << "TESTING returning 1\n";
    retFlag = true;
    return 1;
#else
    return doevaluateSubscriptExpression(subst, node, tnode, ftmp, fval, tarray, list, vval, retFlag);
#endif
}

/** @brief Inner implementation: extract a single vector component via subscript.
 *  @param subst    Surface substitution data.
 *  @param node     Parse-tree node; left: vector, right: integer index (0-3).
 *  @param tnode    Output: trailing node pointer.
 *  @param fval     Output: selected component value.
 *  @param retFlag  Output: true when a result is produced.
 *  @return         1 on success, 0 on out-of-bounds.
 */
static int doevaluateSimpleSubscript(SUBST_PTR subst, NODE_PTR node, NODE_PTR* tnode, Flt* fval, bool& retFlag)
{
    retFlag = true;
    Flt fleft, fright;
    Vec vleft, vright;
    if (eval_node(subst, node->left, &fleft, vleft, tnode) == 2 &&
        eval_node(subst, node->right, &fright, vright, tnode) == 1) {
        int i = fright;
        if (i >= 0 && i < 3)
            *fval = vleft[i];
        else if (i == 3)
            *fval = fleft;
        else {
            serror("Subscript out of bounds in eval\n");
            return 0;
        }
        return 1;
    }
    retFlag = false;
    return {};
}

/** @brief Dispatch simple subscript evaluation (real or test stub).
 *  @param subst    Surface substitution data.
 *  @param node     Parse-tree node.
 *  @param tnode    Output: trailing node pointer.
 *  @param fval     Output: scalar component value.
 *  @param retFlag  Output: early-return flag.
 *  @return         1 always.
 */
static int evaluateSimpleSubscript(SUBST_PTR subst, NODE_PTR node, NODE_PTR* tnode, Flt* fval, bool& retFlag)
{
#ifdef TESTING
    *fval = 1.0;
    return 1;
#else
    return doevaluateSimpleSubscript(subst, node, tnode, fval, retFlag);
#endif
}

/** @brief Evaluate a SUBSCRIPT_EXPER node by dispatching to the appropriate helper.
 *  @param subst  Surface substitution data.
 *  @param node   Parse-tree node.
 *  @param fval   Output: scalar result.
 *  @param vval   Output: vector result.
 *  @param tnode  Output: trailing node pointer.
 *  @return       1 for scalar, 2 for vector, 0 on error.
 */
#ifndef TESTING
static
#endif
int eval_subscript(SUBST_PTR subst, NODE_PTR node, Flt* fval, Vec vval, NODE_PTR* tnode)
{
   map_entries temp;
   LIST_PTR list;
   NODE_PTR tarray;
   Flt cindex, ftmp;
   
  
   Vec default_color;

   if (node->left->exper_type == COLOR_MAP) {
       bool retFlag = false;
       NuVec nvval;
       Vec vtmp;
       int retVal = evaluateColourMap(subst, node, cindex, vtmp, tnode, temp, default_color, ftmp, nvval, fval, retFlag);
       VecCopy(nvval, vval)
       if (retFlag) return retVal;
        // Function name: evaluateColorMap
      }
   else if (node->left->exper_type == SUBSCRIPT_EXPER) {
       bool retFlag=false;
       int retVal = evaluateSubscriptExpression(subst, node, tnode, ftmp, fval, tarray, list, vval, retFlag);
       if (retFlag) return retVal;
      }
   else if (node->left->exper_type == ARRAY) {
       Vec vtmp;
       return evaluateArray(subst, node, ftmp, vtmp, tnode, list, fval, vval);
      }
   else {
       bool retFlag=false;
       int retVal = evaluateSimpleSubscript(subst, node, tnode, fval, retFlag);
       if (retFlag) return retVal;
   } //end last else

   return 0;
}


/** @brief Stub / placeholder for testable eval_subscript (not yet implemented).
 *  @param subst  Surface substitution data.
 *  @param node   Parse-tree node.
 *  @param fval   Output: scalar result (unused).
 *  @param vval   Output: vector result (unused).
 *  @param tnode  Output: trailing node pointer (unused).
 *  @return       0 always.
 */
#ifndef TESTING
static
#endif
int eval_subscriptmock(SUBST_PTR subst, NODE_PTR node, Flt* fval, Vec vval, NODE_PTR* tnode)
{
    //todo: later
    return 0;
}


/** @brief Test whether the line segment from @p start to @p end is unoccluded.
 *  @param start  Origin point of the visibility ray.
 *  @param end    Target point.
 *  @return       1 if the path is clear (or degenerate), 0 if occluded.
 */
int Check_Visibility(Vec start, Vec end)
{
   Ray ray;
   Vec D;
   Flt t;

   VecCopy(start, ray.P);
   VecSub(end, start, D);
   t = sqrt(VecDot(D, D));
   if (t < PLY_EPSILON) return 1;
   D[0] /= t; D[1] /= t; D[2] /= t;
   VecCopy(D, ray.D);
   auto SV = Shadow(nullptr, nullptr, &ray, SMALL, t, 0.0);
   if (SV.i)
      return 1;
   else
      return 0;
}

#ifdef TESTING
/** @brief Testable variant of Check_Visibility with all shade flags enabled.
 *  @param start  Origin point of the visibility ray.
 *  @param end    Target point.
 *  @return       1 if clear or degenerate, 0 if occluded.
 */
int Check_VisibilityMock(Vec start, Vec end)
{
    Ray ray;
    Vec D;
    Flt t;

    VecCopy(start, ray.P);
    VecSub(end, start, D);
    t = sqrt(VecDot(D, D));
    if (t < PLY_EPSILON) return 1;
    D[0] /= t; D[1] /= t; D[2] /= t;
    VecCopy(D, ray.D);
    runtimeState::scene.Global_Shade_Flag = ALL_SHADE_FLAGS;//see what happens
    auto SV = Shadow(nullptr, nullptr, &ray, SMALL, t, 0.0);
    if (SV.i)
    {
        return 1;
    }
    return 0;
}
#endif

//#define pi_3     1.0472
//#define pi_23    2.0944
/** @brief Map a (x, z) position on a colour wheel to an RGB vector.
 *  @param x    X coordinate (controls hue sector).
 *  @param z    Z coordinate (controls hue sector).
 *  @param vec  Output: RGB colour corresponding to the hue angle.
 */
#ifndef TESTING
static
#endif
NuVec color_wheel(Flt x, Flt z)
{
   NuVec vec;
   Flt zx_angle;

   if (ABS(z) < PLY_EPSILON)
      if (ABS(x) < PLY_EPSILON) {
         MakeVector(1.0, 0.0, 0.0, vec);
         }
      else if (x < 0.0) {
         MakeVector(0.0, 1.0, 1.0, vec);
         }
      else {
         MakeVector(1.0, 0.0, 0.0, vec);
         }
   else if (ABS(x) < PLY_EPSILON)
      if (z > 0) {
         MakeVector(0.5, 1.0, 0.0, vec);
         }
      else {
         MakeVector(0.5, 0.0, 1.0, vec);
         }
   else {
      zx_angle = acos(x / sqrt(x*x+z*z));
      if (z > 0.0)
         if (zx_angle < PYM_PI_3) {
            MakeVector(1.0, zx_angle / PYM_PI_3, 0.0, vec);
            }
         else if (zx_angle < pi_23) {
            MakeVector((pi_23 - zx_angle) / PYM_PI_3, 1.0, 0.0, vec);
            }
         else {
            MakeVector(0.0, 1.0, (zx_angle - pi_23)/PYM_PI_3, vec);
            }
      else if (zx_angle < PYM_PI_3) {
         MakeVector(1.0, 0.0, zx_angle / PYM_PI_3, vec);
         }
      else if (zx_angle < pi_23) {
         MakeVector((pi_23 - zx_angle) / PYM_PI_3, 0.0, 1.0, vec);
         }
      else {
         MakeVector(0.0, (zx_angle - pi_23) / PYM_PI_3, 1.0, vec);
         }
      }
   return vec;
}

/** @brief Recursively evaluate a parse-tree node given surface substitution data.
 *
 *  Return codes:
 *  - 0 - unable to evaluate
 *  - 1 - result is a scalar float, stored in @p fval
 *  - 2 - result is a vector, stored in @p vval
 *  - 3 - result is another expression node, stored in @p nval
 *
 *  @param subst  Surface substitution data (P, N, U, W, etc.); may be nullptr.
 *  @param node   Root of the expression sub-tree to evaluate.
 *  @param fval   Output: scalar result.
 *  @param vval   Output: vector result.
 *  @param nval   Output: expression-node result.
 *  @return       0, 1, 2, or 3 as described above.
 */
int eval_node(SUBST_PTR subst, NODE_PTR node, Flt *fval, Vec vval,
          NODE_PTR *nval)
{
   Flt fleft, fright, ftmp;
   Vec vleft, vright, tvec;
   int i, Flag = 0;
   unsigned long nr;
   Ray ray;

   if (node == nullptr) {
      serror("nullptr in eval_node\n");
      return 0;
   }

   *fval = 0.0;
   //std::cout <<"node type="<<node->exper_type<<"\n";
   switch(node->exper_type) {
   case UU_EXPER:
      if (subst == nullptr) return 0;
      *fval = subst->U[0];
      Flag = 1;
      break;
   case UV_EXPER:
      if (subst == nullptr) return 0;
      *fval = subst->U[1];
      Flag = 1;
      break;
   case UW_EXPER:
      if (subst == nullptr) return 0;
      *fval = subst->U[2];
      Flag = 1;
      break;
   case X_EXPER:
      if (subst == nullptr) return 0;
      *fval = subst->P[0];
      Flag = 1;
      break;
   case Y_EXPER:
      if (subst == nullptr) return 0;
      *fval = subst->P[1];
      Flag = 1;
      break;
   case Z_EXPER:
      if (subst == nullptr) return 0;
      *fval = subst->P[2];
      Flag = 1;
      break;
   case VAL_EXPER: 
   {
      auto value=std::get<Flt>(node->exper_data);
      *fval = value;
      Flag = 1;
      break;
   }
   case VEC_EXPER:
   {
      auto vec=std::get<NuVec>(node->exper_data);
      VecCopy(vec, vval);
      *fval = 0.0;
      Flag = 2;
      break;
   }
   case VECTOR_EXPER:
   {
      try {
        auto vec=std::get<vvarr>(node->exper_data);
      if (eval_node(subst, vec[0], &vval[0],
                    tvec, nval) == 1 &&
          eval_node(subst, vec[1], &vval[1],
                    tvec, nval) == 1 &&
          eval_node(subst, vec[2], &vval[2],
                    tvec, nval) == 1) {
         if (vec[3] != nullptr) {
            if (eval_node(subst, vec[3], fval,
                          tvec, nval) != 1)
               return 0;
         } else
            *fval = 0.0;
         Flag = 2;
         }
      else
         return 0;
      }
      catch (std::bad_variant_access&) 
      { std::cout << "our variant doesn't hold what its supposed to at this moment...\n"; }
      break;
   }
   case N_EXPER:
      if (subst == nullptr) return 0;
      vval[0] = subst->N[0];
      vval[1] = subst->N[1];
      vval[2] = subst->N[2];
      *fval   = 0.0;
      return 2;
   case P_EXPER:
      if (subst == nullptr) return 0;
      vval[0] = subst->P[0];
      vval[1] = subst->P[1];
      vval[2] = subst->P[2];
      *fval   = 0.0;
      return 2;
   case U_EXPER:
      if (subst == nullptr) return 0;
      vval[0] = subst->U[0];
      vval[1] = subst->U[1];
      vval[2] = subst->U[2];
      *fval   = 0.0;
      return 2;
   case W_EXPER:
      if (subst == nullptr) return 0;
      vval[0] = subst->W[0];
      vval[1] = subst->W[1];
      vval[2] = subst->W[2];
      *fval   = 0.0;
      return 2;
   case I_EXPER:
      if (subst == nullptr) return 0;
      vval[0] = subst->I[0];
      vval[1] = subst->I[1];
      vval[2] = subst->I[2];
      *fval   = 0.0;
      return 2;
   case START_FRAME:
      *fval = runtimeState::animator.start_frame;
      return 1;
   case END_FRAME:
      *fval = runtimeState::animator.end_frame;
      return 1;
   case TOTAL_FRAMES:
      *fval = runtimeState::animator.total_frames;
      return 1;
   case FRAME:
      *fval = runtimeState::animator.current_frame;
      return 1;
   case OPACITY:
      if (subst == nullptr)
         *fval = 1;
      else
         *fval = 1.0 - subst->U[2];
      return 1;
   case COLOR:
      if (subst == nullptr)
         MakeVector(1,1,1,vval)
      else
         VecCopy(subst->PT, vval)
      *fval = 1.0 - subst->U[2];
      return 2;
   case PLUS_EXPER:
      return eval_plus(subst, node, fval, vval);
   case MINUS_EXPER:
      return eval_minus(subst, node, fval, vval);
   case TIMES_EXPER:
      return eval_times(subst, node, fval, vval);
   case DIV_EXPER:
      return eval_div(subst, node, fval, vval);
   case DOT_EXPER:
      if (eval_node(subst, node->left, &fleft, vleft, nval) == 2 &&
          eval_node(subst, node->right, &fright, vright, nval) == 2) {
         *fval = VecDot(vleft, vright);
         return 1;
         }
      break;
   case POWER_EXPER:
      return eval_power(subst, node, fval, vval);
   case UMINUS_EXPER:
      i = eval_node(subst, node->left, fval, vval, nval);
      if (i == 1)
         *fval *= -1;
      else if (i == 2) {
        vval[0] = -vval[0];
        vval[1] = -vval[1];
        vval[2] = -vval[2];
      } else
         i = 0;
      return i;
   case SUBSCRIPT_EXPER:
      return eval_subscript(subst, node, fval, vval, nval);
   case ARRAY:
      *nval = node;
      Flag = 3;
      break;
   case STRING:
      *nval = node;
      Flag = 3;
      break;
   case ACOS:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, 
        vleft, nval) == 1) {
        *fval = acos(ftmp);
        return 1;
      }
   break;
   }
   case AND_EXPER:
   if (eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
       eval_node(subst, node->right, &fright, vright, nval) == 1) {
      *fval = (fleft != 0.0 && fright != 0.0 ? 1.0 : 0.0);
      return 1;
      }
   else
      return 0;
   break;
   case ASIN:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft, nval) == 1) {
        *fval = asin(ftmp);
        return 1;
      }
      break;
   }
   case ATAN:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft, nval) == 1) {
        *fval = atan(ftmp);
        return 1;
      }
      break;
   }
   case ATAN_TWO:
   if (eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
       eval_node(subst, node->right, &fright, vright, nval) == 1) {
      if (fleft == 0.0 && fright == 0.0)
         *fval = 0.0;
      else
         *fval = atan2(fleft, fright);
      return 1;
      }
   break;
   case BIAS:
   if (eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
       eval_node(subst, node->right, &fright, vright, nval) == 1) {
      if (fleft == 0.0 || fright == 0.0)
         *fval = 0.0;
      else {
         fleft = ((1.0 / fleft) - 2.0) * (1.0 - fright) + 1.0;
         if (fabs(fleft) < PLY_EPSILON)
            *fval = 1.0;
         else
            *fval = fright / fleft;
         }
      return 1;
      }
   break;
   case CEIL:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft, nval) == 1) {
        *fval = ceil(ftmp);
        return 1;
      }
      break;
   }
   case COLOR_WHEEL:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp,
                    vleft, nval) == 1 &&
          eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
          eval_node(subst, node->right, &fright, vright, nval) == 1) {
         NuVec Nuvval =color_wheel(ftmp, fright);         
         assignNuVecToFVec<Flt,3>(Nuvval, vval);
         *fval   = 0.0;
         Flag = 2;
         }
      break;
   }
   case CONDITIONAL_EXPER:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft, nval) == 1) {
         if (ftmp != 0.0)
            return eval_node(subst, node->left, fval, vval, nval);
         else
            return eval_node(subst, node->right, fval, vval, nval);
      }
      break;
   }
   case COS:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft, nval) == 1) {
         *fval = cos(ftmp);
         return 1;
      }
      break;
   }
   case COSH:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft, nval) == 1) {
         *fval = cosh(ftmp);
         return 1;
      }
      break;
   }
   case DNOISE:
      if (eval_node(subst, node->left, &ftmp, vleft, nval) == 2) {
         if (node->right == nullptr) {
            dKaos(vleft, vval, 2.0, 0.5, 1);
            return 2;
            }
         else if ((i = eval_node(subst, node->right, &ftmp,
                                 vright, nval)) == 1) {
            dKaos(vleft, vval, 2.0, 0.5, (int)ftmp);
            return 2;
            }
         else if (i == 2) {
            dKaos(vleft, vval, vright[0], vright[1], (int)vright[2]);
            return 2;
            }
         else
            return 0;
         }
      break;
   case EQUAL_EXPER:
   if ((i = eval_node(subst, node->left, &fleft, vleft, nval)) == 1)
      if (eval_node(subst, node->right, &fright, vright, nval) == 1) {
         *fval = (fleft == fright ? 1.0 : 0.0);
         return 1;
         }
      else
         return 0;
   else if (i == 2 && eval_node(subst, node->right, &fright,
                                vright, nval) == 2) {
      if (vleft[0] == vright[0] &&
          vleft[1] == vright[1] &&
          vleft[2] == vright[2])
         *fval = 1.0;
      else
         *fval = 0.0;
      return 1;
      }
   else
      return 0;
   break;
   case EXP:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft, nval) == 1) {
        *fval = exp(ftmp);
        return 1;
      }
      break;
   }
   case FABS:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      switch (eval_node(subst, param, &ftmp, vleft, nval)) {
        case 1:
           *fval = std::abs(ftmp);
           return 1;
        case 2:
           *fval = std::sqrt(VecDot(vleft, vleft));
           return 1;
        }
        break;
   }
   case FBM:
      return eval_brownian_motion(subst, node, vval);
   case FLOOR:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft, nval) == 1) {
        *fval = floor(ftmp);
        return 1;
      }
      break;
   }
   case FMOD:
   if (eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
       eval_node(subst, node->right, &fright, vright, nval) == 1) {
      if (fabs(fleft) < PLY_EPSILON)
         *fval = 0.0;
      else
         *fval = fmod(fleft, fright);
      return 1;
      }
   break;
   case FNOISE:
      if (eval_node(subst, node->left, &ftmp, vleft, nval) == 2) {
         if (node->right == nullptr) {
            *fval = fnoise(vleft, 2.0, 0.5, 10);
            return 1;
            }
         else if ((i = eval_node(subst, node->right, &ftmp,
                                 vright, nval)) == 1) {
            *fval = fnoise(vleft, 2.0, 0.5, (int)ftmp);
            return 1;
            }
         else if (i == 2) {
            *fval = fnoise(vleft, vright[0], vright[1], (int)vright[2]);
            return 1;
            }
         else
            return 0;
         }
      break;
   case GAIN:
   if (eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
       eval_node(subst, node->right, &fright, vright, nval) == 1) {
      if (fleft == 0.0 || fright == 0.0)
         *fval = 0.0;
      else {
         fleft = ((1.0 / fleft) - 2.0) * (1.0 - fright);
         if (fright < 0.5)
            if (fleft == -1.0)
               *fval = 1.0;
            else
               *fval = fright / (fleft + 1.0);
         else {
            fright = fleft - fright;
            fleft  = fleft - 1.0;
            if (fright == 0.0 || fleft == 0.0)
               *fval = 0.0;
            else
               *fval = fright / fleft;
            }
         }
      return 1;
      }
   break;
   case GREATER_EXPER:
   if (eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
       eval_node(subst, node->right, &fright, vright, nval) == 1) {
      *fval = (fleft > fright ? 1.0 : 0.0);
      return 1;
      }
   break;
   case GTEQ_EXPER:
   if (eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
       eval_node(subst, node->right, &fright, vright, nval) == 1) {
      *fval = (fleft >= fright ? 1.0 : 0.0);
      return 1;
      }
   break;
   case LEGENDRE:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp,
                    vleft, nval) == 1 &&
          eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
          eval_node(subst, node->right, &fright, vright, nval) == 1) {
         *fval = legendre((int)ftmp, (int)fleft, fright);
         return 1;
         }
      break;
   }
   case LESS_EXPER:
   if (eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
       eval_node(subst, node->right, &fright, vright, nval) == 1) {
      *fval = (fleft < fright ? 1.0 : 0.0);
      return 1;
      }
   break;
   case LN:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft, nval) == 1) {
        *fval = log(ftmp);
        return 1;
        }
      break;
   }
   case LOG:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft, nval) == 1) {
        *fval = log10(ftmp);
        return 1;
      }
      break;
   }
   case LTEQ_EXPER:
   if (eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
       eval_node(subst, node->right, &fright, vright, nval) == 1) {
      *fval = (fleft <= fright ? 1.0 : 0.0);
      return 1;
      }
   break;
   case MAXT:
   if (eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
       eval_node(subst, node->right, &fright, vright, nval) == 1) {
      *fval = MAX(fleft, fright);
      return 1;
      }
   break;
   case MINT:
   if (eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
       eval_node(subst, node->right, &fright, vright, nval) == 1) {
      *fval = PLY_MIN(fleft, fright);
      return 1;
      }
   break;
   case NOISE:
      if (eval_node(subst, node->left, &ftmp, vleft, nval) == 2) {
         if (node->right == nullptr) {
            *fval = Kaos(vleft, 2.0, 0.5, 1);
            return 1;
            }
         else if ((i = eval_node(subst, node->right, &ftmp,
                                 vright, nval)) == 1) {
            *fval = Kaos(vleft, 2.0, 0.5, (int)ftmp);
            return 1;
            }
         else if (i == 2) {
            *fval = Kaos(vleft, vright[0], vright[1], (int)vright[2]);
            return 1;
            }
         else
            return 0;
         }
      break;
   case NOT_EXPER:
   if (eval_node(subst, node->left, &fleft, vleft, nval) == 1) {
      *fval = (fleft != 0.0 ? 0.0 : 1.0);
      return 1;
      }
   else
      return 0;
   break;
   case OR_EXPER:
   if (eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
       eval_node(subst, node->right, &fright, vright, nval) == 1) {
      *fval = (fleft != 0.0 || fright != 0.0 ? 1.0 : 0.0);
      return 1;
      }
   else
      return 0;
   break;
   case RAMP:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft, nval) == 1) {
        *fval = ramp(ftmp);
        return 1;
      }
      break;
   }
   case RANDOM:
      *fval = random_number;
      return 1;
   case REFLECT:
      if (eval_node(subst, node->left, &ftmp, vleft, nval) == 2 &&
          eval_node(subst, node->right, &ftmp, vright, nval) == 2) {
         VecNormalize(vleft);
         VecNegate(vleft);
         VecNormalize(vright);
         if (VecDot(vleft, vright) >= 0.0)
            VecNegate(vright)
         SpecularDirection(vleft, vright, vval);
         *fval = 0.0;
         return 2;
         }
      else
         return 0;
      break;
   case RIPPLE:
      return eval_ripple(subst, node, vval);
   case ROTATE:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft,
                    nval) == 2 &&
          eval_node(subst, node->left, &ftmp, vright, nval) == 2) {
         Transform tx;
         if (node->right == nullptr) {
            /* Rotate a vector by a set of angles with respect to the
               coordinate axes */
            VecScale(PYM_PI/180.0, vright);
            Get_Rotation_Transformation(&tx, vright);
            TxVector(vval, vleft, &tx);
            *fval = 0.0;
            return 2;
            }
         else if (eval_node(subst, node->right, &ftmp, tvec, nval) == 1) {
            /* Rotate a vector about an axis */
            ftmp = ftmp * PYM_PI / 180.0;
            Get_Rotate_Transform(&tx, vright, ftmp);
            TxVector(vval, vleft, &tx);
            *fval = 0.0;
            return 2;
            }
         else
            return 0;
         }
      else
         return 0;
      break;
   }
   case SAWTOOTH:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft, nval) == 1) {
        *fval = sawtooth(ftmp);
        return 1;
      }
      break;
   }
   case SIN:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft, nval) == 1) {
        *fval = sin(ftmp);
        return 1;
      }
      break;
   }
   case SINH:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft, nval) == 1) {
        *fval = sinh(ftmp);
        return 1;
      }
      break;
   }
   case SPLINE:
      return eval_spline(subst, node, vval);
   case SQRT:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft, nval) == 1) {
        *fval = sqrt(ftmp);
        return 1;
        }
      break;
   }
   case TAN:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft, nval) == 1) {
        *fval = tan(ftmp);
        return 1;
      }
      break;
   }
   case TANH:
   {
      auto param=std::get<NODE_PTR>(node->exper_data);
      if (eval_node(subst, param, &ftmp, vleft, nval) == 1) {
        *fval = tanh(ftmp);
        return 1;
      }
      break;
   }
   case TRACE:
      {
      if (node->left == nullptr) {
         if (subst == nullptr)
            return 0;
         else
            VecCopy(subst->W, ray.P)
         if (!(eval_node(subst, node->right, &ftmp, ray.D, nval) == 2)) //todo:fix this and the eval_nodes below
            return 0;
         }
      else if (!(eval_node(subst, node->left, &ftmp, ray.P, nval) == 2) ||
               !(eval_node(subst, node->right, &ftmp, ray.D, nval) == 2))
         return 0;
      nr = 0;

      NuVec localvval{ vval[0], vval[1], vval[2] };
      Trace(nullptr, 1, 1.0, &ray, localvval, fval, 1.0, &nr);
      assignNuVecToFVec(localvval, vval);
      //TraceTest(nullptr, 1, 1.0, &ray, vval, fval, 1.0, &nr);
      return 2;
      }
   case VISIBLE:
   if (eval_node(subst, node->left, &ftmp, vleft, nval) == 2 &&
       eval_node(subst, node->right, &ftmp, vright, nval) == 2) {
      if (Check_Visibility(vleft, vright))
         *fval = 1.0;
      else
         *fval = 0.0;
      return 1;
      }
   break;
   case ENVIRONMENT_MAP:
      return eval_environment_map(subst, node, fval, vval);
   case PLANAR_IMAGEMAP:
   case SPHERICAL_IMAGEMAP:
   case CYLINDRICAL_IMAGEMAP:
   case HEIGHT_MAP:
   case INDEXED_MAP:
   case SPHERICAL_INDEXED:
   case CYLINDRICAL_INDEXED:
      return eval_imagemap(node->exper_type, subst, node, fval, vval);
   case PLANAR_BUMPMAP:
   case SPHERICAL_BUMPMAP:
   case CYLINDRICAL_BUMPMAP:
      return eval_bumpmap(node->exper_type, subst, node, vval);
   case TERM:
   /* May be doing a polynomial simplify - quietly fail. */
   return 0;
   break;
   default:
   return 0;
   }
   return Flag;
}

/** @brief Evaluate the formal partial derivative of a parse-tree node with respect to x.
 *
 *  Where a closed-form derivative is not known the function falls back to a
 *  numerical finite-difference approximation.
 *
 *  Return codes are the same as eval_node (0 = error, 1 = scalar, 2 = vector).
 *
 *  @param subst  Surface substitution data (must not be nullptr).
 *  @param node   Expression node to differentiate.
 *  @param fval   Output: scalar derivative d(node)/dx.
 *  @param vval   Output: vector derivative d(node)/dx.
 *  @return       1 for scalar derivative, 2 for vector derivative, 0 on error.
 */
int eval_node_dx(SUBST_PTR subst, NODE_PTR node, Flt *fval, Vec vval)
{
   struct subst_struct temp_subst, *sp;
   int i, j, k;
   Flt val1, val2, val3, val4;
   Vec vtmp1, vtmp2;
   NODE_PTR tnode;

   switch (node->exper_type) {
   case VAL_EXPER:
      *fval = 0.0;
      break;
   case UU_EXPER:
      *fval = subst->UT[0];
      break;
   case UV_EXPER:
      *fval = subst->UT[1];
      break;
   case UW_EXPER:
      *fval = subst->UT[2];
      break;
   case P_EXPER:
   case W_EXPER:
      VecCopy(subst->PT, vval)
      return 2;
   case X_EXPER:
      *fval = subst->PT[0];
      break;
   case Y_EXPER:
      *fval = subst->PT[1];
      break;
   case Z_EXPER:
      *fval = subst->PT[2];
      break;
   case VEC_EXPER:
      MakeVector(0, 0, 0, vval);
      return 2;
   case VECTOR_EXPER:
   {  
      auto vec=std::get<vvarr>(node->exper_data);
      if ((eval_node_dx(subst, vec[0],
                        &vval[0], vtmp1) == 1) &&
          (eval_node_dx(subst, vec[1],
                        &vval[1], vtmp1) == 1) &&
          (eval_node_dx(subst, vec[2],
                        &vval[2], vtmp1) == 1))
         return 2;
      else
         return 0;
      break;
   }
   case PLUS_EXPER:
      /* d(u+v)/dx = du/dx + dv/dx */
      i = eval_node_dx(subst, node->left, &val1, vtmp1);
      j = eval_node_dx(subst, node->right, &val2, vtmp2);
      if (i == 1 && j == 1) {
         *fval = val1 + val2;
         return 1;
         }
      else if (i == 2 && j == 2) {
         VecAdd(vtmp1, vtmp2, vval);
         return 2;
         }
      else
         return 0;
      break;
   case MINUS_EXPER:
      /* d(u-v)/dx = du/dx - dv/dx */
      i = eval_node_dx(subst, node->left, &val1, vtmp1);
      j = eval_node_dx(subst, node->right, &val2, vtmp2);
      if (i == 1 && j == 1) {
         *fval = val1 - val2;
         return 1;
         }
      else if (i == 2 && j == 2) {
         VecSub(vtmp1, vtmp2, vval);
         return 2;
         }
      else
         return 0;
      break;
   case TIMES_EXPER:
      /* d(u*v)/dx = u*dv/dx + v*du/dx */
      i = eval_node(subst, node->left, &val1, vval, &tnode);
      j = eval_node_dx(subst, node->right, &val2, vtmp1);
      if (i == 1 && j == 1) {
         val1 = val1 * val2;

         i = eval_node(subst, node->right, &val2, vval, &tnode);
         j = eval_node_dx(subst, node->left, &val3, vtmp2);
         val2 = val2 * val3;

         if (i == 1 && j == 1) {
            *fval = val1 + val2;
            return 1;
            }
         else
            return 0;
         }
      else if (i == 1 && j == 2) {
         VecScale(val1, vtmp1);

         i = eval_node(subst, node->right, &val2, vval, &tnode);
         j = eval_node_dx(subst, node->left, &val3, vtmp2);

         if (i == 2 && j == 1) {
            VecScale(val3, vval);
            VecAdd(vval, vtmp1, vval);
            return 2;
            }
         else
            return 0;
         }
      else if (i == 2 && j == 1) {
         VecScale(val2, vval);
         VecCopy(vval, vtmp1);

         i = eval_node(subst, node->right, &val2, vval, &tnode);
         j = eval_node_dx(subst, node->left, &val3, vtmp2);

         if (i == 1 && j == 2) {
            VecScale(val2, vtmp2);
            VecAdd(vtmp1, vtmp2, vval);
            return 2;
            }
         else
            return 0;
         }
      break;
   case DIV_EXPER:
      /* d(u/v)/dx = (v*du/dx - u*dv/dx) / v^2 */
      eval_node(subst, node->right, &val1, vval, &tnode);
      eval_node_dx(subst, node->left, &val2, vtmp1);
      val2 = val1 * val2; /* v*du/dx */

      val1 = val1 * val1; /* v^2 */
      if (val1 < PLY_EPSILON) {
         *fval = 0.0;
         return 0;
         }

      eval_node(subst, node->left, &val3, vval, &tnode);
      eval_node_dx(subst, node->right, &val4, vtmp1);
      val3 = val3 * val4; /* u*dv/dx */

      *fval = (val2 - val3) / val1;
      break;
   case POWER_EXPER:
      /* d(u^v)/dx = v*u^(v-1)du/dx + ln(u)*u^v*dv/dx */
      eval_node(subst, node->left, &val1, vval, &tnode);
      eval_node(subst, node->right, &val2, vval, &tnode);
      eval_node_dx(subst, node->left, &val3, vtmp1);
      eval_node_dx(subst, node->right, &val4, vtmp1);
      if (val4 == 0.0)
         *fval = val2 * xpow(val1, val2-1.0) * val3;
      else
         *fval = val2 * xpow(val1, val2-1.0) * val3 +
                 log(ABS(val1)) * xpow(val1, val2) * val4;
      break;
   case ATAN:
   {
      /* d(atan(u))/dx = 1/(1+u^2) * du/dx */
      auto param=std::get<NODE_PTR>(node->exper_data);
      eval_node(subst, param, &val1, vval, &tnode);
      eval_node_dx(subst, param, &val2, vtmp1);
      *fval = val2 / (1.0 + val1 * val1);
      break;
   }
   case COS:
   {
      /* d(cos(u))/dx = -sin(u) * du/dx */
      auto param=std::get<NODE_PTR>(node->exper_data);
      eval_node(subst, param, &val1, vval, &tnode);
      eval_node_dx(subst, param, &val2, vtmp1);
      *fval = -sin(val1) * val2;
      break;
   }
   case EXP:
   {
      /* d(e^u)/dx = e^u * du/dx */
      auto param=std::get<NODE_PTR>(node->exper_data);
      eval_node(subst, param, &val1, vval, &tnode);
      eval_node_dx(subst, param, &val2, vtmp1);
      *fval = exp(val1) * val2;
      break;
   }
/*
   case FABS:
      if (eval_node(subst, node->exper_data.param, &val1, vval, &tnode) == 2)
         val1 = sqrt(VecDot(vval, vval));
      if (eval_node_dx(subst, node->exper_data.param, vval, vtmp1) == 2) {
         VecScale(val1, vval)
         return 2;
         }
      *fval = (val1 < 0 ? -1.0 : 1.0) * ABS(val2);
      break;
*/
   case LN:
   {
      /* d(ln(u))/dx = 1/u * du/dx */
      auto param=std::get<NODE_PTR>(node->exper_data);
      eval_node(subst, param, &val1, vval, &tnode);
      if (ABS(val1) < PLY_EPSILON) {
         *fval = 0.0;
         return 0;
         }
      eval_node_dx(subst, param, &val2, vtmp1);
      *fval = val2 / val1;
      break;
   }
   case LOG:
   {
      /* d(log10(u))/dx = log10(e) * 1/u * du/dx */
      auto param=std::get<NODE_PTR>(node->exper_data);
      eval_node(subst, param, &val1, vval, &tnode);
      if (ABS(val1) < PLY_EPSILON) {
         *fval = 0.0;
         return 0;
         }
      eval_node_dx(subst, param, &val2, vtmp1);
      //*fval = M_LOG10E * val2 / val1;
      *fval = std::numbers::log10e_v<Flt> *val2 / val1;
      break;
   }
   case SIN:
   {
      /* d(sin(u))/dx = cos(u) * du/dx */
      auto param=std::get<NODE_PTR>(node->exper_data);
      eval_node(subst, param, &val1, vval, &tnode);
      eval_node_dx(subst, param, &val2, vtmp1);
      *fval = cos(val1) * val2;
      break;
   }
   case SQRT:
   {
      /* d(sqrt(u))/dx = 0.5 * du/dx / sqrt(u) */
      auto param=std::get<NODE_PTR>(node->exper_data);
      eval_node(subst, param, &val1, vval, &tnode);
      if (val1 <= 0.0) {
         *fval = 0.0;
         return 0;
         }
      eval_node_dx(subst, param, &val2, vtmp1);
      *fval = 0.5 * val2 / sqrt(val1);
      break;
   }
   case TAN:
   {
      /* d(tan(u))/dx = 1/cos(u)^2 * du/dx */
      auto param=std::get<NODE_PTR>(node->exper_data);
      eval_node(subst, param, &val1, vval, &tnode);
      val1 = cos(val1);
      val1 = val1 * val1;
      if (val1 <= 0.0) {
         *fval = 0.0;
         return 0;
         }
      eval_node_dx(subst, param, &val2, vtmp1);
      *fval = val2 / val1;
      break;
   }
   default:
      /* Don't know how to take the formal derivative, so we will
         evaluate the function at two locations and find the slope */
      sp = &temp_subst;
      memcpy(sp, subst, sizeof(struct subst_struct));
      i = eval_node(sp, node, &val1, vtmp1, &tnode);
      for (k=0;k<3;k++) {
         temp_subst.P[k] += PLY_EPSILON * temp_subst.PT[k];
         temp_subst.U[k] += PLY_EPSILON * temp_subst.UT[k];
         }
      j = eval_node(sp, node, &val2, vtmp2, &tnode);
      if (i == 1 && j == 1)
         *fval = (val2 - val1) / PLY_EPSILON;
      else if (i == 2 && j == 2) {
         VecSub(vtmp2, vtmp1, vval);
         VecScale(1.0 / PLY_EPSILON, vval)
         return 2;
         }
   }
   return 1;
}

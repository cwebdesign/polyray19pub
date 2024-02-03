/* eval.c

   compress a parse tree using input values

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
#include "ytab.h"
#include "eval.h"
#include "io.h"
#include "vector.h"
#include "image.h"
#include "intersec.h"
#include "trace.h"
#include "symtab.h"
#include "builder.h"
#include "spline.h"

#ifndef M_LOG10E
#define M_LOG10E 0.434294481903251827651
#endif

/* Noise generator variables */
#define LARGE 100000.0
#define HASH_SIZE 65535
#define spline(x) ((3.0 - 2.0 * (x)) * (x) * (x))
static int mult_table_size = 5;
static unsigned long mult_table[] = { 99137, 113713, 55237, 994472, 441973 };

/* Ripple/Wave variables */
#define MAX_RIPPLE_CENTERS 5
static int ripple_init_flag = 0;
static Vec ripple_centers[MAX_RIPPLE_CENTERS];

/* Create a random number between 0 and 1 */
#define random_number ((Flt)rand() / (Flt)RAND_MAX)

static void
init_ripples()
{
   int i, j;

   srand(42);
   for (i=0;i<MAX_RIPPLE_CENTERS;i++) {
      MakeVector(random_number, random_number, random_number,
                 ripple_centers[i]);
      /* Spread them within the unit cube */
      for (j=0;j<3;j++)
         ripple_centers[i][j] = 2.0 * (ripple_centers[i][j] - 0.5);
      }
   ripple_init_flag = 1;
}

void
ripples(Vec P, Vec N, Flt freq, Flt phase, Flt scale)
{
   int i;
   Vec Pt;
   Flt len, val;

   if (!ripple_init_flag) init_ripples();

   for (i=0;i<MAX_RIPPLE_CENTERS;i++) {
      VecCopy(P, Pt);
      VecSub(Pt, ripple_centers[i], Pt);
      len = VecDot(Pt, Pt);
      if (len < EPSILON)
         len = 1.0;
      else
         len = sqrt(len);

      val = cos(len * freq + phase) * scale /
            (len * (Flt)MAX_RIPPLE_CENTERS);

      VecAddS(val, Pt, N, N);
      }
   VecNormalize(N);
}

static int
eval_ripple(SUBST_PTR subst, NODE_PTR node, Vec vval)
{
   int i;
   Flt len, val, freq, phase;
   Vec P, Center;
   Flt ftmp1;
   Vec vval1;
   NODE_PTR tnode1;

   if (eval_node(subst, node->exper_data.vec[0], &ftmp1, P, &tnode1) != 2) {
      if (subst == NULL)
         return 0;
      else {
         error("ripple coordinate must be vector");
         return 0;
      }
   }

   if (node->exper_data.vec[1] == NULL &&
       node->exper_data.vec[2] == NULL &&
       node->exper_data.vec[3]) {
      /* Evaluate ripple with just the position */
      MakeVector(0, 0, 0, Center)
      freq  = 1.0;
      phase = 0.0;
      }
   else {
      i = eval_node(subst, node->exper_data.vec[1], &ftmp1, Center, &tnode1);
      if (i != 2) {
         if (subst == NULL)
            return 0;
         else {
            error("ripple center must be vector");
            return 0;
         }
      }
      i = eval_node(subst, node->exper_data.vec[2], &freq, vval1, &tnode1);
      if (i != 1) {
         error("ripple frequency must be a floating point value");
         return 0;
      }
      i = eval_node(subst, node->exper_data.vec[3], &phase, vval1, &tnode1);
      if (i != 1) {
         error("ripple phase must be a floating point value");
         return 0;
      }
      }

   VecSub(P, Center, vval);
   len = VecDot(vval, vval);
   if (len < EPSILON)
      len = 1.0;
   else
      len = sqrt(len);
   val = cos(len * freq + phase) / len;
   VecScale(val, vval);

   return 2;
}

static Flt
hash3d(unsigned long x, unsigned long y, unsigned long z)
{
   int i;
   unsigned long K, Kt;
   Flt result;

   K = x | y | z;
   Kt = 0;
   for (i=0;i<mult_table_size;i++)
      Kt = ((Kt + K) * mult_table[i]) % HASH_SIZE;
      /* Kt = ((Kt + K) * mult_table[i]) & 0xffff; */ /* HASH_SIZE = 65535 */
   result = (Flt)Kt / (Flt)(HASH_SIZE - 1);
   return result;
}

static Flt
flt_noise(Vec P)
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
   t = Pt[0] - ix; sx0 = spline(t); sx1 = 1.0 - sx0;
   t = Pt[1] - iy; sy0 = spline(t); sy1 = 1.0 - sy0;
   t = Pt[2] - iz; sz0 = spline(t); sz1 = 1.0 - sz0;
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

#define Perlin_B 256
static int Perlin_p[Perlin_B + Perlin_B + 2];
static Vec Perlin_g[Perlin_B + Perlin_B + 2];
static int Perlin_Flag = 1;

#define Perlin_noise_setup(P, i,b0,b1,r0,r1)\
        t = P[i] + 10000.0231;\
        b0 = ((long)t) & (Perlin_B - 1);\
        b1 = (b0 + 1) & (Perlin_B - 1);\
        r0 = t - (long)t;\
        r1 = r0 - 1.0;

static void
Perlin_init(void)
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

static Flt
Perlin_noise(Vec P)
{
   int bx0, bx1, by0, by1, bz0, bz1;
   int b00, b10, b01, b11;
   Flt rx0, rx1, ry0, ry1, rz0, rz1;
   Flt *q, sx, sy, sz, a, b, c, d, t, u, v;
   int i, j;

   if (Perlin_Flag) {
      Perlin_Flag = 0;
      Perlin_init();
      }

   Perlin_noise_setup(P, 0, bx0, bx1, rx0, rx1);
   Perlin_noise_setup(P, 1, by0, by1, ry0, ry1);
   Perlin_noise_setup(P, 2, bz0, bz1, rz0, rz1);

   i = Perlin_p[bx0];
   j = Perlin_p[bx1];

   b00 = Perlin_p[i + by0];
   b10 = Perlin_p[j + by0];
   b01 = Perlin_p[i + by1];
   b11 = Perlin_p[j + by1];

#define at(rx,ry,rz) (rx * q[0] + ry * q[1] + rz * q[2])
#define s_curve(t) (t * t * (3.0 - (t + t)))
#define lerp(t, a, b) (a + t * (b - a))

   sx = s_curve(rx0);
   sy = s_curve(ry0);
   sz = s_curve(rz0);

   q = Perlin_g[b00 + bz0]; u = at(rx0, ry0, rz0);
   q = Perlin_g[b10 + bz0]; v = at(rx1, ry0, rz0);
   a = lerp(sx, u, v);

   q = Perlin_g[b01 + bz0]; u = at(rx0, ry1, rz0);
   q = Perlin_g[b11 + bz0]; v = at(rx1, ry1, rz0);
   b = lerp(sx, u, v);

   c = lerp(sy, a, b);

   q = Perlin_g[b00 + bz1]; u = at(rx0, ry0, rz1);
   q = Perlin_g[b10 + bz1]; v = at(rx1, ry0, rz1);
   a = lerp(sx, u, v);

   q = Perlin_g[b01 + bz1]; u = at(rx0, ry1, rz1);
   q = Perlin_g[b11 + bz1]; v = at(rx1, ry1, rz1);
   b = lerp(sx, u, v);

   d = lerp(sy, a, b);

   return (0.75 * lerp(sz, c, d) + 0.5);
}

Flt
fnoise(Vec P, Flt pos_scale, Flt noise_scale, int octaves)
{
   Vec Pt;
   Flt result = 0.0;
   Flt scale  = 1.0;
   Flt magnitude = 0.0;
   int i;

   if (octaves <= 0)
      return 0.0;
   VecCopy(P, Pt);
   for (i=0;i<octaves;i++) {
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

Flt
Kaos(Vec P, Flt pos_scale, Flt noise_scale, int octaves)
{
   Vec Pt;
   Flt result = 0.0;
   Flt scale  = 1.0;
   Flt magnitude = 0.0;
   int i;

   if (octaves <= 0)
      return 0.0;
   VecCopy(P, Pt);
   for (i=0;i<octaves;i++) {
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

void
dKaos(Vec P, Vec R, Flt pos_scale, Flt noise_scale, int octaves)
{
   Vec Pt;
   Flt scale, magnitude;
   int i, j;

   /* MakeVector(0.0, 0.0, 0.0, R); */
   MakeVectorZero(R);
   if (octaves <= 0)
      return;
   for (j=0;j<3;j++) {
      for (i=0;i<3;i++)
         Pt[i] = P[(i+j)%3];
      scale = noise_scale;
      magnitude = 0.0;
      R[j] = 0.0;
      for (i=0;i<octaves;i++) {
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


void
dnoise3d(Vec P, Vec R, Flt pos_scale, Flt noise_scale, int octaves)
{
   Vec Pt;
   Flt scale, magnitude;
   int i, j;

   /* MakeVector(0.0, 0.0, 0.0, R);*/
   MakeVectorZero(R);
   if (octaves <= 0)
      return;
   for (j=0;j<3;j++) {
      for (i=0;i<3;i++)
         Pt[i] = P[(i+j)%3];
      scale = noise_scale;
      magnitude = 0.0;
      R[j] = 0.0;
      for (i=0;i<octaves;i++) {
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

static void
brownian_motion(Vec start, int cycles, Vec scale, Vec end)
{
   /* Deflect a particle starting at "start" by random pushes
      of size "scale".  Repeat "cycles" times */
   int i, j;
   Flt rnd;

   VecCopy(start, end);
   for (i=0;i<cycles;i++)
      for (j=0;j<3;j++) {
         rnd = 2.0 * polyray_random() - 1.0;
         end[j] += rnd * scale[j];
         }
}

#define DEFAULT_CYCLES 1
static int
eval_brownian_motion(SUBST_PTR subst, NODE_PTR node, Vec vval)
{
   NODE_PTR tnode;
   Flt fscale;
   Vec start, vscale;
   int stype;

   if (eval_node(subst, node->left, &fscale, start, &tnode) == 2) {
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

      brownian_motion(start, DEFAULT_CYCLES, vscale, vval);
      return 2;
      }
   else
      return 0;
}

Flt
sawtooth(Flt x)
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

Flt
ramp(Flt x)
{
   Flt y;

   if (fabs(x) < EPSILON)
      return 0.0;

   y = fmod(x, 1.0);
   if (y < 0.0)
      y += 1.0;

   return y;
}

/* Legendre polynomial from "Numerical Recipies", Press, et. al. */
static Flt
legendre(int l, int m, Flt x)
{
   Flt fact, pll=0, pmm, pmmp1, somx2;
   int i, ll;

   if (m < 0 || m > l || fabs(x) > 1.0)
      return 0.0;
   pmm = 1.0;
   if (m > 0) {
      somx2 = sqrt((1.0 - x) * (1.0 + x));
      fact = 1.0;
      for (i=1;i<=m;i++) {
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
         for (ll = m + 2; ll <= l; ll++) {
            pll = (x * (2 * ll - 1) * pmmp1 - (ll + m - 1) * pmm) / (ll - m);
            pmm = pmmp1;
            pmmp1 = pll;
            }
         return pll;
         }
      }
}

/* Map a point (x, y, z) on a sphere of radius 1 to a 2-d image. (Or is it the
   other way around?) */
int
spherical_imagemap(Vec P, Flt *u, Flt *v)
{
   Flt len, phi, theta;
   Flt x, y, z;

   /* Make sure this vector is on the unit sphere. */
   len = VecLen(P);
   if (len < EPSILON) {
      warning("Bad point: <%g, %g, %g>, dist = %g in spherical map\n",
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
            theta = M_PI;
      else {
         theta = acos(x / len);
         if (z < 0.0) theta = 2.0 * M_PI - theta;
         }
      }
   *u = theta / (2.0 * M_PI);
   *v = 0.5 + (phi / M_PI);
   return 1;
}

/* Map a point (x, y, z) on a cylinder of radius 1, height 1, that has its
   axis of symmetry along the y-axis to the square [0,1]x[0,1]. */
static int
cylindrical_imagemap(Vec P, Flt *u, Flt * v)
{
   Flt x, len, theta;

   len = sqrt(P[0] * P[0] + P[2] * P[2]);
   /* Make sure this vector is on the unit cylinder. */
   if (len < EPSILON)
      theta = 0;
   else {
      x  = P[0] / len;
      if (P[2] == 0.0)
         if (x > 0)
            theta = 0.0;
         else
            theta = M_PI;
      else {
         theta = acos(x);
         if (P[2] < 0.0)
            theta = (2.0 * M_PI) - theta;
         }
      }
   *u = theta / (2.0 * M_PI);
   *v = P[1];
   return 1;
}

static void
calculate_uv(int map_type, Vec vleft, Flt *u, Flt *v)
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
      error("Unsupported image map type: %d\n", map_type);
      *u = 0;
      *v = 0;
   }
}

static int
eval_imagemap(int map_type, SUBST_PTR subst, NODE_PTR node,
              Flt *fval, Vec vval)
{
   Flt fleft;
   Flt u, v;
   Vec vleft;
   int rflag;
   NODE_PTR tnode, map = node->exper_data.param;

   if (subst == NULL)
      return 0;
   else if ((map->exper_type == IMAGE) &&
       (eval_node(subst, node->left, &fleft, vleft, &tnode) == 2)) {
      rflag = (node->right == NULL ? 0 : 1);
      calculate_uv(map_type, vleft, &u, &v);
      switch (map_type) {
      case PLANAR_IMAGEMAP:
      case SPHERICAL_IMAGEMAP:
      case CYLINDRICAL_IMAGEMAP:
         lookup_image_color(map->exper_data.image, u, v, rflag, fval, vval);
         return 2;
      case HEIGHT_MAP:
         lookup_height(map->exper_data.image, u, v, rflag, fval);
         return 1;
      case INDEXED_MAP:
      case SPHERICAL_INDEXED:
      case CYLINDRICAL_INDEXED:
         lookup_index(map->exper_data.image, u, v, rflag, fval);
         return 1;
      default:
         error("Unknown map type");
         return 0;
         }
      }
   else {
      error("Left node is %d, not an image node\n", node->left->exper_type);
      return 0;
      }
}

static int
eval_bumpmap(int map_type, SUBST_PTR subst, NODE_PTR node,
             Vec vval)
{
   Flt len, u, v, deltau, deltav;
   Flt height0, height1, height2;
   Vec N, N1, N2, A, B, C;
   Vec V0, V1, bump_scale;
   int i;
   NODE_PTR tnode, map;
   Img *image;

   map   = node->exper_data.param;
   image = map->exper_data.image;
   if (subst == NULL)
      return 0;
   else if ((map->exper_type != IMAGE) ||
            (eval_node(subst, node->left, &u, V0, &tnode) != 2))
      return 0;

   /* Determine the magnitude of the bump map */
   if (node->right != NULL) {
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
   deltav = (image->orien & 0x20 ? -1.0 : 1.0) / (Flt)image->length;

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
   if (fabs(len) < EPSILON) {
      /* The original normal is exactly along <0, 1, 0>.  We need to choose
         a different set of orthogonal vectors */
      MakeVector(1, 0, 0, A);
      if (fabs(N[1] - 1.0) < EPSILON)
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

static int
eval_environment_map(SUBST_PTR subst, NODE_PTR node,
                     Flt *fval, Vec vval)
{
   Flt x, y, z, ax, ay, az;
   Flt u, v, r, len;
   int i;
   Vec D;
   NODE_PTR tnode, map = node->right;

   if (subst == NULL)
      return 0;
   else if ((map->exper_type == ENVIRONMENT) &&
            (eval_node(subst, node->left, &len, D, &tnode) == 2)) {
      if ((len = VecNormalize(D)) < EPSILON) {
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

      (void)lookup_image_color(map->exper_data.images[i], u, v, 0, fval, vval);
      return 2;
      }
   else {
      error("Bad environment node\n");
      return 0;
      }
}

/* Simplify additive terms. */
static int
eval_plus(SUBST_PTR subst, NODE_PTR node, Flt *fval, Vec vval)
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

static int
eval_times(SUBST_PTR subst, NODE_PTR node, Flt *fval, Vec vval)
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

static int
eval_div(SUBST_PTR subst, NODE_PTR node, Flt *fval, Vec vval)
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
            /* Invalid result */
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
               message("Division by 0 in eval_div\n");
               return 0;
               }
         case 2:
            MakeVector(vleft[0] * vright[0], vleft[1] * vright[1],
                       vleft[2] * vright[2], vval);
            if (fright != 0.0)
               *fval = fleft / fright;
            else
               *fval = 0.0;
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

static Flt
xpow(Flt in, Flt power)
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

static int
eval_power(SUBST_PTR subst, NODE_PTR node, Flt *fval, Vec vval)
{
   int i;
   Flt fleft, fright;
   Vec vleft, vright;
   NODE_PTR tnode;

   if ((i = eval_node(subst, node->left, &fleft, vleft, &tnode)) == 1) {
      if (eval_node(subst, node->right, &fright, vright, &tnode) == 1) {
#if defined( __GNUC__ )
         /* The "pow" function is causing memory problems if it gets called
            with a negative number.  This has so far only happened with the
            GNU C compiler. */
         *fval = xpow(fleft, fright);
#else
         *fval = pow(fleft, fright);
#endif
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

int
eval_colormap(map_entries cmap, Vec default_color,
              Flt index, Vec color, Flt *alpha)
{
   map_entries temp;
   Flt inter0, inter1;

   temp = cmap;
   while (temp != NULL) {
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

static int
eval_subscript(SUBST_PTR subst, NODE_PTR node,
               Flt *fval, Vec vval, NODE_PTR *tnode)
{
   map_entries temp;
   LIST_PTR list;
   NODE_PTR tarray;
   int i;
   Flt cindex, ftmp;
   Vec vtmp;
   Flt fleft, fright;
   Vec vleft, vright;
   Vec default_color;

   if (node->left->exper_type == COLOR_MAP) {
      if (eval_node(subst, node->right, &cindex, vtmp, tnode) == 1) {
         /* We have a good configuration, walk the list of values and
            find the resulting start and end vectors */
         temp = node->left->exper_data.cmap;
         /*MakeVector(0.0, 0.0, 0.0, default_color);*/
         MakeVectorZero1(default_color);
         if (node->left->left != NULL)
            if (eval_node(subst, node->left->left, &ftmp, vval, tnode) == 2)
               VecCopy(vval, default_color)
         if (eval_colormap(node->left->exper_data.cmap, default_color, cindex,
                           vval, fval))
            return 2;
         }
      else
         return 0;
      }
   else if (node->left->exper_type == SUBSCRIPT_EXPER) {
      i = eval_node(subst, node->left, &fleft, vleft, tnode);
      if (i == 1) {
         error("Attempted to take a subscript of a float");
         return 0;
         }
      else if (i == 2) {
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
         tarray = (*tnode);
         if (eval_node(subst, node->right, &ftmp, vtmp, tnode) == 1) {
            for (list=tarray->exper_data.array,i=ftmp;
                 i > 0 && list != NULL;
                 i--, list = list->next) ;
            if (i == 0 && list != NULL)
               return eval_node(subst, list->element,  fval, vval,
                                tnode);
            else {
               error("Array index (%d) out of range\n", (int)ftmp);
               return 0;
               }
            }
         else
            return 0;
            /* error("Non-integer subscript in array\n"); */
         }
      }
   else if (node->left->exper_type == ARRAY) {
      if (eval_node(subst, node->right, &ftmp, vtmp, tnode) == 1) {
         for (list=node->left->exper_data.array,i=ftmp;
              i > 0 && list != NULL;
              i--, list = list->next) ;
         if (i == 0 && list != NULL)
            return eval_node(subst, list->element,  fval, vval,
                             tnode);
         else {
            error("Array index (%d) out of range\n", (int)ftmp);
            return 0;
            }
         }
      else
         return 0;
      }
   else if (eval_node(subst, node->left,  &fleft,  vleft, tnode) == 2 &&
            eval_node(subst, node->right, &fright, vright, tnode) == 1) {
      i = fright;
      if (i >= 0 && i < 3)
         *fval = vleft[i];
      else if (i == 3)
         *fval = fleft;
      else {
         error("Subscript out of bounds in eval\n");
         return 0;
         }
      return 1;
      }

   return 0;
}

int
Check_Visibility(Vec start, Vec end)
{
   Ray ray;
   Vec D;
   Flt t;

   VecCopy(start, ray.P);
   VecSub(end, start, D);
   t = sqrt(VecDot(D, D));
   if (t < EPSILON) return 1;
   D[0] /= t; D[1] /= t; D[2] /= t;
   VecCopy(D, ray.D);
   if (Shadow(NULL, NULL, &ray, SMALL, t, 0.0, D))
      return 1;
   else
      return 0;
}

#define pi_3     1.0472
#define pi_23    2.0944
static void
color_wheel(Flt x, Flt y, Flt z, Vec vec)
{
   Flt zx_angle;

   if (ABS(z) < EPSILON)
      if (ABS(x) < EPSILON) {
         MakeVector(1.0, 0.0, 0.0, vec);
         }
      else if (x < 0.0) {
         MakeVector(0.0, 1.0, 1.0, vec);
         }
      else {
         MakeVector(1.0, 0.0, 0.0, vec);
         }
   else if (ABS(x) < EPSILON)
      if (z > 0) {
         MakeVector(0.5, 1.0, 0.0, vec);
         }
      else {
         MakeVector(0.5, 0.0, 1.0, vec);
         }
   else {
      zx_angle = acos(x / sqrt(x*x+z*z));
      if (z > 0.0)
         if (zx_angle < pi_3) {
            MakeVector(1.0, zx_angle / pi_3, 0.0, vec);
            }
         else if (zx_angle < pi_23) {
            MakeVector((pi_23 - zx_angle) / pi_3, 1.0, 0.0, vec);
            }
         else {
            MakeVector(0.0, 1.0, (zx_angle - pi_23)/pi_3, vec);
            }
      else if (zx_angle < pi_3) {
         MakeVector(1.0, 0.0, zx_angle / pi_3, vec);
         }
      else if (zx_angle < pi_23) {
         MakeVector((pi_23 - zx_angle) / pi_3, 0.0, 1.0, vec);
         }
      else {
         MakeVector(0.0, (zx_angle - pi_23) / pi_3, 1.0, vec);
         }
      }
}

/* Once a parse tree has been created we need to convert it into a form
   that can be more easily manipulated.  The return values from this function
   and their meaning are:
      0  -  Unable to evaluate
      1  -  Result is a floating point number, value in "fval"
      2  -  Result is a vector, value in "vval"
      3  -  Result is another expression, value in "nval"
*/
int
eval_node(SUBST_PTR subst, NODE_PTR node, Flt *fval, Vec vval,
          NODE_PTR *nval)
{
   Flt fleft, fright, ftmp;
   Vec vleft, vright, tvec;
   int i, Flag = 0;
   unsigned long nr;
   Ray ray;

   if (node == NULL) {
      error("NULL in eval_node\n");
      return 0;
   }

   *fval = 0.0;
   switch(node->exper_type) {
   case UU_EXPER:
      if (subst == NULL) return 0;
      *fval = subst->U[0];
      Flag = 1;
      break;
   case UV_EXPER:
      if (subst == NULL) return 0;
      *fval = subst->U[1];
      Flag = 1;
      break;
   case UW_EXPER:
      if (subst == NULL) return 0;
      *fval = subst->U[2];
      Flag = 1;
      break;
   case X_EXPER:
      if (subst == NULL) return 0;
      *fval = subst->P[0];
      Flag = 1;
      break;
   case Y_EXPER:
      if (subst == NULL) return 0;
      *fval = subst->P[1];
      Flag = 1;
      break;
   case Z_EXPER:
      if (subst == NULL) return 0;
      *fval = subst->P[2];
      Flag = 1;
      break;
   case VAL_EXPER:
      *fval = node->exper_data.value;
      Flag = 1;
      break;
   case VEC_EXPER:
      VecCopy(node->exper_data.v, vval);
      *fval = 0.0;
      Flag = 2;
      break;
   case VECTOR_EXPER:
      if (eval_node(subst, node->exper_data.vec[0], &vval[0],
                    tvec, nval) == 1 &&
          eval_node(subst, node->exper_data.vec[1], &vval[1],
                    tvec, nval) == 1 &&
          eval_node(subst, node->exper_data.vec[2], &vval[2],
                    tvec, nval) == 1) {
         if (node->exper_data.vec[3] != NULL) {
            if (eval_node(subst, node->exper_data.vec[3], fval,
                          tvec, nval) != 1)
               return 0;
         } else
            *fval = 0.0;
         Flag = 2;
         }
      else
         return 0;
      break;
   case N_EXPER:
      if (subst == NULL) return 0;
      vval[0] = subst->N[0];
      vval[1] = subst->N[1];
      vval[2] = subst->N[2];
      *fval   = 0.0;
      return 2;
   case P_EXPER:
      if (subst == NULL) return 0;
      vval[0] = subst->P[0];
      vval[1] = subst->P[1];
      vval[2] = subst->P[2];
      *fval   = 0.0;
      return 2;
   case U_EXPER:
      if (subst == NULL) return 0;
      vval[0] = subst->U[0];
      vval[1] = subst->U[1];
      vval[2] = subst->U[2];
      *fval   = 0.0;
      return 2;
   case W_EXPER:
      if (subst == NULL) return 0;
      vval[0] = subst->W[0];
      vval[1] = subst->W[1];
      vval[2] = subst->W[2];
      *fval   = 0.0;
      return 2;
   case I_EXPER:
      if (subst == NULL) return 0;
      vval[0] = subst->I[0];
      vval[1] = subst->I[1];
      vval[2] = subst->I[2];
      *fval   = 0.0;
      return 2;
   case START_FRAME:
      *fval = start_frame;
      return 1;
   case END_FRAME:
      *fval = end_frame;
      return 1;
   case TOTAL_FRAMES:
      *fval = total_frames;
      return 1;
   case FRAME:
      *fval = current_frame;
      return 1;
   case OPACITY:
      if (subst == NULL)
         *fval = 1;
      else
         *fval = 1.0 - subst->U[2];
      return 1;
   case COLOR:
      if (subst == NULL)
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
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      *fval = acos(ftmp);
      return 1;
      }
   break;
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
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      *fval = asin(ftmp);
      return 1;
      }
   break;
   case ATAN:
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      *fval = atan(ftmp);
      return 1;
      }
   break;
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
         if (fabs(fleft) < EPSILON)
            *fval = 1.0;
         else
            *fval = fright / fleft;
         }
      return 1;
      }
   break;
   case CEIL:
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      *fval = ceil(ftmp);
      return 1;
      }
   break;
   case COLOR_WHEEL:
      if (eval_node(subst, node->exper_data.param, &ftmp,
                    vleft, nval) == 1 &&
          eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
          eval_node(subst, node->right, &fright, vright, nval) == 1) {
         color_wheel(ftmp, fleft, fright, vval);
         *fval   = 0.0;
         Flag = 2;
         }
      break;
   case CONDITIONAL_EXPER:
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      if (ftmp != 0.0)
         return eval_node(subst, node->left, fval, vval, nval);
      else
         return eval_node(subst, node->right, fval, vval, nval);
      }
   break;
   case COS:
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      *fval = cos(ftmp);
      return 1;
      }
   break;
   case COSH:
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      *fval = cosh(ftmp);
      return 1;
      }
   break;
   case DNOISE:
      if (eval_node(subst, node->left, &ftmp, vleft, nval) == 2) {
         if (node->right == NULL) {
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
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      *fval = exp(ftmp);
      return 1;
      }
   break;
   case FABS:
   switch (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval)) {
      case 1:
         *fval = ABS(ftmp);
         return 1;
      case 2:
         *fval = sqrt(VecDot(vleft, vleft));
         return 1;
      }
   break;
   case FBM:
      return eval_brownian_motion(subst, node, vval);
   case FLOOR:
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      *fval = floor(ftmp);
      return 1;
      }
   break;
   case FMOD:
   if (eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
       eval_node(subst, node->right, &fright, vright, nval) == 1) {
      if (fabs(fleft) < EPSILON)
         *fval = 0.0;
      else
         *fval = fmod(fleft, fright);
      return 1;
      }
   break;
   case FNOISE:
      if (eval_node(subst, node->left, &ftmp, vleft, nval) == 2) {
         if (node->right == NULL) {
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
      if (eval_node(subst, node->exper_data.param, &ftmp,
                    vleft, nval) == 1 &&
          eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
          eval_node(subst, node->right, &fright, vright, nval) == 1) {
         *fval = legendre((int)ftmp, (int)fleft, fright);
         return 1;
         }
      break;
   case LESS_EXPER:
   if (eval_node(subst, node->left, &fleft, vleft, nval) == 1 &&
       eval_node(subst, node->right, &fright, vright, nval) == 1) {
      *fval = (fleft < fright ? 1.0 : 0.0);
      return 1;
      }
   break;
   case LN:
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      *fval = log(ftmp);
      return 1;
      }
   break;
   case LOG:
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      *fval = log10(ftmp);
      return 1;
      }
   break;
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
      *fval = MIN(fleft, fright);
      return 1;
      }
   break;
   case NOISE:
      if (eval_node(subst, node->left, &ftmp, vleft, nval) == 2) {
         if (node->right == NULL) {
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
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      *fval = ramp(ftmp);
      return 1;
      }
   break;
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
      if (eval_node(subst, node->exper_data.param, &ftmp, vleft,
                    nval) == 2 &&
          eval_node(subst, node->left, &ftmp, vright, nval) == 2) {
         Transform tx;
         if (node->right == NULL) {
            /* Rotate a vector by a set of angles with respect to the
               coordinate axes */
            VecScale(M_PI/180.0, vright);
            Get_Rotation_Transformation(&tx, vright);
            TxVector(vval, vleft, &tx);
            *fval = 0.0;
            return 2;
            }
         else if (eval_node(subst, node->right, &ftmp, tvec, nval) == 1) {
            /* Rotate a vector about an axis */
            ftmp = ftmp * M_PI / 180.0;
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
   case SAWTOOTH:
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      *fval = sawtooth(ftmp);
      return 1;
      }
   break;
   case SIN:
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      *fval = sin(ftmp);
      return 1;
      }
   break;
   case SINH:
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      *fval = sinh(ftmp);
      return 1;
      }
   break;
   case SPLINE:
      return eval_spline(subst, node, vval);
   case SQRT:
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      *fval = sqrt(ftmp);
      return 1;
      }
   break;
   case TAN:
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      *fval = tan(ftmp);
      return 1;
      }
   break;
   case TANH:
   if (eval_node(subst, node->exper_data.param, &ftmp, vleft, nval) == 1) {
      *fval = tanh(ftmp);
      return 1;
      }
   break;
   case TRACE:
      if (node->left == NULL) {
         if (subst == NULL)
            return 0;
         else
            VecCopy(subst->W, ray.P)
         if (!eval_node(subst, node->right, &ftmp, ray.D, nval) == 2)
            return 0;
         }
      else if (!eval_node(subst, node->left, &ftmp, ray.P, nval) == 2 ||
               !eval_node(subst, node->right, &ftmp, ray.D, nval) == 2)
         return 0;
      nr = 0;
      Trace(NULL, 1, 1.0, &ray, vval, fval, 1.0, &nr);
      return 2;
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

int
eval_node_dx(SUBST_PTR subst, NODE_PTR node, Flt *fval, Vec vval)
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
      if ((eval_node_dx(subst, node->exper_data.vec[0],
                        &vval[0], vtmp1) == 1) &&
          (eval_node_dx(subst, node->exper_data.vec[1],
                        &vval[1], vtmp1) == 1) &&
          (eval_node_dx(subst, node->exper_data.vec[2],
                        &vval[2], vtmp1) == 1))
         return 2;
      else
         return 0;
      break;
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
      if (val1 < EPSILON) {
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
      /* d(atan(u))/dx = 1/(1+u^2) * du/dx */
      eval_node(subst, node->exper_data.param, &val1, vval, &tnode);
      eval_node_dx(subst, node->exper_data.param, &val2, vtmp1);
      *fval = val2 / (1.0 + val1 * val1);
      break;
   case COS:
      /* d(cos(u))/dx = -sin(u) * du/dx */
      eval_node(subst, node->exper_data.param, &val1, vval, &tnode);
      eval_node_dx(subst, node->exper_data.param, &val2, vtmp1);
      *fval = -sin(val1) * val2;
      break;
   case EXP:
      /* d(e^u)/dx = e^u * du/dx */
      eval_node(subst, node->exper_data.param, &val1, vval, &tnode);
      eval_node_dx(subst, node->exper_data.param, &val2, vtmp1);
      *fval = exp(val1) * val2;
      break;
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
      /* d(ln(u))/dx = 1/u * du/dx */
      eval_node(subst, node->exper_data.param, &val1, vval, &tnode);
      if (ABS(val1) < EPSILON) {
         *fval = 0.0;
         return 0;
         }
      eval_node_dx(subst, node->exper_data.param, &val2, vtmp1);
      *fval = val2 / val1;
      break;
   case LOG:
      /* d(log10(u))/dx = log10(e) * 1/u * du/dx */
      eval_node(subst, node->exper_data.param, &val1, vval, &tnode);
      if (ABS(val1) < EPSILON) {
         *fval = 0.0;
         return 0;
         }
      eval_node_dx(subst, node->exper_data.param, &val2, vtmp1);
      *fval = M_LOG10E * val2 / val1;
      break;
   case SIN:
      /* d(sin(u))/dx = cos(u) * du/dx */
      eval_node(subst, node->exper_data.param, &val1, vval, &tnode);
      eval_node_dx(subst, node->exper_data.param, &val2, vtmp1);
      *fval = cos(val1) * val2;
      break;
   case SQRT:
      /* d(sqrt(u))/dx = 0.5 * du/dx / sqrt(u) */
      eval_node(subst, node->exper_data.param, &val1, vval, &tnode);
      if (val1 <= 0.0) {
         *fval = 0.0;
         return 0;
         }
      eval_node_dx(subst, node->exper_data.param, &val2, vtmp1);
      *fval = 0.5 * val2 / sqrt(val1);
      break;
   case TAN:
      /* d(tan(u))/dx = 1/cos(u)^2 * du/dx */
      eval_node(subst, node->exper_data.param, &val1, vval, &tnode);
      val1 = cos(val1);
      val1 = val1 * val1;
      if (val1 <= 0.0) {
         *fval = 0.0;
         return 0;
         }
      eval_node_dx(subst, node->exper_data.param, &val2, vtmp1);
      *fval = val2 / val1;
      break;
   default:
      /* Don't know how to take the formal derivative, so we will
         evaluate the function at two locations and find the slope */
      sp = &temp_subst;
      memcpy(sp, subst, sizeof(struct subst_struct));
      i = eval_node(sp, node, &val1, vtmp1, &tnode);
      for (k=0;k<3;k++) {
         temp_subst.P[k] += EPSILON * temp_subst.PT[k];
         temp_subst.U[k] += EPSILON * temp_subst.UT[k];
         }
      j = eval_node(sp, node, &val2, vtmp2, &tnode);
      if (i == 1 && j == 1)
         *fval = (val2 - val1) / EPSILON;
      else if (i == 2 && j == 2) {
         VecSub(vtmp2, vtmp1, vval);
         VecScale(1.0 / EPSILON, vval)
         return 2;
         }
   }
   return 1;
}


/*
   vector.c

   Vector and Matrix stuff

  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Portions (C) 2000-2024, C. Meli, All rights reserved.

  This software may be used for any private and non-commercial
  use.

  You may not distribute this software, in whole or in part,
  for any commercial purpose, without the express consent of
  the authors.

  There is no warranty or other guarantee of fitness of this software
  for any purpose.  It is provided solely "as is".

*/

#include "defs.h"
#include "vector.h"
#include "io.h"
#include "memory.h"


Vec ZeroVector = { 0.0, 0.0, 0.0 };
// used in other modules!

/* Random number variables */
#define DEFAULT_SEED 7373
#define MULTIPLIER 18829            /* This is 5^15 (mod 65536)     */
static unsigned char shiftreg[66];  /* 521-bit shift register       */
static unsigned char *randptr;      /* initialized for not accessed */
static unsigned char *endreg;       /* always points at 32nd number */
static int rand_init = 0;

static int flagran = 0;
extern Flt urandomdat[1000];

/* refill the shift register */
static void
refill()
{
   unsigned char *p, *q, cy0, cy1;
   int i;
   p = shiftreg;                   /* point at 1st byte.           */
   q = &p[4];                      /* shift 4 * 8 = 32,            */
//   for (i = 0; i < 62; i++)        /* and mod-2 add the register   */
   for (i=62;i--;)
      *p++ ^= *q++;                /* to the 32-bit shift.         */
   p--;                            /* p should now be at byte 61:  */
   q = shiftreg;                   /* first 489 bits are ok.       */
   cy0 = 0;                        /* set carry bit to zero        */
//   for (i = 0; i < 4; i++) {       /* for the remaining 32 bits:   */
   for (i=4;i--;) {
      cy1 = ((*q & 1) ? 0x80 : 0); /* save carry for nxt shift*/
      *p++ ^= (((*q++ >> 1) & 0x7f) | cy0);
                                   /* mod-2 add the 489-shifts     */
      cy0 = cy1;                   /* set carry for next shift     */
      }
   *p ^= cy0;                      /* and mod-2 the final bit.     */
   randptr = shiftreg;             /* point at the first number    */
}

/* Randomize the generator using seed. */
static void
reset_rand_generator(int seed)
{
   int i, *rseed;

   if (seed == 0)
      seed = DEFAULT_SEED;
   else if (seed < 0) {
      rseed = (int *) &shiftreg[16];
      while ((seed = *(++rseed)) == 0)
         ;       /* find something non-zero, whatever */
      }
   for (i = 0; i < 66; i++) {
      /* assume no adverse reaction on integer overflow */
      shiftreg[i++] = ((seed *= MULTIPLIER) &0xff);
      shiftreg[i] = ((seed >> 8) & 0xff);
      }
   shiftreg[65] &= 0x80;   /* mask out bits beyond 521st  */
   refill();               /* refill and reset randptr */
}

/* Return the next random number.  If shift register is
   depleted, refill the entire register. */
static unsigned
urandom(void)
{
   unsigned r;
   if (!rand_init) {
      randptr = shiftreg - 1;
      endreg = shiftreg + 62;
      reset_rand_generator(0);
      rand_init = 1;
      }
   else if (randptr > endreg)
      refill();
   r  =  (unsigned char)(*randptr++);
   r += ((unsigned char)(*randptr++) << 8);
   return r;
}

/* Return a uniform random value in [0, 1] */
Flt
OLDpolyray_random(void)
{
   return (Flt)urandom() / 65535.0;
}

/* Return a uniform random value in [0, 1] */
Flt
polyray_random(void)
{
  if (flagran > 999) flagran=0;
  return urandomdat[flagran++];
}


#ifndef M_PI
#define M_PI 3.141592653689793
#endif


void
VecH(Vec a, Vec b, Vec c)
{
   VecAdd(a, b, c);
   if (VecNormalize(c) < 0.0)
      error("Half vector is null\n");
}

// CM: f/VecNormalize modifies the vector!!
// because it normalizes the vector
float
fVecNormalize(fVec vec)
{
   float len, len1;
   len = VecDot(vec, vec);
   if (len < 1.0e-20) {
      vec[0] = 1.0;
      vec[1] = 0.0;
      vec[2] = 0.0;
      len = 1.0;
      }
   else {
      len = sqrt(len);
      len1 = 1.0 / len;
      VecScale(len1, vec);
      }
   return(len);
}


#ifdef ORIGINALNORMALIZE
Flt
VecNormalize(Vec vec)
{
   Flt len, len1;
   len = VecDot(vec, vec);
   if (len < 1.0e-20) {
/*      vec[0] = 1.0;
      vec[1] = 0.0;
      vec[2] = 0.0;
      len = 1.0; */
      vec[1] = vec[2] = 0.0;
      len = vec[0] = 1.0;
      }
   else {
      len = sqrt(len);
/*      len1 = 1.0 / len;
      VecScale(len1, vec); */
      VecScale( 1.0 / len, vec);
      }
   return(len);
}
#else

/* Copyright C. Meli */
Flt VecNormalize(Vec vec)
{
   Flt len, scl;
   len = VecDot(vec, vec);
   if (len < 1.0e-20) {
      vec[1] = vec[2] = 0.0;
      len = vec[0] = 1.0;
      }
   else {
      len = sqrt(len);
      scl = RECIPROCAL(len);
      VecScale( scl, vec);
      }
   return(len);
}
/* end CM*/
#endif


void
MZero(Matrix result)
{
   int i, j;
   //for (i = 0 ; i < 4 ; i++)
   //   for (j = 0 ; j < 4 ; j++)
   for (i=4;i--;)
     for (j=4;j--;)
         result[i][j] = 0.0;
}

void
MIdentity(Matrix result)
{
   int i, j;
   //for (i=0;i<4;i++)
   //  for (j=0;j<4;j++)
   for (i=4;i--;)
     for (j=4;j--;)
        if (i==j)
           result[i][j] = 1.0;
        else
           result[i][j] = 0.0;
}

/* Code by C. Meli */
void
MCopy(Matrix result, Matrix src)
{
   int i, j;
   for (i=0;i<4;i++)
     for (j=0;j<4;j++)
       result[i][j]=src[i][j];
}


#if defined(__APPLE__) || !defined(EXPERIMENTALMATRIX) 
void MTimes(Matrix result, Matrix a, Matrix b)
{
  result[0][0] = a[0][0] * b[0][0] + a[0][1] * b[1][0] + a[0][2] * b[2][0] + a[0][3] * b[3][0];
  result[0][1] = a[0][0] * b[0][1] + a[0][1] * b[1][1] + a[0][2] * b[2][1] + a[0][3] * b[3][1];
  result[0][2] = a[0][0] * b[0][2] + a[0][1] * b[1][2] + a[0][2] * b[2][2] + a[0][3] * b[3][2];
  result[0][3] = a[0][0] * b[0][3] + a[0][1] * b[1][3] + a[0][2] * b[2][3] + a[0][3] * b[3][3];
  result[1][0] = a[1][0] * b[0][0] + a[1][1] * b[1][0] + a[1][2] * b[2][0] + a[1][3] * b[3][0];
  result[1][1] = a[1][0] * b[0][1] + a[1][1] * b[1][1] + a[1][2] * b[2][1] + a[1][3] * b[3][1];
  result[1][2] = a[1][0] * b[0][2] + a[1][1] * b[1][2] + a[1][2] * b[2][2] + a[1][3] * b[3][2];
  result[1][3] = a[1][0] * b[0][3] + a[1][1] * b[1][3] + a[1][2] * b[2][3] + a[1][3] * b[3][3];
  result[2][0] = a[2][0] * b[0][0] + a[2][1] * b[1][0] + a[2][2] * b[2][0] + a[2][3] * b[3][0];
  result[2][1] = a[2][0] * b[0][1] + a[2][1] * b[1][1] + a[2][2] * b[2][1] + a[2][3] * b[3][1];
  result[2][2] = a[2][0] * b[0][2] + a[2][1] * b[1][2] + a[2][2] * b[2][2] + a[2][3] * b[3][2];
  result[2][3] = a[2][0] * b[0][3] + a[2][1] * b[1][3] + a[2][2] * b[2][3] + a[2][3] * b[3][3];
  result[3][0] = a[3][0] * b[0][0] + a[3][1] * b[1][0] + a[3][2] * b[2][0] + a[3][3] * b[3][0];
  result[3][1] = a[3][0] * b[0][1] + a[3][1] * b[1][1] + a[3][2] * b[2][1] + a[3][3] * b[3][1];
  result[3][2] = a[3][0] * b[0][2] + a[3][1] * b[1][2] + a[3][2] * b[2][2] + a[3][3] * b[3][2];
  result[3][3] = a[3][0] * b[0][3] + a[3][1] * b[1][3] + a[3][2] * b[2][3] + a[3][3] * b[3][3];

}
#else
//based on
//https://stackoverflow.com/questions/18499971/efficient-4x4-matrix-multiplication-c-vs-assembly/18508113#18508113
SuperMatrix myMultiply(SuperMatrix M1, SuperMatrix M2) {
    // Perform a 4x4 matrix multiply by a 4x4 matrix
    // Be sure to run in 64 bit mode and set right flags
    // Properties, C/C++, Enable Enhanced Instruction, /arch:AVX
    // Having MATRIX on a 32 byte boundry does help performance
    SuperMatrix mResult;
    __m256 a0, a1, b0, b1;
    __m256 c0, c1, c2, c3, c4, c5, c6, c7;
    __m256 t0, t1, u0, u1;

    t0 = M1.n[0];                                                   // t0 = a00, a01, a02, a03, a10, a11, a12, a13
    t1 = M1.n[1];                                                   // t1 = a20, a21, a22, a23, a30, a31, a32, a33
    u0 = M2.n[0];                                                   // u0 = b00, b01, b02, b03, b10, b11, b12, b13
    u1 = M2.n[1];                                                   // u1 = b20, b21, b22, b23, b30, b31, b32, b33

    a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(0, 0, 0, 0));        // a0 = a00, a00, a00, a00, a10, a10, a10, a10
    a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(0, 0, 0, 0));        // a1 = a20, a20, a20, a20, a30, a30, a30, a30
    b0 = _mm256_permute2f128_ps(u0, u0, 0x00);                      // b0 = b00, b01, b02, b03, b00, b01, b02, b03
    c0 = _mm256_mul_ps(a0, b0);                                     // c0 = a00*b00  a00*b01  a00*b02  a00*b03  a10*b00  a10*b01  a10*b02  a10*b03
    c1 = _mm256_mul_ps(a1, b0);                                     // c1 = a20*b00  a20*b01  a20*b02  a20*b03  a30*b00  a30*b01  a30*b02  a30*b03

    a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(1, 1, 1, 1));        // a0 = a01, a01, a01, a01, a11, a11, a11, a11
    a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(1, 1, 1, 1));        // a1 = a21, a21, a21, a21, a31, a31, a31, a31
    b0 = _mm256_permute2f128_ps(u0, u0, 0x11);                      // b0 = b10, b11, b12, b13, b10, b11, b12, b13
    c2 = _mm256_mul_ps(a0, b0);                                     // c2 = a01*b10  a01*b11  a01*b12  a01*b13  a11*b10  a11*b11  a11*b12  a11*b13
    c3 = _mm256_mul_ps(a1, b0);                                     // c3 = a21*b10  a21*b11  a21*b12  a21*b13  a31*b10  a31*b11  a31*b12  a31*b13

    a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(2, 2, 2, 2));        // a0 = a02, a02, a02, a02, a12, a12, a12, a12
    a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(2, 2, 2, 2));        // a1 = a22, a22, a22, a22, a32, a32, a32, a32
    b1 = _mm256_permute2f128_ps(u1, u1, 0x00);                      // b0 = b20, b21, b22, b23, b20, b21, b22, b23
    c4 = _mm256_mul_ps(a0, b1);                                     // c4 = a02*b20  a02*b21  a02*b22  a02*b23  a12*b20  a12*b21  a12*b22  a12*b23
    c5 = _mm256_mul_ps(a1, b1);                                     // c5 = a22*b20  a22*b21  a22*b22  a22*b23  a32*b20  a32*b21  a32*b22  a32*b23

    a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(3, 3, 3, 3));        // a0 = a03, a03, a03, a03, a13, a13, a13, a13
    a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(3, 3, 3, 3));        // a1 = a23, a23, a23, a23, a33, a33, a33, a33
    b1 = _mm256_permute2f128_ps(u1, u1, 0x11);                      // b0 = b30, b31, b32, b33, b30, b31, b32, b33
    c6 = _mm256_mul_ps(a0, b1);                                     // c6 = a03*b30  a03*b31  a03*b32  a03*b33  a13*b30  a13*b31  a13*b32  a13*b33
    c7 = _mm256_mul_ps(a1, b1);                                     // c7 = a23*b30  a23*b31  a23*b32  a23*b33  a33*b30  a33*b31  a33*b32  a33*b33

    c0 = _mm256_add_ps(c0, c2);                                     // c0 = c0 + c2 (two terms, first two rows)
    c4 = _mm256_add_ps(c4, c6);                                     // c4 = c4 + c6 (the other two terms, first two rows)
    c1 = _mm256_add_ps(c1, c3);                                     // c1 = c1 + c3 (two terms, second two rows)
    c5 = _mm256_add_ps(c5, c7);                                     // c5 = c5 + c7 (the other two terms, second two rose)

                                                                    // Finally complete addition of all four terms and return the results
    mResult.n[0] = _mm256_add_ps(c0, c4);       // n0 = a00*b00+a01*b10+a02*b20+a03*b30  a00*b01+a01*b11+a02*b21+a03*b31  a00*b02+a01*b12+a02*b22+a03*b32  a00*b03+a01*b13+a02*b23+a03*b33
                                                //      a10*b00+a11*b10+a12*b20+a13*b30  a10*b01+a11*b11+a12*b21+a13*b31  a10*b02+a11*b12+a12*b22+a13*b32  a10*b03+a11*b13+a12*b23+a13*b33
    mResult.n[1] = _mm256_add_ps(c1, c5);       // n1 = a20*b00+a21*b10+a22*b20+a23*b30  a20*b01+a21*b11+a22*b21+a23*b31  a20*b02+a21*b12+a22*b22+a23*b32  a20*b03+a21*b13+a22*b23+a23*b33
                                                //      a30*b00+a31*b10+a32*b20+a33*b30  a30*b01+a31*b11+a32*b21+a33*b31  a30*b02+a31*b12+a32*b22+a33*b32  a30*b03+a31*b13+a32*b23+a33*b33
    return mResult;
}

void MTimes(Matrix result, Matrix a, Matrix b)
{
	SuperMatrix m1, m2;
	MCopy(m1.f,a);MCopy(m2.f,b);
	//m1.f=a;m2.f=b;
    SuperMatrix res=myMultiply(m1,m2);
	result=res.f;
}
#endif

/* end new code by CM */

void
MAdd(Matrix result, Matrix matrix1, Matrix matrix2)
{
   int i, j;

   for (i=0;i<4;i++)
      for (j=0;j<4;j++)
         result[i][j] = matrix1[i][j] + matrix2[i][j];
}

void
MSub(Matrix result, Matrix matrix1, Matrix matrix2)
{
   int i, j;

   for (i=0;i<4;i++)
      for (j=0;j<4;j++)
         result[i][j] = matrix1[i][j] - matrix2[i][j];
}

void
MScale(Matrix result, Matrix matrix1, Flt size)
{
   int i, j;

   for (i=0;i<4;i++)
      for (j=0;j<4;j++)
         result[i][j] = size * matrix1[i][j];
   return;
}

void
MTranspose(Matrix result, Matrix matrix1)
{
   int i, j;
   Matrix temp_matrix;

   for (i=0;i<4;i++)
      for (j=0;j<4;j++)
         temp_matrix[i][j] = matrix1[j][i];

   for (i=0;i<4;i++)
      for (j=0;j<4;j++)
         result[i][j] = temp_matrix[i][j];
}

void
fTxVec(fVec out, fVec vec, Transform *tx)
{
   int i;
   Matrix *matrix = &tx->matrix;
   fVec result;

   if (tx == NULL) {
      VecCopy(vec, out);
      return;
      }

   for (i=0;i<3;i++)
      result[i] = vec[0] * (*matrix)[0][i] +
                  vec[1] * (*matrix)[1][i] +
                  vec[2] * (*matrix)[2][i] +
                           (*matrix)[3][i];
   for (i=0;i<3;i++)
      out[i] = result[i];
}

void
TxVec(Vec out, Vec vec, Transform *tx)
{
   int i;
   Matrix *matrix = &tx->matrix;
   Vec result;

   if (tx == NULL) {
      VecCopy(vec, out);
      return;
      }

   for (i=0;i<3;i++)
      result[i] = vec[0] * (*matrix)[0][i] +
                  vec[1] * (*matrix)[1][i] +
                  vec[2] * (*matrix)[2][i] +
                           (*matrix)[3][i];
   for (i=0;i<3;i++)
      out[i] = result[i];
}

void
InvTxVec(Vec out, Vec vec, Transform *tx)
{
   int i;
   Vec result;
   Matrix *matrix = &tx->inverse;

   if (tx == NULL) {
      VecCopy(vec, out);
      return;
      }

   for (i=0;i<3;i++)
      result[i] = vec[0] * (*matrix)[0][i] +
                  vec[1] * (*matrix)[1][i] +
                  vec[2] * (*matrix)[2][i] +
                           (*matrix)[3][i];
   for (i=0;i<3;i++)
      out[i] = result[i];
}

/* This routine is called so obscenely often, it just *has* to be
   optimized.  We can optimize away the "out" stuff for the case that
   out != vec, and the other cases should be rewritten to meet this
   case, too. Rendering discs.pi spent only more time in eval_node ! */

void
InvTxVec1(Vec out, Vec vec, Transform *tx)
{
   Matrix *matrix = &tx->inverse;

   if (tx == NULL) {
      VecCopy(vec, out);
      return;
      }

   out[0] = vec[0] * (*matrix)[0][0] + vec[1] * (*matrix)[1][0] +
            vec[2] * (*matrix)[2][0] + (*matrix)[3][0];
   out[1] = vec[0] * (*matrix)[0][1] + vec[1] * (*matrix)[1][1] +
            vec[2] * (*matrix)[2][1] + (*matrix)[3][1];
   out[2] = vec[0] * (*matrix)[0][2] + vec[1] * (*matrix)[1][2] +
            vec[2] * (*matrix)[2][2] + (*matrix)[3][2];
}


void
TxVec3(Vec out, Vec vec, Transform *tx)
{
   int i;
   Matrix *matrix = &tx->matrix;
   Vec result;

   if (tx == NULL) {
      VecCopy(vec, out);
      return;
      }

   for (i=0;i<3;i++)
      result[i] = vec[0] * (*matrix)[0][i] +
                  vec[1] * (*matrix)[1][i] +
                  vec[2] * (*matrix)[2][i];
   for (i=0;i<3;i++)
      out[i] = result[i];
}

void
InvTxVec3(Vec out, Vec vec, Transform *tx)
{
   int i;
   Vec result;
   Matrix *matrix = &tx->inverse;

   if (tx == NULL) {
      VecCopy(vec, out);
      return;
      }

   for (i=0;i<3;i++)
      result[i] = vec[0] * (*matrix)[0][i] +
                  vec[1] * (*matrix)[1][i] +
                  vec[2] * (*matrix)[2][i];
   for (i=0;i<3;i++)
      out[i] = result[i];
}

void
fTxNormal(fVec out, fVec vec, Transform *tx)
{
   int i;
   Matrix *matrix = &tx->inverse;
   fVec result;

   if (tx == NULL) {
      VecCopy(vec, out);
      return;
      }

   for (i=0;i<3;i++)
      result[i] = vec[0] * (*matrix)[i][0] +
                  vec[1] * (*matrix)[i][1] +
                  vec[2] * (*matrix)[i][2];
   for (i=0;i<3;i++)
      out[i] = result[i];
}

void
TxNormal(Vec out, Vec vec, Transform *tx)
{
   int i;
   Matrix *matrix = &tx->inverse;
   Vec result;

   if (tx == NULL) {
      VecCopy(vec, out);
      return;
      }

   for (i=0;i<3;i++)
      result[i] = vec[0] * (*matrix)[i][0] +
                  vec[1] * (*matrix)[i][1] +
                  vec[2] * (*matrix)[i][2];
   for (i=0;i<3;i++)
      out[i] = result[i];
}

void
InvTxNormal(Vec out, Vec vec, Transform *tx)
{
   int i;
   Vec result;
   Matrix *matrix = &tx->matrix;

   if (tx == NULL) {
      VecCopy(vec, out);
      return;
      }

   for (i=0;i<3;i++)
      result[i] = vec[0] * (*matrix)[i][0] +
                  vec[1] * (*matrix)[i][1] +
                  vec[2] * (*matrix)[i][2];
   for (i=0;i<3;i++)
      out[i] = result[i];
}

void
Get_Scaling_Transformation(Transform *tx, Vec vector)
{
   MIdentity(tx->matrix);
   tx->matrix[0][0] = vector[0];
   tx->matrix[1][1] = vector[1];
   tx->matrix[2][2] = vector[2];

   MIdentity(tx->inverse);
   tx->inverse[0][0] = (vector[0] == 0.0 ? PLY_HUGE : 1.0 / vector[0]);
   tx->inverse[1][1] = (vector[1] == 0.0 ? PLY_HUGE : 1.0 / vector[1]);
   tx->inverse[2][2] = (vector[2] == 0.0 ? PLY_HUGE : 1.0 / vector[2]);
}

void
Get_Perspective_Transformation(Transform *result, Flt dist)
{
   MIdentity(result->matrix);
   result->matrix[2][2] =  1.0 / (1.0 + dist);
   result->matrix[3][2] =  dist / (1.0 + dist);
   result->matrix[2][3] =  1.0;
   result->matrix[3][3] =  0.0;
   MIdentity(result->inverse);
   result->inverse[2][3] = dist;
}

void
Get_Translation_Transformation(Transform *tx, Vec vector)
{
   MIdentity(tx->matrix);
   tx->matrix[3][0] = vector[0];
   tx->matrix[3][1] = vector[1];
   tx->matrix[3][2] = vector[2];

   MIdentity(tx->inverse);
   tx->inverse[3][0] = 0.0 - vector[0];
   tx->inverse[3][1] = 0.0 - vector[1];
   tx->inverse[3][2] = 0.0 - vector[2];
}

void
Get_Rotation_Transformation(Transform *tx, Vec vector)
{
   Matrix matrix;
   Flt cosx, cosy, cosz, sinx, siny, sinz;

   MIdentity(tx->matrix);
   cosx = cos(vector[0]);
   sinx = sin(vector[0]);
   cosy = cos(vector[1]);
   siny = sin(vector[1]);
   cosz = cos(vector[2]);
   sinz = sin(vector[2]);

   tx->matrix[1][1] =  cosx;
   tx->matrix[2][2] =  cosx;
   tx->matrix[1][2] =  sinx;
   tx->matrix[2][1] = -sinx;
   MTranspose(tx->inverse, tx->matrix);

   MIdentity(matrix);
   matrix[0][0] =  cosy;
   matrix[2][2] =  cosy;
   matrix[0][2] = -siny;
   matrix[2][0] =  siny;
   MTimes(tx->matrix, tx->matrix, matrix);
   MTranspose(matrix, matrix);
   MTimes(tx->inverse, matrix, tx->inverse);

   MIdentity(matrix);
   matrix[0][0] =  cosz;
   matrix[1][1] =  cosz;
   matrix[0][1] =  sinz;
   matrix[1][0] = -sinz;
   MTimes(tx->matrix, tx->matrix, matrix);
   MTranspose(matrix, matrix);
   MTimes(tx->inverse, matrix, tx->inverse);
}

void
Get_Shear_Transformation(Transform *result, Flt xy, Flt xz, Flt yx, Flt yz,
                         Flt zx, Flt zy)
{
   MIdentity(result->matrix);
   result->matrix[0][1] = xy;
   result->matrix[0][2] = xz;
   result->matrix[1][0] = yx;
   result->matrix[1][2] = yz;
   result->matrix[2][0] = zx;
   result->matrix[2][1] = zy;

   MIdentity(result->inverse);
   result->inverse[0][1] = -xy;
   result->inverse[0][2] = -xz;
   result->inverse[1][0] = -yx;
   result->inverse[1][2] = -yz;
   result->inverse[2][0] = -zx;
   result->inverse[2][1] = -zy;
}

void
Compose_Transformations(Transform *tx0, Transform *tx1)
{
   MTimes(tx0->matrix,  tx0->matrix,  tx1->matrix);
   MTimes(tx0->inverse, tx1->inverse, tx0->inverse);
}

Transform *
Get_Transformation()
{
  Transform *tx;

  if ((tx = (Transform *)polyray_malloc(sizeof (Transform))) == NULL)
     error("Cannot allocate transformation");
  MIdentity(tx->matrix);
  MIdentity(tx->inverse);
  return tx;
}

/* Rotation about an arbitrary axis - formula from:
      "Computational Geometry for Design and Manufacture",
      Faux & Pratt
   Note that the angles for this transform are specified in radians.
*/
void
Get_Rotate_Transform(Transform *trans, Vec V, Flt angle)
{
   Flt cosx, sinx;

   (void)VecNormalize(V); /* Note: bad values (zero vector) can occur here */
   MIdentity(trans->matrix);
   cosx = cos(angle);
   sinx = sin(angle);
   trans->matrix[0][0] = V[0] * V[0] + cosx * (1.0 - V[0] * V[0]);
   trans->matrix[0][1] = V[0] * V[1] * (1.0 - cosx) + V[2] * sinx;
   trans->matrix[0][2] = V[0] * V[2] * (1.0 - cosx) - V[1] * sinx;
   trans->matrix[1][0] = V[0] * V[1] * (1.0 - cosx) - V[2] * sinx;
   trans->matrix[1][1] = V[1] * V[1] + cosx * (1.0 - V[1] * V[1]);
   trans->matrix[1][2] = V[1] * V[2] * (1.0 - cosx) + V[0] * sinx;
   trans->matrix[2][0] = V[0] * V[2] * (1.0 - cosx) + V[1] * sinx;
   trans->matrix[2][1] = V[1] * V[2] * (1.0 - cosx) - V[0] * sinx;
   trans->matrix[2][2] = V[2] * V[2] + cosx * (1.0 - V[2] * V[2]);
   MTranspose(trans->inverse, trans->matrix);
}

/* Given a point and a direction and a radius, find the transform
   that brings these into a canonical coordinate system */
void
Get_Coordinate_Transform(Transform *trans, Vec origin, Vec up, Flt r, Flt len)
{
   Transform trans2;
   Vec tmpv;

   Get_Translation_Transformation(trans, origin);
   if (fabs(up[2]) == 1.0) {
      MakeVector(1.0, 0.0, 0.0, tmpv);
      }
   else {
      MakeVector(-up[1], up[0], 0.0, tmpv);
      }
   Get_Rotate_Transform(&trans2, tmpv, -acos(up[2]));
   Compose_Transformations(trans, &trans2);
   MakeVector(1/r, 1/r, 1/len, tmpv);
   Get_Scaling_Transformation(&trans2, tmpv);
   Compose_Transformations(trans, &trans2);
}

/* Find the transformation that takes the current eye position and
   orientation and puts it at {0, 0, 0} with the up vector aligned
   with {0, 1, 0}.  */
Transform *
Normalize_View(Viewpoint *eye)
{
   long xs, ys;
   Flt d, XScale, YScale;
   Vec Va, right;
   Transform *Tv = Get_Transformation();
   Transform Tt;

   /* Move everything so that the eye is at <0,0,0> */
   VecCopy(eye->view_from, Va)
   VecNegate(Va)
   Get_Translation_Transformation(Tv, Va);

   VecSub(eye->view_at, eye->view_from, Va);
   /* Make sure this is a valid sort of setup */
   if (Va[0] == 0.0 && Va[1] == 0.0 && Va[2] == 0.0) {
      warning("Eye position is same as point of interest\n");
      Va[2] = 1;  /* Pick an arbitrary direction to use */
      }
   /* Get the up vector to be perpendicular to the view vector */
   d = VecNormalize(Va);

   if ((fabs(Va[0]+eye->view_up[0]) < EPSILON &&
        fabs(Va[1]+eye->view_up[1]) < EPSILON &&
        fabs(Va[2]+eye->view_up[2]) < EPSILON) ||
       (fabs(Va[0]-eye->view_up[0]) < EPSILON &&
        fabs(Va[1]-eye->view_up[1]) < EPSILON &&
        fabs(Va[2]-eye->view_up[2]) < EPSILON)) {
      /* View and up are either the same or 180 degrees off. Need to
         correct. */
      if (fabs(Va[0]) > 0.0)
         MakeVector(0, 1, 0, right)
      else
         MakeVector(1, 0, 0, right)
      }
   else {
      VecCross(eye->view_up, Va, right);
      VecNormalize(right);
      }

#if 0
printf("up:    <%g,%g,%g>\n", eye->view_up[0], eye->view_up[1], eye->view_up[2]);
printf("Va:    <%g,%g,%g>\n", Va[0], Va[1], Va[2]);
printf("right: <%g,%g,%g>\n", right[0], right[1], right[2]);
#endif

   if (VecClose(Va, right)) {
      /* We need to have the direction and right vectors pointed
         in different directions or the next few steps will fail. */
      if (fabs(Va[0]) > 0.0)
         MakeVector(0, 0, -1, right)
      else
         MakeVector(1, 0, 0, right)
      }

   VecCross(Va, right, eye->view_up);
   VecNormalize(eye->view_up);

#if 0
/* See what we got */
printf("from:  <%g,%g,%g>\n", eye->view_from[0], eye->view_from[1], eye->view_from[2]);
printf("at:    <%g,%g,%g>\n", eye->view_at[0], eye->view_at[1], eye->view_at[2]);
printf("up:    <%g,%g,%g>\n", eye->view_up[0], eye->view_up[1], eye->view_up[2]);
printf("Va:    <%g,%g,%g>\n", Va[0], Va[1], Va[2]);
printf("right: <%g,%g,%g>\n", right[0], right[1], right[2]);
#endif

   /* Create the orthogonal view transformation */
   Tt.matrix[0][0] = right[0];
   Tt.matrix[1][0] = right[1];
   Tt.matrix[2][0] = right[2];
   Tt.matrix[3][0] = 0;

   Tt.matrix[0][1] = eye->view_up[0];
   Tt.matrix[1][1] = eye->view_up[1];
   Tt.matrix[2][1] = eye->view_up[2];
   Tt.matrix[3][1] = 0;

   Tt.matrix[0][2] = Va[0];
   Tt.matrix[1][2] = Va[1];
   Tt.matrix[2][2] = Va[2];
   Tt.matrix[3][2] = 0;

   Tt.matrix[0][3] = 0;
   Tt.matrix[1][3] = 0;
   Tt.matrix[2][3] = 0;
   Tt.matrix[3][3] = 1;
   MIdentity(Tt.inverse);
   Compose_Transformations(Tv, &Tt);

   /* Now add in the perspective transformation */
   /* Get_Perspective_Transformation(&Tt, eye->view_hither); */
   Get_Perspective_Transformation(&Tt, 1.0e-5);
   Compose_Transformations(Tv, &Tt);

   /* Determine how much to scale things by */
   YScale = 0.5 * (Flt)eye->view_yres / tan(eye->view_angle);
   XScale = YScale * (Flt)eye->view_xres /
                       ((Flt)eye->view_yres * eye->view_aspect);
   MakeVector(XScale, -YScale, 1.0, Va);
   Get_Scaling_Transformation(&Tt, Va);
   Compose_Transformations(Tv, &Tt);

   /* Now translate to the center of the screen */
   xs = eye->view_xres / 2;
   ys = eye->view_yres / 2;
   MakeVector(xs, ys, 0.0, Va);
   Get_Translation_Transformation(&Tt, Va);
   Compose_Transformations(Tv, &Tt);

   return Tv;
}

void
geocentric_to_cartesian(Vec Q, Vec P)
{
   Flt r, t0, t1;

   r  = Q[2];
   t1 = sin(Q[1]);
   P[1] = r * t1;
   t0 = sin(Q[0]);
   t1 = sqrt(1.0 - t1 * t1);
   P[2] = r * t0 * t1;
   t0 = cos(Q[0]);
   P[0] = r * t0 * t1;
}

void
cartesian_to_geocentric(Vec P, Vec Q)
{
   Flt len;

   len = VecLen(P);
   Q[2] = len;
   if (len < EPSILON) {
      MakeVector(0, 0, 0, Q);
      return;
      }

   /* Determine the angles */
   Q[1] = asin(P[1] / len);
   if (fabs(Q[1]) + EPSILON > M_PI_2)
      Q[0] = 0.0;
   else {
      Q[0] = atan2(P[2], P[0]);
#if 0
      if (Q[0] < 0 && Q[0] > -EPSILON)
         Q[0] = 0.0;
#endif
      }
}

/* Convert cylindrical coordinates (theta, z, r) to cartesian (x, y, z) */
void
cylindrical_to_cartesian(Vec Q, Vec P)
{
   P[0] = Q[2] * cos(Q[0]);
   P[2] = Q[2] * sin(Q[0]);
   P[1] = Q[1];
}

/* Convert cartesian (x, y, z) to cylindrical coordinates (theta, z, r) */
void
cartesian_to_cylindrical(Vec P, Vec Q)
{
   Q[1] = P[1];
   Q[2] = sqrt(P[0] * P[0] + P[2] * P[2]);
   if (Q[2] < EPSILON)
      Q[0] = 0.0;
   else
      Q[0] = atan2(P[2], P[0]);
}

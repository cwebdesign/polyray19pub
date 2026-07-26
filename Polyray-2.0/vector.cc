/** @file vector.cc
 *  @brief Vector normalisation, 4x4 matrix arithmetic, Transform construction,
 *         and coordinate-system conversions.
 *
 *  Provides two random-number subsystems (a legacy 521-bit shift-register RNG
 *  and the modern openpolyray::Urandomdat wrapper), plus the following groups:
 *  - Vec / fVec normalisation helpers
 *  - 4x4 Matrix operations (zero, identity, copy, multiply, add, sub, scale, transpose)
 *  - Transform application: TxVec / InvTxVec / TxNormal / InvTxNormal
 *    (all variants with and without translation, pointer and reference overloads)
 *  - Transform construction: scale, translate, rotate (Euler + axis-angle),
 *    shear, perspective, canonical-coordinate, and full camera normalisation
 *  - Coordinate conversion: Cartesian <-> geocentric (spherical) and
 *    Cartesian <-> cylindrical
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
#include "factory.h"
#include "vector.h"
#include "io_ply.h"
#include "memory.h"
#include "urandomdat.h"

Vec ZeroVector = { 0.0, 0.0, 0.0 }; /**< Globally available zero vector (0,0,0). */

/* Legacy 521-bit shift-register RNG - superseded by openpolyray::Urandomdat */
constexpr int DEFAULT_SEED= 7373;
constexpr int MULTIPLIER =18829 ;           /* This is 5^15 (mod 65536)     */
static unsigned char shiftreg[66];  /**< 521-bit feedback shift register state. */
static unsigned char *randptr;      /**< Read pointer into shiftreg; null before first call. */
static unsigned char *endreg;       /**< Points one past the last valid byte (shiftreg+62). */
static int rand_init = 0;           /**< 0 until urandom() performs its lazy initialisation. */

static int flagran = 0;             /**< Unused index for the old urandomdat array path. */
openpolyray::Urandomdat ura(42);    /**< Global MT64 RNG instance seeded at 42. */

/** @brief Refill the 521-bit shift register and reset the read pointer.
 *
 *  Applies a linear feedback step: XORs bytes 0-61 with bytes 4-65, then
 *  performs a 1-bit right-shift with carry for the final 32 bits.
 *  After the call @c randptr points back to @c shiftreg[0].
 */
static void refill()
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

/** @brief Seed the 521-bit shift register and perform an initial refill.
 *
 *  If @p seed is 0 the default seed (7373) is used.  If negative, an existing
 *  non-zero 16-bit word from inside the register is reused as the seed.
 *  Each pair of bytes in the register is filled with successive products of
 *  @c seed x MULTIPLIER, producing a deterministic starting state.
 *  @param seed  Non-negative: explicit seed; 0: default; negative: self-seed.
 */
static void reset_rand_generator(int seed)
{
   if (seed == 0)
      seed = DEFAULT_SEED;
   else if (seed < 0) {
      short int *rseed = reinterpret_cast<short int*>(&shiftreg[16]);
      while ((seed = *(++rseed)) == 0)
         ;       /* find something non-zero, whatever */
      }
   for (int i = 0; i < 65; i++) {
      /* assume no adverse reaction on integer overflow */
      shiftreg[i++] = ((seed *= MULTIPLIER) &0xff);
      shiftreg[i] = ((seed >> 8) & 0xff);
      }
   shiftreg[65] &= 0x80;   /* mask out bits beyond 521st  */
   refill();               /* refill and reset randptr */
}

/** @brief Return the next 16-bit unsigned random from the shift register.
 *
 *  Performs lazy initialisation on the first call (seeds with 0 = default).
 *  Calls refill() automatically when all 62 bytes have been consumed.
 *  Assembles two successive bytes into a 16-bit value (little-endian order).
 *  @return  A pseudo-random value in [0, 65535].
 */
static unsigned urandom()
{
   if (!rand_init) {
      randptr = shiftreg - 1;
      endreg = shiftreg + 62;
      reset_rand_generator(0);
      rand_init = 1;
      }
   else if (randptr > endreg)
      refill();
   unsigned r  =  (unsigned char)(*randptr++);
   r += ((unsigned char)(*randptr++) << 8);
   return r;
}

/** @brief Legacy uniform random value in [0, 1] from the shift-register RNG.
 *
 *  @deprecated Superseded by polyray_random() which uses Urandomdat / MT64.
 *  @return  A uniform @c Flt in [0, 1] (divides urandom() by 65535).
 */
static Flt OLDpolyray_random(void)
{
   return (Flt)urandom() / 65535.0;
}

/** @brief Return a uniform random @c Flt in [0, 1] via the MT64 generator.
 *
 *  Delegates to the global @c ura (openpolyray::Urandomdat seeded at 42).
 *  Used throughout the renderer for jitter, Monte-Carlo sampling, and texture
 *  noise.
 *  @return  A uniform @c Flt in [0, 1].
 */
Flt polyray_random(void)
{
  return ura.polyray_random();
}



/** @brief Compute the normalised half-vector of @p a and @p b.
 *
 *  Sets @p c = normalise(@p a + @p b).  Calls serror() if the sum is the
 *  zero vector (i.e. @p a and @p b are anti-parallel).
 *  @param a  First input vector.
 *  @param b  Second input vector.
 *  @param c  Output: normalised bisector of @p a and @p b.
 */
void VecH(const Vec a, const Vec b, Vec& c)
{
   VecAdd(a, b, c);
   if (VecNormalize(c) < 0.0)
      serror("Half vector is null\n");
}

/** @brief Normalise a @c fVec in place and return its original length.
 *
 *  If the squared length is below 1e-20 the vector is replaced with (1,0,0)
 *  and 1.0 is returned to avoid division by zero.  Otherwise the vector is
 *  scaled by 1/length in place.
 *  @param vec  Vector to normalise (modified in place).
 *  @return     The Euclidean length of @p vec before normalisation; 1.0 on
 *              degenerate input.
 */
float fVecNormalize(fVec& vec)
{
   float len, len1;
   len = VecDot(vec, vec);
   if (len < 1.0e-20) {
      vec[0] = 1.0;
      vec[1] = 0.0;
      vec[2] = 0.0;
      len = 1.0;
      //std::cout << "vec[0]=" << vec[0] << " vec[1]=" << vec[1] << " vec[2]=" << vec[2] << std::endl;
      }
   else {
      len = sqrt(len);
      len1 = 1.0 / len;
      VecScale(len1, vec);
      //std::cout<<"vec[0]="<<vec[0]<<" vec[1]="<<vec[1]<<" vec[2]="<<vec[2]<<std::endl;
      }
   return(len);
}


/** @brief Normalise a @c Vec in place (original version, compiled only when ORIGINALNORMALIZE is defined).
 *
 *  Degenerate input (squared length < 1e-20) is replaced with (1,0,0).
 *  @param vec  Vector to normalise (modified in place).
 *  @return     The Euclidean length before normalisation; 1.0 on degenerate input.
 */
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

/** @brief Normalise a @c Vec in place and return its original length.
 *
 *  Degenerate input (squared length < 1e-20) is replaced with (1,0,0) and
 *  1.0 is returned.  Uses RECIPROCAL() for the scale factor.
 *  @param vec  Vector to normalise (modified in place via C-array pointer).
 *  @return     The Euclidean length before normalisation; 1.0 on degenerate input.
 */
Flt VecNormalize(Vec vec) //todo:change to C++ reference,removing C interface
{
   Flt len, scl;
   len = VecDot(vec, vec);
   if (len < 1.0e-20) {
      vec[1] = vec[2] = 0.0;
      len = vec[0] = 1.0;
      //std::cout << "vec[0]=" << vec[0] << " vec[1]=" << vec[1] << " vec[2]=" << vec[2] << std::endl;
      }
   else {
      len = sqrt(len);//might have to change this to slen...
      scl = RECIPROCAL(len);
      VecScale( scl, vec);
      //std::cout << "vec[0]=" << vec[0] << " vec[1]=" << vec[1] << " vec[2]=" << vec[2] << std::endl;
      }
   return(len);
}
#endif

/** @brief Normalise a @c Vec in place and return its original length.
 *
 *  Degenerate input (squared length < 1e-20) is replaced with (1,0,0) and
 *  1.0 is returned.  Uses RECIPROCAL() for the scale factor.
 *  @param vec  Vector to normalise (reference).
 *  @return     The Euclidean length before normalisation; 1.0 on degenerate input.
 */
Flt VecNuNormalize(NuVec& vec) 
{
   Flt len, scl;
   len = VecDot(vec, vec);
   if (len < 1.0e-20) {
      vec[1] = vec[2] = 0.0;
      len = vec[0] = 1.0;
      //std::cout << "vec[0]=" << vec[0] << " vec[1]=" << vec[1] << " vec[2]=" << vec[2] << std::endl;
      }
   else {
      len = sqrt(len);//might have to change this to slen...
      scl = RECIPROCAL(len);
      VecScale( scl, vec);
      //std::cout << "vec[0]=" << vec[0] << " vec[1]=" << vec[1] << " vec[2]=" << vec[2] << std::endl;
      }
   return(len);
}


/** @brief Set all 16 elements of a 4x4 matrix to zero.
 *  @param result  Matrix to zero (modified in place).
 */
void MZero(Matrix result) //todo:change to C++ reference,removing C interface
{
   int i, j;
   //for (i = 0 ; i < 4 ; i++)
   //   for (j = 0 ; j < 4 ; j++)
   for (i=4;i--;)
     for (j=4;j--;)
         result[i][j] = 0.0;
}

/** @brief Set a 4x4 matrix to the identity (1 on diagonal, 0 elsewhere).
 *  @param result  Matrix to initialise (modified in place).
 */
void MIdentity(Matrix result) //todo:change to C++ reference,removing C interface
{
   for (int i=0;i<4;i++)
     for (int j=0;j<4;j++)
        if (i==j)
           result[i][j] = 1.0;
        else
           result[i][j] = 0.0;
}

/** @brief Copy a 4x4 matrix element-by-element.
 *  @param result  Destination matrix.
 *  @param src     Source matrix.
 */
static void MCopy(Matrix result, Matrix src) //todo:change to C++ reference,removing C interface
{
   int i, j;
   for (i=0;i<4;i++)
     for (j=0;j<4;j++)
       result[i][j]=src[i][j];
}
/** @brief Print a 4x4 matrix to stdout for debugging.
 *  @param m  Matrix to print; rows are written as space-separated @c %f values.
 */
static void MPrint(Matrix m)
{
   std::cout<<"Printing Matrix\n";
   int i, j;
   for (i=0;i<4;i++) {
       for (j = 0; j < 4; j++)
           std::cout << m[i][j] << " ";
   std::cout<<"\n";
   }
}

/** @brief Multiply two 4x4 matrices: result = a x b.
 *
 *  Uses a temporary matrix so that @p result may alias @p a or @p b.
 *  The active implementation is the scalar O(n^3) loop. An AVX-accelerated
 *  variant (myMultiply) is compiled in the @c #else branch but is currently
 *  disabled.
 *  @param result  Output matrix (may alias @p a or @p b).
 *  @param a       Left-hand matrix.
 *  @param b       Right-hand matrix.
 */
#if 1 //defined(__APPLE__)
void MTimes(Matrix result, Matrix a, Matrix b) //todo:change to C++ reference,removing C interface
{
  Matrix temp_matrix;
  //for (int i = 0 ; i < 4 ; i++)
  //   for (int j = 0 ; j < 4 ; j++) {
  for (int i = 4; i--;)
      for (int j = 4; j--;) {
          temp_matrix[i][j] = 0.0;
          //for (k = 0 ; k < 4 ; k++)
          for (int k = 4; k--;)
              temp_matrix[i][j] += a[i][k] * b[k][j];
      }

  //for (i = 0 ; i < 4 ; i++)
  //   for (j = 0 ; j < 4 ; j++)
  for (int i = 4; i--;)
      for (int j = 4; j--;)
          result[i][j] = temp_matrix[i][j];
  //printf("MTimes2:"); MPrint(result);
}
#else
//based on
//https://stackoverflow.com/questions/18499971/efficient-4x4-matrix-multiplication-c-vs-assembly/18508113#18508113
SuperMatrix myMultiply(SuperMatrix M1, SuperMatrix M2) {
    // Perform a 4x4 matrix multiply by a 4x4 matrix
    // Be sure to run in 64 bit mode and set right flags
    // Properties, C/C++, Enable Enhanced Instruction, /arch:AVX
    // Having MATRIX on a 32 byte bundry does help performance
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
	auto res=myMultiply(m1,m2);
	result=res.f;
}
#endif

/* end new code by CM */


/** @brief Element-wise matrix addition: result[i][j] = matrix1[i][j] + matrix2[i][j].
 *  @param result   Output matrix.
 *  @param matrix1  First operand.
 *  @param matrix2  Second operand.
 */
void MAdd(Matrix result, Matrix matrix1, Matrix matrix2) //todo:change to C++ reference,removing C interface
{
   int i, j;

   for (i=0;i<4;i++)
      for (j=0;j<4;j++)
         result[i][j] = matrix1[i][j] + matrix2[i][j];
}

/** @brief Element-wise matrix subtraction: result[i][j] = matrix1[i][j] - matrix2[i][j].
 *  @param result   Output matrix.
 *  @param matrix1  Minuend.
 *  @param matrix2  Subtrahend.
 */
void MSub(Matrix result, Matrix matrix1, Matrix matrix2) //todo:change to C++ reference,removing C interface
{
   int i, j;

   for (i=0;i<4;i++)
      for (j=0;j<4;j++)
         result[i][j] = matrix1[i][j] - matrix2[i][j];
}

/** @brief Scalar-multiply a 4x4 matrix: result[i][j] = size x matrix1[i][j].
 *  @param result   Output matrix.
 *  @param matrix1  Input matrix to scale.
 *  @param size     Scale factor applied to every element.
 */
void MScale(Matrix result, Matrix matrix1, Flt size) //todo:change to C++ reference,removing C interface
{
   int i, j;

   for (i=0;i<4;i++)
      for (j=0;j<4;j++)
         result[i][j] = size * matrix1[i][j];
   return;
}

/** @brief Transpose a 4x4 matrix via a temporary so @p result may alias @p matrix1.
 *  @param result   Output transposed matrix.
 *  @param matrix1  Input matrix.
 */
void MTranspose(Matrix result, Matrix matrix1) //todo:change to C++ reference,removing C interface
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

/** @brief Apply a Transform to an @c fVec point including the translation row.
 *
 *  Multiplies @p vec by the 3x4 upper portion of @c tx->matrix (rows 0-2 plus
 *  the translation in row 3).  If @p tx is NULL, @p vec is copied unchanged.
 *  @param out  Output transformed vector.
 *  @param vec  Input vector in object space.
 *  @param tx   Transform to apply; NULL means identity.
 */
void
fTxVec(fVec out, fVec vec, Transform *tx)
{      
   fVec result;

   if (tx == NULL) {
      VecCopy(vec, out);
      return;
      }
   Matrix* matrix = &tx->matrix;

   for (int i=0;i<3;i++)
      result[i] = vec[0] * (*matrix)[0][i] +
                  vec[1] * (*matrix)[1][i] +
                  vec[2] * (*matrix)[2][i] +
                           (*matrix)[3][i];
   for (int i=0;i<3;i++)
      out[i] = result[i];
}

/** @brief Apply a Transform to a @c Vec point including the translation row.
 *
 *  Multiplies @p vec by the 3x4 upper portion of @c tx->matrix.
 *  If @p tx is NULL, @p vec is copied unchanged.
 *  @param out  Output transformed vector.
 *  @param vec  Input vector in object space.
 *  @param tx   Transform to apply; NULL means identity.
 */
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

/** @brief Apply the inverse Transform to a @c Vec point (with translation).
 *
 *  Multiplies @p vec by the 3x4 upper portion of @c tx->inverse (which
 *  includes the translation column).  If @p tx is NULL, @p vec is copied.
 *  @param out  Output: @p vec transformed by the inverse.
 *  @param vec  Input vector in world space.
 *  @param tx   Transform whose inverse to apply; NULL means identity.
 */
void InvTxVec(Vec out, Vec vec, Transform *tx) //todo:change to C++ reference,removing C interface
{
   int i;
   Vec result;
   
#ifdef DEBUG_FN_CALLS
   //smessage("vector::InvTxVec\n");
   //crashes under google test !
#endif

   if (tx == NULL) {
      VecCopy(vec, out);
      return;
      }
   
   Matrix* matrix = &tx->inverse;

   for (i = 0; i < 3; i++) {
#ifdef DEBUG_FN_CALLS
       std::cout<<"matrix="<< (*matrix)[0][i] <<","<< 
           (*matrix)[1][i]<< ","<< (*matrix)[2][i] <<","<<
           (*matrix)[3][i] << "\n";
#endif
       result[i] = vec[0] * (*matrix)[0][i] +
           vec[1] * (*matrix)[1][i] +
           vec[2] * (*matrix)[2][i] +
           (*matrix)[3][i];
   }
   for (i=0;i<3;i++)
      out[i] = result[i];
}

/** @brief Apply the inverse Transform to a @c Vec point (with translation).
 *
 *  Multiplies @p vec by the 3x4 upper portion of @c tx->inverse (which
 *  includes the translation column).  If @p tx is NULL, @p vec is copied.
 *  @param out  Output: @p vec transformed by the inverse.
 *  @param vec  Input vector in world space.
 *  @param tx   Transform whose inverse to apply; NULL means identity.
 */
void InvTxVec_CPP(Vec& out, Vec vec, Transform& tx) 
{
   NuVec result;
   
#ifdef DEBUG_FN_CALLS
   //smessage("vector::InvTxVec\n");
   //crashes under google test !
#endif
   
   Matrix* matrix = &tx.inverse;

   for (int i = 0; i < 3; i++) {
#ifdef DEBUG_FN_CALLS
       std::cout<<"matrix="<< (*matrix)[0][i] <<","<< 
           (*matrix)[1][i]<< ","<< (*matrix)[2][i] <<","<<
           (*matrix)[3][i] << "\n";
#endif
       result[i] = vec[0] * (*matrix)[0][i] +
           vec[1] * (*matrix)[1][i] +
           vec[2] * (*matrix)[2][i] +
           (*matrix)[3][i];
   }
   for (int i=0;i<3;i++)
      out[i] = result[i];
}

/* This routine is called so obscenely often, it just *has* to be
   optimized.  We can optimize away the "out" stuff for the case that
   out != vec, and the other cases should be rewritten to meet this
   case, too. Rendering discs.pi spent only more time in eval_node ! */

/** @brief Optimised inverse-transform of a point (C++ reference output, with translation).
 *
 *  Unrolled version of InvTxVec() that writes directly to @p out without a
 *  temporary, making it suitable for high-frequency call sites.  If @p tx is
 *  @c nullptr, @p vec is copied to @p out.
 *  @param out  Output reference: @p vec transformed by @c tx->inverse.
 *  @param vec  Input vector in world space.
 *  @param tx   Transform pointer; nullptr means identity.
 */
void InvTxVec1(Vec& out, Vec vec, Transform *tx)
{
   if (tx == nullptr) {
      VecCopy(vec, out);
      return;
      }

   Matrix *matrix = &tx->inverse;

   out[0] = vec[0] * (*matrix)[0][0] + vec[1] * (*matrix)[1][0] +
            vec[2] * (*matrix)[2][0] + (*matrix)[3][0];
   out[1] = vec[0] * (*matrix)[0][1] + vec[1] * (*matrix)[1][1] +
            vec[2] * (*matrix)[2][1] + (*matrix)[3][1];
   out[2] = vec[0] * (*matrix)[0][2] + vec[1] * (*matrix)[1][2] +
            vec[2] * (*matrix)[2][2] + (*matrix)[3][2];
}

/** @brief C++ reference variant of InvTxVec1 - inverse-transform a point (no NULL guard).
 *
 *  Identical computation to InvTxVec1() but takes the Transform by reference
 *  (guaranteed non-null) so the null-check branch is eliminated.
 *  @param out  Output reference: @p vec transformed by @c tx.inverse.
 *  @param vec  Input vector in world space.
 *  @param tx   Transform whose inverse to apply.
 */
void C_InvTxVector1(Vec& out, Vec vec, Transform& tx)
{
    Matrix* matrix = &tx.inverse;

    //if (tx == NULL) {
    //    VecCopy(vec, out);
    //    return;
    //}

    out[0] = vec[0] * (*matrix)[0][0] + vec[1] * (*matrix)[1][0] +
        vec[2] * (*matrix)[2][0] + (*matrix)[3][0];
    out[1] = vec[0] * (*matrix)[0][1] + vec[1] * (*matrix)[1][1] +
        vec[2] * (*matrix)[2][1] + (*matrix)[3][1];
    out[2] = vec[0] * (*matrix)[0][2] + vec[1] * (*matrix)[1][2] +
        vec[2] * (*matrix)[2][2] + (*matrix)[3][2];
}


/** @brief Apply the linear (no-translation) part of a Transform to a @c Vec.
 *
 *  Uses only the upper-left 3x3 submatrix of @c tx->matrix; the translation
 *  row is ignored.  Suitable for transforming direction vectors.
 *  If @p tx is NULL, @p vec is copied unchanged.
 *  @param out  Output transformed direction.
 *  @param vec  Input direction in object space.
 *  @param tx   Transform to apply; NULL means identity.
 */
void TxVec3(Vec out, Vec vec, Transform *tx)
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

/** @brief C++ reference variant of TxVec3 - linear transform only, no NULL guard.
 *
 *  Uses the upper-left 3x3 submatrix of @c tx.matrix (no translation).
 *  @param out  Output reference: transformed direction.
 *  @param vec  Input direction in object space.
 *  @param tx   Transform to apply (taken by reference; must be valid).
 */
void C_TxVector3(Vec& out, Vec vec, Transform& tx)
{
   int i;
   Matrix *matrix = &tx.matrix;
   Vec result;

   //if (tx == NULL) {
   //   VecCopy(vec, out);
   //   return;
   //   }

   for (i=0;i<3;i++)
      result[i] = vec[0] * (*matrix)[0][i] +
                  vec[1] * (*matrix)[1][i] +
                  vec[2] * (*matrix)[2][i];
   for (i=0;i<3;i++)
      out[i] = result[i];
}

/** @brief Apply the linear (no-translation) inverse Transform to a @c Vec.
 *
 *  Uses only the upper-left 3x3 submatrix of @c tx->inverse; the translation
 *  row is ignored.  Suitable for direction vectors in inverse space.
 *  If @p tx is NULL, @p vec is copied unchanged.
 *  @param out  Output transformed direction.
 *  @param vec  Input direction in world space.
 *  @param tx   Transform whose linear inverse to apply; NULL means identity.
 */
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

/** @brief Transform an @c fVec surface normal using the inverse-transpose rule.
 *
 *  Multiplies @p vec by the transpose of @c tx->inverse (= columns of the
 *  inverse, not the rows), which is the correct way to transform normals when
 *  the transform includes non-uniform scaling.  If @p tx is NULL the normal
 *  is copied unchanged.
 *  @param out  Output: transformed normal (not yet normalised).
 *  @param vec  Input surface normal in object space.
 *  @param tx   Transform to apply; NULL means identity.
 */
NuVec fTxNormal(fVec vec, Transform *tx)
{
   int i;
   Matrix *matrix = &tx->inverse;
   NuVec result;

   if (tx == nullptr) {
      VecCopy(vec, result);
      return result;
      }

   for (i=0;i<3;i++)
      result[i] = vec[0] * (*matrix)[i][0] +
                  vec[1] * (*matrix)[i][1] +
                  vec[2] * (*matrix)[i][2];
   return result;
}

/** @brief Transform a @c Vec surface normal using the inverse-transpose rule.
 *
 *  Multiplies @p vec by the columns of @c tx->inverse (equivalent to multiplying
 *  by the transpose of the inverse, which preserves perpendicularity under
 *  non-uniform scaling).  If @p tx is NULL the normal is copied unchanged.
 *  @param out  Output: transformed normal (not yet normalised).
 *  @param vec  Input surface normal in object space.
 *  @param tx   Transform to apply; NULL means identity.
 */
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

/** @brief Transform a @c Vec normal by the transpose of @c tx->matrix.
 *
 *  Used when going in the inverse direction: multiplies @p vec by the columns
 *  of @c tx->matrix (= transpose of the forward matrix).  If @p tx is NULL
 *  the normal is copied unchanged.
 *  @param out  Output: transformed normal (not yet normalised).
 *  @param vec  Input normal in world space.
 *  @param tx   Transform to apply; NULL means identity.
 */
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

/** @brief Build a non-uniform scaling Transform.
 *
 *  Sets the diagonal of @c tx->matrix to @p vector and the diagonal of
 *  @c tx->inverse to the reciprocals (PLY_HUGE for a zero component).
 *  @param tx      Output Transform to populate.
 *  @param vector  Scale factors for X, Y, Z axes.
 */
void Get_Scaling_Transformation(Transform *tx, Vec vector)
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

/** @brief Build a non-uniform scaling Transform.
 *
 *  Sets the diagonal of @c tx->matrix to @p vector and the diagonal of
 *  @c tx->inverse to the reciprocals (PLY_HUGE for a zero component).
 *  @param tx      Output Transform to populate.
 *  @param vector  Scale factors for X, Y, Z axes.
 */
void Get_Scaling_Transformation_CPP(Transform& tx, NuVec vector)
{
   MIdentity(tx.matrix);
   tx.matrix[0][0] = vector[0];
   tx.matrix[1][1] = vector[1];
   tx.matrix[2][2] = vector[2];

   MIdentity(tx.inverse);
   tx.inverse[0][0] = (vector[0] == 0.0 ? PLY_HUGE : 1.0 / vector[0]);
   tx.inverse[1][1] = (vector[1] == 0.0 ? PLY_HUGE : 1.0 / vector[1]);
   tx.inverse[2][2] = (vector[2] == 0.0 ? PLY_HUGE : 1.0 / vector[2]);
}


/** @brief Build a perspective-divide Transform for a given focal distance.
 *
 *  Constructs the matrix that performs the classic (z / (1+dist)) perspective
 *  projection.  The inverse undoes the divide.
 *  @param result  Output Transform to populate.
 *  @param dist    Perspective distance parameter; larger values give a weaker effect.
 */
void Get_Perspective_Transformation(Transform *result, Flt dist)
{
   MIdentity(result->matrix);
   result->matrix[2][2] =  1.0 / (1.0 + dist);
   result->matrix[3][2] =  dist / (1.0 + dist);
   result->matrix[2][3] =  1.0;
   result->matrix[3][3] =  0.0;
   MIdentity(result->inverse);
   result->inverse[2][3] = dist;
}

/** @brief Build a perspective-divide Transform for a given focal distance.
 *
 *  Constructs the matrix that performs the classic (z / (1+dist)) perspective
 *  projection.  The inverse undoes the divide.
 *  @param result  Output Transform to populate.
 *  @param dist    Perspective distance parameter; larger values give a weaker effect.
 */
void Get_Perspective_Transformation_CPP(Transform& result, Flt dist)
{
   MIdentity(result.matrix);
   result.matrix[2][2] =  1.0 / (1.0 + dist);
   result.matrix[3][2] =  dist / (1.0 + dist);
   result.matrix[2][3] =  1.0;
   result.matrix[3][3] =  0.0;
   MIdentity(result.inverse);
   result.inverse[2][3] = dist;
}

/** @brief Build a translation Transform from a @c NuVec offset (C++ overload).
 *
 *  Sets @c tx->matrix[3][0..2] to @p vector and @c tx->inverse[3][0..2] to
 *  its negation, with both 4x4 matrices otherwise being identity.
 *  @param tx      Output Transform to populate.
 *  @param vector  Input Translation offset as a @c NuVec (std::array<double,3>).
 */
void Get_Translation_TransformationCPP(Transform& tx, const NuVec vector)
{
#ifdef DEBUG_FN_CALLS
    //smessage("vector::Get_Translation_Transformation\n");
#endif
    MIdentity(tx.matrix);
    tx.matrix[3][0] = vector[0];
    tx.matrix[3][1] = vector[1];
    tx.matrix[3][2] = vector[2];
 #ifdef DEBUG_FN_CALLS
    std::cout << "gtt4\n" << std::flush;
    MPrint(tx.matrix);
    std::cout << "gtt5\n" << std::flush;
#endif
    MIdentity(tx.inverse);
    tx.inverse[3][0] = 0.0 - vector[0];
    tx.inverse[3][1] = 0.0 - vector[1];
    tx.inverse[3][2] = 0.0 - vector[2];
#ifdef DEBUG_FN_CALLS
    std::cout << "call to MPrint\n" << std::flush;
    MPrint(tx.matrix);
#endif
}

/** @brief Build a translation Transform from a @c Vec offset.
 *
 *  Sets @c tx->matrix[3][0..2] to @p vector and @c tx->inverse[3][0..2] to
 *  its negation, with both 4x4 matrices otherwise being identity.
 *  @param tx      Output Transform to populate.
 *  @param vector  Translation offset as a @c Vec (double[3]).
 */
void Get_Translation_Transformation(Transform *tx, Vec vector)
{
#ifdef DEBUG_FN_CALLS
    //smessage("vector::Get_Translation_Transformation\n");
#endif
   MIdentity(tx->matrix);
   tx->matrix[3][0] = vector[0];
   tx->matrix[3][1] = vector[1];
   tx->matrix[3][2] = vector[2];
#ifdef DEBUG_FN_CALLS
   MPrint(tx->matrix);
#endif
   MIdentity(tx->inverse);
   tx->inverse[3][0] = 0.0 - vector[0];
   tx->inverse[3][1] = 0.0 - vector[1];
   tx->inverse[3][2] = 0.0 - vector[2];
#ifdef DEBUG_FN_CALLS
   std::cout << "Bcalling MPrint\n" << std::flush;
   MPrint(tx->matrix);
#endif
}

/** @brief Build a translation Transform from a @c NuVec offset.
 *
 *  Sets @c tx.matrix[3][0..2] to @p vector and @c tx->inverse[3][0..2] to
 *  its negation, with both 4x4 matrices otherwise being identity.
 *  @param tx      Output Transform to populate.
 *  @param vector  Translation offset as a @c NuVec (double[3]).
 */
void Get_Translation_Transformation_CPP(Transform& tx, NuVec vector)
{
#ifdef DEBUG_FN_CALLS
    //smessage("vector::Get_Translation_Transformation_CPP\n");
#endif
   MIdentity(tx.matrix);
   tx.matrix[3][0] = vector[0];
   tx.matrix[3][1] = vector[1];
   tx.matrix[3][2] = vector[2];
#ifdef DEBUG_FN_CALLS
   MPrint(tx.matrix);
#endif
   MIdentity(tx.inverse);
   tx.inverse[3][0] = 0.0 - vector[0];
   tx.inverse[3][1] = 0.0 - vector[1];
   tx.inverse[3][2] = 0.0 - vector[2];
#ifdef DEBUG_FN_CALLS
   std::cout << "Bcalling MPrint\n" << std::flush;
   MPrint(tx.matrix);
#endif
}

/** @brief Build a combined Euler rotation Transform (Rx x Ry x Rz).
 *
 *  Applies rotations in X, Y, then Z order using the angles (in radians)
 *  in @p vector[0..2].  The inverse is constructed as the transpose of each
 *  individual rotation, composed in reverse order, matching the orthogonality
 *  property of rotation matrices.
 *  @param tx      Output Transform to populate.
 *  @param vector  Rotation angles in radians: [0]=X, [1]=Y, [2]=Z.
 */
void Get_Rotation_Transformation(Transform *tx, Vec vector)
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

/** @brief Build a combined Euler rotation Transform (Rx x Ry x Rz).
 *
 *  Applies rotations in X, Y, then Z order using the angles (in radians)
 *  in @p vector[0..2].  The inverse is constructed as the transpose of each
 *  individual rotation, composed in reverse order, matching the orthogonality
 *  property of rotation matrices.
 *  @param tx      Output Transform to populate.
 *  @param vector  Rotation angles in radians: [0]=X, [1]=Y, [2]=Z.
 */
void Get_Rotation_Transformation_CPP(Transform& tx, NuVec vector)
{
    Matrix matrix;
    Flt cosx, cosy, cosz, sinx, siny, sinz;

    MIdentity(tx.matrix);
    cosx = cos(vector[0]);
    sinx = sin(vector[0]);
    cosy = cos(vector[1]);
    siny = sin(vector[1]);
    cosz = cos(vector[2]);
    sinz = sin(vector[2]);

    tx.matrix[1][1] = cosx;
    tx.matrix[2][2] = cosx;
    tx.matrix[1][2] = sinx;
    tx.matrix[2][1] = -sinx;
    MTranspose(tx.inverse, tx.matrix);

    MIdentity(matrix);
    matrix[0][0] = cosy;
    matrix[2][2] = cosy;
    matrix[0][2] = -siny;
    matrix[2][0] = siny;
    MTimes(tx.matrix, tx.matrix, matrix);
    MTranspose(matrix, matrix);
    MTimes(tx.inverse, matrix, tx.inverse);

    MIdentity(matrix);
    matrix[0][0] = cosz;
    matrix[1][1] = cosz;
    matrix[0][1] = sinz;
    matrix[1][0] = -sinz;
    MTimes(tx.matrix, tx.matrix, matrix);
    MTranspose(matrix, matrix);
    MTimes(tx.inverse, matrix, tx.inverse);
}

/** @brief Build a 3D shear Transform.
 *
 *  Sets the six off-diagonal shear coefficients in @c result->matrix and
 *  their negations in @c result->inverse (valid for small shears where the
 *  approximation inverse approx -shear holds).
 *  @param result  Output Transform to populate.
 *  @param xy  X shift per unit Y.  @param xz  X shift per unit Z.
 *  @param yx  Y shift per unit X.  @param yz  Y shift per unit Z.
 *  @param zx  Z shift per unit X.  @param zy  Z shift per unit Y.
 */
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

/** @brief Compose two Transforms in place: tx0 = tx0 x tx1.
 *
 *  Updates @c tx0->matrix = tx0->matrix x tx1->matrix and
 *  @c tx0->inverse = tx1->inverse x tx0->inverse (reverse order for inverses).
 *  @param tx0  Left-hand transform; updated in place with the composition.
 *  @param tx1  Right-hand transform to post-multiply.
 */
void Compose_Transformations(Transform& tx0, Transform &tx1)
{
   MTimes(tx0.matrix,  tx0.matrix,  tx1.matrix);
   MTimes(tx0.inverse, tx1.inverse, tx0.inverse);
}

/** @brief Allocate and return a new identity Transform via FactoryTransform().
 *
 *  Both @c matrix and @c inverse are initialised to the identity.
 *  Calls serror() if allocation fails.
 *  @return  Pointer to a heap-allocated identity Transform; owned by the caller.
 */
std::unique_ptr<Transform> Get_Transformation()
{
  auto tx = FactoryTransform();
  MIdentity(tx->matrix);
  MIdentity(tx->inverse);
  return tx;
}

/** @brief Build a rotation Transform about an arbitrary axis by a given angle.
 *
 *  Uses Rodrigues' rotation formula from Faux & Pratt,
 *  "Computational Geometry for Design and Manufacture".
 *  @p V is normalised in place before use; a zero vector may produce bad results.
 *  The inverse is the transpose (rotation matrices are orthogonal).
 *  @param trans  Output Transform to populate.
 *  @param V      Rotation axis (need not be unit length; normalised in place).
 *  @param angle  Rotation angle in radians.
 */
void Get_Rotate_Transform(Transform *trans, Vec V, Flt angle)
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

/** @brief Build a rotation Transform about an arbitrary axis by a given angle.
 *
 *  Uses Rodrigues' rotation formula from Faux & Pratt,
 *  "Computational Geometry for Design and Manufacture".
 *  @p V is normalised in place before use; a zero vector may produce bad results.
 *  The inverse is the transpose (rotation matrices are orthogonal).
 *  @param trans  Output Transform to populate.
 *  @param V      Rotation axis (need not be unit length; normalised in place).
 *  @param angle  Rotation angle in radians.
 */
void Get_Rotate_Transform_CPP(Transform& trans, NuVec& V, Flt angle)
{
   Flt cosx, sinx;

   (void)VecNuNormalize(V); /* Note: bad values (zero vector) can occur here */
   MIdentity(trans.matrix);
   cosx = cos(angle);
   sinx = sin(angle);
   trans.matrix[0][0] = V[0] * V[0] + cosx * (1.0 - V[0] * V[0]);
   trans.matrix[0][1] = V[0] * V[1] * (1.0 - cosx) + V[2] * sinx;
   trans.matrix[0][2] = V[0] * V[2] * (1.0 - cosx) - V[1] * sinx;
   trans.matrix[1][0] = V[0] * V[1] * (1.0 - cosx) - V[2] * sinx;
   trans.matrix[1][1] = V[1] * V[1] + cosx * (1.0 - V[1] * V[1]);
   trans.matrix[1][2] = V[1] * V[2] * (1.0 - cosx) + V[0] * sinx;
   trans.matrix[2][0] = V[0] * V[2] * (1.0 - cosx) + V[1] * sinx;
   trans.matrix[2][1] = V[1] * V[2] * (1.0 - cosx) - V[0] * sinx;
   trans.matrix[2][2] = V[2] * V[2] + cosx * (1.0 - V[2] * V[2]);
   MTranspose(trans.inverse, trans.matrix);
}

/** @brief Build the canonical-coordinate Transform for a cylinder/cone/paraboloid.
 *
 *  Composes translation (origin to world origin), axis-alignment rotation,
 *  and non-uniform scaling (1/r in XY, 1/len in Z) so that the primitive's
 *  axis maps to the +Z axis with unit radius and unit length.
 *  @param trans   Output Transform to populate.
 *  @param origin  Base-centre point in world space (translated to origin).
 *  @param up      Axis direction (normalised internally via Get_Rotate_Transform).
 *  @param r       Base radius; controls the XY scale factor (1/r).
 *  @param len     Axis length; controls the Z scale factor (1/len).
 */
void Get_Coordinate_TransformCPP(Transform& trans, Vec& origin, NuVec& up, Flt r, Flt len)
{
   Transform trans2;   
#ifdef DEBUG_FN_CALLS
   std::cout << "gct0-debug\n" << std::flush;
   //smessage("vector::Get_Coordinate_TransformCPP\n");     
#endif
   NuVec norigin;
   NuVecCopy(origin, norigin);   
   Get_Translation_TransformationCPP(trans, norigin);
#ifdef DEBUG_FN_CALLS
   MPrint(trans.matrix);
#endif
   NuVec tmpv;
   if (fabs(up[2]) == 1.0) {
      MakeVector(1.0, 0.0, 0.0, tmpv);
      }
   else {
      MakeVector(-up[1], up[0], 0.0, tmpv);
      }
   Get_Rotate_Transform_CPP(trans2, tmpv, -acos(up[2]));
   MPrint(trans2.matrix);
   Compose_Transformations(trans, trans2);
#ifdef DEBUG_FN_CALLS
   MPrint(trans.matrix);
#endif
   MakeVector(1/r, 1/r, 1/len, tmpv);
   Get_Scaling_Transformation_CPP(trans2, tmpv);
#ifdef DEBUG_FN_CALLS
   MPrint(trans2.matrix);
#endif
   Compose_Transformations(trans, trans2);
#ifdef DEBUG_FN_CALLS
   MPrint(trans.matrix);
#endif
}

/** @brief Compute the full camera-to-screen Transform for a Viewpoint.
 *
 *  Composes five successive transforms in order:
 *  1. Translation to move the eye to the origin.
 *  2. Orthogonal basis rotation aligning the view direction with +Z and
 *     the up vector with +Y (degenerate cases are handled).
 *  3. Perspective divide using a very small focal distance (1e-5).
 *  4. Asymmetric scale mapping the frustum to pixel coordinates.
 *  5. Translation to the screen centre.
 *
 *  @param eye  Active Viewpoint supplying from/at/up vectors, hither/yon, and resolution.
 *  @return     Heap-allocated composed Transform; owned by the caller.
 */
std::unique_ptr<Transform> Normalize_View(Viewpoint *eye)
{
   long xs, ys;
   Flt d, XScale, YScale;
   NuVec Va, right;
   auto Tv = Get_Transformation();
   Transform Tt;

   /* Move everything so that the eye is at <0,0,0> */
   VecCopy(eye->view_from, Va)
   VecNegate(Va)
    //printf("Normalize_View: Va = <%g,%g,%g>\n", Va[0], Va[1], Va[2]);
   Get_Translation_TransformationCPP(*Tv, Va);
   //std::cout << "returned from GTT\n" << std::flush;

   VecSub(eye->view_at, eye->view_from, Va);
   /* Make sure this is a valid sort of setup */
   if (Va[0] == 0.0 && Va[1] == 0.0 && Va[2] == 0.0) {
      swarning("Eye position is same as point of interest\n");
      Va[2] = 1;  /* Pick an arbitrary direction to use */
      }
   /* Get the up vector to be perpendicular to the view vector */
   d = VecNuNormalize(Va);
   //printf("Normalized2: Va = <%g,%g,%g>\n", Va[0], Va[1], Va[2]);

   if ((fabs(Va[0]+eye->view_up[0]) < PLY_EPSILON &&
        fabs(Va[1]+eye->view_up[1]) < PLY_EPSILON &&
        fabs(Va[2]+eye->view_up[2]) < PLY_EPSILON) ||
       (fabs(Va[0]-eye->view_up[0]) < PLY_EPSILON &&
        fabs(Va[1]-eye->view_up[1]) < PLY_EPSILON &&
        fabs(Va[2]-eye->view_up[2]) < PLY_EPSILON)) {
      /* View and up are either the same or 180 degrees off. Need to
         correct. */
      if (fabs(Va[0]) > 0.0)
         MakeVector(0, 1, 0, right)
      else
         MakeVector(1, 0, 0, right)
      }
   else {
      VecCross(eye->view_up, Va, right);
      VecNuNormalize(right);
      }

#ifdef DEBUG_FN_CALLS
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

#ifdef DEBUG_FN_CALLS
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
   Compose_Transformations(*Tv, Tt);

   /* Now add in the perspective transformation */
   /* Get_Perspective_Transformation(&Tt, eye->view_hither); */
   Get_Perspective_Transformation(&Tt, 1.0e-5);
   Compose_Transformations(*Tv, Tt);

   /* Determine how much to scale things by */
   YScale = 0.5 * (Flt)eye->view_yres / tan(eye->view_angle);
   XScale = YScale * (Flt)eye->view_xres /
                       ((Flt)eye->view_yres * eye->view_aspect);
   MakeVector(XScale, -YScale, 1.0, Va);
   Get_Scaling_Transformation_CPP(Tt, Va);
   Compose_Transformations(*Tv, Tt);

   /* Now translate to the center of the screen */
   xs = eye->view_xres / 2;
   ys = eye->view_yres / 2;
   MakeVector(xs, ys, 0.0, Va);
   Get_Translation_Transformation_CPP(Tt, Va);
   Compose_Transformations(*Tv, Tt);
   


   return Tv;
}

/** @brief Convert geocentric (spherical) coordinates to Cartesian.
 *
 *  Input @p Q encodes (longitude theta, latitude phi, radius r) as Q[0..2].
 *  Output @p P = (r*cos(theta)*cos(phi), r*sin(phi), r*sin(theta)*cos(phi)).
 *  @param Q  Input: (theta, phi, r) - longitude, latitude, radius.
 *  @param P  Output: Cartesian (x, y, z).
 */
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

/** @brief Convert Cartesian coordinates to geocentric (spherical).
 *
 *  Output @p Q = (theta, phi, r): longitude via atan2(z, x), latitude via
 *  asin(y/r), and radius @c r = |P|.  Returns (0,0,0) for the zero vector.
 *  @param P  Input: Cartesian (x, y, z).
 *  @param Q  Output: (theta, phi, r) - longitude, latitude, radius.
 */
void
cartesian_to_geocentric(Vec P, Vec Q)
{
   Flt len;

   len = VecLen(P);
   Q[2] = len;
   if (len < PLY_EPSILON) {
      MakeVector(0, 0, 0, Q);
      return;
      }

   /* Determine the angles */
   Q[1] = asin(P[1] / len);
   if (fabs(Q[1]) + PLY_EPSILON > PYM_PI_2)
      Q[0] = 0.0;
   else {
      Q[0] = atan2(P[2], P[0]);
#if 0
      if (Q[0] < 0 && Q[0] > -PLY_EPSILON)
         Q[0] = 0.0;
#endif
      }
}

/** @brief Convert cylindrical coordinates (theta, z, r) to Cartesian (x, y, z).
 *
 *  Input @p Q = (theta, z, r); output @p P = (r*cos(theta), z, r*sin(theta)).
 *  Note the Y axis carries the axial coordinate.
 *  @param Q  Input: (theta, z, r) - azimuth, height, radius.
 *  @param P  Output: Cartesian (x, y, z).
 */
void
cylindrical_to_cartesian(Vec Q, Vec P)
{
   P[0] = Q[2] * cos(Q[0]);
   P[2] = Q[2] * sin(Q[0]);
   P[1] = Q[1];
}

/** @brief Convert Cartesian coordinates (x, y, z) to cylindrical (theta, z, r).
 *
 *  Output @p Q = (theta, z, r): azimuth theta = atan2(z, x), height z = y,
 *  and radius r = sqrt(x^2+z^2). theta = 0 when the XZ radius is below PLY_EPSILON.
 *  @param P  Input: Cartesian (x, y, z).
 *  @param Q  Output: (theta, z, r) - azimuth, height, radius.
 */
void
cartesian_to_cylindrical(Vec P, Vec Q)
{
   Q[1] = P[1];
   Q[2] = sqrt(P[0] * P[0] + P[2] * P[2]);
   if (Q[2] < PLY_EPSILON)
      Q[0] = 0.0;
   else
      Q[0] = atan2(P[2], P[0]);
}

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
Flt OLDpolyray_random(void)
{
   return (Flt)urandom() / 65535.0;
}

/* Return a uniform random value in [0, 1] */
Flt polyray_random(void)
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

Flt
VecNormalize(Vec vec)
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

void
MTimes(Matrix result, Matrix matrix1, Matrix matrix2)
{
   int i, j, k;
   Matrix temp_matrix;

   //for (i = 0 ; i < 4 ; i++)
   //   for (j = 0 ; j < 4 ; j++) {
     for (i=4;i--;)
       for (j=4;j--;) {
         temp_matrix[i][j] = 0.0;
         //for (k = 0 ; k < 4 ; k++)
         for (k=4;k--;)
            temp_matrix[i][j] += matrix1[i][k] * matrix2[k][j];
         }

   //for (i = 0 ; i < 4 ; i++)
   //   for (j = 0 ; j < 4 ; j++)
   for (i=4;i--;)
     for (j=4;j--;)
         result[i][j] = temp_matrix[i][j];
}


/* CM: Strassen's algorithm to multiply two 2x2 matrices, from
http://csg-www.lcs.mit.edu:8001/monsoon/tr40/subsection3_5_1.html
Uses 7 multiplications rather than 8
*/

#define MATA(i,j) (matrix1[i-1][j-1])
#define MATB(i,j) (matrix2[i-1][j-1])

void
MStrassen2(Matrix2 result, Matrix2 matrix1, Matrix2 matrix2)
{
  Flt p1,p2,p3,p4,p5,p6,p7;
  p1 = (MATA(1,1)+MATA(2,2))*(MATB(1,1)+MATB(2,2));
  p2 = (MATA(2,1)+MATA(2,2))*MATB(1,1);
  p3 = MATA(1,1)*(MATB(1,2)-MATB(2,2));
  p4 = MATA(2,2)*(MATB(2,1)-MATB(1,1));
  p5 = (MATA(1,1)+MATA(1,2))*MATB(2,2);
  p6 = (MATA(2,1)-MATA(1,1))*(MATB(1,1)+MATB(1,2));
  p7 = (MATA(1,2)-MATA(2,2))*(MATB(2,1)+MATB(2,2));
  result[1-1][1-1] = p1+p4-p5+p7;
  result[1-1][2-1] = p3+p5;
  result[2-1][1-1] = p2+p4;
  result[2-1][2-1] = p1+p3-p2+p6;

}



#define MAT2ADD(r,i,j) {(r)[0][0]=(i)[0][0]+(j)[0][0];(r)[0][1]=(i)[0][1]+(j)[0][1];(r)[1][0]=(i)[1][0]+(j)[1][0];(r)[1][1]=(i)[1][1]+(j)[1][1];}
#define MAT2SUB(r,i,j) {(r)[0][0]=(i)[0][0]-(j)[0][0];(r)[0][1]=(i)[0][1]-(j)[0][1];(r)[1][0]=(i)[1][0]-(j)[1][0];(r)[1][1]=(i)[1][1]-(j)[1][1];}
#define MAT2MUL(r,i,j) {MStrassen2((r),(i),(j));}


void
MStrassen2Mat(Matrix2 C11,Matrix2 C12,Matrix2 C21,Matrix2 C22, Matrix2 a11,Matrix2 a12,Matrix2 a21,Matrix2 a22 , Matrix2 b11,Matrix2 b12,Matrix2 b21,Matrix2 b22)
{
  Matrix2 p1,p2,p3,p4,p5,p6,p7,temp,temp2; //,C11,C12,C21,C22;

 // work out p1..p7
 MAT2ADD(temp,a11,a22);
 MAT2ADD(temp2,b11,b22);
 MAT2MUL(p1,temp,temp2);

 MAT2ADD(temp,a21,a22);
 MAT2MUL(p2,temp,b11);

 MAT2SUB(temp,b12,b22);
 MAT2MUL(p3,a11,temp);

 MAT2SUB(temp,b21,b11);
 MAT2MUL(p4,a22,temp);


 MAT2ADD(temp,a11,a12);
 MAT2MUL(p5,temp,b22);

 MAT2SUB(temp,a21,a11);
 MAT2ADD(temp2,b11,b12);
 MAT2MUL(p6,temp,temp2);

 MAT2SUB(temp,a12,a22);
 MAT2ADD(temp2,b21,b22);
 MAT2MUL(p7,temp,temp2);



 // work out C11..C22
 MAT2ADD(temp,p1,p4);
 MAT2SUB(temp2,temp,p5);
 MAT2ADD(C11,temp2,p7);

 MAT2ADD(C21,p2,p4);

 MAT2ADD(C12,p3,p5);

 MAT2ADD(temp,p1,p3);
 MAT2SUB(temp2,temp,p2);
 MAT2ADD(C22,temp2,p6);







 // now put values in C11..C22 into return MMatrix2

//=C11
// memcpy(result[1-1][1-1],C11,sizeof(Matrix2));
//C12
// memcpy(result[1-1][2-1],C12,sizeof(Matrix2));
//C21
// memcpy(result[2-1][1-1],C21,sizeof(Matrix2));
//C22
//memcpy(result[2-1][2-1],C22,sizeof(Matrix2));

}


/*
function C = strass(A,B,n0)
n = rows(A)
if n<=n0 then C=AB
else
  m=n/2;u=1:m;v=m+1:n; (1:n means loop from 1 to n !!)
  p1=strass(A(u,u)+A(v,v),B(u,u)+B(v,v),n0)
  p2=strass(A(v,u)+A(v,v)

*/

/* n=4. so m=2;u loops 1 to 2 */

#define SETMAT2(m,p,q,r,s) {(m)[0][0]=(p);(m)[0][1]=(q);(m)[1][0]=(r);(m)[1][1]=(s);}

// #define PLUGVALUES(m,a) {SETMAT2((m),(a)[0],(a)[1],(a)[2],(a)[3]);}




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

   for (i=0;i<3;i++) {
      result[i] = vec[0] * (*matrix)[0][i] +
                  vec[1] * (*matrix)[1][i] +
                  vec[2] * (*matrix)[2][i] +
                           (*matrix)[3][i];
   }
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

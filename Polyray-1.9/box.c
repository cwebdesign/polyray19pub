/* bezier.c

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
#include "memory.h"
#include "io.h"
#include "intersec.h"
#include "symtab.h"
#include "scan.h"
#include "vector.h"
#include "bound.h"
#include "box.h"

/* This is a placeholder for primitive data */
typedef struct t_boxdata {
   Flt bounds[2][3];
} BoxData ;

/* Prototypes for this module */
static void BoxNormal(Flt bounds[2][3], Vec P, Vec N);

/* Prototypes for the primitive operators */
void BoxRender(Viewpoint *, BinTree *, Object *);
int BoxIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int BoxInside(Object *, Vec);

ObjectProcs BoxProcs = {
   BoxRender,
   NULL,
   GenericInitialize,
   BoxIntersect,
   BoxInside,
   GenericCopy,
   GenericDelete,
} ;

static void
BoxNormal(Flt bounds[2][3], Vec P, Vec N)
{
   MakeVector(0, 0, 0, N);
        if (equal(P[0], bounds[1][0]))
      N[0] =  1.0;
   else if (equal(P[0], bounds[0][0]))
      N[0] = -1.0;
   else if (equal(P[1], bounds[1][1]))
      N[1] = 1.0;
   else if (equal(P[1], bounds[0][1]))
      N[1] = -1.0;
   else if (equal(P[2], bounds[1][2]))
      N[2] = 1.0;
   else if (equal(P[2], bounds[0][2]))
      N[2] = -1.0;
   else {
      MakeVector(0, 1, 0, N);
      }
}

int
BoxIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
             Flt mindist, Flt maxdist, Isect *hit)
{
   Vec P, N;
   Flt tmin = mindist;
   Flt tmax = maxdist;
   int Flag = 0;
   BoxData *box = (BoxData *)obj->o_data;

   if (determine_start(ray->P, ray->D, box->bounds, &tmin, &tmax)) {
      /* There will be a hit at tmin and tmax. */
      if (tmin > mindist) {
         VecAddScaled(ray->P, tmin, ray->D, P);
         BoxNormal(box->bounds, P, N);
         Insert_Hit(obj, P, N, tmin, P, hit);
         Flag = 1;
         }
      if (tmax < maxdist) {
         VecAddScaled(ray->P, tmax, ray->D, P);
         BoxNormal(box->bounds, P, N);
         Insert_Hit(obj, P, N, tmax, P, hit);
         Flag = 1;
         }
      }
   return Flag;
}

int
BoxInside(Object *obj, Vec P)
{
   int i;
   BoxData *box = (BoxData *)obj->o_data;
   Vec PP;

   /* Transform the ray into the boxes space */
   InvTxVector1(PP, P, obj->o_trans);
   for (i=0;i<3;i++)
      if (PP[i] < box->bounds[0][i] || PP[i] > box->bounds[1][i])
         return 0;
   return 1;
}

Object *
MakeBox(Object *object, Vec v1, Vec v2)
{
   BoxData *box;
   Vec size;
   int i;

   object->o_type = T_BOX;
   object->o_procs = &BoxProcs ;

   VecSub(v1, v2, size);

   if (size[0] == 0.0 || size[1] == 0.0 || size[2] == 0.0)
      error("Degenerate box.\n");

   /* Attempt to allocate memory for this primitive */
   if ((box = (BoxData *)polyray_malloc(sizeof(BoxData))) == NULL)
      error("Failed to allocate box data\n");

   /* Set up the primitive specific information based on the
      input parameters */
   box->bounds[0][0] = MIN(v1[0], v2[0]);
   box->bounds[1][0] = MAX(v1[0], v2[0]);
   box->bounds[0][1] = MIN(v1[1], v2[1]);
   box->bounds[1][1] = MAX(v1[1], v2[1]);
   box->bounds[0][2] = MIN(v1[2], v2[2]);
   box->bounds[1][2] = MAX(v1[2], v2[2]);

   for (i=0;i<3;i++) {
      object->o_bnd.lower_left[i] = box->bounds[0][i];
      object->o_bnd.lengths[i] = (box->bounds[1][i] - box->bounds[0][i]);
      }
   object->o_data = (void *)box;

   return object;
}

static void
make_vert(Object *obj, Vertex *vert,
          Flt x, Flt y, Flt z, Vec N)
{
   Vec P, N1;

   MakeVector(x, y, z, P);
   VecCopy(P, vert->P);
   if (obj->o_trans) {
      TxVector(P, P, obj->o_trans);
      TxNormal(N1, N, obj->o_trans);
      }
   else
      VecCopy(N, N1)
   VecNormalize(N1);
   VecCopy(P, vert->W);
   VecCopy(N1, vert->N);
}

static short bindx[6][4][3] =
  {{{1, 0, 0}, {1, 1, 0}, {1, 1, 1}, {1, 0, 1}},
   {{0, 0, 0}, {0, 0, 1}, {0, 1, 1}, {0, 1, 0}},

   {{0, 1, 0}, {0, 1, 1}, {1, 1, 1}, {1, 1, 0}},
   {{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}},

   {{0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}},
   {{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}}};
static Vec bnorm[6] =
   {{ 1, 0, 0}, {-1, 0, 0}, { 0, 1, 0},
    { 0,-1, 0}, { 0, 0, 1}, { 0, 0,-1}};
void
BoxRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   Poly Polygon;
   BoxData *b = (BoxData *)obj->o_data;
   Vertex *vertptr, *tvert;
   int i, j;

   vertptr = &Polygon.vertices[0];
   for (i=0;i<6;i++) {
      Polygon.n = 4;
      for (j=0,tvert=vertptr;j<4;j++,tvert++)
         make_vert(obj, tvert,
                   b->bounds[bindx[i][j][0]][0],
                   b->bounds[bindx[i][j][1]][1],
                   b->bounds[bindx[i][j][2]][2],
                   bnorm[i]);
      scan_convert(eye, Root, obj, NULL, &Polygon);
      }

}


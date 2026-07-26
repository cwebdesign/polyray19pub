/* box.cc

  Polyray MIT Licensed Revival
  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Copyright (C) 1999-2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "defs3.h"
#include "memory.h"
#include "io_ply.h"
#include "intersec.h"
#include "symtab.h"
#include "scan.h"
#include "vector.h"
#include "bound.h"
#include "box.h"
#include "factory.h"
#include "unixcompat.h" //polyray_pause



/* Prototypes for this module */
static void BoxNormal(const Flt bounds[2][3], const Vec P, Vec& N);


/* Prototypes for the primitive operators */
void BoxRender(Viewpoint *, BinTree *, Object *);
int BoxIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int BoxInside(Object *, Vec);

openpolyray::dispatch::ObjectProcs BoxProcs = {
   .render = BoxRender,
   .evaluate = nullptr,
   .initialize = GenericInitialize,
   .intersect = BoxIntersect,
   .inside = BoxInside,
   .copy = GenericCopy,
   .del = BoxDelete,
} ;

/**
 * @brief Release the heap payload owned by a box primitive.
 *
 * @param object Box object whose `BoxData` payload should be destroyed.
 * @return No return value.
 */
void BoxDelete(Object *object)
{
   if (object->o_copy != 0 || object->o_data == nullptr)
      return;

   delete static_cast<BoxData *>(object->o_data);
   object->o_data = nullptr;
}

/**
 * @brief Compute the outward face normal for a point on a box surface.
 *
 * @param bounds Box minimum and maximum corners by axis.
 * @param P Point on the box surface.
 * @param N Receives the outward normal.
 * @return No return value.
 */
static void
BoxNormal(const Flt bounds[2][3], const Vec P, Vec& N)
{
   MakeVector(0, 0, 0, N);
        if (POLYRAYequal(P[0], bounds[1][0]))
      N[0] =  1.0;
   else if (POLYRAYequal(P[0], bounds[0][0]))
      N[0] = -1.0;
   else if (POLYRAYequal(P[1], bounds[1][1]))
      N[1] = 1.0;
   else if (POLYRAYequal(P[1], bounds[0][1]))
      N[1] = -1.0;
   else if (POLYRAYequal(P[2], bounds[1][2]))
      N[2] = 1.0;
   else if (POLYRAYequal(P[2], bounds[0][2]))
      N[2] = -1.0;
   else {
      MakeVector(0, 1, 0, N);
      }
}

/**
 * @brief Intersect a ray with an axis-aligned box primitive.
 *
 * @param Eye Viewpoint issuing the ray.
 * @param obj Box object being tested.
 * @param ray Ray to intersect.
 * @param mindist Minimum valid hit distance.
 * @param maxdist Maximum valid hit distance.
 * @param hit Intersection record receiving any valid hits.
 * @return Nonzero when at least one hit is recorded, otherwise `0`.
 */
int BoxIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
             Flt mindist, Flt maxdist, Isect *hit)
{
   #ifdef DEBUG_FN_CALLS
   std::cout<<"box::BoxIntersect"<<"\n";
   openpolyray::polyray_pause();
   #endif
   Vec P, N;
   Flt tmin = mindist;
   Flt tmax = maxdist;
   int Flag = 0;
   BoxData *box = (BoxData *)obj->o_data;

   if (determine_start(ray->P, ray->D, box->bounds, &tmin, &tmax)) {
      //std::cout<<"in box::boxintersect\n";exit(2);
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

/**
 * @brief Test whether a point lies inside the box primitive.
 *
 * @param obj Box object being queried.
 * @param P World-space point to test.
 * @return `1` when the point lies inside the box bounds, otherwise `0`.
 */
int BoxInside(Object *obj, Vec P)
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

/**
 * @brief Initialize a box primitive from two opposite corners.
 *
 * @param object Object being initialized as a box primitive.
 * @param v1 First corner of the box.
 * @param v2 Opposite corner of the box.
 * @return `object` configured as a box primitive.
 */
Object *MakeBox(Object *object, Vec v1, Vec v2)
{
   BoxData *box;
   Vec size;
   int i;

   object->o_type = ShapeType::Box;
   object->o_procs = &BoxProcs ;

   VecSub(v1, v2, size);

   if (size[0] == 0.0 || size[1] == 0.0 || size[2] == 0.0)
      serror("Degenerate box.\n");

   /* Attempt to allocate memory for this primitive */
   box = FactoryBoxData();
   if (box == nullptr)   
      serror("Failed to allocate box data\n");

   /* Set up the primitive specific information based on the
      input parameters */
   box->bounds[0][0] = PLY_MIN(v1[0], v2[0]);
   box->bounds[1][0] = MAX(v1[0], v2[0]);
   box->bounds[0][1] = PLY_MIN(v1[1], v2[1]);
   box->bounds[1][1] = MAX(v1[1], v2[1]);
   box->bounds[0][2] = PLY_MIN(v1[2], v2[2]);
   box->bounds[1][2] = MAX(v1[2], v2[2]);

   for (i=0;i<3;i++) {
      object->o_bnd.lower_left[i] = box->bounds[0][i];
      object->o_bnd.lengths[i] = (box->bounds[1][i] - box->bounds[0][i]);
      }
   object->o_data = (void *)box;

   return object;
}

/**
 * @brief Populate one transformed vertex for a rendered box face.
 *
 * @param obj Box object providing the optional transform.
 * @param vert Vertex structure to fill.
 * @param x Local-space X coordinate.
 * @param y Local-space Y coordinate.
 * @param z Local-space Z coordinate.
 * @param N Local-space face normal.
 * @return No return value.
 */
static void make_vert(Object *obj, Vertex *vert,
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
/**
 * @brief Render a box by scan converting its six polygonal faces.
 *
 * @param eye Current viewpoint used for rendering.
 * @param Root Scene bin tree receiving scan-converted output.
 * @param obj Box object to render.
 * @return No return value.
 */
void BoxRender(Viewpoint *eye, BinTree *Root, Object *obj)
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

/** @file tri.cc
 *  @brief Smooth-shaded triangle primitive - intersection, normal interpolation, and scan conversion.
 *
 *  Each triangle stores its three vertex positions, per-vertex normals, UV
 *  coordinates, and a precomputed inverse basis matrix (@c tri_bb) that maps
 *  world-space vectors into a canonical triangle-aligned coordinate system for
 *  fast ray intersection and barycentric decomposition.
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
#include "io_ply.h"
#include "memory.h"
#include "intersec.h"
#include "scan.h"
#include "vector.h"
#include "bound.h"
#include "symtab.h"
#include "roots.h"
#include "util.h"
#include "factory.h"
#include "tri.h"




void TriRender(Viewpoint *, BinTree *, Object *);
int TriIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int TriInside(Object *, Vec);


openpolyray::dispatch::ObjectProcs TriProcs = {
   .render = TriRender,
   .evaluate = nullptr,
   .initialize = GenericInitialize,
   .intersect = TriIntersect,
   .inside = TriInside,
   .copy = GenericCopy,
   .del = TriDelete,
   };


/**
 * @brief Release the heap payload owned by a triangle primitive.
 *
 * @param object Box object whose `TriData` payload should be destroyed.
 * @return No return value.
 */
void TriDelete(Object* object)
{
    if (object->o_copy != 0 || object->o_data == nullptr)
        return;

    delete static_cast<TriData*>(object->o_data);
    object->o_data = nullptr;
}

/** @brief Find a ray-triangle intersection using the precomputed inverse basis matrix.
 *
 *  Projects the ray into the triangle's canonical coordinate system via @c tri_bb.
 *  The hit distance is computed as n/d, and the barycentric coordinates (a, b)
 *  are validated to confirm the hit point lies within the triangle.  On a hit,
 *  the surface normal is interpolated from the three vertex normals using the
 *  barycentric weights, and UV coordinates are interpolated similarly.
 *
 *  @param Eye      Active viewpoint (unused directly; passed through).
 *  @param obj      Triangle object.
 *  @param ray      Incoming ray in world space.
 *  @param mindist  Minimum valid hit distance (near clip).
 *  @param maxdist  Maximum valid hit distance (far clip).
 *  @param hit      Output: intersection accumulator updated via Insert_Hit().
 *  @return         1 if a valid hit was found and recorded, 0 otherwise.
 */
int TriIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
             Flt mindist, Flt maxdist, Isect *hit)
{
   TriData *td = static_cast<TriData *>(obj->o_data);
   Flt n, d, dist;
   Flt r, s, t;
   Flt a, b, u, v;
   Vec P, Q, N, U;

   /*
    * The matrix td -> tri_bb transforms vectors in the world 
    * space into a space with the following properties.
    *
    * 1.  The sides of the triangle are coincident with the
    *     x and y axis, and have unit length.
    * 2.  The normal to the triangle is coincident with the 
    *     z axis.
    *
    */

   /*
    * d is the slope with respect to the z axis.  If d is zero, then
    * the ray is parallel to the plane of the polygon, and we count 
    * it as a miss...
    */
   d = VecDot(ray->D, td->tri_bb[2]);
   if (fabs(d) < PLY_EPSILON)
      return 0;

   /*
    * Q is a vector from the eye to the triangles "origin" vertex.
    * n is then set to be the distance of the tranformed eyepoint
    * to the plane in the polygon.
    * Together, n and d allow you to find the distance to the polygon, 
    * which is merely n / d.
    */
   VecSub(td->tri_P[0], ray->P, Q);
   n = VecDot(Q, td->tri_bb[2]);
   dist = n / d;

   if (dist < mindist) 
      return 0 ;
   
   /* 
    * Q is the point we hit.  Find its position relative to the
    * origin of the triangle.
    */
   VecAddScaled(ray->P, dist, ray->D, P);
   VecSub(P, td -> tri_P[0], Q);

   a = VecDot(Q, td->tri_bb[0]);
   b = VecDot(Q, td->tri_bb[1]);

   if (a < -PLY_EPSILON || b < -PLY_EPSILON || a + b > 1.0+PLY_EPSILON)
      return 0;
   
   r = 1.0 - a - b;
   s = a;
   t = b;

   if (dist > mindist && dist < maxdist) {
      MakeVector(0.0, 0.0, 0.0, N);
      VecAddS(r, td->tri_N[0], N, N);
      VecAddS(s, td->tri_N[1], N, N);
      VecAddS(t, td->tri_N[2], N, N);
      VecNormalize(N);
      u = r * td->tri_u[0] + s * td->tri_u[1] + t * td->tri_u[2];
      v = r * td->tri_v[0] + s * td->tri_v[1] + t * td->tri_v[2];
      MakeVector(u, v, 0, U);
      Insert_Hit(obj, P, N, dist, U, hit);
      return 1;
      }
   else
      return 0;
}

/** @brief Test whether a point lies inside the triangle volume (stub).
 *
 *  Triangles have no volume, so inside testing is not meaningful.
 *  @param obj  Triangle object (unused).
 *  @param P    World-space point to test (unused).
 *  @return     Always 0.
 */
int TriInside(Object *obj, Vec P)
{
   return 0;
}

/** @brief Initialise an Object as a smooth-shaded triangle primitive.
 *
 *  Copies the three vertex positions, normalised normals, and UV coordinates
 *  from @p tri_vert.  UV values equal to PLY_HUGE are replaced with default
 *  coordinates: (0,0), (1,0), (0,1).
 *
 *  Builds the 3x3 basis matrix whose rows are (P1-P0), (P2-P0), and the
 *  face normal, then stores its inverse as @c tri_bb[] for use by TriIntersect().
 *  A degenerate (zero-area) triangle falls back to a unit-X normal.
 *
 *  The tight axis-aligned bounding box is expanded by 2xPLY_EPSILON on each
 *  side to avoid numerical clipping artefacts.
 *
 *  @param object    Pre-allocated Object to configure as T_TRI.
 *  @param tri_vert  Array of exactly 3 UVVert entries (position, normal, u, v).
 *  @return          @p object configured as T_TRI with all data attached.
 */
Object* MakeTri(Object* object, UVVert* tri_vert)
{
    Vec maxs, mins;
    int i;
    Flt len;
    NuVec B[3];

    object->o_type = ShapeType::Tri;
    object->o_procs = &TriProcs;
    
    // Attempt to allocate memory for this primitive 
    TriData *td = FactoryTriData();

    // Copy the points into the patch 
    for (int i = 0; i < 3; ++i) {
        VecCopy(tri_vert[i].pos, td->tri_P[i]);
        fVecNormalize(tri_vert[i].norm);
        VecCopy(tri_vert[i].norm, td->tri_N[i]);
        td->tri_u[i] = tri_vert[i].u;
        td->tri_v[i] = tri_vert[i].v;
    }

    /* Correct the u/v if necessary */
    if (td->tri_u[0] == PLY_HUGE) td->tri_u[0] = 0.0;
    if (td->tri_v[0] == PLY_HUGE) td->tri_v[0] = 0.0;
    if (td->tri_u[1] == PLY_HUGE) td->tri_u[1] = 1.0;
    if (td->tri_v[1] == PLY_HUGE) td->tri_v[1] = 0.0;
    if (td->tri_u[2] == PLY_HUGE) td->tri_u[2] = 0.0;
    if (td->tri_v[2] == PLY_HUGE) td->tri_v[2] = 1.0;

    /*
     * construct the inverse of the matrix...
     * | P1 |
     * | P2 |
     * | N  |
     * and store it in td -> tri_bb[]
     */

    VecSub(td->tri_P[1], td->tri_P[0], B[0]);
    VecSub(td->tri_P[2], td->tri_P[0], B[1]);
    VecCross(B[0], B[1], B[2]);

    len = sqrt(VecDot(B[2], B[2]));
    if (len == 0.0)
        MakeVector(1.0, 0.0, 0.0, B[2])
    else {
        len = 1.0 / len;
        VecScale(len, B[2]);
    }

    //if (!TriInvertMatrix(B, td->tri_bb)) {
    auto ret = InvertMatrix(B);
    for (int i=0;i<3;i++)
       for (int j=0;j<3;j++)
          td->tri_bb[i][j] = ret.out[i][j];
    if (!ret.i) {
      polyray_free(td);
      serror("Degenerate triangle has no invertible basis matrix\n");
   }

   /* Compute bounding information */
   for (i=0;i<3;i++) {
      //mins[i] = PLY_MIN(td->tri_P[0][i], PLY_MIN(td->tri_P[1][i], td->tri_P[2][i]));
      mins[i] = std::min<Flt>({ td->tri_P[0][i], td->tri_P[1][i], td->tri_P[2][i] });
      maxs[i] = MAX(td->tri_P[0][i], MAX(td->tri_P[1][i], td->tri_P[2][i]));
      }
   VecCopy(mins, object->o_bnd.lower_left);
   VecSub(maxs, mins, object->o_bnd.lengths);
   for (i=0;i<3;i++) {
      object->o_bnd.lower_left[i] -= PLY_EPSILON + PLY_EPSILON;
      object->o_bnd.lengths[i] += 4.0 * PLY_EPSILON;
      }
   object->o_data = (void *)td;
   return object;
}


/** @brief Scan-convert the triangle as a 3-vertex polygon.
 *
 *  Copies vertex positions, normals, and UV coordinates into a @c Poly struct,
 *  applies the object's world-space transform if present, and hands off to
 *  scan_convert() for rasterisation.
 *  @param eye   Active viewpoint.
 *  @param Root  Scene BVH receiving any generated sub-primitives.
 *  @param obj   Triangle object to render.
 */
void TriRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   int i;
   Vec P, N;
   TriData *tri = (TriData *)obj->o_data;
   Poly Polygon;
   Object *tobj;

   Polygon.n = 3;
   for (i=0;i<3;i++) {
      P[0] = tri->tri_P[i][0];
      P[1] = tri->tri_P[i][1];
      P[2] = tri->tri_P[i][2];
      N[0] = tri->tri_N[i][0];
      N[1] = tri->tri_N[i][1];
      N[2] = tri->tri_N[i][2];

      VecCopy(P, Polygon.vertices[i].P);
      MakeVector(tri->tri_u[i], tri->tri_v[i], 0.0,
                 Polygon.vertices[i].U);

      tobj = obj;
      if (tobj->o_trans) {
         TxVector(P, P, obj->o_trans);
         TxNormal(N, N, obj->o_trans);
         }

      VecCopy(P, Polygon.vertices[i].W);
      VecCopy(N, Polygon.vertices[i].N);
      }
   scan_convert(eye, Root, obj, NULL, &Polygon);
}

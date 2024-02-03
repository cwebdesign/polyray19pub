/* intersec.c

   Step through all objects and compare the minimum distance of intersection
   of each one (that has an intersection).  Return the minimum hit point.

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
#include "bound.h"
#include "vector.h"
#include "intersec.h"
#include "csg.h"
#include "symtab.h"
#include "roots.h"

int InvertMatrix(fVec in[3], fVec out[3]);

int
Insert_Hit(Object *obj, Vec P, Vec N, Flt t, Vec U, Isect *hit)
{
   Vec W;

   if (hit->flag == 0 || t < hit->isect_t) {
      if (obj->o_trans)
         TxVector(W, P, obj->o_trans)
      else
         VecCopy(P, W);

      /* Check CSG (if any) */
      if (obj->o_parent != NULL &&
          !Inside_CSG_Node(obj->o_csg_tree, W))
         return 0;

      /* Check u,v boundaries */
      if ((Global_Shade_Flag & UV_CHECK) &&
          (obj->o_sflag & UV_CHECK))
         if (U[0] < obj->o_uv_bounds[0] || U[0] > obj->o_uv_bounds[1] ||
             U[1] < obj->o_uv_bounds[2] || U[1] > obj->o_uv_bounds[3])
            return 0;

      hit->flag    = 1;
      hit->obj     = obj;
      hit->isect_t = t;
      hit->texture = NULL;
      VecCopy(U, hit->U);
      VecCopy(W, hit->W);
      if (obj->o_trans)
         TxNormal(hit->N, N, obj->o_trans);
      else
         VecCopy(N, hit->N);

      }
   return 1;
}

#define BARY_VAL1 -0.005
#define BARY_VAL2 1.0001

static int
intersect_triangle(TriangleObject *obj, Ray *ray, Flt mindist, Flt maxdist,
                   Isect *hit)
{
   fVec B[3], IB[3];
   Vec Q, W, W1;
   fVec *V, *N, *U;
   Vec N1, U1;
   Object *pobj;
   Flt d, n, a, b, r, t;
   int ui, vi;

   pobj = obj->o_parent;
   V = pobj->o_vertices->V;
   U = pobj->o_vertices->U;
   N = pobj->o_vertices->N;

   VecSub(V[obj->o_vert[1]], V[obj->o_vert[0]], B[0]);
   VecSub(V[obj->o_vert[2]], V[obj->o_vert[0]], B[1]);
   VecCross(B[0], B[1], B[2]);
   d = sqrt(VecDot(B[2], B[2]));
   if (fabs(d) < EPSILON)
      MakeVector(1.0f, 0.0f, 0.0f, B[2])
   else {
      d = 1.0 / d;
      VecScale(d, B[2]);
      }

   d = VecDot(ray->D, B[2]);
   if (fabs(d) < EPSILON)
      return 0;

   VecSub(V[obj->o_vert[0]], ray->P, Q);
   n = VecDot(Q, B[2]);
   t = n / d;

   if (t < mindist || t > maxdist)
      return 0;

   VecAddScaled(ray->P, t, ray->D, W);

   if (N == NULL && U == NULL) {
      /* Can do normal point in polygon operation */
      VecCopy(B[2], N1)
      VecCopy(W, U1)
      VecCopy(V[obj->o_vert[0]], B[0])
      VecCopy(V[obj->o_vert[1]], B[1])
      VecCopy(V[obj->o_vert[2]], B[2])
      if (fabs(N1[0]) >= fabs(N1[1]) && fabs(N1[0]) >= fabs(N1[2])) {
         ui = 1; vi = 2;
         }
      else if (fabs(N1[1]) >= fabs(N1[0]) && fabs(N1[1]) >= fabs(N1[2])) {
         ui = 0; vi = 2;
         }
      else {
         ui = 0; vi = 1;
         }
      if (!Inside_Polygon(W[ui], W[vi], 3, B, ui, vi))
         return 0;
      }
   else {
      if (!InvertMatrix(B, IB))
         return 0;
      VecSub(W, V[obj->o_vert[0]], Q);
      a = VecDot(Q, IB[0]);
      b = VecDot(Q, IB[1]);
      if (a < BARY_VAL1 || b < BARY_VAL1 || a + b > BARY_VAL2)
         /* The use of BARY_VAL1 is an attempt to compensate for the
            lack of precision in the floating point numbers used in
            the matrices B and IB.  Since floats only have around
            7 digits of precision, we make sure that we allow any
            slop in a and b that is less than that. */
         return 0;
      r = 1.0 - a - b;

      /* Now that we have the barycentric coordinates for the
         triangle, we can interpolate the rest of the information
         from the vertices */
      if (N != NULL) {
         MakeVector(0.0, 0.0, 0.0, N1);
         VecAddS(r, N[obj->o_vert[0]], N1, N1);
         VecAddS(a, N[obj->o_vert[1]], N1, N1);
         VecAddS(b, N[obj->o_vert[2]], N1, N1);
         VecNormalize(N1);
         }
      else
         VecCopy(B[2], N1)

      if (U != NULL) {
         MakeVector(0.0, 0.0, 0.0, U1);
         VecAddS(r, U[obj->o_vert[0]], U1, U1);
         VecAddS(a, U[obj->o_vert[1]], U1, U1);
         VecAddS(b, U[obj->o_vert[2]], U1, U1);
         }
      else
         VecCopy(W, U1)
      }

   if (pobj->o_type != T_RAW_TRIANGLES) {
      VecCopy(W, W1)
      /* Check CSG (if any) */
      if (!Inside_CSG_Node(pobj->o_csg_tree, W1))
         return 0;
      }

   /* Check u,v boundaries */
   if (U1[0] < pobj->o_uv_bounds[0] || U1[0] > pobj->o_uv_bounds[1] ||
       U1[1] < pobj->o_uv_bounds[2] || U1[1] > pobj->o_uv_bounds[3])
      return 0;

   hit->flag    = 1;
   hit->obj     = pobj;
   hit->isect_t = t;
   hit->texture = obj->o_texture;
   VecCopy(U1, hit->U);
   VecCopy(N1, hit->N);
   VecCopy(W, hit->W);

   return 1;
}

/* Shoule we always move the ray start right to the edge of the bounding
   box?  This can be a win for numerical stability. */
#define NORMALIZE_RAY 1

/* For CSG objects, it's important to check bounds/clips in the children */
int
find_object_intersections(Viewpoint *Eye, Object *cobj, Ray *world_ray,
                          Flt mindist, Flt maxdist, Isect *hit)
{
   Object *tobj;
   Isect new_hit;
   Ray object_ray;
   Flt t, d, rayoffset;

   /* May need to use parent object to determine shading
      quality flags */
   tobj = (cobj->o_type == T_POLYGON ? cobj->o_parent : cobj);

   /* If we are just looking for shadows and this object
      does not cast shadows, then don't do any more */
   if (Shadow_Test && !(tobj->o_sflag & CAST_SHADOW))
      return 0;

   /* Intersection tests don't work on particles when we are in the process
      of building particle systems */
   if (Particle_Test && (tobj->o_sflag & PARTICLE_FLAG))
      return 0;

   if (tobj->o_dither >= 0.0 && tobj->o_dither < polyray_random())
      return 0;

   new_hit.flag = 0;

   if (cobj->o_type == T_POLYGON) {
      /* Special case, this is a triangle that makes up part of
         another object. */
      d = 1.0;
      rayoffset = 0.0;
      intersect_triangle((TriangleObject *)cobj, world_ray,
                         mindist, maxdist, &new_hit);
      }
   else {
      /* Transform the ray into the objects coordinates */
      if (cobj->o_trans) {
         InvTxVector1(object_ray.P, world_ray->P, cobj->o_trans);
         InvTxDirection(object_ray.D, world_ray->D, cobj->o_trans);
         d = VecNormalize(object_ray.D);
         mindist *= d;
         maxdist *= d;
         }
      else {
         VecCopy(world_ray->P, object_ray.P);
         VecCopy(world_ray->D, object_ray.D);
         d = 1.0;
         }

      /* If appropriate we adjust the starting point of the ray */
#if NORMALIZE_RAY
      if (mindist > rayeps) {
         rayoffset = mindist;
         mindist   -= rayoffset;
         maxdist   -= rayoffset;
         VecAddScaled(object_ray.P, rayoffset, object_ray.D, object_ray.P)
      } else
         rayoffset = 0.0;
#else
      rayoffset = 0.0;
#endif

      /* Collect all intersections with the object */
      (cobj->o_procs->intersect)(Eye, cobj, &object_ray, mindist, maxdist, &new_hit);
      }

   /* If there is a valid hit after checking the object, then adjust the
      normal and position. */
   if (new_hit.flag) {
#if NORMALIZE_RAY
      t = (new_hit.isect_t + rayoffset) / d;
#else
      t = new_hit.isect_t / d;
#endif
      if (hit->flag == 0 || t < hit->isect_t) {
         hit->flag    = 1;
         hit->obj     = new_hit.obj;
         hit->texture = new_hit.texture;
         hit->isect_t = t;
         VecCopy(new_hit.U, hit->U);
         VecCopy(new_hit.W, hit->W);
         VecCopy(new_hit.N, hit->N);
         return 1;
         }
      }
   return 0;
}

/***********************************************************************
 Intersect(ray, hit, mindist, maxdist)
 
 Returns true if we hit something in the root model between mindist and
 maxdist.  
 Returns the closest hit in the "hit" buffer.
 ***********************************************************************/
int
Intersect(Viewpoint *Eye, BinTree *Root, Ray *world_ray,
          Flt mindist, Flt maxdist, Isect *hit)
{
   ostackptr objs;

   if (Root->slab_root != NULL)
      return BoundIntersect(Eye, Root, world_ray, mindist, maxdist, hit);
   else {
      hit->flag = 0;
      objs = Root->members.list;
      while (objs != NULL) {
         totalQueues++;
         find_object_intersections(Eye, objs->element, world_ray,
                                   mindist, maxdist, hit);
         objs = objs->next;
         }
      return hit->flag;
      }
}

#if 0
/* Return the number of intersections in a BinTree from Ray->P along
   the direction ray->D.  */
#define MAX_OCCLUSIONS 1000
int 
RayBlocks(BinTree *root, Ray *ray, Flt tmin, Flt tmax) 
{
   Ray jray;
   Isect hit;
   int cnt = 0;

   VecCopy(ray->P, jray.P);
   VecCopy(ray->D, jray.D);
   cnt = 0;
   while (tmax > tmin && cnt < MAX_OCCLUSIONS) {
      if (Intersect(root, &jray, &hit, tmin, tmax)) {
         /* Move up a little closer to the target */
         VecCopy(hit.W, jray.P);
         tmax -= hit.isect_t;
         cnt++;
         }
      else
         break;
      }

   return cnt;
}
#endif


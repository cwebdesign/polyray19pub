/* intersec.cc

   Step through all objects and compare the minimum distance of intersection
   of each one (that has an intersection).  Return the minimum hit point.

  Polyray - MIT Licensed Revival
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
#include "bound.h"
#include "vector.h"
#include "intersec.h"
#include "csg.h"
#include "runtime_state.h"
#include "symtab.h"
#include "roots.h"
#include "sphere.h"
//#include "util.h" //InvertMatrix
#include "enqueue.h"



//! Invert a Matrix (C Style coding)
/*!
      @param in Input Matrix
      @param out Output Matrix
      @return 1 if success, 0 if failure
*/
static int C_InvertMatrix(fVec in[3], fVec out[3])
{

    out[0][0] = (in[1][1] * in[2][2] - in[1][2] * in[2][1]);
    out[1][0] = -(in[0][1] * in[2][2] - in[0][2] * in[2][1]);
    out[2][0] = (in[0][1] * in[1][2] - in[0][2] * in[1][1]);

    out[0][1] = -(in[1][0] * in[2][2] - in[1][2] * in[2][0]);
    out[1][1] = (in[0][0] * in[2][2] - in[0][2] * in[2][0]);
    out[2][1] = -(in[0][0] * in[1][2] - in[0][2] * in[1][0]);

    out[0][2] = (in[1][0] * in[2][1] - in[1][1] * in[2][0]);
    out[1][2] = -(in[0][0] * in[2][1] - in[0][1] * in[2][0]);
    out[2][2] = (in[0][0] * in[1][1] - in[0][1] * in[1][0]);

    Flt det = in[0][0] * in[1][1] * in[2][2] +
        in[0][1] * in[1][2] * in[2][0] +
        in[0][2] * in[1][0] * in[2][1] -
        in[0][2] * in[1][1] * in[2][0] -
        in[0][0] * in[1][2] * in[2][1] -
        in[0][1] * in[1][0] * in[2][2];

    if (fabs(det) < PLY_EPSILON)
        return 0;

    det = 1 / det;

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            out[i][j] *= det;
    return 1;
}

//! Insert a hit
/*!
      @param obj Input Object
      @param P Intersection Point
      @param N Surface Normal
      @param t Distance to Intersection
      @param U Texture Coordinates
      @param hit Intersection Record
      @return 1 if success, 0 if failure
*/
//P and N could be const later. 
int Insert_Hit(Object* obj, Vec P, Vec N, Flt t, const Vec U, Isect* hit)
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
        if ((runtimeState::scene.Global_Shade_Flag & UV_CHECK) &&
            (obj->o_sflag & UV_CHECK))
            if (U[0] < obj->o_uv_bounds[0] || U[0] > obj->o_uv_bounds[1] ||
                U[1] < obj->o_uv_bounds[2] || U[1] > obj->o_uv_bounds[3])
                return 0;

        hit->flag = 1;
        hit->obj = obj;
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

//! Intersect triangle
/*!
      @param obj Input Triangle Object
      @param ray Input Ray
      @param mindist Minimum Distance for Intersection
      @param maxdist Maximum Distance for Intersection
      @param hit Intersection Record
      @return 1 if intersection occurs, 0 otherwise
*/
static int intersect_triangle(TriangleObject* obj, Ray* ray, Flt mindist, Flt maxdist,
    Isect* hit)
{
    fVec B[3], IB[3];
    Vec Q, W, W1;
    fVec* V, * N, * U;
    Vec N1, U1;
    Object* pobj;
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
    if (fabs(d) < PLY_EPSILON)
        MakeVector(1.0f, 0.0f, 0.0f, B[2])
    else {
        d = 1.0 / d;
        VecScale(d, B[2]);
    }

    d = VecDot(ray->D, B[2]);
    if (fabs(d) < PLY_EPSILON)
        return 0;

    VecSub(V[obj->o_vert[0]], ray->P, Q);
    n = VecDot(Q, B[2]);
    t = n / d;

    if (t < mindist || t > maxdist)
        return 0;

    VecAddScaled(ray->P, t, ray->D, W);

    if (N == nullptr && U == nullptr) {
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
        if (!C_InvertMatrix(B, IB))
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
        if (N != nullptr) {
            MakeVector(0.0, 0.0, 0.0, N1);
            VecAddS(r, N[obj->o_vert[0]], N1, N1);
            VecAddS(a, N[obj->o_vert[1]], N1, N1);
            VecAddS(b, N[obj->o_vert[2]], N1, N1);
            VecNormalize(N1);
        }
        else
            VecCopy(B[2], N1)

            if (U != nullptr) {
                MakeVector(0.0, 0.0, 0.0, U1);
                VecAddS(r, U[obj->o_vert[0]], U1, U1);
                VecAddS(a, U[obj->o_vert[1]], U1, U1);
                VecAddS(b, U[obj->o_vert[2]], U1, U1);
            }
            else
                VecCopy(W, U1)
    }

    if (pobj->o_type != ShapeType::Raw_Triangles) {
        VecCopy(W, W1)
            /* Check CSG (if any) */
            if (!Inside_CSG_Node(pobj->o_csg_tree, W1))
                return 0;
    }

    /* Check u,v boundaries */
    if (U1[0] < pobj->o_uv_bounds[0] || U1[0] > pobj->o_uv_bounds[1] ||
        U1[1] < pobj->o_uv_bounds[2] || U1[1] > pobj->o_uv_bounds[3])
        return 0;

    hit->flag = 1;
    hit->obj = pobj;
    hit->isect_t = t;
    hit->texture = obj->o_texture;
    VecCopy(U1, hit->U);
    VecCopy(N1, hit->N);
    VecCopy(W, hit->W);

    return 1;
}

/* Shoule we always move the ray start right to the edge of the bounding
   box?  This can be a win for numerical stability. */
const int NORMALIZE_RAY = 1;

//! Find Object Intersections Tester
/*!   @param cobj Input Object
      @param world_ray Input Ray in World Coordinates
      @param mindist Minimum Distance for Intersection
      @param maxdist Maximum Distance for Intersection
      @param hit Intersection Record
      @return 1 if intersection occurs, 0 otherwise
*/
int find_object_intersectionsTester(Object* cobj, Ray* world_ray,
    Flt mindist, Flt maxdist, Isect* hit)
{
    Viewpoint Eye = { 256, 256, 0, 0, 256, 256, 0, 255,
                      {0, 0, -1}, {0, 0, 0}, {0, 1, 0},
                      45.0, SMALL, PLY_HUGE, 0.0, 1.0, -1.0,
                      NULL, NULL, NULL };

    if (cobj == nullptr || world_ray == nullptr || hit == nullptr)
        return 0;

    /* Keep the synthetic eye aligned with the test ray origin. */
    VecCopy(world_ray->P, Eye.view_from);

    return find_object_intersections(&Eye, cobj, world_ray, mindist, maxdist, hit);
}

//! Find Object Intersections
/*!
      @param Eye Viewpoint for the camera
      @param cobj Input Object to check for intersections
      @param world_ray Input Ray in World Coordinates
      @param mindist Minimum Distance for Intersection
      @param maxdist Maximum Distance for Intersection
      @param hit Intersection Record to store results
      @return 1 if intersection occurs, 0 otherwise
      \note For CSG objects, it's important to check bounds/clips in the children
*/
int find_object_intersections(Viewpoint* Eye, Object* cobj, Ray* world_ray,
    Flt mindist, Flt maxdist, Isect* hit)
{
#ifdef DEBUG_FN_CALLS
    std::cout << ("intersec::find_object_intersections\n");//instead of smessage
#endif
    Object* tobj;
    Isect new_hit;
    Ray object_ray;
    Flt t, d, rayoffset;

#ifdef DEBUG_FN_CALLS
    std::cout << "world_ray->P[0]=" << world_ray->P[0] << " world_ray->P[1]=" << world_ray->P[1] << 
         " world_ray->P[2]=" << world_ray->P[2] << std::endl;
    std::cout << "world_ray->D[0]=" << world_ray->D[0] << " world_ray->D[1]=" << world_ray->D[1] <<
         " world_ray->D[2]=" << world_ray->D[2] << std::endl;
#endif
    /* May need to use parent object to determine shading
       quality flags */
    tobj = (cobj->o_type == ShapeType::Polygon ? cobj->o_parent : cobj);

    /* If we are just looking for shadows and this object
       does not cast shadows, then don't do any more */
    if (RuntimeState::Shadow_Test && !(tobj->o_sflag & CAST_SHADOW))
        return 0;

    /* Intersection tests don't work on particles when we are in the process
       of building particle systems */
    //std::cout << "Intersection tests\n";
    //exit(2);
    if (RuntimeState::Particle_Test && (tobj->o_sflag & PARTICLE_FLAG))
        return 0;

    if (tobj->o_dither >= 0.0 && tobj->o_dither < polyray_random())
        return 0;

    new_hit.flag = 0;

    if (cobj->o_type == ShapeType::Polygon) {
        /* Special case, this is a triangle that makes up part of
           another object. */
        d = 1.0;
        rayoffset = 0.0;
        intersect_triangle((TriangleObject*)cobj, world_ray,
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
        if (mindist > RuntimeState::rayeps) {
            rayoffset = mindist;
            mindist -= rayoffset;
            maxdist -= rayoffset;
            VecAddScaled(object_ray.P, rayoffset, object_ray.D, object_ray.P)
        }
        else
            rayoffset = 0.0;
#else
        rayoffset = 0.0;
#endif

        /* Collect all intersections with the object */
        //std::cout << "Sphere intersection test triggered\n";
        //exit(2);

        //cobj->o_procs is 0
        (cobj->o_procs->intersect)(Eye, cobj, &object_ray, mindist, maxdist, &new_hit);
        //SphereIntersect(Eye, cobj, &object_ray, mindist, maxdist, &new_hit);
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
            hit->flag = 1;
            hit->obj = new_hit.obj;
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

//! Find Ray Intersection with Scene
/*!
      Finds the closest intersection point of a ray with objects in the scene.
      Returns true if we hit something in the root model between mindist and maxdist.
      Returns the closest hit in the "hit" buffer.
      @param Eye Viewpoint for the camera
      @param Root Spatial acceleration structure (BinTree) containing objects
      @param world_ray Input Ray in World Coordinates
      @param mindist Minimum Distance for Intersection
      @param maxdist Maximum Distance for Intersection
      @param hit Intersection Record to store results
      @return 1 if intersection occurs, 0 otherwise
*/
int Intersect(Viewpoint* Eye, BinTree* Root, Ray* world_ray,
    Flt mindist, Flt maxdist, Isect* hit)
{    
#ifdef DEBUG_FN_CALLS
    std::cout << ("intersec::Intersect\n");//instead of smessage
#endif    
    ostackptr objs;
#ifdef DEBUG_FN_CALLS
    std::cout << "Ray=<" << world_ray->P[0] << "," << world_ray->P[1] << "," << world_ray->P[2] << ">\n";
    if (Root->slab_root == nullptr) { //fixed for testing!
        std::cout << "Root->slab_root is nullptr\n";
    }
    else {
        std::cout << "Root.slab_root->o_type-" << std::to_underlying(Root->slab_root->o_type) << "\n";
        std::cout << "Root.slab_root->o_bnd.lower_left=" << Root->slab_root->o_bnd.lower_left[0] << ","
            << Root->slab_root->o_bnd.lower_left[1] << "," << Root->slab_root->o_bnd.lower_left[2] << "\n";

    }
#endif
    

    if (Root->slab_root != NULL) {
        //BoundIntersectTester(*Root, world_ray, mindist, maxdist, hit);
        //std::cout << "slab root not null\n";
            //exit(30);
        return BoundIntersect(Eye, *Root, world_ray, mindist, maxdist, hit);
    }
    else {
        hit->flag = 0;
        objs = Root->members.list;
        while (objs != NULL) {
            RuntimeState::stats.totalQueues++;
            find_object_intersections(Eye, objs->element, world_ray,
                mindist, maxdist, hit);
            objs = objs->next;
        }
        return hit->flag;
    }
}

//! Find Ray Intersection with Scene (C++ Reference Version)
/*!
      C++ version of Intersect using references instead of pointers.
      Finds the closest intersection point of a ray with objects in the scene.
      @param Eye Viewpoint for the camera
      @param Root Spatial acceleration structure (BinTree reference) containing objects
      @param world_ray Input Ray in World Coordinates
      @param mindist Minimum Distance for Intersection
      @param maxdist Maximum Distance for Intersection
      @param hit Intersection Record to store results
      @return 1 if intersection occurs, 0 otherwise
*/
int IntersectCPP(Viewpoint* Eye, BinTree& Root, Ray* world_ray,
    Flt mindist, Flt maxdist, Isect* hit)
{
#ifdef DEBUG_FN_CALLS
    std::cout << ("intersec::IntersectCPP\n");//instead of smessage
#endif
    ostackptr objs;
#ifdef DEBUG_FN_CALLS
    std::cout << "Ray=<" << world_ray->P[0] << "," << world_ray->P[1] << "," << world_ray->P[2] << ">\n";
    if (Root.slab_root != nullptr) {
        std::cout << "Root.slab_root->o_type-" << std::to_underlying(Root.slab_root->o_type) << "\n";
        std::cout << "Root.slab_root->o_bnd.lower_left=" << Root.slab_root->o_bnd.lower_left[0] << ","
            << Root.slab_root->o_bnd.lower_left[1] << "," << Root.slab_root->o_bnd.lower_left[2] << "\n";
    }
#endif

    if (Root.slab_root != nullptr) {
        //BoundIntersectTester(*Root, world_ray, mindist, maxdist, hit);
        return BoundIntersect(Eye, Root, world_ray, mindist, maxdist, hit);
    }
    else {
        hit->flag = 0;
        objs = Root.members.list;
        while (objs != nullptr) {
            RuntimeState::stats.totalQueues++;
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
RayBlocks(BinTree* root, Ray* ray, Flt tmin, Flt tmax)
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

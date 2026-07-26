/* sphere.cc

   Processing for sphere primitives

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
#include "io_ply.h"
#include "memory.h"
#include "intersec.h"
#include "runtime_state.h"
#include "symtab.h"
#include "scan.h"
#include "vector.h"
#include "bound.h"
#include "sphere.h"
#include "factory.h"
#include "vec3.h"




#ifndef TESTING
int SphereInside(Object* obj, Vec P);
void SphereUV(Vec Pos, Vec C, Flt* u, Flt* v);
void Ellipse_Evaluator(Object*, Flt, Flt, Vertex*);
#endif

openpolyray::dispatch::ObjectProcs SphereProcs = {
   .render = GenericRender,
   .evaluate = Ellipse_Evaluator,
   .initialize = GenericInitialize,
   .intersect = SphereIntersect,
   .inside = SphereInside,
   .copy = GenericCopy,
   .del = SphereDelete,
};

/**
 * @brief Release the heap payload owned by a sphere primitive.
 *
 * @param object Sphere object whose `SphereData` payload should be destroyed.
 * @return No return value.
 */
void SphereDelete(Object* obj)
{
    if (obj->o_copy != 0 || obj->o_data == nullptr)
        return;

    delete static_cast<SphereData*>(obj->o_data);
    obj->o_data = nullptr;
}


/** @brief Compute the geometric normal of a sphere at a point.
 *  @param sp Sphere primitive data.
 *  @param P  Point on the sphere surface.
 *  @param N  Output normal vector.
 *  @return Always returns 1.
 */
#ifndef TESTING
static
#endif
int SphereNormal(SphereData* sp, Vec P, Vec N)
{
    openpolyray::Vec3 v(P);
    openpolyray::Vec3 sph_centre(sp->sph_center);
    openpolyray::Vec3 newN=v-sph_centre;
    //VecSub(P, sp->sph_center, N);
    newN.Vec3ToVec(N);
    return 1;
}

/** @brief Convert a point on a sphere to UV texture coordinates.
 *  @param Pos Point on or near the sphere surface.
 *  @param C   Sphere center.
 *  @param u   Output horizontal texture coordinate.
 *  @param v   Output vertical texture coordinate.
 *  @return No return value.
 */
void SphereUV(Vec Pos, Vec C, Flt* u, Flt* v)
{    
    Flt x, y, z;
    
    openpolyray::Vec3 vPos(Pos);
    openpolyray::Vec3 vC(C);
    openpolyray::Vec3 vP = vPos-vC;    
    
    // Make sure this vector is on the unit sphere. 
    Flt len = vP.length();// VecLen(P);
    
    if (len < PLY_EPSILON) {
        *u = *v = 0.0;
        return;
    }
    else {
        Flt scp = RECIPROCAL(len);
        x = vP[0] * scp; // / len;
        y = vP[1] * scp; // / len;
        z = vP[2] * scp; // / len;
        /* try scp = 1/len; x=scp*P[0];y=scp*P[1];z=scp*P[2]; */
    }
    /* Determine its angle from the x-z plane. */
    Flt phi = std::asin(y);

    /* Determine its angle from the point (1, 0, 0) in the x-z plane. */
    len = std::sqrt(x * x + z * z);
    Flt theta;
#if 1
    theta = std::atan2(z, x) + PYM_PI;
#else
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
#endif
    * u = 1.0 - theta / (2.0 * PYM_PI);
    *v = 0.5 + (phi / PYM_PI);
}

/** @brief Record a sphere hit at ray parameter @p t if it lies within range.
 *  @param obj      Sphere object being tested.
 *  @param ray      Ray in world space.
 *  @param t        Candidate hit distance along the ray.
 *  @param mindist  Minimum valid hit distance.
 *  @param maxdist  Maximum valid hit distance.
 *  @param sp       Sphere data for @p obj.
 *  @param hit      Intersection accumulator.
 *  @return true when a hit was inserted, false otherwise.
 */
using namespace openpolyray;
static inline bool record_sphere_hit(Object* obj, Ray* ray, Flt t,
    Flt mindist, Flt maxdist, SphereData* sp, Isect* hit)
{
    if (t <= mindist || t >= maxdist) return false;
    //Vec P, N, U;
    Vec U;
    Vec3 P, rP{ ray->P }, rD{ ray->D };
    P.AddScaled(rP, t, rD);
    //VecAddScaled(ray->P, t, ray->D, P);
    Vec vP;
    P.Vec3ToVec(vP);
    if ((runtimeState::scene.Global_Shade_Flag & UV_CHECK) &&
        (obj->o_sflag & UV_CHECK)) {
        Flt u, v; 
        SphereUV(vP, sp->sph_center, &u, &v);
        MakeVector(u, v, 0, U);
    }
    else
        VecCopy(vP, U);
    Vec N;
    SphereNormal(sp, vP, N);
    Insert_Hit(obj, vP, N, t, U, hit);
    return true;
}

/** @brief Intersect a ray with a sphere primitive.
 *  @param Eye      Active viewpoint, or nullptr for non-primary rays.
 *  @param obj      Sphere object to test.
 *  @param ray      Ray in world space.
 *  @param mindist  Minimum valid hit distance.
 *  @param maxdist  Maximum valid hit distance.
 *  @param hit      Intersection accumulator.
 *  @return 1 when at least one valid hit is inserted, 0 otherwise.
 */
int SphereIntersect(Viewpoint* Eye, Object* obj, Ray* ray,
    Flt mindist, Flt maxdist, Isect* hit)
{
    Flt b, disc;
    Vec V;
    SphereData* sp = static_cast<SphereData*>(obj->o_data);
    int Flag = 0;

    VecSub(sp->sph_center, ray->P, V);
    b = VecDot(V, ray->D);
    disc = b * b - VecDot(V, V) + sp->sph_radius2;
    if (disc < 0.0) return 0;//doesnt hit
    disc = std::sqrt(disc);
    /* Near root first, then far root; both share the same hit-recording path. */
    if (record_sphere_hit(obj, ray, b - disc, mindist, maxdist, sp, hit)) Flag = 1;
    if (record_sphere_hit(obj, ray, b + disc, mindist, maxdist, sp, hit)) Flag = 1;
    return Flag;
}

/** @brief Test whether a point lies inside a sphere.
 *  @param obj Sphere object to test.
 *  @param Pos Point in world space.
 *  @return 1 when the point is inside the sphere, 0 otherwise.
 */
int SphereInside(Object* obj, Vec Pos)
{
    SphereData* sp;
    Vec P, D;
    Flt d;

    InvTxVector1(P, Pos, obj->o_trans)
        sp = (SphereData*)obj->o_data;
    VecSub(P, sp->sph_center, D);
    d = VecDot(D, D);
    return ((d < sp->sph_radius2) ? 1 : 0);
}

/** @brief Build a sphere primitive from a center and radius.
 *  @param object Object to initialize.
 *  @param pos    Sphere center.
 *  @param radius Sphere radius.
 *  @return The initialized sphere object.
 */
Object *MakeSphere(Object* object, Vec pos, Flt radius)
{
    int i;

    object->o_type = ShapeType::Sphere;
    object->o_procs = &SphereProcs;
    object->o_uv_steps[0] = 32;
    object->o_uv_steps[1] = 16;

    SphereData* sp = FactorySphereData();
    VecCopy(pos, sp->sph_center);
    sp->sph_radius = radius;
    sp->sph_radius2 = radius * radius;

    /* Compute bounding information */
    for (i=0;i<3;i++) {
        object->o_bnd.lower_left[i] = sp->sph_center[i] - sp->sph_radius;
        object->o_bnd.lengths[i] = 2.0 * sp->sph_radius;
    }

    object->o_data = (void*)sp;
    return object;
}

/** @brief Evaluate a parametric point and normal on the sphere surface.
 *  @param obj   Sphere object being sampled.
 *  @param u     Horizontal parametric coordinate.
 *  @param v     Vertical parametric coordinate.
 *  @param vert  Output vertex receiving position, UV, world position, and normal.
 *  @return No return value.
 */
void Ellipse_Evaluator(Object* obj, Flt u, Flt v, Vertex* vert)
{
    Flt theta, phi;
    Flt radius;
    Vec P, N;
    SphereData* sp = (SphereData*)obj->o_data;

    MakeVector(u, v, 0.0, vert->U);

    radius = sp->sph_radius;

    theta = (TWO_PI * v - PYM_PI) / 2.0;
    phi = TWO_PI * (1.0 - u);

    N[0] = cos(phi) * cos(theta);
    N[1] = sin(theta);
    N[2] = sin(phi) * cos(theta);
    VecAddScaled(sp->sph_center, radius, N, P);

    VecCopy(P, vert->P);
    if (obj->o_trans) {
        TxVector(P, P, obj->o_trans);
        TxNormal(N, N, obj->o_trans);
    }
    VecNormalize(N);
    VecCopy(P, vert->W);
    VecCopy(N, vert->N);
}

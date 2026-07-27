#if !defined(__POLYRAY_BOUNDS_DEFS)
#define __POLYRAY_BOUNDS_DEFS


#include "defs3.h" //for Object
#include <vector>

#ifdef TESTING
#include "enqueue.h"
void Set_Composite_Bounds(CompositeObject* cp);//for the present its here
#endif

//import Enqueue2;//if needed


//extern void AddEyeObjects(Object *obj, std::vector<Object>& eyeprims);
objlistptr eyeprimconvert(std::vector<Object> eyeprims, objlistptr objlist);
//extern void get_bounds(const Object& obj, bbox_info& box);
void BuildBoundingSlabs(BinTree&);



int getAxis(void);//to avoid warning, I'm declaring it here instead of in TESTING
#ifdef TESTING


int compslabs(void const* in_a, void const* in_b);

void FindAxis(Object** Prims, long first, long last);
//void FindAxis_CPP(std::vector<Object> Prims, long first, long last);
Flt SurfaceArea(Vec lengths);
void BuildAreaTable(Object** Prims, long a, long b, Flt* areas);

using ObjPtr = Object*;
int SortAndSplitTesting(ObjPtr& Root, Object **Prims, long *nPrims,
             long first, long last);
std::string BuildBoundingSlabsTesting(BinTree& Root);

#endif


//extern void get_bounds(Object *obj, bbox_info *box);
void get_bounds_CPP(Object& obj, bbox_info& box);
int calc_triangle_bounds(TriangleObject& tri_obj, bbox_info *box);
void recompute_bbox(bbox_info *, Transform *);
void recompute_inverse_bbox(bbox_info *, Transform *);
void recompute_inverse_bbox_CPP(bbox_info& bbox, Transform& trans);
void bbox_intersect(bbox_info *, bbox_info *, bbox_info *);
void bbox_union(bbox_info *, bbox_info *, bbox_info *);
int determine_start(Vec P, Vec D,  Flt bounds[2][3], Flt *min, Flt *max);
int BoundIntersect(Viewpoint *, BinTree& , Ray *, Flt, Flt, Isect *);
int BoundIntersectTester(BinTree& World, Ray* world_ray,
	Flt mindist, Flt maxdist, Isect* hit);
//extern void BuildBoundingSlabs(BinTree *);
void AddEyeObjects(Object *obj, objlistptr eyeprims);
void AddLightObjects(BinTree *Root);

void BuildAreaTestHarness();
void BuildAreaTableTestHarness();

#endif /* __POLYRAY_BOUNDS_DEFS */

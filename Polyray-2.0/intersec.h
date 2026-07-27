#if !defined(__POLYRAY_INTERSECT_DEFS)
#define __POLYRAY_INTERSECT_DEFS

#include "light.h"




/* Ray/object intersection support routines */
extern int Intersect(Viewpoint *, BinTree *, Ray *, Flt, Flt, Isect *);
extern int IntersectCPP(Viewpoint*, BinTree&, Ray*, Flt, Flt, Isect*);

extern int find_object_intersections(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
extern int find_object_intersectionsTester(Object* cobj, Ray* world_ray,
	Flt mindist, Flt maxdist, Isect* hit);

extern int Insert_Hit(Object*,  Vec,  Vec, Flt, const Vec, Isect*);



#endif /* __POLYRAY_INTERSECT_DEFS */

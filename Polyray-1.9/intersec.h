#if !defined(__POLYRAY_INTERSECT_DEFS)
#define __POLYRAY_INTERSECT_DEFS

#include "light.h"

/* Ray/object intersection support routines */
extern int Intersect(Viewpoint *, BinTree *, Ray *, Flt, Flt, Isect *);
extern int find_object_intersections(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
extern void transform_to_here(Object *obj, Vec W, Vec WP);
extern int Insert_Hit(Object *, Vec, Vec, Flt, Vec, Isect *);

#endif /* __POLYRAY_INTERSECT_DEFS */

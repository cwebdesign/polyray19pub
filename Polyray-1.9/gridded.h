#if !defined(__POLYRAY_GRIDDED_DEFS)
#define __POLYRAY_GRIDDED_DEFS

#include "defs3.h"

void GridDelete(Object* object);
int GridInside(Object* obj, Vec P);
int GridIntersect(Viewpoint* Eye, Object* obj, Ray* ray, Flt mindist, Flt maxdist, Isect* hit);


Object *MakeGrid(Object *, char *, ostackptr);


#endif /* __POLYRAY_GRIDDED_DEFS */


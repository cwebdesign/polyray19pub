#if !defined(__POLYRAY_ALIGHT_DEFS)
#define __POLYRAY_ALIGHT_DEFS

#include "defs3.h"


int PolygonLight(Viewpoint *eye, Light *light, PolyAlightData *alight,
             Flt tmin, Vec from, Vec total_color);

#endif /* __POLYRAY_ALIGHT_DEFS */

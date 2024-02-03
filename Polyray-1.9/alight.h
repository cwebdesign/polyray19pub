#if !defined(__POLYRAY_ALIGHT_DEFS)
#define __POLYRAY_ALIGHT_DEFS

typedef struct t_polyalightdata {
   /* Light specific information */
   int ures, vres;               /* # sample points wide/high */
   int adaptive_depth;           /* How deep to recurse when sampling */
   Vec lower_left, upper_right;  /* Bounds of the light emiting polygon */
   Vec ubasis, vbasis;           /* Basis vectors for the polygon */
   Vec *nbuf, *obuf;             /* Temp storage of shadow colors */
   Vec *sbuf1, *sbuf2;           /* Temp storage of shadow points */
   Flt jitter;                   /* Amount of jitter to apply to shadow rays */

   /* Basic polygon information */
   int npoints;
   fVec *points;             /* Polygon boundary (in world coordinates?) */
   fVec normal;
   float d;
   short u, v;
   } PolyAlightData;

extern int
PolygonLight(Viewpoint *eye, Light *light, PolyAlightData *alight,
             Flt tmin, Vec from, Vec total_color);


#endif /* __POLYRAY_ALIGHT_DEFS */

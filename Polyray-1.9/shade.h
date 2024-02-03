#if !defined(__POLYRAY_SHADE_DEFS)
#define __POLYRAY_SHADE_DEFS

/* Color and texture support routines */
extern Surface *find_surface(Viewpoint *, Object *, Texture *, Vec, Vec, Vec, fVec, int);
extern void Shade(Viewpoint *, Object *obj, Texture *, int level,
                  Flt weight, Flt ior, Vec I, Vec W, Vec N, Vec U, Vec col);
extern void ShadeSurface(Viewpoint *, Object *, Surface *, int, Flt,
                         Flt, Vec, Vec, Vec, Vec, Vec *);

#endif /* __POLYRAY_SHADE_DEFS */


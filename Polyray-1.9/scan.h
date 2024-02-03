#if !defined(__POLYRAY_SCAN_DEFS)
#define __POLYRAY_SCAN_DEFS

/* Polygon specific stuff - zbuffer, and shading routines */
extern void scan_convert(Viewpoint *, BinTree *Root, Object *,
                         Texture *, Poly *);
extern void render_prim(Viewpoint *eye, BinTree *Root, Object *pobj,
                        Object *obj);
extern void Uniform_Subdivide(Viewpoint *, BinTree *, Object *);
extern void PutPixel(Viewpoint *, int, int, Vec, Flt);
extern void BboxScreenSize(Viewpoint *eye, bbox_info *bbox, int *x, int *y);

#endif /* __POLYRAY_SCAN_DEFS */


#if !defined(__MCUBE_SCAN_DEFS)
#define __MCUBE_SCAN_DEFS

/* Marching cubes algorithm for converting volume data
   into triangles */
extern void MarchCubes(Viewpoint *, BinTree *Root,
                       int u_steps, int v_steps, int w_steps,
                       bbox_info *bound, Flt T,
                       Flt (*evaluate)(Object *, Vec),
                       int (*gradient)(Object *, Vec, Vec),
                       Object *obj);

#endif /* __MCUBE_SCAN_DEFS */

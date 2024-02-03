#if !defined(__SUBDIVIDE_SCAN_DEFS)
#define __SUBDIVIDE_SCAN_DEFS

/* Uniform subdivision of a primitive into triangles */
extern void Uniform_Subdivide(Viewpoint *, BinTree *, Object *);

/* Division of a polygon into triangles */
extern void Split_Polygon(int cnt, fVec *verts, int x_axis, int y_axis,
                          int *out_cnt, int **out_verts);

#endif /* __SUBDIVIDE_SCAN_DEFS */

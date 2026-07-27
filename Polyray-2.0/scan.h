

#if !defined(__POLYRAY_SCAN_DEFS)
#define __POLYRAY_SCAN_DEFS

#ifdef TESTING
void poly_obj_output(BinTree& Root, Object* obj, Poly* p);
void poly_scan(Viewpoint* eye, Object* obj, Texture* tex, Poly* p);

void copy_vertex(Vertex* v1, Vertex* v2);
void calculate_vertex_delta(Vertex* lp, Vertex* rp,
    Vertex* dx, float dt);
void add_vertex_delta(Vertex* pt, float frac, Vertex* dp);
void add_vertex_delta1(Vertex* pt, Vertex* dp);
int pixelproctesting(Viewpoint* eye, Object* obj, Texture* tex,
    int x, int y, Vertex* pt, int edge_flag);
int edgepixel(Viewpoint* eye, int x, int y);
void compute_line_values(Vertex* v1, Vertex* v2,
    int* x1, int* x2, int* y1, int* y2,
    int* dx, int* dy, int* sx, int* sy,
    float* dtdx, float* dtdy);
void edge_scan(Viewpoint* eye, Object* obj, Poly* p);
void edge_scantesting(Viewpoint* eye, Object* obj, Poly* p);
void poly_outline(Poly* p);
void csg_subdivide_loop(Object* obj, Vec W0, Vec W1, Vec Wres);
short slice_csg_triangle(Object* obj, Vec W0, Vec W1, Vec W2,
    short flag0, short flag1, short flag2, int depth);
short check_leg_lengths(Vec W0, Vec W1, Vec W2, Flt tolerance);
short subdiv_triangle(Object* obj, Vec W0, Vec W1, Vec W2, int depth);
void poly_raw_output(Object* obj, Poly* p);
void emit_raw_triangle(Vec W0, Vec W1, Vec W2);
int pixelproc(Viewpoint* eye, Object* obj, Texture* tex,
    int x, int y, Vertex* pt, int edge_flag);
#endif


/* Polygon specific stuff - zbuffer, and shading routines */
void scan_convert(Viewpoint *, BinTree *Root, Object *,
                         Texture *, Poly *);
void render_prim(Viewpoint *eye, BinTree *Root, Object *pobj,
                        Object *obj);

void PutPixel(Viewpoint *, int, int, Vec, Flt);
void BboxScreenSize(Viewpoint *eye, bbox_info *bbox, int *x, int *y);


#endif /* __POLYRAY_SCAN_DEFS */


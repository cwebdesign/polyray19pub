#if !defined(__SUBDIVIDE_SCAN_DEFS)
#define __SUBDIVIDE_SCAN_DEFS

#include <vector>
#include <array>

struct retUVSteps {
    int adaptiveUVstepsassigned{ 0 };
    int u_steps;
    int v_steps;
};


#ifdef TESTING
void displace_vertex(NODE_PTR displacement, Vertex* vert);
void average_normals(Poly& p);
int add_mesh_normal(fVec* V, int usteps, int vsteps,
    int i0, int j0,
    int i1, int j1,
    int i2, int j2,
    Vec N);
void smooth_mesh(fVec* V, fVec* N, int usteps, int vsteps);
void add_new_triangle(int m, int* out_cnt, int** out_verts);
void HandleTDiscConeCylinderUVSteps(int size, int& u_steps, int& v_steps);
void HandleTSphereUVSteps(int size, int& u_steps, int& v_steps);
retUVSteps recompute_uv_steps(Object& obj, int width, int height);
#endif



/* Uniform subdivision of a primitive into triangles */
void Uniform_Subdivide(Viewpoint *, BinTree *, Object *);

/* Division of a polygon into triangles */
void Old_Split_Polygon(int cnt, fVec *verts, int x_axis, int y_axis,
                              int *out_cnt, int **out_verts);
void Split_Polygon(int cnt, fVec* verts, int x_axis, int y_axis,
    int& out_cnt, int** out_verts);
void cpp_Split_Polygon(int cnt, fVec* verts, int x_axis, int y_axis,
    int& out_cnt, std::vector<std::array<int, 3>>& out_verts);



#endif /* __SUBDIVIDE_SCAN_DEFS */

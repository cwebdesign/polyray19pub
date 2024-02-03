#if !defined(__POLYRAY_HEIGHT_DEFS)
#define __POLYRAY_HEIGHT_DEFS

extern Object *MakeHeight(Object *, char *, int);
extern Object *MakeHeightFn(Object *, int, int, Flt, Flt, Flt, Flt,
                            NODE_PTR, int);
extern Object *MakeSphHeight(Object *object, char *filename, int smoothed,
                             Flt scale, Flt offset);
extern Object *MakeSphHeightFn(Object *object, int xsize, int zsize,
                               NODE_PTR fn, int smoothed, Flt scale,
                               Flt offset);
extern Object *MakeCylHeight(Object *object, char *filename, int smoothed,
                             Flt scale, Flt offset);
extern Object *MakeCylHeightFn(Object *object, int xsize, int zsize,
                               NODE_PTR fn, int smoothed,
                               Flt scale, Flt offset);

/* The "triangle" data structure is used to hold information about
   the individual pieces of a height field.  As each square in the
   field is examined, it is chopped into two right triangles & the
   ray is tested against these triangles.*/
typedef struct {
   Vec v1, v2, v3, N;
   Vec n1, n2, n3, Ni;
   Flt d, dist;
   char type;
   } triangle;

/* Actual data structure maintained for a height field.  This holds
   pointers to the elevation data, min/max information for the
   elevations, overall bounds for the height field, and a triangle
   queue.  For some height fields normal information is stored to do
   a smoothing of the resulting height field.

   As an optimization technique a queue of recently processed
   triangles is maintained, with the expectation that successive rays
   will very likely hit the same triangle.  This technique works best
   if the size of triangles is larger than one pixel in the resulting
   image.
*/
#define MAX_CACHE 8
typedef struct {
   int type;                  /* Type of field, 0 = normal, 1 = smooth */
   float **data;              /* Elevation information */
   fVec **norm;               /* Vertex normals  */
   Flt *phi_sin, *phi_cos;    /* sin and cos of phi for spherical hf's */
   Vec *theta_norms;          /* Normals to spherical hf's */
   int xsize, zsize;          /* # of points/side */
   triangle cache[MAX_CACHE]; /* Cache of nearby triangles */
   int last_cached;           /* Index of last triangle hit */
   int cache_length;          /* Number of cached triangles */
   float low, high;           /* low/high for the entire field */
   Flt boundbox[2][3];        /* Bounds for the entire height field */
   Transform *trans;          /* Transform to object space */
   } HeightData;

/* Routines common to more than one of the flat, spherical, and
   cylindrical height fields */
#define MAX_SPH_DIST 1.0e100

void smooth_height_field(HeightData *hf, int hf_type);
void read_height_data(char *filename, float offset, float scale,
                      float ***data, int *xsize, int *zsize,
                      float *low, float *high);
void HeightDelete(Object *object);
void indexed_cyl_to_cart(HeightData *hf, int u, int v, Vec P);
void indexed_geo_to_cart(HeightData *hf, int u, int v, Vec P);
void create_angle_tables(int hf_type, int u_steps, int v_steps,
                         Flt **phi_sin, Flt **phi_cos, Vec **theta_norms);
void init_u_variables(Flt thetas, Flt thetae,
                      Flt *dtheta, Flt *theta0,
                      int *u0, int *u1, int *u,
                      int u_steps, int dir_flag);
Flt get_theta_t(int u, Vec P, Vec D, int theta_dir_flag,
                Vec *theta_norms, Flt t, Flt mindist, Flt maxdist);
int intersect_square(int x, int z, HeightData *hf, int hf_type,
                     Vec D, Vec P, triangle *hit_tri);

#endif /* __POLYRAY_HEIGHT_DEFS */



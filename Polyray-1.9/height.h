#if !defined(__POLYRAY_HEIGHT_DEFS)
#define __POLYRAY_HEIGHT_DEFS

#include "defs3.h"

void HeightDelete(Object* object);
int HeightInside(Object* obj, Vec P);
int SphHeightInside(Object* obj, Vec P);



#ifdef __cplusplus
	extern "C" {
#endif

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



/* Routines common to more than one of the flat, spherical, and
   cylindrical height fields */
constexpr float MAX_SPH_DIST = 1.0e100;

void smooth_height_field(HeightData *hf, ShapeType hf_type);
void read_height_data(char *filename, float offset, float scale, 
         HeightData *hf);
void indexed_cyl_to_cart(HeightData *hf, int u, int v, Vec P);
void indexed_geo_to_cart(HeightData *hf, int u, int v, Vec P);
void create_angle_tables(ShapeType hf_type, int u_steps, int v_steps,
                         Flt **phi_sin, Flt **phi_cos, Vec **theta_norms);


//void HeightDelete(Object *object);
void init_u_variables(Flt thetas, Flt thetae,
                      Flt *dtheta, Flt *theta0,
                      int *u0, int *u1, int *u,
                      int u_steps, int dir_flag);
Flt get_theta_t(int u, Vec P, Vec D, int theta_dir_flag,
                Vec *theta_norms, Flt t, Flt mindist, Flt maxdist);
int intersect_square(int x, int z, HeightData *hf, ShapeType hf_type,
                     Vec D, Vec P, triangle *hit_tri);

#ifdef __cplusplus
}
#endif

#endif /* __POLYRAY_HEIGHT_DEFS */



#ifndef __LIGHT_DEFS
#define __LIGHT_DEFS
#include "defs3.h"

void LightDelete(Object* object);//moving it outside of extern C breaks linking

extern Light *Current_Light;


struct ShadRet {
    int i;
    NuVec ShadowVec;//SV
};
ShadRet Shadow(Viewpoint* eye, Light* light, Ray* ray,
    Flt tmin, Flt tmax,
    Flt radius);
//extern int Shadow(Viewpoint*, Light*, Ray*, Flt, Flt, Flt, Vec);

void Initialize_Light_Caches(void);
Object* Get_Light_Blocker(Light* light, int depth);
void Set_Light_Blocker(Light* light, int depth, Object* obj);
void Terminate_Light_Caches(void);
void Set_Light_Shadow(int);
void Set_Light_Color(NODE_PTR);
void Set_Light_Radius(Flt);
void Set_Light_Polygon(Flt, Flt, Flt, Flt);
void Transform_Light(Transform*);
void Shear_Light(Flt, Flt, Flt, Flt, Flt, Flt);

NuVec Get_Light_Pos(Light* light);
void Translate_Light(const NuVec v);
void Rotate_Light(NuVec);
void Rotate_Axis_Light(Vec, Flt);
void Scale_Light(Vec);
Light* light_action1(Vec, Vec);
Light* light_action2(Vec);
Light* light_action3(void);
Light* light_action4(Vec);
Light* light_action5(Vec, Vec);
Light* light_action6(void);
void DepthLight3(Vec);
void DepthLight6(Vec);
void DepthLight7(Vec);
Light* SetSpotLight(Vec, Vec, Vec, Flt, Flt, Flt);
 Flt Light_Color(Light* light, Vec W, Vec light_color,
    Vec light_pos, Flt* radius);
void Get_Light_Colors(Viewpoint* Eye, Vec W, Vec* light_colors);

void DepthLight1(Flt);
void DepthLight2(Flt);
void DepthLight4(NODE_PTR);
void DepthLight5(char*);

void DepthLight8(void);
void DepthLight9(Flt);


void Initialize_Lights(void);
void Deallocate_Lights(void);
void Add_To_Lights(Light* light);
Object* MakeLight(Object* object, Light* light);
Light* Copy_Light(Light* light, Transform* tx);

/* Lens flare entry points */
void Draw_Flares(Viewpoint *eye);
void Create_Lens_Flare(void);
void Set_Flare_Color(NODE_PTR color);
void Set_Flare_Count(int count);
void Set_Flare_Spacing(Flt spacing);
void Set_Flare_Seed(int seed);
void Set_Flare_Size(Flt min_rad, Flt max_rad);
void Set_Flare_Concave(Flt concave_ratio);
void Set_Flare_Sphere(Flt radius);



#endif /* __LIGHT_DEFS */

#ifndef __LIGHT_DEFS
#define __LIGHT_DEFS

extern Light *Current_Light;

extern int Shadow(Viewpoint *, Light *, Ray *, Flt, Flt, Flt, Vec);

extern void Initialize_Light_Caches(void);
extern Object *Get_Light_Blocker(Light *light, int depth);
extern void Set_Light_Blocker(Light *light, int depth, Object *obj);
extern void Terminate_Light_Caches(void);

extern void Get_Light_Pos(Light *light, Vec Pos);
extern void Set_Light_Shadow(int);
extern void Set_Light_Color(NODE_PTR);
extern void Set_Light_Radius(Flt);
extern void Set_Light_Polygon(Flt, Flt, Flt, Flt);
extern void Transform_Light(Transform *);
extern void Shear_Light(Flt, Flt, Flt, Flt, Flt, Flt);
extern void Translate_Light(Vec);
extern void Rotate_Light(Vec);
extern void Rotate_Axis_Light(Vec, Flt);
extern void Scale_Light(Vec);
extern Light *light_action1(Vec, Vec);
extern Light *light_action2(Vec);
extern Light *light_action3(void);
extern Light *light_action4(Vec);
extern Light *light_action5(Vec, Vec);
extern Light *light_action6(void);
extern void DepthLight1(Flt);
extern void DepthLight2(Flt);
extern void DepthLight3(Vec);
extern void DepthLight4(NODE_PTR);
extern void DepthLight5(char *);
extern void DepthLight6(Vec);
extern void DepthLight7(Vec);
extern void DepthLight8(void);
extern void DepthLight9(Flt);
extern Light *SetSpotLight(Vec, Vec, Vec, Flt, Flt, Flt);

extern Flt Light_Color(Light *light, Vec W, Vec light_color,
                       Vec light_pos, Flt *radius);
extern void Get_Light_Colors(Viewpoint *Eye, Vec W, Vec *light_colors);
extern void Initialize_Lights(void);
extern void Deallocate_Lights(void);
extern void Add_To_Lights(Light *light);
extern Object *MakeLight(Object *object, Light *light);
extern Light *Copy_Light(Light *light, Transform *tx);

/* Lens flare entry points */
extern void Draw_Flares(Viewpoint *eye);
extern void Create_Lens_Flare(void);
extern void Set_Flare_Color(NODE_PTR color);
extern void Set_Flare_Count(int count);
extern void Set_Flare_Spacing(Flt spacing);
extern void Set_Flare_Seed(int seed);
extern void Set_Flare_Size(Flt min_rad, Flt max_rad);
extern void Set_Flare_Concave(Flt concave_ratio);
extern void Set_Flare_Sphere(Flt radius);


#endif /* __LIGHT_DEFS */

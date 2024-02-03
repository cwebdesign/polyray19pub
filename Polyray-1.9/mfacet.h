#if !defined(__MFACET_DEFS)
#define __MFACET_DEFS

/* Microfacet functions */
extern float D_Phong_Init(Flt);
extern float D_Blinn_Init(Flt);
extern float D_Gaussian_Init(Flt);
extern float D_Reitz_Init(Flt);
extern float D_Cook_Init(Flt);
extern float D_Phong(Vec, Vec, Vec, Flt);
extern float D_Blinn(Vec, Vec, Vec, Flt);
extern float D_Gaussian(Vec, Vec, Vec, Flt);
extern float D_Reitz(Vec, Vec, Vec, Flt);
extern float D_Cook(Vec, Vec, Vec, Flt);

#endif /* __MFACET_DEFS */


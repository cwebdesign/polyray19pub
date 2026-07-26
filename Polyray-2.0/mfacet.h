#if !defined(__MFACET_DEFS)
#define __MFACET_DEFS

/* Microfacet functions */
float D_Phong_Init(Flt);
float D_Blinn_Init(Flt);
float D_Gaussian_Init(Flt);
float D_Reitz_Init(Flt);
float D_Cook_Init(Flt);
float D_Phong(Vec, Vec, Vec, Flt);
float D_Blinn(Vec, Vec, Vec, Flt);
float D_Gaussian(Vec, Vec, Vec, Flt);
float D_Reitz(Vec, Vec, Vec, Flt);
float D_Cook(Vec, Vec, Vec, Flt);

#endif /* __MFACET_DEFS */


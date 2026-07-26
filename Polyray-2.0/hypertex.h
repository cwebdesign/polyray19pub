#if !defined(__POLYRAY_HYPERTEXTURE_DEFS)
#define __POLYRAY_HYPERTEXTURE_DEFS

void HypertextureDelete(Object*);
int HypertextureInside(Object*, Vec);

#ifdef TESTING
int HypertextureNormal(Object* obj, Vec P, Vec N);
void InitializeHypertexture(Object* obj);
void diffuse_lighting(Viewpoint* Eye, Object* obj,
    Flt Kd_scale, fVec Kd_color,
    Vec W, Vec I, Vec dcolor);
#endif

#ifdef __cplusplus
	extern "C" {
#endif

extern Object *MakeHypertexture(Object *, NODE_PTR);

#ifdef __cplusplus
}
#endif

#endif /* __POLYRAY_HYPERTEXTURE_DEFS */


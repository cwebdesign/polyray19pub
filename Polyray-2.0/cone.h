#if !defined(__POLYRAY_CONE_DEFS)
#define __POLYRAY_CONE_DEFS

void ConeDelete(Object* object);

#ifdef __cplusplus
	extern "C" {
#endif

extern Object *MakeCone(Object *, Vec, Flt, Vec, Flt);

#ifdef __cplusplus
  }
#endif

#endif /* __POLYRAY_CONE_DEFS */


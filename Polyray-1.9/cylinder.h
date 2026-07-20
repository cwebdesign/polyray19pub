#if !defined(__POLYRAY_CYLINDER_DEFS)
#define __POLYRAY_CYLINDER_DEFS

void CylinderDelete(Object* object);


#ifdef __cplusplus
	extern "C" {
#endif

extern Object *MakeCylinder(Object *, Vec, Vec, Flt);

#ifdef __cplusplus
  }
#endif

#endif /* __POLYRAY_CYLINDER_DEFS */


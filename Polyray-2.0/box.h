#if !defined(__POLYRAY_BOX_DEFS)
#define __POLYRAY_BOX_DEFS


extern void BoxDelete(Object* object);
#ifdef __cplusplus
	extern "C" {
#endif

extern Object *MakeBox(Object *, Vec, Vec);

#ifdef __cplusplus
  }
#endif

#endif /* __POLYRAY_BOX_DEFS */


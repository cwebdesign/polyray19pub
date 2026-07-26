

#if !defined(__POLYRAY_BEZIER_DEFS)
#define __POLYRAY_BEZIER_DEFS


void BezierDelete(Object* object);
void NurbDelete(Object*);

#ifdef __cplusplus
	extern "C" {
#endif

extern Object *MakeBezier(Object *, int, Flt, int, int, VList *);
extern Object *MakeNurb(Object *, int, int, int, int,
                        NODE_PTR, NODE_PTR, NODE_PTR);
extern Object *MakeNurbTrimmed(Object *, int, int, int, int,
                               NODE_PTR, NODE_PTR, NODE_PTR, NODE_PTR);
extern void BezierNormalTest(void);

#ifdef __cplusplus
  }
#endif

#endif /* __POLYRAY_BEZIER_DEFS */

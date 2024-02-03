#if !defined(__POLYRAY_BEZIER_DEFS)
#define __POLYRAY_BEZIER_DEFS

extern Object *MakeBezier(Object *, int, Flt, int, int, VList *);
extern Object *MakeNurb(Object *, int, int, int, int,
                        NODE_PTR, NODE_PTR, NODE_PTR);
extern void BezierNormalTest(void);

#endif /* __POLYRAY_BEZIER_DEFS */


#if !defined(__POLYRAY_IMPLICIT_DEFS)
#define __POLYRAY_IMPLICIT_DEFS

extern Object *MakeFunction(Object *, NODE_PTR);

extern void
Compute_Step_Values(Vec deltas, int sizes[3], Vec D,
                    int steps[3], int highs[3], float dx[3]);
extern void
Compute_DDA_Start(Vec deltas, int sizes[3], bbox_info *bbox,
                  Vec hitpos, Vec D, int x[3], Flt fx[3]);

#endif /* __POLYRAY_IMPLICIT_DEFS */


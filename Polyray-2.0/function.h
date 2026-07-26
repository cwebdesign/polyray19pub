#if !defined(__POLYRAY_IMPLICIT_DEFS)
#define __POLYRAY_IMPLICIT_DEFS

void FunctionDelete(Object*);
int FunctionInside(Object*, Vec);//needed for testing


#ifdef TESTING
int FunctionNormal(Object* obj, Vec P, Vec N);
void InitializeFunction(Object* obj);
int check_fn_hit(Object* obj, Ray* ray, Isect* hit,
    Flt mindist, Flt maxdist,
    Vec Low, Vec High, Flt* lastf);

#endif


#ifdef __cplusplus
	extern "C" {
#endif
extern Object *MakeFunction(Object *, NODE_PTR);

extern void
Compute_Step_Values(Vec deltas, int sizes[3], Vec D,
                    int steps[3], int highs[3], float dx[3]);
extern void
Compute_DDA_Start(Vec deltas, int sizes[3], bbox_info *bbox,
                  Vec hitpos, Vec D, int x[3], Flt fx[3]);

#ifdef __cplusplus
}
#endif

#endif /* __POLYRAY_IMPLICIT_DEFS */


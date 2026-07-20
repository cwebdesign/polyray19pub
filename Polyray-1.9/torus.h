#if !defined(__POLYRAY_TORUS_DEFS)
#define __POLYRAY_TORUS_DEFS


void TorusDelete(Object* object);

Object *MakeTorus(Object *, Flt, Flt, Vec, Vec);
void Set_Torus_Solver(Object *, int);


#endif /* __POLYRAY_TORUS_DEFS */


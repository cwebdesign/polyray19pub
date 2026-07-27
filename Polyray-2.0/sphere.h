#if !defined(__POLYRAY_SPHERE_DEFS)
#define __POLYRAY_SPHERE_DEFS

void SphereDelete(Object* object);

#ifdef TESTING
int SphereNormal(SphereData* sp, Vec P, Vec N);
void SphereUV(Vec Pos, Vec C, Flt* u, Flt* v);
void Ellipse_Evaluator(Object*, Flt, Flt, Vertex*);
int SphereInside(Object* obj, Vec P);

#endif//TESTING

int SphereIntersect(Viewpoint*, Object*, Ray*, Flt, Flt, Isect*);


Object *MakeSphere(Object *, Vec, Flt);


#endif /* __POLYRAY_SPHERE_DEFS */


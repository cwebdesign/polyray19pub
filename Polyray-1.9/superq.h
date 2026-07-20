#if !defined(__POLYRAY_SUPERQ_DEFS)
#define __POLYRAY_SUPERQ_DEFS

void SuperQDelete(Object* object);


Object *MakeSuperq(Object *, Flt, Flt);
void SuperQ_Evaluator(Object*, Flt, Flt, Vertex*);






#endif /* __POLYRAY_SUPERQ_DEFS */

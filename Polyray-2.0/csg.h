#if !defined(__POLYRAY_CSG_DEFS)
#define __POLYRAY_CSG_DEFS

void CSGDelete(Object*);

int Inside_CSG_Node(csgnodeptr node, Vec W);


Object *MakeCSG(Object *, csgnodeptr);
int Inside_CSG_Nodes(csgnodeptr, Vec);
void set_parent_ptrs(csgnodeptr, csgnodeptr, Object *, Transform *,
                            bbox_info *);
void instantiate_csg(BinTree *, csgnodeptr, int);

#endif /* __POLYRAY_CSG_DEFS */


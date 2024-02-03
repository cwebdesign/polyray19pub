#if !defined(__POLYRAY_CSG_DEFS)
#define __POLYRAY_CSG_DEFS

extern Object *MakeCSG(Object *, csgnodeptr);
extern int Inside_CSG_Node(csgnodeptr, Vec);
extern int Inside_CSG_Nodes(csgnodeptr, Vec);
extern void set_parent_ptrs(csgnodeptr, csgnodeptr, Object *, Transform *,
                            bbox_info *);
extern void instantiate_csg(BinTree *, csgnodeptr, int);

#endif /* __POLYRAY_CSG_DEFS */


#if !defined(__POLYRAY_SIMPLIFY_DEFS)
#define __POLYRAY_SIMPLIFY_DEFS

extern NODE_PTR simplify(NODE_PTR, int);
extern LIST_PTR collect_additive_terms(NODE_PTR);

#endif /* __POLYRAY_SIMPLIFY_DEFS */


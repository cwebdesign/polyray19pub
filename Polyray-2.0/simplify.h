#if !defined(__POLYRAY_SIMPLIFY_DEFS)
#define __POLYRAY_SIMPLIFY_DEFS



void HandleLeftValExper(NODE_PTR node);

void HandleLeftNotVal(NODE_PTR& node, NODE_PTR tnode);

 NODE_PTR simplify(NODE_PTR, int);
 LIST_PTR collect_additive_terms(NODE_PTR);



#endif /* __POLYRAY_SIMPLIFY_DEFS */


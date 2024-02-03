#if !defined(__POLYRAY_SPLINE_DEFS)
#define __POLYRAY_SPLINE_DEFS

extern NODE_PTR make_spline_node(NODE_PTR type, NODE_PTR param,
                                 NODE_PTR ctl_points, NODE_PTR ctl_params);
extern void show_spline_node(NODE_PTR node);
extern void deallocate_spline_node(NODE_PTR node);
extern void *copy_spline_node(void *);
extern int eval_spline(SUBST_PTR subst, NODE_PTR node, Vec vval);

#endif /* __POLYRAY_SPLINE_DEFS */

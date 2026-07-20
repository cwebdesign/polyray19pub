#if !defined(__POLYRAY_SPLINE_DEFS)
#define __POLYRAY_SPLINE_DEFS

  


NODE_PTR make_spline_node(NODE_PTR type, NODE_PTR param,
                                 NODE_PTR ctl_points, NODE_PTR ctl_params);
void show_spline_node(NODE_PTR node);
void deallocate_spline_node(NODE_PTR node);
spline_node *copy_spline_node(spline_node*);
int eval_spline(SUBST_PTR subst, NODE_PTR node, Vec vval);



#endif /* __POLYRAY_SPLINE_DEFS */

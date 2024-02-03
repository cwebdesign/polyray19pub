#if !defined(__POLYRAY_BUILDER_DEFS)
#define __POLYRAY_BUILDER_DEFS

/* Expression manipulation functions */
extern NODE_PTR make_node(int, NODE_PTR, NODE_PTR);
extern NODE_PTR make_value_node(Flt);
extern NODE_PTR make_value_term_node(Flt);
extern NODE_PTR make_fn1_node(int, NODE_PTR);
extern NODE_PTR make_fn2_node(int, NODE_PTR, NODE_PTR);
extern NODE_PTR make_fn3_node(int, NODE_PTR, NODE_PTR, NODE_PTR);
extern NODE_PTR make_cond_node(NODE_PTR, NODE_PTR, NODE_PTR);
extern NODE_PTR make_vector_node(LIST_PTR);
extern NODE_PTR make_vec_node(Flt, Flt, Flt);
extern NODE_PTR make_vector3_node(NODE_PTR, NODE_PTR, NODE_PTR);
extern NODE_PTR make_vector4_node(NODE_PTR, NODE_PTR, NODE_PTR, NODE_PTR);
extern NODE_PTR make_cmap_node(map_entries, NODE_PTR);
extern NODE_PTR make_image_node(char *, NODE_PTR);
extern NODE_PTR make_environ_node(char *, char *, char *,
                                  char *, char *, char *);
extern NODE_PTR make_assignment_node(char *, NODE_PTR);
extern LIST_PTR make_list_node(NODE_PTR);
extern NODE_PTR make_array_node(LIST_PTR);
extern NODE_PTR make_string_node(char *);
extern NODE_PTR copy_node(NODE_PTR);
extern void show_node(NODE_PTR);
extern NODE_PTR simplify(NODE_PTR, int);
extern void deallocate_list(LIST_PTR);
extern void deallocate_node(NODE_PTR);
extern void deallocate_cmap_node(map_entries);
extern void delete_draw_nodes(DrawNode *);
extern DrawNode *make_draw_node(Flt, Flt, int, NODE_PTR, NODE_PTR);

#endif /* __POLYRAY_BUILDER_DEFS */


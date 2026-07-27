
#if !defined(__POLYRAY_BUILDER_DEFS)
#define __POLYRAY_BUILDER_DEFS

#ifdef TESTING

map_entries copy_cmap_node(map_entries cnode);
LIST_PTR copy_list(LIST_PTR entries);
NODE_PTR make_image_nodemock(char* filename, NODE_PTR support);
NODE_PTR make_environ_nodemock(char* file0, char* file1, char* file2,
	char* file3, char* file4, char* file5);
std::string lookup_fn(int fntype);
void show_cmap_node(map_entries cnode);
void show_array_node(LIST_PTR list);


#endif//TESTING

#include <memory>
extern NODE_PTR make_string_node(std::string); //char *);
//extern std::unique_ptr<exper_str> copy_string_node_new(std::unique_ptr<exper_str> ptr);

	//extern NODE_PTR simplify(NODE_PTR, int);//in simplify? conflicts


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
extern NODE_PTR copy_node_void(void *);
extern void show_node(NODE_PTR);

extern void deallocate_list(LIST_PTR);
extern void deallocate_node(NODE_PTR);
extern void deallocate_cmap_node(map_entries);
extern void delete_draw_nodes(DrawNode *);
extern DrawNode *make_draw_node(Flt, Flt, int, NODE_PTR, NODE_PTR);



#endif /* __POLYRAY_BUILDER_DEFS */


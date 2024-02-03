#if !defined(__POLYRAY_PSUPPORT_DEFS)
#define __POLYRAY_PSUPPORT_DEFS

/* Parser support variables */
extern fVec *pl, *plist;
extern ostackptr Object_Stack;
extern tstackptr Texture_Stack;
extern blobstackptr blob_components;
extern blobstackptr blob_component;
extern int condition_flags[];
extern int condition_depth;
extern Transform *Current_Transform;
extern Special_Surface *CurrentSurface;
extern int npoints;
extern int ObjectDepth;
extern txstackptr txstack;
extern UVVert tri_vertex[3];

/* Parser support functions */
extern void ReadSceneFile(char *str);
extern void TransformObject(Object *, Transform *);
extern void RotateObject(Object *, Vec);
extern void RotateAxisObject(Object *, Vec, Flt);
extern void TranslateObject(Object *, Vec);
extern void ShearObject(Object *, Flt, Flt, Flt, Flt, Flt, Flt);
extern void ScaleObject(Object *, Vec);
extern void InitializeSurface(Surface *);
extern void InitializeSpecialSurface(Special_Surface *NewSurf);
extern void build_outfile_name(char *outfilebase, char *outfilename);
extern int check_condition(void);
extern void surface_action1(void);
extern void surface_action2(char *);
extern void push_texture(Texture *text);
extern Transform * transform_action1(void);
extern Transform * transform_action2(char *);
extern void translate_transform(Transform *, Vec);
extern void rotate_transform(Transform *, Vec);
extern void axis_rotate_transform(Transform *, Vec, Flt);
extern void scale_transform(Transform *, Vec);
extern Texture *pop_texture(void);
extern Texture *texture_action1(void);
extern Texture *texture_action2(char *);
extern tstackptr texture_list_action1(Texture *);
extern tstackptr texture_list_action2(tstackptr, Texture *);
extern LIST_PTR expression_action1(LIST_PTR, NODE_PTR);
extern NODE_PTR exper_action(char *);
extern void polynomial_action1(NODE_PTR data, int solver);
extern csgnodeptr make_csg_node(int, void *, void *);
extern void csg_action1(csgnodeptr);
extern VList *add_bezier_point(VList *points, Vec point);
extern Object *object_action1(void);
extern Object *object_action2(char *);
extern void haze_action(Flt, Flt, Vec);
extern void color_action(Special_Surface *, NODE_PTR);
extern void ambient_action(Special_Surface *, NODE_PTR, NODE_PTR);
extern void color_map_action(Special_Surface *, map_entries, NODE_PTR);
extern void diffuse_action(Special_Surface *, NODE_PTR, NODE_PTR);
extern void brilliance_action(Special_Surface *, NODE_PTR);
extern void lookup_function_action(Special_Surface *, NODE_PTR);
extern void microfacet_action(Special_Surface *, int, NODE_PTR);
extern void normal_action(Special_Surface *, NODE_PTR);
extern void position_action(Special_Surface *, NODE_PTR);
extern void octaves_action(Special_Surface *, NODE_PTR);
extern void frequency_action(Special_Surface *, NODE_PTR);
extern void bump_scale_action(Special_Surface *, NODE_PTR);
extern void phase_action(Special_Surface *, NODE_PTR);
extern void position_function_action(Special_Surface *, NODE_PTR);
extern void position_scale_action(Special_Surface *, NODE_PTR);
extern void reflection_action(Special_Surface *, NODE_PTR, NODE_PTR);
extern void specular_action(Special_Surface *, NODE_PTR, NODE_PTR);
extern void transmission_action(Special_Surface *, NODE_PTR, NODE_PTR,NODE_PTR);
extern void turbulence_action(Special_Surface *, NODE_PTR);
extern void background_action(NODE_PTR);
extern void flush_action(int);
extern map_entries map_entry_action1(Flt, Flt, Vec, Flt, Vec, Flt);
extern map_entries map_entry_action2(map_entries, map_entries);
extern void root_solver_action(Object *, int);
extern NODE_PTR check_term(char *, LIST_PTR);
extern NODE_PTR check_term0(char *);
extern void spherical_component_action(Vec, Flt, Flt);
extern void cylindrical_component_action(Vec, Vec, Flt, Flt);
extern void planar_component_action(Vec, Flt, Flt, Flt);
extern void toroidal_component_action(Vec, Vec, Flt, Flt, Flt);
extern char *translate_string(char *);
extern char *build_string(LIST_PTR);
extern void evaluate_system_call(LIST_PTR);
extern void push_tx(Transform *);
extern void pop_tx(void);
extern texture_fn_entries make_texture_fn_entry(NODE_PTR, Texture *);
extern texture_fn_entries texture_fn_action2(texture_fn_entries,
                                             texture_fn_entries);
extern texture_map_entries make_texture_map_entry(Flt, Flt, Texture *,
                                                  Texture *);
extern texture_map_entries copy_texture_map(texture_map_entries map);
extern texture_map_entries texture_map_action1(char *);
extern texture_map_entries texture_map_action2(texture_map_entries,
                                               texture_map_entries);
extern int create_string(NODE_PTR, char **);
extern void draw_action(Flt low, Flt high, int steps,
                        NODE_PTR points, NODE_PTR color);

#endif /* __POLYRAY_PSUPPORT_DEFS */


#if !defined(__POLYRAY_EVAL_DEFS)
#define __POLYRAY_EVAL_DEFS

extern int eval_node(SUBST_PTR, NODE_PTR, Flt *, Vec, NODE_PTR *);
extern int eval_node_dx(SUBST_PTR, NODE_PTR, Flt *, Vec);
extern int eval_colormap(map_entries, Vec, Flt, Vec, Flt *);
extern Flt polyray_random(void);
extern int spherical_imagemap(Vec, Flt *, Flt *);
extern Flt Kaos(Vec P, Flt pos_scale, Flt noise_scale, int octaves);
extern void dKaos(Vec P, Vec D, Flt pos_scale, Flt noise_scale, int octaves);
extern void dnoise3d(Vec P, Vec D, Flt pos_scale, Flt noise_scale, int octaves);
extern Flt fnoise(Vec P, Flt pos_scale, Flt noise_scale, int octaves);
extern Flt sawtooth(Flt);
extern Flt ramp(Flt);
extern void ripples(Vec, Vec, Flt, Flt, Flt);
extern int Check_Visibility(Vec start, Vec end);

#endif /* __POLYRAY_EVAL_DEFS */


#if !defined(__POLYRAY_EVAL_DEFS)
#define __POLYRAY_EVAL_DEFS


#ifdef TESTING
Flt xpow(Flt in, Flt power);
NuVec color_wheel(Flt x, Flt z);

void init_ripples(void);
extern int ripple_init_flag;
int eval_ripplemock(SUBST_PTR subst, NODE_PTR node, Vec& vval);
Flt hash3d(unsigned long x, unsigned long y,
	unsigned long z);
Flt flt_noise(Vec P);
void Perlin_init(void);
Flt Perlin_noise(Vec P);
#ifdef __cplusplus
extern "C" {
#endif
void brownian_motion(Vec start, int cycles, Vec scale, Vec end);
void brownian_motionmock(Vec start, int cycles, Vec scale, Vec end);
int eval_brownian_motionmock(SUBST_PTR subst, NODE_PTR node, Vec& vval);
void calculate_uv(int map_type, Vec vleft, Flt* u, Flt* v);
int eval_imagemapmock(int map_type, SUBST_PTR subst, NODE_PTR node, NODE_PTR nodeleft, Img* image,
	Flt* fval, Vec& vval);
int eval_bumpmap(int map_type, SUBST_PTR subst, NODE_PTR node,
	Vec vval);
int eval_environment_mapmock(SUBST_PTR subst, NODE_PTR node,
	Flt* fval, Vec vval);
int eval_plusmock(SUBST_PTR subst, NODE_PTR node, Flt* fval, Vec vval);
int eval_minusmock(SUBST_PTR subst, NODE_PTR node, Flt* fval, Vec vval);
int eval_timesmock(SUBST_PTR subst, NODE_PTR node, Flt* fval, Vec vval);
int eval_divmock(SUBST_PTR subst, NODE_PTR node, Flt* fval, Vec vval);
int eval_powermock(SUBST_PTR subst, NODE_PTR node, Flt* fval, Vec vval);
static int evaluateColourMap(SUBST_PTR subst, NODE_PTR node, Flt& cindex, Vec& vtmp, NODE_PTR* tnode, map_entries& temp, Vec& default_color, Flt& ftmp, Vec& vval, Flt* fval, bool& retFlag);
static int evaluateArray(SUBST_PTR subst, NODE_PTR node, Flt& ftmp, Vec& vtmp, NODE_PTR* tnode, LIST_PTR& list, int& i, Flt* fval, Vec vval);
static int evaluateSimpleSubscript(SUBST_PTR subst, NODE_PTR node, NODE_PTR* tnode, Flt* fval, bool& retFlag);
int eval_subscript(SUBST_PTR subst, NODE_PTR node,
	Flt* fval, Vec vval, NODE_PTR* tnode);
static int evaluateSubscriptExpression(SUBST_PTR subst, NODE_PTR node, NODE_PTR* tnode, Flt& ftmp, Flt* fval, NODE_PTR& tarray, LIST_PTR& list, Vec vval, bool& retFlag);
int eval_subscriptmock(SUBST_PTR subst, NODE_PTR node,
	Flt* fval, Vec vval, NODE_PTR* tnode);
int Check_VisibilityMock(Vec start, Vec end);

#ifdef __cplusplus
	}
#endif
int doevaluateColourMapmock(SUBST_PTR subst, NODE_PTR node, Flt& cindex, Vec& vtmp,
	NODE_PTR* tnode, map_entries& temp, Vec& default_color,
	Flt& ftmp, NuVec& vval, Flt* fval, bool& retFlag);
Flt legendre(int l, int m, Flt x);
int cylindrical_imagemap(Vec P, Flt* u, Flt* v);

#endif

#ifdef __cplusplus
	extern "C" {
#endif
extern int eval_node(SUBST_PTR, NODE_PTR, Flt *, Vec, NODE_PTR *);
extern int eval_node_dx(SUBST_PTR, NODE_PTR, Flt *, Vec);
extern int eval_colormap(map_entries, Vec, Flt, Vec, Flt *);
//extern Flt polyray_random(void);
extern int spherical_imagemap(Vec, Flt *, Flt *);
extern Flt Kaos(Vec P, Flt pos_scale, Flt noise_scale, int octaves);
extern void dKaos(Vec P, Vec D, Flt pos_scale, Flt noise_scale, int octaves);
extern void dnoise3d(Vec P, Vec D, Flt pos_scale, Flt noise_scale, int octaves);
extern Flt fnoise(Vec P, Flt pos_scale, Flt noise_scale, int octaves);
extern Flt sawtooth(Flt);
extern Flt ramp(Flt);
extern void ripples(Vec, Vec, Flt, Flt, Flt);
extern int Check_Visibility(Vec start, Vec end);

#ifdef __cplusplus
  }
#endif

#endif /* __POLYRAY_EVAL_DEFS */

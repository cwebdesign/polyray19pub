#if !defined(__ROOT_SOLVER_DEFS)
#define __ROOT_SOLVER_DEFS

int Inside_Contour_CPP(Flt x, Flt y, int itype,
    int n, const std::vector<NuVec>& points);

int  solve_linear(Flt *, Flt *, Flt, Flt);
int  solve_quadratic(Flt *, Flt *, Flt, Flt);
int  solve_cubic(Flt *, Flt *, Flt, Flt);

// Polynomial solver entry points 

extern int  solve_quartic(Flt *, Flt *, Flt, Flt);
extern int  solve_quartic1(Flt *, Flt *, Flt, Flt);
extern int  bounded_polysolve(int, Flt *, Flt *, Flt, Flt);
extern int  Inside_Polygon(Flt, Flt, int, fVec *, int, int);
extern int  Inside_Contour(Flt x, Flt y, int itype, int n, fVec *points);
extern long binomial(int, int);
extern int  binomials[15][15];

#ifdef TESTING
extern int factor_out_test(int n, int i, int& c, int *s);
extern void factor1_test(int n, int& c, int *s);
extern int modp_test(int u_ord, const LFlt *u_coef, int v_ord, const LFlt *v_coef,
                     int *r_ord, LFlt *r_coef);
extern int buildsturm_test(int ord, const LFlt *coef, int *derived_ord, LFlt *derived_coef);
extern LFlt polyeval_test(LFlt x, int n, const LFlt *Coeffs);
extern int numchanges_test(int ord, const LFlt *coef, LFlt a);
extern int regula_falsa_test(int order, const LFlt *coef, LFlt a, LFlt b, Flt *val);
extern int newton_raphson_guarded_test(int order, const LFlt *coef, LFlt a, LFlt b, Flt *val);
extern int sbisect_test(int ord, const LFlt *coef, LFlt min, LFlt max, Flt *roots);
extern int difficult_coeffs_test(int n, Flt *x);
#endif


#endif /* __ROOT_SOLVER_DEFS */

#if !defined(__ROOT_SOLVER_DEFS)
#define __ROOT_SOLVER_DEFS

/* Polynomial solver entry points */
extern int  solve_linear(Flt *, Flt *, Flt, Flt);
extern int  solve_quadratic(Flt *, Flt *, Flt, Flt);
extern int  solve_cubic(Flt *, Flt *, Flt, Flt);
extern int  solve_quartic(Flt *, Flt *, Flt, Flt);
extern int  solve_quartic1(Flt *, Flt *, Flt, Flt);
extern int  bounded_polysolve(int, Flt *, Flt *, Flt, Flt);
extern int  Inside_Polygon(Flt, Flt, int, fVec *, int, int);
extern int  Inside_Contour(Flt x, Flt y, int itype, int n, fVec *points);
extern long binomial(int, int);
extern int  binomials[15][15];

#endif /* __ROOT_SOLVER_DEFS */


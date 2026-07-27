#if !defined(__POLYRAY_POLYNOMIAL_DEFS)
#define __POLYRAY_POLYNOMIAL_DEFS

void PolynomialDelete(Object*);
int PolynomialInside(Object*, Vec);

Object *MakeNewPolynomial(Object *, int, Flt *, int);
void Set_Polynomial_Solver(Object *, int);


#endif /* __POLYRAY_POLYNOMIAL_DEFS */


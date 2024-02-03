#if !defined(__VECTOR_DEFS)
#define __VECTOR_DEFS

/* Give prototypes of various vector/matrix/polynomial functions. */
extern Flt polyray_random(void);
extern void VecH(Vec, Vec, Vec);
extern float fVecNormalize(fVec);
extern Flt VecNormalize(Vec);
extern void MZero(Matrix);
extern void MIdentity(Matrix);
extern void MTimes(Matrix, Matrix, Matrix);
extern void MTimesOriginal(Matrix, Matrix, Matrix);
extern void MStrassen2(Matrix2, Matrix2, Matrix2);
extern void MStrassen2Mat(Matrix2, Matrix2, Matrix2, Matrix2, Matrix2,Matrix2,Matrix2,Matrix2, Matrix2,Matrix2,Matrix2,Matrix2);
extern void MAdd(Matrix, Matrix, Matrix);
extern void MSub(Matrix, Matrix, Matrix);
extern void MScale(Matrix, Matrix, Flt);
extern void MTranspose(Matrix, Matrix);
extern void fTxVec(fVec, fVec, Transform *);
extern void TxVec(Vec, Vec, Transform *);
extern void TxVec3(Vec, Vec, Transform *);
extern void fTxNormal(fVec, fVec, Transform *);
extern void TxNormal(Vec, Vec, Transform *);
extern void InvTxVec(Vec, Vec, Transform *);
extern void InvTxVec1(Vec, Vec, Transform *);
extern void InvTxVec3(Vec, Vec, Transform *);
extern void InvTxNormal(Vec, Vec, Transform *);
extern Transform *Get_Transformation(void);
extern void Get_Scaling_Transformation(Transform *, Vec);
extern void Get_Translation_Transformation(Transform *, Vec);
extern void Get_Rotation_Transformation(Transform *, Vec);
extern void Get_Shear_Transformation(Transform *, Flt, Flt, Flt, Flt,
                                     Flt, Flt);
extern void Compose_Transformations(Transform *, Transform *);
extern void Get_Coordinate_Transform(Transform *trans, Vec origin,
                                    Vec up, Flt r, Flt len);
extern void Get_Rotate_Transform(Transform *trans, Vec V, Flt angle);
extern void SpecularDirection(Vec, Vec, Vec);
extern int  TransmissionDirection(Flt, Flt, Vec, Vec, Vec);
extern void Get_Perspective_Transformation(Transform *, Flt);
extern Transform *Normalize_View(Viewpoint *);

#define phi2v(s, p)   ((int)floor((double)((s)-1)*((p)/M_PI+0.5)))
#define v2phi(s, v)   ((((double)(v)/(double)((s)-1))-0.5)*M_PI)
#define theta2u(s, t) ((int)floor((double)((s)-1)*(t)/(2.0*M_PI)))
#define u2theta(s, u) ((((double)(u)/(double)((s)-1)))*2.0*M_PI)

/* Trig functions good to 8 digits of accuracy */
void geocentric_to_cartesian(Vec Q, Vec P);
void cartesian_to_geocentric(Vec P, Vec Q);
void cylindrical_to_cartesian(Vec Q, Vec P);
void cartesian_to_cylindrical(Vec P, Vec Q);

#endif /* __VECTOR_DEFS */

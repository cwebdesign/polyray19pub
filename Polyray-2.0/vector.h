#pragma once
#if !defined(__VECTOR_DEFS)
#define __VECTOR_DEFS

#include <memory>


extern void C_InvTxVector1(Vec& out, Vec vec, Transform& tx);
extern void C_TxVector3(Vec& out, Vec vec, Transform& tx);
extern void VecH(const Vec a, const Vec b, Vec& c);
extern float fVecNormalize(fVec&);
void InvTxVec1(Vec& out, Vec vec, Transform* tx);
void Get_Coordinate_TransformCPP(Transform& trans, Vec& origin, NuVec& up, Flt r, Flt len);
void Get_Translation_TransformationCPP(Transform& tx, const NuVec vector);
void Get_Rotation_Transformation_CPP(Transform& tx, NuVec vector);
void Get_Rotate_Transform_CPP(Transform& trans, NuVec& V, Flt angle);
void InvTxVec_CPP(Vec& out, Vec vec, Transform& tx);
std::unique_ptr<Transform> Normalize_View(Viewpoint *eye);
std::unique_ptr<Transform> Get_Transformation();
void Compose_Transformations(Transform& tx0, Transform &tx1);
void Get_Scaling_Transformation_CPP(Transform& tx, NuVec vector);
void Get_Perspective_Transformation_CPP(Transform&, Flt);
void Get_Translation_Transformation_CPP(Transform& tx, NuVec vector);

/* Give prototypes of various vector/matrix/polynomial functions. */
extern Flt polyray_random();
//extern void VecH(Vec, Vec, Vec);
//extern float fVecNormalize(fVec);
extern Flt VecNormalize(Vec);
Flt VecNuNormalize(NuVec& vec);
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
//extern void fTxNormal(fVec, fVec, Transform *);
NuVec fTxNormal(fVec vec, Transform* tx);
extern void TxNormal(Vec, Vec, Transform *);
extern void InvTxVec(Vec, Vec, Transform *);

extern void InvTxVec3(Vec, Vec, Transform *);
extern void InvTxNormal(Vec, Vec, Transform *);

extern void Get_Scaling_Transformation(Transform *, Vec);
extern void Get_Translation_Transformation(Transform *, Vec);
extern void Get_Rotation_Transformation(Transform *, Vec);
extern void Get_Shear_Transformation(Transform *, Flt, Flt, Flt, Flt,
                                     Flt, Flt);

extern void Get_Rotate_Transform(Transform *trans, Vec V, Flt angle);
extern void SpecularDirection(Vec, Vec, Vec);
extern int  TransmissionDirection(Flt, Flt, Vec, Vec, Vec);
extern void Get_Perspective_Transformation(Transform *, Flt);

#define phi2v(s, p)   ((int)floor((double)((s)-1)*((p)/PYM_PI+0.5)))
#define v2phi(s, v)   ((((double)(v)/(double)((s)-1))-0.5)*PYM_PI)
#define theta2u(s, t) ((int)floor((double)((s)-1)*(t)/(2.0*PYM_PI)))
#define u2theta(s, u) ((((double)(u)/(double)((s)-1)))*2.0*PYM_PI)

/* Trig functions good to 8 digits of accuracy */
void geocentric_to_cartesian(Vec Q, Vec P);
void cartesian_to_geocentric(Vec P, Vec Q);
void cylindrical_to_cartesian(Vec Q, Vec P);
void cartesian_to_cylindrical(Vec P, Vec Q);

#endif /* __VECTOR_DEFS */

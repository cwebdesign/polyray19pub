#if !defined(__POLYRAY_GLYPH_DEFS)
#define __POLYRAY_GLYPH_DEFS

#include "defs3.h"

#ifdef TESTING
void Split_Contour(Viewpoint* eye, BinTree* Root, Object* obj,
	int count, cpointPtr* contour_points, int* flags);
int Point_Segment_Test(Flt x, Flt y, Flt x0, Flt y0, Flt x1,
	Flt y1);
int Inside_Glyph(GlyphData* glyph, Flt x, Flt y);
void GetZeroOneHits(GlyphData* glyph, Vec P, Vec D, Flt* t0,
	Flt* t1);
void Linear_Contour_Evaluater(Object* obj, Flt x0, Flt x1, Flt y0, Flt y1,
	Flt u, Flt v, Vertex* vert);
void Quadratic_Contour_Evaluater(Object* obj, Flt xt0,
	Flt xt1, Flt xt2,
	Flt yt0, Flt yt1, Flt yt2,
	Flt u, Flt v, Vertex* vert);
cpointPtr alloc_cpoint(float x, float y);
void Push_Point(cpointPtr* contour_points, int i, Flt x, Flt y);
void Insert_Point(cpointPtr cpoints, Flt x, Flt y);
cpointPtr Rightmost(cpointPtr contour);
Flt WindingNumber(cpointPtr contour);
void ReverseContour(cpointPtr contour);
Flt SegmentAngle(Flt dx0, Flt dy0, Flt dx1, Flt dy1);
void Create_Point_Contours(Object* obj,
	cpointPtr* contour_points, int* flags);
void Contour_Orientations(int count, cpointPtr* contour_points, int* flags);
void Concatenate_Contours(int* count, cpointPtr* contour_points, int* flags);
void Deallocate_Contours(int count, cpointPtr* contour_points, int* flags);
void Render_Glyph_Faces(Viewpoint* eye, BinTree* Root, Object* obj);
void Render_Glyph_Sides(Viewpoint* eye, BinTree* Root, Object* obj);

#endif

void GlyphDelete(Object* object);

#ifdef __cplusplus
	extern "C" {
#endif

extern Object *MakeGlyph(Object *, int, Contour *);

#ifdef __cplusplus
}
#endif

#endif /* __POLYRAY_GLYPH_DEFS */


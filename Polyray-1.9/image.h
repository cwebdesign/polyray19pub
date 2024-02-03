#if !defined(__POLYRAY_IMAGE_DEFS)
#define __POLYRAY_IMAGE_DEFS

#define POLY_CLIP_OUT 0         /* polygon entirely outside box */
#define POLY_CLIP_PARTIAL 1     /* polygon partially inside */
#define POLY_CLIP_IN 2          /* polygon entirely inside box */

/* Routines to manage the image we are building:
      allocating buffers,
      get/set zbuffer values
      get/set pixel values
*/

/* Polygon specific stuff: zbuffer, clipping, and shading routines */
extern int poly_clip_to_box(Poly *p1, Poly_box *box);
extern void Allocate_Scan_Buffers(Viewpoint *eye, Pic *pic, int y_start, int y_end);
extern void Initialize_Clipping(Viewpoint *eye, int y_start, int y_end);
extern void Destroy_Scan_Buffers(Viewpoint *);
extern void Put_Pixel(Viewpoint *, int, int, Vec, Flt);
extern void Get_Pixel(Viewpoint *, int, int, Vec, Flt *);
extern float ZBuffer_Read(Viewpoint *eye, int x, int y);
extern void ZBuffer_Write(Viewpoint *eye, int x, int y, float z);
extern void DoDrawing(Viewpoint *eye, DrawNode *nodes);
extern float tx_point(Transform *tx, Vec P, fVec S);
extern void draw_point(Viewpoint *eye, float x, float y, float z,
                       Vec C, Flt opac);
extern void draw_2dline(Viewpoint *eye, fVec P0, fVec C0, float opac0,
                        fVec P1, fVec C1, float opac1);

/* Image file routines */
extern void quantize_depth(float depth, Vec color, Flt *opacity);
extern void FreeImg(Img *img);
extern Img *TGAReadImage(char *);
extern int lookup_image_color(Img *image, Flt x, Flt y, int rflag,
                              Flt *opac, Vec color);
extern int lookup_height(Img *image, Flt x, Flt y, int rflag, Flt *height);
extern int lookup_index(Img *image, Flt x, Flt y, int rflag, Flt *index);
extern float image_height(Img *image, int x, int y);

#endif /* __POLYRAY_IMAGE_DEFS */


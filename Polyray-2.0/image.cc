/* image.cc

  Polyray - MIT Licensed Revival
  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Copyright (C) 1999-2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", (C), WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <expected>
#include <optional>
#include <string>

#include "defs3.h"
#include "memory.h"
#include "io_ply.h"
#include "vector.h"
#include "runtime_state.h"
#include "symtab.h"
#include "eval.h"
#include "display.h"
#include "bound.h"
#include "builder.h"
#include "light.h"
#include "jpeg.h"
#include "pic.h"
#include "factory.h"
#include "image.h"
#include "pngread.h"
#include "image_decoder.h"
#include "util.h"

static unsigned char idbuf[256];     /* Read the id field */

/**
 * @brief Set up the 3-D and 2-D screen clipping windows and the view vector.
 *
 * Initialises the global `pbox` (3-D homogeneous clip box) and `win` (2-D
 * screen clip rectangle) for the given horizontal scanline range, and computes
 * the normalised viewing direction `ViewVec` from the eye's from/at points.
 *
 * @param eye Viewpoint providing resolution and from/at points.
 * @param y_start First scanline of the clipping window.
 * @param y_end Last scanline of the clipping window.
 * @return No return value.
 */
void Initialize_Clipping(Viewpoint *eye, int y_start, int y_end)
{
   /* 3-D screen clipping window */
   pbox.x0 = 0;
   pbox.x1 = (Flt)eye->view_xres + 1;
   pbox.y0 = y_start;
   pbox.y1 = y_end + 1;
   pbox.z0 = SMALL; /* eye->view_hither; */
   pbox.z1 = PLY_HUGE;

   /* 2-D screen clipping window */
   win.x0 = pbox.x0;
   win.x1 = pbox.x1-1;
   win.y0 = pbox.y0;
   win.y1 = pbox.y1;

   VecSub(eye->view_at, eye->view_from, ViewVec);
   VecNormalize(ViewVec);
}

/**
 * @brief Plot a single pixel on the display from 8-bit RGB components.
 *
 * @param x Pixel column.
 * @param y Pixel row.
 * @param r Red component (0-255).
 * @param g Green component (0-255).
 * @param b Blue component (0-255).
 * @return No return value.
 */
static void display_old_pixel(int x, int y,
                  unsigned char r, unsigned char g, unsigned char b)
{
   Vec color;
   MakeVector((Flt)r / 255.0, (Flt)g / 255.0, (Flt)b / 255.0, color);
   display_plot(x, y, color);
}

/**
 * @brief Redraw a full scanline on the display from the screen buffer.
 *
 * @param eye Viewpoint whose `SBuffer` supplies the pixel colors.
 * @param y Scanline row to redraw.
 * @return No return value.
 */
static void display_old_line(Viewpoint *eye, int y)
{
   int x;
   /* CM added (unsigned) cast 19/1/2002 */
   for (x=0;(unsigned)x<eye->view_xres;x++)
      display_old_pixel(x, y, eye->SBuffer[y][x].r,
                        eye->SBuffer[y][x].g,
                        eye->SBuffer[y][x].b);
}

/**
 * @brief Encode a depth value into an RGB color plus opacity.
 *
 * When the render pixel size is 32 bits (and `float`/`unsigned char` have the
 * expected sizes) the raw floating-point depth bytes are packed directly into
 * the color and opacity channels. Otherwise the depth is clamped to [0,255]
 * and split across the RGB channels as a fixed-point fraction with opacity 1.
 *
 * @param depth Depth value to encode.
 * @param color Receives the encoded RGB color.
 * @param opacity Receives the encoded opacity.
 * @return No return value.
 */
void quantize_depth(float depth, Vec color, Flt *opacity)
{
   unsigned char r, g, b;
   unsigned char *byteptr;

   if (runtimeState::settings.pixelsize == 32 && sizeof(float) == 4 &&
       sizeof(unsigned char) == 1) {
      /* Store as a floating point number - this is obviously
         a machine specific result for the floating point
         number that is stored.  There is an assumption here
         that a "float" type is exactly 4 bytes and that the
         size of an "unsigned char" is exactly 1 byte. */
      byteptr = (unsigned char *)&depth;
      MakeVector(((float)byteptr[2] + 0.5)/ 255.0,
                 ((float)byteptr[1] + 0.5)/ 255.0,
                 ((float)byteptr[0] + 0.5)/ 255.0,
                 color);
      *opacity = ((float)byteptr[3] + 0.5)/ 255.0;
      }
   else {
      if (depth > 255.0)
         depth = 255.0;
      else if (depth < 0.0)
         depth = 0.0;
      r = (unsigned char)depth;
      g = (unsigned char)((depth - (float)r) * 256.0);
      b = (unsigned char)((depth - ((float)r + (float)g / 256.0)) * 256.0 * 256.0);
      MakeVector((float)r / 255.0, (float)g / 256.0, (float)b / 256.0, color)
      *opacity = 1.0;
      }
}

/**
 * @brief Encode a depth value into an RGB color plus opacity.
 *
 * When the render pixel size is 32 bits (and `float`/`unsigned char` have the
 * expected sizes) the raw floating-point depth bytes are packed directly into
 * the color and opacity channels. Otherwise the depth is clamped to [0,255]
 * and split across the RGB channels as a fixed-point fraction with opacity 1.
 *
 * @param depth Depth value to encode.
 * @param color Receives the encoded RGB color.
 * @param opacity Receives the encoded opacity.
 * @return No return value.
 */
void quantize_depth_CPP(float depth, NuVec& color, Flt& opacity)
{
   unsigned char r, g, b;
   unsigned char *byteptr;

   if (runtimeState::settings.pixelsize == 32 && sizeof(float) == 4 &&
       sizeof(unsigned char) == 1) {
      /* Store as a floating point number - this is obviously
         a machine specific result for the floating point
         number that is stored.  There is an assumption here
         that a "float" type is exactly 4 bytes and that the
         size of an "unsigned char" is exactly 1 byte. */
      byteptr = (unsigned char *)&depth;
      color = { ((float)byteptr[2] + 0.5)/ 255.0,
                 ((float)byteptr[1] + 0.5)/ 255.0,
                 ((float)byteptr[0] + 0.5)/ 255.0
      };
      opacity = ((float)byteptr[3] + 0.5)/ 255.0;
      }
   else {
      if (depth > 255.0)
         depth = 255.0;
      else if (depth < 0.0)
         depth = 0.0;
      r = (unsigned char)depth;
      g = (unsigned char)((depth - (float)r) * 256.0);
      b = (unsigned char)((depth - ((float)r + (float)g / 256.0)) * 256.0 * 256.0);
      color = { (float)r / 255.0, (float)g / 256.0,
          (float)b / 256.0
      };
      opacity = 1.0;
      }
}

/**
 * @brief Fill one scanline of the screen buffer with the background.
 *
 * Depending on the render mode this writes a depth-quantised color, a wire
 * frame / hidden-line clear color, an evaluated background expression, or the
 * flat background color into every pixel of the given row of `eye->SBuffer`.
 *
 * @param eye Viewpoint whose `SBuffer` row is filled.
 * @param row Scanline to fill.
 * @param xres Number of columns to write.
 * @param yres Total vertical resolution (used to normalise the sample point).
 * @return Empty on success, or an @ref ErrImage on an unresolved background
 *         expression.
 */
static
std::expected<void,ErrImage> set_background(Viewpoint *eye, int row, int xres, int yres)
{
   Flt opacity;
   Vec color;
   struct subst_struct subst;
   Flt ftemp;
   NODE_PTR tnode;

   opacity = 0.0;
   for (int col=0;col<xres;col++) {
      if (runtimeState::settings.DepthRender)
         quantize_depth((runtimeState::settings.pixelsize == 32 ? PLY_HUGE : 256.0), color, &opacity);
      else if (runtimeState::settings.Render_Method == rmode::WIRE_FRAME ||
               runtimeState::settings.Render_Method == rmode::HIDDEN_LINE)
#if defined( MAC )
         MakeVector(1, 1, 1, color)
#else
         MakeVector(0, 0, 0, color)
#endif
      else if (RuntimeState::Background != nullptr) {
         subst.P[0] = (Flt)col / (Flt)xres;
         subst.P[1] = 0.0;
         subst.P[2] = (Flt)(yres - row) / (Flt)yres;
         MakeVector(subst.P[0], subst.P[2], 0, subst.U);
         MakeVector(0, 0, 0, subst.PT);
         MakeVector(0.0, 0.0, 0.0, subst.W);
         /* Fix N to be direction of ray through this pixel */
         MakeVector(0.0, 0.0, 0.0, subst.N);
         MakeVector(0.0, 0.0, 0.0, subst.I);
         if (eval_node(&subst, RuntimeState::Background, &ftemp, color, &tnode) != 2)
             return std::unexpected(ErrImage::ErrUnresolvedBackgroundExpression);   
         //serror("Unresolved background expression\n");
         }
      else {
         VecCopy(RuntimeState::BackgroundColor, color);
         }
      eye->SBuffer[row][col].r = (int)(255.0 * color[0]);
      eye->SBuffer[row][col].g = (int)(255.0 * color[1]);
      eye->SBuffer[row][col].b = (int)(255.0 * color[2]);
      eye->SBuffer[row][col].o = (int)(255.0 * opacity);
      }
      return {};
}

/**
 * @brief Paint the background over a block of the display.
 *
 * For non ray-tracing renders with no background expression, selects the
 * appropriate clear color for the current render mode and fills the display
 * region spanning the given rows with a single box.
 *
 * @param color Scratch vector used to hold the chosen clear color.
 * @param ybeg First row of the block.
 * @param xres Horizontal extent of the block.
 * @param yres Vertical resolution, clamped against @p yend.
 * @param yend Last row of the block.
 * @return No return value.
 */
void PaintBackground(Vec& color, int ybeg, int xres, int yres, int yend)
{
    if (Global::Display_Flag != 0 && runtimeState::settings.Render_Method != rmode::RAY_TRACING &&
        RuntimeState::Background == nullptr) {
        if (runtimeState::settings.DepthRender)
            MakeVector(1, 1, 0, color)
        else if (runtimeState::settings.Render_Method == rmode::WIRE_FRAME ||
            runtimeState::settings.Render_Method == rmode::HIDDEN_LINE)
#if defined( MAC )
            MakeVector(1, 1, 1, color)
#else
            MakeVector(0, 0, 0, color)
#endif
        else
            VecCopy(RuntimeState::BackgroundColor, color)

            display_box(0, ybeg, xres, PLY_MIN(yres, yend), color);
    }
}

/**
 * @brief Allocate memory for the screen and depth buffers.
 *
 * Allocates the per-row Z-buffer and screen (S-)buffer for the scanline range
 * [@p ybeg, @p yend] (plus the extra row/column needed for filter
 * antialiasing), initialising the depth to `PLY_HUGE` and painting or reusing
 * background pixels as appropriate. Also allocates the edge tracking arrays.
 *
 * @param eye Viewpoint that receives the allocated buffers.
 * @param pic Optional picture used to reuse pixels from a resumed render; may
 *        be `NULL`.
 * @param ybeg First scanline to allocate.
 * @param yend Last scanline to allocate.
 * @return Empty on success, or an @ref ErrRow describing the failed allocation
 *         or unresolved background expression.
 */
std::expected<void, ErrRow> Allocate_Scan_Buffers(Viewpoint* eye, Pic* pic, int ybeg, int yend)
{
    Vec color;
    int col, xres, yres;

    /* If we are doing filter antialiasing then we need an extra pixel in
       both the x and y directions */
    xres = eye->view_xres + 1;
    yres = eye->view_yres + 1;

    /* Paint the background */
    PaintBackground(color, ybeg, xres, yres, yend);

    /* Allocate image/depth buffers and repaint old lines */
    eye->ZBuffer = (float**)polyray_malloc(yres * sizeof(float*));
    if (eye->ZBuffer == nullptr)
        return std::unexpected(ErrRow{ErrImage::ErrFailedToAllocateZBufferMoreMemNeeded, 0});
      //serror("Failed to allocate the Z-Buffer (more memory is needed)\n");
   for (int row=0;row<yres;row++) {
      if (row < ybeg || row > yend)
         eye->ZBuffer[row] = nullptr;
      else {
         eye->ZBuffer[row] = (float*)polyray_malloc(xres * sizeof(float));
         if (eye->ZBuffer[row] == nullptr)
             return std::unexpected(ErrRow{ErrImage::ErrFailedToAllocateRowOfZBuffer, row});
            //serror("Failed to allocate row %d of Z-Buffer (try -M 256 command line option)\n", row);
         if (row == ybeg)
            for (col=0;col<xres;col++)
               eye->ZBuffer[row][col] = PLY_HUGE;
         else
            memcpy(eye->ZBuffer[row], eye->ZBuffer[ybeg], xres * sizeof(float));
         }
      }

   eye->SBuffer = (rgbo**)polyray_malloc(yres * sizeof(rgbo *));
   if (eye->SBuffer == nullptr)
       return std::unexpected(ErrRow{ ErrImage::ErrFailedToAllocateSBuffer, 0 });
      //serror("Failed to allocate the S-Buffer (try -M 256 command line option)\n");
   for (int row=0;row<yres;row++) {
      if (row < ybeg || row > yend)
         eye->SBuffer[row] = nullptr;
      else {
         eye->SBuffer[row] = (rgbo*)polyray_malloc(xres * sizeof(rgbo));
         if (eye->SBuffer[row] == nullptr)
            serror("Failed to allocate row %d of S-Buffer (try -M 256 command line option)\n", row);

         if ((pic != NULL) && (pic->resume != NULL) &&
             get_old_image_line(eye, pic, row)) {
            /* Got pixels from the old image file and will reuse them */
            if (Global::Display_Flag != 0)
               display_old_line(eye, row);
            if (runtimeState::settings.Render_Method != rmode::RAY_TRACING &&
                row >= eye->view_ystart &&
                row <= eye->view_yend) {
                   auto ret=set_background(eye, row, xres, yres);
                   if (!ret.has_value())
                       return std::unexpected(ErrRow{ ErrImage::ErrUnresolvedBackgroundExpression, row });
                }
            }
         else if (runtimeState::settings.Render_Method != rmode::RAY_TRACING) {
            if (row == ybeg || RuntimeState::Background != NULL) {
               auto ret=set_background(eye, row, xres, yres);
               if (!ret.has_value())
                   return std::unexpected(ErrRow{ ErrImage::ErrUnresolvedBackgroundExpression, row });
            }
            else
               memcpy(eye->SBuffer[row], eye->SBuffer[ybeg], xres * sizeof(rgbo));
            if (Global::Display_Flag != 0 && RuntimeState::Background != NULL)
               display_old_line(eye, row);
            }
         }
      }

   eye->edgey = (int *)polyray_malloc((eye->view_xres + 2) * sizeof(int));
   eye->edgex = (int *)polyray_malloc((eye->view_yres + 4) * sizeof(int));
   return {};
}

/**
 * @brief Free the screen and depth buffers allocated by Allocate_Scan_Buffers.
 *
 * Releases every row of the S-buffer and Z-buffer, the buffer arrays
 * themselves, and the edge tracking arrays, clearing the pointers on @p eye.
 *
 * @param eye Viewpoint whose buffers are released.
 * @return No return value.
 */
void Destroy_Scan_Buffers(Viewpoint *eye)
{
   int row;

   if (eye->SBuffer != nullptr) {
	  /* CM added (unsigned) cast 19/1/2002 */
      for (row=0;(unsigned)row<=eye->view_yres;row++)
         if (eye->SBuffer[row] != nullptr) {
            polyray_free(eye->SBuffer[row]);
            }
      polyray_free(eye->SBuffer);
      eye->SBuffer = nullptr;
      }
   if (eye->ZBuffer != nullptr) {
	  /* CM added (unsigned) cast 19/1/2002 */
      for (row=0;(unsigned)row<=eye->view_yres;row++)
         if (eye->ZBuffer[row] != nullptr)
            polyray_free(eye->ZBuffer[row]);
      polyray_free(eye->ZBuffer);
      eye->ZBuffer = nullptr;
      }
   if (eye->edgey != NULL)
      polyray_free(eye->edgey);
   if (eye->edgex != NULL)
      polyray_free(eye->edgex);
}

/**
 * @brief Read the stored depth at a pixel.
 *
 * @param eye Viewpoint whose Z-buffer is queried.
 * @param x Pixel column.
 * @param y Pixel row.
 * @return The stored depth, or `PLY_HUGE` / `-PLY_HUGE` when the buffer is
 *         absent or the coordinate is out of bounds.
 */
float ZBuffer_Read(Viewpoint *eye, int x, int y)
{
   if (eye->ZBuffer == NULL)
      return PLY_HUGE;

   if (eye->ZBuffer[y] == NULL) {
      swarning("Out of bounds pixel: (%d,%d)\n", x, y);
      return -PLY_HUGE;
      }

   /* CM added (unsigned) casts 19/1/2002 */
   if (x < 0 || (unsigned) x > eye->view_xres ||
       y < 0 || (unsigned) y > eye->view_yres) {
      swarning("Bad coordinate (%d, %d) in zbuffer_read\n", x, y);
      return PLY_HUGE;
      }

   return eye->ZBuffer[y][x];
}

/**
 * @brief Store a depth value at a pixel.
 *
 * No-op when the Z-buffer is absent; warns/errors on out-of-bounds access.
 *
 * @param eye Viewpoint whose Z-buffer is written.
 * @param x Pixel column.
 * @param y Pixel row.
 * @param z Depth value to store.
 * @return No return value.
 */
void
ZBuffer_Write(Viewpoint *eye, int x, int y, float z)
{
   if (eye->ZBuffer == NULL)
      return;

   if (eye->ZBuffer[y] == NULL) {
      swarning("Out of bounds pixel: (%d,%d)\n", x, y);
      return;
      }

   /* CM added (unsigned) casts 19/1/2002 */
   if (x < 0 || (unsigned) x > eye->view_xres ||
       y < 0 || (unsigned) y > eye->view_yres)
      serror("Bad coordinate (%d, %d) in zbuffer_write\n", x, y);
   eye->ZBuffer[y][x] = z;
}

/**
 * @brief Write a color and opacity to a pixel of the screen buffer.
 *
 * Converts the floating-point color and opacity to clamped 8-bit components
 * before storing them in `eye->SBuffer`. No-op when the buffer is absent.
 *
 * @param eye Viewpoint whose screen buffer is written.
 * @param x Pixel column.
 * @param y Pixel row.
 * @param color RGB color to store.
 * @param opacity Opacity to store.
 * @return No return value.
 */
void
Put_Pixel(Viewpoint *eye, int x, int y, Vec color, Flt opacity)
{
  int i;

   if (eye->SBuffer == NULL)
      return;

   if (eye->SBuffer[y] == NULL) {
      swarning("Out of bounds pixel: (%d,%d)\n", x, y);
      return;
      }

   /* CM added (unsigned) casts 19/1/2002 */
   if (x < 0 || (unsigned)x > eye->view_xres || y < 0 || (unsigned)y > eye->view_yres)
      serror("Bad coordinate (%d, %d)\n", x, y);

  i = 255.0 * color[2];
  if (i<0) i=0;
  else if (i>=256) i = 255;
  eye->SBuffer[y][x].b = i;

  i = 255.0 * color[1];
  if (i<0) i=0;
  else if (i>=256) i = 255;
  eye->SBuffer[y][x].g = i;

  i = 255.0 * color[0];
  if (i<0) i=0;
  else if (i > 255) i = 255;
  eye->SBuffer[y][x].r = i;

  i = 255.0 * opacity;
  if (i<0) i=0;
  else if (i>255) i = 255;
  eye->SBuffer[y][x].o = i;
}

/**
 * @brief Read the color and opacity of a pixel from the screen buffer.
 *
 * Converts the stored 8-bit components back to floating point. On an
 * out-of-bounds coordinate returns black with opacity 1; no-op when the buffer
 * is absent.
 *
 * @param eye Viewpoint whose screen buffer is queried.
 * @param x Pixel column.
 * @param y Pixel row.
 * @param color Receives the RGB color.
 * @param opacity Receives the opacity.
 * @return No return value.
 */
void
Get_Pixel(Viewpoint *eye, int x, int y, Vec color, Flt *opacity)
{
   if (eye->SBuffer == NULL)
      return;

   if (eye->SBuffer[y] == NULL) {
      swarning("Out of bounds pixel: (%d,%d)\n", x, y);
      return;
      }

   /* First make sure it's within the entire image */
   /* CM added (unsigned) cast 19/1/2002 */
   if (x < 0 || (unsigned)x > eye->view_xres || y < 0 || (unsigned)y > eye->view_yres) {
      /* No, return dummy values */
      swarning("Bad coordinate (%d, %d)\n", x, y);
      MakeVector(0, 0, 0, color);
      *opacity = 1.0;
      return;
      }

   color[0] = eye->SBuffer[y][x].r / 255.0;
   color[1] = eye->SBuffer[y][x].g / 255.0;
   color[2] = eye->SBuffer[y][x].b / 255.0;
   *opacity = eye->SBuffer[y][x].o / 255.0;
}


/**
 * @brief Clip a convex polygon against a single half-space.
 *
 * Copies the portion of @p p satisfying `sign*s[index] < k*sw` into @p q,
 * where `s` is a Vertex cast as an array of `Flt`. @p index selects sx, sy or
 * sz (screen-space x, y or z). Thus, to clip against xmin use
 * `poly_clip_to_halfspace(p, q, XINDEX, -1., -xmin)`, and to clip against
 * xmax use `poly_clip_to_halfspace(p, q, XINDEX, 1., xmax)`.
 *
 * @param p Input polygon.
 * @param q Receives the clipped polygon.
 * @param index Vertex-coordinate index to test (screen x, y or z).
 * @param sign Sign selecting the min (-1) or max (+1) plane.
 * @param k Plane position in the tested coordinate.
 * @return No return value.
 */
static void
poly_clip_to_halfspace(Poly *p, Poly *q, int index,
                       Flt sign, Flt k)
{
   int i;
   Vertex *u, *v, *wp;
   Flt t, tu, tv;
   Vec V;

   q->n = 0;
   /* start with u=vert[n-1], v=vert[0] */
   u = &p->vertices[p->n-1];
   v = &p->vertices[0];
   tu = sign * u->S[index] - u->w * k;
   for (i=p->n; i>0; i--, u=v, tu=tv, v++) {
      /* on old polygon (p), u is previous vertex,v is the
       * current vertex tv is negative if vertex v is in */
      tv = sign * v->S[index] - v->w * k;
      if (tu <= 0.0 ^ tv <= 0.0) {
         /* edge crosses plane; add intersection point to q */
         t = tu / (tu - tv);
         wp = &q->vertices[q->n];

         wp->w = u->w + t * (v->w - u->w);
         VecSub(v->S, u->S, V);
         VecAddScaled(u->S, t, V, wp->S);
         VecSub(v->W, u->W, V);
         VecAddScaled(u->W, t, V, wp->W);
         if (runtimeState::settings.Render_Method == rmode::GOURAD_SHADE ||
             runtimeState::settings.Render_Method == rmode::SCAN_CONVERSION) {
            VecSub(v->P, u->P, V);
            VecAddScaled(u->P, t, V, wp->P);
            VecSub(v->N, u->N, V);
            VecAddScaled(u->N, t, V, wp->N);
            VecSub(v->U, u->U, V);
            VecAddScaled(u->U, t, V, wp->U);
            }
         q->n++;
         }

      if (tv <= 0.0)  /* vertex v is in, copy it to q */
         q->vertices[q->n++] = *v;
      }
}

std::optional<PolyClip> CLIP_AND_SWAP(int index, Flt sign, Flt k,
    Poly* p, Poly* q, Poly* r, Poly* p1)
{
    poly_clip_to_halfspace(p, q, index, sign, sign * k);
    if (q->n == 0) { p1->n = 0; return PolyClip::PolyClipOut; }
    r = p; p = q; q = r;
    return std::nullopt;
}

/**
 * @brief Clip a convex polygon to a screen-space box.
 *
 * Clips @p p1 to @p box using each vertex's homogeneous screen coordinates
 * (sx, sy, sz, sw), testing `v->sx/v->sw` against the box x-range and similarly
 * for y and z. When the polygon is cut by the box @p p1 is modified in place.
 *
 * @param p1 Polygon to clip; modified in place when partially clipped.
 * @param box Screen-space clipping box.
 * @return `POLY_CLIP_IN` if entirely inside, `POLY_CLIP_OUT` if entirely
 *         outside, or `POLY_CLIP_PARTIAL` if cut by the box.
 */
PolyClip poly_clip_to_box(Poly *p1, Poly_box *box)
{
   int x0out = 0, x1out = 0;
   int y0out = 0, y1out = 0;
   int z0out = 0, z1out = 0;
   int i;
   Vertex *v1,*v2;
   Poly p2, *p, *q, *r;

   /* count vertices "outside" with respect to each
      of the six planes */
   for (v1=&(p1->vertices[0]), i=p1->n; i>0; i--, ++v1) {
      if (v1->S[0] < box->x0 * v1->w) x0out++; /* out on left */
      if (v1->S[0] > box->x1 * v1->w) x1out++; /* out on right */
      if (v1->S[1] < box->y0 * v1->w) y0out++; /* out on top */
      if (v1->S[1] > box->y1 * v1->w) y1out++; /* out on bottom */
      if (v1->S[2] < box->z0 * v1->w) z0out++; /* out on near */
      if (v1->S[2] > box->z1 * v1->w) z1out++; /* out on far */
      }

   /* Check if all vertices inside */
   if (x0out+x1out+y0out+y1out+z0out+z1out == 0)
      return PolyClip::PolyClipIn;

   /* Check if all vertices are "outside" any of the six planes */
   if (x0out==p1->n || x1out==p1->n || y0out==p1->n ||
       y1out==p1->n || z0out==p1->n || z1out==p1->n) {
      p1->n = 0;
      return PolyClip::PolyClipOut;
      }

   /* Clip against each of the planes that might cut the polygon,
      at each step toggling between polygons p1 and p2 */
   p = p1;
   q = &p2;
   if (x0out) { auto ret = CLIP_AND_SWAP(0, -1.0, box->x0, p, q, r, p1); if (ret.has_value()) return ret.value(); }
   if (x1out) { auto ret = CLIP_AND_SWAP(0,  1.0, box->x1, p, q, r, p1); if (ret.has_value()) return ret.value(); }
   if (y0out) { auto ret = CLIP_AND_SWAP(1, -1.0, box->y0, p, q, r, p1); if (ret.has_value()) return ret.value(); }
   if (y1out) { auto ret = CLIP_AND_SWAP(1,  1.0, box->y1, p, q, r, p1); if (ret.has_value()) return ret.value(); }
   if (z0out) { auto ret = CLIP_AND_SWAP(2, -1.0, box->z0, p, q, r, p1); if (ret.has_value()) return ret.value(); }
   if (z1out) { auto ret = CLIP_AND_SWAP(2,  1.0, box->z1, p, q, r, p1); if (ret.has_value()) return ret.value(); }
   /* if result ended up in p2 then copy it to p1 */
   if (p == &p2) {
      p1->n = p2.n;
      for (i=0,v1=&(p2.vertices[0]),v2=&(p1->vertices[0]);
           i<p2.n;
           i++,v1++,v2++)
         *v2 = *v1;
      }
   return PolyClip::PolyClipPartial;
}

/**
 * @brief Draw and depth-test a single evaluated point into the image.
 *
 * Rejects points outside the clip window. When the point passes the depth
 * test it optionally re-encodes the color for depth renders, composites over
 * the existing pixel if the opacity is below 1, writes the pixel to the output
 * buffer and/or display, and updates the depth buffer.
 *
 * @param eye Viewpoint whose buffers/display are updated.
 * @param x Screen x coordinate.
 * @param y Screen y coordinate.
 * @param z Depth of the point.
 * @param C Color of the point (may be modified for depth renders/compositing).
 * @param opac Opacity of the point (defined "backwards", i.e. 0 == opaque).
 * @return No return value.
 */
void
draw_point(Viewpoint *eye, float x, float y, float z, Vec C, Flt opac)
{
   Flt ftemp, opac1;
   Vec C1;
   unsigned char r, g, b;


   if (x < win.x0 || x >= win.x1 ||
       y < win.y0 || y >= win.y1)
      return;

   /* Put the evaluated color at the given drawing location */
   if (z <= ZBuffer_Read(eye, x, y)) {
/* message("Draw <%g, %g> with color <%g,%g,%g>\n",
       x, y, C[0], C[1], C[2]); */
      /* If this is a depth render then modify the colors
         to reflect how the depth is stored */
      if (RuntimeState::settings.DepthRender) {
         ftemp = z;
         if (ftemp > 255.0)
            ftemp = 255.0;
         else if (ftemp < 0.0)
            ftemp = 0.0;
         r = (unsigned char)ftemp;
         g = (unsigned char)((ftemp - (float)r) * 256.0);
         b = (unsigned char)((ftemp - ((float)r + (float)g / 256.0)) * 256.0 * 256.0);
         MakeVector((float)r / 255.0, (float)g / 256.0, (float)b / 256.0, C)
         }

      /* If the opacity of the pixel is less than 1 then
         we need to include background color into the final
         color */
      opac = 1.0 - opac; /* Opacity is defined backwards in data files */
      if (opac < 1.0) {
         Get_Pixel(eye, x, y, C1, &opac1);
         VecComb(opac, C, 1.0 - opac, C1, C);
         opac = 1.0;
         }

      /* Write the color to the output file */
      if (RuntimeState::File_Generation_Flag)
         Put_Pixel(eye, x, y, C, 1.0);

      /* If the display is active then draw the pixel on screen */
      if (Global::Display_Flag)
         display_plot(x, y, C);

      /* Save the current depth */
      ZBuffer_Write(eye, x, y, z);
      }
}

/**
 * @brief Draw a color/depth-interpolated line using Bresenham's algorithm.
 *
 * The z component is tracked as the line is drawn between two x-y points, and
 * color and opacity are interpolated per pixel. The line must be clipped to
 * the visible area of the image before this routine is called (actual image
 * coordinates are used to move from pixel to pixel).
 *
 * @param eye Viewpoint whose buffers are drawn into.
 * @param P0 Start point (screen x, y, z).
 * @param C0 Start color.
 * @param opac0 Start opacity.
 * @param P1 End point (screen x, y, z).
 * @param C1 End color.
 * @param opac1 End opacity.
 * @return No return value.
 */
static void
draw_line(Viewpoint *eye,
          fVec P0, fVec C0, float opac0,
          fVec P1, fVec C1, float opac1)
{
   int x, y, x1, y1, x2, y2;
   int ax, ay, sx, sy, dx, dy, d1;
   float dtdx, dtdy;
   fVec dxc, dyc, P;
   float dxo, dyo;
   float dxz, dyz, z;
   Vec C;
   Flt opac;

   /* Start by clipping the line to the viewable area */

   /* Determine screen coordinates ... */
   x1 = P0[0];
   y1 = P0[1];
   x2 = P1[0];
   y2 = P1[1];

/*
message("draw line: <%d,%d> - <%d,%d>\n",
        x1, y1, x2, y2);
message("color: <%g,%g,%g> - <%g,%g,%g>\n",
        C0[0], C0[1], C0[2], C1[0], C1[1], C1[2]);
*/
   dx = x2 - x1;
   dy = y2 - y1;
   sx = SGN(dx);
   sy = SGN(dy);

   dtdx = (dx == 0 ? 0.0 : (float)sx / (float)dx);
   dtdy = (dy == 0 ? 0.0 : (float)sy / (float)dy);

   ax = ABS(dx) << 1;
   ay = ABS(dy) << 1;

   /* Calculate the deltas between locations and colors */
   VecSub(C1, C0, dxc);
   VecScale(dtdx, dxc);
   dxo = (opac1 - opac0) * dtdx;
   dxz = (P1[2] - P0[2]) * dtdx;
   VecSub(C1, C0, dyc);
   VecScale(dtdx, dyc);
   dyo = (opac1 - opac0) * dtdy;
   dyz = (P1[2] - P0[2]) * dtdy;

   x = x1;
   y = y1;
   z = P0[2];

   /* Draw the end pixel of the line */
   MakeVector(x, y, z, P);
   VecCopy(C0, C);
   opac = opac0;
   draw_point(eye, x, y, z, C, opac);

   if (ax > ay) {
      /* x dominant */
      d1 = ay - (ax >> 1);
      for (;;) {
         if (x == x2) break;
         if (d1 >= 0) {
            y += sy;
            d1 -= ax;
            }

         VecAdd(dxc, C, C);
         z += dxz;
         opac += dxo;

         x += sx;
         d1 += ay;

         draw_point(eye, x, y, z, C, opac);
         }
      }
   else {
      /* y dominant */
      d1 = ax - (ay >> 1);
      for (;;) {
         if (y == y2) break;

         if (d1 >= 0) {
            x += sx;
            d1 -= ay;
            }

         VecAdd(dyc, C, C);
         z += dyz;
         opac += dyo;

         y += sy;
         d1 += ax;

         draw_point(eye, x, y, z, C, opac);
         }
      }
}

/**
 * @brief Transform a world point into homogeneous screen coordinates.
 *
 * @param tx Transform applied to the point.
 * @param P World-space point.
 * @param S Receives the transformed screen coordinates.
 * @return The homogeneous component; divide @p S by this value to return to
 *         world space.
 */
float tx_point(Transform& tx, Vec P, fVec S)
{
   Flt w;
   fVec P1;

   VecCopy(P, P1);
   w = P[0] * tx.matrix[0][3] +
       P[1] * tx.matrix[1][3] +
       P[2] * tx.matrix[2][3] +
              tx.matrix[3][3];
   fTxVec(S, P1, &tx);
   return w;
}

/**
 * @brief Draw a line between two world-space points, clipping to the view box.
 *
 * Packs the line into a two-vertex polygon so the shared polygon clipper can
 * be used, recovers the true depth of the (possibly clipped) endpoints, and
 * draws the resulting screen-space segment(s).
 *
 * @param eye Viewpoint whose buffers are drawn into.
 * @param P0 First world-space endpoint.
 * @param C0 Color at the first endpoint.
 * @param opac0 Opacity at the first endpoint.
 * @param P1 Second world-space endpoint.
 * @param C1 Color at the second endpoint.
 * @param opac1 Opacity at the second endpoint.
 * @return `0` if clipped out of existence, `1` otherwise.
 */
static int draw_3dline(Viewpoint *eye, Vec P0, Vec C0, Flt opac0,
            Vec P1, Vec C1, Flt opac1)
{
   rmode old_render_method;
   Poly poly;
   Vertex *v0, *v1;
   fVec S0, S1;
   Vec D;
   
   old_render_method = runtimeState::settings.Render_Method;
   runtimeState::settings.Render_Method = rmode::SCAN_CONVERSION;

   /* Copy line information into a polygon so the normal
      polygon clipping routine can be used. */
   v0 = &poly.vertices[0];
   v1 = &poly.vertices[1];
   poly.n = 2;

   VecCopy(P0, v0->W);
   v0->w = tx_point(* eye->WS, P0, v0->S);
   VecCopy(C0, v0->U);
   MakeVector(opac0, 0, 0, v0->N);

   VecCopy(P1, v1->W);
   v1->w = tx_point(* eye->WS, P1, v1->S);
   VecCopy(C1, v1->U);
   MakeVector(opac1, 0, 0, v1->N);

   PolyClip i = poly_clip_to_box(&poly, &pbox);
   if (i == PolyClip::PolyClipOut) {
      runtimeState::settings.Render_Method = old_render_method;
      return 0;
      }
   else if (poly.n == 2) {
      /* Calculate actual depth to the end points */
      VecSub(P0, eye->view_from, D)
      v0->S[0] /= v0->w;
      v0->S[1] /= v0->w;
      v0->S[2] = VecLen(D) * SGN(VecDot(D, ViewVec));

      VecSub(P1, eye->view_from, D)
      v1->S[0] /= v1->w;
      v1->S[1] /= v1->w;
      v1->S[2] = VecLen(D) * SGN(VecDot(D, ViewVec));
      /* Draw the (possibly clipped) line */
      draw_line(eye, v0->S, v0->U, v0->N[0],
                v1->S, v1->U, v1->N[0]);
      }
   else {
      /* We probably have a line that follows along
         one or more edges of the screen */
      for (int i=0;i<poly.n-1;i++,v0++,v1++) {
         VecCopy(v0->S, S0);
         VecCopy(v1->S, S1);

         /* Calculate actual depth to the end points */
         VecSub(v0->W, eye->view_from, D)
         S0[0] /= v0->w;
         S0[1] /= v0->w;
         S0[2] = VecLen(D) * SGN(VecDot(D, ViewVec));
         VecSub(v1->W, eye->view_from, D)
         S1[0] /= v0->w;
         S1[1] /= v0->w;
         S1[2] = VecLen(D) * SGN(VecDot(D, ViewVec));

         draw_line(eye, S0, v0->U, v0->N[0],
                   S1, v1->U, v1->N[0]);
         }
      }

   runtimeState::settings.Render_Method = old_render_method;
   return 1;
}

#define LEFT   0x01
#define RIGHT  0x02
#define BOTTOM 0x04
#define TOP    0x08

/**
 * @brief Compute the Cohen-Sutherland outcode for a point.
 *
 * @param x Point x coordinate.
 * @param y Point y coordinate.
 * @return Bitmask of `LEFT`, `RIGHT`, `BOTTOM` and `TOP` flags indicating which
 *         sides of the clip window the point lies outside.
 */
static unsigned char
ComputeOutCode(float x, float y)
{
   unsigned char code = 0;

#if 1
   if (x < win.x0) code |= LEFT;
   if (x > win.x1) code |= RIGHT;
   if (y < win.y0) code |= BOTTOM;
   if (y > win.y1) code |= TOP;
#else
   if (x < win.x0 - 1) code |= LEFT;
   if (x > win.x1 + 1) code |= RIGHT;
   if (y < win.y0 - 1) code |= BOTTOM;
   if (y > win.y1 + 1) code |= TOP;
#endif

   return code;
}

/**
 * @brief Clip a 2-D line to the image window and draw it.
 *
 * Copies the endpoints locally, applies Cohen-Sutherland clipping against the
 * screen window (interpolating position, color and opacity at each
 * intersection), and draws the accepted segment. Throws on an abort request.
 *
 * @param eye Viewpoint whose buffers are drawn into.
 * @param P0_in First endpoint (screen x, y, z).
 * @param C0_in Color at the first endpoint.
 * @param opac0 Opacity at the first endpoint.
 * @param P1_in Second endpoint (screen x, y, z).
 * @param C1_in Color at the second endpoint.
 * @param opac1 Opacity at the second endpoint.
 * @return No return value.
 */
void
draw_2dline(Viewpoint *eye,
            fVec P0_in, fVec C0_in, float opac0,
            fVec P1_in, fVec C1_in, float opac1)
{
   float opac, t;
   fVec D, P, C;
   fVec P0, C0, P1, C1;
   int accept, done;
   unsigned char outcode0, outcode1, outcodeOut;

   /* Since operations on components of arrays is destructive, we
      copy them into a local variable prior to any clipping. */
   VecCopy(P0_in, P0)
   VecCopy(P1_in, P1)
   VecCopy(C0_in, C0)
   VecCopy(C1_in, C1)

   accept = 0;
   done   = 0;
   outcode0 = ComputeOutCode(P0[0], P0[1]);
   outcode1 = ComputeOutCode(P1[0], P1[1]);

/*
message("2DLine0: <%g,%g,%g> - <%g,%g,%g>\n",
        P0[0], P0[1], P0[2],
        P1[0], P1[1], P1[2]);
*/
   do {
      if (outcode0 == 0 && outcode1 == 0) {
         accept = 1;
         done   = 1;
         }
      else if (outcode0 & outcode1)
         done = 1;
      else {
         if (outcode0)
            outcodeOut = outcode0;
         else
            outcodeOut = outcode1;

         /* Now find intersection point. */
         if (outcodeOut & TOP) {
            t = (win.y1 - P0[1]) / (P1[1] - P0[1]);
            VecSub(P1, P0, D);
            VecAddS(t, D, P0, P)
            VecSub(C1, C0, D)
            VecAddS(t, D, C0, C)
            opac = opac0 + t * (opac1 - opac0);
            }
         else if (outcodeOut & BOTTOM) {
            t = (win.y0 - P0[1]) / (P1[1] - P0[1]);
            VecSub(P1, P0, D);
            VecAddS(t, D, P0, P)
            VecSub(C1, C0, D)
            VecAddS(t, D, C0, C)
            opac0 = opac0 + t * (opac1 - opac0);
            }
         else if (outcodeOut & RIGHT) {
            t = (win.x1 - P0[0]) / (P1[0] - P0[0]);
            VecSub(P1, P0, D);
            VecAddS(t, D, P0, P)
            VecSub(C1, C0, D)
            VecAddS(t, D, C0, C)
            opac = opac0 + t * (opac1 - opac0);
            }
         else if (outcodeOut & LEFT) {
            t = (win.x0 - P0[0]) / (P1[0] - P0[0]);
            VecSub(P1, P0, D);
            VecAddS(t, D, P0, P)
            VecSub(C1, C0, D)
            VecAddS(t, D, C0, C)
            opac = opac0 + t * (opac1 - opac0);
            }

         /* Set up for next pass */
         if (outcodeOut == outcode0) {
            VecCopy(P, P0)
            VecCopy(C, C0)
            opac0 = opac;
            outcode0 = ComputeOutCode(P0[0], P0[1]);
/*
message("New 2DLine(0): <%g,%g,%g> - <%g,%g,%g>\n",
        P0[0], P0[1], P0[2],
        P1[0], P1[1], P1[2]);
*/
            }
         else {
            VecCopy(P, P1)
            VecCopy(C, C1)
            opac1 = opac;
            outcode1 = ComputeOutCode(P1[0], P1[1]);
/*
message("New 2DLine(1): <%g,%g,%g> - <%g,%g,%g>\n",
        P0[0], P0[1], P0[2],
        P1[0], P1[1], P1[2]);
*/
            }
         }
/* message("codes: %x, %x\n", outcode0, outcode1); */
if ((runtimeState::Check_Abort_Flag == 1) && _kbhit())
   throw std::runtime_error("ABORT");
      } while (!done);

   if (accept) {
/*
message("2DLine1 <%g,%g,%g> - <%g,%g,%g>\n",
        P0[0], P0[1], P0[2],
        P1[0], P1[1], P1[2]);
printf("p0: (%d,%d), p1: (%d,%d)\n",
       (int)P0_in[0], (int)P0_in[1], (int)P1_in[0], (int)P1_in[1]);
printf("*p0: (%d,%d), p1: (%d,%d)\n",
       (int)P0[0], (int)P0[1], (int)P1[0], (int)P1[1]);
*/
      /* Draw the line */
      draw_line(eye, P0, C0, opac0, P1, C1, opac1);
      }
}

#if 0
static void
draw_circles(Viewpoint *eye)
{
   fVec P0, P1, C0, C1, Black;
   float rad, cx, cy;
   float x0, y0, x1, y1;
   float t, dt;
   int i, j, steps = 20;

   MakeVector(0,0,0,Black);
   for (i=0;i<20;i++) {
/* message("Circle: %d\n", i); */
      rad = eye->view_xres * (1 + polyray_random()) / 3;
      cx  = eye->view_xres * (polyray_random() - 0.5);
      cy  = eye->view_yres * (polyray_random() - 0.5);
      x0 = cx + rad;
      y0 = cy;
      for (j=0,t=dt=TWO_PI/steps;j<steps;j++,t+=dt) {
         x1 = cx + rad * cos(t);
         y1 = cy + rad * sin(t);
         MakeVector(x0, y0, 0, P0);
         MakeVector(x1, y1, 0, P1);
         draw_2dline(eye, P0, Black, 0.0, P1, Black, 0.0);
         x0 = x1;
         y0 = y1;
         }
      }
}
#endif

/**
 * @brief Execute all overlay/scene drawing commands and lens flares.
 *
 * Draws the lens flares, then walks the list of draw nodes evaluating each
 * curve's position and color functions and connecting the sampled points with
 * 3-D lines (or a single point for a zero-step node).
 *
 * @param eye Viewpoint whose buffers are drawn into.
 * @param nodes Linked list of draw commands to execute.
 * @return No return value.
 */
void
DoDrawing(Viewpoint *eye, DrawNode *nodes)
{
   Flt ftemp, u, opac, opac0;
   float w, x, y, z;
   Vec P, C, P0, C0, D;
   fVec S;
   NODE_PTR tnode;
   struct subst_struct subst, *sp;
   float deltau;
   int i, j, k, steps;

/* draw_circles(eye); */

   Draw_Flares(eye);

   sp = &subst;
   for (;nodes!=NULL;nodes=nodes->next) {
      reset_subst(sp);
      steps = nodes->steps;
      if (steps < 1) {
         j = 0;
         deltau = 1.0;
         }
      else {
         j = steps;
         deltau = (nodes->high - nodes->low) / steps;
         }

      /* Dot to dot along the curve */
      for (i=0,u=nodes->low;i<=j;i++,u+=deltau) {
         MakeVector(u, 0, 0, subst.U);
         k = eval_node(sp, nodes->draw_fn, &ftemp, P, &tnode);
         if (k != 2)
            serror("Drawing location must be a vector\n");

         k = eval_node(sp, nodes->color_fn, &opac, C, &tnode);
         if (k != 2)
            serror("Drawing color must be a vector\n");

         if (i == 0) {
            VecSub(P, eye->view_from, D);
            z = VecLen(D) * SGN(VecDot(D, ViewVec));
            w = tx_point(* eye->WS, P, S);
            x = S[0] / w;
            y = S[1] / w;
            draw_point(eye, x, y, z, C, opac);
            }
         else
            draw_3dline(eye, P0, C0, opac0, P, C, opac);

         VecCopy(P, P0)
         VecCopy(C, C0)
         opac0 = opac;
         }
      }
}

/**
 * @brief Read a Targa (TGA) image from an open file into an @ref Img.
 *
 * Parses the 18-byte TGA header, reads the optional color map, allocates the
 * row buffers, and decodes the pixel data (handling both run-length encoded
 * and uncompressed images). Populates the @ref Img metadata fields on success.
 *
 * @param ifile Open file positioned anywhere (rewound internally).
 * @param img Image structure to populate.
 * @return `1` on success; unsupported or truncated files raise an error via
 *         `serror`.
 */
static int
read_TGA_image(FILE *ifile, Img *img)
{
   int h, i, j, k, v;
   unsigned char tgaheader[18];
   unsigned ftype, idlen, cmlen, cmsiz, psize, orien;
   unsigned width, length;
   unsigned char *cmap;
   unsigned char **imgbuf;
   unsigned char bytes[4];

   fseek(ifile, 0, SEEK_SET);

   if (fread(tgaheader, 18, 1, ifile) != 1)
      serror("reading header of %s\n", img->filename.c_str());

   idlen  = tgaheader[ 0];
   ftype  = tgaheader[ 2];
   cmlen  = tgaheader[ 5] + (tgaheader[ 6] << 8);
   cmsiz  = tgaheader[ 7] / 8;
   width  = tgaheader[12] + (tgaheader[13] << 8);
   length = tgaheader[14] + (tgaheader[15] << 8);
   psize  = tgaheader[16] / 8;
   orien  = tgaheader[17] & 0x20; /* Right side up ? */

/*
message("Read image: %s, type %d, size (%dx%d), psize %d, cmlen %d, cmsiz %d\n",
       img->filename, ftype, width, length, psize, cmlen, cmsiz);
message("                idlen %d, oren %d\n", idlen, orien);
*/

   if (ftype == 8 || ftype == 9 || ftype == 10 || ftype == 11)
      img->cflag = 1;
   else if (ftype == 1 || ftype == 2 || ftype == 3)
      img->cflag = 0;
   else
      serror("Unsupported Targa type: %d\n", ftype);

   /* Skip over the picture information */
   if (idlen > 0 && fread(idbuf, idlen, 1, ifile) != 1)
      serror("reading identification field of %s\n", img->filename.c_str());

   /* Read in the the color map */
   if (cmlen > 0) {
      cmap = (unsigned char*)polyray_malloc(sizeof(unsigned char) * cmsiz * cmlen);
      if (cmap  == NULL)
         serror("Failed to allocate memory for color map\n");
      /* CM added (unsigned) cast 19/1/2002 */
      for (i=0;(unsigned)i<cmlen * cmsiz;i++) {
         if ((h = fgetc(ifile)) == EOF)
            serror("Premature EOF in image file color map\n");
         cmap[i] = (unsigned char)h;
         }
      img->cmap = cmap;
      }
   else
      img->cmap = NULL;

   /* Allocate the row buffers for the image */
   if ((imgbuf = (unsigned char**)polyray_malloc(length * sizeof(unsigned char *))) == NULL)
      serror("Failed to allocate image memory\n");
   /* CM added (unsigned) cast 19/1/2002 */
   for (i=0;(unsigned)i<length;i++) {
      imgbuf[i] = (unsigned char*)polyray_malloc(width * psize * sizeof(unsigned char));
      if (imgbuf[i] == NULL)
         serror("Failed to allocate image memory\n");
      }

   /* Read the image */
   if (img->cflag) {
      i = 0; /* row counter */
      j = 0; /* column counter */
      /* CM added (unsigned) cast 19/1/2002 */
      while ((unsigned)i < length) {
         /* Grab a header */
         if ((h = fgetc(ifile)) == EOF)
            serror("Premature EOF in image file(1)\n");
         if (h & 0x80) {
            /* Repeat buffer */
            h &= 0x7F;
            /* CM added (unsigned) cast 19/1/2002 */
            for (k=0;(unsigned)k<psize;k++) {
               if ((v = fgetc(ifile)) == EOF)
                  serror("Premature EOF in image file(2)\n");
               bytes[k] = (unsigned char)v;
               }
            for (;h>=0;h--) {
		 	   /* CM added (unsigned) cast 19/1/2002 */
               for (k=0;(unsigned)k<psize;k++)
                  imgbuf[i][j*psize+k] = (unsigned char)bytes[k];
               /* CM added (unsigned) cast 19/1/2002 */
               if ((unsigned)++j == width) {
                  i++;
                  j = 0;
                  }
               }
            }
         else {
            /* Copy buffer */
            for (;h>=0;h--) {
			   /* CM added (unsigned) cast 19/1/2002 */
               for (k=0;(unsigned)k<psize;k++) {
                  if ((v = fgetc(ifile)) == EOF)
                     serror("Premature EOF in image file(3)\n");
                  imgbuf[i][j*psize+k] = (unsigned char)v;
                  }
               /* CM added (unsigned) cast 19/1/2002 */
               if ((unsigned)++j == width) {
                  i++;
                  j = 0;
                  }
               }
            }
         }
      }
   else
      /* Simple image file, read in all of the pixels */
      /* CM added (unsigned) casts 19/1/2002 */
      for (i=0;(unsigned)i<length;i++)
         for (j=0;(unsigned)j<width;j++)
            for (k=0;(unsigned)k<psize;k++)
               if ((v = fgetc(ifile)) == EOF)
                  serror("Premature EOF in image file\n");
               else
                  imgbuf[i][j*psize+k] = (unsigned char)v;
   img->copy   = 0;
   img->pixel_subtype  = ftype;
   img->cmlen  = cmlen;
   img->cmsiz  = cmsiz;
   img->width  = width;
   img->length = length;
   img->bytes_per_pixel  = psize * 8;
   img->scanline_order  = orien;
   img->image  = imgbuf;
   return 1;
}

/**
 * @brief Read an image file, auto-detecting its format.
 *
 * Opens @p filename along the Polyray search path and tries the unified
 * decoder (JPEG, PNG, GIF); if that fails it falls back to the Targa reader.
 *
 * @param filename Name of the image file to load.
 * @return The loaded image on success, or an error string describing why the
 *         file could not be opened or understood.
 */
std::expected<Img*,std::string> ReadImage(char *filename)
{
   FILE *filep; 
   Img *tmp;

   tmp = FactoryImg();
   std::string sfilename {filename};
   tmp->filename =sfilename;
   if ((filep = PathFileOpencpp(RuntimeState::POLYRAY_PATH_STRING,
       sfilename, "rb")) == nullptr) {
      delete tmp;
      return std::unexpected("Unable to open file: '"+sfilename+"'");
   }
   /* Try JPEG, PNG, and GIF via the unified decoder. */
   {
      DecodeOptions decopt{};
      ImgPopulateOptions popopt{};
      auto src = ImageSource::FromFile(filep, OwnershipMode::borrowed);
      if (decode_image_into_img(src, tmp, decopt, popopt)) {
         fclose(filep);
         return tmp;
         }
   }

   /* Fall back to Targa - the only format not handled above. */
   if (read_TGA_image(filep, tmp)) {
      fclose(filep);
      return tmp;
      }
   else {
      /* Wasn't an image type Polyray understands */
      swarning("Can't find image file: '%s'", filename);
      fclose(filep);
      FreeImg(tmp);
      return std::unexpected("Format not understood in file "+sfilename);
      }
}

/**
 * @brief Read an image file, aborting on failure.
 *
 * Thin wrapper over @ref ReadImage that raises an error via `serror` instead
 * of returning an error value.
 *
 * @param filename Name of the image file to load.
 * @return The loaded image (never returns on failure).
 */
Img *TGAReadImage(char *filename)
{
   auto ret = ReadImage(filename);
   if (ret.has_value())
      return ret.value();
   serror("%s",("TGAReadImage: "+ret.error()).c_str());
   return nullptr;//will never reach this
}

/**
 * @brief Free the memory used by an image.
 *
 * Releases the row buffers and color map of an image (such as one created by
 * @ref ReadImage or @ref TGAReadImage) unless it is a shared copy, then
 * deletes the @ref Img itself. Safe to call with `nullptr`.
 *
 * @param img Image to free; may be `nullptr`.
 * @return No return value.
 */
void
FreeImg(Img *img)
{
   int j;

   if (img == nullptr)
      return;

   /* Free up the original image now that is has been converted into
      the height field data structure. */
   if (img->copy == 0) {
      /* Deallocate the z-buffer */
      /* CM added (unsigned) cast 19/1/2002 */
      if (img->image != nullptr) {
         for (j=0;(unsigned)j<img->length;j++)
            if (img->image[j] != NULL)
               polyray_free(img->image[j]);
         polyray_free(img->image);
      }
      if (img->cmap != NULL)
         polyray_free(img->cmap);
   }
   delete img;
}

/**
 * @brief Convert normalised image coordinates to integer pixel indices.
 *
 * Maps @p x and @p y in [0,1) to pixel column/row indices, optionally wrapping
 * out-of-range coordinates when @p rflag is set, and accounts for the image's
 * scanline order. Indices are clamped to the valid pixel range.
 *
 * @param image Image being sampled.
 * @param x Normalised horizontal coordinate.
 * @param y Normalised vertical coordinate.
 * @param rflag Non-zero to repeat (wrap) out-of-range coordinates.
 * @param u Receives the pixel column index.
 * @param v Receives the pixel row index.
 * @return `1` if a pixel was addressed, `0` if the coordinate was out of range
 *         and wrapping was disabled.
 */
static int
calculate_offset(Img *image, Flt x, Flt y, int rflag, int *u, int *v)
{
   /* Calculate the floating point offset into the image */
   if (x < 0.0 || x >= 1.0) {
      if (!rflag)
         return 0;
      if (x < 0.0) x = 1.0 - fmod(fabs(x), 1.0);
      else x = fmod(x, 1.0);
      }
   if (y < 0.0 || y >= 1.0) {
      if (!rflag)
         return 0;
      if (y < 0.0) y = 1.0 - fmod(fabs(y), 1.0);
      else y = fmod(y, 1.0);
      }

   /* Figure out the pixel location in the bitmap */
   *u = (int)(x * image->width);
   /* CM added (unsigned) cast 19/1/2002 */
   if ((unsigned)*u >= image->width)
      *u = image->width-1;
   else if (*u < 0)
      *u = 0;
   if (image->scanline_order & 0x20)
      *v = image->length - (int)(y * image->length) - 1;
   else
      *v = (int)(y * image->length);
   /* CM added (unsigned) cast 19/1/2002 */
   if ((unsigned)*v >= image->length)
      *v = image->length-1;
   else if (*v < 0)
      *v = 0;
   return 1;
}

/**
 * @brief Sample an image's color and opacity at normalised coordinates.
 *
 * Resolves the pixel at (@p x, @p y) and decodes it according to the image
 * pixel subtype (color-mapped, raw, or monochrome), including 16-bit packed
 * and 32-bit alpha formats, into a floating-point color and opacity.
 *
 * @param image Image being sampled.
 * @param x Normalised horizontal coordinate.
 * @param y Normalised vertical coordinate.
 * @param rflag Non-zero to repeat (wrap) out-of-range coordinates.
 * @param opac Receives the sampled opacity (1 - alpha).
 * @param color Receives the sampled RGB color.
 * @return `1` if a pixel was sampled, `0` if the coordinate was out of range.
 */
int lookup_image_color(Img *image, Flt x, Flt y, int rflag,
                   Flt *opac, Vec color)
{
   unsigned char bytes[4];
   unsigned char r, g, b, o;
   int i, indexx, indexy;
   long map_index;

   /* Calculate the floating point offset into the image */
   if (!calculate_offset(image, x, y, rflag, &indexx, &indexy)) {       
      MakeVector(0, 0, 0, color);
      return 0;
      }

   /* Pull the color out of the image buffer */
   switch (image->pixel_subtype) {
   case 1:
   case 9:
      /* Color mapped images */
      /* Calculate the index */
      map_index = 0;
      for (i=0;i<image->bytes_per_pixel/8;i++)
         map_index = map_index * 256 +
                     image->image[indexy][(image->bytes_per_pixel/8)*indexx+i];
      /* CM added (unsigned) cast 19/1/2002 */
      if (map_index < 0 || (unsigned)map_index > image->cmlen)
         serror("Bad index: %d of %d at pixel [%d,%d], psize: %d in lookup_image_color\n",
               (int)map_index, (int)image->cmlen,
               (int)indexy, (int)indexx, image->bytes_per_pixel/8);
      /* Grab the color information from the color map */
      /* CM added (unsigned) cast 19/1/2002 */
      for (i=0;(unsigned)i<image->cmsiz;i++)
         bytes[i] = image->cmap[image->cmsiz*map_index+i];
      if (image->bytes_per_pixel == 16) {
         b = (bytes[0] & 0x1f) << 3;
         g = (((bytes[1] & 0x03) << 3) | ((bytes[0] & 0xe0) >> 5)) << 3;
         r = ((bytes[1] & 0x7c) << 1);
         o = (bytes[1] & 0x80 ? 0 : 255);
         }
      else {
         b = bytes[0];
         g = bytes[1];
         r = bytes[2];
         o = (image->bytes_per_pixel == 32 ? bytes[3] : 255);
         }
      break;
   case 2:
   case 10:
      /* Raw images */
      for (i=0;i<image->bytes_per_pixel/8;i++)
         bytes[i] = image->image[indexy][(image->bytes_per_pixel/8) * indexx + i];
      if (image->bytes_per_pixel == 16) {
         b = (bytes[0] & 0x1f) << 3;
         g = (((bytes[1] & 0x03) << 3) | ((bytes[0] & 0xe0) >> 5)) << 3;
         r = ((bytes[1] & 0x7c) << 1);
         o = (bytes[1] & 0x80 ? 0 : 255);
         }
      else {
         b = bytes[0];
         g = bytes[1];
         r = bytes[2];
         o = (image->bytes_per_pixel == 32 ? bytes[3] : 255);
         }
      break;
   case 3:
   case 11:
      /* Monochrome images */
      r = image->image[indexy][indexx];
      g = r;
      b = r;
      o = 255;
      break;
   default:
      serror("Bad image type in lookup_image_color\n");
   }

   /* Turn the r, g, b values into a floating point color */
   MakeVector((Flt)r / 255.0, (Flt)g / 255.0, (Flt)b / 255.0, color);
   *opac = 1.0 - (Flt)o / 255.0;
   return 1;
}

/**
 * @brief Sample an image as a height field at normalised coordinates.
 *
 * Resolves the pixel at (@p x, @p y) and interprets its bytes as a height
 * value, biased by 128, according to the bytes-per-pixel (8/16/24/32-bit, the
 * last optionally a machine-native float).
 *
 * @param image Image being sampled.
 * @param x Normalised horizontal coordinate.
 * @param y Normalised vertical coordinate.
 * @param rflag Non-zero to repeat (wrap) out-of-range coordinates.
 * @param height Receives the sampled height.
 * @return `1` if a pixel was sampled, `0` if the coordinate was out of range.
 */
int
lookup_height(Img *image, Flt x, Flt y, int rflag, Flt *height)
{
   unsigned char bytes[4];
   unsigned char r, g, b;
   int i, indexx, indexy;
   long map_index;
   float depth;

   if (!calculate_offset(image, x, y, rflag, &indexx, &indexy)) {
      *height = 0.0;
      return 0;
      }

   /* Pull the color out of the image buffer */
   switch (image->pixel_subtype) {
      case 1:
      case 9:
         /* Color mapped images */
         /* Simply use the index as the height */
         map_index = 0;
         for (i=0;i<(image->bytes_per_pixel/8);i++)
            bytes[i] = image->image[indexy][(image->bytes_per_pixel/8) * indexx + i];
         break;
      case 2:
      case 3:
      case 10:
      case 11:
         /* Raw images (color or monochrome) */
         for (i=0;i<(image->bytes_per_pixel/8);i++)
            bytes[i] = image->image[indexy][(image->bytes_per_pixel/8) * indexx + i];
         break;
      default:
         serror("Bad image type in lookup_height\n");
      }

   if (image->bytes_per_pixel == 8)
      *height = (float)bytes[0] - 128.0;
   else if (image->bytes_per_pixel == 16) {
      g = bytes[0];
      r = bytes[1];
      *height = ((float)r + (float)g / 256.0) - 128.0;
      }
   else if (image->bytes_per_pixel == 24) {
      b = bytes[0];
      g = bytes[1];
      r = bytes[2];
      *height = ((float)r + (float)g / 256.0 + (float)b / 65536.0) - 128.0;
      }
   else if (image->bytes_per_pixel == 32) {
      if (sizeof(float) == 4 && sizeof(unsigned char) == 1) {
         /* Retrieve a machine dependent floating point number. */
         memcpy(&depth, &bytes[0], 4);
         *height = (Flt)depth;
         }
      else {
         b = bytes[0];
         g = bytes[1];
         r = bytes[2];
         *height = ((float)r + (float)g / 256.0 + (float)b / 65536.0) - 128.0;
         }
      }
   else
      serror("Unsupported height map type: %d bytes/pixel\n", image->bytes_per_pixel);

   return 1;
}

/**
 * @brief Determine the height value of a particular pixel in an image.
 *
 * Like @ref lookup_height but addressed by integer pixel coordinates rather
 * than normalised ones; returns 0 for out-of-bounds pixels and accounts for
 * the image's scanline order.
 *
 * @param image Image being sampled.
 * @param x Pixel column.
 * @param y Pixel row.
 * @return The decoded height value, or 0 when the pixel is out of bounds.
 */
float
image_height(Img *image, int x, int y)
{
   unsigned char bytes[4];
   unsigned char r, g, b;
   int i;
   float depth;

   if (x < 0 || x >= image->width ||
       y < 0 || y >= image->length)
      return 0.0;

   if (image->scanline_order & 0x20)
      y = image->length - y - 1;

   /* Pull the color out of the image buffer */
   switch (image->pixel_subtype) {
      case 1:
      case 9:
         /* Color mapped images */
         /* Simply use the index as the height */
         for (i=0;i<(image->bytes_per_pixel/8);i++)
            bytes[i] = image->image[y][(image->bytes_per_pixel/8) * x + i];
         break;
      case 2:
      case 3:
      case 10:
      case 11:
         /* Raw images (color or monochrome) */
         for (i=0;i<(image->bytes_per_pixel/8);i++)
            bytes[i] = image->image[y][(image->bytes_per_pixel/8) * x + i];
         break;
      default:
         serror("Bad image type in lookup_height\n");
      }

   if (image->bytes_per_pixel == 8)
      depth = (float)bytes[0] - 128.0;
   else if (image->bytes_per_pixel == 16) {
      g = bytes[0];
      r = bytes[1];
      depth = ((float)r + (float)g / 256.0) - 128.0;
      }
   else if (image->bytes_per_pixel == 24) {
      b = bytes[0];
      g = bytes[1];
      r = bytes[2];
      depth = ((float)r + (float)g / 256.0 + (float)b / 65536.0) - 128.0;
      }
   else if (image->bytes_per_pixel == 32) {
      if (sizeof(float) == 4 && sizeof(unsigned char) == 1) {
         /* Retrieve a machine dependent floating point number. */
         memcpy(&depth, &bytes[0], 4);
         }
      else {
         b = bytes[0];
         g = bytes[1];
         r = bytes[2];
         depth = ((float)r + (float)g / 256.0 + (float)b / 65536.0) - 128.0;
         }
      }
   else
      serror("Unsupported height map type: %d bytes/pixel\n", image->bytes_per_pixel);

   return depth;
}

/**
 * @brief Sample an image's raw index/intensity at normalised coordinates.
 *
 * Resolves the pixel at (@p x, @p y) and returns its color-map index (for
 * color-mapped images) or a single intensity byte (for raw/monochrome images).
 *
 * @param image Image being sampled.
 * @param x Normalised horizontal coordinate.
 * @param y Normalised vertical coordinate.
 * @param rflag Non-zero to repeat (wrap) out-of-range coordinates.
 * @param index Receives the sampled index/intensity value.
 * @return `1` if a pixel was sampled, `0` if the coordinate was out of range.
 */
int
lookup_index(Img *image, Flt x, Flt y, int rflag, Flt *index)
{
   unsigned char bytes[4];
   unsigned char r;
   int i, indexx, indexy;

   if (!calculate_offset(image, x, y, rflag, &indexx, &indexy)) {
      *index = 0.0;
      return 0;
      }

   /* Pull the color out of the image buffer */
   switch (image->pixel_subtype) {
   case 1:
   case 9:
      /* Color mapped images */
      /* Calculate the index */
      *index = 0;
      for (i=0;i<(image->bytes_per_pixel/8);i++)
         *index = *index * 256 +
                     image->image[indexy][(image->bytes_per_pixel/8)*indexx+i];
      if (*index < 0 || *index > image->cmlen)
         serror("Bad index: %d in lookup_image_color\n", *index);
      break;
   case 2:
   case 10:
      /* Raw images */
      for (i=0;i<(image->bytes_per_pixel/8);i++)
         bytes[i] = image->image[indexy][(image->bytes_per_pixel/8) * indexx + i];
      if (image->bytes_per_pixel == 16)
         r = bytes[1];
      else
         r = bytes[2];
      *index = r;
      break;
   case 3:
   case 11:
      /* Raw monochrome images */
      *index = image->image[indexy][indexx];
      break;
   default:
      serror("Bad image type in lookup_index\n");
   }
   return 1;
}


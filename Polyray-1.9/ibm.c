/* ibm.c

  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.

  This software may be used for any private and non-commercial
  use.

  You may not distribute this software, in whole or in part,
  for any commercial purpose, without the express consent of
  the authors.

  There is no warranty or other guarantee of fitness of this software
  for any purpose.  It is provided solely "as is".

*/

#undef unix
//#define __BCC55__


int Pallette_Start = 0;      /* Pallette entries start at 0 */
int Pallette_Flag = 1;       /* Use 884 pallette */
int Display_Flag = 1;        /* Use standard VGA */
int Reset_Display_Flag = 1;  /* Turn on the requested video mode */
int Dither_Flag = 0;         /* Use ordered dither on video display */

/* Define the top right, the width, and the length of usuable display area */
int Display_x0 = -1, Display_y0 = -1;
int Display_xl = -1, Display_yl = -1;

#if (defined(unix) || defined(linux) ) && !defined(MINGW)
/* UNIX interface */


#include "defs.h"
#include "display.h"
#include "memory.h"
#include "io.h"
#include "unixcompat.h"

typedef unsigned char byte;

//extern void X11_PutPixel(int x,int y,byte r,byte g,byte b);
//extern void DeinitDisplay(void);
//extern void InitDisplay(int width,int height,char *s);

void display_plot(int x, int y, Vec color)
{
  //X11_PutPixel(x,y,(color[0]<0?0:(color[0]>1?255:(int) (color[0]*255))),
  //                 (color[1]<0?0:(color[1]>1?255:(int) (color[1]*255))),
  //                 (color[2]<0?0:(color[2]>1?255:(int) (color[2]*255))));
}

void
display_box(int x0, int y0, int x1, int y1, Vec color)
{
}

void
display_close(int wait_flag)
{
  if (wait_flag) {
    printf("\nFinished. Click into the window to continue\n");
    //SpecialStatus("Done");
    //getch();
    pause();
  }
  //DeinitDisplay();
  return;
}

void display_init(Viewpoint *eye, char *S)
{
  char s[128];
  sprintf(s,"Polyray: %s",S);
  //InitDisplay(eye->view_xres,eye->view_yres,s);
}

void display_line(int x0, int y0, int x1, int y1, Vec color)
{
}

int getch()
{
  //FlushDisplay();
  if (Display_Flag) {
   // while (PollEvents()) ;
  }
  else
    getchar();
}

/* EITHER DOS/DJGPP-GNU OR BCC55 OR MINGW assumed */

/* #elsif defined( __BCC55__ ) || defined( __GNUC__ ) || defined( MINGW ) */

#else

#include "defs.h"
#include "display.h"
#include "memory.h"
#include "io.h"


void display_plot(int x, int y, Vec color)
{
 ;
}

void display_box(int x0, int y0, int x1, int y1, Vec color)
{
  ;
}

void display_close(int wait_flag)
{
  if (wait_flag) {
//    printf("\nFinished. Click into the window to continue\n");
//    SpecialStatus("Done");
    printf("\nFinished. Press any key to continue\n");
#if !defined(__APPLE__)
    getch();
#endif
  }
//  DeinitDisplay();
  return;
}

void display_init(Viewpoint *eye, char *s1)
{
  char s[128];
  sprintf(s,"Polyray: %s",s1);
  Display_Flag = 0;
}

void display_line(int x0, int y0, int x1, int y1, Vec color)
{
}

int getch()
{
   return getchar();
}



//#else   /* normal boring MS-DOG stuff */
#endif

#ifdef VERY_OLD_MSDOG_STUFF
#include <io.h>
#if !defined( MAC )
#include <dos.h>
#if defined( __GNUC__ )
#include <pc.h>
#define MK_FP(seg, ofs) ((void *)(0xE0000000 + ((seg)<<4) + ofs))
#define FP_OFF(ptr) (unsigned short)(ptr)
#define FP_SEG(ptr) (unsigned short)(((unsigned long)ptr >> 16) & 0x0FFF)
#define segread(x) (void)(x)
#define getch() getkey()
#define __far
#define outp( portid,v )  outportb( portid,v )
#define inp( portid ) inportb( portid )
#elif defined( DOS386 )
#include <conio.h>
#elif defined( __WATCOMC__ )
#include <float.h>
#define CLD clear_direction()
unsigned short __8087cw = IC_AFFINE | RC_NEAR | PC_64  | 0x007F;
void clear_direction(void);
#pragma aux clear_direction = 0xFC parm [] modify nomemory;
  typedef unsigned long DWORD;

  #define D32RealSeg(P)   ((((DWORD) (P)) >> 4) & 0xFFFF)
  #define D32RealOff(P)   (((DWORD) (P)) & 0xF)

  static struct rminfo {          /* DPMI Real Mode Call structure */
    long EDI;
    long ESI;
    long EBP;
    long reserved_by_system;
    long EBX;
    long EDX;
    long ECX;
    long EAX;
    short flags;
    short ES,DS,FS,GS,IP,CS,SP,SS;
  } RMI;
  /* CCP - End */

  #define ax eax
  #define bx ebx
  #define cx ecx
  #define dx edx
  #define di edi
  #define int86 int386
  #define int86x int386x
  #ifdef MK_FP
  #undef MK_FP
  #endif
  #define MK_FP(seg,ofs)  ((void *)(((unsigned long)(seg) << 4) + (unsigned)(ofs)))
  #define __far
#else
#define __far
#endif
#endif

#include "defs.h"
#include "display.h"
#include "memory.h"
#include "io.h"

typedef struct {
   unsigned char bytes;
   unsigned char byte[3];
   } quantized_color;

static quantized_color black1 = {1, {0, 0, 0}};
static quantized_color black2 = {2, {0, 0, 0}};
static quantized_color black3 = {3, {0, 0, 0}};

/* Format of VESA global information */
struct VgaInfoBlock   {
  unsigned char      VESASignature[4];
  unsigned char      VESAVersion;
  unsigned char      VESARevision;
  unsigned short     OEMStringPtrOff;
  unsigned short     OEMStringPtrSeg;
  unsigned char      Capabilities[4];
  unsigned short     VideoModePtrOff;
  unsigned short     VideoModePtrSeg;
  };

/* Format of the information for individual VESA display modes */
struct ModeInfoBlock   {
  unsigned short ModeAttributes;
  unsigned char  WinAAttributes;
  unsigned char  WinBAttributes;
  unsigned short WinGranularity;
  unsigned short WinSize;
  unsigned short WinASegment;
  unsigned short WinBSegment;
  unsigned short WinFunctPtrOff;
  unsigned short WinFunctPtrSeg;
  unsigned short BytesPerScanLine;

  /* the remainder of this structure is optional */

  unsigned short XResolution;
  unsigned short YResolution;
  unsigned char XCharSize;
  unsigned char YCharSize;
  unsigned char NumberOfPlanes;
  unsigned char BitsPerPixel;
  unsigned char NumberOfBanks;
  unsigned char MemoryModel;
  unsigned char BankSize;
};

static Flt X_Display_Scale = 1.0;
static Flt Y_Display_Scale = 1.0;

static unsigned offx = 0;
static unsigned offy = 0;
static unsigned maxx = 320;
static unsigned maxy = 200;
static unsigned line_length = 320;
static unsigned screen_maxx = 320;
static unsigned screen_maxy = 200;
static unsigned long granularity = 65536;

typedef unsigned char pallette_array[256][3];
static pallette_array *pallette = NULL;

static int EGA_colors[16][3] =
   {{  0,  0,  0}, {  0,  0,255}, {  0,255,  0}, {  0,127,127},
    {255,  0,  0}, {127,  0,127}, {127,127,  0}, { 85, 85, 85},
    {170,170,170}, {127,127,255}, {127,255,127}, {  0,255,255},
    {255,127,127}, {255,  0,255}, {255,255,  0}, {255,255,255}};
static int EGA_remap[16] =
   {0,  6,  5,  4,  3,  2, 1,  7,
    8, 14, 13, 12, 11, 10, 9, 15};
static int Dither_Matrix[8][8] =
   {{ 0, 32,  8, 40,  2, 34, 10, 42},
    {48, 16, 56, 24, 50, 18, 58, 26},
    {12, 44,  4, 36, 14, 46,  6, 38},
    {60, 28, 52, 20, 62, 30, 54, 22},
    { 3, 35, 11, 43,  1, 33,  9, 41},
    {51, 19, 59, 27, 49, 17, 47, 25},
    {15, 47,  7, 39, 13, 45,  5, 37},
    {63, 31, 55, 23, 61, 29, 53, 21}};

/* Global variables to keep track of VESA calls */
static struct ModeInfoBlock VESAModeInfo;

#ifdef __DJGPP__
  short video;
#endif


#if defined( DOS386 )
static short real_buf[2];
/* Pointer to real memory for looking at VESA information */
static unsigned char __far *real_ptr;

/* Ask the dos extender where to communicate with real mode BIOS calls */
static void
find_real_buf()
{
   struct SREGS sregs;
   union REGS regs;

   regs.x.ax = 0x250d;
   segread(&sregs);
   int86x(0x21, &regs, &regs, &sregs);

   *((unsigned *)&real_buf) = regs.e.ebx;
   real_ptr = MK_FP(sregs.es, regs.e.edx);
}

#elif defined( __GNUC__ )
static int real_buf[2];
static unsigned char __far *real_ptr;

void
find_real_buf()
{
   static unsigned char *memptr = (unsigned char *)0xE0000000;
   static unsigned char buf[0x1000];

   union REGS reg;
   unsigned char *p, *filename;
   int i, occur = 0;

   strcpy(buf, "tmpXXXXX");
   filename = mktemp(buf);

   for (p = &memptr[0xA0000-1-strlen((char *)buf)]; p != memptr; p--)
      if (buf[0] == *p && strcmp((char *)buf, (char *)p) == 0) {
         real_ptr = p;
         occur++;
         }

   if (!occur)
      error("Video init failure");

   reg.h.ah = 0x1A;
   reg.x.dx = (int)buf;
   int86(0x21, &reg, &reg);

   reg.h.ah = 0x4E;
   reg.x.dx = (int)"*.*";
   int86(0x21, &reg, &reg);
   real_buf[0] = (reg.x.dx & 0xFFFF) - 43;

   getcwd(buf, sizeof(buf));
   if ((i = strlen(buf)) < 2) i = 2;
   real_ptr = real_ptr - 3 - i;
}
#elif defined( __WATCOMC__ )
unsigned int answerSel;          /* Save area for the selector pointer */
unsigned char *real_ptr;

void
find_real_buf()
{
   union REGS regs;

   regs.x.ax = 0x0100;          /* DPMI allocate DOS memory */
   regs.x.bx = 17;              /* Number of paragraphs requested */
   int86(0x31, &regs, &regs);

   answerSel = regs.x.dx;     /* Save selector for free memory function */
   real_ptr =  (void *) ((regs.x.ax & 0xFFFF) << 4);
    if (real_ptr == NULL) {
        error("Failed to allocate DOS memory\n");
        exit(1);
    }
}
#endif

static void
bios_putpixel(int x, int y, int color)
{

#if !defined(__BCC55__)

   union REGS reg;

   reg.h.ah = 0x0c;
   reg.h.al = color;
   reg.h.bh = 0;
   reg.x.cx = x;
   reg.x.dx = y;
   int86(0x10, &reg, &reg);
#endif
}

static void
setvideomode(int mode)
{
#if !defined( __BCC55__ )
   union REGS reg;
   reg.h.ah = 0;
   reg.h.al = mode;
   int86(0x10, &reg, &reg);
#endif
}

static int
setvesabank(int bank)
{
#if !defined(__BCC55__)
   static int current_bank = -1;
   union REGS reg;

   if (bank != current_bank) {
      current_bank = bank;
      reg.x.ax = 0x4F05;
      reg.x.bx = 0;
      reg.x.dx = bank;
      int86(0x10, &reg, &reg);
      return (reg.x.ax == 0x004f ? 1 : 0);
      }
#endif
   return 1;
}

static int
setvesamode(int mode)
{
#if !defined(__BCC55__)
   union REGS regs;
   struct SREGS sregs;
   struct ModeInfoBlock *VgaPtr = &VESAModeInfo;
   unsigned int i;

   /* Call VESA function 1 to get mode info */
   regs.h.ah = 0x4f;
   regs.h.al = 0x01;
   regs.x.cx = mode;
#if defined( DOS386 )
   regs.x.di = real_buf[0];
   segread(&sregs);
   sregs.es  = real_buf[1];
   int86x_real(0x10, &regs, &regs, &sregs);
   i = regs.x.ax;
   if (i != 0x004f) {
      /* printf("Mode %x not available: %d\n", mode, i); */
      return 0;
   }
   for (i=0; i<sizeof(struct ModeInfoBlock); i++)
      ((unsigned char *)VgaPtr)[i] = real_ptr[i];
#elif defined( __GNUC__ )
   regs.x.di = real_buf[0];
   int86x(0x10, &regs, &regs, &sregs);
   if (regs.h.al != 0x4f || regs.h.ah != 0)
      return 0;
   for (i=0; i<sizeof(struct ModeInfoBlock); i++)
      ((unsigned char *)VgaPtr)[i] = real_ptr[i];
#elif defined( __WATCOMC__ )
   for (i=0; i<256; i++)
      real_ptr[i] = 0;
   memset(&RMI, 0, sizeof(RMI));
   RMI.EAX = 0x4F01;
   RMI.ECX = mode;
   RMI.ES  = D32RealSeg(real_ptr);
   RMI.EDI = D32RealOff(real_ptr);
   segread(&sregs);              /* init our selectors */
   regs.x.ax = 0x0300;
   regs.x.bx = 0x10;
   regs.x.cx = 0;
   sregs.es  = FP_SEG(&RMI);      /* get segment of rmi */
   regs.x.di = FP_OFF(&RMI);     /* deposit results here */
   int86x(0x31, &regs, &regs, &sregs);
   i = RMI.EAX;
   if (i != 0x004f) {
      /* printf("Mode %d not available: %d\n", mode, i); */
      return 0;
   }
   for (i=0; i<sizeof(struct ModeInfoBlock); i++)
      ((unsigned char *)VgaPtr)[i] = real_ptr[i];
#else
   regs.x.di = FP_OFF(VgaPtr);
   sregs.es  = FP_SEG(VgaPtr);
   int86x(0x10, &regs, &regs, &sregs);
   i = regs.x.ax;
   if (i != 0x004f) {
      /* printf("Mode %x not available: %d\n", mode, i); */
      return 0;
   }
#endif

   /* The mode is supported - save useful information and initialize
      the graphics display */
   line_length = VgaPtr->BytesPerScanLine;
   granularity = ((unsigned long)VgaPtr->WinGranularity) << 10;
   screen_maxx = VgaPtr->XResolution;
   screen_maxy = VgaPtr->YResolution;

/*
printf("Gran: %d/%d, Line: %d, maxx: %d, maxy: %d\n",
       (int)VgaPtr->WinGranularity, granularity,
       (int)line_length, screen_maxx, screen_maxy);
*/

   if (Reset_Display_Flag) {
      /* Now go set the video adapter into the requested mode */
      regs.x.ax  = 0x4F02;
      regs.x.bx = mode;
#if defined( __WATCOMC__ )
      regs.x.ebx = mode;
#endif
      int86(0x10, &regs, &regs);
      i = regs.x.ax;
      return (i == 0x004f ? 1 : 0);
      }
   else
      /* We are supposed to already be in the requested mode, and the
         display adapter supports it, so return success.  */
#endif
      return 1;
}

void
setmany(unsigned char palbuf[256][3], int start, int count)
{
   unsigned i, j;
#if !defined(__BCC55__)
   j = start + count;
   for (i=0,j=start;i<count;i++,j++) {
      outp(0x3c8, j);
      outp(0x3c9, palbuf[i][0]);
      outp(0x3c9, palbuf[i][1]);
      outp(0x3c9, palbuf[i][2]);
      }
#endif
}

static void
pallette_init()
{
   unsigned i, r, g, b;
#if !defined (__BCC55__)

   if (pallette == NULL) {
      pallette = polyray_malloc(sizeof(pallette_array));
      if (pallette == NULL)
         error("Failed to allocate pallette array\n");
      }

   if (Pallette_Flag == 0) {
      for (i=0;i<64;i++) {
         (*pallette)[i][0] = i;
         (*pallette)[i][1] = i;
         (*pallette)[i][2] = i;
         }
      setmany(*pallette, MIN(Pallette_Start, 192), 64);
      }
   else if (Pallette_Flag == 1) {
      i = 0;
      for (r=0;r<8;r++)
         for (g=0;g<8;g++)
            for (b=0;b<4;b++) {
               (*pallette)[i][0] = r << 3;
               (*pallette)[i][1] = g << 3;
               (*pallette)[i][2] = b << 4;
               i++;
               }
      setmany(*pallette, 0, 256);
      }
   else if (Pallette_Flag == 2) {
      i = 0;
      for (r=0;r<6;r++)
         for (g=0;g<6;g++)
            for (b=0;b<6;b++) {
               (*pallette)[i][0] = 63.0 * (float)r / 5.0;
               (*pallette)[i][1] = 63.0 * (float)g / 5.0;
               (*pallette)[i][2] = 63.0 * (float)b / 5.0;
               i++;
               }
      setmany(*pallette, MIN(Pallette_Start, 40), 216);
      }
   else if (Pallette_Flag == 3) {
      i = 0;
      for (i=0;i<16;i++) {
         (*pallette)[i][0] = EGA_colors[i][0];
         (*pallette)[i][1] = EGA_colors[i][1];
         (*pallette)[i][2] = EGA_colors[i][2];
         }
      setmany(*pallette, MIN(Pallette_Start, 240), 16);
      }
   else
      error("Bad pallette value: %d\n", Pallette_Flag);
#endif
}

static void
quantize_4bit(Vec color, quantized_color *qcolor)
{
   unsigned char r, g;
   float d;

   qcolor->bytes = 1;

   /* EGA colors */
   d = 0.5;
   r = 0;
   if (color[0] + color[1] - color[2] > d)
      r |= 0x01;
   if (color[0] - color[1] + color[2] > d)
      r |= 0x02;
   if (-color[0] + color[1] + color[2] > d)
      r |= 0x04;
   d = 1.5;
   g = 0x08;
   if (r == 0) {
      d = 0.5;
      g = 0x07;
      }
   else if (r == 7) {
      d = 2.5;
      r = 0x08;
      g = 0x07;
      }
   if (color[0] + color[1] + color[2] > d)
      r |= g;
   qcolor->byte[0] = EGA_remap[r];
}

static void
quantize(int x, int y, Vec color, quantized_color *qcolor)
{
#if !defined(__BCC55__)
   int i;
   Vec color2;
   Flt bump;
   unsigned r, g, b;

   if (Dither_Flag == 1) {
      bump = Dither_Matrix[x & 0x07][y & 0x07] / 255.0;
      if ((Display_Flag >= FIRST_HICOLOR_MODE &&
           Display_Flag <  FIRST_HICOLOR_MODE + VIDEO_RESOLUTIONS) ||
          (Display_Flag >= FIRST_16BIT_MODE &&
           Display_Flag <  FIRST_16BIT_MODE + VIDEO_RESOLUTIONS))
         bump /= 8.0;
      else if (Display_Flag >= FIRST_TRUECOLOR_MODE &&
               Display_Flag <  FIRST_TRUECOLOR_MODE + VIDEO_RESOLUTIONS)
         bump /= 64.0;
      color2[0] = color[0] + bump;
      color2[1] = color[1] + bump;
      color2[2] = color[2] + bump;
      }
   else
      VecCopy(color, color2)

   i = 255.0 * color2[2];
   if (i<0) i=0;
   else if (i>=256) i = 255;
   b = (unsigned char)i;

   i = 255.0 * color2[1];
   if (i<0) i=0;
   else if (i>=256) i = 255;
   g = (unsigned char)i;

   i = 255.0 * color2[0];
   if (i<0) i=0;
   else if (i>=256) i = 255;
   r = (unsigned char)i;

   if (Display_Flag >= FIRST_4BIT_MODE &&
       Display_Flag <  FIRST_4BIT_MODE + VIDEO_RESOLUTIONS) {
      qcolor->bytes = 1;
      quantize_4bit(color2, qcolor);
      qcolor->byte[0] += Pallette_Start;
      }
   else if (Display_Flag >= FIRST_8BIT_MODE &&
       Display_Flag <  FIRST_8BIT_MODE + VIDEO_RESOLUTIONS) {
      qcolor->bytes = 1;
      switch (Pallette_Flag) {
      case 3:
         quantize_4bit(color2, qcolor);
         break;
      case 2:
         /* 666 */
         qcolor->byte[0] = 6 * (6 * (r / 51) + (g / 51)) + b / 51;
         break;
      case 1:
         /* 884 */
         qcolor->byte[0] = (r & 0xE0) | ((g & 0xE0) >> 3) | (b >> 6);
         break;
      default:
         /* Greyscale */
         qcolor->byte[0] = (r>g?(r>b?r>>2:b>>2):(g>b?g>>2:b>>2));
         }
      qcolor->byte[0] += Pallette_Start;
      }
   else if (Display_Flag >= FIRST_HICOLOR_MODE &&
            Display_Flag <  FIRST_HICOLOR_MODE + VIDEO_RESOLUTIONS) {
      /* Hicolor, add bits together */
      qcolor->bytes = 2;
      qcolor->byte[1] = ((r >> 1) & 0x7c) | (g >> 6);
      qcolor->byte[0] = ((g << 2) & 0xe0) | (b >> 3);
      }
   else if (Display_Flag >= FIRST_16BIT_MODE &&
            Display_Flag <  FIRST_16BIT_MODE + VIDEO_RESOLUTIONS) {
      /* Hicolor, add bits together */
      qcolor->bytes = 2;
      qcolor->byte[1] = (r & 0xf8) | (g >> 5);
      qcolor->byte[0] = ((g << 2) & 0xe0) | (b >> 3);
      }
   else if (Display_Flag >= FIRST_TRUECOLOR_MODE &&
            Display_Flag <  FIRST_TRUECOLOR_MODE + VIDEO_RESOLUTIONS) {
      /* Truecolor modes, use quantized pixels */
      qcolor->bytes = 3;
      qcolor->byte[2] = r;
      qcolor->byte[1] = g;
      qcolor->byte[0] = b;
      }
   else {
      display_close(0);
      error("Bad display mode in quantize");
      }
#endif
}

static void
plotpoint(int x, int y, quantized_color *color)
{
#if !defined (__BCC55__)
   unsigned char __far *fp;
   unsigned long fpa, fpb;
   unsigned bank;
   int i;

   /* Don't plot points outside the screen boundaries */
   if (x < 0 || x >= screen_maxx) return;
   if (y < 0 || y >= screen_maxy) return;

   if (Display_Flag >= FIRST_4BIT_MODE &&
       Display_Flag <  FIRST_4BIT_MODE + VIDEO_RESOLUTIONS) {
      /* Have to do things a little bit differently in 16 color modes */
      bios_putpixel(x, y, color->byte[0]);
      }
   else {
      fpa = (unsigned long)line_length * y + (x * color->bytes);

/* if defined DJGPP ... was here, perhaps slower! */

      for (i=0;i<color->bytes;i++,fpa++) {
         if (Display_Flag > 1) {
            /* Only have more than 64K pixels in VESA modes */
            bank = fpa / granularity;
            setvesabank(bank);
            }
         fpb = fpa % granularity;

#if defined( __DJGPP__   )
         _farpokeb (video, fpb, color->byte[i]);
#elif defined( DOS386 )
         fp = MK_FP(_x386_zero_base_selector, 0xA0000 + fpb);
#elif defined( __GNUC__ )
         fp = (unsigned char *)MK_FP(0xA000, (unsigned int)fpb);
#elif defined( __WATCOMC__ )
         fp = (unsigned char *)MK_FP(0xA000, (unsigned int)fpb);
#else
         fp = MK_FP(0xA000, fpb);
#endif

#ifndef __DJGPP__
         *fp = color->byte[i];
#endif
         }
      }
#endif
}

/* Draw a line between two points */
static void
line2d(int x1, int y1, int x2, int y2, quantized_color *qcolor)
{
   int d1, x, y;
   int ax, ay;
   int sx, sy;
   int dx, dy;

   dx = x2 - x1;
   ax = ABS(dx) << 1;
   sx = SGN(dx);
   dy = y2 - y1;
   ay = ABS(dy) << 1;
   sy = SGN(dy);

   x = x1;
   y = y1;

   plotpoint(x, y, qcolor);
   if (ax > ay) {
      /* x dominant */
      d1 = ay - (ax >> 1);
      for (;;) {
         if (x==x2) return;
         if (d1>=0) {
            y += sy;
            d1 -= ax;
            }
         x += sx;
         d1 += ay;
         plotpoint(x, y, qcolor);
         }
      }
   else {
      /* y dominant */
      d1 = ax - (ay >> 1);
      for (;;) {
         if (y == y2) return;
         if (d1 >= 0) {
            x += sx;
            d1 -= ay;
            }
         y += sy;
         d1 += ax;
         plotpoint(x, y, qcolor);
         }
      }
}

void
display_clear(void)
{
   unsigned i, j;

   if (Display_Flag >= 1) {
      /* clear the display the hard way, a pixel at a time. */
      setvesabank(0);
      for (i=offy;i<maxy;i++)
         for (j=offx;j<maxx;j++)
            if (Display_Flag < FIRST_HICOLOR_MODE)
               plotpoint(j, i, &black1);
            else if (Display_Flag < FIRST_TRUECOLOR_MODE)
               plotpoint(j, i, &black2);
            else
               plotpoint(j, i, &black3);
      }
}



void
display_init(Viewpoint *eye, char *S)
{
   static int init_flag = 0;
   int sflag, xres, yres, x1, y1;

   #if defined( __DJGPP__   )
     video = __dpmi_segment_to_descriptor(0xa000);
   #endif


   sflag = 0; /* No subscreen window */
   xres = eye->view_xres;
   yres = eye->view_yres;
   if (Display_x0 < 0) Display_x0 = 0; else sflag = 1;
   if (Display_y0 < 0) Display_y0 = 0; else sflag = 1;
   if (Display_xl < 0) Display_xl = xres; else sflag = 1;
   if (Display_yl < 0) Display_yl = yres; else sflag = 1;

   /* Now go set up the display */
   if (init_flag)
      display_clear();
   else {
#if defined( DOS386 ) || defined( __GNUC__ ) || defined( __WATCOMC__ )
      if (Display_Flag != FIRST_8BIT_MODE)
         find_real_buf();
#endif
      init_flag = 1;
      switch (Display_Flag) {

         /* Truecolor video modes */
         case FIRST_TRUECOLOR_MODE+4:
            if (setvesamode(0x11b)) {
               Display_Flag = FIRST_TRUECOLOR_MODE+4;
               break;
               }
         case FIRST_TRUECOLOR_MODE+3:
            if (setvesamode(0x118)) {
               Display_Flag = FIRST_TRUECOLOR_MODE+3;
               break;
               }
         case FIRST_TRUECOLOR_MODE+2:
            if (setvesamode(0x115)) {
               Display_Flag = FIRST_TRUECOLOR_MODE+2;
               break;
               }
         case FIRST_TRUECOLOR_MODE+1:
            if (setvesamode(0x112)) {
               Display_Flag = FIRST_TRUECOLOR_MODE+1;
               break;
               }
         case FIRST_TRUECOLOR_MODE:
            if (setvesamode(0x10f)) {
               Display_Flag = FIRST_TRUECOLOR_MODE;
               break;
               }

         /* Hicolor video modes */
         case FIRST_16BIT_MODE+4:
            if (setvesamode(0x11a)) {
               Display_Flag = FIRST_16BIT_MODE+4;
               break;
               }
         case FIRST_HICOLOR_MODE+4:
            if (setvesamode(0x119)) {
               Display_Flag = FIRST_HICOLOR_MODE+4;
               break;
               }
         case FIRST_16BIT_MODE+3:
            if (setvesamode(0x117)) {
               Display_Flag = FIRST_16BIT_MODE+3;
               break;
               }
         case FIRST_HICOLOR_MODE+3:
            if (setvesamode(0x116)) {
               Display_Flag = FIRST_HICOLOR_MODE+3;
               break;
               }
         case FIRST_16BIT_MODE+2:
            if (setvesamode(0x114)) {
               Display_Flag = FIRST_16BIT_MODE+2;
               break;
               }
         case FIRST_HICOLOR_MODE+2:
            if (setvesamode(0x113)) {
               Display_Flag = FIRST_HICOLOR_MODE+2;
               break;
               }
         case FIRST_16BIT_MODE+1:
            if (setvesamode(0x111)) {
               Display_Flag = FIRST_16BIT_MODE+1;
               break;
               }
         case FIRST_HICOLOR_MODE+1:
            if (setvesamode(0x110)) {
               Display_Flag = FIRST_HICOLOR_MODE+1;
               break;
               }
         case FIRST_16BIT_MODE:
            if (setvesamode(0x10e)) {
               Display_Flag = FIRST_16BIT_MODE;
               break;
               }
         case FIRST_HICOLOR_MODE:
            if (setvesamode(0x10d)) {
               Display_Flag = FIRST_HICOLOR_MODE;
               break;
               }

         /* Now try the 8 bit modes, starting at the most extreme */
         case FIRST_8BIT_MODE+4:
            if (setvesamode(0x107)) {
               Display_Flag = FIRST_8BIT_MODE+4;
               pallette_init();
               break;
               }
         case FIRST_8BIT_MODE+3:
            if (setvesamode(0x105)) {
               Display_Flag = FIRST_8BIT_MODE+3;
               pallette_init();
               break;
               }
         case FIRST_8BIT_MODE+2:
            if (setvesamode(0x103)) {
               Display_Flag = FIRST_8BIT_MODE+2;
               pallette_init();
               break;
               }
         case FIRST_8BIT_MODE+1:
            if (setvesamode(0x101)) {
               Display_Flag = FIRST_8BIT_MODE+1;
               pallette_init();
               break;
               }
         case FIRST_8BIT_MODE:
            if (Reset_Display_Flag)
               setvideomode(19);
            screen_maxx = 320;
            screen_maxy = 200;
            Display_Flag = FIRST_8BIT_MODE;
            pallette_init();
            break;

         case FIRST_4BIT_MODE+1:
            if (Reset_Display_Flag)
               setvideomode(18);
            screen_maxx = 640;
            screen_maxy = 480;
            Pallette_Flag = 3; /* Must be 4 bit pallette */
            Display_Flag = FIRST_4BIT_MODE+1;
            break;

         case FIRST_4BIT_MODE:
            if (Reset_Display_Flag)
               setvideomode(13);
            screen_maxx = 320;
            screen_maxy = 200;
            Pallette_Flag = 3; /* Must be 4 bit pallette */
            Display_Flag = FIRST_4BIT_MODE;
            break;

         default:
            error("Failed to set video mode: %d\n", Display_Flag);
            }
         }

   /* Do some conditioning on the screen window to ensure it doesn't cause
      pixels to be generated off screen */
   x1 = Display_x0 + Display_xl;
   y1 = Display_y0 + Display_yl;
   if (Display_x0 < 0) { Display_x0 = 0; }
   if (Display_y0 < 0) { Display_y0 = 0; }
   if (x1 >= screen_maxx) { Display_xl = screen_maxx - Display_x0; }
   if (y1 >= screen_maxy) { Display_yl = screen_maxy - Display_y0; }

   /* If there is a rendering subwindow of the screen that we should
      be working in.  If so, then set the scaling values appropriately. */
   if (sflag) {
      offx = Display_x0;
      offy = Display_y0;
      maxx = Display_x0 + Display_xl;
      maxy = Display_y0 + Display_yl;
      }
   else {
      offx = 0;
      offy = 0;
      maxx = screen_maxx;
      maxy = screen_maxy;
      }

#if 0
   if (xres > Display_xl)
      X_Display_Scale = (Flt)Display_xl / (Flt)xres;
   else
      X_Display_Scale = 1.0;
   if (yres > Display_yl)
      Y_Display_Scale = (Flt)Display_yl / (Flt)yres;
   else
      Y_Display_Scale = 1.0;
#else
      X_Display_Scale = (Flt)Display_xl / (Flt)xres;
      Y_Display_Scale = (Flt)Display_yl / (Flt)yres;
#endif
   if (X_Display_Scale < Y_Display_Scale)
      Y_Display_Scale = X_Display_Scale;
   else if (Y_Display_Scale < X_Display_Scale)
      X_Display_Scale = Y_Display_Scale;
}

void
display_close(int wait_flag)
{
#if !defined ( __BCC55__ )
    union REGS regs;

   if (wait_flag) {
#if !defined( _WINDOWS )
      while (!_kbhit()) ;
#endif
      if (!getch()) getch();
      }

   if (pallette != NULL) {
      polyray_free(pallette);
      pallette = NULL;
      }

#if defined( __WATCOMC__ )
    regs.x.ax = 0x0003;
    int86(0x10, &regs, &regs);
    regs.x.ax = 0x0101;            /* DPMI free DOS memory */
    regs.x.dx = answerSel;         /* Descriptor (memory) to be freed */
    int86 (0x31, &regs, &regs);
#endif

   /* Go back to standard text mode */
   regs.x.ax = 0x0003;
   int86(0x10, &regs, &regs);

#endif
   return;
}

static void
clip_display_point(int *x, int *y)
{
   if (*x < Display_x0)
      *x = Display_x0;
   else if (*x >= Display_x0 + Display_xl)
      *x = Display_x0 + Display_xl - 1;
   if (*y < Display_y0)
      *y = Display_y0;
   else if (*y >= Display_y0 + Display_yl)
      *y = Display_y0 + Display_yl - 1;
}

static void
putbox(int x0, int y0, int x1, int y1, quantized_color *qcolor)
{
   int i, j;

   for (i=y0;i<y1;i++)
      for (j=x0;j<x1;j++)
         plotpoint(j, i, qcolor);
}

void
display_box(int x0, int y0, int x1, int y1, Vec color)
{
   quantized_color qcolor;
   int xt0, yt0, xt1, yt1;

   yt0 = (int)((Flt)offy + Y_Display_Scale * (Flt)y0);
   xt0 = (int)((Flt)offx + X_Display_Scale * (Flt)x0);
   yt1 = (int)((Flt)offy + Y_Display_Scale * (Flt)(y1));
   xt1 = (int)((Flt)offx + X_Display_Scale * (Flt)(x1));

   clip_display_point(&xt0, &yt0);
   clip_display_point(&xt1, &yt1);

   quantize(x0, y0, color, &qcolor);
   putbox(xt0, yt0, xt1, yt1, &qcolor);
}

void
display_plot(int x, int y, Vec color)
{
   int x0, y0, x1, y1;
   quantized_color qcolor;

   y0 = (int)((Flt)offy + Y_Display_Scale * (Flt)y);
   x0 = (int)((Flt)offx + X_Display_Scale * (Flt)x);
   y1 = (int)((Flt)offy + Y_Display_Scale * (Flt)(y + 1));
   x1 = (int)((Flt)offx + X_Display_Scale * (Flt)(x + 1));

   clip_display_point(&x0, &y0);
   clip_display_point(&x1, &y1);

   quantize(x, y, color, &qcolor);
   if (x0 == x1 && y0 == y1)
      plotpoint(x0, y0, &qcolor);
   else
      putbox(x0, y0, x1, y1, &qcolor);
}

void
display_line(int x0, int y0, int x1, int y1, Vec color)
{
   quantized_color qcolor;

   /* Scale from image size to actual screen pixel size */
   x0 = offx + X_Display_Scale * x0;
   y0 = offy + Y_Display_Scale * y0;
   x1 = offx + X_Display_Scale * x1;
   y1 = offy + Y_Display_Scale * y1;

   clip_display_point(&x0, &y0);
   clip_display_point(&x1, &y1);

   quantize(x0, y0, color, &qcolor);
   line2d(x0, y0, x1, y1, &qcolor);
}

#endif


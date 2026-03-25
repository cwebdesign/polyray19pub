#if !defined(__DISPLAY_DEFS)
#define __DISPLAY_DEFS
/**
  Polyray � MIT Licensed Revival
  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Copyright (C) 1999-2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the �Software�), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/* Video resolutions and mode starting numbers */
#define VIDEO_RESOLUTIONS 5
#define FIRST_8BIT_MODE 1
#define FIRST_HICOLOR_MODE (FIRST_8BIT_MODE + VIDEO_RESOLUTIONS)
#define FIRST_16BIT_MODE (FIRST_HICOLOR_MODE + VIDEO_RESOLUTIONS)
#define FIRST_TRUECOLOR_MODE  (FIRST_16BIT_MODE + VIDEO_RESOLUTIONS)
#define FIRST_4BIT_MODE (FIRST_TRUECOLOR_MODE + VIDEO_RESOLUTIONS)

extern int Pallette_Start;
extern int Pallette_Flag;
extern int Display_Flag;
extern int Reset_Display_Flag;
extern int Display_x0, Display_y0, Display_xl, Display_yl;
extern int Dither_Flag;

/* Display routines */
void display_init(Viewpoint *, char *);
void display_clear(void);
void display_plot(int, int, Vec);
void display_close(int);
void display_line(int, int, int, int, Vec);
void display_box(int, int, int, int, Vec);

#endif /* __DISPLAY_DEFS */

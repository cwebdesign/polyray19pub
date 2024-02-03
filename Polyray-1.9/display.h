#if !defined(__DISPLAY_DEFS)
#define __DISPLAY_DEFS

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

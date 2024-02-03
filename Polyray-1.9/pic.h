#if !defined(__POLYRAY_PICTURE_DEFS)
#define __POLYRAY_PICTURE_DEFS

/* Routines for managing the output image */

extern int           buffer_update;
extern unsigned long buffer_size;

extern Pic *TGAOpen(char *, Viewpoint *, int, int, unsigned int);
extern void TGAClose(Pic *);
extern void TGADump(Viewpoint *, Pic *, unsigned int, unsigned int);
extern void PBMDump(Viewpoint *, Pic *, unsigned int, unsigned int);

extern int get_old_image_line(Viewpoint *, Pic *, unsigned);

#endif /* __POLYRAY_PICTURE_DEFS */

#if !defined(__POLYRAY_PICTURE_DEFS)
#define __POLYRAY_PICTURE_DEFS


// Routines for managing the output image 



Pic *TGAOpen(char *, Viewpoint *, int, int, unsigned int);
void TGAClose(Pic *);
void TGADump(Viewpoint *, Pic *, unsigned int, unsigned int);
void PBMDump(Viewpoint *, Pic *, unsigned int, unsigned int);

int get_old_image_line(Viewpoint *, Pic *, unsigned);


#endif /* __POLYRAY_PICTURE_DEFS */

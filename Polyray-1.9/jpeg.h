#if !defined(__POLYRAY_JPEG_DEFS)
#define __POLYRAY_JPEG_DEFS

/* Tries to read a file as a JPEG image, returns 1 if
   it was successful, 0 otherwise. */
int read_JPEG_image(FILE *, Img *);
int read_GIF_image(FILE *, Img *);
int read_PNG_image(FILE *, Img *);

#endif /* __POLYRAY_JPEG_DEFS */

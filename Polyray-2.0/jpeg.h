#if !defined(__POLYRAY_JPEG_DEFS)
#define __POLYRAY_JPEG_DEFS

#ifdef __cplusplus
	extern "C" {
#endif

/* Tries to read a file as a JPEG image, returns 1 if
   it was successful, 0 otherwise. */
int read_JPEG_image(FILE *, Img *);
int read_GIF_image(FILE *, Img *);
//int read_PNG_image(FILE *, Img *);

#ifdef __cplusplus
  }
#endif

#endif /* __POLYRAY_JPEG_DEFS */

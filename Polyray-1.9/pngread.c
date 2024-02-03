#include <ctype.h>
#include "defs.h"
#include "memory.h"
#include "io.h"
#include "jpeg.h"

#define LITTLE_ENDIAN

#include "pngread.h"

#define DEFINE_ENUMS
#include "pngerrs.h"
#define DEFINE_STRINGS
#include "pngerrs.h"

/* The final image after we are done processing the JPEG image */
static Img *output_image = NULL;

static PNG_STATE ps = {0}; /* Referenced by tempfile.c, etc. */

/*
 * Local definitions and statics
 */

static int decode_chunk(void);
static int decode_IHDR(void);
static int decode_PLTE(void);
static int decode_gAMA(void);
static int decode_tRNS(void);
static int decode_cHRM(void);
static int decode_pHYs(void);
static int decode_oFFs(void);
static int decode_sCAL(void);
static int skip_chunk_data(void);
static int validate_image(IMG_INFO *);

/* PNG reader */
int
read_PNG_image(FILE *ifile, Img *img)
{
    int err;
    // char *cp;
    IMG_INFO *png_image;

    output_image = img;

    png_image = (IMG_INFO *)polyray_malloc((size_t)IMG_SIZE);
    if (NULL == png_image) error(ptot_error_messages[ERR_MEMORY]);

    /* Make sure we are at the beginning of the image file */
    fseek(ifile, 0, SEEK_SET);

    err = read_PNG(ifile, png_image);

    polyray_free(png_image);

    return (err == 0 ? 1 : 0);
}

static void
output_init(IMG_INFO *png_image, Img *image)
{
   int i;
   unsigned char **imgbuf;

   /* output_image->filename has already been set before getting here */
   image->copy     = 0;
   image->psize    = png_image->samples_per_pixel * 8;
   image->cflag    = 0;
   image->width    = png_image->width;
   image->length   = png_image->height;
   image->orien    = 0x20;
   image->cmlen    = 0;
   image->cmsiz    = 3;
   image->cmap     = NULL;

   /* Set the type of the image (matching Targa definitions) */
   if (png_image->samples_per_pixel > 1)
      image->ftype    = 2; /* Raw 24 bit image */
   else if (png_image->is_palette)
      image->ftype    = 1; /* Color mapped image */
   else
      image->ftype    = 3; /* Grayscale image*/

   /* Allocate the row buffers for the image */
   imgbuf = polyray_malloc(image->length * sizeof(unsigned char *));
   if (imgbuf == NULL)
      error("No memory");
   for (i=0;i<image->length;i++) {
      imgbuf[i] = polyray_malloc(image->width * (image->psize/8) *
                                 sizeof(unsigned char));
      if (imgbuf[i] == NULL)
         warning("Failed to allocate image memory\n");
      }
   image->image = imgbuf;           /* Image buffer, fully allocated */
}

/*
 * PNG-specific code begins here.
 *
 * read_PNG() reads the PNG file into the passed IMG_INFO struct.
 * Returns 0 on success.
 */

int
read_PNG(FILE *inf, IMG_INFO *image)
{
    int err;

    memset(image, 0, IMG_SIZE);
    memset(&ps, 0, sizeof ps);

    ps.inf = inf;
    ps.image = image;
    if (NULL == (ps.buf = (U8 *)polyray_malloc(IOBUF_SIZE)))
      return ERR_MEMORY;
    /*
     * Skip signature and possible MacBinary header, and
     * verify signature. A more robust implementation might
     * search for the file signature anywhere in the first
     * 1k bytes or so, but in practice, the method shown
     * is adequate or file I/O applications.
     */
    fread(ps.buf, 1, 8, inf);
    ps.buf[8] = '\0';
    if (0 != memcmp(ps.buf, PNG_Signature, 8)) {
        fread(ps.buf, 1, 128, inf);
        ps.buf[128] = '\0';
        if (0 != memcmp(ps.buf+120, PNG_Signature, 8)) {
            err = ERR_BAD_PNG;
            goto err_out;
        }
    }

    ps.got_first_chunk = ps.got_first_idat = FALSE;
    do {
        if (0 != (err = get_chunk_header())) goto err_out;
        if (0 != (err = decode_chunk())) goto err_out;
        /*
         * IHDR must be the first chunk.
         */
        if (!ps.got_first_chunk &&
          (PNG_CN_IHDR != ps.current_chunk_name))
          warning(ptot_error_messages[WARN_BAD_PNG]);
        ps.got_first_chunk = TRUE;
        /*
         * Extra unused bytes in chunk?
         */
        if (0 != ps.bytes_remaining) {
            warning(ptot_error_messages[WARN_EXTRA_BYTES]);
            if (0 != (err = skip_chunk_data())) goto err_out;
        }
        if (0 != (err = verify_chunk_crc())) goto err_out;

    } while (PNG_CN_IEND != ps.current_chunk_name);

    if (!ps.got_first_idat) {
        err = ERR_NO_IDAT;
        goto err_out;
    }
    if (0 != (err = validate_image(image))) goto err_out;

    if (EOF != getc(inf)) warning(ptot_error_messages[WARN_EXTRA_BYTES]);

    err = 0;
err_out:
    polyray_free(ps.buf);
    return err;
}

/*
 * decode_chunk() is just a dispatcher, shunting the work of
 * decoding the incoming chunk (whose header we have just read)
 * to the appropriate handler.
 */

static int
decode_chunk(
    void)
{
    /*
     * Every case in the switch below should set err. We set it
     * here to gurantee that we hear about it if we don't.
     */
    int err = ERR_ASSERT;

    switch (ps.current_chunk_name) {

    case PNG_CN_IHDR:   err = decode_IHDR();    break;
    case PNG_CN_gAMA:   err = decode_gAMA();    break;
    case PNG_CN_IDAT:   err = decode_IDAT();    break;
    /*
     * PNG allows a suggested colormap for 24-bit images. TIFF
     * does not, and PLTE is not copy-safe, so we discard it.
     */
    case PNG_CN_PLTE:
        if (ps.image->is_palette) err = decode_PLTE();
        else err = skip_chunk_data();
        break;

    case PNG_CN_tRNS:   err = decode_tRNS();    break;
    case PNG_CN_cHRM:   err = decode_cHRM();    break;
    case PNG_CN_pHYs:   err = decode_pHYs();    break;
    case PNG_CN_oFFs:   err = decode_oFFs();    break;
    case PNG_CN_sCAL:   err = decode_sCAL();    break;

    case PNG_CN_tEXt:
    case PNG_CN_zTXt:
    case PNG_CN_tIME:   /* Will be recreated */
    case PNG_CN_hIST:   /* Not safe to copy */
    case PNG_CN_bKGD:
    case PNG_CN_sBIT:
        err = skip_chunk_data();
        break;
    case PNG_CN_IEND:   /* We're done */
        err = 0;
        break;
    default:
        err = skip_chunk_data();
        break;
    }
    return err;
}

/*
 * get_chunk_header() reads the first 8 bytes of each chunk, which
 * include the length and ID fields.  It returns 0 on success.
 * The crc argument is preconditioned and then updated with the
 * chunk name read.
 */

int
get_chunk_header(
    void)
{
    int byte;

    if (8 != fread(ps.buf, 1, 8, ps.inf)) return ERR_READ;

    ps.bytes_remaining = BE_GET32(ps.buf);
    ps.current_chunk_name= BE_GET32(ps.buf+4);
    ps.bytes_in_buf = 0;

    if (ps.bytes_remaining > PNG_MaxChunkLength)
      warning(ptot_error_messages[WARN_BAD_PNG]);

    for (byte = 4; byte < 8; ++byte)
      if (!isalpha(ps.buf[byte])) return ERR_BAD_PNG;

    ps.crc = update_crc(0xFFFFFFFFL, ps.buf+4, 4);
    return 0;
}

/*
 * get_chunk_data() reads chunk data into the buffer,
 * returning the number of bytes actually read.  Do not
 * use this for IDAT chunks; they are dealt with specially
 * by the fill_buf() function.
 */

U32
get_chunk_data(
    U32 bytes_requested)
{
    ps.bytes_in_buf = (U32)fread(ps.buf, 1,
      (size_t)min(IOBUF_SIZE, bytes_requested), ps.inf);

    ps.bytes_remaining -= ps.bytes_in_buf;

    ps.crc = update_crc(ps.crc, ps.buf, ps.bytes_in_buf);
    return ps.bytes_in_buf;
}

/*
 * Assuming we have read a chunk header and all the chunk data,
 * we now check to see that the CRC stored at the end of the
 * chunk matches the one we've calculated.
 */

int
verify_chunk_crc(
    void)
{
    if (4 != fread(ps.buf, 1, 4, ps.inf)) return ERR_READ;

    if ((ps.crc ^ 0xFFFFFFFFL) != BE_GET32(ps.buf)) {
        warning(ptot_error_messages[WARN_BAD_CRC]);
    }
    return 0;
}

/*
 * Read and decode IHDR. Errors that would probably cause the
 * IDAT reader to fail are returned as errors; less serious
 * errors generate a warning but continue anyway.
 */

static int
decode_IHDR(
    void)
{
    if (ps.bytes_remaining < 13) return ERR_BAD_PNG;
    if (13 != get_chunk_data(13)) return ERR_READ;

    ps.image->width = BE_GET32(ps.buf);
    ps.image->height = BE_GET32(ps.buf+4);

    if (0 != ps.buf[10] || 0 != ps.buf[11])
      return ERR_BAD_PNG;   /* Compression & filter type */

    ps.image->is_interlaced = ps.buf[12];
    if (!(0 == ps.image->is_interlaced ||
      1 == ps.image->is_interlaced)) return ERR_BAD_PNG;

    ps.image->is_color = (0 != (ps.buf[9] & PNG_CB_Color));
    ps.image->is_palette = (0 != (ps.buf[9] & PNG_CB_Palette));
    ps.image->has_alpha = (0 != (ps.buf[9] & PNG_CB_Alpha));

    ps.image->samples_per_pixel = 1;
    if (ps.image->is_color && !ps.image->is_palette)
      ps.image->samples_per_pixel = 3;
    if (ps.image->has_alpha) ++ps.image->samples_per_pixel;

    if (ps.image->is_palette && ps.image->has_alpha)
      warning(ptot_error_messages[WARN_BAD_PNG]);
    /*
     * Check for invalid bit depths.  If a bitdepth is
     * not one we can read, abort processing.  If we can
     * read it, but it is illegal, issue a warning and
     * continue anyway.
     */
    ps.image->bits_per_sample = ps.buf[8];

    if (!(1 == ps.buf[8] || 2 == ps.buf[8] || 4 == ps.buf[8] ||
      8 == ps.buf[8] || 16 == ps.buf[8])) return ERR_BAD_PNG;

    if ((ps.buf[8] > 8) && ps.image->is_palette)
      warning(ptot_error_messages[WARN_BAD_PNG]);

    if ((ps.buf[8] < 8) && (2 == ps.buf[9] || 4 == ps.buf[9] ||
      6 == ps.buf[9])) return ERR_BAD_PNG;

    output_init(ps.image, output_image);

    return 0;
}

/*
 * Decode gAMA chunk.
 */

static int
decode_gAMA(
    void)
{
    if (0 != ps.image->palette_size)
      warning(ptot_error_messages[WARN_LATE_GAMA]);

    if (ps.bytes_remaining < 4) return ERR_BAD_PNG;
    if (4 != get_chunk_data(4)) return ERR_READ;

    ps.image->source_gamma = (double)BE_GET32(ps.buf) / 100000.0;
    return 0;
}

/*
 * Decode PLTE chunk. Number of entries is determined by
 * chunk length. A non-multiple of 3 is technically an error;
 * we just issue a warning in that case. IOBUF_SIZE must be
 * 768 or greater, so we check that at compile time here.
 */

#if (IOBUF_SIZE < 768)
#  error "IOBUF_SIZE must be >= 768"
#endif

static int
decode_PLTE(
    void)
{
    int i; //, j;
    U32 bytes_read;

    if (!ps.image->is_color) warning(ptot_error_messages[WARN_PLTE_GRAY]);
    if (0 != ps.image->palette_size) {
        warning(ptot_error_messages[WARN_MULTI_PLTE]);
        return skip_chunk_data();
    }
    ps.image->palette_size =
      min(256, (int)(ps.bytes_remaining / 3));
    if (0 == ps.image->palette_size) return ERR_BAD_PNG;

    bytes_read = get_chunk_data(3 * ps.image->palette_size);
    if (bytes_read < (U32)(3 * ps.image->palette_size))
      return ERR_READ;

    memcpy(ps.image->palette, ps.buf, 3 * ps.image->palette_size);

    output_image->cmap = polyray_malloc(3 * ps.image->palette_size);

    /* Correct the rgb orientation of PNG to the bgr of Polyray */
    for (i=0;i<3*ps.image->palette_size;i+=3) {
       output_image->cmap[i  ] = ps.buf[i+2];
       output_image->cmap[i+1] = ps.buf[i+1];
       output_image->cmap[i+2] = ps.buf[i  ];
       }
    output_image->cmlen = ps.image->palette_size;
    output_image->cmsiz = 3;

    return 0;
}

/*
 * Copy transparency data into structure. We will later expand the
 * TIFF data into full alpha to account for its lack of this data.
 */

static int
decode_tRNS(
    void)
{
    int i;
    U32 bytes_read;

    if (ps.image->has_trns) warning(ptot_error_messages[WARN_MULTI_TRNS]);
    ps.image->has_trns = TRUE;

    if (ps.image->is_palette) {
        if (0 == ps.image->palette_size) {
            warning(ptot_error_messages[WARN_LATE_TRNS]);
        }
        bytes_read = get_chunk_data(ps.bytes_remaining);
        memcpy(ps.image->palette_trans_bytes,
          ps.buf, (size_t)bytes_read);

        for (i = bytes_read; i < ps.image->palette_size; ++i)
          ps.image->palette_trans_bytes[i] = 255;

    } else if (ps.image->is_color) {
        if (ps.bytes_remaining < 6) return ERR_BAD_PNG;
        bytes_read = get_chunk_data(6);
        for (i = 0; i < 3; ++i)
          ps.image->trans_values[i] = BE_GET16(ps.buf + 2 * i);
    } else {
        if (ps.bytes_remaining < 2) return ERR_BAD_PNG;
        ps.image->trans_values[0] = BE_GET16(ps.buf);
    }
    return 0;
}

static int
decode_cHRM(
    void)
{
    int i;

    if (ps.bytes_remaining < 32) return ERR_BAD_PNG;
    if (32 != get_chunk_data(32)) return ERR_READ;

    for (i = 0; i < 8; ++i)
      ps.image->chromaticities[i] = BE_GET32(ps.buf + 4 * i);

    return 0;
}

static int
decode_pHYs(
    void)
{
    if (ps.bytes_remaining < 9) return ERR_BAD_PNG;
    if (9 != get_chunk_data(9)) return ERR_READ;

    ps.image->resolution_unit = ps.buf[8];
    if (ps.buf[8] > PNG_MU_Meter) warning(ptot_error_messages[WARN_BAD_VAL]);

    ps.image->xres = BE_GET32(ps.buf);
    ps.image->yres = BE_GET32(ps.buf + 4);

    return 0;
}

static int
decode_oFFs(
    void)
{
    if (ps.bytes_remaining < 9) return ERR_BAD_PNG;
    if (9 != get_chunk_data(9)) return ERR_READ;

    ps.image->offset_unit = ps.buf[8];
    if (ps.buf[8] > PNG_MU_Micrometer) warning(ptot_error_messages[WARN_BAD_VAL]);

    ps.image->xoffset = BE_GET32(ps.buf);
    ps.image->yoffset = BE_GET32(ps.buf + 4);

    return 0;
}

/*
 * Decode sCAL chunk. Note: as of this writing, this is not
 * an official PNG chunk. It probably will be by the time
 * you read this, but it might possibly change in some way.
 * You have been warned. It also has no TIFF equivalent, so
 * this only gets read into the structure.
 */

static int
decode_sCAL(
    void)
{
    get_chunk_data(ps.bytes_remaining);
    if (ps.bytes_in_buf == IOBUF_SIZE) {
        --ps.bytes_in_buf;
        warning(ptot_error_messages[WARN_BAD_PNG]);
    }
    ps.buf[ps.bytes_in_buf] = '\0';

    ps.image->scale_unit = ps.buf[0];
    if (ps.buf[0] < PNG_MU_Meter || ps.buf[0] > PNG_MU_Radian)
      warning(ptot_error_messages[WARN_BAD_VAL]);

    ps.image->xscale = atof((char *)ps.buf+1);
    ps.image->yscale = atof((char *)ps.buf + (strlen((char *)ps.buf+1)) + 2);

    return 0;
}

/*
 * Skip all remaining data in current chunk.
 */

static int
skip_chunk_data(
    void)
{
    U32 bytes_read;

    do {
        bytes_read = get_chunk_data(ps.bytes_remaining);
    } while (0 != bytes_read);

    return 0;
}

/*
 * Ensure that the image structure we have created by reading
 * the input PNG is compatible with whatever we intend to do
 * with it. In this case, TIFF can handle anything, so we just
 * use this as a sanity check on some basic assumptions.
 */

static int
validate_image(
    IMG_INFO *image)
{
    if (0 == image->width || 0 == image->height)
      return ERR_BAD_IMAGE;
    if (image->samples_per_pixel < 1 ||
      image->samples_per_pixel > 4) return ERR_BAD_IMAGE;
    if (image->is_palette && (image->palette_size < 1 ||
      image->palette_size > 256)) return ERR_BAD_IMAGE;

    return 0;
}

/*
 * End of ptot.c.
 */

/*
 * crc32.c
 *
 * Function to calculate 32-bit CRC values for PNG chunks.
 *
 **********
 *
 * HISTORY
 *
 * 95-03-10 Created by Lee Daniel Crocker <lee@piclab.com>
 *          <URL:http://www.piclab.com/piclab/index.html>
 */

static U32 crc_table[256] = { 0xFFFFFFFFL };
static void build_crc_table(U32 *);

U32
update_crc(
    U32 input_crc,
    U8 *data,
    U32 count)
{
    U32 crc, byte;

    if (0xFFFFFFFFL == *crc_table) build_crc_table(crc_table);

    crc = input_crc;

    for (byte = 0; byte < count; ++byte) {
        crc = ((crc >> 8) & 0xFFFFFFL) ^
          crc_table[ (crc ^ data[byte]) & 0xFF ];
    }
    return crc;
}

static void
build_crc_table(
    U32 *table)
{
    int byte, bit;
    U32 accum;

    for (byte = 0; byte < 256; ++byte) {
        accum = byte;

        for (bit = 0; bit < 8; ++bit) {
            if (accum & 1) accum = (accum >> 1) ^ 0xEDB88320L;
            else accum >>= 1;
        }
        table[byte] = accum;
    }
}
/* inflate.c -- put in the public domain by Mark Adler
   version c14p, 4 October 1994 */
#define PKZIP_BUG_WORKAROUND    /* PKZIP 1.93a problem--live with it */

/*
    inflate.h must supply the uch slide[WSIZE] array, the voidp typedef
    (void if (void *) is accepted, else char) and the NEXTBYTE,
    FLUSH() and memzero macros.  If the window size is not 32K, it
    should also define WSIZE.  If INFMOD is defined, it can include
    compiled functions to support the NEXTBYTE and/or FLUSH() macros.
    There are defaults for NEXTBYTE and FLUSH() below for use as
    examples of what those functions need to do.  Normally, you would
    also want FLUSH() to compute a crc on the data.  inflate.h also
    needs to provide these typedefs:

        typedef unsigned char uch;
        typedef unsigned short ush;
        typedef unsigned long ulg;

    This module uses the external functions malloc() and free() (and
    probably memset() or bzero() in the memzero() macro).  Their
    prototypes are normally found in <string.h> and <stdlib.h>.
 */
#define INFMOD          /* tell inflate.h to include code to be compiled */

#ifndef WSIZE           /* default is 32K */
#  define WSIZE 0x8000  /* window size--must be a power of two, and at least */
#endif                  /* 32K for zip's deflate method */

#ifndef NEXTBYTE        /* default is to simply get a byte from stdin */
#  define NEXTBYTE getchar()
#endif

#ifndef FPRINTF
#  define FPRINTF fprintf
#endif

#ifndef FLUSH           /* default is to simply write the buffer to stdout */
#  define FLUSH(n) fwrite(slide, 1, n, stdout)  /* return value not used */
#endif
/* Warning: the fwrite above might not work on 16-bit compilers, since
   0x8000 might be interpreted as -32,768 by the library function. */

#ifndef Trace
#  ifdef DEBUG
#    define Trace(x) fprintf x
#  else
#    define Trace(x)
#  endif
#endif


/* Huffman code lookup table entry--this entry is four bytes for machines
   that have 16-bit pointers (e.g. PC's in the small or medium model).
   Valid extra bits are 0..13.  e == 15 is EOB (end of block), e == 16
   means that v is a literal, 16 < e < 32 means that v is a pointer to
   the next table, which codes e - 16 bits, and lastly e == 99 indicates
   an unused code.  If a code with e == 99 is looked up, this implies an
   error in the data. */
struct huft {
  uch e;                /* number of extra bits or operation */
  uch b;                /* number of bits in this code or subcode */
  union {
    ush n;              /* literal, length base, or distance base */
    struct huft *t;     /* pointer to next level of table */
  } v;
};


/* Function prototypes */
static int huft_build(unsigned *, unsigned, unsigned, ush *, ush *,
                   struct huft **, int *);
static int huft_free(struct huft *);
int inflate_codes(struct huft *, struct huft *, int, int);
int inflate_stored(void);
int inflate_fixed(void);
int inflate_dynamic(void);
int inflate_block(int *);
int inflate(void);
int inflate_free(void);


/* The inflate algorithm uses a sliding 32K byte window on the uncompressed
   stream to find repeated byte strings.  This is implemented here as a
   circular buffer.  The index is updated simply by incrementing and then
   and'ing with 0x7fff (32K-1). */
/* It is left to other modules to supply the 32K area.  It is assumed
   to be usable as if it were declared "uch slide[32768];" or as just
   "uch *slide;" and then malloc'ed in the latter case.  The definition
   must be in unzip.h, included above. */
unsigned wp;            /* current position in slide */


/* Tables for deflate from PKZIP's appnote.txt. */
static unsigned border[] = {    /* Order of the bit length code lengths */
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
static ush cplens[] = {         /* Copy lengths for literal codes 257..285 */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
        /* note: see note #13 above about the 258 in this list. */
static ush cplext[] = {         /* Extra bits for literal codes 257..285 */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99}; /* 99==invalid */
static ush cpdist[] = {         /* Copy offsets for distance codes 0..29 */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
static ush cpdext[] = {         /* Extra bits for distance codes */
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};

/* And'ing with mask[n] masks the lower n bits */
ush mask[] = {
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};


/* Macros for inflate() bit peeking and grabbing.
   The usage is:

        NEEDBITS(j)
        x = b & mask[j];
        DUMPBITS(j)

   where NEEDBITS makes sure that b has at least j bits in it, and
   DUMPBITS removes the bits from b.  The macros use the variable k
   for the number of bits in b.  Normally, b and k are register
   variables for speed, and are initialized at the begining of a
   routine that uses these macros from a global bit buffer and count.

   In order to not ask for more bits than there are in the compressed
   stream, the Huffman tables are constructed to only ask for just
   enough bits to make up the end-of-block code (value 256).  Then no
   bytes need to be "returned" to the buffer at the end of the last
   block.  See the huft_build() routine.
 */

ulg bb;                         /* bit buffer */
unsigned bk;                    /* bits in bit buffer */

#ifndef CHECK_EOF
#  define NEEDBITS(n) {while(k<(n)){b|=((ulg)NEXTBYTE)<<k;k+=8;}}
#else
#  define NEEDBITS(n) {while(k<(n)){int c=NEXTBYTE;if(c==EOF)return 1;\
    b|=((ulg)c)<<k;k+=8;}}
#endif                      /* Piet Plomp:  change "return 1" to "break" */

#define DUMPBITS(n) {b>>=(n);k-=(n);}


/*
   Huffman code decoding is performed using a multi-level table lookup.
   The fastest way to decode is to simply build a lookup table whose
   size is determined by the longest code.  However, the time it takes
   to build this table can also be a factor if the data being decoded
   is not very long.  The most common codes are necessarily the
   shortest codes, so those codes dominate the decoding time, and hence
   the speed.  The idea is you can have a shorter table that decodes the
   shorter, more probable codes, and then point to subsidiary tables for
   the longer codes.  The time it costs to decode the longer codes is
   then traded against the time it takes to make longer tables.

   This results of this trade are in the variables lbits and dbits
   below.  lbits is the number of bits the first level table for literal/
   length codes can decode in one step, and dbits is the same thing for
   the distance codes.  Subsequent tables are also less than or equal to
   those sizes.  These values may be adjusted either when all of the
   codes are shorter than that, in which case the longest code length in
   bits is used, or when the shortest code is *longer* than the requested
   table size, in which case the length of the shortest code in bits is
   used.

   There are two different values for the two tables, since they code a
   different number of possibilities each.  The literal/length table
   codes 286 possible values, or in a flat code, a little over eight
   bits.  The distance table codes 30 possible values, or a little less
   than five bits, flat.  The optimum values for speed end up being
   about one bit more than those, so lbits is 8+1 and dbits is 5+1.
   The optimum values may differ though from machine to machine, and
   possibly even between compilers.  Your mileage may vary.
 */


int lbits = 9;          /* bits in base literal/length lookup table */
int dbits = 6;          /* bits in base distance lookup table */


/* If BMAX needs to be larger than 16, then h and x[] should be ulg. */
#define BMAX 16         /* maximum bit length of any code (16 for explode) */
#define N_MAX 288       /* maximum number of codes in any set */


unsigned hufts;         /* track memory usage */


static int
huft_build(b, n, s, d, e, t, m)
unsigned *b;            /* code lengths in bits (all assumed <= BMAX) */
unsigned n;             /* number of codes (assumed <= N_MAX) */
unsigned s;             /* number of simple-valued codes (0..s-1) */
ush *d;                 /* list of base values for non-simple codes */
ush *e;                 /* list of extra bits for non-simple codes */
struct huft **t;        /* result: starting table */
int *m;                 /* maximum lookup bits, returns actual */
/* Given a list of code lengths and a maximum table size, make a set of
   tables to decode that set of codes.  Return zero on success, one if
   the given code set is incomplete (the tables are still built in this
   case), two if the input is invalid (all zero length codes or an
   oversubscribed set of lengths), and three if not enough memory.
   The code with value 256 is special, and the tables are constructed
   so that no bits beyond that code are fetched when that code is
   decoded. */
{
  unsigned a;                   /* counter for codes of length k */
  unsigned c[BMAX+1];           /* bit length count table */
  unsigned el;                  /* length of EOB code (value 256) */
  unsigned f;                   /* i repeats in table every f entries */
  int g;                        /* maximum code length */
  int h;                        /* table level */
  register unsigned i;          /* counter, current code */
  register unsigned j;          /* counter */
  register int k;               /* number of bits in current code */
  int lx[BMAX+1];               /* memory for l[-1..BMAX-1] */
  int *l = lx+1;                /* stack of bits per table */
  register unsigned *p;         /* pointer into c[], b[], or v[] */
  register struct huft *q;      /* points to current table */
  struct huft r;                /* table entry for structure assignment */
  struct huft *u[BMAX];         /* table stack */
  static unsigned v[N_MAX];     /* values in order of bit length */
  register int w;               /* bits before this table == (l * h) */
  unsigned x[BMAX+1];           /* bit offsets, then code stack */
  unsigned *xp;                 /* pointer into x */
  int y;                        /* number of dummy codes added */
  unsigned z;                   /* number of entries in current table */


  /* Generate counts for each bit length */
  el = n > 256 ? b[256] : BMAX; /* set length of EOB code, if any */
  memzero((char *)c, sizeof(c));
  p = b;  i = n;
  do {
    c[*p]++; p++;               /* assume all entries <= BMAX */
  } while (--i);
  if (c[0] == n)                /* null input--all zero length codes */
  {
    *t = (struct huft *)NULL;
    *m = 0;
    return 0;
  }


  /* Find minimum and maximum length, bound *m by those */
  for (j = 1; j <= BMAX; j++)
    if (c[j])
      break;
  k = j;                        /* minimum code length */
  if ((unsigned)*m < j)
    *m = j;
  for (i = BMAX; i; i--)
    if (c[i])
      break;
  g = i;                        /* maximum code length */
  if ((unsigned)*m > i)
    *m = i;


  /* Adjust last length count to fill out codes, if needed */
  for (y = 1 << j; j < i; j++, y <<= 1)
    if ((y -= c[j]) < 0)
      return 2;                 /* bad input: more codes than bits */
  if ((y -= c[i]) < 0)
    return 2;
  c[i] += y;


  /* Generate starting offsets into the value table for each length */
  x[1] = j = 0;
  p = c + 1;  xp = x + 2;
  while (--i) {                 /* note that i == g from above */
    *xp++ = (j += *p++);
  }


  /* Make a table of values in order of bit lengths */
  p = b;  i = 0;
  do {
    if ((j = *p++) != 0)
      v[x[j]++] = i;
  } while (++i < n);


  /* Generate the Huffman codes and for each, make the table entries */
  x[0] = i = 0;                 /* first Huffman code is zero */
  p = v;                        /* grab values in bit order */
  h = -1;                       /* no tables yet--level -1 */
  w = l[-1] = 0;                /* no bits decoded yet */
  u[0] = (struct huft *)NULL;   /* just to keep compilers happy */
  q = (struct huft *)NULL;      /* ditto */
  z = 0;                        /* ditto */

  /* go through the bit lengths (k already is bits in shortest code) */
  for (; k <= g; k++)
  {
    a = c[k];
    while (a--)
    {
      /* here i is the Huffman code of length k bits for value *p */
      /* make tables up to required level */
      while (k > w + l[h])
      {
        w += l[h++];            /* add bits already decoded */

        /* compute minimum size table less than or equal to *m bits */
        z = (z = g - w) > (unsigned)*m ? *m : z;        /* upper limit */
        if ((f = 1 << (j = k - w)) > a + 1)     /* try a k-w bit table */
        {                       /* too few codes for k-w bit table */
          f -= a + 1;           /* deduct codes from patterns left */
          xp = c + k;
          while (++j < z)       /* try smaller tables up to z bits */
          {
            if ((f <<= 1) <= *++xp)
              break;            /* enough codes to use up j bits */
            f -= *xp;           /* else deduct codes from patterns */
          }
        }
        if ((unsigned)w + j > el && (unsigned)w < el)
          j = el - w;           /* make EOB code end at table */
        z = 1 << j;             /* table entries for j-bit table */
        l[h] = j;               /* set table size in stack */

        /* allocate and link in new table */
        if ((q = (struct huft *)polyray_malloc((z + 1)*sizeof(struct huft))) ==
            (struct huft *)NULL)
        {
          if (h)
            huft_free(u[0]);
          return 3;             /* not enough memory */
        }
        hufts += z + 1;         /* track memory usage */
        *t = q + 1;             /* link to list for huft_free() */
        *(t = &(q->v.t)) = (struct huft *)NULL;
        u[h] = ++q;             /* table starts after link */

        /* connect to last table, if there is one */
        if (h)
        {
          x[h] = i;             /* save pattern for backing up */
          r.b = (uch)l[h-1];    /* bits to dump before this table */
          r.e = (uch)(16 + j);  /* bits in this table */
          r.v.t = q;            /* pointer to this table */
          j = (i & ((1 << w) - 1)) >> (w - l[h-1]);
          u[h-1][j] = r;        /* connect to last table */
        }
      }

      /* set up table entry in r */
      r.b = (uch)(k - w);
      if (p >= v + n)
        r.e = 99;               /* out of values--invalid code */
      else if (*p < s)
      {
        r.e = (uch)(*p < 256 ? 16 : 15);    /* 256 is end-of-block code */
        r.v.n = *p++;           /* simple code is just the value */
      }
      else
      {
        r.e = (uch)e[*p - s];   /* non-simple--look up in lists */
        r.v.n = d[*p++ - s];
      }

      /* fill code-like entries with r */
      f = 1 << (k - w);
      for (j = i >> w; j < z; j += f)
        q[j] = r;

      /* backwards increment the k-bit code i */
      for (j = 1 << (k - 1); i & j; j >>= 1)
        i ^= j;
      i ^= j;

      /* backup over finished tables */
      while ((i & ((1 << w) - 1)) != x[h])
        w -= l[--h];            /* don't need to update q */
    }
  }


  /* return actual size of base table */
  *m = l[0];


  /* Return true (1) if we were given an incomplete table */
  return y != 0 && g != 1;
}



static int huft_free(t)
struct huft *t;         /* table to free */
/* Free the malloc'ed tables built by huft_build(), which makes a linked
   list of the tables it made, with the links in a dummy first entry of
   each table. */
{
  register struct huft *p, *q;


  /* Go through linked list, freeing from the malloced (t[-1]) address. */
  p = t;
  while (p != (struct huft *)NULL)
  {
    q = (--p)->v.t;
    polyray_free((voidp *)p);
    p = q;
  }
  return 0;
}



#ifdef ASM_INFLATECODES
#  define inflate_codes(tl,td,bl,bd)  flate_codes(tl,td,bl,bd,(uch *)slide)
   int flate_codes(struct huft *, struct huft *, int, int, uch *);

#else

int inflate_codes(tl, td, bl, bd)
struct huft *tl, *td;   /* literal/length and distance decoder tables */
int bl, bd;             /* number of bits decoded by tl[] and td[] */
/* inflate (decompress) the codes in a deflated (compressed) block.
   Return an error code or zero if it all goes ok. */
{
  register unsigned e;  /* table entry flag/number of extra bits */
  unsigned n, d;        /* length and index for copy */
  unsigned w;           /* current window position */
  struct huft *t;       /* pointer to table entry */
  unsigned ml, md;      /* masks for bl and bd bits */
  register ulg b;       /* bit buffer */
  register unsigned k;  /* number of bits in bit buffer */


  /* make local copies of globals */
  b = bb;                       /* initialize bit buffer */
  k = bk;
  w = wp;                       /* initialize window position */


  /* inflate the coded data */
  ml = mask[bl];           /* precompute masks for speed */
  md = mask[bd];
  while (1)                     /* do until end of block */
  {
    NEEDBITS((unsigned)bl)
    if ((e = (t = tl + ((unsigned)b & ml))->e) > 16)
      do {
        if (e == 99)
          return 1;
        DUMPBITS(t->b)
        e -= 16;
        NEEDBITS(e)
      } while ((e = (t = t->v.t + ((unsigned)b & mask[e]))->e) > 16);
    DUMPBITS(t->b)
    if (e == 16)                /* then it's a literal */
    {
      slide[w++] = (uch)t->v.n;
      if (w == WSIZE)
      {
        FLUSH(w);
        w = 0;
      }
    }
    else                        /* it's an EOB or a length */
    {
      /* exit if end of block */
      if (e == 15)
        break;

      /* get length of block to copy */
      NEEDBITS(e)
      n = t->v.n + ((unsigned)b & mask[e]);
      DUMPBITS(e);

      /* decode distance of block to copy */
      NEEDBITS((unsigned)bd)
      if ((e = (t = td + ((unsigned)b & md))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((unsigned)b & mask[e]))->e) > 16);
      DUMPBITS(t->b)
      NEEDBITS(e)
      d = w - t->v.n - ((unsigned)b & mask[e]);
      DUMPBITS(e)

      /* do the copy */
      do {
        n -= (e = (e = WSIZE - ((d &= WSIZE-1) > w ? d : w)) > n ? n : e);
#ifndef NOMEMCPY
        if (w - d >= e)         /* (this test assumes unsigned comparison) */
        {
          memcpy(slide + w, slide + d, e);
          w += e;
          d += e;
        }
        else                      /* do it slow to avoid memcpy() overlap */
#endif /* !NOMEMCPY */
          do {
            slide[w++] = slide[d++];
          } while (--e);
        if (w == WSIZE)
        {
          FLUSH(w);
          w = 0;
        }
      } while (n);
    }
  }


  /* restore the globals from the locals */
  wp = w;                       /* restore global window pointer */
  bb = b;                       /* restore global bit buffer */
  bk = k;


  /* done */
  return 0;
}

#endif /* ASM_INFLATECODES */



int inflate_stored()
/* "decompress" an inflated type 0 (stored) block. */
{
  unsigned n;           /* number of bytes in block */
  unsigned w;           /* current window position */
  register ulg b;       /* bit buffer */
  register unsigned k;  /* number of bits in bit buffer */


  /* make local copies of globals */
  Trace((stderr, "\nstored block"));
  b = bb;                       /* initialize bit buffer */
  k = bk;
  w = wp;                       /* initialize window position */


  /* go to byte boundary */
  n = k & 7;
  DUMPBITS(n);


  /* get the length and its complement */
  NEEDBITS(16)
  n = ((unsigned)b & 0xffff);
  DUMPBITS(16)
  NEEDBITS(16)
  if (n != (unsigned)((~b) & 0xffff))
    return 1;                   /* error in compressed data */
  DUMPBITS(16)


  /* read and output the compressed data */
  while (n--)
  {
    NEEDBITS(8)
    slide[w++] = (uch)b;
    if (w == WSIZE)
    {
      FLUSH(w);
      w = 0;
    }
    DUMPBITS(8)
  }


  /* restore the globals from the locals */
  wp = w;                       /* restore global window pointer */
  bb = b;                       /* restore global bit buffer */
  bk = k;
  return 0;
}


/* Globals for literal tables (built once) */
struct huft *fixed_tl = (struct huft *)NULL;
struct huft *fixed_td;
int fixed_bl, fixed_bd;

int inflate_fixed()
/* decompress an inflated type 1 (fixed Huffman codes) block.  We should
   either replace this with a custom decoder, or at least precompute the
   Huffman tables. */
{
  /* if first time, set up tables for fixed blocks */
  Trace((stderr, "\nliteral block"));
  if (fixed_tl == (struct huft *)NULL)
  {
    int i;                /* temporary variable */
    static unsigned l[288]; /* length list for huft_build */

    /* literal table */
    for (i = 0; i < 144; i++)
      l[i] = 8;
    for (; i < 256; i++)
      l[i] = 9;
    for (; i < 280; i++)
      l[i] = 7;
    for (; i < 288; i++)          /* make a complete, but wrong code set */
      l[i] = 8;
    fixed_bl = 7;
    if ((i = huft_build(l, 288, 257, cplens, cplext,
                        &fixed_tl, &fixed_bl)) != 0)
    {
      fixed_tl = (struct huft *)NULL;
      return i;
    }

    /* distance table */
    for (i = 0; i < 30; i++)      /* make an incomplete code set */
      l[i] = 5;
    fixed_bd = 5;
    if ((i = huft_build(l, 30, 0, cpdist, cpdext, &fixed_td, &fixed_bd)) > 1)
    {
      huft_free(fixed_tl);
      fixed_tl = (struct huft *)NULL;
      return i;
    }
  }


  /* decompress until an end-of-block code */
  return inflate_codes(fixed_tl, fixed_td, fixed_bl, fixed_bd) != 0;
}



int inflate_dynamic()
/* decompress an inflated type 2 (dynamic Huffman codes) block. */
{
  int i;                /* temporary variables */
  unsigned j;
  unsigned l;           /* last length */
  unsigned m;           /* mask for bit lengths table */
  unsigned n;           /* number of lengths to get */
  struct huft *tl;      /* literal/length code table */
  struct huft *td;      /* distance code table */
  int bl;               /* lookup bits for tl */
  int bd;               /* lookup bits for td */
  unsigned nb;          /* number of bit length codes */
  unsigned nl;          /* number of literal/length codes */
  unsigned nd;          /* number of distance codes */
#ifdef PKZIP_BUG_WORKAROUND
  static unsigned ll[288+32]; /* literal/length and distance code lengths */
#else
  static unsigned ll[286+30]; /* literal/length and distance code lengths */
#endif
  register ulg b;       /* bit buffer */
  register unsigned k;  /* number of bits in bit buffer */


  /* make local bit buffer */
  Trace((stderr, "\ndynamic block"));
  b = bb;
  k = bk;


  /* read in table lengths */
  NEEDBITS(5)
  nl = 257 + ((unsigned)b & 0x1f);      /* number of literal/length codes */
  DUMPBITS(5)
  NEEDBITS(5)
  nd = 1 + ((unsigned)b & 0x1f);        /* number of distance codes */
  DUMPBITS(5)
  NEEDBITS(4)
  nb = 4 + ((unsigned)b & 0xf);         /* number of bit length codes */
  DUMPBITS(4)
#ifdef PKZIP_BUG_WORKAROUND
  if (nl > 288 || nd > 32)
#else
  if (nl > 286 || nd > 30)
#endif
    return 1;                   /* bad lengths */


  /* read in bit-length-code lengths */
  for (j = 0; j < nb; j++)
  {
    NEEDBITS(3)
    ll[border[j]] = (unsigned)b & 7;
    DUMPBITS(3)
  }
  for (; j < 19; j++)
    ll[border[j]] = 0;


  /* build decoding table for trees--single level, 7 bit lookup */
  bl = 7;
  if ((i = huft_build(ll, 19, 19, NULL, NULL, &tl, &bl)) != 0)
  {
    if (i == 1)
      huft_free(tl);
    return i;                   /* incomplete code set */
  }


  /* read in literal and distance code lengths */
  n = nl + nd;
  m = mask[bl];
  i = l = 0;
  while ((unsigned)i < n)
  {
    NEEDBITS((unsigned)bl)
    j = (td = tl + ((unsigned)b & m))->b;
    DUMPBITS(j)
    j = td->v.n;
    if (j < 16)                 /* length of code in bits (0..15) */
      ll[i++] = l = j;          /* save last length in l */
    else if (j == 16)           /* repeat last length 3 to 6 times */
    {
      NEEDBITS(2)
      j = 3 + ((unsigned)b & 3);
      DUMPBITS(2)
      if ((unsigned)i + j > n)
        return 1;
      while (j--)
        ll[i++] = l;
    }
    else if (j == 17)           /* 3 to 10 zero length codes */
    {
      NEEDBITS(3)
      j = 3 + ((unsigned)b & 7);
      DUMPBITS(3)
      if ((unsigned)i + j > n)
        return 1;
      while (j--)
        ll[i++] = 0;
      l = 0;
    }
    else                        /* j == 18: 11 to 138 zero length codes */
    {
      NEEDBITS(7)
      j = 11 + ((unsigned)b & 0x7f);
      DUMPBITS(7)
      if ((unsigned)i + j > n)
        return 1;
      while (j--)
        ll[i++] = 0;
      l = 0;
    }
  }


  /* free decoding table for trees */
  huft_free(tl);


  /* restore the global bit buffer */
  bb = b;
  bk = k;


  /* build the decoding tables for literal/length and distance codes */
  bl = lbits;
  if ((i = huft_build(ll, nl, 257, cplens, cplext, &tl, &bl)) != 0)
  {
    if (i == 1 && !qflag) {
      FPRINTF(stderr, "(incomplete l-tree)  ");
      huft_free(tl);
    }
    return i;                   /* incomplete code set */
  }
  bd = dbits;
  if ((i = huft_build(ll + nl, nd, 0, cpdist, cpdext, &td, &bd)) != 0)
  {
    if (i == 1 && !qflag) {
      FPRINTF(stderr, "(incomplete d-tree)  ");
#ifdef PKZIP_BUG_WORKAROUND
      i = 0;
    }
#else
      huft_free(td);
    }
    huft_free(tl);
    return i;                   /* incomplete code set */
#endif
  }


  /* decompress until an end-of-block code */
  if (inflate_codes(tl, td, bl, bd))
    return 1;


  /* free the decoding tables, return */
  huft_free(tl);
  huft_free(td);
  return 0;
}



int inflate_block(e)
int *e;                 /* last block flag */
/* decompress an inflated block */
{
  unsigned t;           /* block type */
  register ulg b;       /* bit buffer */
  register unsigned k;  /* number of bits in bit buffer */


  /* make local bit buffer */
  b = bb;
  k = bk;


  /* read in last block bit */
  NEEDBITS(1)
  *e = (int)b & 1;
  DUMPBITS(1)


  /* read in block type */
  NEEDBITS(2)
  t = (unsigned)b & 3;
  DUMPBITS(2)


  /* restore the global bit buffer */
  bb = b;
  bk = k;


  /* inflate that block type */
  if (t == 2)
    return inflate_dynamic();
  if (t == 0)
    return inflate_stored();
  if (t == 1)
    return inflate_fixed();


  /* bad block type */
  return 2;
}



int inflate()
/* decompress an inflated entry */
{
  int e;                /* last block flag */
  int r;                /* result code */
  unsigned h;           /* maximum struct huft's malloc'ed */


  /* initialize window, bit buffer */
  wp = 0;
  bk = 0;
  bb = 0;


  /* decompress until the last block */
  h = 0;
  do {
    hufts = 0;
    if ((r = inflate_block(&e)) != 0)
      return r;
    if (hufts > h)
      h = hufts;
  } while (!e);


  /* flush out slide */
  FLUSH(wp);


  /* return success */
  Trace((stderr, "\n%u bytes in Huffman tables (%d/entry)\n",
         h * sizeof(struct huft), sizeof(struct huft)));
  return 0;
}

int inflate_free()
{
  if (fixed_tl != (struct huft *)NULL)
  {
    huft_free(fixed_td);
    huft_free(fixed_tl);
    fixed_td = fixed_tl = (struct huft *)NULL;
  }
  return 0;
}

/* End of inflate.c */

/*
 * zchunks.c
 *
 * Code for handling deflated chunks (IDAT and zTXt) is naturally
 * much larger than that for all the other chunks, so I move it all
 * here (as well as tEXt, which shares code with zTXt).
 *
 **********
 *
 * HISTORY
 *
 * 95-03-10 Created by Lee Daniel Crocker <lee@piclab.com>
 *          <URL:http://www.piclab.com/piclab/index.html>
 */
/*
 * Interlacing tables
 */
static int
    interlace_pattern[8][8] = {
        { 0, 5, 3, 5, 1, 5, 3, 5 },
        { 6, 6, 6, 6, 6, 6, 6, 6 },
        { 4, 5, 4, 5, 4, 5, 4, 5 },
        { 6, 6, 6, 6, 6, 6, 6, 6 },
        { 2, 5, 3, 5, 2, 5, 3, 5 },
        { 6, 6, 6, 6, 6, 6, 6, 6 },
        { 4, 5, 4, 5, 4, 5, 4, 5 },
        { 6, 6, 6, 6, 6, 6, 6, 6 } },
    starting_row[7] =   { 0, 0, 4, 0, 2, 0, 1 },
    starting_col[7] =   { 0, 4, 0, 2, 0, 1, 0 },
    row_increment[7] =  { 8, 8, 8, 4, 4, 2, 2 },
    col_increment[7] =  { 8, 8, 4, 4, 2, 2, 1 };

static int zlib_start(void);
static void zlib_end(void);
static void unfilter(int);
static void write_byte(void);

/*
 * Decode IDAT chunk. Most of the real work is done inside
 * the NEXTBYTE and FLUSH macros that interface with inflate.c.
 */

#define IS_ZTXT (PNG_CN_zTXt == ps.current_chunk_name)
#define IS_TEXT (PNG_CN_tEXt == ps.current_chunk_name)
#define IS_IDAT (PNG_CN_IDAT == ps.current_chunk_name)

int
decode_IDAT(
    void)
{
    int err, bpp, pass;
    /*
     * Palette chunk must appear before IDAT for palette-
     * based images.  This is technically a fatal error
     * in the PNG, but we will process the image anyway
     * as a grayscale so the user can see _something_.
     */
    if (ps.image->is_palette && (0 == ps.image->palette_size)) {
        warning(ptot_error_messages[WARN_NO_PLTE]);
        ps.image->is_palette = ps.image->is_color = FALSE;
    }
    ps.got_first_idat = TRUE;
    bpp = ps.image->bits_per_sample / 8;
    if (0 == bpp) bpp = 1;
    ps.byte_offset = ps.image->samples_per_pixel * bpp;
    /*
     * Allocate largest line needed for filtering
     */
    ps.line_size = new_line_size(ps.image, 0, 1);
    ps.this_line = (U8 *)polyray_malloc(ps.line_size);
    ps.last_line = (U8 *)polyray_malloc(ps.line_size);

    if (NULL == ps.this_line || NULL == ps.last_line) {
        err = ERR_MEMORY;
        goto di_err_out;
    }
    memset(ps.this_line, 0, ps.line_size);
    memset(ps.last_line, 0, ps.line_size);

    ps.current_row = ps.interlace_pass = ps.line_x = 0;
    ps.cur_filter = 255;

    ps.bytes_in_buf = 0L;   /* Required before calling NEXTBYTE */
    ps.bufp = ps.buf;

    if (0 != (err = zlib_start())) goto di_err_out;

    if (ps.image->is_interlaced) {
        ps.line_size = new_line_size(ps.image, 0, 8);
    } else {
        ps.line_size = new_line_size(ps.image, 0, 1);
    }
    if (0 != (err = inflate())) goto di_err_out;

di_err_out:
    if (NULL != ps.this_line) polyray_free(ps.this_line);
    if (NULL != ps.last_line) polyray_free(ps.last_line);

    zlib_end();
    return err;
}

/*
 * Assume that the next byte to read in the file begins the
 * compressed area of an IDAT or zTXt. Set up the necessary
 * structures for decompression.
 */

static int
zlib_start(
    void)
{
    ps.sum1 = 1;    /* Precondition Adler checksum */
    ps.sum2 = 0;
    ps.inflate_flags = (NEXTBYTE << 8);
    ps.inflate_flags |= NEXTBYTE;

    ps.inflate_window_size =
      1L << (((ps.inflate_flags >> 12) & 0x0F) + 8);

    if (ps.inflate_window_size > 32768) return ERR_COMP_HDR;

    if ( (0 != (ps.inflate_flags % 31)) ||
      (8 != ((ps.inflate_flags >> 8) & 0x0F)) ||
      (0 != (ps.inflate_flags & 0x0020)) ) return ERR_COMP_HDR;

    ps.inflate_window =
      (U8 *)polyray_malloc((size_t)(ps.inflate_window_size));
    if (NULL == ps.inflate_window) return ERR_MEMORY;

    ps.inflated_chunk_size = 0L;
    return 0;
}

/*
 * Clean up decompressor and verify checksum.
 */

static void
zlib_end(
    void)
{
    U16 sum1, sum2;

    if (NULL == ps.inflate_window) return;
    polyray_free(ps.inflate_window);

    sum2 = NEXTBYTE << 8;
    sum2 |= NEXTBYTE;
    sum1 = NEXTBYTE << 8;
    sum1 |= NEXTBYTE;

    if ((sum1 != ps.sum1) || (sum2 != ps.sum2))
      warning(ptot_error_messages[WARN_BAD_SUM]);

    inflate_free();
}

/*
 * Unfilter the image data byte passed in, and put it into the
 * ps.this_line[] array for write_pixel to find.
 */

static void
unfilter(
    int inbyte)
{
    int prediction, pA, pB, pC, dA, dB, dC;

    if (PNG_PF_None == ps.cur_filter) prediction = 0;
    else {
        pA = ((ps.line_x < ps.byte_offset) ? 0 :
          ps.this_line[ps.line_x - ps.byte_offset]);
        pB = ps.last_line[ps.line_x];
        pC = ((ps.line_x < ps.byte_offset) ? 0 :
          ps.last_line[ps.line_x - ps.byte_offset]);

        switch (ps.cur_filter) {
        case PNG_PF_Sub:
            prediction = pA;
            break;
        case PNG_PF_Up:
            prediction = pB;
            break;
        case PNG_PF_Average:
            prediction = ((pA + pB) / 2);
            break;
        case PNG_PF_Paeth:
            prediction = pA + pB - pC;
            dA = abs(prediction - pA);
            dB = abs(prediction - pB);
            dC = abs(prediction - pC);
            if (dA <= dB && dA <= dC) prediction = pA;
            else if (dB <= dC) prediction = pB;
            else prediction = pC;
            break;
        default:
            ;
        }
    }
    ps.this_line[ps.line_x] = 0xFF & (inbyte + prediction);
}


#define BPS (ps.image->bits_per_sample)
#undef BMAX
#define BMAX ((1<<BPS)-1)

/*
 * Calculate how many bytes of image data will appear
 * per line of the given image, accounting for the start
 * and increment of the current interlace pass.
 */
size_t
new_line_size(
    IMG_INFO *image,
    int start,
    int increment)
{
    U32 pixels;
    size_t size;

    pixels = (((image->width - start) - 1) / increment) + 1;

    if (BPS < 8) {
        size = ((BPS * (pixels - 1)) / 8) + 1;
    } else {
        size = pixels * image->samples_per_pixel * (BPS / 8);
    }
    return size;
}

static void
write_byte(void)
{
    unsigned char *image_ptr, bytes[4];
    U8 *temp, byte;
    U32 start, increment;
    int pixel, got_bits, i, byte_count;

    /*
     * Advance pointers and handle interlacing.
     */
    if (++ps.line_x >= ps.line_size) {
        /*
         * We've now received all the bytes for a single
         * scanline. Here we write them to the tempfile,
         * unpacking 1, 2, and 4-bit values into whole bytes.
         */
        image_ptr = output_image->image[ps.current_row];

        if (ps.image->is_interlaced) {
            start = starting_col[ps.interlace_pass];
            increment = col_increment[ps.interlace_pass];
        } else {
            start = 0;
            increment = 1;
        }
        temp = ps.this_line;
        got_bits = 0;

        for (ps.current_col = start;
             ps.current_col < ps.image->width;
             ps.current_col += increment) {

            if (BPS < 8) {
                if (got_bits == 0) {
                    byte = *temp++;
                    got_bits = 8;
                }
                pixel = (byte >> (8 - BPS)) & BMAX;
                if (output_image->ftype != 1)
                   pixel = (pixel * 255) / BMAX;
                byte <<= BPS;
                got_bits -= BPS;
                image_ptr[ps.current_col] = pixel;
            } else {
                if (ps.image->bits_per_sample == 16 &&
                    ps.image->samples_per_pixel == 1)
                   byte_count = 2;
                else
                   byte_count = ps.image->samples_per_pixel;
                for (i=0;i<byte_count;i++) {
                   bytes[i] = *temp++;
                   if (ps.image->bits_per_sample == 16 &&
                       ps.image->samples_per_pixel != 1)
                      byte = *temp++;
                   }
                i = byte_count * ps.current_col;
                if (ps.image->samples_per_pixel == 1) {
                   if (ps.image->bits_per_sample == 16) {
                      /* Handle 16 bit greyscale differently */
                      image_ptr[i] = bytes[0];
                      image_ptr[i+1] = bytes[1];
                      }
                   else
                      image_ptr[i] = bytes[0];
                   }
                else if (ps.image->samples_per_pixel == 2) {
                   /* What format is this that has two samples per pixel? */
                   image_ptr[i] = bytes[0];
                   image_ptr[i+1] = bytes[1];
                   }
                else {
                   image_ptr[i] = bytes[2];
                   image_ptr[i+1] = bytes[1];
                   image_ptr[i+2] = bytes[0];
                   if (ps.image->samples_per_pixel == 4)
                      image_ptr[i+3] = bytes[3];
                   }
                }
        }

        ps.cur_filter = 255;
        ps.line_x = 0;
        temp = ps.last_line;
        ps.last_line = ps.this_line;
        ps.this_line = temp;

        if (ps.image->is_interlaced) {
            ps.current_row +=
              row_increment[ps.interlace_pass];

            if (ps.current_row >= ps.image->height) {
                /*
                 * The return here happens after the
                 * last pixel has been read.
                 */
                if (++ps.interlace_pass > 6) {
                    --ps.interlace_pass;
                    return;
                }
                ps.current_row =
                  starting_row[ps.interlace_pass];
                ps.line_size = new_line_size(ps.image,
                  starting_col[ps.interlace_pass],
                  col_increment[ps.interlace_pass]);

                memset(ps.last_line, 0, ps.line_size);
            }
        } else {
            ++ps.current_row;
        }
    }
}

#undef BPS
#undef BMAX

/*
 * These next functions are required for interfacing with
 * Mark Adler's inflate.c.  fill_buf() is called by
 * NEXTBYTE when the I/O buffer is empty. It knows about
 * split IDATs and deals with them specially. These two
 * functions are used by zTXt as well.
 */

U8
fill_buf(
    void)
{
    int err;

    if (0 == ps.bytes_remaining) {
        /*
         * Current IDAT is exhausted. Continue on to the next
         * one. Only IDATs can be split this way.
         */
        if (IS_ZTXT) return ERR_BAD_PNG;
        if (0 != (err = verify_chunk_crc())) return err;

        if (0 != (err = get_chunk_header())) return err;
        if (!IS_IDAT) return ERR_EARLY_EOI;
    }
    ps.bufp = ps.buf;
    ps.bytes_in_buf = (S32)fread(ps.buf, 1,
      (size_t)min(IOBUF_SIZE, ps.bytes_remaining), ps.inf);

    ps.bytes_remaining -= ps.bytes_in_buf;
    if (0 == ps.bytes_in_buf) return ERR_READ;
    ps.crc = update_crc(ps.crc, ps.buf, ps.bytes_in_buf);

    --ps.bytes_in_buf;
    return *ps.bufp++;
}

/*
 * Flush uncompressed bytes from inflate window. This function
 * is used for both IDAT and zTXt chunks.
 */

void
flush_window(
    U32 size)
{
    U8 *wp, byte;
    U32 length, sum1, sum2;
    int loopcount;

    /*
     * Compute Adler checksum on uncompressed data, then write.
     * We can safely delay the mod operation for 5552 bytes
     * without overflowing our 32-bit accumulators.
     */
    wp = ps.inflate_window;
    length = size;
    sum1 = ps.sum1;
    sum2 = ps.sum2;

    while (length > 0) {
        loopcount = (length > 5552) ? 5552 : length;
        length -= loopcount;

        do {
            sum1 += *wp++;
            sum2 += sum1;
        } while (--loopcount);

        sum1 %= 65521;
        sum2 %= 65521;
    }
    ps.sum1 = (U16)sum1;
    ps.sum2 = (U16)sum2;
    /*
     * Write uncompressed bytes to output file.
     */
    ps.inflated_chunk_size += size;
    wp = ps.inflate_window;
    length = size;

    do {
        byte = *wp++;

        if (255 == ps.cur_filter) {
            ps.cur_filter = byte;

            if (ps.cur_filter > 4) {
                warning(ptot_error_messages[WARN_FILTER]);
                ps.cur_filter = 0;
            }
        } else {
            unfilter(byte);
            write_byte();
        }
    } while (--length);
}

#undef IS_ZTXT
#undef IS_TEXT
#undef IS_IDAT

/* End of Zchunks.c */
/* End of tempfile.c */

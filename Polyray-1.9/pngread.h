/*
 * ptot.h
 *
 * Header file for PNG to TIFF converter.
 *
 **********
 *
 * HISTORY
 *
 * 95-03-10 Created by Lee Daniel Crocker <lee@piclab.com>
 *          <URL:http://www.piclab.com/piclab/index.html>
 */

#ifdef _X86_            /* Intel i86 family */
#  define LITTLE_ENDIAN
#endif

#ifdef _SPARC_
#  define BIG_ENDIAN
#  ifndef FILENAME_MAX /* Work around stupid header file bugs */
#    define FILENAME_MAX 1024
#  endif
#  ifndef SEEK_SET
#    define SEEK_SET 0
#  endif
#endif

#ifndef min
#define min(x,y) (((x)<(y))?(x):(y))
#endif
#ifndef max
#define max(x,y) (((y)<(x))?(x):(y))
#endif

/*
 * Some types and macros for easier porting. Byte swapping is the
 * major issue because we have to convert big-endiang PNG to
 * native-endian TIFF on whatever architecture we're compiled on.
 * The code depends heavily on the endianness definition above.
 * Functions would be a lot simpler than macros here, but are less
 * likely to be optimized down to simple inline byte swaps. Note
 * that some of these macros evaluate the address twice, so don't
 * pass "*p++" to them!
 */

#ifndef MAC
typedef signed char     S8;
typedef unsigned char   U8;
typedef signed short    S16;
typedef unsigned short  U16;
typedef signed long     S32;
typedef unsigned long   U32;
#else
#define S8 signed char
#define U8 unsigned char
#define S16 signed short
#define U16 unsigned short
#define S32 signed long
#define U32 unsigned long
#endif

#ifndef TRUE
#  define TRUE 1
#  define FALSE 0
#endif

#define LOBYTE(w)   ((U8)((w)&0xFF))
#define HIBYTE(w)   ((U8)(((w)>>8)&0xFF))
#define LOWORD(d)   ((U16)((d)&0xFFFF))
#define HIWORD(d)   ((U16)(((d)>>16)&0xFFFF))

#define PUT16(p,w)  (*(U16*)(p)=(w))    /* Native byte order */
#define GET16(p)    (*(U16*)(p))
#define PUT32(p,d)  (*(U32*)(p)=(d))
#define GET32(p)    (*(U32*)(p))

#if !defined(BIG_ENDIAN) && !defined(LITTLE_ENDIAN)
#  error "No byte order defined"
#endif

#ifdef BIG_ENDIAN
#  define BE_GET16(p)     GET16(p)
#  define BE_PUT16(p,w)   PUT16((p),(w))
#  define BE_GET32(p)     GET32(p)
#  define BE_PUT32(p,d)   PUT32((p),(d))
#  define LE_GET16(p)     ((U16)(*(U8*)(p)&0xFF)|\
                            (*((U8*)(p)+1)<<8))
#  define LE_PUT16(p,w)   (((*(U8*)(p))=LOBYTE(w)),\
                            ((*((U8*)(p)+1))=HIBYTE(w)))
#  define LE_GET32(p)     (((U32)LE_GET16(p))|\
                            LE_GET16((U8*)(p)+2)<<16)
#  define LE_PUT32(p,d)   (LE_PUT16((p),LOWORD(d)),\
                            LE_PUT16((U8*)(p)+2,HIWORD(d)))
#else
#  define BE_GET16(p)     ((U16)(*(U8*)(p)<<8)|\
                            (*((U8*)(p)+1)&0xFF))
#  define BE_PUT16(p,w)   (((*(U8*)(p))=HIBYTE(w)),\
                            ((*((U8*)(p)+1))=LOBYTE(w)))
#  define BE_GET32(p)     (((U32)BE_GET16(p)<<16)|\
                            BE_GET16((U8*)(p)+2))
#  define BE_PUT32(p,d)   (BE_PUT16((p),HIWORD(d)),\
                            BE_PUT16((U8*)(p)+2,LOWORD(d)))
#  define LE_GET16(p)     GET16(p)
#  define LE_PUT16(p,w)   PUT16((p),(w))
#  define LE_GET32(p)     GET32(p)
#  define LE_PUT32(p,d)   PUT32((p),(d))
#endif

/*
 * Miscellaneous PNG definitions.
 */

#define PNG_Signature       "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"
#define PNG_MaxChunkLength  0x7FFFFFFFL

#define PNG_CN_IHDR 0x49484452L     /* Chunk names */
#define PNG_CN_PLTE 0x504C5445L
#define PNG_CN_IDAT 0x49444154L
#define PNG_CN_IEND 0x49454E44L
#define PNG_CN_gAMA 0x67414D41L
#define PNG_CN_sBIT 0x73424954L
#define PNG_CN_cHRM 0x6348524DL
#define PNG_CN_tRNS 0x74524E53L
#define PNG_CN_bKGD 0x624B4744L
#define PNG_CN_hIST 0x68495354L
#define PNG_CN_tEXt 0x74455874L
#define PNG_CN_zTXt 0x7A545874L
#define PNG_CN_pHYs 0x70485973L
#define PNG_CN_oFFs 0x6F464673L
#define PNG_CN_tIME 0x74494D45L
#define PNG_CN_sCAL 0x7343414CL

#define PNG_CF_Ancillary    0x20000000L /* Chunk flags */
#define PNG_CF_Private      0x00200000L
#define PNG_CF_CopySafe     0x00000020L

#define PNG_FT_Adaptive 0   /* Filtering type */
#define PNG_CT_Deflate  0   /* Compression type */
#define PNG_IT_None     0   /* Interlace types */
#define PNG_IT_Costello 1

#define PNG_CB_Palette  0x01    /* Colortype bits */
#define PNG_CB_Color    0x02
#define PNG_CB_Alpha    0x04

#define PNG_MU_None         0   /* Measurement units */
#define PNG_MU_Pixel        0
#define PNG_MU_Meter        1
#define PNG_MU_Micrometer   1
#define PNG_MU_Radian       2

#define PNG_PF_None     0   /* Prediction filters */
#define PNG_PF_Sub      1
#define PNG_PF_Up       2
#define PNG_PF_Average  3
#define PNG_PF_Paeth    4

/*
 * Structure for holding miscellaneous image information. The
 * conversion program will read an image into this structure, then
 * pass it to the output function. In this implementation, the
 * image data bytes are stored in a file, which is pointed to by
 * this structure. This is so the code will work on small-memory
 * architectures like MS-DOS. On Unix, Win32 (NT/Chicago), and other
 * systems, it might make more sense to simply allocate one big
 * chunk of memory for the image and replace the image_data_file
 * string with an image_data_buffer pointer.
 */

#define N_KEYWORDS 5

typedef struct _PNG_image_info {
    U32 width, height;
    U32 xoffset, yoffset;
    U32 xres, yres;
    double xscale, yscale;
    double source_gamma;
    U32 chromaticities[8];      /* Fixed point x 100000 */
    int resolution_unit;        /* Units as in PNG */
    int offset_unit, scale_unit;
    int samples_per_pixel;
    int bits_per_sample;
    int significant_bits[4];
    int background_color[4];
    int is_color, has_alpha, has_trns;
    int is_interlaced, is_palette;
    int palette_size;
    U8 palette[3 * 256];
    U16 trans_values[3];
    U8 palette_trans_bytes[256];
    char *keywords[N_KEYWORDS];
    char *pixel_data_file;      /* Where to find the pixels */
    U32 png_data_size;
    char *png_data_file;    /* Untranslatable PNG chunks */
} IMG_INFO;

#define IMG_SIZE (sizeof (struct _PNG_image_info))

extern char *keyword_table[N_KEYWORDS];
extern U16 ASCII_tags[N_KEYWORDS];

/*
 * Local ASSERT macro.  Assumes the function Assert() is
 * defined somewhere in the calling program (in this case,
 * it's in ptot.c).
 */

#ifndef NDEBUG
#  define ASSERT(x) ((x)?(void)0:Assert(__FILE__,__LINE__))
#  define TRACE_STR(x) (fprintf(stderr,"TR: %s\n",(x)),\
                         fflush(stderr))
#  define TRACE_INT(x) (fprintf(stderr,"TR: %ld\n",(long)(x)),\
                         fflush(stderr))
#else
#  define ASSERT(x)
#  define TRACE_STR(x)
#  define TRACE_INT(x)
#endif

/*
 * Prototypes
 */

U32 update_crc(U32, U8 *, U32);

int read_PNG(FILE *, IMG_INFO *);
int get_chunk_header(void);
U32 get_chunk_data(U32);
int verify_chunk_crc(void);

int decode_IDAT(void);
U8 fill_buf(void);
void flush_window(U32);
int decode_text(void);
int copy_unknown_chunk_data(void);
size_t new_line_size(IMG_INFO *, int, int);

int get_local_byte_order(void);
int write_TIFF(FILE *, IMG_INFO *);

int create_tempfile(int); 
int open_tempfile(int);
void close_all_tempfiles(void);
void remove_all_tempfiles(void);

/*
 * Interface to Mark Adler's inflate.c
 */

int inflate(void);

typedef unsigned char uch;
typedef unsigned short ush;
typedef unsigned long ulg;
typedef void *voidp;

#define slide (ps.inflate_window)
#define WSIZE ((size_t)(ps.inflate_window_size))
#define NEXTBYTE ((--ps.bytes_in_buf>=0)?(*ps.bufp++):fill_buf())
#define FLUSH(n) flush_window(n)
#define memzero(a,s) memset((a),0,(s))
#define qflag 1

/*
 * A state structure is used to store all needed info about
 * the reading process, so that we don't have to pass 4 or 5
 * arguments to every function in ptot.c.  This is also used
 * to share data with inflate.c.
 */

#define IOBUF_SIZE 8192 /* Must be at least 768 for PLTE */

typedef struct _png_state {
    FILE *inf, *tf[7];
    char *tfnames[7];
    IMG_INFO *image;
    U8 *buf, *bufp;
    U32 crc, bytes_remaining;
    U32 inflated_chunk_size;
    U32 current_chunk_name;
    S32 bytes_in_buf;       /* Must be signed! */
    U32 inflate_window_size;
    U8 *inflate_window;
    U16 inflate_flags;
    U16 sum1, sum2;
    U8 *last_line, *this_line;
    size_t byte_offset;
    size_t line_size, line_x;
    int interlace_pass;
    U32 current_row, current_col;
    int cur_filter;
    int got_first_chunk;
    int got_first_idat;
} PNG_STATE;

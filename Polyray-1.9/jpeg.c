#include "defs.h"
#include "memory.h"
#include "io.h"
#include "jpeg.h"

/* The final image after we are done processing the JPEG image */
static Img *output_image = NULL;
static unsigned current_row = 0;

/* Jump out point when things go wrong */
jmp_buf JPEG_environ;

#ifndef SEEK_SET
#define SEEK_SET        0       /* seek from start of file      */
#define SEEK_CUR        1       /* relative to current position */
#define SEEK_END        2       /* relative to end of file      */
#endif

/* Decoder capability options: */
#define BITS_IN_JSAMPLE   8
#define MAXJSAMPLE      255
#define CENTERJSAMPLE   128
#define NEED_COMPLEX_CONTROLLER

#define MEMZERO(target,size)    memset((void *)(target), 0, (size_t)(size))
#define MEMCOPY(dest,src,size)  memcpy((void *)(dest), (const void *)(src), (size_t)(size))

/* Known size definitions of variables */
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef short INT16;
typedef long INT32;
#if !defined( FALSE )
#define FALSE   0
#endif
#if !defined( TRUE )
#define TRUE    1
#endif
typedef short boolean;

typedef unsigned char JSAMPLE;
#define GETJSAMPLE(value)  (value)

/* Types for JPEG compression parameters and working tables. */
typedef enum {                  /* defines known color spaces */
        CS_UNKNOWN,             /* error/unspecified */
        CS_GRAYSCALE,           /* monochrome (only 1 component) */
        CS_RGB,                 /* red/green/blue */
        CS_YCbCr,               /* Y/Cb/Cr (also known as YUV) */
        CS_YIQ,                 /* Y/I/Q */
        CS_CMYK                 /* C/M/Y/K */
} COLOR_SPACE;

typedef struct {                /* Basic info about one component */
  /* These values are fixed over the whole image */
  /* For compression, they must be supplied by the user interface; */
  /* for decompression, they are read from the SOF marker. */
        short component_id;     /* identifier for this component (0..255) */
        short component_index;  /* its index in SOF or comp_info[] */
        short h_samp_factor;    /* horizontal sampling factor (1..4) */
        short v_samp_factor;    /* vertical sampling factor (1..4) */
        short quant_tbl_no;     /* quantization table selector (0..3) */
  /* These values may vary between scans */
  /* For compression, they must be supplied by the user interface; */
  /* for decompression, they are read from the SOS marker. */
        short dc_tbl_no;        /* DC entropy table selector (0..3) */
        short ac_tbl_no;        /* AC entropy table selector (0..3) */
  /* These values are computed during compression or decompression startup */
        long true_comp_width;   /* component's image width in samples */
        long true_comp_height;  /* component's image height in samples */
        /* the above are the logical dimensions of the downsampled image */
  /* These values are computed before starting a scan of the component */
        short MCU_width;        /* number of blocks per MCU, horizontally */
        short MCU_height;       /* number of blocks per MCU, vertically */
        short MCU_blocks;       /* MCU_width * MCU_height */
        long downsampled_width; /* image width in samples, after expansion */
        long downsampled_height; /* image height in samples, after expansion */
        /* the above are the true_comp_xxx values rounded up to multiples of */
        /* the MCU dimensions; these are the working dimensions of the array */
        /* as it is passed through the DCT or IDCT step.  NOTE: these values */
        /* differ depending on whether the component is interleaved or not!! */
  /* This flag is used only for decompression.  In cases where some of the */
  /* components will be ignored (eg grayscale output from YCbCr image), */
  /* we can skip IDCT etc. computations for the unused components. */
        boolean component_needed; /* do we need the value of this component? */
} jpeg_component_info;

typedef JSAMPLE *JSAMPROW;      /* ptr to one image row of pixel samples. */
typedef JSAMPROW *JSAMPARRAY;   /* ptr to some rows (a 2-D sample array) */
typedef JSAMPARRAY *JSAMPIMAGE; /* a 3-D sample array: top index is color */

/* Callback for moving resulting information into our image buffer */
static void put_pixel_rows(int num_rows, JSAMPIMAGE pixel_data);

#define DCTSIZE         8       /* The basic DCT block is 8x8 samples */
#define DCTSIZE2        64      /* DCTSIZE squared; # of elements in a block */

typedef INT16 QUANT_VAL;                /* element of a quantization table */
typedef QUANT_VAL QUANT_TBL[DCTSIZE2];  /* A quantization table */
typedef QUANT_VAL *QUANT_TBL_PTR;       /* pointer to same */

typedef short JCOEF;
typedef JCOEF JBLOCK[DCTSIZE2]; /* one block of coefficients */
typedef JBLOCK *JBLOCKROW;      /* pointer to one row of coefficient blocks */
typedef JBLOCKROW *JBLOCKARRAY;         /* a 2-D array of coefficient blocks */
typedef JBLOCKARRAY *JBLOCKIMAGE;       /* a 3-D array of coefficient blocks */
typedef JCOEF *JCOEFPTR;        /* useful in a couple of places */

typedef JCOEF DCTELEM;
typedef DCTELEM DCTBLOCK[DCTSIZE2];

/* Huffman coding tables.  */
#define HUFF_LOOKAHEAD  8       /* # of bits of lookahead */

typedef struct {
  /* These two fields directly represent the contents of a JPEG DHT marker */
        UINT8 bits[17];         /* bits[k] = # of symbols with codes of */
                                /* length k bits; bits[0] is unused */
        UINT8 huffval[256];     /* The symbols, in order of incr code length */
  /* This field is used only during compression.  It's initialized FALSE when
   * the table is created, and set TRUE when it's been output to the file.
   */
        boolean sent_table;     /* TRUE when table has been output */
  /* The remaining fields are computed from the above to allow more efficient
   * coding and decoding.  These fields should be considered private to the
   * Huffman compression & decompression modules.  We use a union since only
   * one set of fields is needed at a time.
   */
        union {
          struct {              /* encoding tables: */
            UINT16 ehufco[256]; /* code for each symbol */
            char ehufsi[256];   /* length of code for each symbol */
          } enc;
          struct {              /* decoding tables: */
            /* Basic tables: (element [0] of each array is unused) */
            INT32 mincode[17];  /* smallest code of length k */
            INT32 maxcode[18];  /* largest code of length k (-1 if none) */
            /* (maxcode[17] is a sentinel to ensure huff_DECODE terminates) */
            int valptr[17];     /* huffval[] index of 1st symbol of length k */
            /* Lookahead tables: indexed by the next HUFF_LOOKAHEAD bits of
             * the input data stream.  If the next Huffman code is no more
             * than HUFF_LOOKAHEAD bits long, we can obtain its length and
             * the corresponding symbol directly from these tables.
             */
            int look_nbits[1<<HUFF_LOOKAHEAD]; /* # bits, or 0 if too long */
            UINT8 look_sym[1<<HUFF_LOOKAHEAD]; /* symbol, or unused */
          } dec;
        } priv;
} HUFF_TBL;

#define NUM_QUANT_TBLS      4   /* quantization tables are numbered 0..3 */
#define NUM_HUFF_TBLS       4   /* Huffman tables are numbered 0..3 */
#define NUM_ARITH_TBLS      16  /* arith-coding tables are numbered 0..15 */
#define MAX_COMPS_IN_SCAN   4   /* JPEG limit on # of components in one scan */
#define MAX_SAMP_FACTOR     4   /* JPEG limit on sampling factors */
#define MAX_BLOCKS_IN_MCU   10  /* JPEG limit on # of blocks in an MCU */

#define JPEG_BUF_SIZE   4096 /* bytes */
#define JGETC() (--bytes_in_buffer<0?read_jpeg_data():(UINT8)(*next_input_byte++))
#define JUNGETC(ch) (bytes_in_buffer++, *(--next_input_byte) = (UINT8)(ch))

#define MIN_UNGET       4       /* may always do at least 4 JUNGETCs */
#define JFREAD(file,buf,sizeofbuf)  \
  ((size_t) fread((void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))

static char *No_Memory = "Out of memory";
static char *Bad_JPEG = "Bad JPEG file";
static char *Corrupt_JPEG = "Corrupt JPEG file";

static int bytes_in_buffer = 0;
static char *next_input_byte = NULL;
static char input_buffer[JPEG_BUF_SIZE + MIN_UNGET + 1];
static char *input_buffer_ptr = NULL;

/* Working data for decompression */
static FILE *input_file = NULL;
static COLOR_SPACE out_color_space;    /* colorspace of output */

static long image_width;        /* overall image width */
static long image_height;       /* overall image height */

static short data_precision;    /* bits of precision in image data */

static COLOR_SPACE jpeg_color_space; /* colorspace of JPEG file */

static UINT8 density_unit;      /* JFIF code for pixel size units */
static UINT16 X_density;        /* Horizontal pixel density */
static UINT16 Y_density;        /* Vertical pixel density */

static short num_components;    /* # of color components in JPEG image */
static jpeg_component_info *comp_info;

static QUANT_TBL_PTR quant_tbl_ptrs[NUM_QUANT_TBLS];

static HUFF_TBL * dc_huff_tbl_ptrs[NUM_HUFF_TBLS];
static HUFF_TBL * ac_huff_tbl_ptrs[NUM_HUFF_TBLS];

static UINT8 arith_dc_L[NUM_ARITH_TBLS]; /* L vals for DC arith-coding tbls */
static UINT8 arith_dc_U[NUM_ARITH_TBLS]; /* U vals for DC arith-coding tbls */
static UINT8 arith_ac_K[NUM_ARITH_TBLS]; /* Kx vals for AC arith-coding tbls */

static boolean arith_code;      /* TRUE=arithmetic coding, FALSE=Huffman */

static UINT16 restart_interval;/* MCUs per restart interval, or 0 for none */

static short max_h_samp_factor; /* largest h_samp_factor */
static short max_v_samp_factor; /* largest v_samp_factor */
static short color_out_comps;   /* # of color components output */

/* These fields are valid during any one scan */
static short comps_in_scan;     /* # of JPEG components input this time */
static jpeg_component_info *cur_comp_info[MAX_COMPS_IN_SCAN];

static long MCUs_per_row;       /* # of MCUs across the image */
static long MCU_rows_in_scan;   /* # of MCU rows in the image */

static short blocks_in_MCU;     /* # of DCT blocks per MCU */
static short MCU_membership[MAX_BLOCKS_IN_MCU];

static JCOEF last_dc_val[MAX_COMPS_IN_SCAN]; /* last DC coef for each comp */
static UINT16 restarts_to_go;   /* MCUs left in this restart interval */
static short next_restart_num;  /* # of next RSTn marker (0..7) */

typedef void (*d_pipeline_controller_ptr)(void);
static d_pipeline_controller_ptr d_pipeline_controller;

typedef void (*dMCU_ptr)(JBLOCKIMAGE);
dMCU_ptr disassemble_MCU;
typedef void (*colorout_init_ptr)(void);
typedef void (*color_convert_ptr)(int, long, JSAMPIMAGE, JSAMPIMAGE);
typedef void (*colorout_term_ptr)(void);
typedef void (*downsample_ptr)(int, long, int, long, int, JSAMPARRAY,
                               JSAMPARRAY, JSAMPARRAY , JSAMPARRAY);
typedef void (*upsample_ptr)(int, long, int, long, int, JSAMPARRAY,
                                    JSAMPARRAY, JSAMPARRAY, JSAMPARRAY);
typedef void (*quantize_method_ptr)(int, JSAMPIMAGE, JSAMPARRAY);
typedef void (*quantize_caller_ptr)(quantize_method_ptr);
static upsample_ptr upsample[MAX_COMPS_IN_SCAN];
static colorout_init_ptr colorout_init;
static color_convert_ptr color_convert;
static colorout_term_ptr colorout_term;

#define RST0    0xD0            /* RST0 marker code */

typedef enum {                  /* JPEG marker codes */
  M_SOF0  = 0xc0,
  M_SOF1  = 0xc1,
  M_SOF2  = 0xc2,
  M_SOF3  = 0xc3,
  
  M_SOF5  = 0xc5,
  M_SOF6  = 0xc6,
  M_SOF7  = 0xc7,
  
  M_JPG   = 0xc8,
  M_SOF9  = 0xc9,
  M_SOF10 = 0xca,
  M_SOF11 = 0xcb,
  
  M_SOF13 = 0xcd,
  M_SOF14 = 0xce,
  M_SOF15 = 0xcf,
  
  M_DHT   = 0xc4,
  
  M_DAC   = 0xcc,
  
  M_RST0  = 0xd0,
  M_RST1  = 0xd1,
  M_RST2  = 0xd2,
  M_RST3  = 0xd3,
  M_RST4  = 0xd4,
  M_RST5  = 0xd5,
  M_RST6  = 0xd6,
  M_RST7  = 0xd7,
  
  M_SOI   = 0xd8,
  M_EOI   = 0xd9,
  M_SOS   = 0xda,
  M_DQT   = 0xdb,
  M_DNL   = 0xdc,
  M_DRI   = 0xdd,
  M_DHP   = 0xde,
  M_EXP   = 0xdf,
  
  M_APP0  = 0xe0,
  M_APP15 = 0xef,
  
  M_JPG0  = 0xf0,
  M_JPG13 = 0xfd,
  M_COM   = 0xfe,
  
  M_TEM   = 0x01,
  
  M_ERROR = 0x100
} JPEG_MARKER;

/*
 * About the data structures:
 *
 * The processing chunk size for upsampling is referred to in this file as
 * a "row group": a row group is defined as Vk (v_samp_factor) sample rows of
 * any component while downsampled, or Vmax (max_v_samp_factor) unsubsampled
 * rows.  In an interleaved scan each MCU row contains exactly DCTSIZE row
 * groups of each component in the scan.  In a noninterleaved scan an MCU row
 * is one row of blocks, which might not be an integral number of row groups;
 * therefore, we read in Vk MCU rows to obtain the same amount of data as we'd
 * have in an interleaved scan.
 * To provide context for the upsampling step, we have to retain the last
 * two row groups of the previous MCU row while reading in the next MCU row
 * (or set of Vk MCU rows).  To do this without copying data about, we create
 * a rather strange data structure.  Exactly DCTSIZE+2 row groups of samples
 * are allocated, but we create two different sets of pointers to this array.
 * The second set swaps the last two pairs of row groups.  By working
 * alternately with the two sets of pointers, we can access the data in the
 * desired order.
 *
 * Cross-block smoothing also needs context above and below the "current" row.
 * Since this is an optional feature, I've implemented it in a way that is
 * much simpler but requires more than the minimum amount of memory.  We
 * simply allocate three extra MCU rows worth of coefficient blocks and use
 * them to "read ahead" one MCU row in the file.  For a typical 1000-pixel-wide
 * image with 2x2,1x1,1x1 sampling, each MCU row is about 50Kb; an 80x86
 * machine may be unable to apply cross-block smoothing to wider images.
 */

/* Static variables to avoid passing 'round extra parameters */

static INT32 get_buffer;        /* current bit-extraction buffer */
static int bits_left;           /* # of unused bits in it */
static boolean printed_eod;     /* flag to suppress multiple end-of-data msgs */

static void
fix_huff_tbl (HUFF_TBL * htbl)
/* Compute derived values for a Huffman table */
{
  int p, i, l, si;
  int lookbits, ctr;
  char huffsize[257];
  UINT16 huffcode[257];
  UINT16 code;
  
  /* Figure C.1: make table of Huffman code length for each symbol */
  /* Note that this is in code-length order. */

  p = 0;
  for (l = 1; l <= 16; l++) {
    for (i = 1; i <= (int) htbl->bits[l]; i++)
      huffsize[p++] = (char) l;
  }
  huffsize[p] = 0;
  
  /* Figure C.2: generate the codes themselves */
  /* Note that this is in code-length order. */
  
  code = 0;
  si = huffsize[0];
  p = 0;
  while (huffsize[p]) {
    while (((int) huffsize[p]) == si) {
      huffcode[p++] = code;
      code++;
    }
    code <<= 1;
    si++;
  }

  /* Figure F.15: generate decoding tables for bit-sequential decoding */

  p = 0;
  for (l = 1; l <= 16; l++) {
    if (htbl->bits[l]) {
      htbl->priv.dec.valptr[l] = p; /* huffval[] index of 1st symbol of code length l */
      htbl->priv.dec.mincode[l] = huffcode[p]; /* minimum code of length l */
      p += htbl->bits[l];
      htbl->priv.dec.maxcode[l] = huffcode[p-1]; /* maximum code of length l */
    } else {
      htbl->priv.dec.maxcode[l] = -1; /* -1 if no codes of this length */
    }
  }
  htbl->priv.dec.maxcode[17] = 0xFFFFFL; /* ensures huff_DECODE terminates */

  /* Compute lookahead tables to speed up decoding.
   * First we set all the table entries to 0, indicating "too long";
   * then we iterate through the Huffman codes that are short enough and
   * fill in all the entries that correspond to bit sequences starting
   * with that code.
   */

  MEMZERO(htbl->priv.dec.look_nbits, sizeof(htbl->priv.dec.look_nbits));

  p = 0;
  for (l = 1; l <= HUFF_LOOKAHEAD; l++) {
    for (i = 1; i <= (int) htbl->bits[l]; i++, p++) {
      /* l = current code's length, p = its index in huffcode[] & huffval[]. */
      /* Generate left-justified code followed by all possible bit sequences */
      lookbits = huffcode[p] << (HUFF_LOOKAHEAD-l);
      for (ctr = 1 << (HUFF_LOOKAHEAD-l); ctr > 0; ctr--) {
        htbl->priv.dec.look_nbits[lookbits] = l;
        htbl->priv.dec.look_sym[lookbits] = htbl->huffval[p];
        lookbits++;
      }
    }
  }
}


/*
 * Code for extracting the next N bits from the input stream.
 * (N never exceeds 15 for JPEG data.)
 * This needs to go as fast as possible!
 *
 * We read source bytes into get_buffer and dole out bits as needed.
 * If get_buffer already contains enough bits, they are fetched in-line
 * by the macros check_bit_buffer and get_bits.  When there aren't enough
 * bits, fill_bit_buffer is called; it will attempt to fill get_buffer to
 * the "high water mark" (not just to the number of bits needed; this reduces
 * the function-call overhead cost of entering fill_bit_buffer).
 * On return, fill_bit_buffer guarantees that get_buffer contains at least
 * the requested number of bits --- dummy zeroes are inserted if necessary.
 *
 * On most machines MIN_GET_BITS should be 25 to allow the full 32-bit width
 * of get_buffer to be used.  (On machines with wider words, an even larger
 * buffer could be used.)  However, on some machines 32-bit shifts are
 * relatively slow and take time proportional to the number of places shifted.
 * (This is true with most PC compilers, for instance.)  In this case it may
 * be a win to set MIN_GET_BITS to the minimum value of 15.  This reduces the
 * average shift distance at the cost of more calls to fill_bit_buffer.
 */

#ifdef SLOW_SHIFT_32
#define MIN_GET_BITS  15        /* minimum allowable value */
#else
#define MIN_GET_BITS  25        /* max value for 32-bit get_buffer */
#endif


static void
JPEG_error(char *error_string)
{
   if (error_string != No_Memory)
      longjmp(JPEG_environ, 1);
   else
      error(error_string);
}

/* Reload the input buffer after it's been emptied, and return the next byte. */
static UINT8
read_jpeg_data(void)
{
   next_input_byte = input_buffer_ptr + MIN_UNGET;

   /* Don't know why the -1 is necessary at the end of the file read.  For
      some reason it makes it work correctly. */
   bytes_in_buffer = (int)JFREAD(input_file, next_input_byte, JPEG_BUF_SIZE)-1;
   if (bytes_in_buffer <= 0) {
      JPEG_error("Premature EOF in JPEG file");
      next_input_byte[0] = (char) 0xFF;
      next_input_byte[1] = (char) M_EOI;
      bytes_in_buffer = 2;
      return JGETC();
      }
   else
      return *next_input_byte++;
}

static void
fill_bit_buffer (int nbits)
/* Load up the bit buffer to a depth of at least nbits */
{
  /* Attempt to load at least MIN_GET_BITS bits into get_buffer. */
  /* (It is assumed that no request will be for more than that many bits.) */
  while (bits_left < MIN_GET_BITS) {
    UINT8 c = JGETC();
    
    /* If it's 0xFF, check and discard stuffed zero byte */
    if (c == 0xFF) {
      UINT8 c2 = JGETC();
      if (c2 != 0) {
        /* Oops, it's actually a marker indicating end of compressed data. */
        /* Better put it back for use later */
        JUNGETC(c2);
        JUNGETC(c);
        /* There should be enough bits still left in the data segment; */
        /* if so, just break out of the while loop. */
        if (bits_left >= nbits)
          break;
        /* Uh-oh.  Report corrupted data to user and stuff zeroes into
         * the data stream, so that we can produce some kind of image.
         * Note that this will be repeated for each byte demanded for the
         * rest of the segment; this is a bit slow but not unreasonably so.
         * The main thing is to avoid getting a zillion warnings, hence
         * we use a flag to ensure that only one warning appears.
         */
        if (! printed_eod) {
          warning(Corrupt_JPEG);
          printed_eod = TRUE;
        }
        c = 0;                  /* insert a zero byte into bit buffer */
      }
    }

    /* OK, load c into get_buffer */
    get_buffer = (get_buffer << 8) | c;
    bits_left += 8;
  }
}


/*
 * These macros provide the in-line portion of bit fetching.
 * Correct usage is:
 *      check_bit_buffer(n);            ensure there are N bits in get_buffer
 *      val = get_bits(n);              fetch N bits
 * The value n should be a simple variable, not an expression, because it
 * is evaluated multiple times.
 * peek_bits() fetches next N bits without removing them from the buffer.
 */

#define check_bit_buffer(nbits) \
        { if (bits_left < (nbits))  fill_bit_buffer(nbits); }

#define get_bits(nbits) \
        (((int) (get_buffer >> (bits_left -= (nbits)))) & ((1<<(nbits))-1))

#define peek_bits(nbits) \
        (((int) (get_buffer >> (bits_left -  (nbits)))) & ((1<<(nbits))-1))


/*
 * Routines to extract next Huffman-coded symbol from input bit stream.
 * We use a lookahead table to process codes of up to HUFF_LOOKAHEAD bits
 * without looping.  Usually, more than 95% of the Huffman codes will be 8
 * or fewer bits long.  The few overlength codes are handled with a loop.
 * The primary case is made a macro for speed reasons; the secondary
 * routine slow_DECODE is rarely entered and need not be inline code.
 *
 * Notes about the huff_DECODE macro:
 * 1. The first if-test is coded to call fill_bit_buffer only when necessary.
 * 2. If the lookahead succeeds, we need only decrement bits_left to remove
 *    the proper number of bits from get_buffer.
 * 3. If the lookahead table contains no entry, the next code must be
 *    more than HUFF_LOOKAHEAD bits long.
 * 4. Near the end of the data segment, we may fail to get enough bits
 *    for a lookahead.  In that case, we do it the hard way.
 */

#define huff_DECODE(htbl,result) \
{ int nb, look;                                 \
  if (bits_left >= HUFF_LOOKAHEAD ||                            \
      (fill_bit_buffer(0), bits_left >= HUFF_LOOKAHEAD)) {      \
    look = peek_bits(HUFF_LOOKAHEAD);                           \
    if ((nb = htbl->priv.dec.look_nbits[look]) != 0) {          \
      bits_left -= nb;                                          \
      result = htbl->priv.dec.look_sym[look];                   \
    } else                                                      \
      result = slow_DECODE(htbl, HUFF_LOOKAHEAD+1);             \
  } else                                                        \
    result = slow_DECODE(htbl, 1);                              \
}

  
static int
slow_DECODE (HUFF_TBL * htbl, int min_bits)
{
  int l = min_bits;
  INT32 code;

  /* huff_DECODE has determined that the code is at least min_bits */
  /* bits long, so fetch that many bits in one swoop. */

  check_bit_buffer(l);
  code = get_bits(l);

  /* Collect the rest of the Huffman code one bit at a time. */
  /* This is per Figure F.16 in the JPEG spec. */

  while (code > htbl->priv.dec.maxcode[l]) {
    code <<= 1;
    check_bit_buffer(1);
    code |= get_bits(1);
    l++;
  }

  /* With garbage input we may reach the sentinel value l = 17. */

  if (l > 16) {
    warning(Corrupt_JPEG);
    return 0; /* fake a zero as the safest result */
  }

  return htbl->huffval[ htbl->priv.dec.valptr[l] +
                        ((int) (code - htbl->priv.dec.mincode[l])) ];
}


/* Figure F.12: extend sign bit.
 * On some machines, a shift and add will be faster than a table lookup.
 */

#ifdef AVOID_TABLES

#define huff_EXTEND(x,s)  ((x) < (1<<((s)-1)) ? (x) + (((-1)<<(s)) + 1) : (x))

#else

#define huff_EXTEND(x,s)  ((x) < extend_test[s] ? (x) + extend_offset[s] : (x))

static const int extend_test[16] =   /* entry n is 2**(n-1) */
  { 0, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000 };

static const int extend_offset[16] = /* entry n is (-1 << n) + 1 */
  { 0, ((-1)<<1) + 1, ((-1)<<2) + 1, ((-1)<<3) + 1, ((-1)<<4) + 1,
    ((-1)<<5) + 1, ((-1)<<6) + 1, ((-1)<<7) + 1, ((-1)<<8) + 1,
    ((-1)<<9) + 1, ((-1)<<10) + 1, ((-1)<<11) + 1, ((-1)<<12) + 1,
    ((-1)<<13) + 1, ((-1)<<14) + 1, ((-1)<<15) + 1 };

#endif /* AVOID_TABLES */

/*
 * Initialize for a Huffman-compressed scan.
 * This is invoked after reading the SOS marker.
 */
static void
decoder_init()
{
  short ci;
  jpeg_component_info * compptr;

  /* Initialize static variables */
  bits_left = 0;
  printed_eod = FALSE;

  for (ci = 0; ci < comps_in_scan; ci++) {
    compptr = cur_comp_info[ci];
    /* Make sure requested tables are present */
    if (dc_huff_tbl_ptrs[compptr->dc_tbl_no] == NULL ||
        ac_huff_tbl_ptrs[compptr->ac_tbl_no] == NULL)
      JPEG_error(Bad_JPEG);
    /* Compute derived values for Huffman tables */
    /* We may do this more than once for same table, but it's not a big deal */
    fix_huff_tbl(dc_huff_tbl_ptrs[compptr->dc_tbl_no]);
    fix_huff_tbl(ac_huff_tbl_ptrs[compptr->ac_tbl_no]);
    /* Initialize DC predictions to 0 */
    last_dc_val[ci] = 0;
  }

  /* Initialize restart stuff */
  restarts_to_go = restart_interval;
  next_restart_num = 0;
}

static int
next_marker(void)
/* Find the next JPEG marker */
/* Note that the output might not be a valid marker code, */
/* but it will never be 0 or FF */
{
  UINT8 c;
  int nbytes;

  nbytes = 0;
  do {
    do {                        /* skip any non-FF bytes */
      nbytes++;
      c = JGETC();
    } while (c != 0xFF);
    do {                        /* skip any duplicate FFs */
      /* we don't increment nbytes here since extra FFs are legal */
      c = JGETC();
    } while (c == 0xFF);
  } while (c == 0);             /* repeat if it was a stuffed FF/00 */

  if (nbytes != 1)
    warning(Corrupt_JPEG);

  return c;
}

static void
resync_to_restart(int marker)
{
  int desired = next_restart_num;
  int action = 1;

  /* Always put up a warning. */
  warning(Corrupt_JPEG);

  /* Outer loop handles repeated decision after scanning forward. */
  for (;;) {
    if (marker < (int) M_SOF0)
      action = 2;               /* invalid marker */
    else if (marker < (int) M_RST0 || marker > (int) M_RST7)
      action = 3;               /* valid non-restart marker */
    else {
      if (marker == ((int) M_RST0 + ((desired+1) & 7)) ||
          marker == ((int) M_RST0 + ((desired+2) & 7)))
        action = 3;             /* one of the next two expected restarts */
      else if (marker == ((int) M_RST0 + ((desired-1) & 7)) ||
               marker == ((int) M_RST0 + ((desired-2) & 7)))
        action = 2;             /* a prior restart, so advance */
      else
        action = 1;             /* desired restart or too far away */
    }

    switch (action) {
    case 1:
      /* Let entropy decoder resume processing. */
      return;
    case 2:
      /* Scan to the next marker, and repeat the decision loop. */
      marker = next_marker();
      break;
    case 3:
      /* Put back this marker & return. */
      /* Entropy decoder will be forced to process an empty segment. */
      JUNGETC(marker);
      JUNGETC(0xFF);
      return;
    }
  }
}

/* Check for a restart marker & resynchronize decoder.  */
static void
process_restart()
{
  UINT8 c;
  int nbytes;
  short ci;

  /* Throw away any unused bits remaining in bit buffer */
  nbytes = bits_left / 8;       /* count any full bytes loaded into buffer */
  bits_left = 0;
  printed_eod = FALSE;          /* next segment can get another warning */

  /* Scan for next JPEG marker */
  do {
    do {                        /* skip any non-FF bytes */
      nbytes++;
      c = JGETC();
    } while (c != 0xFF);
    do {                        /* skip any duplicate FFs */
      /* we don't increment nbytes here since extra FFs are legal */
      c = JGETC();
    } while (c == 0xFF);
  } while (c == 0);             /* repeat if it was a stuffed FF/00 */

  if (nbytes != 1)
    warning(Corrupt_JPEG);

  if (c != (RST0 + next_restart_num))
    /* Uh-oh, the restart markers have been messed up too. */
    /* Let the file-format module try to figure out how to resync. */
    resync_to_restart(c);

  /* Re-initialize DC predictions to 0 */
  for (ci = 0; ci < comps_in_scan; ci++)
    last_dc_val[ci] = 0;

  /* Update restart state */
  restarts_to_go = restart_interval;
  next_restart_num = (next_restart_num + 1) & 7;
}


/* ZAG[i] is the natural-order position of the i'th element of zigzag order.
 * If the incoming data is corrupted, decode_mcu could attempt to
 * reference values beyond the end of the array.  To avoid a wild store,
 * we put some extra zeroes after the real entries.
 */

static const short ZAG[DCTSIZE2+16] = {
  0,  1,  8, 16,  9,  2,  3, 10,
 17, 24, 32, 25, 18, 11,  4,  5,
 12, 19, 26, 33, 40, 48, 41, 34,
 27, 20, 13,  6,  7, 14, 21, 28,
 35, 42, 49, 56, 57, 50, 43, 36,
 29, 22, 15, 23, 30, 37, 44, 51,
 58, 59, 52, 45, 38, 31, 39, 46,
 53, 60, 61, 54, 47, 55, 62, 63,
  0,  0,  0,  0,  0,  0,  0,  0, /* extra entries in case k>63 below */
  0,  0,  0,  0,  0,  0,  0,  0
};


/*
 * Decode and return one MCU's worth of Huffman-compressed coefficients.
 * This routine also handles quantization descaling and zigzag reordering
 * of coefficient values.
 *
 * The i'th block of the MCU is stored into the block pointed to by
 * MCU_data[i].  WE ASSUME THIS AREA HAS BEEN ZEROED BY THE CALLER.
 * (Wholesale zeroing is usually a little faster than retail...)
 */
static void
decode_mcu(JBLOCKROW *MCU_data)
{
  int s, k, r;
  short blkn, ci;
  JBLOCKROW block;
  QUANT_TBL_PTR quanttbl;
  HUFF_TBL *dctbl;
  HUFF_TBL *actbl;
  jpeg_component_info * compptr;

  /* Account for restart interval, process restart marker if needed */
  if (restart_interval) {
    if (restarts_to_go == 0)
      process_restart();
    restarts_to_go--;
  }

  /* Outer loop handles each block in the MCU */

  for (blkn = 0; blkn < blocks_in_MCU; blkn++) {
    block = MCU_data[blkn];
    ci = MCU_membership[blkn];
    compptr = cur_comp_info[ci];
    quanttbl = quant_tbl_ptrs[compptr->quant_tbl_no];
    actbl = ac_huff_tbl_ptrs[compptr->ac_tbl_no];
    dctbl = dc_huff_tbl_ptrs[compptr->dc_tbl_no];

    /* Decode a single block's worth of coefficients */

    /* Section F.2.2.1: decode the DC coefficient difference */
    huff_DECODE(dctbl, s);
    if (s) {
      check_bit_buffer(s);
      r = get_bits(s);
      s = huff_EXTEND(r, s);
    }

    /* Convert DC difference to actual value, update last_dc_val */
    s += last_dc_val[ci];
    last_dc_val[ci] = (JCOEF) s;
    /* Descale and output the DC coefficient (assumes ZAG[0] = 0) */
    (*block)[0] = (JCOEF) (((JCOEF) s) * quanttbl[0]);
    
    /* Section F.2.2.2: decode the AC coefficients */
    /* Since zero values are skipped, output area must be zeroed beforehand */
    for (k = 1; k < DCTSIZE2; k++) {
      huff_DECODE(actbl, s);
      
      r = s >> 4;
      s &= 15;
      
      if (s) {
        k += r;
        check_bit_buffer(s);
        r = get_bits(s);
        s = huff_EXTEND(r, s);
        /* Descale coefficient and output in natural (dezigzagged) order */
        (*block)[ZAG[k]] = (JCOEF) (((JCOEF) s) * quanttbl[k]);
      } else {
        if (r != 15)
          break;
        k += 15;
      }
    }
  }
}


/*
 * Finish up at the end of a Huffman-compressed scan.
 */

static void
decoder_term()
{
  /* No work needed */
}

/*
 * These variables are logically local to the pipeline controller,
 * but we make them static so that scan_big_image can use them
 * without having to pass them through the quantization routines.
 */

static int rows_in_mem;         /* # of sample rows in full-size buffers */
/* Work buffer for data being passed to output module. */

/* This has color_out_comps components if not quantizing, */

static JSAMPLE * sample_range_limit; /* table for fast range-limiting */

/* but only one component when quantizing. */
static JSAMPIMAGE output_workspace;

/*
 * Memory allocation and freeing are controlled by the regular library
 * routines malloc() and free().
 */

static void *
jget_small (size_t sizeofobject)
{
  return (void *)polyray_malloc(sizeofobject);
}

static void
jfree_small (void * object)
{
  polyray_free(object);
}

/*
 * Management of "small" objects.
 * These are all-in-memory, and are in near-heap space on an 80x86.
 */

typedef union small_struct * small_ptr;
typedef double align_type;

typedef union small_struct {
        small_ptr next;         /* next in list of allocated objects */
        align_type dummy;       /* ensures alignment of following storage */
      } small_hdr;

static small_ptr small_list;    /* head of list */

static void *
alloc_small (size_t sizeofobject)
/* Allocate a "small" object */
{
  small_ptr result;

  sizeofobject += sizeof(small_hdr); /* add space for header */

  result = (small_ptr) jget_small(sizeofobject);
  if (result == NULL)
    JPEG_error(No_Memory);

  result->next = small_list;
  small_list = result;
  result++;                     /* advance past header */

  return (void *) result;
}


static void
free_small (void *ptr)
/* Free a "small" object */
{
  small_ptr hdr;
  small_ptr * llink;

  hdr = (small_ptr) ptr;
  hdr--;                        /* point back to header */

  /* Remove item from list -- linear search is fast enough */
  llink = &small_list;
  while (*llink != hdr) {
    if (*llink == NULL)
      JPEG_error(No_Memory);
    llink = &( (*llink)->next );
  }
  *llink = hdr->next;

  jfree_small((void *) hdr);
}

typedef struct small_sarray_struct * small_sarray_ptr;

typedef struct small_sarray_struct {
        small_sarray_ptr next;  /* next in list of allocated sarrays */
        long numrows;           /* # of rows in this array */
        long rowsperchunk;      /* max # of rows per allocation chunk */
        JSAMPROW dummy;         /* ensures alignment of following storage */
      } small_sarray_hdr;

static small_sarray_ptr small_sarray_list; /* head of list */

#define jget_large(sizeofobject)        jget_small(sizeofobject)
#define jfree_large(object)             jfree_small(object)
#define MAX_ALLOC_CHUNK         1000000000L

static JSAMPARRAY
alloc_small_sarray (long samplesperrow, long numrows)
/* Allocate a "small" (all-in-memory) 2-D sample array */
{
  small_sarray_ptr hdr;
  JSAMPARRAY result;
  JSAMPROW workspace;
  long rowsperchunk, currow, i;

  /* Calculate max # of rows allowed in one allocation chunk */
  rowsperchunk = MAX_ALLOC_CHUNK / (samplesperrow * sizeof(JSAMPLE));
  if (rowsperchunk <= 0)
      JPEG_error("JPEG image too large");

  /* Get space for header and row pointers; this is always "near" on 80x86 */
  hdr = (small_sarray_ptr) alloc_small((size_t) (numrows * sizeof(JSAMPROW)
                                                 + sizeof(small_sarray_hdr)));

  result = (JSAMPARRAY) (hdr+1); /* advance past header */

  /* Insert into list now so free_all does right thing if I fail */
  /* after allocating only some of the rows... */
  hdr->next = small_sarray_list;
  hdr->numrows = 0;
  hdr->rowsperchunk = rowsperchunk;
  small_sarray_list = hdr;

  /* Get the rows themselves; on 80x86 these are "far" */
  currow = 0;
  while (currow < numrows) {
    rowsperchunk = MIN(rowsperchunk, numrows - currow);
    workspace = (JSAMPROW) jget_large((size_t) (rowsperchunk * samplesperrow
                                                * sizeof(JSAMPLE)));
    if (workspace == NULL)
      JPEG_error(No_Memory);
    for (i = rowsperchunk; i > 0; i--) {
      result[currow++] = workspace;
      workspace += samplesperrow;
    }
    hdr->numrows = currow;
  }

  return result;
}

static void
free_small_sarray (JSAMPARRAY ptr)
/* Free a "small" (all-in-memory) 2-D sample array */
{
  small_sarray_ptr hdr;
  small_sarray_ptr * llink;
  long i;

  hdr = (small_sarray_ptr) ptr;
  hdr--;                        /* point back to header */

  /* Remove item from list -- linear search is fast enough */
  llink = &small_sarray_list;
  while (*llink != hdr) {
    if (*llink == NULL)
      JPEG_error(No_Memory);
    llink = &( (*llink)->next );
  }
  *llink = hdr->next;

  /* Free the rows themselves; on 80x86 these are "far" */
  /* Note we only free the row-group headers! */
  for (i = 0; i < hdr->numrows; i += hdr->rowsperchunk) {
    jfree_large((void *) ptr[i]);
  }

  /* Free header and row pointers */
  free_small((void *) hdr);
}

/*
 * Management of "small" (all-in-memory) 2-D coefficient-block arrays.
 * This is essentially the same as the code for sample arrays, above.
 */

typedef struct small_barray_struct * small_barray_ptr;

typedef struct small_barray_struct {
        small_barray_ptr next;  /* next in list of allocated barrays */
        long numrows;           /* # of rows in this array */
        long rowsperchunk;      /* max # of rows per allocation chunk */
        JBLOCKROW dummy;        /* ensures alignment of following storage */
      } small_barray_hdr;

static small_barray_ptr small_barray_list; /* head of list */

static JBLOCKARRAY
alloc_small_barray (long blocksperrow, long numrows)
/* Allocate a "small" (all-in-memory) 2-D coefficient-block array */
{
  small_barray_ptr hdr;
  JBLOCKARRAY result;
  JBLOCKROW workspace;
  long rowsperchunk, currow, i;

  /* Calculate max # of rows allowed in one allocation chunk */
  rowsperchunk = MAX_ALLOC_CHUNK / (blocksperrow * sizeof(JBLOCK));
  if (rowsperchunk <= 0)
      JPEG_error("JPEG image too large");

  /* Get space for header and row pointers; this is always "near" on 80x86 */
  hdr = (small_barray_ptr) alloc_small((size_t) (numrows * sizeof(JBLOCKROW)
                                                 + sizeof(small_barray_hdr)));

  result = (JBLOCKARRAY) (hdr+1); /* advance past header */

  /* Insert into list now so free_all does right thing if I fail */
  /* after allocating only some of the rows... */
  hdr->next = small_barray_list;
  hdr->numrows = 0;
  hdr->rowsperchunk = rowsperchunk;
  small_barray_list = hdr;

  /* Get the rows themselves; on 80x86 these are "far" */
  currow = 0;
  while (currow < numrows) {
    rowsperchunk = MIN(rowsperchunk, numrows - currow);
    workspace = (JBLOCKROW) jget_large((size_t) (rowsperchunk * blocksperrow
                                                 * sizeof(JBLOCK)));
    if (workspace == NULL)
      JPEG_error(No_Memory);
    for (i = rowsperchunk; i > 0; i--) {
      result[currow++] = workspace;
      workspace += blocksperrow;
    }
    hdr->numrows = currow;
  }

  return result;
}


static void
free_small_barray (JBLOCKARRAY ptr)
/* Free a "small" (all-in-memory) 2-D coefficient-block array */
{
  small_barray_ptr hdr;
  small_barray_ptr * llink;
  long i;

  hdr = (small_barray_ptr) ptr;
  hdr--;                        /* point back to header */

  /* Remove item from list -- linear search is fast enough */
  llink = &small_barray_list;
  while (*llink != hdr) {
    if (*llink == NULL)
      JPEG_error(No_Memory);
    llink = &( (*llink)->next );
  }
  *llink = hdr->next;

  /* Free the rows themselves; on 80x86 these are "far" */
  /* Note we only free the row-group headers! */
  for (i = 0; i < hdr->numrows; i += hdr->rowsperchunk) {
    jfree_large((void *) ptr[i]);
  }

  /* Free header and row pointers */
  free_small((void *) hdr);
}

typedef struct big_sarray_control * big_sarray_ptr;
struct big_sarray_control {
        long rows_in_array;     /* total virtual array height */
        long samplesperrow;     /* width of array (and of memory buffer) */
        long unitheight;        /* # of rows accessed by access_big_sarray() */
        JSAMPARRAY mem_buffer;  /* the in-memory buffer */
        long rows_in_mem;       /* height of memory buffer */
        long rowsperchunk;      /* allocation chunk size in mem_buffer */
        long cur_start_row;     /* first logical row # in the buffer */
        big_sarray_ptr next;    /* link to next big sarray control block */
};

static big_sarray_ptr big_sarray_list; /* head of list */

static big_sarray_ptr
request_big_sarray (long samplesperrow, long numrows, long unitheight)
/* Request a "big" (virtual-memory) 2-D sample array */
{
  big_sarray_ptr result;

  /* get control block */
  result = (big_sarray_ptr)alloc_small(sizeof(struct big_sarray_control));

  result->rows_in_array = numrows;
  result->samplesperrow = samplesperrow;
  result->unitheight = unitheight;
  result->mem_buffer = NULL;    /* marks array not yet realized */
  result->next = big_sarray_list; /* add to list of big arrays */
  big_sarray_list = result;

  return result;
}

static long
jmem_available (long min_bytes_needed, long max_bytes_needed)
{
  return max_bytes_needed;
}

static void
alloc_big_arrays (long extra_small_samples, long extra_small_blocks,
                  long extra_medium_space)
/* Allocate the in-memory buffers for any unrealized "big" arrays */
/* 'extra' values are upper bounds for total future small-array requests */
/* and far-heap requests */
{
  long total_extra_space = extra_small_samples * sizeof(JSAMPLE)
                           + extra_small_blocks * sizeof(JBLOCK)
                           + extra_medium_space;
  long space_per_unitheight, maximum_space, avail_mem;
  long unitheights, max_unitheights;
  big_sarray_ptr sptr;

  /* Compute the minimum space needed (unitheight rows in each buffer)
   * and the maximum space needed (full image height in each buffer).
   * These may be of use to the system-dependent jmem_available routine.
   */
  space_per_unitheight = 0;
  maximum_space = total_extra_space;
  for (sptr = big_sarray_list; sptr != NULL; sptr = sptr->next) {
    if (sptr->mem_buffer == NULL) { /* if not realized yet */
      space_per_unitheight += sptr->unitheight *
                              sptr->samplesperrow * sizeof(JSAMPLE);
      maximum_space += sptr->rows_in_array *
                       sptr->samplesperrow * sizeof(JSAMPLE);
    }
  }

  if (space_per_unitheight <= 0)
    return;                     /* no unrealized arrays, no work */

  /* Determine amount of memory to actually use; this is system-dependent. */
  avail_mem = jmem_available(space_per_unitheight + total_extra_space,
                             maximum_space);

  /* If the maximum space needed is available, make all the buffers full
   * height; otherwise parcel it out with the same number of unitheights
   * in each buffer.
   */
  if (avail_mem >= maximum_space)
    max_unitheights = 1000000000L;
  else {
    max_unitheights = (avail_mem - total_extra_space) / space_per_unitheight;
    /* If there doesn't seem to be enough space, try to get the minimum
     * anyway.  This allows a "stub" implementation of jmem_available().
     */
    if (max_unitheights <= 0)
      max_unitheights = 1;
  }

  /* Allocate the in-memory buffers and initialize backing store as needed. */

  for (sptr = big_sarray_list; sptr != NULL; sptr = sptr->next) {
    if (sptr->mem_buffer == NULL) { /* if not realized yet */
      unitheights = (sptr->rows_in_array + sptr->unitheight - 1L)
                    / sptr->unitheight;
      if (unitheights <= max_unitheights) {
        /* This buffer fits in memory */
        sptr->rows_in_mem = sptr->rows_in_array;
        }
      else
        JPEG_error(No_Memory);
      sptr->mem_buffer = alloc_small_sarray(sptr->samplesperrow,
                                            sptr->rows_in_mem);
      /* Reach into the small_sarray header and get the rowsperchunk field.
       * Yes, I know, this is horrible coding practice.
       */
      sptr->rowsperchunk =
        ((small_sarray_ptr) sptr->mem_buffer)[-1].rowsperchunk;
      sptr->cur_start_row = 0;
    }
  }
}

static JSAMPARRAY
access_big_sarray (big_sarray_ptr ptr, long start_row, boolean writable)
{
  /* debugging check */
  if (start_row < 0 || start_row+ptr->unitheight > ptr->rows_in_array ||
      ptr->mem_buffer == NULL)
    JPEG_error(No_Memory);

  /* Return address of proper part of the buffer */
  return ptr->mem_buffer + (start_row - ptr->cur_start_row);
}

static void
free_big_sarray (big_sarray_ptr ptr)
/* Free a "big" (virtual-memory) 2-D sample array */
{
  big_sarray_ptr * llink;

  /* Remove item from list -- linear search is fast enough */
  llink = &big_sarray_list;
  while (*llink != ptr) {
    if (*llink == NULL)
      JPEG_error(No_Memory);
    llink = &( (*llink)->next );
  }
  *llink = ptr->next;

  if (ptr->mem_buffer != NULL)  /* just in case never realized */
    free_small_sarray(ptr->mem_buffer);

  free_small((void *) ptr);     /* free the control block too */
}

static void
free_all (void)
{
  /* First free any open "big" arrays -- these may release small arrays */
  while (big_sarray_list != NULL)
    free_big_sarray(big_sarray_list);
  /* Free any open small arrays -- these may release small objects */
  /* +1's are because we must pass a pointer to the data, not the header */
  while (small_sarray_list != NULL)
    free_small_sarray((JSAMPARRAY) (small_sarray_list + 1));
  while (small_barray_list != NULL)
    free_small_barray((JBLOCKARRAY) (small_barray_list + 1));
  /* Free any remaining small objects */
  while (small_list != NULL)
    free_small((void *) (small_list + 1));
}

static void
jselmemmgr()
{
  /* Initialize list headers to empty */
  small_list = NULL;
  small_sarray_list = NULL;
  small_barray_list = NULL;
  big_sarray_list = NULL;
}

#ifdef NEED_COMPLEX_CONTROLLER
/* Full-size image array holding upsampled, but not color-processed data. */
static big_sarray_ptr *fullsize_image;
static JSAMPIMAGE fullsize_ptrs; /* workspace for access_big_sarray() result */
#endif

static long
jround_up(long a, long b)
/* Compute a rounded up to next multiple of b; a >= 0, b > 0 */
{
  a += b-1;
  return a - (a % b);
}

/* Initialize for processing a scan.  */
static void
disassemble_init(void)
{
  /* no work for now */
}

/* Clean up after a scan.  */
static void
disassemble_term(void)
{
  /* no work for now */
}

static void
jcopy_sample_rows (JSAMPARRAY input_array, int source_row,
                   JSAMPARRAY output_array, int dest_row,
                   int num_rows, long num_cols)
/* Copy some rows of samples from one place to another.
 * num_rows rows are copied from input_array[source_row++]
 * to output_array[dest_row++]; these areas should not overlap.
 * The source and destination arrays must be at least as wide as num_cols.
 */
{
  JSAMPROW inptr, outptr;
  long count;
  int row;

  input_array += source_row;
  output_array += dest_row;

  for (row = num_rows; row > 0; row--) {
    inptr = *input_array++;
    outptr = *output_array++;
    for (count = num_cols; count > 0; count--)
      *outptr++ = *inptr++;
  }
}

static void
jzero_far(void *target, size_t bytestozero)
{
  char *ptr = (char *) target;
  size_t count;

  for (count = bytestozero; count > 0; count--) {
    *ptr++ = 0;
  }
}

/*
 * Fetch one MCU row from entropy_decode, build coefficient array.
 * This version is used for noninterleaved (single-component) scans.
 */
static void
disassemble_noninterleaved_MCU(JBLOCKIMAGE image_data)
{
  JBLOCKROW MCU_data[1];
  long mcuindex;

  /* this is pretty easy since there is one component and one block per MCU */

  /* Pre-zero the target area to speed up entropy decoder */
  /* (we assume wholesale zeroing is faster than retail) */
  jzero_far((void *) image_data[0][0],
            (size_t) (MCUs_per_row * sizeof(JBLOCK)));

  for (mcuindex = 0; mcuindex < MCUs_per_row; mcuindex++) {
    /* Point to the proper spot in the image array for this MCU */
    MCU_data[0] = image_data[0][0] + mcuindex;
    /* Fetch the coefficient data */
    decode_mcu(MCU_data);
  }
}

/*
 * Fetch one MCU row from entropy_decode, build coefficient array.
 * This version is used for interleaved (multi-component) scans.
 */
static void
disassemble_interleaved_MCU(JBLOCKIMAGE image_data)
{
  JBLOCKROW MCU_data[MAX_BLOCKS_IN_MCU];
  long mcuindex;
  short blkn, ci, xpos, ypos;
  jpeg_component_info * compptr;
  JBLOCKROW image_ptr;

  /* Pre-zero the target area to speed up entropy decoder */
  /* (we assume wholesale zeroing is faster than retail) */
  for (ci = 0; ci < comps_in_scan; ci++) {
    compptr = cur_comp_info[ci];
    for (ypos = 0; ypos < compptr->MCU_height; ypos++) {
      jzero_far((void *) image_data[ci][ypos],
                (size_t)(MCUs_per_row * compptr->MCU_width * sizeof(JBLOCK)));
    }
  }

  for (mcuindex = 0; mcuindex < MCUs_per_row; mcuindex++) {
    /* Point to the proper spots in the image array for this MCU */
    blkn = 0;
    for (ci = 0; ci < comps_in_scan; ci++) {
      compptr = cur_comp_info[ci];
      for (ypos = 0; ypos < compptr->MCU_height; ypos++) {
        image_ptr = image_data[ci][ypos] + (mcuindex * compptr->MCU_width);
        for (xpos = 0; xpos < compptr->MCU_width; xpos++) {
          MCU_data[blkn] = image_ptr;
          image_ptr++;
          blkn++;
        }
      }
    }
    /* Fetch the coefficient data */
    decode_mcu(MCU_data);
  }
}


/* The method selection routine for MCU disassembly.  */
static void
jseldmcu(void)
{
  if (comps_in_scan == 1)
    disassemble_MCU = disassemble_noninterleaved_MCU;
  else
    disassemble_MCU = disassemble_interleaved_MCU;
}

/*
 * Upsample pixel values of a single component.
 * This version handles any integral sampling ratios.
 *
 * This is not used for typical JPEG files, so it need not be fast.
 * Nor, for that matter, is it particularly accurate: the algorithm is
 * simple replication of the input pixel onto the corresponding output
 * pixels.  The hi-falutin sampling literature refers to this as a
 * "box filter".  A box filter tends to introduce visible artifacts,
 * so if you are actually going to use 3:1 or 4:1 sampling ratios
 * you would be well advised to improve this code.
 */
static void
int_upsample(int which_component, long input_cols, int input_rows,
             long output_cols, int output_rows,
             JSAMPARRAY above, JSAMPARRAY input_data, JSAMPARRAY below,
             JSAMPARRAY output_data)
{
  jpeg_component_info * compptr = cur_comp_info[which_component];
  JSAMPROW inptr, outptr;
  JSAMPLE invalue;
  short h_expand, h;
  short v_expand, v;
  int inrow, outrow;
  long incol;

  h_expand = max_h_samp_factor / compptr->h_samp_factor;
  v_expand = max_v_samp_factor / compptr->v_samp_factor;

  outrow = 0;
  for (inrow = 0; inrow < input_rows; inrow++) {
    for (v = 0; v < v_expand; v++) {
      inptr = input_data[inrow];
      outptr = output_data[outrow++];
      for (incol = 0; incol < input_cols; incol++) {
        invalue = GETJSAMPLE(*inptr++);
        for (h = 0; h < h_expand; h++) {
          *outptr++ = invalue;
        }
      }
    }
  }
}

/*
 * Upsample pixel values of a single component.
 * This version handles the common case of 2:1 horizontal and 1:1 vertical.
 *
 * The upsampling algorithm is linear interpolation between pixel centers,
 * also known as a "triangle filter".  This is a good compromise between
 * speed and visual quality.  The centers of the output pixels are 1/4 and 3/4
 * of the way between input pixel centers.
 */
static void
h2v1_upsample(int which_component, long input_cols, int input_rows,
              long output_cols, int output_rows,
              JSAMPARRAY above, JSAMPARRAY input_data, JSAMPARRAY below,
              JSAMPARRAY output_data)
{
  JSAMPROW inptr, outptr;
  int invalue;
  int inrow;
  long colctr;

  for (inrow = 0; inrow < input_rows; inrow++) {
    inptr = input_data[inrow];
    outptr = output_data[inrow];
    /* Special case for first column */
    invalue = GETJSAMPLE(*inptr++);
    *outptr++ = (JSAMPLE) invalue;
    *outptr++ = (JSAMPLE) ((invalue * 3 + GETJSAMPLE(*inptr) + 2) >> 2);

    for (colctr = input_cols - 2; colctr > 0; colctr--) {
      /* General case: 3/4 * nearer pixel + 1/4 * further pixel */
      invalue = GETJSAMPLE(*inptr++) * 3;
      *outptr++ = (JSAMPLE) ((invalue + GETJSAMPLE(inptr[-2]) + 2) >> 2);
      *outptr++ = (JSAMPLE) ((invalue + GETJSAMPLE(*inptr) + 2) >> 2);
    }

    /* Special case for last column */
    invalue = GETJSAMPLE(*inptr);
    *outptr++ = (JSAMPLE) ((invalue * 3 + GETJSAMPLE(inptr[-1]) + 2) >> 2);
    *outptr++ = (JSAMPLE) invalue;
  }
}

/*
 * Upsample pixel values of a single component.
 * This version handles the common case of 2:1 horizontal and 2:1 vertical.
 *
 * The upsampling algorithm is linear interpolation between pixel centers,
 * also known as a "triangle filter".  This is a good compromise between
 * speed and visual quality.  The centers of the output pixels are 1/4 and 3/4
 * of the way between input pixel centers.
 */
static void
h2v2_upsample(int which_component, long input_cols, int input_rows,
              long output_cols, int output_rows,
              JSAMPARRAY above, JSAMPARRAY input_data, JSAMPARRAY below,
              JSAMPARRAY output_data)
{
  JSAMPROW inptr0, inptr1, outptr;
  int thiscolsum, lastcolsum, nextcolsum;
  int inrow, outrow, v;
  long colctr;

  outrow = 0;
  for (inrow = 0; inrow < input_rows; inrow++) {
    for (v = 0; v < 2; v++) {
      /* inptr0 points to nearest input row, inptr1 points to next nearest */
      inptr0 = input_data[inrow];
      if (v == 0) {             /* next nearest is row above */
        if (inrow == 0)
          inptr1 = above[input_rows-1];
        else
          inptr1 = input_data[inrow-1];
      } else {                  /* next nearest is row below */
        if (inrow == input_rows-1)
          inptr1 = below[0];
        else
          inptr1 = input_data[inrow+1];
      }
      outptr = output_data[outrow++];

      /* Special case for first column */
      thiscolsum = GETJSAMPLE(*inptr0++) * 3 + GETJSAMPLE(*inptr1++);
      nextcolsum = GETJSAMPLE(*inptr0++) * 3 + GETJSAMPLE(*inptr1++);
      *outptr++ = (JSAMPLE) ((thiscolsum * 4 + 8) >> 4);
      *outptr++ = (JSAMPLE) ((thiscolsum * 3 + nextcolsum + 8) >> 4);
      lastcolsum = thiscolsum; thiscolsum = nextcolsum;

      for (colctr = input_cols - 2; colctr > 0; colctr--) {
        /* General case: 3/4 * nearer pixel + 1/4 * further pixel in each */
        /* dimension, thus 9/16, 3/16, 3/16, 1/16 overall */
        nextcolsum = GETJSAMPLE(*inptr0++) * 3 + GETJSAMPLE(*inptr1++);
        *outptr++ = (JSAMPLE) ((thiscolsum * 3 + lastcolsum + 8) >> 4);
        *outptr++ = (JSAMPLE) ((thiscolsum * 3 + nextcolsum + 8) >> 4);
        lastcolsum = thiscolsum; thiscolsum = nextcolsum;
      }

      /* Special case for last column */
      *outptr++ = (JSAMPLE) ((thiscolsum * 3 + lastcolsum + 8) >> 4);
      *outptr++ = (JSAMPLE) ((thiscolsum * 4 + 8) >> 4);
    }
  }
}

/*
 * Upsample pixel values of a single component.
 * This version handles the special case of a full-size component.
 */
static void
fullsize_upsample(int which_component, long input_cols, int input_rows,
                  long output_cols, int output_rows,
                  JSAMPARRAY above, JSAMPARRAY input_data, JSAMPARRAY below,
                  JSAMPARRAY output_data)
{
  jcopy_sample_rows(input_data, 0, output_data, 0, output_rows, output_cols);
}

/*
 * The method selection routine for upsampling.
 * Note that we must select a routine for each component.
 */
static void
jselupsample(void)
{
  short ci;
  jpeg_component_info * compptr;

  for (ci = 0; ci < comps_in_scan; ci++) {
    compptr = cur_comp_info[ci];
    if (compptr->h_samp_factor == max_h_samp_factor &&
        compptr->v_samp_factor == max_v_samp_factor)
      upsample[ci] = fullsize_upsample;
    else if (compptr->h_samp_factor * 2 == max_h_samp_factor &&
             compptr->v_samp_factor == max_v_samp_factor)
      upsample[ci] = h2v1_upsample;
    else if (compptr->h_samp_factor * 2 == max_h_samp_factor &&
             compptr->v_samp_factor * 2 == max_v_samp_factor)
      upsample[ci] = h2v2_upsample;
    else if ((max_h_samp_factor % compptr->h_samp_factor) == 0 &&
             (max_v_samp_factor % compptr->v_samp_factor) == 0)
      upsample[ci] = int_upsample;
    else
      JPEG_error(Bad_JPEG);
  }
}

static void
d_per_scan_method_selection()
/* Central point for per-scan method selection */
{
  /* MCU disassembly */
  jseldmcu();

  /* Upsampling of pixels */
  jselupsample();
}

/* Utility routines: common code for pipeline controllers */
static void
interleaved_scan_setup(void)
/* Compute all derived info for an interleaved (multi-component) scan */
/* On entry, comps_in_scan and cur_comp_info[] are set up */
{
  short ci, mcublks;
  jpeg_component_info *compptr;

  if (comps_in_scan > MAX_COMPS_IN_SCAN)
    JPEG_error(Bad_JPEG);

  MCUs_per_row = (image_width + max_h_samp_factor*DCTSIZE - 1)
                        / (max_h_samp_factor*DCTSIZE);

  MCU_rows_in_scan = (image_height + max_v_samp_factor*DCTSIZE - 1)
                            / (max_v_samp_factor*DCTSIZE);
  
  blocks_in_MCU = 0;

  for (ci = 0; ci < comps_in_scan; ci++) {
    compptr = cur_comp_info[ci];
    /* for interleaved scan, sampling factors give # of blocks per component */
    compptr->MCU_width = compptr->h_samp_factor;
    compptr->MCU_height = compptr->v_samp_factor;
    compptr->MCU_blocks = compptr->MCU_width * compptr->MCU_height;
    /* compute physical dimensions of component */
    compptr->downsampled_width = jround_up(compptr->true_comp_width,
                                           (long) (compptr->MCU_width*DCTSIZE));
    compptr->downsampled_height = jround_up(compptr->true_comp_height,
                                            (long) (compptr->MCU_height*DCTSIZE));
    /* Sanity check */
    if (compptr->downsampled_width !=
        (MCUs_per_row * (compptr->MCU_width*DCTSIZE)))
      JPEG_error(Bad_JPEG);

    /* Prepare array describing MCU composition */
    mcublks = compptr->MCU_blocks;
    if (blocks_in_MCU + mcublks > MAX_BLOCKS_IN_MCU)
      JPEG_error(Bad_JPEG);
    while (mcublks-- > 0) {
      MCU_membership[blocks_in_MCU++] = ci;
    }
  }

  d_per_scan_method_selection();
}

static void
noninterleaved_scan_setup(void)
/* Compute all derived info for a noninterleaved (single-component) scan */
/* On entry, comps_in_scan = 1 and cur_comp_info[0] is set up */
{
  jpeg_component_info *compptr = cur_comp_info[0];

  /* for noninterleaved scan, always one block per MCU */
  compptr->MCU_width = 1;
  compptr->MCU_height = 1;
  compptr->MCU_blocks = 1;
  /* compute physical dimensions of component */
  compptr->downsampled_width = jround_up(compptr->true_comp_width,
                                         (long) DCTSIZE);
  compptr->downsampled_height = jround_up(compptr->true_comp_height,
                                          (long) DCTSIZE);

  MCUs_per_row = compptr->downsampled_width / DCTSIZE;
  MCU_rows_in_scan = compptr->downsampled_height / DCTSIZE;

  /* Prepare array describing MCU composition */
  blocks_in_MCU = 1;
  MCU_membership[0] = 0;

  d_per_scan_method_selection();
}



static JSAMPIMAGE
alloc_sampimage(int num_comps, long num_rows, long num_cols)
/* Allocate an in-memory sample image (all components same size) */
{
  JSAMPIMAGE image;
  int ci;

  image = (JSAMPIMAGE)alloc_small(num_comps * sizeof(JSAMPARRAY));
  for (ci = 0; ci < num_comps; ci++) {
    image[ci] = alloc_small_sarray(num_cols, num_rows);
  }
  return image;
}

static JBLOCKIMAGE
alloc_MCU_row(void)
/* Allocate one MCU row's worth of coefficient blocks */
{
  JBLOCKIMAGE image;
  int ci;

  image = (JBLOCKIMAGE)alloc_small(comps_in_scan * sizeof(JBLOCKARRAY));
  for (ci = 0; ci < comps_in_scan; ci++)
    image[ci] = alloc_small_barray(cur_comp_info[ci]->downsampled_width /
                                   DCTSIZE,(long)cur_comp_info[ci]->MCU_height);
  return image;
}


#ifdef NEED_COMPLEX_CONTROLLER  /* not used by simple controller */

static void
free_MCU_row(JBLOCKIMAGE image)
/* Release a coefficient block array created by alloc_MCU_row */
{
  int ci;

  for (ci = 0; ci < comps_in_scan; ci++)
    free_small_barray(image[ci]);
  free_small((void *)image);
}

#endif


static void
alloc_sampling_buffer(JSAMPIMAGE sampled_data[2])
/* Create a downsampled-data buffer having the desired structure */
/* (see comments at head of file) */
{
  short ci, vs, i;

  /* Get top-level space for array pointers */
  sampled_data[0] = (JSAMPIMAGE)alloc_small
                                (comps_in_scan * sizeof(JSAMPARRAY));
  sampled_data[1] = (JSAMPIMAGE)alloc_small
                                (comps_in_scan * sizeof(JSAMPARRAY));

  for (ci = 0; ci < comps_in_scan; ci++) {
    vs = cur_comp_info[ci]->v_samp_factor; /* row group height */
    /* Allocate the real storage */
    sampled_data[0][ci] = alloc_small_sarray(
                                cur_comp_info[ci]->downsampled_width,
                                (long) (vs * (DCTSIZE+2)));

    /* Create space for the scrambled-order pointers */
    sampled_data[1][ci] = (JSAMPARRAY)alloc_small
                                (vs * (DCTSIZE+2) * sizeof(JSAMPROW));

    /* Duplicate the first DCTSIZE-2 row groups */
    for (i = 0; i < vs * (DCTSIZE-2); i++)
      sampled_data[1][ci][i] = sampled_data[0][ci][i];

    /* Copy the last four row groups in swapped order */
    for (i = 0; i < vs * 2; i++) {
      sampled_data[1][ci][vs*DCTSIZE+i] = sampled_data[0][ci][vs*(DCTSIZE-2)+i];
      sampled_data[1][ci][vs*(DCTSIZE-2)+i] = sampled_data[0][ci][vs*DCTSIZE+i];
    }
  }
}


#ifdef NEED_COMPLEX_CONTROLLER  /* not used by simple controller */

static void
free_sampling_buffer(JSAMPIMAGE sampled_data[2])
/* Release a sampling buffer created by alloc_sampling_buffer */
{
  short ci;

  for (ci = 0; ci < comps_in_scan; ci++) {
    /* Free the real storage */
    free_small_sarray(sampled_data[0][ci]);
    /* Free the scrambled-order pointers */
    free_small((void *) sampled_data[1][ci]);
  }

  /* Free the top-level space */
  free_small((void *) sampled_data[0]);
  free_small((void *) sampled_data[1]);
}

#endif


/*
 * Several decompression processes need to range-limit values to the range
 * 0..MAXJSAMPLE; the input value may fall somewhat outside this range
 * due to noise introduced by quantization, roundoff error, etc.  These
 * processes are inner loops and need to be as fast as possible.  On most
 * machines, particularly CPUs with pipelines or instruction prefetch,
 * a (range-check-less) C table lookup
 *              x = sample_range_limit[x];
 * is faster than explicit tests
 *              if (x < 0)  x = 0;
 *              else if (x > MAXJSAMPLE)  x = MAXJSAMPLE;
 * These processes all use a common table prepared by the routine below.
 *
 * The table will work correctly for x within MAXJSAMPLE+1 of the legal
 * range.  This is a much wider range than is needed for most cases,
 * but the wide range is handy for color quantization.
 * Note that the table is allocated in near data space on PCs; it's small
 * enough and used often enough to justify this.
 */

static void
prepare_range_limit_table(void)
/* Allocate and fill in the sample_range_limit table */
{
  JSAMPLE * table;
  int i;

  table = (JSAMPLE *)alloc_small(3 * (MAXJSAMPLE+1) * sizeof(JSAMPLE));
  sample_range_limit = table + (MAXJSAMPLE+1);
  for (i = 0; i <= MAXJSAMPLE; i++) {
    table[i] = 0;                       /* sample_range_limit[x] = 0 for x<0 */
    table[i+(MAXJSAMPLE+1)] = (JSAMPLE) i;      /* sample_range_limit[x] = x */
    table[i+(MAXJSAMPLE+1)*2] = MAXJSAMPLE;     /* x beyond MAXJSAMPLE */
  }
}


static void
duplicate_row (JSAMPARRAY image_data,
               long num_cols, int source_row, int num_rows)
/* Duplicate the source_row at source_row+1 .. source_row+num_rows */
/* This happens only at the bottom of the image, */
/* so it needn't be super-efficient */
{
  int row;

  for (row = 1; row <= num_rows; row++) {
    jcopy_sample_rows(image_data, source_row, image_data, source_row + row,
                      1, num_cols);
  }
}

static void
expand(JSAMPIMAGE sampled_data, JSAMPIMAGE fullsize_data, long fullsize_width,
        short above, short current, short below, short out)
/* Do upsampling expansion of a single row group (of each component). */
/* above, current, below are indexes of row groups in sampled_data;       */
/* out is the index of the target row group in fullsize_data.             */
/* Special case: above, below can be -1 to indicate top, bottom of image. */
{
  jpeg_component_info *compptr;
  JSAMPARRAY above_ptr, below_ptr;
  JSAMPROW dummy[MAX_SAMP_FACTOR]; /* for downsample expansion at top/bottom */
  short ci, vs, i;

  for (ci = 0; ci < comps_in_scan; ci++) {
    compptr = cur_comp_info[ci];
    /* don't bother to upsample an uninteresting component */
    if (! compptr->component_needed)
      continue;

    vs = compptr->v_samp_factor; /* row group height */

    if (above >= 0)
      above_ptr = sampled_data[ci] + above * vs;
    else {
      /* Top of image: make a dummy above-context with copies of 1st row */
      /* We assume current=0 in this case */
      for (i = 0; i < vs; i++)
        dummy[i] = sampled_data[ci][0];
      above_ptr = (JSAMPARRAY) dummy; /* possible near->far pointer conv */
    }

    if (below >= 0)
      below_ptr = sampled_data[ci] + below * vs;
    else {
      /* Bot of image: make a dummy below-context with copies of last row */
      for (i = 0; i < vs; i++)
        dummy[i] = sampled_data[ci][(current+1)*vs-1];
      below_ptr = (JSAMPARRAY) dummy; /* possible near->far pointer conv */
    }

    upsample[ci]((int) ci, compptr->downsampled_width, (int) vs,
                 fullsize_width, (int)max_v_samp_factor, above_ptr,
                 sampled_data[ci] + current * vs, below_ptr,
                 fullsize_data[ci] + out * max_v_samp_factor);
  }
}


static void
emit_1pass(int num_rows, JSAMPIMAGE fullsize_data, JSAMPARRAY dummy)
/* Do color processing and output of num_rows full-size rows. */
/* This is not used when doing 2-pass color quantization. */
/* The dummy argument simply lets this be called via scan_big_image. */
{
  color_convert(num_rows, image_width, fullsize_data, output_workspace);
  put_pixel_rows(num_rows, output_workspace);
}


/* Support routines for complex controller. */

#ifdef NEED_COMPLEX_CONTROLLER

static void
scan_big_image(quantize_method_ptr quantize_method)
/* Apply quantize_method to entire image stored in fullsize_image[]. */
/* This is the "iterator" routine used by the 2-pass color quantizer. */
/* We also use it directly in some cases. */
{
  long pixel_rows_output;
  short ci;

  for (pixel_rows_output = 0; pixel_rows_output < image_height;
       pixel_rows_output += rows_in_mem) {
    /* Realign the big buffers */
    for (ci = 0; ci < num_components; ci++) {
      fullsize_ptrs[ci] = access_big_sarray(fullsize_image[ci],
                                            pixel_rows_output, FALSE);
    }
    /* Let the quantizer have its way with the data.
     * Note that output_workspace is simply workspace for the quantizer;
     * when it's ready to output, it must call put_pixel_rows itself.
     */
    (*quantize_method)((int) MIN((long) rows_in_mem,
                                 image_height - pixel_rows_output),
                       fullsize_ptrs, output_workspace[0]);
  }
}

#endif /* NEED_COMPLEX_CONTROLLER */

/* Initialize for upsampling a scan.  */
static void upsample_init(void) { }

/* Clean up after a scan.  */
static void upsample_term(void) { }

#define CONST_BITS  13
#define PASS1_BITS  2

#define ONE     ((INT32) 1)

#define CONST_SCALE (ONE << CONST_BITS)

/* Convert a positive real constant to an integer scaled by CONST_SCALE. */

#define FIXY(x) ((INT32) ((x) * CONST_SCALE + 0.5))

#define FIX_0_298631336  ((INT32)  2446)        /* FIXY(0.298631336) */
#define FIX_0_390180644  ((INT32)  3196)        /* FIXY(0.390180644) */
#define FIX_0_541196100  ((INT32)  4433)        /* FIXY(0.541196100) */
#define FIX_0_765366865  ((INT32)  6270)        /* FIXY(0.765366865) */
#define FIX_0_899976223  ((INT32)  7373)        /* FIXY(0.899976223) */
#define FIX_1_175875602  ((INT32)  9633)        /* FIXY(1.175875602) */
#define FIX_1_501321110  ((INT32)  12299)       /* FIXY(1.501321110) */
#define FIX_1_847759065  ((INT32)  15137)       /* FIXY(1.847759065) */
#define FIX_1_961570560  ((INT32)  16069)       /* FIXY(1.961570560) */
#define FIX_2_053119869  ((INT32)  16819)       /* FIXY(2.053119869) */
#define FIX_2_562915447  ((INT32)  20995)       /* FIXY(2.562915447) */
#define FIX_3_072711026  ((INT32)  25172)       /* FIXY(3.072711026) */

/* Initialize for colorspace conversion.  */
#define RIGHT_SHIFT(x,shft)     ((x) >> (shft))

#define DESCALE(x,n)  RIGHT_SHIFT((x) + (ONE << ((n)-1)), n)
#ifdef SHORTxSHORT_32           /* may work if 'int' is 32 bits */
#define MULTIPLY(var,const)  (((INT16) (var)) * ((INT16) (const)))
#endif
#ifdef SHORTxLCONST_32          /* known to work with Microsoft C 6.0 */
#define MULTIPLY(var,const)  (((INT16) (var)) * ((INT32) (const)))
#endif

#ifndef MULTIPLY                /* default definition */
#define MULTIPLY(var,const)  ((var) * (const))
#endif

/* Perform the inverse DCT on one block of coefficients.  */
static void
j_rev_dct(DCTBLOCK data)
{
  INT32 tmp0, tmp1, tmp2, tmp3;
  INT32 tmp10, tmp11, tmp12, tmp13;
  INT32 z1, z2, z3, z4, z5;
  DCTELEM *dataptr;
  int rowctr;

  /* Pass 1: process rows. */
  /* Note results are scaled up by sqrt(8) compared to a true IDCT; */
  /* furthermore, we scale the results by 2**PASS1_BITS. */

  dataptr = data;
  for (rowctr = DCTSIZE-1; rowctr >= 0; rowctr--) {
    /* Due to quantization, we will usually find that many of the input
     * coefficients are zero, especially the AC terms.  We can exploit this
     * by short-circuiting the IDCT calculation for any row in which all
     * the AC terms are zero.  In that case each output is equal to the
     * DC coefficient (with scale factor as needed).
     * With typical images and quantization tables, half or more of the
     * row DCT calculations can be simplified this way.
     */

    if ((dataptr[1] | dataptr[2] | dataptr[3] | dataptr[4] |
         dataptr[5] | dataptr[6] | dataptr[7]) == 0) {
      /* AC terms all zero */
      DCTELEM dcval = (DCTELEM) (dataptr[0] << PASS1_BITS);
      
      dataptr[0] = dcval;
      dataptr[1] = dcval;
      dataptr[2] = dcval;
      dataptr[3] = dcval;
      dataptr[4] = dcval;
      dataptr[5] = dcval;
      dataptr[6] = dcval;
      dataptr[7] = dcval;
      
      dataptr += DCTSIZE;       /* advance pointer to next row */
      continue;
    }

    /* Even part: reverse the even part of the forward DCT. */
    /* The rotator is sqrt(2)*c(-6). */

    z2 = (INT32) dataptr[2];
    z3 = (INT32) dataptr[6];

    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = ((INT32) dataptr[0] + (INT32) dataptr[4]) << CONST_BITS;
    tmp1 = ((INT32) dataptr[0] - (INT32) dataptr[4]) << CONST_BITS;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    
    /* Odd part per figure 8; the matrix is unitary and hence its
     * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
     */

    tmp0 = (INT32) dataptr[7];
    tmp1 = (INT32) dataptr[5];
    tmp2 = (INT32) dataptr[3];
    tmp3 = (INT32) dataptr[1];

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */
    
    tmp0 = MULTIPLY(tmp0, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
    z1 = MULTIPLY(z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
    z2 = MULTIPLY(z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
    z3 = MULTIPLY(z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
    z4 = MULTIPLY(z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */
    
    z3 += z5;
    z4 += z5;
    
    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */

    dataptr[0] = (DCTELEM) DESCALE(tmp10 + tmp3, CONST_BITS-PASS1_BITS);
    dataptr[7] = (DCTELEM) DESCALE(tmp10 - tmp3, CONST_BITS-PASS1_BITS);
    dataptr[1] = (DCTELEM) DESCALE(tmp11 + tmp2, CONST_BITS-PASS1_BITS);
    dataptr[6] = (DCTELEM) DESCALE(tmp11 - tmp2, CONST_BITS-PASS1_BITS);
    dataptr[2] = (DCTELEM) DESCALE(tmp12 + tmp1, CONST_BITS-PASS1_BITS);
    dataptr[5] = (DCTELEM) DESCALE(tmp12 - tmp1, CONST_BITS-PASS1_BITS);
    dataptr[3] = (DCTELEM) DESCALE(tmp13 + tmp0, CONST_BITS-PASS1_BITS);
    dataptr[4] = (DCTELEM) DESCALE(tmp13 - tmp0, CONST_BITS-PASS1_BITS);

    dataptr += DCTSIZE;         /* advance pointer to next row */
  }

  /* Pass 2: process columns. */
  /* Note that we must descale the results by a factor of 8 == 2**3, */
  /* and also undo the PASS1_BITS scaling. */

  dataptr = data;
  for (rowctr = DCTSIZE-1; rowctr >= 0; rowctr--) {
    /* Columns of zeroes can be exploited in the same way as we did with rows.
     * However, the row calculation has created many nonzero AC terms, so the
     * simplification applies less often (typically 5% to 10% of the time).
     * On machines with very fast multiplication, it's possible that the
     * test takes more time than it's worth.  In that case this section
     * may be commented out.
     */

#ifndef NO_ZERO_COLUMN_TEST
    if ((dataptr[DCTSIZE*1] | dataptr[DCTSIZE*2] | dataptr[DCTSIZE*3] |
         dataptr[DCTSIZE*4] | dataptr[DCTSIZE*5] | dataptr[DCTSIZE*6] |
         dataptr[DCTSIZE*7]) == 0) {
      /* AC terms all zero */
      DCTELEM dcval = (DCTELEM) DESCALE((INT32) dataptr[0], PASS1_BITS+3);
      
      dataptr[DCTSIZE*0] = dcval;
      dataptr[DCTSIZE*1] = dcval;
      dataptr[DCTSIZE*2] = dcval;
      dataptr[DCTSIZE*3] = dcval;
      dataptr[DCTSIZE*4] = dcval;
      dataptr[DCTSIZE*5] = dcval;
      dataptr[DCTSIZE*6] = dcval;
      dataptr[DCTSIZE*7] = dcval;
      
      dataptr++;                /* advance pointer to next column */
      continue;
    }
#endif

    /* Even part: reverse the even part of the forward DCT. */
    /* The rotator is sqrt(2)*c(-6). */

    z2 = (INT32) dataptr[DCTSIZE*2];
    z3 = (INT32) dataptr[DCTSIZE*6];

    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = ((INT32) dataptr[DCTSIZE*0] + (INT32) dataptr[DCTSIZE*4]) << CONST_BITS;
    tmp1 = ((INT32) dataptr[DCTSIZE*0] - (INT32) dataptr[DCTSIZE*4]) << CONST_BITS;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;
    
    /* Odd part per figure 8; the matrix is unitary and hence its
     * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
     */

    tmp0 = (INT32) dataptr[DCTSIZE*7];
    tmp1 = (INT32) dataptr[DCTSIZE*5];
    tmp2 = (INT32) dataptr[DCTSIZE*3];
    tmp3 = (INT32) dataptr[DCTSIZE*1];

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */
    
    tmp0 = MULTIPLY(tmp0, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
    z1 = MULTIPLY(z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
    z2 = MULTIPLY(z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
    z3 = MULTIPLY(z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
    z4 = MULTIPLY(z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */
    
    z3 += z5;
    z4 += z5;
    
    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */

    dataptr[DCTSIZE*0] = (DCTELEM) DESCALE(tmp10 + tmp3,
                                           CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*7] = (DCTELEM) DESCALE(tmp10 - tmp3,
                                           CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*1] = (DCTELEM) DESCALE(tmp11 + tmp2,
                                           CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*6] = (DCTELEM) DESCALE(tmp11 - tmp2,
                                           CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*2] = (DCTELEM) DESCALE(tmp12 + tmp1,
                                           CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*5] = (DCTELEM) DESCALE(tmp12 - tmp1,
                                           CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*3] = (DCTELEM) DESCALE(tmp13 + tmp0,
                                           CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*4] = (DCTELEM) DESCALE(tmp13 - tmp0,
                                           CONST_BITS+PASS1_BITS+3);
    
    dataptr++;                  /* advance pointer to next column */
  }
}

/*
 * Perform inverse DCT on each block in an MCU row's worth of data;
 * output the results into a sample array starting at row start_row.
 * NB: start_row can only be nonzero when dealing with a single-component
 * scan; otherwise we'd have to pass different offsets for different
 * components, since the heights of interleaved MCU rows can vary.
 * But the pipeline controller logic is such that this is not necessary.
 */
static void
reverse_DCT(JBLOCKIMAGE coeff_data, JSAMPIMAGE output_data, int start_row)
{
  DCTBLOCK block;
  JBLOCKROW browptr;
  JSAMPARRAY srowptr;
  jpeg_component_info * compptr;
  long blocksperrow, bi;
  short numrows, ri;
  short ci;

  for (ci = 0; ci < comps_in_scan; ci++) {
    compptr = cur_comp_info[ci];
    /* don't bother to IDCT an uninteresting component */
    if (! compptr->component_needed)
      continue;
    /* calculate size of an MCU row in this component */
    blocksperrow = compptr->downsampled_width / DCTSIZE;
    numrows = compptr->MCU_height;
    /* iterate through all blocks in MCU row */
    for (ri = 0; ri < numrows; ri++) {
      browptr = coeff_data[ci][ri];
      srowptr = output_data[ci] + (ri * DCTSIZE + start_row);
      for (bi = 0; bi < blocksperrow; bi++) {
        /* copy the data into a local DCTBLOCK.  This allows for change of
         * representation (if DCTELEM != JCOEF).  On 80x86 machines it also
         * brings the data back from FAR storage to NEAR storage.
         */
        { JCOEFPTR elemptr = browptr[bi];
          DCTELEM *localblkptr = block;
          int elem = DCTSIZE2;

          while (--elem >= 0)
            *localblkptr++ = (DCTELEM) *elemptr++;
        }

        j_rev_dct(block);       /* perform inverse DCT */

        /* Output the data into the sample array.
         * Note change from signed to unsigned representation:
         * DCT calculation works with values +-CENTERJSAMPLE,
         * but sample arrays always hold 0..MAXJSAMPLE.
         * We have to do range-limiting because of quantization errors in the
         * DCT/IDCT phase.  We use the sample_range_limit[] table to do this
         * quickly; the CENTERJSAMPLE offset is folded into table indexing.
         */
        { JSAMPROW elemptr;
          DCTELEM *localblkptr = block;
          JSAMPLE *range_limit = sample_range_limit + CENTERJSAMPLE;
#if DCTSIZE != 8
          int elemc;
#endif
          int elemr;

          for (elemr = 0; elemr < DCTSIZE; elemr++) {
            elemptr = srowptr[elemr] + (bi * DCTSIZE);
#if DCTSIZE == 8                /* unroll the inner loop */
            *elemptr++ = range_limit[*localblkptr++];
            *elemptr++ = range_limit[*localblkptr++];
            *elemptr++ = range_limit[*localblkptr++];
            *elemptr++ = range_limit[*localblkptr++];
            *elemptr++ = range_limit[*localblkptr++];
            *elemptr++ = range_limit[*localblkptr++];
            *elemptr++ = range_limit[*localblkptr++];
            *elemptr++ = range_limit[*localblkptr++];
#else
            for (elemc = DCTSIZE; elemc > 0; elemc--) {
              *elemptr++ = range_limit[*localblkptr++];
            }
#endif
          }
        }
      }
    }
  }
}

static void read_scan_trailer(void) { }

static void
process_comment(long comment_length)
{
   while (comment_length-- > 0)
      (void)JGETC();
}

/* Routines to parse JPEG markers & save away the useful info.  */
static long
get_2bytes(void)
{
  INT32 a;
  a = JGETC();
  return (a << 8) + JGETC();
}

static void
get_dht()
/* Process a DHT marker */
{
  INT32 length;
  UINT8 bits[17];
  UINT8 huffval[256];
  int i, index, count;
  HUFF_TBL **htblptr;
  
  length = get_2bytes() - 2;
  
  while (length > 0) {
    index = JGETC();
    bits[0] = 0;
    count = 0;
    for (i = 1; i <= 16; i++) {
      bits[i] = (UINT8)JGETC();
      count += bits[i];
    }

    length -= 1 + 16;

    if (count > 256 || ((INT32) count) > length)
      JPEG_error(Bad_JPEG);

    for (i = 0; i < count; i++)
      huffval[i] = (UINT8)JGETC();

    length -= count;

    if (index & 0x10) {
      /* AC table definition */
      index -= 0x10;
      htblptr = &ac_huff_tbl_ptrs[index];
      }
   else
      /* DC table definition */
      htblptr = &dc_huff_tbl_ptrs[index];

    if (index < 0 || index >= NUM_HUFF_TBLS)
      JPEG_error(Bad_JPEG);

    if (*htblptr == NULL)
      *htblptr = (HUFF_TBL *)alloc_small(sizeof(HUFF_TBL));
  
    memcpy((*htblptr)->bits, bits, sizeof((*htblptr)->bits));
    memcpy((*htblptr)->huffval, huffval, sizeof((*htblptr)->huffval));
    }
}

static void
get_dac(void)
/* Process a DAC marker */
{
  INT32 length;
  int index, val;

  length = get_2bytes() - 2;
  
  while (length > 0) {
    index = JGETC();
    val = JGETC();

    if (index < 0 || index >= (2*NUM_ARITH_TBLS))
      JPEG_error(Bad_JPEG);

    if (index >= NUM_ARITH_TBLS)
      /* define AC table */
      arith_ac_K[index-NUM_ARITH_TBLS] = (UINT8) val;
    else {
      /* define DC table */
      arith_dc_L[index] = (UINT8) (val & 0x0F);
      arith_dc_U[index] = (UINT8) (val >> 4);
      if (arith_dc_L[index] > arith_dc_U[index])
        JPEG_error(Bad_JPEG);
      }

    length -= 2;
  }
}


static void
get_dqt(void)
/* Process a DQT marker */
{
  INT32 length;
  int n, i, prec;
  UINT16 tmp;
  QUANT_TBL_PTR quant_ptr;
  
  length = get_2bytes() - 2;
  
  while (length > 0) {
    n = JGETC();
    prec = n >> 4;
    n &= 0x0F;

    if (n >= NUM_QUANT_TBLS)
      JPEG_error(Bad_JPEG);
      
    if (quant_tbl_ptrs[n] == NULL)
      quant_tbl_ptrs[n] = (QUANT_TBL_PTR)alloc_small(sizeof(QUANT_TBL));
    quant_ptr = quant_tbl_ptrs[n];

    for (i = 0; i < DCTSIZE2; i++) {
      tmp = JGETC();
      if (prec)
        tmp = (tmp<<8) + JGETC();
      quant_ptr[i] = tmp;
      }

    length -= DCTSIZE2+1;
    if (prec) length -= DCTSIZE2;
  }
}

static void
get_dri(void)
/* Process a DRI marker */
{
  if (get_2bytes() != 4)
    JPEG_error(Bad_JPEG);
  restart_interval = (UINT16)get_2bytes();
}

static void
get_app0(void)
/* Process an APP0 marker */
{
#define JFIF_LEN 14
  INT32 length;
  UINT8 b[JFIF_LEN];
  int buffp;

  length = get_2bytes() - 2;

  /* See if a JFIF APP0 marker is present */

  if (length >= JFIF_LEN) {
    for (buffp = 0; buffp < JFIF_LEN; buffp++)
      b[buffp] = (UINT8)JGETC();
    length -= JFIF_LEN;

    if (b[0]==0x4A && b[1]==0x46 && b[2]==0x49 && b[3]==0x46 && b[4]==0) {
      /* Found JFIF APP0 marker: check version */
      /* Major version must be 1 */
      if (b[5] != 1)
        JPEG_error(Bad_JPEG);
      /* Save info */
      density_unit = b[7];
      X_density = (b[8] << 8) + b[9];
      Y_density = (b[10] << 8) + b[11];
      /* Assume colorspace is YCbCr, unless UI has overridden me */
      if (jpeg_color_space == CS_UNKNOWN)
        jpeg_color_space = CS_YCbCr;
    }
  }

  while (--length >= 0)         /* skip any remaining data */
    (void)JGETC();
}

static void
get_com(void)
/* Process a COM marker */
/* Actually we just pass this off to an application-supplied routine */
{
  INT32 length;
  
  length = get_2bytes() - 2;
  
  process_comment(length);
}

static void
skip_variable()
{
  INT32 length;
  
  length = get_2bytes();
  for (length -= 2; length > 0; length--)
    (void)JGETC();
}

static int
process_tables(void)
/* Scan and process JPEG markers that can appear in any order */
/* Return when an SOI, EOI, SOFn, or SOS is found */
{
  int c;

  for (;;) {
    c = next_marker();
      
    switch (c) {
    case M_SOF0:
    case M_SOF1:
    case M_SOF2:
    case M_SOF3:
    case M_SOF5:
    case M_SOF6:
    case M_SOF7:
    case M_JPG:
    case M_SOF9:
    case M_SOF10:
    case M_SOF11:
    case M_SOF13:
    case M_SOF14:
    case M_SOF15:
    case M_SOI:
    case M_EOI:
    case M_SOS:
      return c;
      
    case M_DHT:
      get_dht();
      break;
      
    case M_DAC:
      get_dac();
      break;
      
    case M_DQT:
      get_dqt();
      break;
      
    case M_DRI:
      get_dri();
      break;
      
    case M_APP0:
      get_app0();
      break;
      
    case M_COM:
      get_com();
      break;

    case M_RST0:                /* these are all parameterless */
    case M_RST1:
    case M_RST2:
    case M_RST3:
    case M_RST4:
    case M_RST5:
    case M_RST6:
    case M_RST7:
    case M_TEM:
      break;

    default:    /* must be DNL, DHP, EXP, APPn, JPGn, or RESn */
      skip_variable();
      break;
    }
  }
}

static void
get_sos(void)
/* Process a SOS marker */
{
  INT32 length;
  int i, ci, n, c, cc;
  jpeg_component_info *compptr;
  
  length = get_2bytes();
  
  n = JGETC();  /* Number of components */
  comps_in_scan = n;
  length -= 3;
  
  if (length != (n * 2 + 3) || n < 1 || n > MAX_COMPS_IN_SCAN)
    JPEG_error(Bad_JPEG);
  
  for (i = 0; i < n; i++) {
    cc = JGETC();
    c = JGETC();
    length -= 2;
    
    for (ci = 0; ci < num_components; ci++)
      if (cc == comp_info[ci].component_id)
        break;
    
    if (ci >= num_components)
      JPEG_error(Bad_JPEG);
    
    compptr = &comp_info[ci];
    cur_comp_info[i] = compptr;
    compptr->dc_tbl_no = (c >> 4) & 15;
    compptr->ac_tbl_no = (c     ) & 15;
    }
  
  while (length > 0) {
    (void)JGETC();
    length--;
    }
}

/* Read the start of a scan (everything through the SOS marker).
   Return TRUE if find SOS, FALSE if find EOI.  */
static boolean
read_scan_header(void)
{
  int c;
  
  /* Process markers until SOS or EOI */
  c = process_tables();
  
  switch (c) {
  case M_SOS:
    get_sos();
    return TRUE;
    
  case M_EOI:
    return FALSE;

  default:
    JPEG_error(Bad_JPEG);
    break;
  }
  return FALSE;                 /* keeps lint happy */
}

/*
 * Decompression pipeline controller used for single-scan files
 * without 2-pass color quantization.
 */

static void
simple_dcontroller(void)
{
  long fullsize_width;          /* # of samples per row in full-size buffers */
  long cur_mcu_row;             /* counts # of MCU rows processed */
  long pixel_rows_output;       /* # of pixel rows actually emitted */
  int mcu_rows_per_loop;        /* # of MCU rows processed per outer loop */
  /* Work buffer for dequantized coefficients (IDCT input) */
  JBLOCKIMAGE coeff_data;
  /* Work buffer for cross-block smoothing input */

  /* Work buffer for downsampled image data (see comments at head of file) */
  JSAMPIMAGE sampled_data[2];
  /* Work buffer for upsampled data */
  JSAMPIMAGE fullsize_data;
  int whichss, ri;
  short i;

  /* Compute dimensions of full-size pixel buffers */
  /* Note these are the same whether interleaved or not. */
  rows_in_mem = max_v_samp_factor * DCTSIZE;
  fullsize_width = jround_up(image_width, (long)(max_h_samp_factor * DCTSIZE)); 

  /* Prepare for single scan containing all components */
  if (comps_in_scan == 1) {
    noninterleaved_scan_setup();
    /* Need to read Vk MCU rows to obtain Vk block rows */
    mcu_rows_per_loop = cur_comp_info[0]->v_samp_factor;
    }
  else {
    interleaved_scan_setup();
    /* in an interleaved scan, one MCU row provides Vk block rows */
    mcu_rows_per_loop = 1;
    }

  /* Allocate working memory: */
  prepare_range_limit_table();
  /* coeff_data holds a single MCU row of coefficient blocks */
  coeff_data = alloc_MCU_row();

  /* sampled_data is sample data before upsampling */
  alloc_sampling_buffer(sampled_data);
  /* fullsize_data is sample data after upsampling */
  fullsize_data = alloc_sampimage((int)num_components,
                                  (long) rows_in_mem, fullsize_width);
  /* output_workspace is the color-processed data */
  output_workspace = alloc_sampimage((int)color_out_comps,
                                     (long) rows_in_mem, fullsize_width);

  /* Tell the memory manager to instantiate big arrays.
   * We don't need any big arrays in this controller,
   * but some other module (like the output file writer) may need one.
   */
  alloc_big_arrays((long) 0,                    /* no more small sarrays */
                   (long) 0,                    /* no more small barrays */
                   (long) 0);                   /* no more "medium" objects */

  /* Initialize to read scan data */

  decoder_init();
  upsample_init();
  disassemble_init();

  /* Loop over scan's data: rows_in_mem pixel rows are processed per loop */

  pixel_rows_output = 0;
  whichss = 1;                  /* arrange to start with sampled_data[0] */

  for (cur_mcu_row = 0; cur_mcu_row < MCU_rows_in_scan;
       cur_mcu_row += mcu_rows_per_loop) {
    whichss ^= 1;               /* switch to other downsampled-data buffer */

    /* Obtain v_samp_factor block rows of each component in the scan. */
    /* This is a single MCU row if interleaved, multiple MCU rows if not. */
    /* In the noninterleaved case there might be fewer than v_samp_factor */
    /* block rows remaining; if so, pad with copies of the last pixel row */
    /* so that upsampling doesn't have to treat it as a special case. */

    for (ri = 0; ri < mcu_rows_per_loop; ri++) {
      if (cur_mcu_row + ri < MCU_rows_in_scan) {
        /* OK to actually read an MCU row. */
        disassemble_MCU(coeff_data);
      
        reverse_DCT(coeff_data, sampled_data[whichss], ri * DCTSIZE);
        }
      else {
        /* Need to pad out with copies of the last downsampled row. */
        /* This can only happen if there is just one component. */
        duplicate_row(sampled_data[whichss][0],
                      cur_comp_info[0]->downsampled_width,
                      ri * DCTSIZE - 1, DCTSIZE);
      }
    }

    /* Upsample the data */
    /* First time through is a special case */

    if (cur_mcu_row) {
      /* Expand last row group of previous set */
      expand(sampled_data[whichss], fullsize_data, fullsize_width,
             (short) DCTSIZE, (short) (DCTSIZE+1), (short) 0,
             (short) (DCTSIZE-1));
      /* and dump the previous set's expanded data */
      emit_1pass (rows_in_mem, fullsize_data, (JSAMPARRAY) NULL);
      pixel_rows_output += rows_in_mem;
      /* Expand first row group of this set */
      expand(sampled_data[whichss], fullsize_data, fullsize_width,
             (short) (DCTSIZE+1), (short) 0, (short) 1,
             (short) 0);
    } else {
      /* Expand first row group with dummy above-context */
      expand(sampled_data[whichss], fullsize_data, fullsize_width,
             (short) (-1), (short) 0, (short) 1,
             (short) 0);
    }
    /* Expand second through next-to-last row groups of this set */
    for (i = 1; i <= DCTSIZE-2; i++) {
      expand(sampled_data[whichss], fullsize_data, fullsize_width,
             (short) (i-1), (short) i, (short) (i+1),
             (short) i);
    }
  } /* end of outer loop */

  /* Expand the last row group with dummy below-context */
  /* Note whichss points to last buffer side used */
  expand(sampled_data[whichss], fullsize_data, fullsize_width,
         (short) (DCTSIZE-2), (short) (DCTSIZE-1), (short) (-1),
         (short) (DCTSIZE-1));
  /* and dump the remaining data (may be less than full height) */
  emit_1pass((int)(image_height - pixel_rows_output),
              fullsize_data, (JSAMPARRAY) NULL);

  /* Clean up after the scan */
  disassemble_term();
  upsample_term();
  decoder_term();
  read_scan_trailer();

  /* Verify that we've seen the whole input file */
  if (read_scan_header())
    warning(Corrupt_JPEG);

  /* Release working memory */
  /* (no work -- we let free_all release what's needful) */
}

/*
 * Decompression pipeline controller used for multiple-scan files
 * and/or 2-pass color quantization.
 *
 * The current implementation places the "big" buffer at the stage of
 * upsampled, non-color-processed data.  This is the only place that
 * makes sense when doing 2-pass quantization.  For processing multiple-scan
 * files without 2-pass quantization, it would be possible to develop another
 * controller that buffers the downsampled data instead, thus reducing the size
 * of the temp files (by about a factor of 2 in typical cases).  However,
 * our present upsampling logic is dependent on the assumption that
 * upsampling occurs during a scan, so it's much easier to do the
 * enlargement as the JPEG file is read.  This also simplifies life for the
 * memory manager, which would otherwise have to deal with overlapping
 * access_big_sarray() requests.
 * At present it appears that most JPEG files will be single-scan,
 * so it doesn't seem worthwhile to worry about this optimization.
 */
#ifdef NEED_COMPLEX_CONTROLLER

static void
complex_dcontroller()
{
  long fullsize_width;          /* # of samples per row in full-size buffers */
  long cur_mcu_row;             /* counts # of MCU rows processed */
  long pixel_rows_output;       /* # of pixel rows actually emitted */
  int mcu_rows_per_loop;        /* # of MCU rows processed per outer loop */
  /* Work buffer for dequantized coefficients (IDCT input) */
  JBLOCKIMAGE coeff_data;

  /* Work buffer for downsampled image data (see comments at head of file) */
  JSAMPIMAGE sampled_data[2];
  int whichss, ri;
  short ci, i;
  boolean single_scan;

  /* Compute dimensions of full-size pixel buffers */
  /* Note these are the same whether interleaved or not. */
  rows_in_mem = max_v_samp_factor * DCTSIZE;
  fullsize_width = jround_up(image_width,
                             (long) (max_h_samp_factor * DCTSIZE));

  /* Allocate all working memory that doesn't depend on scan info */
  prepare_range_limit_table();
  /* output_workspace is the color-processed data */
  output_workspace = alloc_sampimage((int)color_out_comps,
                                     (long) rows_in_mem, fullsize_width);

  /* Get a big image: fullsize_image is sample data after upsampling. */
  fullsize_image = (big_sarray_ptr *)alloc_small
                        (num_components * sizeof(big_sarray_ptr));
  for (ci = 0; ci < num_components; ci++)
    fullsize_image[ci] = request_big_sarray(fullsize_width,
                            jround_up(image_height, (long) rows_in_mem),
                            (long)rows_in_mem);

  /* Also get an area for pointers to currently accessible chunks */
  fullsize_ptrs = (JSAMPIMAGE)alloc_small
                                (num_components * sizeof(JSAMPARRAY));

  /* Tell the memory manager to instantiate big arrays */
  alloc_big_arrays((long) (fullsize_width       /* max width in samples */
         * max_v_samp_factor*(DCTSIZE+2)        /* max height */
         * num_components),             /* max components per scan */
         /* extra barray space is for MCU-row buffers: */
         (long)(fullsize_width / DCTSIZE)       /* max width in blocks */
         * max_v_samp_factor            /* max height */
         * num_components,              /* max components per scan */
         /* no extra "medium"-object space */
         (long)0);

  /* If file is single-scan, we can do color quantization prescan on-the-fly
   * during the scan (we must be doing 2-pass quantization, else this method
   * would not have been selected).  If it is multiple scans, we have to make
   * a separate pass after we've collected all the components.  (We could save
   * some I/O by doing CQ prescan during the last scan, but the extra logic
   * doesn't seem worth the trouble.)
   */

  single_scan = (comps_in_scan == num_components);

  /* Loop over scans in file */
  do {
    
    /* Prepare for this scan */
    if (comps_in_scan == 1) {
      noninterleaved_scan_setup();
      /* Need to read Vk MCU rows to obtain Vk block rows */
      mcu_rows_per_loop = cur_comp_info[0]->v_samp_factor;
    } else {
      interleaved_scan_setup();
      /* in an interleaved scan, one MCU row provides Vk block rows */
      mcu_rows_per_loop = 1;
    }
    
    /* Allocate scan-local working memory */
    /* coeff_data holds a single MCU row of coefficient blocks */
    coeff_data = alloc_MCU_row();

    /* sampled_data is sample data before upsampling */
    alloc_sampling_buffer(sampled_data);

    /* line up the big buffers for components in this scan */
    for (ci = 0; ci < comps_in_scan; ci++)
      fullsize_ptrs[ci] = access_big_sarray(
                             fullsize_image[cur_comp_info[ci]->component_index],
                             (long) 0, TRUE);
    
    /* Initialize to read scan data */
    
    decoder_init();
    upsample_init();
    disassemble_init();
    
    /* Loop over scan's data: rows_in_mem pixel rows are processed per loop */
    
    pixel_rows_output = 0;
    whichss = 1;                /* arrange to start with sampled_data[0] */
    
    for (cur_mcu_row = 0; cur_mcu_row < MCU_rows_in_scan;
         cur_mcu_row += mcu_rows_per_loop) {
      whichss ^= 1;             /* switch to other downsampled-data buffer */

      /* Obtain v_samp_factor block rows of each component in the scan. */
      /* This is a single MCU row if interleaved, multiple MCU rows if not. */
      /* In the noninterleaved case there might be fewer than v_samp_factor */
      /* block rows remaining; if so, pad with copies of the last pixel row */
      /* so that upsampling doesn't have to treat it as a special case. */
      
      for (ri = 0; ri < mcu_rows_per_loop; ri++) {
        if (cur_mcu_row + ri < MCU_rows_in_scan) {
          /* OK to actually read an MCU row. */
          disassemble_MCU(coeff_data);
          
          reverse_DCT(coeff_data, sampled_data[whichss], ri * DCTSIZE);
          }
        else {
          /* Need to pad out with copies of the last downsampled row. */
          /* This can only happen if there is just one component. */
          duplicate_row(sampled_data[whichss][0],
                        cur_comp_info[0]->downsampled_width,
                        ri * DCTSIZE - 1, DCTSIZE);
        }
      }
      
      /* Upsample the data */
      /* First time through is a special case */
      
      if (cur_mcu_row) {
        /* Expand last row group of previous set */
        expand(sampled_data[whichss], fullsize_ptrs, fullsize_width,
               (short) DCTSIZE, (short) (DCTSIZE+1), (short) 0,
               (short) (DCTSIZE-1));

        /* Realign the big buffers */
        pixel_rows_output += rows_in_mem;
        for (ci = 0; ci < comps_in_scan; ci++) {
          fullsize_ptrs[ci] =
           access_big_sarray(fullsize_image[cur_comp_info[ci]->component_index],
                             pixel_rows_output, TRUE);
        }
        /* Expand first row group of this set */
        expand(sampled_data[whichss], fullsize_ptrs, fullsize_width,
               (short) (DCTSIZE+1), (short) 0, (short) 1,
               (short) 0);
      } else {
        /* Expand first row group with dummy above-context */
        expand(sampled_data[whichss], fullsize_ptrs, fullsize_width,
               (short) (-1), (short) 0, (short) 1,
               (short) 0);
      }
      /* Expand second through next-to-last row groups of this set */
      for (i = 1; i <= DCTSIZE-2; i++) {
        expand(sampled_data[whichss], fullsize_ptrs, fullsize_width,
               (short) (i-1), (short) i, (short) (i+1),
               (short) i);
      }
    } /* end of loop over scan's data */
    
    /* Expand the last row group with dummy below-context */
    /* Note whichss points to last buffer side used */
    expand(sampled_data[whichss], fullsize_ptrs, fullsize_width,
           (short) (DCTSIZE-2), (short) (DCTSIZE-1), (short) (-1),
           (short) (DCTSIZE-1));
    
    /* Clean up after the scan */
    disassemble_term();
    upsample_term();
    decoder_term();
    read_scan_trailer();

    /* Release scan-local working memory */
    free_MCU_row(coeff_data);
    free_sampling_buffer(sampled_data);
    
    /* Repeat if there is another scan */
  } while ((!single_scan) && read_scan_header());

  if (single_scan) {
    /* If we expected just one scan, make SURE there's just one */
    if (read_scan_header())
      warning(Corrupt_JPEG);
    /* We did the CQ prescan on-the-fly, so we are all set. */
  }

  /* Now that we've collected the data, do color processing and output */
  scan_big_image(emit_1pass);

  /* Release working memory */
  /* (no work -- we let free_all release what's needful) */
}

#endif /* NEED_COMPLEX_CONTROLLER */

/*
 * The method selection routine for decompression pipeline controllers.
 * Note that at this point we've already read the JPEG header and first SOS,
 * so we can tell whether the input is one scan or not.
 */
static void
jseldpipeline(void)
{
  if (comps_in_scan == num_components)
    d_pipeline_controller = simple_dcontroller;
  else
    d_pipeline_controller = complex_dcontroller;
}

static void
get_sof(void)
/* Process a SOFn marker */
{
  INT32 length;
  short ci;
  int c;
  jpeg_component_info * compptr;
  
  length = get_2bytes();
  
  data_precision = JGETC();
  image_height   = get_2bytes();
  image_width    = get_2bytes();
  num_components = JGETC();

  /* We don't support files in which the image height is initially specified */
  /* as 0 and is later redefined by DNL.  As long as we have to check that,  */
  /* might as well have a general sanity check. */
  if (image_height <= 0 || image_width <= 0 || num_components <= 0)
    JPEG_error(Bad_JPEG);

  if (data_precision != 8)
    JPEG_error(Bad_JPEG);

  if (length != (num_components * 3 + 8))
    JPEG_error(Bad_JPEG);

  comp_info = (jpeg_component_info *)
              alloc_small(num_components * sizeof(jpeg_component_info));
  
  for (ci = 0; ci < num_components; ci++) {
    compptr = &comp_info[ci];
    compptr->component_index = ci;
    compptr->component_id = JGETC();
    c = JGETC();
    compptr->h_samp_factor = (c >> 4) & 15;
    compptr->v_samp_factor = (c     ) & 15;
    compptr->quant_tbl_no  = JGETC();
    compptr->component_needed = TRUE; /* assume all components are wanted */
    }
}

static void
get_soi(void)
/* Process an SOI marker */
{
  int i;
  
  /* Reset all parameters that are defined to be reset by SOI */
  for (i = 0; i < NUM_ARITH_TBLS; i++) {
    arith_dc_L[i] = 0;
    arith_dc_U[i] = 1;
    arith_ac_K[i] = 5;
    }
  restart_interval = 0;

  density_unit = 0;     /* set default JFIF APP0 values */
  X_density = 1;
  Y_density = 1;
}

/* Initialize and read the file header (everything through the SOF marker). */
static void
read_file_header(void)
{
  int c;

  /* Demand an SOI marker at the start of the file --- otherwise it's
   * probably not a JPEG file at all.  If the user interface wants to support
   * nonstandard headers in front of the SOI, it must skip over them itself
   * before calling jpeg_decompress().
   */
  if (JGETC() != 0xFF  ||  JGETC() != (int) M_SOI)
     JPEG_error(Bad_JPEG);

  get_soi();            /* OK, process SOI */

  /* Process markers until SOF */
  c = process_tables();

  switch (c) {
  case M_SOF0:
  case M_SOF1:
    get_sof();
    arith_code = FALSE;
    break;
      
  case M_SOF9:
    get_sof();
    arith_code = TRUE;
    break;

  default:
    JPEG_error(Bad_JPEG);
    break;
  }

  /* Figure out what colorspace we have */
  /* (too bad the JPEG committee didn't provide a real way to specify this) */
  switch (num_components) {
  case 1:
    jpeg_color_space = CS_GRAYSCALE;
    break;

  case 3:
    /* if we saw a JFIF marker, leave it set to YCbCr; */
    /* also leave it alone if UI has provided a value */
    if (jpeg_color_space == CS_UNKNOWN) {
      short cid0 = comp_info[0].component_id;
      short cid1 = comp_info[1].component_id;
      short cid2 = comp_info[2].component_id;

      if (cid0 == 1 && cid1 == 2 && cid2 == 3)
        jpeg_color_space = CS_YCbCr; /* assume it's JFIF w/out marker */
      else if (cid0 == 1 && cid1 == 4 && cid2 == 5)
        jpeg_color_space = CS_YIQ; /* prototype's YIQ matrix */
      else
        jpeg_color_space = CS_YCbCr;
    }
    break;

  case 4:
    jpeg_color_space = CS_CMYK;
    break;

  default:
    jpeg_color_space = CS_UNKNOWN;
    break;
  }
}


/*
 * Finish up at the end of the file.
 */

static void
read_file_trailer(void)
{
  /* no work needed */
}

static void
j_d_defaults(boolean standard_buffering)
/* NB: the external methods must already be set up. */
{
  short i;

  /* Initialize pointers as needed to mark stuff unallocated. */
  /* Outer application may fill in default tables for abbreviated files... */
  comp_info = NULL;
  for (i = 0; i < NUM_QUANT_TBLS; i++)
    quant_tbl_ptrs[i] = NULL;
  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    dc_huff_tbl_ptrs[i] = NULL;
    ac_huff_tbl_ptrs[i] = NULL;
    }

  /* Default to RGB output */
  /* UI can override by changing out_color_space */
  out_color_space = CS_RGB;
  jpeg_color_space = CS_UNKNOWN;
  
  /* Allocate memory for input buffer, unless outer application provides it. */
  if (standard_buffering) {
    input_buffer_ptr = &input_buffer[0];
    bytes_in_buffer  = 0;
    }
}

#define SCALEBITS       16      /* speedier right-shift on some machines */
#define ONE_HALF        ((INT32) 1 << (SCALEBITS-1))
#define FIXX(x)         ((INT32) ((x) * (1L<<SCALEBITS) + 0.5))

static int * Cr_r_tab;          /* => table for Cr to R conversion */
static int * Cb_b_tab;          /* => table for Cb to B conversion */
static INT32 * Cr_g_tab;        /* => table for Cr to G conversion */
static INT32 * Cb_g_tab;        /* => table for Cb to G conversion */

static void
ycc_rgb_init(void)
{
  INT32 i, x2;

  Cr_r_tab = (int *)alloc_small((MAXJSAMPLE+1) * sizeof(int));
  Cb_b_tab = (int *)alloc_small((MAXJSAMPLE+1) * sizeof(int));
  Cr_g_tab = (INT32 *)alloc_small((MAXJSAMPLE+1) * sizeof(INT32));
  Cb_g_tab = (INT32 *)alloc_small((MAXJSAMPLE+1) * sizeof(INT32));

  for (i = 0; i <= MAXJSAMPLE; i++) {
    /* i is the actual input pixel value, in the range 0..MAXJSAMPLE */
    /* The Cb or Cr value we are thinking of is x = i - MAXJSAMPLE/2 */
    x2 = 2*i - MAXJSAMPLE;      /* twice x */
    /* Cr=>R value is nearest int to 1.40200 * x */
    Cr_r_tab[i] = (int)
                    RIGHT_SHIFT(FIXX(1.40200/2) * x2 + ONE_HALF, SCALEBITS);
    /* Cb=>B value is nearest int to 1.77200 * x */
    Cb_b_tab[i] = (int)
                    RIGHT_SHIFT(FIXX(1.77200/2) * x2 + ONE_HALF, SCALEBITS);
    /* Cr=>G value is scaled-up -0.71414 * x */
    Cr_g_tab[i] = (- FIXX(0.71414/2)) * x2;
    /* Cb=>G value is scaled-up -0.34414 * x */
    /* We also add in ONE_HALF so that need not do it in inner loop */
    Cb_g_tab[i] = (- FIXX(0.34414/2)) * x2 + ONE_HALF;
  }
}

/* Convert some rows of samples to the output colorspace.  */
static void
ycc_rgb_convert(int num_rows, long num_cols,
                JSAMPIMAGE input_data, JSAMPIMAGE output_data)
{
  int y, cb, cr;
  JSAMPROW inptr0, inptr1, inptr2;
  JSAMPROW outptr0, outptr1, outptr2;
  long col;
  /* copy these pointers into registers if possible */
  JSAMPLE * range_limit = sample_range_limit;
  int *Crrtab = Cr_r_tab;
  int *Cbbtab = Cb_b_tab;
  INT32 *Crgtab = Cr_g_tab;
  INT32 *Cbgtab = Cb_g_tab;
  int row;
  
  for (row = 0; row < num_rows; row++) {
    inptr0 = input_data[0][row];
    inptr1 = input_data[1][row];
    inptr2 = input_data[2][row];
    outptr0 = output_data[0][row];
    outptr1 = output_data[1][row];
    outptr2 = output_data[2][row];
    for (col = 0; col < num_cols; col++) {
      y  = GETJSAMPLE(inptr0[col]);
      cb = GETJSAMPLE(inptr1[col]);
      cr = GETJSAMPLE(inptr2[col]);
      /* Note: if the inputs were computed directly from RGB values,
       * range-limiting would be unnecessary here; but due to possible
       * noise in the DCT/IDCT phase, we do need to apply range limits.
       */
      outptr0[col] = range_limit[y + Crrtab[cr]];       /* red */
      outptr1[col] = range_limit[y +                    /* green */
                                 ((int) RIGHT_SHIFT(Cbgtab[cb] + Crgtab[cr],
                                                    SCALEBITS))];
      outptr2[col] = range_limit[y + Cbbtab[cb]];       /* blue */
    }
  }
}

/* Finish up at the end of the file.  */
static void
ycc_rgb_term(void)
{
  free_small(Cr_r_tab);
  free_small(Cb_b_tab);
  free_small(Cr_g_tab);
  free_small(Cb_g_tab);
}

/* Initialize for colorspace conversion.  */
static void null_init(void) { }

/* Color conversion for no colorspace change: just copy the data.  */
static void
null_convert(int num_rows, long num_cols,
             JSAMPIMAGE input_data, JSAMPIMAGE output_data)
{
  short ci;

  for (ci = 0; ci < num_components; ci++) {
    jcopy_sample_rows(input_data[ci], 0, output_data[ci], 0,
                      num_rows, num_cols);
  }
}

static void null_term() { }

/* The method selection routine for output colorspace conversion. */
static void
jseldcolor(void)
{
  /* Make sure num_components agrees with jpeg_color_space */
  switch (jpeg_color_space) {
  case CS_GRAYSCALE:
    if (num_components != 1)
      JPEG_error(Bad_JPEG);
    break;

  case CS_RGB:
  case CS_YCbCr:
  case CS_YIQ:
    if (num_components != 3)
      JPEG_error(Bad_JPEG);
    break;

  case CS_CMYK:
    if (num_components != 4)
      JPEG_error(Bad_JPEG);
    break;

  default:
    JPEG_error("Unsupported JPEG color space");
    break;
  }

  /* Set color_out_comps and conversion method based on requested space. */
  /* Also clear the component_needed flags for any unused components, */
  /* so that earlier pipeline stages can avoid useless computation. */

  if (jpeg_color_space == CS_YCbCr) {
    color_out_comps = 3;
    color_convert = ycc_rgb_convert;
    colorout_init = ycc_rgb_init;
    colorout_term = ycc_rgb_term;
    }
  else if (jpeg_color_space == CS_RGB) {
    color_out_comps = 3;
    color_convert = null_convert;
    colorout_init = null_init;
    colorout_term = null_term;
    }
  else if (jpeg_color_space == CS_GRAYSCALE) {
    color_out_comps = 1;
    color_convert = null_convert;
    colorout_init = null_init;
    colorout_term = null_term;
    }
  else
    JPEG_error("Unsupported JPEG color space");
}

static void
d_initial_method_selection(void)
/* Central point for initial method selection (after reading file header) */
{
  /* JPEG file scanning method selection is already done. */
  /* So is output file format selection (both are done by user interface). */

  /* Gamma and color space conversion */
  jseldcolor();

  /* Entropy decoding: either Huffman or arithmetic coding. */
  if (arith_code)
    JPEG_error(Bad_JPEG);

  /* Pipeline control */
  jseldpipeline();
}

static void
initial_setup(void)
/* Do computations that are needed before initial method selection */
{
  short ci;
  jpeg_component_info *compptr;

  /* Compute maximum sampling factors; check factor validity */
  max_h_samp_factor = 1;
  max_v_samp_factor = 1;
  for (ci = 0; ci < num_components; ci++) {
    compptr = &comp_info[ci];
    if (compptr->h_samp_factor<=0 || compptr->h_samp_factor>MAX_SAMP_FACTOR ||
        compptr->v_samp_factor<=0 || compptr->v_samp_factor>MAX_SAMP_FACTOR)
      JPEG_error(Bad_JPEG);
    max_h_samp_factor = MAX(max_h_samp_factor, compptr->h_samp_factor);
    max_v_samp_factor = MAX(max_v_samp_factor, compptr->v_samp_factor);

  }

  /* Compute logical downsampled dimensions of components */
  for (ci = 0; ci < num_components; ci++) {
    compptr = &comp_info[ci];
    compptr->true_comp_width = (image_width * compptr->h_samp_factor
                                + max_h_samp_factor - 1)
                                / max_h_samp_factor;
    compptr->true_comp_height = (image_height * compptr->v_samp_factor
                                 + max_v_samp_factor - 1)
                                 / max_v_samp_factor;
  }
}

static void
output_init(void)
/* This routine should do any setup required */
{
   int i;
   unsigned char **imgbuf;

  /* This routine can initialize for output based on the data passed in cinfo.
   * Useful fields include:
   *    image_width, image_height       Pretty obvious, I hope.
   *    data_precision                  bits per pixel value; typically 8.
   *    out_color_space                 output colorspace previously requested
   *    color_out_comps                 number of color components in same
   */
   /* output_image->filename has already been set before getting here */
   output_image->copy     = 0;                /* This is a new image */
   output_image->psize    = color_out_comps*8;/* Bytes per pixel */
   output_image->cflag    = 0;                /* Not RLE compressed */
   output_image->width    = image_width;      /* Width in pixels */
   output_image->length   = image_height;     /* Height in pixels */
   output_image->orien    = 0x20;             /* Assume normal top to bottom */
   output_image->cmlen    = 0;                /* Assume it's not color mapped */
   output_image->cmsiz    = 0;                /* Zero length color map */
   output_image->cmap     = NULL;             /* No data in color map */

   /* Set the type of the image (matching Targa definitions) */
   if (color_out_comps == 1)
      output_image->ftype    = 3; /* Grayscale image*/
   else
      output_image->ftype    = 2; /* Raw 24 bit image */

   /* Allocate the row buffers for the image */
   imgbuf = polyray_malloc(output_image->length * sizeof(unsigned char *));
   if (imgbuf == NULL)
      JPEG_error(No_Memory);
   for (i=0;i<output_image->length;i++) {
      imgbuf[i] = polyray_malloc(output_image->width * (output_image->psize/8) *
                                 sizeof(unsigned char));
      if (imgbuf[i] == NULL)
         warning("Failed to allocate image memory\n");
      }
   output_image->image = imgbuf;           /* Image buffer, fully allocated */
   current_row = 0;                        /* Nothing read in yet */
}

static void
put_pixel_rows(int num_rows, JSAMPIMAGE pixel_data)
{
  /* This example shows how you might write full-color RGB data (3 components)
     to an output file in which the data is stored 3 bytes per pixel. */
  unsigned char *image_ptr;
  JSAMPROW ptr0, ptr1, ptr2;
  long col;
  int row;
  
   for (row = 0; row < num_rows; row++) {
      image_ptr = output_image->image[current_row + row];
      if (num_components == 1) {
         ptr0 = pixel_data[0][row];
         for (col = 0; col < image_width; col++) {
            *image_ptr = GETJSAMPLE(*ptr0); image_ptr++; ptr0++;
            }
         }
      else {
         ptr2 = pixel_data[0][row];
         ptr1 = pixel_data[1][row];
         ptr0 = pixel_data[2][row];
         for (col = 0; col < image_width; col++) {
            *image_ptr = GETJSAMPLE(*ptr0); image_ptr++; ptr0++;
            *image_ptr = GETJSAMPLE(*ptr1); image_ptr++; ptr1++;
            *image_ptr = GETJSAMPLE(*ptr2); image_ptr++; ptr2++;
            }
         }
      }
   current_row += num_rows;
}

static void
output_term(void)
{
  /* This termination routine may not need to do anything. */
  /* Note that the JPEG code will only call it during successful exit; */
  /* if you want it called during error exit, you gotta do that yourself. */
}

/* This is the main entry point to the JPEG decompressor.  */
int
read_JPEG_image(FILE *ifile, Img *img)
{
   /* Initialize memory manager variables */
   jselmemmgr();

   if (setjmp(JPEG_environ)) {
      free_all();
      return 0;
      }

   /* Set the global input file to the image file */
   fseek(ifile, 0, SEEK_SET);
   input_file = ifile;
   output_image = img;

   j_d_defaults(TRUE);

   /* Read the JPEG file header markers; everything up through the first SOS
    * marker is read now.  NOTE: the user interface must have initialized the
    * read_file_header method pointer (eg, by calling jselrjfif or jselrtiff).
    * The other file reading methods (read_scan_header etc.) were probably
    * set at the same time, but could be set up by read_file_header itself.
    */
   read_file_header();
   if (! read_scan_header())
      JPEG_error(Bad_JPEG);

   /* Give UI a chance to adjust decompression parameters and select */
   /* output file format based on info from file header. */
   if (jpeg_color_space == CS_GRAYSCALE)
     out_color_space = CS_GRAYSCALE;

   /* Now select methods for decompression steps. */
   initial_setup();
   d_initial_method_selection();

   /* Initialize the output file & other modules as needed */
   /* (modules needing per-scan init are called by pipeline controller) */

   output_init();
   colorout_init();

   /* And let the pipeline controller do the rest. */
   d_pipeline_controller();

   /* Finish output file, release working storage, etc */
   colorout_term();
   output_term();
   read_file_trailer();

   free_all();

   /* My, that was easy, wasn't it? */
   return 1;
}

/*
 * jrdgif.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains routines to read input images in GIF format.
 *
 * These routines may need modification for non-Unix environments or
 * specialized applications.  As they stand, they assume input from
 * an ordinary stdio stream.  They further assume that reading begins
 * at the start of the file; input_init may need work if the
 * user interface has already read some data (e.g., to determine that
 * the file is indeed GIF format).
 *
 * These routines are invoked via the methods get_input_row
 * and input_init/term.
 */

/*
 * This code is loosely based on giftoppm from the PBMPLUS distribution
 * of Feb. 1991.  That file contains the following copyright notice:
 * +-------------------------------------------------------------------+
 * | Copyright 1990, David Koblas.                                     |
 * |   Permission to use, copy, modify, and distribute this software   |
 * |   and its documentation for any purpose and without fee is hereby |
 * |   granted, provided that the above copyright notice appear in all |
 * |   copies and that both that copyright notice and this permission  |
 * |   notice appear in supporting documentation.  This software is    |
 * |   provided "as is" without express or implied warranty.           |
 * +-------------------------------------------------------------------+
 *
 * We are also required to state that
 *    "The Graphics Interchange Format(c) is the Copyright property of
 *    CompuServe Incorporated. GIF(sm) is a Service Mark property of
 *    CompuServe Incorporated."
 */
#define MAX_LZW_BITS    12      /* maximum LZW code size */
#define LZW_TABLE_SIZE  (1<<MAX_LZW_BITS) /* # of possible LZW symbols */

/* Macros for extracting header data --- note we assume chars may be signed */

#define LM_to_uint(a,b)         ((((b)&0xFF) << 8) | ((a)&0xFF))

#define BitSet(byte, bit)       ((byte) & (bit))
#define INTERLACE       0x40    /* mask for bit signifying interlaced image */
#define COLORMAPFLAG    0x80    /* mask for bit signifying colormap presence */

#define ReadOK(file,buffer,len) (JFREAD(file,buffer,len) == ((size_t) (len)))

/* Static vars for GetCode and LZWReadByte */

static unsigned char code_buf[256+4]; /* current input data block */
static int last_byte;           /* # of bytes in code_buf */
static int last_bit;            /* # of bits in code_buf */
static int cur_bit;             /* next bit index to read */
static boolean out_of_blocks;   /* TRUE if hit terminator data block */

static int input_code_size;     /* codesize given in GIF file */
static int clear_code,end_code; /* values for Clear and End codes */

static int code_size;           /* current actual code size */
static int limit_code;          /* 2^code_size */
static int max_code;            /* first unused code value */
static boolean first_time;      /* flags first call to LZWReadByte */

/* LZW decompression tables:
 *   symbol_head[K] = prefix symbol of any LZW symbol K (0..LZW_TABLE_SIZE-1)
 *   symbol_tail[K] = suffix byte   of any LZW symbol K (0..LZW_TABLE_SIZE-1)
 * Note that entries 0..end_code of the above tables are not used,
 * since those symbols represent raw bytes or special codes.
 *
 * The stack represents the not-yet-used expansion of the last LZW symbol.
 * In the worst case, a symbol could expand to as many bytes as there are
 * LZW symbols, so we allocate LZW_TABLE_SIZE bytes for the stack.
 * (This is conservative since that number includes the raw-byte symbols.)
 *
 * The tables are allocated from FAR heap space since they would use up
 * rather a lot of the near data space in a PC.
 */

static UINT16 *symbol_head; /* => table of prefix symbols */
static UINT8  *symbol_tail; /* => table of suffix bytes */
static UINT8  *symbol_stack; /* stack for symbol expansions */
static UINT8  *sp;          /* stack pointer */

/* Static state for interlaced image processing */

static boolean is_interlaced;   /* TRUE if have interlaced image */
static long pass2_offset;       /* # of pixel rows in pass 1 */
static long pass3_offset;       /* # of pixel rows in passes 1&2 */
static long pass4_offset;       /* # of pixel rows in passes 1,2,3 */


/* Forward declarations */
static void load_interlaced_image();

typedef void (*get_input_row_ptr)(void);
static get_input_row_ptr get_input_row;

static int
ReadByte(void)
/* Read next byte from GIF file */
{
  int c;

  if ((c = getc(input_file)) == EOF)
    JPEG_error("Premature EOF in GIF file");
  return c;
}


static int
GetDataBlock(unsigned char *buf)
/* Read a GIF data block, which has a leading count byte */
/* A zero-length block marks the end of a data block sequence */
{
  int count;

  count = ReadByte();
  if (count > 0) {
    if (! ReadOK(input_file, buf, count))
      JPEG_error("Premature EOF in GIF file");
  }
  return count;
}

static void
SkipDataBlocks(void)
/* Skip a series of data blocks, until a block terminator is found */
{
  unsigned char buf[256];

  while (GetDataBlock(buf) > 0)
    /* skip */;
}

static void
ReInitLZW(void)
/* (Re)initialize LZW state; shared code for startup and Clear processing */
{
  code_size = input_code_size+1;
  limit_code = clear_code << 1; /* 2^code_size */
  max_code = clear_code + 2;    /* first unused code value */
  sp = symbol_stack;            /* init stack to empty */
}

static void
InitLZWCode (void)
/* Initialize for a series of LZWReadByte (and hence GetCode) calls */
{
  /* GetCode initialization */
  last_byte = 2;                /* make safe to "recopy last two bytes" */
  last_bit = 0;                 /* nothing in the buffer */
  cur_bit = 0;                  /* force buffer load on first call */
  out_of_blocks = FALSE;

  /* LZWReadByte initialization */
  clear_code = 1 << input_code_size; /* compute special code values */
  end_code = clear_code + 1;    /* note that these do not change */
  first_time = TRUE;
  ReInitLZW();
}

static int
GetCode(void)
/* Fetch the next code_size bits from the GIF data */
/* We assume code_size is less than 16 */
{
  INT32 accum;
  int offs, ret, count;

  if ( (cur_bit+code_size) > last_bit) {
    /* Time to reload the buffer */
    if (out_of_blocks) {
      warning("Ran out of GIF bits");
      return end_code;          /* fake something useful */
    }
    /* preserve last two bytes of what we have -- assume code_size <= 16 */
    code_buf[0] = code_buf[last_byte-2];
    code_buf[1] = code_buf[last_byte-1];
    /* Load more bytes; set flag if we reach the terminator block */
    if ((count = GetDataBlock(&code_buf[2])) == 0) {
      out_of_blocks = TRUE;
      warning("Ran out of GIF bits");
      return end_code;          /* fake something useful */
    }
    /* Reset counters */
    cur_bit = (cur_bit - last_bit) + 16;
    last_byte = 2 + count;
    last_bit = last_byte * 8;
  }

  /* Form up next 24 bits in accum */
  offs = cur_bit >> 3;          /* byte containing cur_bit */
  accum = code_buf[offs+2];
  accum <<= 8;
  accum |= code_buf[offs+1];
  accum <<= 8;
  accum |= code_buf[offs];

  /* Right-align cur_bit in accum, then mask off desired number of bits */
  accum >>= (cur_bit & 7);
  ret = ((int) accum) & ((1 << code_size) - 1);
  
  cur_bit += code_size;
  return ret;
}

static int
LZWReadByte(void)
/* Read an LZW-compressed byte */
{
  static int oldcode;           /* previous LZW symbol */
  static int firstcode;         /* first byte of oldcode's expansion */
  register int code;            /* current working code */
  int incode;                   /* saves actual input code */

  /* First time, just eat the expected Clear code(s) and return next code, */
  /* which is expected to be a raw byte. */
  if (first_time) {
    first_time = FALSE;
    code = clear_code;          /* enables sharing code with Clear case */
  } else {

    /* If any codes are stacked from a previously read symbol, return them */
    if (sp > symbol_stack)
      return (int) *(--sp);

    /* Time to read a new symbol */
    code = GetCode();

  }

  if (code == clear_code) {
    /* Reinit static state, swallow any extra Clear codes, and */
    /* return next code, which is expected to be a raw byte. */
    ReInitLZW();
    do {
      code = GetCode();
    } while (code == clear_code);
    if (code > clear_code) {    /* make sure it is a raw byte */
      warning("Corrupt data in GIF file");
      code = 0;                 /* use something valid */
    }
    firstcode = oldcode = code; /* make firstcode, oldcode valid! */
    return code;
  }

  if (code == end_code) {
    /* Skip the rest of the image, unless GetCode already read terminator */
    if (! out_of_blocks) {
      SkipDataBlocks();
      out_of_blocks = TRUE;
    }
    /* Complain that there's not enough data */
    warning("Premature end of GIF image");
    /* Pad data with 0's */
    return 0;                   /* fake something usable */
  }

  /* Got normal raw byte or LZW symbol */
  incode = code;                /* save for a moment */
  
  if (code >= max_code) {       /* special case for not-yet-defined symbol */
    /* code == max_code is OK; anything bigger is bad data */
    if (code > max_code) {
      warning("Corrupt data in GIF file");
      incode = 0;               /* prevent creation of loops in symbol table */
    }
    *sp++ = (UINT8) firstcode;  /* it will be defined as oldcode/firstcode */
    code = oldcode;
  }

  /* If it's a symbol, expand it into the stack */
  while (code >= clear_code) {
    *sp++ = symbol_tail[code];  /* tail of symbol: a simple byte value */
    code = symbol_head[code];   /* head of symbol: another LZW symbol */
  }
  /* At this point code just represents a raw byte */
  firstcode = code;             /* save for possible future use */

  /* If there's room in table, */
  if ((code = max_code) < LZW_TABLE_SIZE) {
    /* Define a new symbol = prev sym + head of this sym's expansion */
    symbol_head[code] = oldcode;
    symbol_tail[code] = (UINT8) firstcode;
    max_code++;
    /* Is it time to increase code_size? */
    if ((max_code >= limit_code) && (code_size < MAX_LZW_BITS)) {
      code_size++;
      limit_code <<= 1;         /* keep equal to 2^code_size */
    }
  }
  
  oldcode = incode;             /* save last input symbol for future use */
  return firstcode;             /* return first byte of symbol's expansion */
}

static void
ReadColorMap(int cmaplen)
/* Read a GIF colormap */
{
   int i;
   unsigned char *cmapptr;

   /* If there was already a color map, then get rid of it. */
   if (output_image->cmap != NULL) {
      polyray_free(output_image->cmap);
      output_image->cmap = NULL;
      }

   /* If the color map is 0 length, then do nothing */
   if (cmaplen == 0) {
      output_image->cmap  = NULL;
      output_image->cmlen = 0;
      return;
      }

   /* Read in the new color map */
   cmapptr = (unsigned char *)polyray_malloc(3 * cmaplen);
   output_image->cmap = cmapptr;
   for (i = 0; i < cmaplen; i++) {
      /* Have to reorder the color samples in a GIF color map
         so that they will be BGR rather than RGB.  This is
         because all the image processing routines are based on
         the Targa color order. */
      cmapptr[2] = (JSAMPLE) ReadByte();
      cmapptr[1] = (JSAMPLE) ReadByte();
      cmapptr[0] = (JSAMPLE) ReadByte();
      cmapptr += 3;
      }
   output_image->cmlen = cmaplen;
}

static void
DoExtension(void)
/* Process an extension block */
/* Currently we ignore 'em all */
{
  int extlabel;

  /* Read extension label byte */
  extlabel = ReadByte();

  /* Skip the data block(s) associated with the extension */
  SkipDataBlocks();
}


/* Read the file header; return image size and component count. */
static void
input_GIF_init(void)
{
   char hdrbuf[10];              /* workspace for reading control blocks */
   UINT16 width, height;         /* image dimensions */
   int colormaplen, aspectRatio, c, i;
   unsigned char **imgbuf;

  /* Read and verify GIF Header */
  if (! ReadOK(input_file, hdrbuf, 6))
     longjmp(JPEG_environ, 1);

  if (hdrbuf[0] != 'G' || hdrbuf[1] != 'I' || hdrbuf[2] != 'F')
     longjmp(JPEG_environ, 1);

  /* Check for expected version numbers.
   * If unknown version, give warning and try to process anyway;
   * this is per recommendation in GIF89a standard.
   */
  if ((hdrbuf[3] != '8' || hdrbuf[4] != '7' || hdrbuf[5] != 'a') &&
      (hdrbuf[3] != '8' || hdrbuf[4] != '9' || hdrbuf[5] != 'a'))
    warning("Unexpected GIF version number '%c%c%c'",
            hdrbuf[3], hdrbuf[4], hdrbuf[5]);

  /* Read and decipher Logical Screen Descriptor */
  if (! ReadOK(input_file, hdrbuf, 7))
     longjmp(JPEG_environ, 1);

   width = LM_to_uint(hdrbuf[0],hdrbuf[1]);
   height = LM_to_uint(hdrbuf[2],hdrbuf[3]);
   colormaplen = 2 << (hdrbuf[4] & 0x07);
   output_image->cmap  = NULL;

  /* we ignore the color resolution, sort flag, and background color index */
  aspectRatio = hdrbuf[6] & 0xFF;

  /* Read global colormap if header indicates it is present */
  if (BitSet(hdrbuf[4], COLORMAPFLAG))
    ReadColorMap(colormaplen);

  /* Scan until we reach start of desired image.
   * We don't currently support skipping images, but could add it easily.
   */
  for (;;) {
    c = ReadByte();

    if (c == ';')               /* GIF terminator?? */
     longjmp(JPEG_environ, 1);

    if (c == '!') {             /* Extension */
      DoExtension();
      continue;
    }
    
    if (c != ',') {             /* Not an image separator? */
      continue;
    }

    /* Read and decipher Local Image Descriptor */
    if (! ReadOK(input_file, hdrbuf, 9))
     longjmp(JPEG_environ, 1);

    /* we ignore top/left position info, also sort flag */
    width = LM_to_uint(hdrbuf[4],hdrbuf[5]);
    height = LM_to_uint(hdrbuf[6],hdrbuf[7]);
    is_interlaced = BitSet(hdrbuf[8], INTERLACE);

    /* Read local colormap if header indicates it is present */
    /* Note: if we wanted to support skipping images, */
    /* we'd need to skip rather than read colormap for ignored images */
    if (BitSet(hdrbuf[8], COLORMAPFLAG)) {
      colormaplen = 2 << (hdrbuf[8] & 0x07);
      ReadColorMap(colormaplen);
    }

    input_code_size = ReadByte(); /* get minimum-code-size byte */
    if (input_code_size < 2 || input_code_size >= MAX_LZW_BITS)
     longjmp(JPEG_environ, 1);

    /* Reached desired image, so break out of loop */
    /* If we wanted to skip this image, */
    /* we'd call SkipDataBlocks and then continue the loop */
    break;
  }

  /* Prepare to read selected image: first initialize LZW decompressor */
  symbol_head = (UINT16 *)alloc_small(LZW_TABLE_SIZE * sizeof(UINT16));
  symbol_tail = (UINT8 *)alloc_small(LZW_TABLE_SIZE * sizeof(UINT8));
  symbol_stack = (UINT8 *)alloc_small(LZW_TABLE_SIZE * sizeof(UINT8));
  InitLZWCode();

  /*
   * If image is interlaced, we read it into a full-size sample array,
   * decompressing as we go; then get_input_row selects rows from the
   * sample array in the proper order.
   */
  if (is_interlaced)
    get_input_row = load_interlaced_image;

   /* output_image->filename has already been set before getting here */
   output_image->copy     = 0;        /* This is a new image */
   output_image->ftype    = 1;        /* Color mapped image */
   output_image->psize    = 8;        /* Bits per pixel */
   output_image->cflag    = 0;        /* Not RLE compressed */
   output_image->width    = width;    /* Width in pixels */
   output_image->length   = height;   /* Height in pixels */
   output_image->orien    = 0x20;     /* Assume normal top to bottom */
   output_image->cmsiz    = 3;        /* RGB color map */

   if (colormaplen == 0) {
      /* Turn this into a raw monochrome image */
      output_image->ftype = 3;
      output_image->cmlen = 0;
      output_image->cmap  = NULL;
      }

   /* Allocate the row buffers for the image */
   imgbuf = polyray_malloc(output_image->length * sizeof(unsigned char *));
   if (imgbuf == NULL)
      JPEG_error(No_Memory);
   for (i=0;i<output_image->length;i++) {
      imgbuf[i] = polyray_malloc(output_image->width * (output_image->psize/8) *
                                 sizeof(unsigned char));
      if (imgbuf[i] == NULL)
         JPEG_error(No_Memory);
      }
   output_image->image = imgbuf;           /* Image buffer, fully allocated */
   current_row = 0;                        /* Nothing read in yet */
}

/* Read one row of pixels. This version is used for noninterlaced
  GIF images, we read directly from the GIF file. */
static void
get_noninterlaced_row(void)
{
  long col;
  unsigned char *row_ptr;

   row_ptr = output_image->image[current_row];
   for (col = output_image->width; col > 0; col--)
      *row_ptr++ = LZWReadByte();
   current_row++;
}


/* Read one row of pixels. This version is used for interlaced GIF
  images, we read from the big in-memory image. */
static void
get_interlaced_row(void)
{
  long col, irow;

  /* Figure out which row of interlaced image is needed, and access it. */
  switch ((int) (current_row & 7L)) {
  case 0:                       /* first-pass row */
    irow = current_row >> 3;
    break;
  case 4:                       /* second-pass row */
    irow = (current_row >> 3) + pass2_offset;
    break;
  case 2:                       /* third-pass row */
  case 6:
    irow = (current_row >> 2) + pass3_offset;
    break;
  default:                      /* fourth-pass row */
    irow = (current_row >> 1) + pass4_offset;
    break;
  }

  /* Scan the row, expand colormap, and output */
  for (col = output_image->width; col > 0; col--)
      output_image->image[irow][col] = LZWReadByte();

  current_row++;             /* for next time */
}

/*
 * Read one row of pixels.
 * This version is used for the first call on get_input_row when
 * reading an interlaced GIF file: we read the whole image into memory.
 */

static void
load_interlaced_image(void)
{
  /* Replace method pointer so subsequent calls don't come here. */
  get_input_row = get_interlaced_row;

  /* Initialize for get_interlaced_row, and perform first call on it. */
  current_row = 0;
  pass2_offset = (output_image->length + 7L) / 8L;
  pass3_offset = pass2_offset + (output_image->length + 3L) / 8L;
  pass4_offset = pass3_offset + (output_image->length + 1L) / 4L;

  get_interlaced_row();
}


/* Finish up at the end of the file. */
static void input_GIF_term(void) { }

/* This is the main entry point to the GIF decompressor.  */
int
read_GIF_image(FILE *ifile, Img *img)
{
   int i;

   /* Initialize memory manager variables */
   jselmemmgr();

   if (setjmp(JPEG_environ)) {
      free_all();
      return 0;
      }
 
   /* Set the global input file to the image file */
   fseek(ifile, 0, SEEK_SET);
   input_file = ifile;
   output_image = img;

   /* assume uninterlaced */
   get_input_row = get_noninterlaced_row;

   /* Read the file header information */
   input_GIF_init();

   /* Load the GIF image into output_image */
   for (i=0;i<output_image->length;i++)
      get_input_row();

   input_GIF_term();

   /* Terminate memory manager variables */
   free_all();

   return 1;
}

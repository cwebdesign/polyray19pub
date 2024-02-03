/*
 * errors.h
 *
 * Error codes and messages for PNG to TIFF converter.
 *
 **********
 *
 * HISTORY
 *
 * 95-03-10 Created by Lee Daniel Crocker <lee@piclab.com>
 *          <URL:http://www.piclab.com/piclab/index.html>
 */

#ifndef DEFINE_ENUMS
#  ifndef DEFINE_STRINGS
#    error "errors.h included without definitions selected."
#  endif

#  define ASSOCIATE(id,str) str,

char *ptot_error_messages[] = {

#else /* DEFINE_ENUMS */

#  define ASSOCIATE(id,str) id,

typedef enum {

#endif /* DEFINE_ENUMS */

ASSOCIATE( ERR_ASSERT,      "Assertion failure or internal error")
ASSOCIATE( ERR_USAGE,       "Usage: ptot filename[.png]")
ASSOCIATE( ERR_MEMORY,      "Could not allocate memory")
ASSOCIATE( ERR_READ,        "Failure reading input file")
ASSOCIATE( ERR_WRITE,       "Failure writing output file")
ASSOCIATE( ERR_BAD_PNG,     "Input PNG file is incorrect format")
ASSOCIATE( ERR_BAD_IMAGE,   "Input image failed consistency check")
ASSOCIATE( ERR_BYTE_ORDER,  "Incompatible byte order")
ASSOCIATE( ERR_NO_IDAT,     "No image data found")
ASSOCIATE( ERR_COMP_HDR,    "Input PNG has invalid compression header")
ASSOCIATE( ERR_EARLY_EOI,   "Incomplete IDAT on input")
ASSOCIATE( ERR_INFLATE,     "Decompression failure")
ASSOCIATE( WARN_BAD_CRC,    "Input PNG file failed CRC check")
ASSOCIATE( WARN_BAD_SUM,    "Uncompressed image data failed sum check")
ASSOCIATE( WARN_BAD_PNG,    "Invalid (but recoverable) PNG file")
ASSOCIATE( WARN_EXTRA_BYTES,"Extra bytes in PNG file ignored")
ASSOCIATE( WARN_PLTE_GRAY,  "Palette appears in grayscale image")
ASSOCIATE( WARN_LATE_GAMA,  "Gamma value appears after palette")
ASSOCIATE( WARN_LATE_TRNS,  "Transparency chunk appears after palette")
ASSOCIATE( WARN_NO_PLTE,    "No palette found for palette-based image")
ASSOCIATE( WARN_MULTI_PLTE, "More than one palette present")
ASSOCIATE( WARN_MULTI_TRNS, "More than one transparency chunk present")
ASSOCIATE( WARN_FILTER,     "Unknown prediction filter in input PNG")
ASSOCIATE( WARN_BAD_VAL,    "Unknown value in PNG chunk")

#ifdef DEFINE_ENUMS
WARN_END
#else
"End of warning list"
#endif

#ifdef DEFINE_ENUMS

} PTOT_ERROR;
#  undef DEFINE_ENUMS

#else

};
#  define PTOT_NMESSAGES ((sizeof ptot_error_messages)/ \
                           (sizeof ptot_error_messages[0]))
#  undef DEFINE_STRINGS

#endif

#undef ASSOCIATE

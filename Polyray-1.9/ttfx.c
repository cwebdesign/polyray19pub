/* TrueType font file processing */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(MAC)
#include <console.h>
#define FSCFG_BIG_ENDIAN
#else
#define PC_OS
#endif

#ifndef SEEK_SET
#define SEEK_SET        0       /* seek from start of file      */
#define SEEK_CUR        1       /* relative to current position */
#define SEEK_END        2       /* relative to end of file      */
#endif

#define EPSILON 1.0e-10
#define MAX_ITERATIONS 50
#define COEFF_LIMIT 1.0e-20

#define ONCURVE             0x01
#define XSHORT              0x02
#define YSHORT              0x04
#define REPEAT_FLAGS        0x08 /* repeat flag n times */
#define SHORT_X_IS_POS      0x10 /* the short vector is positive */
#define NEXT_X_IS_ZERO      0x10 /* the relative x coordinate is zero */
#define SHORT_Y_IS_POS      0x20 /* the short vector is positive */
#define NEXT_Y_IS_ZERO      0x20 /* the relative y coordinate is zero */

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef long int32;
typedef unsigned long uint32;

typedef short FUnit;
typedef unsigned short uFUnit;

#ifdef PC_OS
typedef long Fixed;
#endif

#ifndef FAR
#define FAR
#endif

#ifndef NEAR
#define NEAR
#endif

#if defined(FSCFG_BIG_ENDIAN)
/* target byte order matches Motorola 68000 */
#define SWAPL(a)        (a)
#define SWAPW(a)        (a)
#else
/* Macros to turn little endian words/longs into big endian words/longs */
#define FS_2BYTE(p)  (((unsigned short)((p)[0]) << 8) |  (p)[1])
#define FS_4BYTE(p)  (FS_2BYTE((p)+2) | ( (FS_2BYTE(p)+0L) << 16))
#define SWAPW(a) ((short) FS_2BYTE((unsigned char FAR *)(&a)))
#define SWAPL(a) ((long) FS_4BYTE((unsigned char FAR *)(&a)))
#endif

/* Constants defined in Motorola order (use SWAPL to correct) */
static uint32 tag_CharToIndexMap = 0x636d6170; /* 'cmap' */
static uint32 tag_FontHeader     = 0x68656164; /* 'head' */
static uint32 tag_GlyphData      = 0x676c7966; /* 'glyf' */
static uint32 tag_IndexToLoc     = 0x6c6f6361; /* 'loca' */
static uint32 tag_Kerning        = 0x6b65726e; /* 'kern' */
static uint32 tag_MaxProfile     = 0x6d617870; /* 'maxp' */

typedef struct {
   uint32 bc;
   uint32 ad;
   } BigDate;

typedef struct {
   int32  tag;
   uint32 checkSum;
   uint32 offset;
   uint32 length;
   } sfnt_DirectoryEntry;

typedef struct {
   int32 version;                  /* 0x10000 (1.0) */
   uint16 numOffsets;              /* number of tables */
   uint16 searchRange;             /* (max2 <= numOffsets)*16 */
   uint16 entrySelector;           /* log2 (max2 <= numOffsets) */
   uint16 rangeShift;              /* numOffsets*16-searchRange*/
   sfnt_DirectoryEntry table[1];   /* table[numOffsets] */
   } sfnt_OffsetTable;

typedef struct {
   Fixed       version;            /* for this table, set to 1.0 */
   Fixed       fontRevision;       /* For Font Manufacturer */
   uint32      checkSumAdjustment;
   uint32      magicNumber;        /* signature, must be 0x5F0F3CF5 == MAGIC */
   uint16      flags;
   uint16      unitsPerEm;         /* How many in Font Units per EM */

   BigDate     created;
   BigDate     modified;

   /** This is the font wide bounding box in ideal space
       (baselines and metrics are NOT worked into these numbers) **/
   FUnit       xMin;
   FUnit       yMin;
   FUnit       xMax;
   FUnit       yMax;

   uint16      macStyle;               /* macintosh style word */
   uint16      lowestRecPPEM;          /* lowest recommended pixels per Em */

   /* 0: fully mixed directional glyphs,
      1: only strongly L->R or T->B glyphs,
     -1: only strongly R->L or B->T glyphs,
      2: like 1 but also contains neutrals,
     -2: like -1 but also contains neutrals */
   int16       fontDirectionHint;

   int16       indexToLocFormat;
   int16       glyphDataFormat;
   } sfnt_FontHeader;

typedef struct {
   uint16  format;
   uint16  length;
   uint16  version;
   } sfnt_mappingTable;

typedef struct {
   uint16  platformID;
   uint16  specificID;
   uint32  offset;
   } sfnt_platformEntry;

typedef sfnt_platformEntry FAR *sfnt_platformEntryPtr;
typedef sfnt_DirectoryEntry FAR *sfnt_DirectoryEntryPtr;

typedef struct {
    int16 numContours;
    int16 xMin;
    int16 yMin;
    int16 xMax;
    int16 yMax;
    } GlyphHeader;

typedef struct {
   GlyphHeader header;
   uint16 numPoints;
   uint16 *endPoints;
   uint8  *flags;
   float  *x, *y;
   } GlyphOutline;

typedef struct {
   uint8 inside_flag;  /* 1 if this an inside contour, 0 if outside */
   uint16 count;       /* Number of points in the contour */
   uint8  *flags;      /* On/off curve flags */
   float  *x, *y;      /* Coordinates of control vertices */
   } Contour;

typedef struct FontFileInfoStruct *FontFileInfoPtr;
typedef struct GlyphStruct *GlyphPtr;

/* Contour information for a single glyph */
typedef struct GlyphStruct {
   GlyphHeader header;     /* Count and sizing information about this glyph */
   uint16 glyph_index;     /* Internal glyph index for this character */
   Contour *contours;      /* Array of outline contours */
   FontFileInfoPtr parent; /* Parent font for this glyph */
   GlyphPtr next;          /* Next cached glyph */
   char   c;               /* Character code */
   } Glyph;

/* Useful general data about this font */
typedef struct FontFileInfoStruct {
   char *filename;
   FILE *fp;
   uint16 numGlyphs;
   uint16 unitsPerEm;
   int16  loca_format;
   uint32 *loca_table;
   uint32 loca_table_offset;
   uint32 cmap_table_offset;
   uint32 glyph_table_offset;
   GlyphPtr glyphs;                   /* Cached info for this font */
   struct FontFileInfoStruct *next; /* Next font */
   } FontFileInfo;

static FontFileInfo *TTFonts = NULL;

static FontFileInfo *
OpenFontFile(char *filename)
{
   int i;
   FontFileInfo *fontlist;

   /* First look to see if we have already opened this font */
   for (fontlist = TTFonts;fontlist!=NULL;fontlist=fontlist->next)
      if (!strcmp(filename, fontlist->filename))
         break;

   if (fontlist != NULL) {
      /* We have a match, use the previous information */
      if ((fontlist->fp == NULL) &&
          (fontlist->fp = fopen(fontlist->filename, "rb")) == NULL) {
         fprintf(stderr, "Can't open font file: '%s'\n",
                 fontlist->filename);
         exit(1);
         }
      }
   else {
      /* We haven't looked at this font before, let's allocate a
         holder for the information and set some defaults */
      fontlist = (FontFileInfo *)malloc(sizeof(FontFileInfo));
      if (fontlist == NULL) {
         fprintf(stderr, "Out of memory\n");
         exit(1);
         }
      i = strlen(filename) + 1;
      fontlist->filename = malloc(i * sizeof(char));
      strcpy(fontlist->filename, filename);
      if ((fontlist->fp = fopen(fontlist->filename, "rb")) == NULL) {
         fprintf(stderr, "Can't open font file: '%s'\n",
                 fontlist->filename);
         exit(1);
         }
      fontlist->numGlyphs = 0;
      fontlist->unitsPerEm = 16384;
      fontlist->loca_format = 0;
      fontlist->loca_table_offset = 0;
      fontlist->loca_table = NULL;
      fontlist->cmap_table_offset = 0;
      fontlist->glyph_table_offset = 0;
      fontlist->glyphs = NULL;
      fontlist->next = NULL;
      }

   return fontlist;
}

static void
FreeFontInfo()
{
   int i;
   FontFileInfo *oldfont, *tempfont;
   GlyphPtr glyphs, tempglyph;

   for (oldfont=TTFonts;oldfont!=NULL;) {
      if (oldfont->fp != NULL)
         fclose(oldfont->fp);
      free(oldfont->filename);
      if (oldfont->loca_table != NULL)
         free(oldfont->loca_table);
      for (glyphs=oldfont->glyphs;glyphs!=NULL;) {
         for (i=0;i<glyphs->header.numContours;i++) {
            free(glyphs->contours[i].flags);
            free(glyphs->contours[i].x);
            free(glyphs->contours[i].y);
            }
         free(glyphs->contours);
         tempglyph = glyphs;
         glyphs = glyphs->next;
         free(tempglyph);
         }
      tempfont = oldfont;
      oldfont = oldfont->next;
      free(tempfont);
      }
   TTFonts = NULL;
}

/* Transform a glyph from TrueType storage format to something a little
   easier to manage */
static GlyphPtr
ConvertOutlineToGlyph(FontFileInfo *ffile, GlyphOutline *ttglyph)
{
   GlyphPtr glyph;
   float *temp_x, *temp_y;
   uint8 *temp_f;
   uint16 i, j, last_j;

   /* Create storage for this glyph */
   if ((glyph = malloc(sizeof(Glyph))) == NULL)
      exit(1);
   glyph->contours = malloc(ttglyph->header.numContours * sizeof(Contour));
   if (glyph->contours == NULL)
      exit(1);

   /* Copy sizing information about this glyph */
   memcpy(&glyph->header, &ttglyph->header, sizeof(GlyphHeader));

   /* Keep track of the parent font for this glyph */
   glyph->parent = ffile;

   /* Now copy the vertex information into the contours */
   for (i=0, last_j=0;i<ttglyph->header.numContours;i++) {
      /* Figure out number of points in contour */
      j = ttglyph->endPoints[i] - last_j + 1;

      /* Copy the coordinate information into the glyph */
      temp_x = malloc((j+1) * sizeof(float));
      temp_y = malloc((j+1) * sizeof(float));
      temp_f = malloc((j+1) * sizeof(uint8));
      if (temp_x == NULL || temp_y == NULL || temp_f == NULL)
         exit(1);
      memcpy(temp_x, &ttglyph->x[last_j], j * sizeof(float));
      memcpy(temp_y, &ttglyph->y[last_j], j * sizeof(float));
      memcpy(temp_f, &ttglyph->flags[last_j], j * sizeof(uint8));
      temp_x[j] = ttglyph->x[last_j];
      temp_y[j] = ttglyph->y[last_j];
      temp_f[j] = ttglyph->flags[last_j];

      /* Figure out if this is an inside or outside contour */
      glyph->contours[i].inside_flag = 0;

      /* Plug in the reset of the contour components into the glyph */
      glyph->contours[i].count = j;
      glyph->contours[i].x = temp_x;
      glyph->contours[i].y = temp_y;
      glyph->contours[i].flags = temp_f;

      /* Set last_j to point to the beginning of the next contour's
         coordinate information */
      last_j = ttglyph->endPoints[i] + 1;
      }

/* Show statistics about this glyph */
/*
printf("Number of contours: %u\n",
       glyph->header.numContours);
printf("X extent: [%f, %f]\n",
       (float)glyph->header.xMin / (float)glyph->parent->unitsPerEm,
       (float)glyph->header.xMax / (float)glyph->parent->unitsPerEm);
printf("Y extent: [%f, %f]\n",
       (float)glyph->header.yMin / (float)glyph->parent->unitsPerEm,
       (float)glyph->header.yMax / (float)glyph->parent->unitsPerEm);
printf("Converted coord list(%d):\n", (int)glyph->header.numContours);
for (i=0;i<glyph->header.numContours;i++) {
   for (j=0;j<=glyph->contours[i].count;j++)
      printf("  %c[%f, %f]\n",
             (glyph->contours[i].flags[j] & ONCURVE ? '*' : ' '),
             glyph->contours[i].x[j], glyph->contours[i].y[j]);
   printf("\n");
   }
*/

   return glyph;
}

/* Read the contour information for a specific glyph */
static GlyphPtr
ExtractGlyphInfo(FontFileInfo *ffile, uint16 glyph_index, unsigned char c)
{
   int i, j;
   uint8 temp8, flag, repeat_count;
   uint16 temp16, n, nc;
   int16 coord, itemp16;
   Fixed tempF;
   GlyphOutline *ttglyph;
   GlyphPtr glyph;

   if ((ttglyph = malloc(sizeof(GlyphOutline))) == NULL) {
      fprintf(stderr, "Out of memory\n");
      exit(1);
      }

   fseek(ffile->fp, ffile->glyph_table_offset +
                    ffile->loca_table[glyph_index], SEEK_SET);
   fread(&ttglyph->header, 1, sizeof(GlyphHeader), ffile->fp);
   ttglyph->header.numContours = SWAPW(ttglyph->header.numContours);
   ttglyph->header.xMax        = SWAPW(ttglyph->header.xMax);
   ttglyph->header.yMax        = SWAPW(ttglyph->header.yMax);
   ttglyph->header.xMin        = SWAPW(ttglyph->header.xMin);
   ttglyph->header.yMin        = SWAPW(ttglyph->header.yMin);

   nc = ttglyph->header.numContours;
   if (nc > 0) {
      /* Grab the contour endpoints */
      if ((ttglyph->endPoints = malloc(nc * sizeof(uint16))) == NULL) {
         fprintf(stderr, "Out of memory\n");
         exit(1);
         }
      for (i=0;i<nc;i++) {
         fread(&temp16, 1, sizeof(temp16), ffile->fp);
         ttglyph->endPoints[i] = SWAPW(temp16);
         }

      /* Skip over the instructions */
      fread(&temp16, 1, sizeof(temp16), ffile->fp);
      fseek(ffile->fp, SWAPW(temp16), SEEK_CUR);

      /* Determine the number of points making up this glyph */
      n = ttglyph->numPoints = ttglyph->endPoints[nc-1] + 1;

      /* Read the flags */
      if ((ttglyph->flags = malloc(n * sizeof(uint8))) == NULL) {
         fprintf(stderr, "Out of memory\n");
         exit(1);
         }
      for (i=0;i<ttglyph->numPoints;i++) {
         fread(&ttglyph->flags[i], 1, 1, ffile->fp);
         if (ttglyph->flags[i] & REPEAT_FLAGS) {
            fread(&repeat_count, 1, 1, ffile->fp);
            for (;repeat_count>0;repeat_count--,i++) {
               ttglyph->flags[i+1] = ttglyph->flags[i];
               }
            }
         }

      /* Read the coordinate vectors */
      if ((ttglyph->x = malloc(n * sizeof(float))) == NULL ||
          (ttglyph->y = malloc(n * sizeof(float))) == NULL) {
         fprintf(stderr, "Out of memory\n");
         exit(1);
         }
      coord = 0;
      for (i=0;i<ttglyph->numPoints;i++) {
         /* Read each x coordinate */
         flag = ttglyph->flags[i];
         if (flag & XSHORT) {
            fread(&temp8, 1, 1, ffile->fp);
            if (flag & SHORT_X_IS_POS)
               coord += temp8;
            else
               coord -= temp8;
            }
         else if (!(flag & NEXT_X_IS_ZERO)) {
            fread(&itemp16, 1, 2, ffile->fp);
            coord += SWAPW(itemp16);
            }
         ttglyph->x[i] = (float)coord / (float)ffile->unitsPerEm;
         }

      coord = 0;
      for (i=0;i<ttglyph->numPoints;i++) {
         /* Read each y coordinate */
         flag = ttglyph->flags[i];
         if (flag & YSHORT) {
            fread(&temp8, 1, 1, ffile->fp);
            if (flag & SHORT_Y_IS_POS)
               coord += temp8;
            else
               coord -= temp8;
            }
         else if (!(flag & NEXT_Y_IS_ZERO)) {
            fread(&itemp16, 1, 2, ffile->fp);
            coord += SWAPW(itemp16);
            }
         ttglyph->y[i] = (float)coord / (float)ffile->unitsPerEm;
         }

      /* Convert the glyph outline information from TrueType layout
         into a more easily processed format */
      glyph = ConvertOutlineToGlyph(ffile, ttglyph);
      glyph->c = c;
      glyph->glyph_index = glyph_index;

      /* Free up outline information */
      free(ttglyph->y);
      free(ttglyph->x);
      free(ttglyph->endPoints);
      free(ttglyph->flags);
      }
   else if (nc == 0) {
      /* Do nothing */
      }
   else {
      printf("Can't handle multiple component glyphs\n");
      }

   return glyph;
}

/* The file pointer must be pointing immediately after the version entry
   in the encoding table for the next two functions to work. */
static uint16
ProcessFormat0Glyph(FILE *fp, unsigned char search_char)
{
   uint8 temp_index;

   fseek(fp, search_char, SEEK_CUR);
   fread(&temp_index, 1, 1, fp); /* Each index is exactly 1 byte */
   return (uint16)temp_index;
}

static uint16
ProcessFormat4Glyph(FILE *fp, unsigned char search_char)
{
   int i;
   uint16  glyph_index;
   long glyphIDoffset;
   uint16 temp16;
   uint16 segCount, searchRange, entrySelector, rangeShift;
   uint16 *endCount, *startCount, *idDelta, *idRangeOffset;

   fread(&temp16, 1, sizeof(uint16), fp);
   segCount = SWAPW(temp16) >> 1;
   fread(&temp16, 1, sizeof(uint16), fp);
   searchRange = SWAPW(temp16);
   fread(&temp16, 1, sizeof(uint16), fp);
   entrySelector = SWAPW(temp16);
   fread(&temp16, 1, sizeof(uint16), fp);
   rangeShift = SWAPW(temp16);

   /* Now allocate and read in the segment arrays */
   endCount      = malloc(segCount * sizeof(uint16));
   startCount    = malloc(segCount * sizeof(uint16));
   idDelta       = malloc(segCount * sizeof(uint16));
   idRangeOffset = malloc(segCount * sizeof(uint16));
   if (endCount == NULL || startCount == NULL || idDelta == NULL ||
       idRangeOffset == NULL) {
      fprintf(stderr, "Out of memory\n");
      exit(1);
      }
   for (i=0;i<segCount;i++) {
      fread(&temp16, 1, sizeof(uint16), fp);
      endCount[i] = SWAPW(temp16);
      }
   fread(&temp16, 1, sizeof(uint16), fp); /* Skip over 'reservedPad' */
   for (i=0;i<segCount;i++) {
      fread(&temp16, 1, sizeof(uint16), fp);
      startCount[i] = SWAPW(temp16);
      }
   for (i=0;i<segCount;i++) {
      fread(&temp16, 1, sizeof(uint16), fp);
      idDelta[i] = SWAPW(temp16);
      }
   glyphIDoffset = ftell(fp);
   for (i=0;i<segCount;i++) {
      fread(&temp16, 1, sizeof(uint16), fp);
      idRangeOffset[i] = SWAPW(temp16);
      }

   /* Now search the segments for our character */
   glyph_index = 0;
   for (i=0;i<segCount;i++) {
      if ((uint8)search_char <= endCount[i]) {
         if ((uint8)search_char > startCount[i]) {
            /* Found mapping */
            if (idRangeOffset[i] == 0)
               glyph_index = (uint16)search_char + idDelta[i];
            else {
               /* Alternate encoding of glyph indices, relies on a quite
                  unusual way of storing the offsets.  Not really sure about
                  the division by 2 on idRangeOffset, but the Microsoft
                  manual seemed to do it that way. */
               glyphIDoffset += i * sizeof(uint16) + idRangeOffset[i]/2 +
                                (search_char - startCount[i]);
               fseek(fp, glyphIDoffset, SEEK_SET);
               fread(&temp16, 1, sizeof(uint16), fp);
               glyph_index = SWAPW(temp16);
               if (glyph_index != 0)
                  glyph_index = glyph_index + idDelta[i];
               }
            }
         break;
         }
      }

   /* Deallocate the memory we used for the segment arrays */
   free(endCount);
   free(startCount);
   free(idDelta);
   free(idRangeOffset);

   return glyph_index;
}

/* Trimmed table mapping */
static uint16
ProcessFormat6Glyph(FILE *fp, unsigned char search_char)
{
   uint16 temp16, firstCode, entryCount;
   uint8 glyph_index;

   fread(&temp16, 1, 2, fp);
   firstCode = SWAPW(temp16);
   fread(&temp16, 1, 2, fp);
   entryCount = SWAPW(temp16);
   if (search_char >= firstCode && search_char < firstCode + entryCount) {
      fseek(fp, search_char - firstCode, SEEK_CUR);
      fread(&temp16, 1, 2, fp);
      glyph_index = SWAPW(temp16);
      }
   else
      glyph_index = 0;
   return glyph_index;
}


/* find the character mapping for 'search_char' */
static uint16
ProcessCharMap(FontFileInfo *ffile, unsigned char search_char)
{
   long old_table_offset;
   uint16 glyph_index, temp16;
   uint16 entry_ID, entry_EID;
   uint32 entry_offset;
   sfnt_platformEntry cmapEntry;
   sfnt_mappingTable encodingTable;
   int i, table_count;

   /* Move to the start of the character map, skipping over the format
      entry, right to the number of entries. */
   fseek(ffile->fp, ffile->cmap_table_offset + sizeof(uint16), SEEK_SET);
   fread(&temp16, 1, sizeof(uint16), ffile->fp);
   table_count = SWAPW(temp16);

   /* Search the tables until we find the glyph index for the search
      character.  Just return the first one we find... */
   for (i=0;i<table_count;i++) {
      fread(&cmapEntry, 1, sizeof(cmapEntry), ffile->fp);
      entry_ID     = SWAPW(cmapEntry.platformID);
      entry_EID    = SWAPW(cmapEntry.specificID);
      entry_offset = SWAPL(cmapEntry.offset);
      old_table_offset = ftell(ffile->fp);

      fseek(ffile->fp, ffile->cmap_table_offset + entry_offset, SEEK_SET);
      fread(&encodingTable, 1, sizeof(encodingTable), ffile->fp);
      /*
      printf("Encoding table, format: %u, length: %u, version: %u\n",
             SWAPW(encodingTable.format),
             SWAPW(encodingTable.length),
             SWAPW(encodingTable.version));
      */

      if (SWAPW(encodingTable.format) == 0) {
         /* Translation is simple - add 'entry_char' to the start
            of the table and grab what's there */
         /* printf("Apple standard index mapping\n"); */
         glyph_index = ProcessFormat0Glyph(ffile->fp, search_char);
         }
      else if (SWAPW(encodingTable.format) == 4) {
         /* Microsoft UGL encoding */
         /* printf("Microsoft standard index mapping\n"); */
         glyph_index = ProcessFormat4Glyph(ffile->fp, search_char);
         }
      else if (SWAPW(encodingTable.format) == 6) {
         /* printf("Trimmed table mapping\n"); */
         glyph_index = ProcessFormat6Glyph(ffile->fp, search_char);
         }
      else
         printf("Unsupported index mapping format: %u\n",
                SWAPW(encodingTable.format));

      return glyph_index;

      /* Go back to the set of table descriptions at the beginning of
         the cmap table. */
      fseek(ffile->fp, old_table_offset, SEEK_SET);
      }

   /* No character mapping was found - very odd, we should really have
      had the character in at least one table.  Perhaps getting here
      means we didn't have any character mapping tables */
   return 0;
}

/* This routine determines the total number of glyphs in a TrueType file.
   Necessary so that we can allocate the proper amount of storage for
   the glyph location table.  */
static void
ProcessMaxpTable(FontFileInfo *ffile, sfnt_DirectoryEntry *Table)
{
   long old_fp_offset;
   Fixed temp_fixed;
   uint16 temp16;

   old_fp_offset = ftell(ffile->fp);
   fseek(ffile->fp, SWAPL(Table->offset), SEEK_SET);
   fread(&temp_fixed, 1, sizeof(temp_fixed), ffile->fp);
   fread(&temp16, 1, sizeof(temp16), ffile->fp);
   ffile->numGlyphs = SWAPW(temp16);
   fseek(ffile->fp, old_fp_offset, SEEK_SET);
}

/* Process the font header table */
static void
ProcessHeadTable(FontFileInfo *ffile, sfnt_DirectoryEntry *Table)
{
   long old_fp_offset;
   sfnt_FontHeader fontHeader;

   old_fp_offset = ftell(ffile->fp);
   fseek(ffile->fp, SWAPL(Table->offset), SEEK_SET);

   fread(&fontHeader, 1, sizeof(fontHeader), ffile->fp);
   ffile->loca_format = SWAPW(fontHeader.indexToLocFormat);
   ffile->unitsPerEm = SWAPW(fontHeader.unitsPerEm);

   /* Back to where we started */
   fseek(ffile->fp, old_fp_offset, SEEK_SET);
}

/* Determine the relative offsets of glyphs */
static void
ProcessLocaTable(FontFileInfo *ffile)
{
   int i;
   uint16 temp16;
   uint32 temp32;

   /* Move to location of table in file */
   fseek(ffile->fp, ffile->loca_table_offset, SEEK_SET);

   /* Make sure we understand the size of the entries in the table */
   ffile->loca_table = malloc((ffile->numGlyphs + 1) * sizeof(uint32));
   if (ffile->loca_table == NULL) {
      fprintf(stderr, "Out of memory\n");
      exit(1);
      }

   /* Now read and save the loca table */
   for (i=0;i<ffile->numGlyphs;i++) {
      if (ffile->loca_format == 0) {
         fread(&temp16, 1, sizeof(uint16), ffile->fp);
         ffile->loca_table[i] = (uint32)SWAPW(temp16) << 1;
         }
      else {
         fread(&temp32, 1, sizeof(uint32), ffile->fp);
         ffile->loca_table[i] = SWAPL(temp32);
         }
      }
}

/* Gather information about a specific font.  The font file must
   be opened prior to calling this routine. */
static FontFileInfo *
ProcessFontFile(char *fontfilename)
{
   unsigned i, c;
   sfnt_OffsetTable OffsetTable;
   sfnt_DirectoryEntry Table;
   FontFileInfo *ffile;

   /* Open the font file */
   ffile = OpenFontFile(fontfilename);

   /* Read the initial directory header on the TTF.  The numOffsets variable
      tells us how many tables are present in this file. */
   fread(&OffsetTable, 1, sizeof(OffsetTable) - sizeof(sfnt_DirectoryEntry),
         ffile->fp);
   c = (unsigned)SWAPW(OffsetTable.numOffsets);

   /* Process general font information and save it. */
   for (i = 0; i < c && i < 40; i++) {
      if (fread(&Table, 1, sizeof(Table), ffile->fp) != sizeof(Table)) {
         printf("Read failed on table #%d\n", i);
         exit(-1);
         }
      if (Table.tag == SWAPL(tag_CharToIndexMap))
         ffile->cmap_table_offset = SWAPL(Table.offset);
      else if (Table.tag == SWAPL(tag_MaxProfile))
         ProcessMaxpTable(ffile, &Table);
      else if (Table.tag == SWAPL(tag_IndexToLoc))
         ffile->loca_table_offset = SWAPL(Table.offset);
      else if (Table.tag == SWAPL(tag_FontHeader))
         ProcessHeadTable(ffile, &Table);
      else if (Table.tag == SWAPL(tag_GlyphData))
         ffile->glyph_table_offset = SWAPL(Table.offset);
      }

   /* Close the font file descriptor (don't want to waste them) */
   fclose(ffile->fp);
   ffile->fp = NULL;

   /* Return the information about this font */
   return ffile;
}


static GlyphPtr
ProcessCharacter(FontFileInfo *ffile, unsigned char search_char)
{
   uint16 glyph_index;
   GlyphOutline *ttglyph;
   GlyphPtr glyph;

   /* See if we have already processed this glyph */
   if (ffile->glyphs != NULL) {
      for (glyph=ffile->glyphs;glyph!=NULL;glyph=glyph->next)
         if (glyph->c == search_char) {
            /* Found it, no need to do any more work */
            return glyph;
            }
      }

   /* Open the font file (if necessary) */
   if ((ffile->fp == NULL) &&
       (ffile->fp = fopen(ffile->filename, "rb")) == NULL) {
      fprintf(stderr, "Can't open font file: '%s'\n", ffile->filename);
      exit(1);
      }

   /* Now go extract information about the search character */
   if (ffile->numGlyphs > 0 && ffile->loca_table_offset > 0 &&
       ffile->cmap_table_offset > 0 && ffile->glyph_table_offset > 0) {
      /* We have enough information on the file layout to go look for
         the information about the search character */
      ProcessLocaTable(ffile);
      glyph_index = ProcessCharMap(ffile, search_char);
      glyph = ExtractGlyphInfo(ffile, glyph_index, search_char);

      /* Add this glyph to the ones we already know about */
      glyph->next = ffile->glyphs;
      ffile->glyphs = glyph;
      }

   /* Close the font file */
   fclose(ffile->fp);
   ffile->fp = NULL;

   /* Glyph is all built */
   return glyph;
}

static void
OutputPolyrayGlyph(FontFileInfo *ffile, Glyph *glyph, float offset)
{
   int i, j;

   printf("object { // Char: '%c'\n", (char)glyph->c);
   printf("   glyph %u\n", glyph->header.numContours);
   for (i=0;i<glyph->header.numContours;i++) {
      printf("      contour %d, ", (int)glyph->contours[i].count);
      for (j=0;j<glyph->contours[i].count;j++) {
         printf("<%f, %f, %c>",
                glyph->contours[i].x[j] + offset, glyph->contours[i].y[j],
                (glyph->contours[i].flags[j] & ONCURVE ? '0' : '1'));
         if (j < glyph->contours[i].count - 1) {
            printf(", ");
	    if (j > 0 && !(j % 5))
	       printf("\n");
	    }
         else
            printf("\n");
         }
      }
printf("   }\n");
}

int
main(int argc, char **argv)
{
   FontFileInfo *ffile;
   int i, slen;
   unsigned char *search_string = NULL;
   GlyphPtr glyph;
   float toffset, offset;

#ifdef MAC
   /* Special case for Macintosh - this command pulls up a
      dialog that simulates a command line */
   argc = ccommand(&argv);
#endif

   if (argc < 2) {
      printf("usage: ttfx *.ttf string [char offset]\n");
      printf("e.g.\n");
      printf("       ttfx arialb.ttf Foo 0.1\n");
      printf("       ttfx \\windows\\system\\times.ttf Foo\n");
      exit(-1);
      }
   else if (argc > 2) {
      slen = strlen(argv[2]);
      search_string = malloc((slen + 1) * sizeof(unsigned char));
      for (i=0;i<=slen;i++)
         search_string[i] = (unsigned char)argv[2][i];
      if (argc > 3)
         offset = atof(argv[3]);
      else
         offset = 0.0;
      }
   else {
      slen = 1;
      search_string = malloc(2 * sizeof(unsigned char));
      search_string[0] = 'X';
      search_string[1] = '\0';
      offset = 0.0;
      }

   /* Get general font info */
   ffile = ProcessFontFile(argv[1]);

   printf("// Font: '%s'\n", ffile->filename);
   if (slen > 1)
      /* Need to create the string as the union of the glyphs that
         make up each character in the string */
      printf("object {\n");

   /* Get info about each character in the string */
   for (i=0,toffset=0.0;i<slen;i++) {
      glyph = ProcessCharacter(ffile, search_string[i]);
/* printf("Kick: %f\n",
	  glyph->header.xMin / (float)glyph->parent->unitsPerEm); */
      /* Align this character to x = 0 on the left */
      toffset -= glyph->header.xMin / (float)glyph->parent->unitsPerEm;

      /* Additional spacing based on the kerning table could go right here */

      /* Now we convert the glyph into Polyray format */
      OutputPolyrayGlyph(ffile, glyph, toffset);

/*
printf("Bump: %f\n", offset +
                (glyph->header.xMax - glyph->header.xMin) /
                   (float)glyph->parent->unitsPerEm);
*/
      /* Adjust character spacing to account for the width of the character,
         plus the user-specified character spacing */
      toffset += offset +
                (glyph->header.xMax - glyph->header.xMin) /
                   (float)glyph->parent->unitsPerEm;

      /* If this is a union, then need to emit the "+" character */
      if (slen > 1 && i < slen - 1)
         printf("+\n");
      }

   /* Close up the union of glyphs */
   if (slen > 1)
      printf("   }\n");

   FreeFontInfo();

   if (search_string != NULL)
      free(search_string);

   return 0;
}

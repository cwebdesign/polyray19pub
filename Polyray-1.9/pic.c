/*
  pic.c

  Image file I/O

  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.

  This software may be used for any private and non-commercial
  use.

  You may not distribute this software, in whole or in part,
  for any commercial purpose, without the express consent of
  the authors.

  There is no warranty or other guarantee of fitness of this software
  for any purpose.  It is provided solely "as is".

*/

#ifdef _MSC_VER
#include "unixcompat.h"
#else
#include <unistd.h> /* for close */
#endif
#include "defs.h"
#include "pic.h"
#include "io.h"
#include "memory.h"
#include "display.h"
#include "symtab.h"

extern int outformat;


#ifndef SEEK_SET
#define SEEK_SET        0       /* seek from start of file      */
#define SEEK_CUR        1       /* relative to current position */
#define SEEK_END        2       /* relative to end of file      */
#endif

#define POLYRAY_RENDERED_LINE_TAG 0x0496

/* Variables used by compressed Targa routines */
static char *polyray_id = "Polyray v1.8c 2004 by Alexander Enzmann, modifications by Clyde Meli for ISM212 lectures";

/* Return 0 if file broken, 1 if pixel successfully read */
static int
get_pixel(FILE *filep, int psize,
          unsigned char *r, unsigned char *g,
          unsigned char *b, unsigned char *o)
{
   int bytes[4];
   int size = psize;
   switch (size) {
   case 8:
      bytes[0] = fgetc(filep);
      if (bytes[0] == EOF)
         return 0;
      *b = bytes[0];
      *g = *b;
      *r = *b;
      *o = 255;
      break;
   case 16:
      bytes[0] = fgetc(filep);
      bytes[1] = fgetc(filep);
      if (bytes[0] == EOF || bytes[1] == EOF)
         return 0;
      *b = (bytes[0] & 0x1f) << 3;
      *g = (((bytes[1] & 0x03) << 3) | ((bytes[0] & 0xe0) >> 5)) << 3;
      *r = ((bytes[1] & 0x7c) << 1);
      *o = (bytes[1] & 0x80 ? 0 : 255);
      break;
   case 24:
      bytes[0] = fgetc(filep);
      bytes[1] = fgetc(filep);
      bytes[2] = fgetc(filep);
      if (bytes[0] == EOF || bytes[1] == EOF || bytes[2] == EOF)
         return 0;
      *b = bytes[0];
      *g = bytes[1];
      *r = bytes[2];
      *o = 255;
      break;
   case 32:
      bytes[0] = fgetc(filep);
      bytes[1] = fgetc(filep);
      bytes[2] = fgetc(filep);
      bytes[3] = fgetc(filep);
      if (bytes[0] == EOF || bytes[1] == EOF ||
          bytes[2] == EOF || bytes[3] == EOF)
         return 0;
      *b = bytes[0];
      *g = bytes[1];
      *r = bytes[2];
      *o = bytes[3];
      break;
   default:
      error("Bad pixel size %d in get_pixel\n", size);
   }
   return 1;
}

static void
put_pixel(Pic *pic, unsigned char r, unsigned char g, unsigned char b,
          unsigned char o)
{
   int size = pic->psize;
   unsigned char grey, low, high;

   switch (size) {
   case 8:
      /* Output the average of the three colors, using the formula:
            g = x * r + y * g + z * b */
      if (DepthRender == 1)
         grey = r;
      else
         grey = (unsigned char)(0.59 * r + 0.3 * g + 0.11 * b);
      fputc(grey, pic->filep);
      break;
   case 16:


   /* CM added 10/mar/2000 */
   if (outformat == OUT_TARGA) {
   /* CM end */
      if (DepthRender == 1) {
         high = r;
         low  = g;
         }
      else {
         /* Pack the bytes for output */
         r >>= 3;
         g >>= 3;
         b >>= 3;
         high = (r << 2) | ((g & 0x18) >> 3);
         low  = ((g & 0x07) << 5) | b;
         if (o < 255) high |= 0x80;
         }
      fputc(low, pic->filep);
      fputc(high, pic->filep);
      break;
   /* CM added 10/mar/2000 */
   }
   else { /* this is PPM RAW FORMAT */
     fputc(r, pic->filep);
     fputc(g, pic->filep);
     fputc(b, pic->filep);
   }
   /* CM end */
   case 24:

      /* CM added 10/mar/2000 */
      if (outformat == OUT_TARGA) {
      /* CM end */
      fputc(b, pic->filep);
      fputc(g, pic->filep);
      fputc(r, pic->filep);
      /* CM added 10/mar/2000 */
      } else {
       fputc(r, pic->filep);
       fputc(g, pic->filep);
       fputc(b, pic->filep);
      }
      /* CM end */

      break;
   case 32:
      /* CM added 10/mar/2000 */
      if (outformat == OUT_TARGA) {
      /* CM end */
      fputc(b, pic->filep);
      fputc(g, pic->filep);
      fputc(r, pic->filep);
      fputc(o, pic->filep);
      /* CM added 10/mar/2000 */
      } else {
      /* print in ASCII format PPM */
      fprintf(pic->filep, "%d %d %d\n",r,g,b);
      }
      /* CM end */
      break;
   default:
      error("Bad pixel size: %d in put_pixel\n", size);
   }

   /* OLD: if (buffer_update && ++pic->OutputCount >= buffer_size) { */
   /* CM modified 10/mar/2000 */
   if (buffer_update && (unsigned) (++pic->OutputCount) >= buffer_size) {
   /* CM end */

      fflush(pic->filep);
      pic->filep = freopen(pic->filename, "ab", pic->filep);
      pic->OutputCount = 0;
      }
}

static int
pixels_equal(int count, unsigned char *buf1, unsigned char *buf2)
{
   int i;
   for (i=0;i<count;i++)
      if (buf1[i] != buf2[i]) {
         return 0;
         }
   return 1;
}

static void
put_buffer(Pic *pic, unsigned char r, unsigned char g, unsigned char b,
           unsigned char o)
{
   int i, j, size = pic->psize / 8;
   unsigned char bytes[4];

   switch (size) {
   case 1:
      bytes[0] = r;
      break;
   case 2:
      /* Pack the bytes for output */
      r >>= 3;
      g >>= 3;
      b >>= 3;
      bytes[1] = (r << 2) | ((g & 0x18) >> 3);
      bytes[0] = ((g & 0x07) << 5) | b;
      if (o < 255) bytes[1] |= 0x80;
      break;
   case 3:
      bytes[0] = b;
      bytes[1] = g;
      bytes[2] = r;
      break;
   case 4:
      bytes[0] = b;
      bytes[1] = g;
      bytes[2] = r;
      bytes[3] = o;
      break;
   default:
      error("Bad pixel size %d in put_buffer\n", size);
   }

   /* See if we are going to cross a line boundary */
   if (pic->ColumnCount == pic->x) {
      /* Flush the current contents to the file, then start
         the buffering process over again. */
      if (pic->CopyCount > 0) {
         fputc((unsigned char)pic->CopyCount-1, pic->filep);
         for (i=0;i<pic->CopyCount;i++)
            for (j=0;j<size;j++)
               fputc(pic->buffer[i][j], pic->filep);
         }
      else if (pic->RepeatCount > 0) {
         fputc(((unsigned char)(pic->RepeatCount-1)) | 0x80, pic->filep);
         for (j=0;j<size;j++)
            fputc(pic->buffer[0][j], pic->filep);
         }
      pic->CopyCount = 0;
      pic->RepeatCount = 0;
      pic->ColumnCount = 0;
      }

   pic->ColumnCount++;
   if (pic->CopyCount == 0 && pic->RepeatCount == 0) {
      pic->CopyCount = 1;
      for (j=0;j<size;j++) pic->buffer[0][j] = bytes[j];
      }
   else if (pic->CopyCount == 1 &&
            pixels_equal(size, &(pic->buffer[0][0]), &bytes[0])) {
      pic->CopyCount   = 0;
      pic->RepeatCount = 2;
      }
   else if (pic->CopyCount > 1 &&
            pixels_equal(size, &(pic->buffer[pic->CopyCount-1][0]),
                         &bytes[0])) {
      fputc((unsigned char)(pic->CopyCount-2), pic->filep);
      for (i=0;i<pic->CopyCount-1;i++)
         for (j=0;j<size;j++)
            fputc(pic->buffer[i][j], pic->filep);
      pic->OutputCount += pic->CopyCount;
      /* CM added (unsigned) cast 19/1/2002 */
      if (buffer_update && (unsigned) pic->OutputCount >= buffer_size) {
         fflush(pic->filep);
         pic->filep = freopen(pic->filename, "ab", pic->filep);
         pic->OutputCount = 0;
         }
      pic->CopyCount   = 0;
      pic->RepeatCount = 2;
      for (j=0;j<size;j++)
         pic->buffer[0][j] = bytes[j];
      }
   else if (pic->CopyCount > 0) {
      for (j=0;j<size;j++)
         pic->buffer[pic->CopyCount][j] = bytes[j];
      pic->CopyCount++;
      if (pic->CopyCount > 127) {
         /* Buffer full, dump */
         fputc((unsigned char)pic->CopyCount-1, pic->filep);
         for (i=0;i<pic->CopyCount;i++)
            for (j=0;j<size;j++)
               fputc(pic->buffer[i][j], pic->filep);
         pic->OutputCount += pic->CopyCount;
         /* CM added (unsigned) cast 19/1/2002 */
         if (buffer_update && (unsigned) pic->OutputCount >= buffer_size) {
            fflush(pic->filep);
            pic->filep = freopen(pic->filename, "ab", pic->filep);
            pic->OutputCount = 0;
            }
         pic->CopyCount   = 0;
         pic->RepeatCount = 0;
         }
      }
   else if (pixels_equal(size, &(pic->buffer[0][0]), &bytes[0])) {
      pic->RepeatCount++;
      if (pic->RepeatCount > 127) {
         /* Buffer full, dump */
         fputc(((unsigned char)(pic->RepeatCount-1)) | 0x80, pic->filep);
         for (j=0;j<size;j++)
            fputc(pic->buffer[0][j], pic->filep);
         pic->OutputCount += pic->RepeatCount;
         /* CM added (unsigned) cast 19/1/2002 */
         if (buffer_update && (unsigned) pic->OutputCount >= buffer_size) {
            fflush(pic->filep);
            pic->filep = freopen(pic->filename, "ab", pic->filep);
            pic->OutputCount = 0;
            }
         pic->CopyCount   = 0;
         pic->RepeatCount = 0;
         }
      }
   else {
      fputc(((unsigned char)(pic->RepeatCount-1)) | 0x80, pic->filep);
      for (j=0;j<size;j++)
         fputc(pic->buffer[0][j], pic->filep);
      for (j=0;j<size;j++)
         pic->buffer[0][j] = bytes[j];
      pic->OutputCount += pic->RepeatCount;
      /* CM added (unsigned) cast 19/1/2002 */
      if (buffer_update && (unsigned) pic->OutputCount >= buffer_size) {
         fflush(pic->filep);
         pic->filep = freopen(pic->filename, "ab", pic->filep);
         pic->OutputCount = 0;
         }
      pic->CopyCount   = 1;
      pic->RepeatCount = 0;
      }
}

static Pic *
NewPic(Viewpoint *eye, unsigned out_psize, unsigned cflag)
{
   Pic *tmp;

   if ((tmp = (Pic *)polyray_malloc(sizeof(Pic))) == NULL)
      error("Failed to allocate picture data\n");
   tmp->filename     = NULL;
   tmp->filep        = NULL;
   tmp->x            = eye->view_xres;
   tmp->y            = eye->view_yres;
   tmp->psize        = out_psize;
   tmp->cflag        = cflag;
   tmp->CopyCount    = 0;
   tmp->RepeatCount  = 0;
   tmp->OutputCount  = 0;
   tmp->ColumnCount  = 0;
   tmp->resume       = NULL;
   tmp->ofilep       = NULL;
   tmp->line_offsets = NULL;
   tmp->line_flags   = NULL;
   return tmp;
}

#define TARGA_HEADER_LEN 18

static int
ReadTGAHeader(FILE *filep, unsigned *idlen, unsigned *ftype,
              unsigned *cmlen, unsigned *cmsiz, unsigned *width,
              unsigned *length, unsigned *psize, unsigned *orien)
{
   unsigned char tgaheader[TARGA_HEADER_LEN];

   if (fseek(filep, 0, SEEK_SET))
      /* Failed to seek to the beginning of the file */
      return 0;

   if (fread(tgaheader, TARGA_HEADER_LEN, 1, filep) != 1)
      /* Failed to read an entire header */
      return 0;

   *idlen  = tgaheader[ 0];
   *ftype  = tgaheader[ 2];
   *cmlen  = tgaheader[ 5] + (tgaheader[ 6] << 8);
   *cmsiz  = tgaheader[ 7];
   *width  = tgaheader[12] + (tgaheader[13] << 8);
   *length = tgaheader[14] + (tgaheader[15] << 8);
   *psize  = tgaheader[16];
   *orien  = tgaheader[17];
   return 1;
}

#define TAG_ENTRY_LEN 10
#define TARGA_FOOTER_LEN 26
static char *TGA_SIGNATURE = "TRUEVISION-XFILE.";

static int
ReadTGAFooter(FILE *filep, int yres, unsigned char *line_flags)
{
   char tgafooter[TARGA_FOOTER_LEN];
   unsigned char tag_entry[TAG_ENTRY_LEN];
   long extension_offset, directory_offset;
   long polyray_tag_offset, polyray_tag_length;
   int i, j, lowb, highb, tag_count, tag_id;

   if (fseek(filep, -TARGA_FOOTER_LEN, SEEK_END))
      /* Failed to seek to the end of the file */
      return 0;

   if (fread(tgafooter, TARGA_FOOTER_LEN, 1, filep) != 1)
      /* Failed to read an entire header */
      return 0;

   if (strcmp(TGA_SIGNATURE, &tgafooter[8]))
      /* This TGA file doesn't have any additional information */
      return 0;

   extension_offset  =  (long)tgafooter[0] +
                       ((long)tgafooter[1] <<  8) +
                       ((long)tgafooter[2] << 16) +
                       ((long)tgafooter[3] << 24);
   directory_offset  =  (long)tgafooter[0] +
                       ((long)tgafooter[1] <<  8) +
                       ((long)tgafooter[2] << 16) +
                       ((long)tgafooter[3] << 24);

   /* If there is a developers area, then let's look for a table that
      indicates which lines have been rendered */
   if (fseek(filep, directory_offset, SEEK_SET))
      /* Failed to seek to the developers area */
      return 0;

   if ((lowb = fgetc(filep)) == EOF || (highb = fgetc(filep)) == EOF)
      return 0;

   tag_count = lowb + (highb << 8);

   for (i=0;i<tag_count;i++) {
      if (fread(tag_entry, TAG_ENTRY_LEN, 1, filep) != 1)
         /* Failed to read developers tag */
         return 0;
      tag_id = tag_entry[0] + (tag_entry[1] << 8);
      if (tag_id == POLYRAY_RENDERED_LINE_TAG) {
         polyray_tag_offset  =  (long)tag_entry[2] +
                               ((long)tag_entry[3] <<  8) +
                               ((long)tag_entry[4] << 16) +
                               ((long)tag_entry[5] << 24);
         polyray_tag_length  =  (long)tag_entry[6] +
                               ((long)tag_entry[7] <<  8) +
                               ((long)tag_entry[8] << 16) +
                               ((long)tag_entry[9] << 24);

         if (polyray_tag_length != yres) {
            warning("Old image has improper # of render tags %d vs %d",
                    (int)polyray_tag_length, (int)yres);
            return 0;
            }

         if (fseek(filep, directory_offset, SEEK_SET))
            /* Failed to seek to the rendered line flag table */
            return 0;

         /* Read the set of flags inicating which lines have/have not
            been rendered */
         for (j=0;j<polyray_tag_length;j++) {
            if ((lowb = fgetc(filep)) == EOF) {
               warning("Failed to read render tags from old image");
               return 0;
               }
            line_flags[j] = lowb;
            }
         /* Read in the render tags properly */
         return 1;
         }
      }

   /* Didn't find a table with render tags in it. */
   return 0;
}

/* Routines for initializing/resuming an image file:
      TGAOpen - Allocates the Pic data structure, finds out where
                the scanline are in the old file, and opens the
                new image file for output.
*/
Pic *
TGAOpen(char *filename, Viewpoint *eye, int resume, int cflag,
        unsigned int out_psize)
{
   Pic *tmp;
   unsigned h, i, j, out_ftype, line_count;
   unsigned ftype, idlen, cmlen, cmsiz, psize, orien;
   unsigned cmbytes=0, width, length;
   char tfilename[256];
   unsigned char rc, gc, bc, oc;
   long last_valid_byte, pixels, image_size;
   unsigned char tgaheader[TARGA_HEADER_LEN];
   int footer_flag;

   if (out_psize == 8)
      out_ftype = (cflag ? 11 : 3);
   else
      out_ftype = (cflag ? 10 : 2);

   tmp = NewPic(eye, out_psize, cflag);
   tmp->filename = (char *)polyray_malloc(strlen(filename)+1);
   if (tmp->filename == NULL)
      error("Failed to allocate picture data\n");
   //strcpy(tmp->filename, filename);
   strncpy(tmp->filename,filename,sizeof(tmp->filename)-1);//leave room for null cm160519
   tmp->filename[sizeof(tmp->filename)-1]='\0';//cm160519

   tmp->line_flags = polyray_malloc(tmp->y * sizeof(unsigned char));
   for (i=0;i<tmp->y;i++)
      tmp->line_flags[i] = 0;
   tmp->resume = NULL;

   if (resume) {
      /* Look for the old file and see if it is really there.
         If so, then check it's image characteristics against
         what we want for this image.  If they match then move
         the old image to a temporary file. */
      tmp->ofilep = PathFileOpen(POLYRAY_PATH_STRING, filename, "rb");
      if (tmp->ofilep == NULL)
         /* Failed to open the old file */
         goto create_new_file;

      if (!ReadTGAHeader(tmp->ofilep, &idlen, &ftype, &cmlen, &cmsiz,
                         &width, &length, &psize, &orien))
         /* The old file is severly broken, simply ignore it */
         goto create_new_file;

      /* We just skipped the header */
      last_valid_byte = TARGA_HEADER_LEN;

      /* Check it's height, width, etc. */
      if (eye->view_xres != width || eye->view_yres != length) {
         warning("Overriding original image (%dx%d) to match new (%dx%d)\n",
                 width, length, eye->view_xres, eye->view_yres);
         fclose(tmp->ofilep);
         tmp->ofilep = NULL;
         goto create_new_file;
         }

      if (out_ftype != ftype) {
         warning("Overriding original file type (%d) to match new (%d)\n",
                 ftype, out_ftype);
         fclose(tmp->ofilep);
         tmp->ofilep = NULL;
         goto create_new_file;
         }

      if (orien != 0x20) {
         warning("Original file orientation doesn't match new\n");
         fclose(tmp->ofilep);
         tmp->ofilep = NULL;
         goto create_new_file;
         }

      if (out_psize != psize) {
         warning("Overriding original Bits/pixel (%d) to match new (%d)\n",
                 psize, out_psize);
         fclose(tmp->ofilep);
         tmp->ofilep = NULL;
         goto create_new_file;
         }

      /* Skip over the picture information */
      if (fseek(tmp->ofilep, idlen, SEEK_CUR)) {
         warning("Restart image is broken in header, image will be rebuilt\n");
         goto create_new_file;
         }
      else
         last_valid_byte += (long)idlen;

      /* Figure out how many bytes in the color map */
           if (cmsiz <=  8) cmbytes = (long)cmlen ;
      else if (cmsiz <= 16) { times2(cmlen,cmbytes); } /* cmbytes = (long)cmlen * 2L; */
      else if (cmsiz <= 24) { times3(cmlen,cmbytes); } /* cmbytes = (long)cmlen * 3L; */
      else if (cmsiz <= 32) cmbytes = (long)cmlen * 4L;
      else error("Can't handle color maps with %d bits/pixel\n", cmlen);

      /* Skip over the the color map */
      if (fseek(tmp->ofilep, cmbytes, SEEK_CUR)) {
         warning("Restart image broken in the color map\n");
         goto create_new_file;
         }
      else
         last_valid_byte += cmbytes;

      /* Before reading the image, we will look for a file footer that
         describes how much of this image has been previously rendered.
         The last 26 bytes of the file should contain pointers to the
         additional information we need. */
      if (ReadTGAFooter(tmp->ofilep, length, tmp->line_flags))
         footer_flag = 1;
      else
         footer_flag = 0;
      if (fseek(tmp->ofilep, last_valid_byte, SEEK_SET)) {
         warning("Failed to read image, old image possibly damaged in footer");
         goto create_new_file;
         }

      /* We appear to have a good file all the way up to the image
         information.  At this point we allocate an array to hold
         offsets into the file.  Each offset in the array points to
         the beginning of a scan line of pixels. */
      tmp->line_offsets = polyray_malloc((eye->view_yres+1) * sizeof(long));
      tmp->line_offsets[0] = last_valid_byte;
      line_count = 1;

      /* Read through the entire file, determining the starts of
         each line. */
      if (tmp->cflag) {
         pixels = 0;
         image_size = (long)eye->view_yres * (long)eye->view_xres;
         while (pixels < image_size)
            /* Grab a header */
            if ((h = fgetc(tmp->ofilep)) == EOF)
               /* At end of file - quit where we are now */
               break;
            else if (h & 0x80) {
               /* Repeat buffer */
               h &= 0x7F;
               if (get_pixel(tmp->ofilep, tmp->psize, &rc, &gc, &bc, &oc)) {
                  /* Valid repeat packet. */
                  last_valid_byte += 1 + tmp->psize;
                  pixels += h + 1;
                  if ((pixels % eye->view_xres) == 0) {
                     if (!footer_flag)
                        tmp->line_flags[line_count-1] = 1;
                     tmp->line_offsets[line_count] = last_valid_byte;
                     line_count += 1;
                     }
                  }
               else
                  break;
               }
            else {
               /* Copy buffer */
               for (i=0,j=1;i<=h && j;i++)
                  /* Read a bunch of pixels */
                  if (!get_pixel(tmp->ofilep, tmp->psize, &rc, &gc, &bc, &oc))
                     j = 0;
               if (!j)
                  break;
               last_valid_byte += 1 + tmp->psize * (h + 1);
               pixels += h + 1;
               if ((pixels % eye->view_xres) == 0) {
                  if (!footer_flag)
                     tmp->line_flags[line_count-1] = 1;
                  tmp->line_offsets[line_count] = last_valid_byte;
                  line_count += 1;
                  }
               }
         }
      else {
         /* Read the entire file */
         for (i=0;i<eye->view_yres;i++) {
            for (j=0;j<eye->view_xres;j++) {
               if (!get_pixel(tmp->ofilep, tmp->psize, &rc, &gc, &bc, &oc))
                  break;
               last_valid_byte += tmp->psize;
               }
            if (!footer_flag)
               tmp->line_flags[line_count-1] = 1;
            tmp->line_offsets[line_count] = last_valid_byte;
            line_count += 1;
            }
         }

      /* Set the line offsets to 0 for any incomplete lines in
         the old image file */
      for (i=line_count;i<eye->view_yres;i++)
         tmp->line_offsets[i] = 0;

      /* Now that we have the offsets into the image, move the
         old image into a temporary file */
      fclose(tmp->ofilep);
      //tmpnam(tfilename);
      char tmp_name[] = "/tmp/polyray.XXXXXX";

//	  tmp_fd = tmpfile_s(&stream);
	
	  	  int tmp_fd = mkstemp(tmp_name);
		  if (tmp_fd == -1) {
			  error("error: failed to create temporary file\n");
		  }  	  

	  	  (void) close(tmp_fd);
	  	  strncpy(tfilename,tmp_name,255); //the above replaces code for tmpnam
      //cm160519
      i = strlen(tfilename) + 1;
      tmp->resume = polyray_malloc(i * sizeof(char));
      //strcpy(tmp->resume,tfilename);
      //strncpy(tmp->resume, tfilename,i-1);//cm160519
      memcpy(tmp->resume,tfilename,i);
      tmp->resume[sizeof(tmp->resume)-1]='\0';//cm160519
      if (rename(filename, tmp->resume)) {
         warning("Failed to save old image\n");
         polyray_free(tmp->resume);
         polyray_free(tmp->line_offsets);
         tmp->resume = NULL;
         tmp->line_offsets = NULL;
         }
      if ((tmp->ofilep = PathFileOpen(POLYRAY_PATH_STRING, tfilename, "rb")) == NULL)
         error("File access error\n");
      }
   else {
      tmp->ofilep = NULL;
      tmp->resume = NULL;
      tmp->line_offsets = NULL;
      }

create_new_file:
   /* Open the new output image file and write the header
      information to it. */
   tmp->filep = fopen(filename, "wb");
   /* tmp->filep = PathFileOpen(POLYRAY_PATH_STRING, filename, "wb"); */
   if (tmp->filep == NULL)
      error("Failed to open image file: %s\n", filename);

    /* still doing tgaheader for now even for PPM! - CM 10mar2000*/
    memset(tgaheader, 0, 18);
    tgaheader[ 0] = strlen(polyray_id)+1;
    tgaheader[ 2] = out_ftype;
    tgaheader[12] = eye->view_xres & 0xFF;
    tgaheader[13] = (eye->view_xres >> 8) & 0xFF;
    tgaheader[14] = eye->view_yres & 0xFF;
    tgaheader[15] = (eye->view_yres >> 8) & 0xFF;
    tgaheader[16] = out_psize;
    tgaheader[17] = 0x20; /* Always create a top to bottom image */

   /*CM added 10/mar/2000 */
   if (outformat == OUT_TARGA) {
   /*CM end*/

    /* Write the Targa file header */
    fwrite(tgaheader, 18, 1, tmp->filep);

    /* Write out polyray info */
    fwrite(polyray_id, strlen(polyray_id)+1, 1, tmp->filep);
   /* CM added 10/mar/2000 */
   } else { /* it's PPM raw format */
     fprintf(tmp->filep,"P6\n#%s\n%d %d\n255\n",polyray_id,eye->view_xres,eye->view_yres);
   }
   /* CM end */

   /* Make sure the counters are properly initialized */
   tmp->CopyCount = 0;
   tmp->RepeatCount = 0;
   tmp->OutputCount = 0;
   tmp->ColumnCount = 0;

   return tmp;
}

/* Put a quantized color into the image buffer */
static void
set_buffer_color(Viewpoint *eye, int row, int col,
                 unsigned char rc, unsigned char gc,
                 unsigned char bc, unsigned char oc)
{
/* CM 10/mar/2000 - casted view_yxres to ints for comparison */
   if (row < 0 || row > (int) eye->view_yres ||
       col < 0 || col > (int) eye->view_xres  ||
       eye->SBuffer[row] == NULL) {
      warning("Bad pixel (%d,%d) in set_buffer_color", row, col);
      }
   else {
      eye->SBuffer[row][col].r = rc;
      eye->SBuffer[row][col].g = gc;
      eye->SBuffer[row][col].b = bc;
      eye->SBuffer[row][col].o = oc;
      }
}

/* Read a line of pixels from the old image.  We are assured that the
   pixels are really there as long as the value in line_offsets[y] is
   non-zero.  If bad things happened to the file between the time we
   first looked at it and now, then this routine will fail. */
int
get_old_image_line(Viewpoint *eye, Pic *pic, unsigned y)
{
   unsigned int i,h,j,pixels;
   unsigned char rc, gc, bc, oc;

   if (pic->line_offsets == NULL)
      return 0;
   else if (pic->line_offsets[y] > 0) {
      if (fseek(pic->ofilep, pic->line_offsets[y], SEEK_SET))
         return 0;
      /* Read through the entire file, determining the starts of
         each line. */
      if (pic->cflag) {
         pixels = 0;
         while (pixels < eye->view_xres)
            /* Grab a header */
            if ((h = fgetc(pic->ofilep)) == EOF)
               /* At end of file - quit where we are now */
               return 0;
            else if (h & 0x80) {
               /* Repeat buffer */
               h &= 0x7F;
               if (get_pixel(pic->ofilep, pic->psize, &rc, &gc, &bc, &oc)) {
                  /* Valid repeat packet. */
                  for (j=0;j<h+1;j++)
                     set_buffer_color(eye, y, pixels+j, rc, gc, bc, oc);
                  pixels += h + 1;
                  }
               else
                  return 0;
               }
            else {
               /* Copy buffer */
               for (i=0,j=1;i<=h && j;i++)
                  /* Read a bunch of pixels */
                  if (get_pixel(pic->ofilep, pic->psize, &rc, &gc, &bc, &oc))
                     set_buffer_color(eye, y, pixels+i, rc, gc, bc, oc);
                  else
                     return 0;
               pixels += h + 1;
               }
         }
      else
         /* Read a row of uncompressed pixels */
         for (j=0;j<eye->view_xres;j++)
            if (!get_pixel(pic->ofilep, pic->psize, &rc, &gc, &bc, &oc))
               return 0;
            else
               set_buffer_color(eye, y, j, rc, gc, bc, oc);
      }
   else
      return 0;

   /* Read in the entire line, since we may be doing a filtered or
      antialised image, we duplicate the last pixel in the line. */
   set_buffer_color(eye, y, eye->view_xres, rc, gc, bc, oc);
   return 1;
}

/* Flush any pending pixels to the file */
static void
flush_pic_buffers(Pic *pic)
{
   int i, j;

   if (pic->cflag && (pic->CopyCount > 0 || pic->RepeatCount > 0)) {
      /* Flush the current contents to the file */
      if (pic->CopyCount > 0) {
         fputc((unsigned char)pic->CopyCount-1, pic->filep);
         for (i=0;i<pic->CopyCount;i++)
            for (j=0;j<pic->psize/8;j++)
               fputc(pic->buffer[i][j], pic->filep);
         pic->CopyCount = 0;
         }
      else if (pic->RepeatCount > 0) {
         fputc(((unsigned char)(pic->RepeatCount-1)) | 0x80, pic->filep);
         for (j=0;j<pic->psize/8;j++)
            fputc(pic->buffer[0][j], pic->filep);
         pic->RepeatCount = 0;
         }
      }
}

/*
   Pad out the image file if we weren't done (use pixels from the
   old image if we were resuming an image

   Close and remove the old image file (if one).

   Close the image file
*/
void
TGAClose(Pic *pic)
{
   unsigned int i, j;

   /* Since we only emit pixels for entire lines, there should be no
      leftovers in either the copy or repeat buffers.  If so, then
      print a warning and fix. */
   flush_pic_buffers(pic);

   /* If this is a brand new image, then we write out blank lines
      for any part of the image that wasn't rendered. */
   if (pic->resume == NULL)
      for (i=0;i<pic->y;i++) {
         if (pic->line_flags[i])
            continue;
         for (j=0;j<pic->x;j++) {
            if (pic->cflag)
               put_buffer(pic, 0, 0, 0, 0);
            else
               put_pixel(pic, 0, 0, 0, 0);
            }
         }

   fclose(pic->filep);
   polyray_free(pic->filename);
   if (pic->ofilep != NULL) {
      fclose(pic->ofilep);
      remove(pic->resume);
      polyray_free(pic->resume);
      }
   if (pic->line_flags != NULL)
      polyray_free(pic->line_flags);
   if (pic->line_offsets != NULL)
      polyray_free(pic->line_offsets);
   polyray_free(pic);
}

/* Dump out the most recently rendered set of scan lines */
void
TGADump(Viewpoint *eye, Pic *pic, unsigned int ybeg, unsigned int yend)
{
   unsigned char r, g, b, o;
   unsigned t;
   unsigned row, col;
   int filter_flag;

   if (antialias > 0 && Rendering_Method != RAY_TRACING)
      filter_flag = 1;
   else
      filter_flag = 0;
   yend = MIN(yend, eye->view_yres-1);

/* printf("Dump: %d - %d\n", ybeg, yend); */
   for (row=ybeg;row<=yend;row++) {
      if (eye->SBuffer[row] == NULL)
         error("Bad row: %d\n", row);
      for (col=0;col<eye->view_xres;col++) {
         if (filter_flag) {
            t =  eye->SBuffer[row][col].r +
                 eye->SBuffer[row][col+1].r +
                 eye->SBuffer[row+1][col].r +
                 eye->SBuffer[row+1][col+1].r;
            r = t >> 2;
            t =  eye->SBuffer[row][col].g +
                 eye->SBuffer[row][col+1].g +
                 eye->SBuffer[row+1][col].g +
                 eye->SBuffer[row+1][col+1].g;
            g = t >> 2;
            t =  eye->SBuffer[row][col].b +
                 eye->SBuffer[row][col+1].b +
                 eye->SBuffer[row+1][col].b +
                 eye->SBuffer[row+1][col+1].b;
            b = t >> 2;
            t =  eye->SBuffer[row][col].o +
                 eye->SBuffer[row][col+1].o +
                 eye->SBuffer[row+1][col].o +
                 eye->SBuffer[row+1][col+1].o;
            o = t >> 2;
            }
         else {
            r = eye->SBuffer[row][col].r;
            g = eye->SBuffer[row][col].g;
            b = eye->SBuffer[row][col].b;
            o = eye->SBuffer[row][col].o;
            }

         if (pic->cflag)
            put_buffer(pic, r, g, b, o);
         else
            put_pixel(pic, r, g, b, o);
         }
      /* Tag the line as having been written */
      pic->line_flags[row] = 1;
      }
   flush_pic_buffers(pic);
}

/* Dump out the most recently rendered set of scan lines IN RAW PPM format */
void
PPMDump(Viewpoint *eye, Pic *pic, unsigned int ybeg, unsigned int yend)
{
   unsigned char r, g, b, o;
   unsigned t;
   unsigned row, col;
   int filter_flag;

   if (antialias > 0 && Rendering_Method != RAY_TRACING)
      filter_flag = 1;
   else
      filter_flag = 0;
   yend = MIN(yend, eye->view_yres-1);

/* printf("Dump: %d - %d\n", ybeg, yend); */
   for (row=ybeg;row<=yend;row++) {
      if (eye->SBuffer[row] == NULL)
         error("Bad row: %d\n", row);
      for (col=0;col<eye->view_xres;col++) {
         if (filter_flag) {
            t =  eye->SBuffer[row][col].r +
                 eye->SBuffer[row][col+1].r +
                 eye->SBuffer[row+1][col].r +
                 eye->SBuffer[row+1][col+1].r;
            r = t >> 2;
            t =  eye->SBuffer[row][col].g +
                 eye->SBuffer[row][col+1].g +
                 eye->SBuffer[row+1][col].g +
                 eye->SBuffer[row+1][col+1].g;
            g = t >> 2;
            t =  eye->SBuffer[row][col].b +
                 eye->SBuffer[row][col+1].b +
                 eye->SBuffer[row+1][col].b +
                 eye->SBuffer[row+1][col+1].b;
            b = t >> 2;
            t =  eye->SBuffer[row][col].o +
                 eye->SBuffer[row][col+1].o +
                 eye->SBuffer[row+1][col].o +
                 eye->SBuffer[row+1][col+1].o;
            o = t >> 2;
            }
         else {
            r = eye->SBuffer[row][col].r;
            g = eye->SBuffer[row][col].g;
            b = eye->SBuffer[row][col].b;
            o = eye->SBuffer[row][col].o;
            }

         if (pic->cflag)
            put_buffer(pic, r, g, b, o);
         else
            put_pixel(pic, r, g, b, o);
         }
      /* Tag the line as having been written */
      pic->line_flags[row] = 1;
      }
   flush_pic_buffers(pic);
}


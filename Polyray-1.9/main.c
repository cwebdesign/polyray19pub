/*
   Main driver for ray-tracer

  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Additions (C) 1999-2024, Clyde Meli, All rights reserved.

  This software may be used for any private and non-commercial
  use.

  You may not distribute this software, in whole or in part,
  for any commercial purpose, without the express consent of
  the authors.

  There is no warranty or other guarantee of fitness of this software
  for any purpose.  It is provided solely "as is".

*/
#include <time.h>

#define inmain
#include "defs.h"
#include "vector.h"
#include "io.h"
#include "memory.h"
#include "psupport.h"
#include "particle.h"
#include "light.h"
#include "sweep.h"
#include "scan.h"
#include "screen.h"
#include "display.h"
#include "pic.h"
#include "symtab.h"
#include "bound.h"
#include "image.h"
#include "intersec.h"
#include <locale.h> //CM: for mingw bugfix
#include "unixcompat.h"

#include "bound.h"//for testing bound

void setupcolors2(void); //CM

/* Platform/compiler specific include files */
#if defined( MAC )
#include <profile.h>
#endif

/*CM added 10/mar/2000 */
int outformat = OUT_TARGA;
/*CM end */

static int skip_frame = -1;

/*CM added 28/feb/2002 */
#if defined( WINTARGET )
int main(int argc, char **argv);

// Windows global variables

HWND hWnd; // used for future Windows capabilities
HINSTANCE hInst; // and used in InitWindow
int CmdShow; //     "
#endif
/*CM end */


/*CM added 2/nov/2002 */
void hitanykey()
{
#if defined (WINTARGET)

#else

//#ifndef unix
//#if defined( MAC )
//   message("Press mouse to continue\n");
//#else
//   message("Press any key to continue\n");
//#endif
//   while (!kbhit()) ;
//#if !defined( MAC )
//   (void)getch();
//#endif
//#endif

#endif

  polyray_pause();//cm160519
}
/*CM end */

static void show_arguments(void)
{
 dumpversion();

message("Syntax: polyray filename (options)\n");
message("\n File options:\n");
message("     -o outputfile      [ default is 'out.tga'                  ]\n");
message("     -p pixelsize       [ bits/pixel [8, 16, 24, 32]            ]\n");
message("     -d                 [ Render as a depth map                 ]\n");
message("     -u                 [ Turn off RLE compression of output    ]\n");
message("     -x columns         [ trace 'colunms' pixels per row        ]\n");
message("     -y lines           [ trace 'lines' rows                    ]\n");
message("     -B                 [ flush the output file every line      ]\n");
message("     -b count           [ flush every 'count' pixels            ]\n");
message("     -R                 [ Resume an old trace                   ]\n");
message("     -z y0 y1           [ Render from line y0 to line y1        ]\n");
message("     -G                 [ output in RAW PPM format (not yet)    ]\n");
fflush(message_log);
hitanykey();
message("\n Rendering options:\n");
message("     -r method          [ Render: 0=Ray,1=Scan,2=Wire,3=Hidden  ]\n");
message("                        [         4=Gourad,5=Raw,6=uv           ]\n");
message("     -q flags           [ Turn on/off various shading options:  ]\n");
message("                        [ 1=shadow, 2=reflect, 4=transmit, 8=two]\n");
message("                        [ sides, 16=check uv, 32=flip normals   ]\n");
message("                        [ 63=all flags                          ]\n");
message("     -N                 [ Don't generate an image file          ]\n");
message("     -M kbytes          [ Max # of KBytes for image buffer      ]\n");
fflush(message_log);
hitanykey();

message("\n");
message("\nStatus options:\n");
message("     -t freqency        [ Status: 0=none,1=totals,2=line,3=pixel]\n");
message("\n Antialiasing options for ray-tracing:\n");
message("     -a mode            [ AA: 0=none,1=filter,2-4=adaptive      ]\n");
message("     -T threshold       [ Threshold to start oversampling       ]\n");
message("     -S samples         [ Use 'samples' rays/pixel              ]\n");
message("\n Optimization options:\n");
message("     -O optimizer       [ 0 = none, 1 = slabs                   ]\n");
message("\n Display option:\n");
message("     -V mode            [ 0=none, 1-5=8bit, 6-10=15bit,         ]\n");
message("                        [ 11-15=16bit, 16-20=24bit, 21-22=4bit  ]\n");
message("     -P pallette        [ 0=grey, 1=332, 2=666, 3=EGA           ]\n");
message("     -e start           [ Start position in VGA pallette        ]\n");
message("     -W                 [ Wait for key before clearing display  ]\n");
message("     -D flag            [ 0=no video dither, 1 = use dither     ]\n");
message("\n Frame counter option:\n");
message("     -F start_frame     [ Skip frames until start_frame         ]\n");
message("\n Abort option:\n");
message("     -Q abort_option    [ 0 = no abort, 1 = check by pixel      ]\n");
message("                        [ 2 = by line/object                    ]\n");
message("\n");
fflush(message_log);
//BezierNormalTest();
//BuildAreaTableTestHarness();

}

/* get option letter from argument vector */
static int _optind = 2, /* index into parent argv vector */
           _optopt;     /* character checked for validity */
static char *optarg1,
            *optarg2;  /* argument(s) associated with option */

#define BADCH   (int)'?'
#define EMSG   ""

static int _getopt(int nargc, char **nargv, char *ostr)
{
   static char *place = EMSG; /* option letter processing */
   char *oli;                 /* option letter list index */

   if(!*place) {         /* update scanning pointer */
      if (_optind >= nargc || *(place = nargv[_optind]) != '-' || !*++place)
         return(EOF);
      if (*place == '-') {   /* found "--" */
         ++_optind;
         return(EOF);
         }
      }

   /* option letter okay? */
   if ((_optopt = (int)*place++) == (int)':' ||
       (_optopt == (int)'+') ||
       !(oli = strchr(ostr,_optopt))) {
      if(!*place) ++_optind;
      warning(": illegal option -- '%c'", (char)_optopt);
      return 0;
      }

   if (*++oli == ':') {
      /* need an argument */
      if (*place)
         optarg1 = place;   /* no white space */
      else if (nargc <= ++_optind) { /* no arg */
         place = EMSG;
         warning(": option requires an argument -- '%c'", (char)_optopt);
         return 0;
      }
       else optarg1 = nargv[_optind];   /* white space */
      place = EMSG;
      ++_optind;
      }
   else if (*oli == '+') {
      /* need two arguments */
      if (*place)
         optarg1 = place;   /* no white space */
      else if (nargc <= ++_optind) { /* no arg */
         place = EMSG;
         warning(": option requires an argument -- '%c'", (char)_optopt);
         return 0;
         }
      else
         optarg1 = nargv[_optind];   /* white space */
      if (nargc <= ++_optind) { /* no arg */
         place = EMSG;
         warning(": option requires two arguments -- '%c'", (char)_optopt);
         return 0;
         }
      else
         optarg2 = nargv[_optind];
      place = EMSG;
      ++_optind;
      }
   else {
      /* don't need argument */
      optarg1 = NULL;
      if (!*place) ++_optind;
   }
   return(_optopt);         /* dump back option letter */
}

static void
Polygon_Render(Viewpoint *eye, BinTree *Root)
{
   ostackptr objs;
   Object *obj;
   int i;
#ifdef unix
   char tmp[100];
#endif

   /* Read and render the objects */
   if (setjmp(abort_environ) == 0) {
      for (i=0,objs=Root->members.list;objs!=NULL;i++,objs=objs->next) {
         obj = objs->element;
         if (tickflag > 1)
            status("\r%d ", i);
#ifdef unix
         if (start_frame!=end_frame)
           sprintf(tmp,"F%d/%d, p%d/%ld", current_frame-start_frame,
                end_frame-start_frame, i, Root->members.count);
         else
           sprintf(tmp,"p%d/%ld",i,Root->members.count);
         //SpecialStatus(tmp);
#endif
         if ((Check_Abort_Flag == 2) && _kbhit()) {
#if defined( MAC )
            Abort_Flag = 1;
#else
            Abort_Flag = getchar();//getch()
#endif
            break;
            }
         render_prim(eye, Root, NULL, obj);
         }
      }
   else {
#if defined( MAC )
      Abort_Flag = 1;
#else
      if (_kbhit())
         Abort_Flag = getchar();
      else
         Abort_Flag = 1;
#endif
      }
}

static void
StartupDisplay(Viewpoint *eye, char *infilename)
{
   if (Display_Flag != 0)
      /* Initialize the display */
      display_init(eye, infilename);
}

static void
Startup_Statistics(char *instr, char *outstr, Viewpoint *eye)
{
   if (tickflag > 0) {
      status("Data: %s, Targa: %s\n", instr, outstr);
      status("Prims = %lu, Lights = %d, Memory: %lu\n",
             Root.members.count, nLights, nMalloc - nFree);
      status("Res: %dx%d, Aspect: %0.4lg, Shade: %x\n",
              eye->view_xres, eye->view_yres,
              eye->view_aspect, Global_Shade_Flag);
      if (end_frame > start_frame || total_frames != 0)
         status("Frame %d in [%d, %d]\n",
                current_frame, start_frame, end_frame);
      }
}

/* Dump out the statistics for this frame. */
static void
PrintStatistics(time_t ptime, time_t ttime, int wait)
{
   if (Display_Flag && Reset_Display_Flag &&
       ( (current_frame == end_frame) ||
        ((Check_Abort_Flag != 0) && Abort_Flag)) )
      display_close(wait);
   if (tickflag > 0) {
      status("preprocess time            %-8ld seconds\n", (long)ptime);
      status("tracing time               %-8ld seconds\n", (long)ttime);
      if (Rendering_Method == RAY_TRACING) {
         status("number of eye rays cast:   %-8lu\n", nRays);
         status("number of shadow rays:     %-8lu\n", nShadows);
         if (nReflected > 0)
            status("number of reflected rays:  %-8lu\n", nReflected);
         if (nRefracted > 0)
            status("number of refracted rays:  %-8lu\n", nRefracted);
         if (nTIR > 0)
            status("number of TIR rays:        %-8lu\n", nTIR);
         if (nJittered > 0)
            status("number of jittered rays:   %-8lu\n", nJittered);
         if (totalQueueResets > 0) {
            status("number of queue resets:    %-8lu\n", totalQueueResets) ;
            status("avg number of queues/ray:  %-8g\n", (Flt)nEnqueued /
                                                        (Flt)totalQueueResets) ;
            status("max queue size:            %-8lu\n", maxQueueSize) ;
            }
         status("number of shadow checks:   %-8lu\n", totalShadows);
         status("number of cache hits:      %-8lu\n", totalShadowCaches);
         status("number of bounds checked:  %-8lu\n", nChecked);
         status("number of bounds queued:   %-8lu\n", nEnqueued);
         status("number of objects checked: %-8lu\n", totalQueues);
         }
      }
}

static void
initialize_scan_data(Viewpoint *eye, Pic *pic,
                     int y_start, int y_end,
                     int Start_Line, int End_Line,
                     int alloc_flag)
{
   /* Set up the view transformation */
   if (eye->WS != NULL)
      polyray_free(eye->WS);
   eye->WS = Normalize_View(eye);
#if 0
printf("Y: %d - %d, Strip: %d, %d\n", y_start, y_end, Start_Line, End_Line);
#endif
   /* This is necessary so that drawing commands will work in
      non file generation modes.  Otherwise we could stuff this
      call into the Allocation routine below. */
   Initialize_Clipping(eye, MAX(y_start,Start_Line), MIN(y_end,End_Line));

   if (alloc_flag)
      Allocate_Scan_Buffers(eye, pic, y_start, y_end);
}

/* Between each generated frame, reclaim any memory that was allocated. */
static void
reset_data(void)
{
   /* Reset the global bean counters */
   Initialize_Bean_Counters();

   /* Allocate memory for the various data structures */
   Initialize_Symtab();

   Initialize_Lights();
}

static void
read_defaults(int argc, char **argv, char *outfilename,
              Viewpoint *eye, int *resume,
              int *wait, int *Start_Line, int *End_Line)
{
   static char *opts = "a:b:D:e:M:o:O:p:P:q:Q:r:S:t:T:V:x:y:z+F:BNGRduvW";
   int c;
   int line_buffer = 0;

   /* Set defaults for the output file */
   if (!filebaseflag)
      strcpy(outfilebase, "out");
   strcpy(outfilename, "out.tga");
   _optind = 2; /* Reset the index into parent argv vector */

   /* Loop through the command line options */
   while ((c = _getopt(argc, argv, opts)) != EOF) {
      switch (c) {
      case 'a':
         antialias = atoi(optarg1);
         if (antialias > 4)
            antialias = 4;
         else if (antialias < 0)
            antialias = 0;
         break;
      case 'b':
         if (line_buffer)
            warning("Line buffering already specified\n");
         else {
            buffer_update = 1;
            buffer_size = atoi(optarg1);
            }
         break;
      case 'B':
         if (buffer_update)
            warning("Pixel buffering already specified\n");
         else {
            line_buffer   = 1;
            buffer_update = 1;
            }
         break;
      case 'd':
         DepthRender = 1;
         break;
      case 'D':
         Dither_Flag = atoi(optarg1);
         if (Dither_Flag < 0 || Dither_Flag > 1)
            error("Dither flag must be either 0 (off) or 1 (on)\n");
         break;
      case 'e':
         Pallette_Start = atoi(optarg1);
         if (Pallette_Start < 0 || Pallette_Start > 240)
            error("First entry of pallette must be between 0 and 240\n");
         break;
      case 'F':
         skip_frame = atoi(optarg1);
         break;
      case 'G':
         outformat = OUT_RAWPPM;
         strcpy(outfilename, "out.ppm");
         break;

      case 'M':
         MaxBufferRAM = 1024L * atoi(optarg1);
         break;
      case 'N':
         File_Generation_Flag = 0;
         break;
      case 'o':
         strcpy(outfilename, optarg1);
         break;
      case 'O':
         Optimizer = atoi(optarg1);
         if (Optimizer < 0 || Optimizer > 1)
            error("Optimizer must be one of: 0 [none], 1 [slabs]\n");
         break ;
      case 'p':
         pixelsize = atoi(optarg1);
         if (pixelsize != 8 && pixelsize != 16 &&
             pixelsize != 24 && pixelsize != 32)
            error("Pixel size must be 8, 16, 24, or 32 bits\n");
         break;
      case 'P':
         Pallette_Flag = atoi(optarg1);
         if (Pallette_Flag < 0 || Pallette_Flag > 3)
            error("Pallette must be one of: 0 [grey], 1 [8-8-4], 2 [6-6-6], 3 [4 bit]\n");
         break;
      case 'q':
         Global_Shade_Flag = atoi(optarg1);
         if (Global_Shade_Flag > ALL_SHADE_FLAGS)
            error("Too many bits set in shade flag (must be less than %d\n",
                  ALL_SHADE_FLAGS);
         break ;
      case 'Q':
         Check_Abort_Flag = atoi(optarg1);
         if (Check_Abort_Flag < 0 || Check_Abort_Flag > 2) {
            warning("Abort flag must be between 0 and 2");
            Check_Abort_Flag = 1;
            }
         break ;
      case 'r':
         Rendering_Method = atoi(optarg1) + RAY_TRACING;
         if (Rendering_Method < 1 || Rendering_Method > LAST_RENDER_MODE)
            error("Rendering must be one of: 0 [raytrace], 1 [scan], 2 [wire], 3 [hidden line], 4 [gourad], 5 [raw tri], 6 [uv tri]\n");
         if (Rendering_Method == RAW_TRIANGLES ||
             Rendering_Method == UV_TRIANGLES ||
             Rendering_Method == CSG_TRIANGLES)
            File_Generation_Flag = 0;
         break;
      case 'R':
         *resume = 1;
         break;
      case 'S':
         maxsamples = atoi(optarg1);
         if (maxsamples <= 0) {
            warning("Samples must be > 0, reset to 4\n");
            maxsamples = 4;
            }
         break;
      case 'T':
         antialias_threshold = atof(optarg1);
         if (antialias_threshold < 0.0) {
            warning("Antialias threshold must be >= 0.0, reset to 0.02\n");
            antialias_threshold = 0.02;
            }
         antialias_threshold *= antialias_threshold;
         break ;
      case 't':
         tickflag = atoi(optarg1);
         if (tickflag == 0) status_flag = 0;
         break;
      case 'u':
         pixel_encoding = 0;
         break;
      case 'V':
         Display_Flag = atoi(optarg1);
         if (Display_Flag < 0) {
            Display_Flag = -Display_Flag;
            Reset_Display_Flag = 0;
            }
         else if (Display_Flag > FIRST_4BIT_MODE+1)
            error("Unsupported display mode");
         else
            Reset_Display_Flag = 1;
         break;
      case 'W':
         *wait = 1;
         break ;
      case 'x':
         eye->view_xres = atoi(optarg1);
         break;
      case 'y':
         eye->view_yres = atoi(optarg1);
         break;
      case 'z':
         *Start_Line = atoi(optarg1);
         *End_Line = atoi(optarg2);
         break;
      }
   }

   if (line_buffer)
      buffer_size = eye->view_xres;
}

static void
set_line_boundaries(Viewpoint *eye, int *Start_Line, int *End_Line)
{
   if (*Start_Line < 0) {
      eye->view_ystart = 0;
      *Start_Line = 0;
      }
   else
      eye->view_ystart = *Start_Line;
   if (*End_Line < 0) {
      eye->view_yend = eye->view_yres;
      *End_Line = eye->view_yres;
      }
   else
      eye->view_yend = *End_Line;

   if (eye->view_ystart > eye->view_yend) {
      warning("Start line of image %d is after end line %d\n",
            eye->view_ystart, eye->view_yend);
      *Start_Line = -1;
      *End_Line   = -1;
      }
   else if (eye->view_ystart >= eye->view_yres) {
      warning("Start line of image %d is after end of image %d\n",
            eye->view_ystart, eye->view_yres);
      *Start_Line = -1;
      *End_Line   = -1;
      }
   else if (eye->view_yend > eye->view_yres) {
      warning("End line of image %d is too high (reset to %d)\n",
              eye->view_yend, eye->view_yres);
      eye->view_yend = eye->view_yres;
      *End_Line = eye->view_yres;
      }
}

static void
render_scene(int argc, char **argv, Viewpoint *eye, char *infilename,
             char *outfilename, char *outfilebase, int resume,
             time_t *atime, int *wait, int Start_Line, int End_Line)
{
   Pic *pic;
   time_t ptime, ttime;
   unsigned int y;
   long ram_needed, max_lines;

   /* At last we get to the main driver loop. */
   for (;;) {
      Start_Line = End_Line = -1;

      /* If we are on the second (or later) frame then reset various
         pieces of data */
      if (Parsed_Flag)
         reset_data();

      /* Parse the data file */
#ifdef DEBUG
printf("parsing");
#endif
      ReadSceneFile(infilename);
#ifdef DEBUG
printf("parsed");
#endif

      /* Use command line params to override values that appeared
         in either the ini file or the data file */
      read_defaults(argc, argv, outfilename, eye, &resume,
                    wait, &Start_Line, &End_Line);

      /* If the amount of RAM to use hasn't been specified, then
         assume that the entire image buffer should be allocated. */
      if (MaxBufferRAM < 0)
         MaxBufferRAM = 8L * (eye->view_xres + 1) * (eye->view_yres + 1);

      /* Determine the line boundaries for this image */
      set_line_boundaries(eye, &Start_Line, &End_Line);

      /* If the output of this render is a depth file then make sure
         that the file is neither RLE compressed nor antialiased. */
      if (DepthRender == 1) { pixel_encoding = 0; antialias = 0; }

      /* If this is the first pass, then we need to set up the animation
         counters. */
      if (!Parsed_Flag) {
         Parsed_Flag = 1;
         if (end_frame < 0) end_frame = total_frames;
         current_frame = start_frame;
         if (tickflag > 0)

          dumpversion();

         }

      /* Allocate and initialize memory, set the global shading flags. */
      if (Rendering_Method == SCAN_CONVERSION ||
          Rendering_Method == HIDDEN_LINE ||
          Rendering_Method == GOURAD_SHADE ||
          (Rendering_Method == WIRE_FRAME && File_Generation_Flag != 0)) {
         /* By default there is no shadow/reflect/transmit
            performed for polygonal scenes */
         if (Global_Shade_Flag & UNSET_SFLAG)
            Global_Shade_Flag = TWO_SIDED_SURFS | NORMAL_CORRECT;
         }
      else if (Global_Shade_Flag & UNSET_SFLAG)
         /* By default check almost everything when raytracing */
         Global_Shade_Flag = SHADOW_CHECK | REFLECT_CHECK | TRANSMIT_CHECK |
                             UV_CHECK | CAST_SHADOW | NORMAL_CORRECT;

      if (Global_Shade_Flag & SHADOW_CHECK)
         Initialize_Light_Caches();

      if (end_frame > start_frame || total_frames != 0)
         build_outfile_name(outfilebase, outfilename);

      InstantiateParticles(eye);

      if (current_frame >= skip_frame) {
         /* This section performs the rendering of a single
            frame.  An inner loop is required to allow for the
            rendering of a strip of the image at a time. */
         StartupDisplay(eye, infilename);
         AddLightObjects(&Root);

         Startup_Statistics(infilename, outfilename, eye);

         time(&ptime);

         /* Start by opening a file to hold the image.  If it already
            exists, then the image characteristics stored in the file
            will be used for the final image. */
         if (File_Generation_Flag)
            pic = TGAOpen(outfilename, eye, resume, pixel_encoding,
                          pixelsize);
         else
            pic = NULL;

         if ((Optimizer > 0) &&
             ((Rendering_Method == RAY_TRACING) ||
              ((Rendering_Method == GOURAD_SHADE ||
                Rendering_Method == SCAN_CONVERSION) &&
               (Global_Shade_Flag &
                  (SHADOW_CHECK | REFLECT_CHECK | TRANSMIT_CHECK))))) {
            /* We are either ray tracing or we have shading options
               in a scan conversion mode that may lead to a recursive
               call to the ray tracer.  In either case if slab
               optimization is turned on we have to create the slabs. */
            BuildBoundingSlabs(&Root);

            /* Create a list of all objects (prims and composites) that
               contain the eye within their bounds */
            AddEyeObjects(Root.slab_root, &Root.eyeprims);
            }

         /* Figure out how many lines at a time we can render */
         if ((Rendering_Method == WIRE_FRAME && File_Generation_Flag == 0) ||
             Rendering_Method == RAW_TRIANGLES ||
             Rendering_Method == UV_TRIANGLES ||
             Rendering_Method == CSG_TRIANGLES) {
            ram_needed = 0;
            max_lines  = eye->view_yres;
            }
         else {
            ram_needed = 8L * (eye->view_xres + 1) * (eye->view_yres + 1);
            max_lines  = MaxBufferRAM / (8L * (eye->view_xres + 1));
            if (max_lines < 3) {
               warning("Too little RAM allocated for rendering: %ld",
                       MaxBufferRAM);
               max_lines = 3;
               }
            else if ( (unsigned)max_lines > eye->view_yres)
               max_lines = eye->view_yres + 1;
            }

         /* Create the image in a series of strips */
         for (y=0;y<eye->view_yres;y+=max_lines) {
            if (ram_needed == 0)
               initialize_scan_data(eye, pic, y, y + max_lines,
                                    Start_Line, End_Line, 0);
            else
               initialize_scan_data(eye, pic, y, y + max_lines,
                                    Start_Line, End_Line, 1);

            /* Now go render each strip (as long as it overlaps the part
               of the image that we want to render) */
            if ((y < (unsigned)Start_Line && y+max_lines >= (unsigned)Start_Line) ||
                (y >= (unsigned)Start_Line && y <= (unsigned)End_Line)) {
               if (Rendering_Method == RAY_TRACING)
                  Screen(eye, MAX(y, (unsigned)Start_Line), MIN(y + max_lines, (unsigned)End_Line));
               else
                  Polygon_Render(eye, &Root);
                }

            /* Perform and point/line/spline drawing commands */
            if (Check_Abort_Flag == 0 || Abort_Flag == 0)
               DoDrawing(eye, Draw_Commands);

            /* Blast the contents of this strip out to the file */
            if (File_Generation_Flag)
            /* CM 10/mar/2000 check if outputting targa */
	    {
             if (outformat == OUT_TARGA) {
            /* CM end */
               TGADump(eye, pic, y, y+max_lines-1);
            /* CM 10/mar 2000 additional part for other formats */
            } else {
               //PPMDump(eye, pic, y, y+max_lines-1); -- still not working
              }
	    }
            /* CM end */


            if (ram_needed != 0)
               Destroy_Scan_Buffers(eye);

            /* Check for an abort before continuing with the render */
            if (Check_Abort_Flag && Abort_Flag != 0)
               break;
            }

         /* If we were creating an output file then we close it now */
         if (File_Generation_Flag)
            TGAClose(pic);

         time(&ttime);
         ttime -= ptime;
         ptime -= *atime;
         }
      else if (tickflag > 0)
         status("\rSkipping frame: %d    ", current_frame);

      /* Only deallocate everything if this is the last frame in animation */
      if (current_frame < end_frame &&
          (!Check_Abort_Flag || Abort_Flag == 0))
         Deallocate_Symtab(0);
      else
         Deallocate_Symtab(1);

      Deallocate_Lights();

      /* Deallocate the perspective view transform */
      if (eye->WS != NULL) {
         polyray_free(eye->WS);
         eye->WS = NULL;
         }

      if (current_frame > skip_frame)
         PrintStatistics(ptime, ttime, *wait);

#if defined( MAC )
      if (_kbhit() && Check_Abort_Flag > 0)
#else
      if (_kbhit() && (Check_Abort_Flag > 0 || (Abort_Flag = getch()) == 27))
#endif
         break;
      time(atime);

#if defined( DEBUG_POINTERS ) && 0
   allocation_status();
#endif

      /* Increment the frame counter and see if we are done */
      current_frame++;
      if ((current_frame > end_frame) ||
          (Check_Abort_Flag != 0 && Abort_Flag != 0))
         break;
      }

   /* Deallocate any memory devoted to particle systems */
   FreeParticles();
#if defined( DEBUG_POINTERS ) && 0
   allocation_status();
#endif
}

#if defined( WINTARGET )
//---------------The MainWindowProcedure---------------

char *textonscreen=NULL;

LRESULT WINAPI MainWindowProcedure( HWND hWnd,
                                    UINT msg,
                                    WPARAM wParam,
                                    LPARAM lParam){
	   PAINTSTRUCT ps;
           HDC hDC; // handle to external device
           static int x_max,y_max,xlen,ylen,curx=0,cury=0,lineno=0;
           static short cxChar,cyChar; // variables that will save average character width and height
           TEXTMETRIC tm;
           char text[10];

	   switch( msg ) {

           case WM_SIZE:
                x_max=LOWORD(lParam);
                y_max=HIWORD(lParam);
                return(0);

	   case WM_PAINT:
                hDC=BeginPaint(hWnd,&ps); // use screen
                // for output see http://www.xdtech.com/winprog/Output.htm
                curx=cxChar;
                cury=cyChar*(lineno+1);
                if (textonscreen == NULL) {
                  TextOut( hDC, curx, cury, "BadMemAlloc!", 13 );
                }
                else {
                  TextOut( hDC, curx, cury, textonscreen, strlen(textonscreen));
                }
                EndPaint(hWnd,&ps);
		return (0);
           case WM_DESTROY:
                 free(textonscreen);
	         PostQuitMessage( 0 );
                 break;

           case WM_CREATE:
                textonscreen=(char *)malloc(90000);
                textonscreen="THE FIRST TEXT\n";
                hDC=GetDC(hWnd);
                GetTextMetrics(hDC,&tm);
                cxChar=tm.tmAveCharWidth;
                cyChar=tm.tmHeight+tm.tmExternalLeading;
                ReleaseDC(hWnd,hDC);
                return(0);
                // snipped from http://amalfi.dis.unina.it/~carlosan/F3+BD/chap2/default.htm
           default:
	         return( DefWindowProc( hWnd, msg, wParam, lParam ));

   	   }

}

//------------------------Globalz----------------------

void InitWindow(void)
{
        // Init the Window stuff...
	WNDCLASS windowClass;        // Declare a windowsClass structure;

	// Fill the structure with relevant info;
	windowClass.lpszClassName     = "_Windowz";
	windowClass.lpfnWndProc       = MainWindowProcedure;
	windowClass.style             = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
        windowClass.hInstance         = hInst;
	windowClass.hIcon             = LoadIcon( NULL, IDI_APPLICATION );
	windowClass.hCursor           = LoadCursor( NULL, IDC_ARROW );

	// I like Black-Backgrounds, but 4 regular standard background just use :
	// (HBRUSH)( COLOR_WINDOW+1 );
//        windowClass.hbrBackground     = (HBRUSH)(GetStockObject (BLACK_BRUSH));
        windowClass.hbrBackground     = GetStockObject(WHITE_BRUSH);

        windowClass.lpszMenuName      = (LPSTR) NULL;
	windowClass.cbClsExtra        = 0;
	windowClass.cbWndExtra        = 0;


	// And register at Bill's place;
        if (RegisterClass( &windowClass )==0) {
           MessageBox(0, "Could not register Window", "Polyray", MB_ICONHAND|MB_OK);
           exit(1);
	}

	// And create the window itself;
        hWnd = CreateWindow( "_Windowz",
                                     "Polyray 1.8c Win95 Buffy",
                             	     WS_OVERLAPPEDWINDOW,
                             	     0,
			     	     0,
                             	     CW_USEDEFAULT,
                             	     CW_USEDEFAULT,
			     	     NULL,
                             	     NULL,
                                     hInst,
                             	     NULL);


	// Let's Put thiz Sucka on screen :o)
        ShowWindow ( hWnd, CmdShow); // SW_SHOWMAXIMIZED
        UpdateWindow(hWnd); // causes background to be painted
}


//-----------------------WinMainFunction---------------
#pragma argsused
int PASCAL WinMain(  HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
		   {
//        HWND windowHandle;           // Declare a handle to the window;
	MSG message;    	     // Declare a message struct

        int argc=1; // for parsing cli params
        int i;
        char **argv; // used in parsing and call of main

        hInst = hInstance;
        CmdShow = nCmdShow;

        // command line handling from:
        // http://www.circlemud.org/pub/CircleMUD/contrib/windows/circle.w95-port.txt

        if (strcmp(lpCmdLine, ""))
        {
           /* Find out number of tokens in the string */
           for (i=0; i<strlen(lpCmdLine); i++)
           {
             if(lpCmdLine[i]==' ')
               argc++;
           }
           argc++; // the last token in the string won't have a trailing space

           // allocate memory
           argv = malloc(argc + 1);

           // fill the array
           argv[0]="polyray.exe"; // program filename
           argv[1]=strtok(lpCmdLine, " "); // get the first token

           for (i=2;i<argc;i++)
             argv[i]=strtok(NULL, " "); // pull out any others

           // finish the array off
           argv[argc + 1] = '\0';
        }
        else
        {
           // allocate the memory
           argv = malloc(2*sizeof(char*));

           // fill the array
           argv[0]="polyray.exe"; // program filename
           argv[1]='\0'; // terminate the struct
        }



	if (!hPrevInstance)
	{
          InitWindow();
        }

        // call main function with the parsed command line parameters
        main(argc,argv);

        // free memory allocated for cmdline parameters
        free(argv);
//        return 0;


	// The following loop is the standard messaging loop.........
	// It consumes the windowing messages.

	while( GetMessage( &message, 0, 0, 0 ) ) {

		TranslateMessage( &message );
		DispatchMessage( &message );
	}

        return message.wParam;

}

#endif


int main(int _argc, char *argv[])
{
   char *infilename = NULL;
   char outfilename[128];
   int resume = 0;
   int wait = 0;
   int Start_Line = -1, End_Line = -1;
   time_t atime;
  /*CM 11/4/2003 */
  extern FILE *yyin, *yyout;
  /*END: CM*/
   /* CM 5/11/2002 */
   message_log=stderr; //no longer should be initialised in io.c...
   /*END:CM*/
   int argc = _argc;
   //char *argv[2];
   //argv[0] = _argv[0];
   //argc = 2;
   //argv[1] = "test0.pi";
   //since VC++ isn't reading parameters when debugging
  /* CM 20/2/2006 */
  //printf("Setting up colors\n");
  setupcolors2(); //set up colors not able to be initialised any more in gcc...

  /*END:CM*/

  /* CM 7/3/2006 fix for msvcrt.dll bug in pre-XP SP2 as https://sourceforge.net/tracker/?func=detail&atid=102435&aid=944512&group_id=2435%C2%A0%3Cbr */
  //char* __old = setlocale(LC_ALL, NULL);
  //char* __sav = (char*) malloc(strlen(__old) + 1);
  //strcpy(__sav, __old);
  //setlocale(LC_ALL, "C");
  //  snprintf a number
  /*END:CM*/


   if (argc < 2) {
      show_arguments();
 #if defined( WINTARGET )
      return (1);
#else
      exit(1);
#endif
      }
   else {
      infilename = argv[1];
      }

  yyin=stdin;yyout=stdout; /*CM */
   /* Set the warning and status log files to stderr */
   SetMessageLog(NULL);
   /* Here is the actual set of calls that does something */
   time(&atime);
   Initialize_Symtab();
   Read_Initialization_Data();
   //printf("argc=%d\n", argc);
   read_defaults(argc, argv, outfilename, &Eye, &resume,
                 &wait, &Start_Line, &End_Line);
   render_scene(argc, argv, &Eye, infilename, outfilename, outfilebase,
                resume, &atime, &wait, Start_Line, End_Line);
//printf("5555\n");

   /* Close the warning and status log files */
   SetMessageLog(NULL);

#if defined( MAC )
   // freopen("profile.out", "w", stdout);
#elif !defined( _WINDOWS )
   if (kbhit()) getch();
#endif

  // setlocale(LC_ALL, __sav);//CM-see above
  // free (__sav);//CM-same
   exit( Abort_Flag ); /* Return the status */

   return 0; /* Just to keep the compiler happy */
}


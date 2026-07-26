/*
   Main driver for ray-tracer

  Polyray MIT Licensed Revival
  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Copyright (C) 1999-2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
//#include <time.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <utility> //for std::to_underlying
#include <cstdlib> //for setenv on MSYS2

#define SDL_MAIN_HANDLED
#include "display_backend.h"


#define inmain
#include "defs3.h"
#include "getopt.h"//cm
#include "consolewin.h"
#include "enqueue.h"

#include "vector.h"
#include "io_ply.h"
#include "memory.h"
#include "psupport.h"
#include "particle.h"
#include "light.h"
#include "sweep.h"
#include "scan.h"
#include "screen.h"
#include "display.h"
#include "pic.h"
#include "runtime_state.h"
#include "symtab.h"
#include "bound.h"
#include "image.h"
#include "intersec.h"

#include "sphere.h" //test
#include "symtab.h" //test


#include <locale.h> //CM: for mingw bugfix
//#include <curses.h> //CM: for getch() on mac. But then I get problems linking on mac
#include "unixcompat.h"
#include "util.h"

#include "bound.h"//for testing bound
#include "colour.h"

/* Platform/compiler specific include files */
#if defined( MAC )
#include <profile.h>
#endif

/*CM added 10/mar/2000 */
//OUT_FORMAT outformat = OUT_FORMAT::OUT_TARGA;
/*CM end */

static int skip_frame = -1;

/*CM added 28/feb/2002 */
#if defined( WINTARGET )
int main(int argc, char **argv);
#endif

/*CM end */


/*CM added 2/nov/2002 */
static void hitanykey()
{
#if defined (WINTARGET)

#else

#endif

  openpolyray::polyray_pause();//cm160519
}
/*CM end */

static void show_arguments(void)
{
 openpolyray::dumpversion();

smessage("Syntax: polyray filename (options)\n"\
"\n File options:\n"\
"     -o outputfile      [ default is 'out.tga'                  ]\n"\
"     -p pixelsize       [ bits/pixel [8, 16, 24, 32]            ]\n"\
"     -d                 [ Render as a depth map                 ]\n"\
"     -u                 [ Turn off RLE compression of output    ]\n"\
"     -x columns         [ trace 'colunms' pixels per row        ]\n"\
"     -y lines           [ trace 'lines' rows                    ]\n"\
"     -B                 [ flush the output file every line      ]\n"\
"     -b count           [ flush every 'count' pixels            ]\n"\
"     -R                 [ Resume an old trace                   ]\n"\
"     -z y0 y1           [ Render from line y0 to line y1        ]\n"\
"     -G                 [ output in RAW PPM format (not yet)    ]\n");
fflush(message_log);
hitanykey();
smessage("\n Rendering options:\n"\
"     -r method          [ Render: 0=Ray,1=Scan,2=Wire,3=Hidden  ]\n"\
"                        [         4=Gourad,5=Raw,6=uv           ]\n"\
"     -q flags           [ Turn on/off various shading options:  ]\n"\
"                        [ 1=shadow, 2=reflect, 4=transmit, 8=two]\n"\
"                        [ sides, 16=check uv, 32=flip normals   ]\n"\
"                        [ 63=all flags                          ]\n"\
"     -N                 [ Don't generate an image file          ]\n"\
"     -M kbytes          [ Max # of KBytes for image buffer      ]\n");
fflush(message_log);
hitanykey();

smessage("\n"\
"\nStatus options:\n"\
"     -t freqency        [ Status: 0=none,1=totals,2=line,3=pixel]\n"\
"\n Antialiasing options for ray-tracing:\n");
smessage("     -a mode            [ AA: 0=none,1=filter,2-4=adaptive      ]\n");
smessage("     -c                 [ Enable colinear triangle removal      ]\n");
smessage("     -T threshold       [ Threshold to start oversampling       ]\n");
smessage("     -S samples         [ Use 'samples' rays/pixel              ]\n");
smessage("\n Optimization options:\n");
smessage("     -O optimizer       [ 0 = none, 1 = slabs                   ]\n");
smessage("\n Display option:\n");
smessage("     -V mode            [ 0=none, 1-5=8bit, 6-10=15bit,         ]\n");
smessage("                        [ 11-15=16bit, 16-20=24bit, 21-22=4bit  ]\n");
smessage("     -P pallette        [ 0=grey, 1=332, 2=666, 3=EGA           ]\n");
smessage("     -e start           [ Start position in VGA pallette        ]\n");
smessage("     -W                 [ Wait for key before clearing display  ]\n");
smessage("     -D flag            [ 0=no video dither, 1 = use dither     ]\n");
smessage("\n Frame counter option:\n");
smessage("     -F start_frame     [ Skip frames until start_frame         ]\n");
smessage("\n Abort option:\n");
smessage("     -Q abort_option    [ 0 = no abort, 1 = check by pixel      ]\n");
smessage("                        [ 2 = by line/object                    ]\n");
smessage("     -s                 [ Allow System Calls                    ]\n");
smessage("\n");
fflush(message_log);
//BezierNormalTest();
//BuildAreaTableTestHarness();

}

/* get option letter from argument vector */
//static int _optind = 2, /* index into parent argv vector */
//           _optopt;     /* character checked for validity */
//static char *optarg1,
//            *optarg2;  /* argument(s) associated with option */

//constexpr int BADCH =  (int)'?';
//constexpr char *EMESG =  "";


static void Polygon_Render(Viewpoint *eye, BinTree *Root)
{
   ostackptr objs;
   Object *obj;
   int i;
#ifdef unix
   char tmp[100];
#endif

   /* Read and render the objects */   
   try {
      for (i=0,objs=Root->members.list;objs!=NULL;i++,objs=objs->next) {
         obj = objs->element;
         if (RuntimeState::settings.tickflag > 1)
            status((char*)"\r%d ", i);
#ifdef unix
         if (start_frame!=end_frame)
           sprintf(tmp,"F%d/%d, p%d/%ld", current_frame-start_frame,
                end_frame-start_frame, i, Root->members.count);
         else
           sprintf(tmp,"p%d/%ld",i,Root->members.count);
         //SpecialStatus(tmp);
#endif
         if ((runtimeState::Check_Abort_Flag == 2) && _kbhit()) {
             runtimeState::Abort_Flag = getchar();//getch()
            break;
            }
         render_prim(eye, Root, NULL, obj);
         }
      }
   catch (const std::exception& e) {
#if defined( MAC )
      runtimeState::Abort_Flag = 1;
#else
      if (_kbhit())
          runtimeState::Abort_Flag = getchar();
      else
          runtimeState::Abort_Flag = 1;
#endif
      }
}

static void StartupDisplay(Viewpoint *eye, char *infilename)
{
   if (Global::Display_Flag != 0)
      /* Initialize the display */
      display_init(eye, infilename);
}

static void Startup_Statistics(char *instr, const std::string outstr, Viewpoint *eye)
{
   if (RuntimeState::settings.tickflag > 0) {
      std::cout<<"Filename: "<<outstr<<"\n";
      status((char*)"Data: %s, Targa: %s\n", instr, outstr.c_str());
      status((char*)"Prims = %lu, Lights = %d, Memory: %lu\n",
          RuntimeState::scene.Root.members.count, RuntimeState::nLights, nMalloc - nFree);
      status((char*)"Res: %dx%d, Aspect: %0.4lg, Shade: %x\n",
              eye->view_xres, eye->view_yres,
              eye->view_aspect, runtimeState::scene.Global_Shade_Flag);
      if (runtimeState::animator.end_frame > runtimeState::animator.start_frame || 
          runtimeState::animator.total_frames != 0)
         status((char*)"Frame %d in [%d, %d]\n",
             runtimeState::animator.current_frame, 
             runtimeState::animator.start_frame, 
             runtimeState::animator.end_frame);
      //openpolyray::polyray_pause();
      }
}

/* Dump out the statistics for this frame. */
static void PrintStatistics(time_t ptime, time_t ttime, int wait)
{
   if (Global::Display_Flag && Global::Reset_Display_Flag &&
       ( (runtimeState::animator.current_frame == runtimeState::animator.end_frame) ||
        ((runtimeState::Check_Abort_Flag != 0) && runtimeState::Abort_Flag)) )
      display_close(wait);
   if (RuntimeState::settings.tickflag > 0) {
      status((char*)"preprocess time            %-8ld seconds\n", (long)ptime);
      status((char*)"tracing time               %-8ld seconds\n", (long)ttime);
      if (runtimeState::settings.Render_Method == rmode::RAY_TRACING) {
         status((char*)"number of eye rays cast:   %-8lu\n", runtimeState::stats.nRays);
         status((char*)"number of shadow rays:     %-8lu\n", runtimeState::stats.nShadows);
         if (runtimeState::stats.nReflected > 0)
            status((char*)"number of reflected rays:  %-8lu\n", runtimeState::stats.nReflected);
         if (runtimeState::stats.nRefracted > 0)
            status((char*)"number of refracted rays:  %-8lu\n", runtimeState::stats.nRefracted);
         if (runtimeState::stats.nTIR > 0)
            status((char*)"number of TIR rays:        %-8lu\n", runtimeState::stats.nTIR);
         if (runtimeState::stats.nJittered > 0)
            status((char*)"number of jittered rays:   %-8lu\n", runtimeState::stats.nJittered);
         if (runtimeState::stats.totalQueueResets > 0) {
            status((char*)"number of queue resets:    %-8lu\n", runtimeState::stats.totalQueueResets) ;
            status((char*)"avg number of queues/ray:  %-8g\n", (Flt)runtimeState::stats.nEnqueued /
                                                        (Flt)runtimeState::stats.totalQueueResets) ;
            status((char*)"max queue size:            %-8lu\n", runtimeState::stats.maxQueueSize) ;
            }
         status((char*)"number of shadow checks:   %-8lu\n", runtimeState::stats.totalShadows);
         status((char*)"number of cache hits:      %-8lu\n", runtimeState::stats.totalShadowCaches);
         status((char*)"number of bounds checked:  %-8lu\n", runtimeState::stats.nChecked);
         status((char*)"number of bounds queued:   %-8lu\n", runtimeState::stats.nEnqueued);
         status((char*)"number of objects checked: %-8lu\n", runtimeState::stats.totalQueues);
         }
      }
}

static void HandleErrImage(std::expected<void, ErrRow>& ret)
{
    switch (ret.error().err) {
    case ErrImage::ErrFailedToAllocateRowOfZBuffer:
        serror("Failed to allocate row %d of Z-Buffer (try -M 256 command line option)\n", ret.error().row);
        break;
    case ErrImage::ErrFailedToAllocateZBufferMoreMemNeeded:
        serror("Failed to allocate the Z-Buffer (more memory is needed)\n");
        break;
    case ErrImage::ErrFailedToAllocateSBuffer:
        serror("Failed to allocate the S-Buffer (try -M 256 command line option)\n");
        break;
    case ErrImage::ErrUnresolvedBackgroundExpression:
        serror("Unresolved background expression\n");
        break;
    default:
        serror("Unknown error");
    }
}

static void initialize_scan_data(Viewpoint *eye, Pic *pic,
                     int y_start, int y_end,
                     int Start_Line, int End_Line,
                     int alloc_flag)
{
   // Set up the view transformation 
   eye->WS = Normalize_View(eye);//if eye->WS points anywhere it will be automatically released
#ifdef DEBUG_FN_CALLS
std::cout << "Y: " << y_start << " - " << y_end << ", Strip: " << Start_Line << ", " << End_Line << "\n";
#endif
   /* This is necessary so that drawing commands will work in
      non file generation modes.  Otherwise we could stuff this
      call into the Allocation routine below. */
   Initialize_Clipping(eye, MAX(y_start,Start_Line), PLY_MIN(y_end,End_Line));   

   if (alloc_flag) {
       auto ret=Allocate_Scan_Buffers(eye, pic, y_start, y_end);
       if (!ret.has_value()) {
           HandleErrImage(ret);
           exit(1);//won't get there thanks to serror.
       }
   }
}

/* Between each generated frame, reclaim any memory that was allocated. */
static void reset_data(void)
{
   /* Reset the global render counters */
    RuntimeState::Reset_Render_Statistics();

   /* Allocate memory for the various data structures */
   Initialize_Symtab();

   Initialize_Lights();
}

static void read_defaults(const int argc, const char **argv, std::string& outfilename,
              Viewpoint *eye, int *resume,
              int *wait, int *Start_Line, int *End_Line)
{
   std::string opts = "a:b:D:e:M:o:O:p:P:q:Q:r:S:t:T:V:x:y:sz+F:BNGRduvW";
   int c;
   int line_buffer = 0;

   #ifdef DEBUG_FN_CALLS
   std::cout<<"main::read_defaults\n";
   std::cout<<"outfilename="<<outfilename<<"\n";
   #endif
   /* Set defaults for the output file */
   if (!RuntimeState::filebaseflag)
       RuntimeState::outfilebase="out";
   if (outfilename=="") outfilename="out.tga";
   #ifdef DEBUG_FN_CALLS
   std::cout<<"main::read_defaults 2nd part\n";
   std::cout<<"outfilename="<<outfilename<<"\n";
   #endif

   /* Loop through the command line options */
   GetOpt _getopt;
   while ((c = _getopt.getopt(argc, argv, opts)) != EOF) {
      #ifdef DEBUG
      std::cout<<"received opt "<<char(c)<<"\n";
      #endif
      switch (c) {
      case 'a':
          RuntimeState::settings.antialias = stoi(_getopt.GetOptarg1());
         if (RuntimeState::settings.antialias > 4)
            RuntimeState::settings.antialias = 4;
         else if (RuntimeState::settings.antialias < 0)
            RuntimeState::settings.antialias = 0;
         break;
      case 'b':
         if (line_buffer)
            swarning("Line buffering already specified\n");
         else {
            RuntimeState::buffer_update = 1;
            RuntimeState::buffer_size = stoi(_getopt.GetOptarg1());
            }
         break;
      case 'B':
         if (RuntimeState::buffer_update)
            swarning("Pixel buffering already specified\n");
         else {
            line_buffer   = 1;
            RuntimeState::buffer_update = 1;
            }
         break;
      case 'd':
          RuntimeState::settings.DepthRender = 1;
         break;
      case 'D':
          Global::Dither_Flag = stoi(_getopt.GetOptarg1());
         if (Global::Dither_Flag < 0 || Global::Dither_Flag > 1)
            serror("Dither flag must be either 0 (off) or 1 (on)\n");
         break;
      case 'e':
          Global::Pallette_Start = stoi(_getopt.GetOptarg1());
         if (Global::Pallette_Start < 0 || Global::Pallette_Start > 240)
            serror("First entry of pallette must be between 0 and 240\n");
         break;
      case 'F':
         skip_frame = stoi(_getopt.GetOptarg1());
         break;
      case 'G':
         RuntimeState::outformat = OUT_FORMAT::OUT_RAWPPM;
         outfilename="out.ppm";
         break;

      case 'M':
          RuntimeState::settings.MaxBufferRAM = 1024L * stoi(_getopt.GetOptarg1());
         break;
      case 'N':
          RuntimeState::File_Generation_Flag = 0;
         break;
      case 'o':
         outfilename=_getopt.GetOptarg1();
         //std::cout<<"outfilename="<<outfilename<<"\n";
         //openpolyray::polyray_pause();
         break;
      case 'O':
         RuntimeState::settings.Optimizer = stoi(_getopt.GetOptarg1());
         if (RuntimeState::settings.Optimizer < 0 || RuntimeState::settings.Optimizer > 1)
            serror("Optimizer must be one of: 0 [none], 1 [slabs]\n");
         break ;
      case 'p':
          runtimeState::settings.pixelsize = stoi(_getopt.GetOptarg1());
         if (runtimeState::settings.pixelsize != 8 && runtimeState::settings.pixelsize != 16 &&
             runtimeState::settings.pixelsize != 24 && runtimeState::settings.pixelsize != 32)
            serror("Pixel size must be 8, 16, 24, or 32 bits\n");
         break;
      case 'P':
          Global::Pallette_Flag = stoi(_getopt.GetOptarg1());
         if (Global::Pallette_Flag < 0 || Global::Pallette_Flag > 3)
            serror("Pallette must be one of: 0 [grey], 1 [8-8-4], 2 [6-6-6], 3 [4 bit]\n");
         break;
      case 'q':
          runtimeState::scene.Global_Shade_Flag = stoi(_getopt.GetOptarg1());
         if (runtimeState::scene.Global_Shade_Flag > ALL_SHADE_FLAGS)
            serror("Too many bits set in shade flag (must be less than %d\n",
                  ALL_SHADE_FLAGS);
         break ;
      case 'Q':
          runtimeState::Check_Abort_Flag = stoi(_getopt.GetOptarg1());
         if (runtimeState::Check_Abort_Flag < 0 || runtimeState::Check_Abort_Flag > 2) {
            swarning("Abort flag must be between 0 and 2");
            runtimeState::Check_Abort_Flag = 1;
            }
         break ;
      case 'r':
         runtimeState::settings.Render_Method = static_cast<rmode>(std::stoi(_getopt.GetOptarg1()) + 
           static_cast<int>(rmode::RAY_TRACING));
         if (runtimeState::settings.Render_Method < rmode::RAY_TRACING || static_cast<int>(runtimeState::settings.Render_Method) > static_cast<int>(rmode::LAST_RENDER_MODE))
            serror("Rendering must be one of: 0 [raytrace], 1 [hidden line], 4 [gourad], 5 [raw tri], 6 [uv tri]\n");
         if (runtimeState::settings.Render_Method == (rmode::RAW_TRIANGLES) ||
             runtimeState::settings.Render_Method == (rmode::UV_TRIANGLES) ||
             runtimeState::settings.Render_Method == (rmode::CSG_TRIANGLES))
         //stdc++23: if (runtimeState::settings.Render_Method == std::to_underlying(rmode::RAW_TRIANGLES) ||
         //    runtimeState::settings.Render_Method == std::to_underlying(rmode::UV_TRIANGLES) ||
         //    runtimeState::settings.Render_Method == std::to_underlying(rmode::CSG_TRIANGLES))
             RuntimeState::File_Generation_Flag = 0;
         break;
      case 'R':
         *resume = 1;
         break;
      case 'S':
          runtimeState::settings.maxsamples = stoi(_getopt.GetOptarg1());
         if (runtimeState::settings.maxsamples <= 0) {
            swarning("Samples must be > 0, reset to 4\n");
            runtimeState::settings.maxsamples = 4;
            }
         break;
      case 'T':
          runtimeState::settings.antialias_threshold = std::stof(_getopt.GetOptarg1());
         if (runtimeState::settings.antialias_threshold < 0.0) {
            swarning("Antialias threshold must be >= 0.0, reset to 0.02\n");
            runtimeState::settings.antialias_threshold = 0.02;
            }
         runtimeState::settings.antialias_threshold *= runtimeState::settings.antialias_threshold;
         break ;
      case 't':
          RuntimeState::settings.tickflag = stoi(_getopt.GetOptarg1());
         if (RuntimeState::settings.tickflag == 0) RuntimeState::status_flag = 0;
         break;
      case 'u':
          runtimeState::settings.pixel_encoding = 0;
         break;
      case 'V':
          Global::Display_Flag = stoi(_getopt.GetOptarg1());
         if (Global::Display_Flag < 0) {
             Global::Display_Flag =  - Global::Display_Flag;
            Global::Reset_Display_Flag = 0;
            }
         else if (Global::Display_Flag > FIRST_4BIT_MODE+1)
            serror("Unsupported display mode");
         else
             Global::Reset_Display_Flag = 1;
         break;
      case 'W':
         *wait = 1;
         break ;
      case 'x':
         eye->view_xres = stoi(_getopt.GetOptarg1());
         //std::cout<<"we have read param "<<eye->view_xres<<"\n";
         break;
      case 'y':
         eye->view_yres = stoi(_getopt.GetOptarg1());
         break;
      case 'z':
         *Start_Line = stoi(_getopt.GetOptarg1());
         *End_Line = stoi(_getopt.GetOptarg2());
         break;
      case 's':
          runtimeState::Allow_SystemCalls = 1;
          break;
      }
   }

   if (line_buffer)
       runtimeState::buffer_size = eye->view_xres;
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
      swarning("Start line of image %d is after end line %d\n",
            eye->view_ystart, eye->view_yend);
      *Start_Line = -1;
      *End_Line   = -1;
      }
   else if (eye->view_ystart >= eye->view_yres) {
      swarning("Start line of image %d is after end of image %d\n",
            eye->view_ystart, eye->view_yres);
      *Start_Line = -1;
      *End_Line   = -1;
      }
   else if (eye->view_yend > eye->view_yres) {
      swarning("End line of image %d is too high (reset to %d)\n",
              eye->view_yend, eye->view_yres);
      eye->view_yend = eye->view_yres;
      *End_Line = eye->view_yres;
      }
}

Surface* Flat_Eval(Viewpoint*, Object*, Texture* tex,
    Vec, Vec, Vec, Vec, float, float, int);//needed for gcc?
//testing routine for textures - CM
Surface* Flat_Eval(Viewpoint*, Object*, Texture* tex,
    Vec, Vec, Vec, Vec, float, float, int)
{
    return (Surface*)(tex->data);  // just return the static surface
}

static void render_scene(int argc, char **argv, Viewpoint *eye, std::string infilename,
             std::string& outfilename, const std::string outfilebase, int resume,
             time_t *atime, int *wait, int Start_Line, int End_Line)
{
   #ifdef DEBUG_FN_CALLS
   smessage("main::render_scene\n");
   #endif
   Pic *pic;
   time_t ptime, ttime;   
   long ram_needed, max_lines;
   #ifdef DEBUG_FN_CALLS
   //std::cout<<"main::render_scene\n";
   std::cout<<"outfilename="<<outfilename<<"\n";
   //openpolyray::polyray_pause();
   #endif

   /* At last we get to the main driver loop. */
   for (;;) {
       Start_Line = End_Line = -1;

       /* If we are on the second (or later) frame then reset various
          pieces of data */
       if (runtimeState::Parsed_Flag)
           reset_data();

       /* Parse the data file */
#ifdef DEBUG_FN_CALLS
       std::cout << "main::parsing\n";
#endif
       ReadSceneFile(infilename);
#ifdef DEBUG_FN_CALLS
       std::cout << "main::parsed\n";
#endif

       /* Use command line params to override values that appeared
          in either the ini file or the data file */
       read_defaults(argc, (const char**)argv, outfilename, eye, &resume,
           wait, &Start_Line, &End_Line);

       /* If the amount of RAM to use hasn't been specified, then
          assume that the entire image buffer should be allocated. */
       if (runtimeState::settings.MaxBufferRAM < 0)
           runtimeState::settings.MaxBufferRAM = 8L * (eye->view_xres + 1) * (eye->view_yres + 1);

       /* Determine the line boundaries for this image */
       set_line_boundaries(eye, &Start_Line, &End_Line);

       /* If the output of this render is a depth file then make sure
          that the file is neither RLE compressed nor antialiased. */
       if (RuntimeState::settings.DepthRender == 1) { runtimeState::settings.pixel_encoding = 0; RuntimeState::settings.antialias = 0; }

       /* If this is the first pass, then we need to set up the animation
          counters. */
       if (!RuntimeState::Parsed_Flag) {
           RuntimeState::Parsed_Flag = 1;
           if (RuntimeState::animator.end_frame < 0) RuntimeState::animator.end_frame = RuntimeState::animator.total_frames;
           RuntimeState::animator.current_frame = RuntimeState::animator.start_frame;
           if (RuntimeState::settings.tickflag > 0)
           {
               openpolyray::dumpversion();
			   //std::cout << "current_frame=" << current_frame << "\n";
               //std::cout << "ENDING HERE FRAME main.cc\n";
               //exit(200);
           }
           
       }

       /* Allocate and initialize memory, set the global shading flags. */
       if (runtimeState::settings.Render_Method == (rmode::SCAN_CONVERSION) ||
           runtimeState::settings.Render_Method == (rmode::HIDDEN_LINE) ||
           RuntimeState::settings.Render_Method == (rmode::GOURAD_SHADE) ||
           (RuntimeState::settings.Render_Method == (rmode::WIRE_FRAME) && RuntimeState::File_Generation_Flag != 0)) {
           /* By default there is no shadow/reflect/transmit
              performed for polygonal scenes */
           if (RuntimeState::scene.Global_Shade_Flag & UNSET_SFLAG)
               RuntimeState::scene.Global_Shade_Flag = TWO_SIDED_SURFS | NORMAL_CORRECT;
       }
       else if (RuntimeState::scene.Global_Shade_Flag & UNSET_SFLAG)
           /* By default check almost everything when raytracing */
           RuntimeState::scene.Global_Shade_Flag = SHADOW_CHECK | REFLECT_CHECK | TRANSMIT_CHECK |
           UV_CHECK | CAST_SHADOW | NORMAL_CORRECT;

       if (runtimeState::scene.Global_Shade_Flag & SHADOW_CHECK)
           Initialize_Light_Caches();

       if (RuntimeState::animator.end_frame > RuntimeState::animator.start_frame || RuntimeState::animator.total_frames != 0)
           outfilename = build_outfile_name(RuntimeState::outfilebase);

#ifdef SIMPLESPHERETEST
       // ---- TEST OBJECT (HARD-CODED SPHERE) ----
       Vec center = { 0.0, 0.0, 0.0 };
       Flt radius = 1.0;

       auto composite = FactoryCompositeObject(1);

       // Allocate base object
       Object* obj = (Object*)default_malloc(sizeof(Object));
       memset(obj, 0, sizeof(Object));
       //composite->c_object = { *obj };//check
	  

      // Create the sphere
      //bbox_info* bbox = new bbox_info;
      //Object* psphere = MakeSphere(obj, center, radius);
      //std::unique_ptr<Object> sphere(psphere);
      
      
      //std::unique_ptr<Object> sphere(MakeSphere(obj, center, radius));
      // 2) Sphere: allocate with new (matches delete by unique_ptr)
auto sphere = std::make_unique<Object>();
*sphere = Object{}; // zero-initialize POD fields safely

MakeSphere(sphere.get(), center, radius); // fill fields, no allocation mismatch


      sphere->o_csg_tree = NULL;
      sphere->o_parent = NULL;

      sphere->o_trans = NULL;
      sphere->o_displace = NULL;
      sphere->o_vertices = NULL;//or something!
      sphere->o_uv_steps[0] = 16;
      sphere->o_uv_steps[1] = 16;
      sphere->o_uv_bounds[0] = 100;
      sphere->o_uv_bounds[1] = 100;
      sphere->o_uv_bounds[2] = 100;
      sphere->o_uv_bounds[3] = 100;
      //MakeVector(-100, -100, -100, sphere->o_bnd.lower_left);
      //MakeVector(2000, 2000, 2000, sphere->o_bnd.lengths);
      MakeVector(center[0] - radius, center[1] - radius, center[2] - radius, sphere->o_bnd.lower_left);
      MakeVector(radius * 2.0, radius * 2.0, radius * 2.0, sphere->o_bnd.lengths);

       // Create the surface with desired material properties
      Surface* surf = (Surface*)default_malloc(sizeof(Surface));
      memset(surf, 0, sizeof(Surface));

 
      
      // Ambient term
      surf->Ka_color[0] = 1.0f;  // R
      surf->Ka_color[1] = 1.0f;  // G
      surf->Ka_color[2] = 1.0f;  // B
      surf->Ka_scale = 0.1f;

      // Diffuse term
      surf->Kd_color[0] = 1.0f;
      surf->Kd_color[1] = 1.0f;
      surf->Kd_color[2] = 1.0f;
      surf->Kd_scale = 1.0f;

      // Specular term
      surf->Ks_color[0] = 0.0f;
      surf->Ks_color[1] = 0.0f;
      surf->Ks_color[2] = 0.0f;
      surf->Ks_scale = 0.0f;

      // Reflection, Transmission
      surf->Kr_scale = 0.0f;
      surf->Kt_scale = 0.0f;

      // Microfacet distribution
      surf->D = NULL;
      surf->D_coeff = 0.0f;

      // Index of refraction
      surf->ior = 1.0f;

           // Create the Texture and wire it up
      Texture* tex = (Texture*)default_malloc(sizeof(Texture));
      memset(tex, 0, sizeof(Texture));
      // Procedural evaluator: always return this surface
      tex->eval = Flat_Eval;  // We'll define this below
      tex->data = (void*)surf;
      tex->type = 0;
      tex->copy_flag = 0;
      tex->data = (void*)surf;   // your surf setup above
      
      // Assign a basic texture (to ensure it's visible)      
      sphere->o_texture = tex;
      
      // put sphere inside composite
      composite->c_object.push_back(sphere.release());
      composite->c_size = static_cast<unsigned short>(composite->c_object.size());

// composite bounds
      //get_bounds_CPP(*sphere, *bbox);
      
      //composite->o_bnd = sphere->o_bnd;  // copy bounds
      composite->o_bnd = composite->c_object[0]->o_bnd;//why?
    
      // Debug-only: insert the test object into the regular scene object list.
      Add_To_BinTree(Global::Root, composite);
#endif
   
      
      InstantiateParticles(eye);

      if (RuntimeState::animator.current_frame >= skip_frame) {
         /* This section performs the rendering of a single
            frame.  An inner loop is required to allow for the
            rendering of a strip of the image at a time. */
         StartupDisplay(eye, (char*)infilename.c_str());
         AddLightObjects(&RuntimeState::scene.Root);

         Startup_Statistics((char*)infilename.c_str(), outfilename, eye);

         time(&ptime);

         /* Start by opening a file to hold the image.  If it already
            exists, then the image characteristics stored in the file
            will be used for the final image. */
         if (RuntimeState::File_Generation_Flag)
            pic = TGAOpen((char*)outfilename.c_str(), eye, resume, RuntimeState::settings.pixel_encoding,
                          RuntimeState::settings.pixelsize);
         else
            pic = NULL;

         if ((RuntimeState::settings.Optimizer > 0) &&
             ((RuntimeState::settings.Render_Method == (rmode::RAY_TRACING)) ||
              ((RuntimeState::settings.Render_Method == (rmode::GOURAD_SHADE) ||
                RuntimeState::settings.Render_Method == (rmode::SCAN_CONVERSION)) &&
               (RuntimeState::scene.Global_Shade_Flag &
                  (SHADOW_CHECK | REFLECT_CHECK | TRANSMIT_CHECK))))) {
            /* We are either ray tracing or we have shading options
               in a scan conversion mode that may lead to a recursive
               call to the ray tracer.  In either case if slab
               optimization is turned on we have to create the slabs. */
               
            BuildBoundingSlabs(RuntimeState::scene.Root);//&Root in old code
#ifdef DEBUG_FN_CALLS
            std::cout << "main::AfterBuildBoundingSlabs - Root.slab_root->o_type-" << std::to_underlying(RuntimeState::scene.Root.slab_root->o_type) << "\n";
            std::cout << "Root.slab_root->o_bnd.lower_left=" << RuntimeState::scene.Root.slab_root->o_bnd.lower_left[0] << ","
                << RuntimeState::scene.Root.slab_root->o_bnd.lower_left[1] << "," << RuntimeState::scene.Root.slab_root->o_bnd.lower_left[2] << "\n";
#endif
            

            /* Create a list of all objects (prims and composites) that
               contain the eye within their bounds */
               //std::vector<Object> oureyeprims;
            //AddEyeObjects(Root.slab_root, oureyeprims);
            AddEyeObjects(RuntimeState::scene.Root.slab_root, &RuntimeState::scene.Root.eyeprims);
#ifdef DEBUG_FN_CALLS
            std::cout << "main::After AddEyeObjects - Root.slab_root->o_type-" << std::to_underlying(RuntimeState::scene.Root.slab_root->o_type) << "\n";
            std::cout << "Root.slab_root->o_bnd.lower_left=" << RuntimeState::scene.Root.slab_root->o_bnd.lower_left[0] << ","
                << RuntimeState::scene.Root.slab_root->o_bnd.lower_left[1] << "," << RuntimeState::scene.Root.slab_root->o_bnd.lower_left[2] << "\n";
#endif
            #ifdef DEBUG
            //smessage("eyeobjects size=%d\n",oureyeprims.size());
           
            openpolyray::polyray_pause();
            #endif
            //auto ret = eyeprimconvert(oureyeprims, &Root.eyeprims);
            }

         /* Figure out how many lines at a time we can render */
         if ((runtimeState::settings.Render_Method == (rmode::WIRE_FRAME) && RuntimeState::File_Generation_Flag == 0) ||
             runtimeState::settings.Render_Method == (rmode::RAW_TRIANGLES) ||
             runtimeState::settings.Render_Method == (rmode::UV_TRIANGLES) ||
             runtimeState::settings.Render_Method == (rmode::CSG_TRIANGLES)) {
            ram_needed = 0;
            max_lines  = eye->view_yres;
            }
         else {
            ram_needed = 8L * (eye->view_xres + 1) * (eye->view_yres + 1);
            max_lines  = runtimeState::settings.MaxBufferRAM / (8L * (eye->view_xres + 1));
            if (max_lines < 3) {
               swarning("Too little RAM allocated for rendering: %ld",
                       runtimeState::settings.MaxBufferRAM);
               max_lines = 3;
               }
            else if ( (unsigned)max_lines > eye->view_yres)
               max_lines = eye->view_yres + 1;
            }

         /* Create the image in a series of strips */

         for (int y=0;y<eye->view_yres;y+=max_lines) {

            if (ram_needed == 0)
               initialize_scan_data(eye, pic, y, y + max_lines,
                                    Start_Line, End_Line, 0);
            else
               initialize_scan_data(eye, pic, y, y + max_lines,
                                    Start_Line, End_Line, 1);
            //std::cout << "inited scan data\n" << std::flush;

            /* Now go render each strip (as long as it overlaps the part
               of the image that we want to render) */
            if ((y < Start_Line && y+max_lines >= Start_Line) ||
                (y >= Start_Line && y <= End_Line)) {

                int tester = PLY_MIN(y + max_lines, End_Line);

               if (runtimeState::settings.Render_Method == (rmode::RAY_TRACING))
                  Screen(eye, MAX(y, Start_Line), PLY_MIN(y + max_lines, End_Line));
               else
                  Polygon_Render(eye, &RuntimeState::scene.Root);
                }
            //std::cout << "rendered!\n" << std::flush;
            /* Perform and point/line/spline drawing commands */
            if (RuntimeState::Check_Abort_Flag == 0 || RuntimeState::Abort_Flag == 0)
               DoDrawing(eye, Draw_Commands);

            /* Blast the contents of this strip out to the file */
            if (RuntimeState::File_Generation_Flag)
            /* CM 10/mar/2000 check if outputting targa */
	    {
             if (RuntimeState::outformat == OUT_FORMAT::OUT_TARGA) {
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
            if (RuntimeState::Check_Abort_Flag && RuntimeState::Abort_Flag != 0)
               break;
            }

         /* If we were creating an output file then we close it now */
         if (RuntimeState::File_Generation_Flag)
            TGAClose(pic);

         time(&ttime);
         ttime -= ptime;
         ptime -= *atime;
         }
      else if (RuntimeState::settings.tickflag > 0)
         status((char*)"\rSkipping frame: %d    ", RuntimeState::animator.current_frame);

      /* Only deallocate everything if this is the last frame in animation */
      if ((RuntimeState::animator.current_frame < RuntimeState::animator.end_frame) &&
          (!RuntimeState::Check_Abort_Flag || RuntimeState::Abort_Flag == 0))
         Deallocate_Symtab(0);
      else
         Deallocate_Symtab(1);

      Deallocate_Lights();

      // Deallocate the perspective view transform 
      if (eye->WS != nullptr) {
         eye->WS = nullptr;
         }

      if (RuntimeState::animator.current_frame > skip_frame)
         PrintStatistics(ptime, ttime, *wait);

#if defined( MAC )
      if (_kbhit() && RuntimeState::Check_Abort_Flag > 0)
#else
      if (_kbhit() && (RuntimeState::Check_Abort_Flag > 0 || (RuntimeState::Abort_Flag = getchar()) == 27))
#endif
         break;
      time(atime);

#if defined( DEBUG_POINTERS ) && 0
   allocation_status();
#endif

      /* Increment the frame counter and see if we are done */
      RuntimeState::animator.current_frame++;
      #ifdef DEBUG_FN_CALLS
      std::cout<<"current_frame incremented to "<< RuntimeState::animator.current_frame<<"\n";
      exit(2);
      #endif
      if ((RuntimeState::animator.current_frame > RuntimeState::animator.end_frame) ||
          (RuntimeState::Check_Abort_Flag != 0 && RuntimeState::Abort_Flag != 0))
         break;
      }

   /* Deallocate any memory devoted to particle systems */
   FreeParticles();
#if defined( DEBUG_POINTERS ) && 0
   allocation_status();
#endif
}




extern FILE * yyin, * yyout;

int main(int _argc, char *argv[])
{
   char *infilename = NULL;
   std::string outfilename;
   int resume = 0;
   int wait = 0;
   int Start_Line = -1, End_Line = -1;
   time_t atime;
  /*CM 11/4/2003 */
  
  /*END: CM*/
   /* CM 5/11/2002 */
   message_log=stderr; //no longer should be initialised in io.c...
#ifdef USE_SDL
   SDL_SetMainReady();
   if (!SDL_Init(SDL_INIT_VIDEO)) {
       fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
       exit(1);
   }
   const bool *keys = SDL_GetKeyboardState(NULL);
#endif
   /*END:CM*/
   int argc = _argc;

  /*END:CM*/

  
   //argc = 2; //vs debugging only!
  
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


  // infilename = (char*)"test0.pi";//vs debugging only!

  yyin=stdin;yyout=stdout; /*CM */
   /* Set the warning and status log files to stderr */
   SetMessageLog("");//(NULL);
   
   time(&atime);
   std::error_code ec;

   RuntimeState::POLYRAY_PATH=getEnvVar(RuntimeState::POLYRAY_PATH_STRING);
   if (RuntimeState::POLYRAY_PATH.empty())
   {
      try {
        std::filesystem::path wd=std::filesystem::current_path();
        RuntimeState::POLYRAY_PATH = wd.generic_string();
      
        
      }
      catch (const std::exception& ex)
      {
          RuntimeState::POLYRAY_PATH =  ".";
      }
   }

   RuntimeState::POLYRAY_DAT = getEnvVar(RuntimeState::POLYRAY_DAT_STRING);
   if (RuntimeState::POLYRAY_DAT.empty())
   {
       try {
           std::filesystem::path wd = std::filesystem::current_path();
           RuntimeState::POLYRAY_DAT = wd.generic_string();


       }
       catch (const std::exception& ex)
       {
           RuntimeState::POLYRAY_DAT = ".";
       }
   }

   Initialize_Symtab();
   LoadPolyrayIni();

   read_defaults(argc, (const char**)argv, outfilename, &RuntimeState::scene.Eye, &resume,
                 &wait, &Start_Line, &End_Line);
   render_scene(argc, argv, &runtimeState::scene.Eye, infilename, outfilename, RuntimeState::outfilebase,
                resume, &atime, &wait, Start_Line, End_Line);
   #ifdef DEBUG
   smessage("render_scene finished\n");
   #endif

   /* Close the warning and status log files */
   SetMessageLog("");//(NULL);

#if defined( MAC )
   // freopen("profile.out", "w", stdout);
#elif !defined( _WINDOWS )
   if (_kbhit()) getchar();
#endif

  // setlocale(LC_ALL, __sav);//CM-see above
  // free (__sav);//CM-same
   exit(RuntimeState::Abort_Flag ); /* Return the status */

   return 0; /* Just to keep the compiler happy */
}

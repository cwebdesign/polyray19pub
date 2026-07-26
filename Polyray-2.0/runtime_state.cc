/**
 * @file runtime_state.cc
 * @brief Runtime state for Symbol table, object lifecycle, and scene-global support routines.
 *
 * Support routines for reusable objects, strings, vectors, and scene setup.
 * 
  Polyray  MIT Licensed Revival
  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Copyright (C) 1999-2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the 
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the 
Software.

THE SOFTWARE IS PROVIDED "AS IS", (C), WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <cstdarg>
#include <new>
#include <string>
#include <string_view>
#ifdef DEBUG_POINTERS
#include <unordered_set>
#endif
#include <gsl/gsl>


#include "runtime_state.h"
#include "mfacet.h"//cm

namespace RuntimeState {

    RenderStatistics stats;
    RenderSettings settings;
    Animation animator;
    SceneState scene;


    /* Run-time globals */
    int Parsed_Flag = 0;          // First time reading input file
    int Shadow_Test = 0;          // Are we just looking for shadows?
    int Particle_Test = 0;        // Are we building particles?
    int File_Generation_Flag = 1; // Normally generate a Targa image
    int Allow_SystemCalls = 0; //allow systme calls by default is OFF
    int status_flag = 1;          // Set to 0 to eliminate status output
    int warnings_flag = 1;        // Set to 0 to eliminate warning output
    int errors_flag = 1;          // Set to 0 to eliminate error output




        Viewpoint       Eye = { 256, 256, 0, 0, 256, 256, 0, 255,
                                {0, 0, -1}, {0, 0, 0}, {0, 1, 0},
                                45.0, SMALL, PLY_HUGE, 0.0, 1.0, -1.0,
                                nullptr, nullptr, nullptr, nullptr, {} };



        /* Global light variables */
        Light* Lights = nullptr;
        int   nLights = 0;
        Light** light_array;
        /* Global image variables */
        int             buffer_update = 0;
        unsigned long   buffer_size = 0;

        YY_BUFFER_STATE oldbuf;
        YY_BUFFER_STATE newbuf;


    
    long MaxBufferRAM = 2048L * 2048L; /* Maximum RAM (in Kbytes) used by S&Z buffers */

    
    Flt             Global_Haze = 0.0;
    Flt             Global_Haze_Start = 0.0;
    Vec             Global_Haze_Color = { 0, 0, 0 };
    NODE_PTR        Background = nullptr;
    Vec             BackgroundColor;
    Vec             White = { 1.0, 1.0, 1.0 };


    /* Variables for trimming CSG raw triangles */
    Flt csg_leg_tolerance = 0.05;
    Flt csg_subdivision_depth = 1;

    std::string POLYRAY_PATH_STRING = "POLYRAY_PATH";//the env var name
    std::string POLYRAY_DAT_STRING  = "POLYRAY_DAT_PATH";//env var name for DAT files and ini file new location
    std::string POLYRAY_PATH = "";
    std::string POLYRAY_DAT = "";

    std::string     outfilebase;
    int             filebaseflag = 0;

    //TODO:these three variables should be replaced
    int             Check_Abort_Flag = 1; /* Look for an abort */    
    int             Abort_Flag = 0;

    /* Global variables to keep track of where we are currently tracing */
    int current_row;
    int current_col;
    int recursion_depth;

    OUT_FORMAT outformat = OUT_FORMAT::OUT_TARGA;//added by CM in 2000 in main.cc


    /* Bounding cluster size */
    int clustersize = 4;

    /* A matte white surface is used when none is specified */

//    float D_Phong(Vec, Vec, Vec, Flt); include mfacet instead

    Surface DefaultSurface =
    { {1.0f, 1.0f, 1.0f}, 0.2f, /* Ambient */
      1.0f,                     /* Brilliance */
      {1.0f, 1.0f, 1.0f}, 0.8f, /* Diffuse */
      {1.0f, 1.0f, 1.0f}, 0.0f, /* Specular */
      {1.0f, 1.0f, 1.0f}, 0.0f, /* Reflection */
      {1.0f, 1.0f, 1.0f}, 0.0f, /* Transmission */
      D_Phong,
      1.0f,
      1.0f };

    Flt rayeps = 1e-3;

    
    //helper functions
    /**
     * @brief Reset per-ray traversal state counters.
     * @return No return value.
     */
    void Reset_Runtime_State(void)
    {
        current_row = 0;
        current_col = 0;
        recursion_depth = 0;
    }

    /**
     * @brief Reset animation and per-frame state.
     * @return No return value.
     */
    void Reset_Frame_State(void)
    {
        animator.reset();        
    }

    /**
     * @brief Reset accumulated render statistics.
     * @return No return value.
     */
    void Reset_Render_Statistics(void)
    {
        stats.reset();
    }

}//namespace RuntimeState

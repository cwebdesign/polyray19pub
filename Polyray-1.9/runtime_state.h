#pragma once
/*
Polyray MIT Licensed Revival
Copyright(C) 1993 - 1996, Alexander Enzmann, All rights reserved.
Copyright(C) 1999 - 2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files(the Software), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and /or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#if !defined(__POLYRAY_SYMTAB_RUNTIME_DEFS)
#define __POLYRAY_SYMTAB_RUNTIME_DEFS

#include "display_backend.h"

#include <string>

#include "defs3.h"
#include "lex.yy.h"//for reflex


// Animation variables for generating frame sequences 
struct Animation {
	int start_frame=0;
	int end_frame=-1;
	int total_frames=0;
	int current_frame=0;
	Flt frame_time=1.0;//amount of time which passes per frame
	
	void reset() noexcept { *this = {}; }
};
    


	    
	

// Rendering quality variables 
struct RenderSettings {
		int pixelsize = 24;//24 bits per pixel, 8 each RGB.
		int pixel_encoding = 1;// RLE compression default
		int DepthRender = 0;// Defaults to normal colouring 
		Flt minweight = 0.01;
		int maxlevel = MAXLEVEL;
		int antialias = 2;//previously:0; 0=none, 1=filter, 2-3=adaptive 
		int maxsamples = DEFAULT_SAMPLES;
		Flt antialias_threshold = DEFAULT_THRESHOLD * DEFAULT_THRESHOLD;
		long            MaxBufferRAM = 2048L * 2048L;
		Flt             csg_leg_tolerance = 0.05;
		Flt             csg_subdivision_depth = 1;
		rmode             Render_Method= rmode::RAY_TRACING;
		int Optimizer=1;// Slabs, array based sorting 
		int clustersize = 4;//Bounding cluster size for ray tracing
		int tickflag = 2;//0=none,1=totals,2=line,3=pixel(SUGGESTED?CHECK!)
		bool check_colineartriangles = false;
		void reset() noexcept { *this = {}; }
	};

	struct RenderStatistics {
		unsigned long nRays = 0;
		unsigned long nChecked = 0;
		unsigned long nShadows = 0;
		unsigned long nReflected = 0;
		unsigned long nRefracted = 0;
		unsigned long nTIR = 0;
		unsigned long nJittered = 0;
		unsigned long totalShadows, totalShadowCaches;
		unsigned long maxQueueSize;
		unsigned long totalQueues;
		unsigned long totalQueueResets;
		unsigned long nEnqueued;

		
		void reset() noexcept { *this = {}; }
	};

	struct SceneState {
		unsigned short  Global_Shade_Flag = UNSET_SFLAG;
		Viewpoint       Eye = { 256, 256, 0, 0, 256, 256, 0, 255,
								{0, 0, -1}, {0, 0, 0}, {0, 1, 0},
								45.0, SMALL, PLY_HUGE, 0.0, 1.0, -1.0,
								nullptr, nullptr, nullptr, nullptr, {} };
		BinTree         Root;
		void reset() noexcept { *this = {}; }
	};

	namespace RuntimeState
	{
		extern RenderStatistics stats;
		extern RenderSettings settings;
		extern Animation animator;
		extern SceneState scene;

		/* Runtime status variables */
		extern int Parsed_Flag;
		extern int Shadow_Test;
		extern int Particle_Test;

		// Global image variables 
		extern int             buffer_update;
		extern unsigned long   buffer_size;

		extern Light* Lights;
		extern int   nLights;
		extern Light** light_array;

		extern YY_BUFFER_STATE oldbuf;
		extern YY_BUFFER_STATE newbuf;


		extern int Display_Flag;
		extern Surface         DefaultSurface;
		extern Flt             rayeps;
		

		extern std::string     outfilebase;
		extern int             filebaseflag;
		extern OUT_FORMAT      outformat;

		extern int     Check_Abort_Flag;
		extern int     Abort_Flag;
		extern jmp_buf abort_environ;

		// Debug/ runtime screen variables 
		extern int current_row;
		extern int current_col;
		extern int recursion_depth;

		extern std::string POLYRAY_PATH_STRING;
		extern std::string POLYRAY_DAT_STRING;
		extern std::string POLYRAY_PATH;
		extern std::string POLYRAY_DAT;

		extern int File_Generation_Flag;
		extern int Allow_SystemCalls;


		extern Flt             Global_Haze;// = 0.0;
		extern Flt             Global_Haze_Start;// = 0.0;
		extern Vec             Global_Haze_Color;// = { 0, 0, 0 };
		extern NODE_PTR        Background;// = nullptr;
		extern Vec             BackgroundColor;
		extern Vec             White;// = { 1.0, 1.0, 1.0 };


		//helper functions
		extern void Reset_Runtime_State(void);
		extern void Reset_Frame_State(void);
		extern void Reset_Render_Statistics(void);
	};//namespace RuntimeState

/* Backward-compatible alias while remaining files migrate. */
namespace runtimeState = RuntimeState;


#endif // __POLYRAY_SYMTAB_RUNTIME_DEFS

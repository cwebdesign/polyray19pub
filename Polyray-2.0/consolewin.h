// This is a personal academic project. Dear PVS-Studio, please check it.

//consolewin.h
#pragma once
#ifndef CONSOLEWIN_H
#define CONSOLEWIN_H

#include <iostream>
#include <array>
#include <string>
#include <string_view>

#include "defs3.h" 

#include "psupport.h"

//#define USE_SDL

#ifdef USE_SDL
#ifdef _MXSC_VER
#include "sdl\SDL3\SDL.h"
#else
//#define SDL_MAIN_HANDLED
#include "display_backend.h"
#endif
#endif





//#include "io.h"
//#include "parse.h"
//#include "memory.h"



namespace openpolyray
{

	class ConsoleWin {
	private:		
		int Pallette_Start = 0;      /* Pallette entries start at 0 */
		int Pallette_Flag = 1;       /* Use 884 pallette */
		int Display_Flag = 1;        /* Use standard VGA */
		int Reset_Display_Flag=0;

		/* Define the top right, the width, and the length of usuable display area */
		int Display_x0 = -1, Display_y0 = -1;
		int Display_xl = -1, Display_yl = -1;
#ifdef USE_SDL
		SDL_Window* window = nullptr;
		SDL_Renderer* renderer = nullptr;
		SDL_Texture* canvas = nullptr;   // persistent render target so pixels accumulate across presents
		Uint64 last_present = 0;         // SDL_GetTicks() at last present, for throttling
		void present(bool force);        // drain events + blit canvas to screen (throttled unless force)
#endif
	public:
		ConsoleWin(void);
		//virtual Rgb value(const vec2 & uv, const vec3 & p) const;
		~ConsoleWin(void) {
#ifdef USE_SDL
			SDL_DestroyTexture(canvas);
			SDL_DestroyWindow(window);
			SDL_Quit();
#endif
		};
		int get_Display_Flag(void) { return Display_Flag; }
		void set_Display_Flag(int _df) { Display_Flag = _df; }
		int get_Reset_Display_Flag(void) { return Reset_Display_Flag; }
		void set_Reset_Display_Flag(int _df) { Reset_Display_Flag = _df; }

		int get_Pallette_Start(void) {
			return Pallette_Start;
		}
		void set_Pallette_Start(int _p)
		{
			Pallette_Start = _p;
			//::Pallette_Start = _p;
		}
		int get_Pallette_Flag(void) {
			return Pallette_Flag;
		}
		void set_Pallette_Flag(int _p)
		{
			Pallette_Flag = _p;
			//::Pallette_Start = _p;
		}
		void display_plot(int x, int y, Vec color);
		void display_box(int x0, int y0, int x1, int y1, Vec color);
		void display_close(int wait_flag);
		void display_init(Viewpoint* eye, const std::string_view& S);
		void display_line(int x0, int y0, int x1, int y1, Vec color);
		void setx0(int _x0) { Display_x0 = _x0; };
		void setxl(int _xl) { Display_xl = _xl; };
		void sety0(int _y0) { Display_y0 = _y0; };
		void setyl(int _yl) { Display_yl = _yl; };
		int getch(void);
		bool kbhit(void);


	};


	
}
#endif
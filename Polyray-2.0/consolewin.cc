/* consolewin.cc

  Copyright (C) 2021-2024, C. Meli, All rights reserved.

  Polyray - MIT Licensed Revival
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

#include <iostream>
#include <algorithm>
#include <string>
#include <string_view>

#include "defs3.h"
//#include "display.h"
#include "io_ply.h"
//#include "unixcompat.h"
#include "consolewin.h"
#include "util.h"


namespace openpolyray
{
	constexpr int WIDTH = 640;
	constexpr int HEIGHT = 480;

    ConsoleWin::ConsoleWin(void) {
#ifdef USE_SDL
		if (!SDL_Init(SDL_INIT_VIDEO)) {
			std::cout << "SDL_Init failed: "<<SDL_GetError()<<"\n";
			exit(-1);
		}
		window = SDL_CreateWindow("Hello, World!",
			WIDTH, HEIGHT,
			SDL_WINDOW_HIGH_PIXEL_DENSITY);
		if (window == nullptr) {
			std::cout <<"Could not create window: "<<SDL_GetError()<<"\n";
			exit(-2);
		}

		renderer = SDL_CreateRenderer(window, nullptr);
		if (renderer == nullptr) {
			std::cout << "Could not create renderer: " << SDL_GetError() << "\n";
			exit(-3);
		}
		//std::cout << "SDL render driver: " << SDL_GetRendererName(renderer) << "\n";

		// Bring the window to the front: on Windows it otherwise often opens
		// behind the launching console and looks like it never appeared.
		SDL_ShowWindow(window);
		SDL_RaiseWindow(window);

		// Draw into a persistent target texture: a plain renderer's backbuffer is
		// undefined after each present (notably the macOS Metal backend), so
		// incremental pixel plotting only survives if it accumulates in a texture.
		canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT);
		if (canvas == nullptr) {
			std::cout << "Could not create render-target texture: " << SDL_GetError() << "\n";
			exit(-4);
		}
		SDL_SetRenderTarget(renderer, canvas);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		present(true);   // show the (empty) window immediately and pump the event queue
#endif
    };

#ifdef USE_SDL
	// Copy the accumulated canvas to the screen and pump the OS event queue.
	// macOS will not composite/show the window until events are pumped, so this
	// is what actually makes the window visible during a render. Throttled to
	// ~30 fps unless forced, so it does not stall the tracer.
	void ConsoleWin::present(bool force)
	{
		Uint64 now = SDL_GetTicks();
		if (!force && now - last_present < 33)
			return;
		SDL_Event event;
		while (SDL_PollEvent(&event)) { /* discard during render; display_close does the final wait */ }
		SDL_SetRenderTarget(renderer, nullptr);   // target the screen
		SDL_RenderClear(renderer);
		SDL_RenderTexture(renderer, canvas, nullptr, nullptr);
		SDL_RenderPresent(renderer);
		SDL_SetRenderTarget(renderer, canvas);    // resume drawing into the canvas
		last_present = now;
	}
#endif

	bool ConsoleWin::kbhit(void)
	{
#ifdef USE_SDL
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_KEY_DOWN)
			{
				return true;

			}
			return false;
		}
#else
		return _kbhit();
#endif
	}
	int ConsoleWin::getch()
	{
		if (Display_Flag) {
			// while (PollEvents()) ;
			//for now
#ifdef USE_SDL
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_EVENT_KEY_DOWN)
				{
					switch (event.key.key)
					{
					case SDLK_RETURN:
						return '\n';
					case SDLK_0:
						return '0';
					case SDLK_ESCAPE:
						return 27;
					}
					return 0;

				}
				return 0;
			}
#else
			//return std::cin.get();
#if defined(_MSC_VER) 
			return _getch();
#else
  #if defined(__MINGW32__)
			return getchar();
  #else //Linux etc
			return getchar();//openpolyray::getch();
  #endif
#endif
#endif
		}
		else
			return std::cin.get();
	}

	void ConsoleWin::display_plot(int x, int y, Vec color)
	{
#ifdef USE_SDL
		SDL_SetRenderDrawColor(renderer,
			Uint8(std::clamp(color[0], 0.0, 1.0) * 255),
			Uint8(std::clamp(color[1], 0.0, 1.0) * 255),
			Uint8(std::clamp(color[2], 0.0, 1.0) * 255), 255);
		SDL_RenderPoint(renderer, float(x), float(y));
		present(false);
#endif
	}

	void ConsoleWin::display_box(int x0, int y0, int x1, int y1, Vec color)
	{
#ifdef USE_SDL
		SDL_FRect rect{ .x = float(x0), .y = float(y0),
			.w = float(x1 - x0), .h = float(y1 - y0) };
		SDL_SetRenderDrawColor(renderer,
			Uint8(std::clamp(color[0], 0.0, 1.0) * 255),
			Uint8(std::clamp(color[1], 0.0, 1.0) * 255),
			Uint8(std::clamp(color[2], 0.0, 1.0) * 255), 255);
		SDL_RenderRect(renderer, &rect);
		present(false);
#endif
	}
	void ConsoleWin::display_close([[maybe_unused]] int wait_flag)
	{
#ifdef USE_SDL
		present(true);   // blit the completed image to the screen

		// Always pause while the window is up so the finished render is visible
		// until the user dismisses it; the -W wait_flag is redundant here.
		std::cout << "\nFinished. Press a key in the window (or close it) to continue\n";
		bool done = false;
		while (!done) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_EVENT_KEY_DOWN ||
					event.type == SDL_EVENT_QUIT)
					done = true;
			}
			SDL_Delay(10);
		}
#else
		if (wait_flag) {
			std::cout << "\nFinished. Press any key to continue\n";
			getch();
		}
#endif
		//DeinitDisplay();
		return;
	}

	void ConsoleWin::display_init([[maybe_unused]] Viewpoint* eye,
		[[maybe_unused]] const std::string_view& S)
	{
#ifdef USE_SDL
		// Size the window/canvas to the actual render resolution; otherwise a
		// scene larger than the default is clipped to the top-left corner.
		int w = (eye && eye->view_xres > 0) ? eye->view_xres : WIDTH;
		int h = (eye && eye->view_yres > 0) ? eye->view_yres : HEIGHT;

		// Cap the window to the display's usable area (keeping aspect ratio); the
		// canvas stays full render resolution and present() scales it to fit, so a
		// huge render still shows the whole image without an off-screen window.
		int win_w = w, win_h = h;
		SDL_DisplayID disp = SDL_GetDisplayForWindow(window);
		if (disp == 0) disp = SDL_GetPrimaryDisplay();
		SDL_Rect ub;
		if (SDL_GetDisplayUsableBounds(disp, &ub)) {
			int max_w = int(ub.w * 0.9f), max_h = int(ub.h * 0.9f);
			if (win_w > max_w || win_h > max_h) {
				float scale = std::min(float(max_w) / win_w, float(max_h) / win_h);
				win_w = int(win_w * scale);
				win_h = int(win_h * scale);
			}
		}

		std::string title = "Polyray: " + std::string(S);
		SDL_SetWindowSize(window, win_w, win_h);
		SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		SDL_SetWindowTitle(window, title.c_str());

		// Recreate the accumulation canvas at the render resolution.
		SDL_DestroyTexture(canvas);
		canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_TARGET, w, h);
		if (canvas == nullptr) {
			std::cout << "Canvas " << w << "x" << h << " failed ("
				<< SDL_GetError() << "), falling back to " << WIDTH << "x" << HEIGHT << "\n";
			SDL_SetWindowSize(window, WIDTH, HEIGHT);
			canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
				SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT);
		}
		SDL_SetRenderTarget(renderer, canvas);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RaiseWindow(window);
		present(true);
#endif
	}
	void ConsoleWin::display_line(int x0, int y0, int x1, int y1, Vec color)
	{
#ifdef USE_SDL
		SDL_SetRenderDrawColor(renderer,
			Uint8(std::clamp(color[0], 0.0, 1.0) * 255),
			Uint8(std::clamp(color[1], 0.0, 1.0) * 255),
			Uint8(std::clamp(color[2], 0.0, 1.0) * 255), 255);
		SDL_RenderLine(renderer, float(x0), float(y0), float(x1), float(y1));
		present(false);
#endif
	}
	

}




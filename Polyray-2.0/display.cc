/* display.cc

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

#include <format>
#include <string>

#include "defs3.h"
#include "display.h"
#include "memory.h"
#include "io_ply.h"

#ifdef USE_SDL
#include "consolewin.h"
static openpolyray::ConsoleWin *cw = nullptr;
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <conio.h> //for _kbhit()
#else
int getch(); // MinGW gets this from <conio.h> via defs3.h; everyone else needs it
#endif

/** @namespace Global
 *  @brief Display-mode configuration variables shared across the rendering pipeline.
 *
 *  All variables are initialised to their default values at program start.
 *  They may be overridden by command-line options before display_init() is called.
 */
namespace Global {
int Pallette_Start = 0;      /**< First palette entry index (default 0). */
int Pallette_Flag = 1;       /**< Palette mode: 0=greyscale, 1=884, 2=666, 3=EGA. */
int Display_Flag = 1;        /**< Active video mode; 0 after display_init on stub builds. */
int Reset_Display_Flag = 1;  /**< Non-zero to switch the adapter into the requested mode. */
int Dither_Flag = 0;         /**< Non-zero to apply ordered dithering when quantising pixels. */

/** @brief Left edge of the renderable screen window (-1 = use full width). */
int Display_x0 = -1;
/** @brief Top edge of the renderable screen window (-1 = use full height). */
int Display_y0 = -1;
/** @brief Width of the renderable screen window (-1 = use full width). */
int Display_xl = -1;
/** @brief Height of the renderable screen window (-1 = use full height). */
int Display_yl = -1;
}




/** @brief Plot a single pixel on the display.
 *  @param x     Horizontal pixel coordinate.
 *  @param y     Vertical pixel coordinate.
 *  @param color RGB colour in [0,1] range.
 *  @return No return value.
 */
void display_plot(int x, int y, Vec color)
{
 #ifdef USE_SDL
  if (cw) cw->display_plot(x, y, color);
#endif
}

/** @brief Fill a rectangular region on the display (MINGW stub - no-op).
 *  @param x0    Left column.
 *  @param y0    Top row.
 *  @param x1    Right column (exclusive).
 *  @param y1    Bottom row (exclusive).
 *  @param color RGB fill colour in [0,1] range.
 *  @return No return value.
 */
void display_box(int x0, int y0, int x1, int y1, Vec color)
{
  #ifdef USE_SDL
  if (cw) cw->display_box(x0, y0, x1, y1, color);
#endif
}

/** @brief Tear down the display and optionally wait for a keypress (MINGW).
 *  @param wait_flag  Non-zero to print a completion message and call getch().
 *  @return No return value.
 */
void display_close(int wait_flag)
{
#ifdef USE_SDL
  if (cw) {
    cw->display_close(wait_flag);
    return;
  }
#endif
  if (wait_flag) {
//    printf("\nFinished. Click into the window to continue\n");
//    SpecialStatus("Done");
    std::cout<<"\nFinished. Press any key to continue\n";
#if !defined(__APPLE__) && !defined(__sun) && !defined(__sun__) && !defined(__illumos__)
    getch();
#endif
  }
//  DeinitDisplay();
  return;
}

/** @brief Initialise the display for rendering (MINGW).
 *  @details Formats the window title string and sets Global::Display_Flag to 0,
 *           indicating that no live display is active.
 *  @param eye  Viewpoint supplying the output resolution (unused on this build).
 *  @param s1   Scene name used to build the window title string.
 *  @return No return value.
 */
void display_init(Viewpoint *eye, char *s1)
{
  std::string ss1{};
  if (s1) ss1=std::string(s1);
  std::string s = std::format("Polyray: {}", ss1);
  
  #ifdef USE_SDL
    if (!cw) cw = new openpolyray::ConsoleWin();   // window is created here
  cw->display_init(eye, s);
  Global::Display_Flag = 1;
  #else
  Global::Display_Flag = 0;
  #endif
}

/** @brief Draw a line between two screen points (MINGW stub - no-op).
 *  @param x0    Start column.
 *  @param y0    Start row.
 *  @param x1    End column.
 *  @param y1    End row.
 *  @param color RGB line colour in [0,1] range.
 *  @return No return value.
 */
void display_line(int x0, int y0, int x1, int y1, Vec color)
{
  #ifdef USE_SDL
  if (cw) cw->display_line(x0, y0, x1, y1, color);
#endif
}

/** @brief Read one character from stdin (MINGW).
 *  @return The character code returned by getchar().
 */
int getch()
{
  #ifdef USE_SDL
  if (cw) return cw->getch();
  #endif
  return getchar();
}




#pragma once

#if defined(__sun) || defined(__sun__) || defined(__illumos__)
#define POLYRAY_PLATFORM_TRIBBLIX 1
#endif

#if !defined(POLYRAY_PLATFORM_TRIBBLIX)
#include <SDL3/SDL.h>
#define POLYRAY_HAS_SDL3 1
  #ifdef SDL_MAIN_HANDLED
  #include <SDL3/SDL_main.h> // REQUIRED in SDL3 for the entry point to work. this should only be called by the main.cc file
  #endif


extern const Uint8* keys;//cm - only for SDL
#else
#define POLYRAY_HAS_SDL3 0
#endif



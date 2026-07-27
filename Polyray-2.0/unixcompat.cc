/** @file unixcompat.cc
 *  @brief Platform-compatibility shims for non-Unix build targets.
 *
 *  Provides thin wrappers that smooth over differences between POSIX/GCC and
 *  MSVC/Windows builds:
 *  - polyray_pause() - cross-platform "press enter to continue" pause.
 *  - dumpversion()   - prints the build version string via sstatus().
 *  - mkstemp()       - POSIX temporary-file function, emulated on MSVC via
 *                      @c tmpnam_s (active) or a full Win32 implementation
 *                      (disabled; guarded by @c _MSC_VERUNUSED).
 *
 *  Polyray - MIT Licensed Revival
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

//#if defined(__CYGWIN__)
//#include <ncurses/ncurses.h> //instead of the old conio
//#else
//#include <ncurses.h>
//#endif
#include <stdio.h>
#include <string.h> //strlen

#include <string>
//#include <SDL.h>
#include "io_ply.h"
#include "unixcompat.h"
#include "version.h"

#include <iostream>

namespace openpolyray {
	//#if defined(__unix__)||defined(__UNIX__)||defined(unix)||defined(linux)||defined(__linux)

	/** @brief Pause execution and prompt the user to press Enter.
	 *
	 *  On non-SDL builds this is effectively a no-op (the blocking @c getchar()
	 *  call is commented out).  On SDL builds the SDL event pump would be used,
	 *  but that path is also disabled.  The function exists so that interactive
	 *  debug sessions can insert a breakpoint-friendly pause without a platform
	 *  @c getch() / @c kbhit() dependency.
	 */
	void polyray_pause(void)
	{
		puts("Press enter to continue");
#ifdef USE_SDL
		//SDL_PumpEvents();

		//do {
		//} while (!keys[SDLK_Return]);
#else
		//fflush(stdout);//otherwise it will not be seen!
		//while (getchar() !='\n')
		;
#endif
	}

	/** @brief Print the renderer version string to the status output.
	 *
	 *  Calls sstatus() with a build-specific version banner that includes the
	 *  build codename, compiler, and copyright notice.  The banner differs
	 *  between MSVC (@c _MSC_VER defined) and GCC/Clang builds.
	 */
	void dumpversion(void)
	{
#if !defined(__GNUC__)
		sstatus("Polyray " POLYRAY_VERSION_STRING " 2026 (\"" POLYRAY_CODENAME "\" Build), Compiled with Visual Studio. Copyright (C) 1996-2026 Dr Clyde Meli. Copyright (c) 1994 Alexander Enzmann\n");
#else
		sstatus("Polyray " POLYRAY_VERSION_STRING " 2026 (\"" POLYRAY_CODENAME "\" Build), Compiled with GCC. Copyright (C) 1996-2026 Dr Clyde Meli. Copyright (c) 1994 Alexander Enzmann\n");
#endif
	}

	//https://stackoverflow.com/questions/22028142/read-only-one-char-from-cin
//getch();}   //equivalent
//#endif

#ifdef _MSC_VER
	/** @brief POSIX @c mkstemp() emulation for MSVC using @c tmpnam_s.
	 *
	 *  Generates a unique temporary filename by delegating to the CRT
	 *  @c tmpnam_s(), which writes at most 16 characters into @p tmp.
	 *  The file is @b not opened; callers must open it themselves after
	 *  this function returns, accepting the inherent TOCTOU race.
	 *
	 *  @note The template string must be at most 15 characters (null
	 *        terminator included) to fit the 16-byte @c tmpnam_s buffer.
	 *        Unlike POSIX @c mkstemp, the trailing "XXXXXX" convention is
	 *        not enforced.
	 *
	 *  @param tmp  Input/output buffer holding the filename template; overwritten
	 *              with a unique name on success.  Must be at most 15 chars.
	 *  @return     0 on success (the @c tmpnam_s return value); -1 if
	 *              @p tmp exceeds 15 characters or @c tmpnam_s fails.
	 */
	int mkstemp(char* tmp) //max 16 chars!!
	{
		FILE* stream;
		if (strlen(tmp) > 15) return -1;

		errno_t tmp_fd;
		tmp_fd = tmpnam_s(tmp, 16);
		if (tmp_fd) {
			return -1;// error("error: failed to create temporary file\n");
		}
		return tmp_fd;

	}
#endif
#ifdef _MSC_VERUNUSED /* intentionally dead - _MSC_VERUNUSED is never defined */
	/** @brief Full POSIX @c mkstemp() emulation for Win32 (disabled - requires MFC).
	 *
	 *  Derived from glibc @c sysdeps/posix/tempname.c (LGPL 2.1+).  Uses
	 *  @c GetSystemTime() and @c GetCurrentThreadId() to seed a counter that
	 *  generates up to @c ATTEMPTS_MIN (62^3) candidate names by substituting
	 *  base-62 characters into the trailing six @c X positions of @p tmpl.
	 *  Each candidate is tried with @c open(O_CREAT|O_EXCL) until one succeeds.
	 *
	 *  @note Guarded by @c _MSC_VERUNUSED which is never defined, so this block
	 *        is compiled out unconditionally.  Use the @c tmpnam_s variant above
	 *        for MSVC builds.
	 *
	 *  @param tmpl  Filename template ending in exactly six @c X characters;
	 *               overwritten in-place with the unique name on success.
	 *  @return      A valid file descriptor (>= 0) on success; -1 with @c errno
	 *               set on failure.
	 */
	//https://stackoverflow.com/questions/6036227/mkstemp-implementation-for-win32
	/* mkstemp extracted from libc/sysdeps/posix/tempname.c.  Copyright
	(C) 1991-1999, 2000, 2001, 2006 Free Software Foundation, Inc.

	The GNU C Library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.  */

	static const char letters[] =
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

#include <errno.h>
	/* Generate a temporary file name based on TMPL.  TMPL must match the
	rules for mk[s]temp (i.e. end in "XXXXXX").  The name constructed
	does not exist at the time of the call to mkstemp.  TMPL is
	overwritten with the result.  */
	int
		mkstemp(char* tmpl)
	{
		int len;
		char* XXXXXX;
		static unsigned long long value;
		unsigned long long random_time_bits;
		unsigned int count;
		int fd = -1;
		int save_errno = errno;

		/* A lower bound on the number of temporary files to attempt to
		generate.  The maximum total number of temporary file names that
		can exist for a given template is 62**6.  It should never be
		necessary to try all these combinations.  Instead if a reasonable
		number of names is tried (we define reasonable as 62**3) fail to
		give the system administrator the chance to remove the problems.  */
#define ATTEMPTS_MIN (62 * 62 * 62)

		/* The number of times to attempt to generate a temporary file.  To
		conform to POSIX, this must be no smaller than TMP_MAX.  */
#if ATTEMPTS_MIN < TMP_MAX
		unsigned int attempts = TMP_MAX;
#else
		unsigned int attempts = ATTEMPTS_MIN;
#endif

		len = strlen(tmpl);
		if (len < 6 || strcmp(&tmpl[len - 6], "XXXXXX"))
		{
			errno = EINVAL;
			return -1;
		}

		/* This is where the Xs start.  */
		XXXXXX = &tmpl[len - 6];

		/* Get some more or less random data.  */
		{
			SYSTEMTIME      stNow;
			FILETIME ftNow;

			// get system time
			GetSystemTime(&stNow);
			stNow.wMilliseconds = 500;
			if (!SystemTimeToFileTime(&stNow, &ftNow))
			{
				errno = -1;
				return -1;
			}

			random_time_bits = (((unsigned long long)ftNow.dwHighDateTime << 32)
				| (unsigned long long)ftNow.dwLowDateTime);
		}
		value += random_time_bits ^ (unsigned long long)GetCurrentThreadId();

		for (count = 0; count < attempts; value += 7777, ++count)
		{
			unsigned long long v = value;

			/* Fill in the random bits.  */
			XXXXXX[0] = letters[v % 62];
			v /= 62;
			XXXXXX[1] = letters[v % 62];
			v /= 62;
			XXXXXX[2] = letters[v % 62];
			v /= 62;
			XXXXXX[3] = letters[v % 62];
			v /= 62;
			XXXXXX[4] = letters[v % 62];
			v /= 62;
			XXXXXX[5] = letters[v % 62];

			fd = open(tmpl, O_RDWR | O_CREAT | O_EXCL, _S_IREAD | _S_IWRITE);
			if (fd >= 0)
			{
				errno = save_errno;
				return fd;
			}
			else if (errno != EEXIST)
				return -1;
		}

		/* We got out of the loop because we ran out of combinations to try.  */
		errno = EEXIST;
		return -1;
	}
#endif

} //namespace



  /* Unix compatibity defines */

//#if defined(__CYGWIN__)
//#include <ncurses/ncurses.h> //instead of the old conio
//#else
//#include <ncurses.h>
//#endif
#include <stdio.h>
#include "io.h"

//#include <iostream>

//namespace polyray {
//#if defined(__unix__)||defined(__UNIX__)||defined(unix)||defined(linux)||defined(__linux)

//char kbhit(void) {return std::cin(); }
// Read single characters from cin.
    void polyray_pause(void)
    {
		puts("Press enter to continue");
		fflush(stdout);//otherwise it will not be seen!
		while (getchar() !='\n')
		  ;
	}

void dumpversion(void)
{
#if !defined(__GNUC__)
            status("Polyray v1.9.3 2043, (\"Asteroids\" Build), Compiled with Visual Studio for CIS3105. Portions (C) 1996-2024 Clyde Meli. Copyright (c) 1994 Alexander Enzmann\n");
#else
            //status("Polyray v2.0 2019 (\"PlusPlus 64\" Build), Compiled with GCC 7.4.0(Ubuntu for Windows version) for CIS3105. Portions (C) 1996-2019 Dr Clyde Meli. Copyright (c) 1994 Alexander Enzmann\n");
            status("Polyray v1.9.3 2024 (\"Travel Malta\" Build), Compiled with GCC for CIS3105. Portions (C) 1996-2024 Dr Clyde Meli. Copyright (c) 1994 Alexander Enzmann\n");

#endif
}

	//https://stackoverflow.com/questions/22028142/read-only-one-char-from-cin
//getch();}   //equivalent
//#endif

#ifdef _MSC_VER
int mkstemp(char *tmp) //max 16 chars!!
{
	FILE *stream;
	if (strlen(tmp) > 15) return -1;

	errno_t tmp_fd;
	tmp_fd = tmpnam_s(tmp, 16);
	if (tmp_fd) {
		return -1;// error("error: failed to create temporary file\n");
	}
	return tmp_fd;
}
#endif
#ifdef _MSC_VERUNUSED //not used! needs MFC
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
mkstemp(char *tmpl)
{
	int len;
	char *XXXXXX;
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


//}

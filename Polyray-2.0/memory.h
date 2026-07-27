#pragma once
/* memory.h

  Polyray - MIT Licensed Revival
Copyright(C) 1993 - 1996, Alexander Enzmann, All rights reserved.
Copyright(C) 1999 - 2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and /or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", (C), WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#if !defined(__POLYRAY_MEMORY_DEFS)
#define __POLYRAY_MEMORY_DEFS

#include <stdlib.h>
#include <string.h>

/* #define DEBUG_POINTERS */
#if defined( DEBUG_POINTERS )
#define polyray_malloc(x) debug_malloc(__FILE__, __LINE__, x)
#define polyray_free(x) debug_free(__FILE__, __LINE__, x)
#else
#define polyray_malloc(x) default_malloc(x)
#define polyray_free(x) default_free(x)
#endif

/* Memory allocation functions (providing hooks for tests) */
void *debug_malloc(const char *, int, size_t);
void debug_free(const char *, int, void *);
void *default_malloc(size_t);
void default_free(void *);
void allocation_status();
void free_all_memory();

/* Memory monitoring variables */
extern unsigned long nMalloc;
extern unsigned long nFree;

#endif /* __POLYRAY_MEMORY_DEFS */

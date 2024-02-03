/* memory.h

  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.

  This software may be used for any private and non-commercial
  use.

  You may not distribute this software, in whole or in part,
  for any commercial purpose, without the express consent of
  the authors.

  There is no warranty or other guarantee of fitness of this software
  for any purpose.  It is provided solely "as is".

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
void *debug_malloc(char *, int, size_t);
void debug_free(char *, int, void *);
void *default_malloc(size_t);
void default_free(void *);
void allocation_status(void);
void free_all_memory(void);

/* Memory monitoring variables */
extern unsigned long nMalloc;
extern unsigned long nFree;

#endif /* __POLYRAY_MEMORY_DEFS */


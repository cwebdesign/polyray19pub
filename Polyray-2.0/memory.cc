/* memory.cc 

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
#include "memory.h"
#include "io_ply.h"

typedef struct memory_chain_struct *memory_chain;
struct memory_chain_struct {
#if defined( DEBUG_POINTERS )
   const char *filename;
   int lineno;
#endif
   size_t size;
   memory_chain last, next;
   };

/* Memory monitoring variables */
unsigned long nMalloc = 0;
unsigned long nFree   = 0;

static unsigned long nMallocCount = 0;
static unsigned long nFreeCount = 0;
static memory_chain memory_chain_head = NULL;
static int debug_memory = 1;

/* Should we check for valid pointers prior to a free? */
/* #define COMPLETE_MEMORY_DEBUG */

//! Debug Memory Allocation
/*!
      Allocates memory with debug tracking and file/line information.
      Maintains a linked list of allocated blocks for memory leak detection.
      \param filename Source file name where allocation was requested
      \param lineno Line number where allocation was requested
      \param size Number of bytes to allocate
      \return Pointer to allocated memory, exits on failure
      \note Tracks allocation count and total size for statistics
*/
void *debug_malloc(const char *filename, int lineno, size_t size)
{
   #ifdef DEBUG_FN_CALLS
		//  printf("memory::debug_malloc\n");
   #endif
   void *ptr;
   memory_chain memptr;
   unsigned ptr_blk_size;

   if (debug_memory) {
      if (size == 0)
         swarning("Zero allocation at %s, line %d\n", filename, lineno);
      ptr_blk_size = sizeof(struct memory_chain_struct);
      ptr = malloc(size + ptr_blk_size);
      if (ptr == NULL)
         serror("Failed malloc(%ld) at line %d of file %s\n",
               (long)size, lineno, filename);
      memptr = (memory_chain)ptr;
#if defined( DEBUG_POINTERS )
      memptr->filename = filename;
      memptr->lineno   = lineno;
#endif
      memptr->size     = size;
      memptr->next     = memory_chain_head;
      memptr->last     = NULL;
      if (memory_chain_head != NULL)
         memory_chain_head->last = memptr;
      memory_chain_head = memptr;

      /* Step the pointer over the information we have stored */
      ptr = (void *)((char *)ptr + ptr_blk_size);
      }
   else
      ptr = malloc(size);
   nMalloc += size;
   nMallocCount += 1;
   #ifdef DEBUG_FN_CALLS
		  //printf("memory::debug_malloc returning\n");
   #endif
   return ptr;
}

//! Debug Memory Deallocation
/*!
      Deallocates memory with debug tracking and validity checking.
      Removes the block from the memory chain and updates statistics.
      \param filename Source file name where deallocation was requested
      \param lineno Line number where deallocation was requested
      \param ptr Pointer to memory to deallocate (must not be NULL)
      \return void
      \note Exits with error if attempt to free NULL or double-free is detected
*/
void debug_free(const char *filename, int lineno, void *ptr)
{
   #ifdef DEBUG_FN_CALLS
		  //printf("memory::debug_free\n");
   #endif
   memory_chain oldptr;
#if defined( COMPLETE_MEMORY_DEBUG )
   memory_chain tempptr;
#endif

   if (ptr == NULL) {
      if (filename != NULL)
         serror("attempt to deallocate NULL at %s, line %d\n", filename, lineno);
      else
         serror("attempt to deallocate NULL\n");
      }

   if (debug_memory) {
      oldptr = (memory_chain)((char *)ptr - sizeof(struct memory_chain_struct));

#if defined( COMPLETE_MEMORY_DEBUG )
      for (tempptr=memory_chain_head;
           tempptr!=NULL && tempptr != oldptr;
           tempptr = tempptr->next)
          ;
      if (tempptr != oldptr) {
         fatal((char*)"Attempt to double free at %s, line %d\n", filename, lineno);
         return;
         }
#endif
    //printf("reset the connections\n");
      /* Reset the connections within the memory chain */
      if (oldptr->next != NULL)
         oldptr->next->last = oldptr->last;

      if (oldptr->last == NULL) {
         memory_chain_head = oldptr->next;
         if (memory_chain_head != NULL)
            memory_chain_head->last = NULL;
         }
      else
         oldptr->last->next = oldptr->next;

      nFree += oldptr->size;
      nFreeCount += 1;
      #ifdef DEBUG_FN_CALLS
		  //printf("in memory::debug_malloc calling free oldptr\n");
      #endif
      free((void *)oldptr);
      oldptr = NULL;
      }
   else {
      /* Can't keep track of how much we dumped, only that we free'd it */
      nFreeCount += 1;
      #ifdef DEBUG_FN_CALLS
		  //printf("in memory::debug_malloc calling free ptr\n");
      #endif
      free(ptr);
      ptr = NULL;
      }
}

//! Default Memory Allocation
/*!
      Wrapper for debug_malloc with no file/line tracking.
      \param size Number of bytes to allocate
      \return Pointer to allocated memory
*/
void *default_malloc(size_t size)
{
   return debug_malloc(NULL, 0, size);
}

//! Default Memory Deallocation
/*!
      Wrapper for debug_free with no file/line tracking.
      \param ptr Pointer to memory to deallocate
      \return void
*/
void default_free(void *ptr)
{
   debug_free(NULL, 0, ptr);
}

//! Print Memory Allocation Statistics
/*!
      Prints memory allocation/deallocation statistics and lists any remaining allocated blocks.
      Shows total allocated, freed, allocation count, free count, and unfreed memory with source info.
      \return void
*/
void allocation_status()
{
   memory_chain tempptr;

   if (debug_memory) {
      smessage("alloc: %-8ld, free: %-8ld, acount: %-8ld, fcount: %-8ld\n",
              nMalloc, nFree, nMallocCount, nFreeCount);
      if (memory_chain_head != NULL) {
         smessage("Leftovers:\n");
         tempptr = memory_chain_head;
         while (tempptr != NULL) {
#if defined( DEBUG_POINTERS )
            message("   File: '%s', Line: %d, Size: %ld, ptr: %p\n",
                   tempptr->filename, tempptr->lineno, (long)tempptr->size,
                   tempptr);
#else
            smessage("   Size: %ld, ptr: %p\n", (long)tempptr->size, tempptr);
#endif
            tempptr = tempptr->next;
            }
         }
      else if (nMallocCount - nFreeCount != 0)
         smessage("Unaccounted memory: %ld\n", nMallocCount - nFreeCount);
      }
   nMallocCount = 0;
   nFreeCount = 0;
}

//! Free All Allocated Memory
/*!
      Deallocates all memory in the memory chain.
      Called at program termination to clean up all tracked allocations.
      \return void
*/
void
free_all_memory()
{
   #ifdef DEBUG_FN_CALLS
		  printf("memory::free_all_memory\n");
   #endif
   int cnt;
   memory_chain tempptr;

   for (cnt=0;memory_chain_head!=NULL;cnt++) {
      tempptr = memory_chain_head;
      memory_chain_head = memory_chain_head->next;
      free(tempptr);
      tempptr=NULL;
      }
/*
   if (debug_memory)
      status("Reclaimed %d allocs\n", cnt);
*/
}

#include "memory.h"
#include "io.h"

typedef struct memory_chain_struct *memory_chain;
struct memory_chain_struct {
#if defined( DEBUG_POINTERS )
   char *filename;
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

void *
debug_malloc(char *filename, int lineno, size_t size)
{
   void *ptr;
   memory_chain memptr;
   unsigned ptr_blk_size;

   if (debug_memory) {
      if (size == 0)
         warning("Zero allocation at %s, line %d\n", filename, lineno);
      ptr_blk_size = sizeof(struct memory_chain_struct);
      ptr = malloc(size + ptr_blk_size);
      if (ptr == NULL)
         error("Failed malloc(%ld) at line %d of file %s\n",
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
   return ptr;
}

void
debug_free(char *filename, int lineno, void *ptr)
{
   memory_chain oldptr;
#if defined( COMPLETE_MEMORY_DEBUG )
   memory_chain tempptr;
#endif

   if (ptr == NULL) {
      if (filename != NULL)
         error("attempt to deallocate NULL at %s, line %d\n", filename, lineno);
      else
         error("attempt to deallocate NULL\n");
      }

   if (debug_memory) {
      oldptr = (memory_chain)((char *)ptr - sizeof(struct memory_chain_struct));

#if defined( COMPLETE_MEMORY_DEBUG )
      for (tempptr=memory_chain_head;
           tempptr!=NULL && tempptr != oldptr;
           tempptr = tempptr->next)
          ;
      if (tempptr != oldptr) {
         fatal("Attempt to double free at %s, line %d\n", filename, lineno);
         return;
         }
#endif

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
      free((void *)oldptr);
      }
   else {
      /* Can't keep track of how much we dumped, only that we free'd it */
      nFreeCount += 1;
      free(ptr);
      }
}

void *
default_malloc(size_t size)
{
   return debug_malloc(NULL, 0, size);
}

void
default_free(void *ptr)
{
   debug_free(NULL, 0, ptr);
}

/* Print out statistics on how much was allocated, how much was
   freed, and what leftovers still exist and where they came from */
void
allocation_status()
{
   memory_chain tempptr;

   if (debug_memory) {
      message("alloc: %-8ld, free: %-8ld, acount: %-8ld, fcount: %-8ld\n",
              nMalloc, nFree, nMallocCount, nFreeCount);
      if (memory_chain_head != NULL) {
         message("Leftovers:\n");
         tempptr = memory_chain_head;
         while (tempptr != NULL) {
#if defined( DEBUG_POINTERS )
            message("   File: '%s', Line: %d, Size: %ld, ptr: %p\n",
                   tempptr->filename, tempptr->lineno, (long)tempptr->size,
                   tempptr);
#else
            message("   Size: %ld, ptr: %p\n", (long)tempptr->size, tempptr);
#endif
            tempptr = tempptr->next;
            }
         }
      else if (nMallocCount - nFreeCount != 0)
         message("Unaccounted memory: %ld\n", nMallocCount - nFreeCount);
      }
   nMallocCount = 0;
   nFreeCount = 0;
}

void
free_all_memory()
{
   int cnt;
   memory_chain tempptr;

   for (cnt=0;memory_chain_head!=NULL;cnt++) {
      tempptr = memory_chain_head;
      memory_chain_head = memory_chain_head->next;
      free(tempptr);
      }
/*
   if (debug_memory)
      status("Reclaimed %d allocs\n", cnt);
*/
}

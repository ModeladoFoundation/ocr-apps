/// \file
/// Wrappers for memory allocation.

#ifndef _MEMUTILS_H_
#define _MEMUTILS_H_

#include <stdlib.h>

#ifdef CNCOCR_TG
static inline void *malloc(size_t i) {
  // dummy function
  return NULL;
}

static inline void *calloc(size_t s, size_t i) {
  // dummy function
  return NULL;
}

static inline void *realloc(void *ptr, size_t i) {
  // dummy function
  return NULL;
}

static inline void free(void *i) {
  // dummy function
}
#endif

#define freeMe(s,element) {if(s->element) comdFree(s->element);  s->element = NULL;}

static void* comdMalloc(size_t iSize)
{
   return malloc(iSize);
}

static void* comdCalloc(size_t num, size_t iSize)
{
   return calloc(num, iSize);
}

static void* comdRealloc(void* ptr, size_t iSize)
{
   return realloc(ptr, iSize);
}

static void comdFree(void *ptr)
{
   free(ptr);
}
#endif

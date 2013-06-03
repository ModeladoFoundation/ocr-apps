#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "MEM.h"

#if CILK
extern "C" {
#endif

#ifdef FSIM

void *DRAM_MALLOC(size_t count, size_t sizeof_type) { 
  return malloc(count*sizeof_type);
}

void  DRAM_FREE(void *ptr) {
  free(ptr);
}

#endif

#if CILK
} // extern "C"
#endif

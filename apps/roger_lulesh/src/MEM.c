#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "MEM.h"

#if CILK
extern "C" {
#endif

#if defined(FSIM) || defined(OCR)

void *DRAM_MALLOC(size_t count, size_t sizeof_type) { 
  return malloc(count*sizeof_type);
}

void  DRAM_FREE(void *ptr) {
  free(ptr);
}

#endif // FSIM or OCR

#if CILK
} // extern "C"
#endif

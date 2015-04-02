#ifndef __XSTG_MALLOC_H__
#define __XSTG_MALLOC_H__

#include <stdint.h>
#include <stddef.h>
#include <_ansi.h>
#include <reent.h>

// VERY IMPORTANT: The maximum size of the SP supported
// is *very* dependent on a few parameters:
//  - xstg_size_t: this encodes the "addresses" and size of
//      each block. It must be able to contain the entire range
//      of addresses
//  - this is partly compensated by ELEMENT_SIZE_LOG2/BYTES which
//      contains the number of bytes per "unit" of memory. This
//      increases granularity to be able to address more with fewer
//      bits. Therefore, xstg_size_t needs only to encode
//      the total size in ELEMENT_SIZE_BYTES unit.
//  - FL_MAX_LOG2: The total size of memory is (1<<FL_MAX_LOG2) * ELEMENT_SIZE_BYTES


// Type used to store the "size" of a block
// The minimum size of a block will be 4*sizeof(xstg_size_t)
// padded to be a multiple of 2^ALIGN_LOG2 bytes
// NOTE: When changing this:
//  - make sure the fls routine in xstg_malloc.c exists for the size
//  - change FL_MAX_LOG2 if required
typedef uint32_t xstg_size_t;
#define SIZE_XSTG_SIZE 32

#define RARG struct _reent *reent_ptr,

// Taken from D. Lea's mallocr.c in newlib. These macros are used
// to move memory and zero it out. Note that we use uint32_t because
// the allocated size is a multiple of 4 bytes (NOT 8). Change if this
// changes.
// TODO: Maybe optimize this
#define MALLOC_ZERO(charp, nbytes)                                            \
do {                                                                          \
  uint32_t* mzp = (uint32_t*)(charp);                                         \
  long mctmp = (nbytes)/sizeof(uint32_t), mcn;                                \
  if (mctmp < 8) mcn = 0; else { mcn = (mctmp-1)/8; mctmp %= 8; }             \
  switch (mctmp) {                                                            \
    case 0: for(;;) { *mzp++ = 0;                                             \
    case 7:           *mzp++ = 0;                                             \
    case 6:           *mzp++ = 0;                                             \
    case 5:           *mzp++ = 0;                                             \
    case 4:           *mzp++ = 0;                                             \
    case 3:           *mzp++ = 0;                                             \
    case 2:           *mzp++ = 0;                                             \
    case 1:           *mzp++ = 0; if(mcn <= 0) break; mcn--; }                \
  }                                                                           \
} while(0)

#define MALLOC_COPY(dest,src,nbytes)                                          \
do {                                                                          \
  uint32_t* mcsrc = (uint32_t*) src;                                          \
  uint32_t* mcdst = (uint32_t*) dest;                                         \
  long mctmp = (nbytes)/sizeof(uint32_t), mcn;                                \
  if (mctmp < 8) mcn = 0; else { mcn = (mctmp-1)/8; mctmp %= 8; }             \
  switch (mctmp) {                                                            \
    case 0: for(;;) { *mcdst++ = *mcsrc++;                                    \
    case 7:           *mcdst++ = *mcsrc++;                                    \
    case 6:           *mcdst++ = *mcsrc++;                                    \
    case 5:           *mcdst++ = *mcsrc++;                                    \
    case 4:           *mcdst++ = *mcsrc++;                                    \
    case 3:           *mcdst++ = *mcsrc++;                                    \
    case 2:           *mcdst++ = *mcsrc++;                                    \
    case 1:           *mcdst++ = *mcsrc++; if(mcn <= 0) break; mcn--; }       \
  }                                                                           \
} while(0)

void* tlsf_malloc(RARG size_t size);
void tlsf_free(RARG void* ptr);
void* tlsf_realloc(RARG void* ptr, size_t size);

#ifdef AFL
unsigned int initializeMemoryManager(void* location, xstg_size_t realSize);
#endif

// Debug functions
typedef void (*walkerAction)(void*, void*);

void walkHeap(walkerAction action, void* extra);

/* Returns the number of errors in the heap structure
 * and the total amount of free space left
 */
int checkHeap(unsigned int *freeRemaining);

#endif /* __XSTG_MALLOC_H__ */


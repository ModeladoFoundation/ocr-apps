/*
 * This library was inspired by work published by Miguel
 * Masmano: http://rtportal.upv.es/rtmalloc/allocators/tlsf/index.shtml.
 * However, no code from that website (which is under GPL/LGPL) was
 * used or looked at. The papers published were used however. In particular,
 * see: http://rtportal.upv.es/rtdsa/files/rtportal.upv.es.rtdsa/documents/tlsf_desc.pdf.
 *
 * The starting point for this code was instead an alternate version
 * placed in the public domain written by Matthew Conte (http://tlsf.baisoku.org)
 *
 * This code was entirely written by Romain Cledat at Intel (romain.e.cledat@intel.com)
 */

#include "xstg_malloc.h"
#include <errno.h>
#include <stdio.h>

// Assertions
#ifdef DEBUG_XSTG_MEM
#include "assert.h"
#define ASSERT(a) assert(a)
#else
#define ASSERT(a)
#endif

#define xstg_static_assert(e) extern char (*xstg_static_assert(void))[sizeof(char[1-2*!(e)])]

#define BOOL uint8_t
#define FALSE 0
#define TRUE 1

#define _NULL 0

/*
 * Configuration constants
 */

// Number of bytes the "size" of a block
// is a multiple of. A size of 1 means 1 word (4 bytes, 32 bits)
// This is used to "extend" the range possible by trading some
// granularity in allocation. The maximum size allocatable/addressable
// will be ELEMENT_SIZE_BYTES*2^{sizeof(xstg_size_t)*8} bytes
// NOTE: Must be at least 2 bytes
#define ELEMENT_SIZE_LOG2 2
#define ELEMENT_SIZE_BYTES 4

// Alignment of returned block
#define ALIGN_LOG2  3   // 8 byte alignment
#define ALIGN_BYTES 8

// Alignment of full block (including headers)
// Can be ALIGN_LOG2 or half of that if 2 fields
// of xstg_size_t will "pad" the full block to start
// on an ALIGN_BYTES alignment
#define ALIGN_BLOCK_LOG2 2 // 4 byte
#define ALIGN_BLOCK_BYTES 4

// Number of subdivisions in the second-level list
// 2^SL_COUNT_LOG2 must be < to sizeof(xstg_size_t)*8
#define SL_COUNT_LOG2 3

// Support allocations of size up to (1 << FL_MAX_LOG2) * ELEMENT_SIZE_BYTES
// Note that bitmaps are of type xstg_size_t so everything must fit in that...
// For 64K memory: 14 (if ELEMENT_SIZE_LOG2 == 2)
// For 1024K memory: 18 (if ELEMENT_SIZE_LOG2 == 2)
#define FL_MAX_LOG2 18

// Some computed values:
//  - SL_COUNT: Number of buckets in each SL list
//  - FL_COUNT_SHIFT: For the first SL_COUNT, we do not need to maintain
//          first level lists (since they all go in the "0th" one.
//  - FL_COUNT: Number of buckets in FL (so number of SL bitmaps)
//  - ZERO_LIST_SIZE: Size under which we are in SL 0
enum computed_vals {
    SL_COUNT                = (1 << SL_COUNT_LOG2),
    FL_COUNT_SHIFT  = (SL_COUNT_LOG2),
    FL_COUNT                = (FL_MAX_LOG2 - FL_COUNT_SHIFT + 1),
    ZERO_LIST_SIZE  = (1 << FL_COUNT_SHIFT),
};

/*
 * Header of a block (allocated or free) of memory. Note that not all
 * fields are valid at all times:
 *
 * Free block:
 * xstg_size_t prevFreeBlock; <-- can NEVER be 0 or 1 (would indicate
 *  an occupied block). This should be fine as that would mean that
 *  the address of the previous block is 0x4 (in absolute) which is
 *  never going to happen...
 * xstg_size_t sizeBlock;
 * xstg_size_t nextFreeBlock; <-- This field aligned 2^ALIGN_LOG2 bytes
 * <SPACE>
 * xstg_size_t sizeBlock; (used by next occupied block); must be at the very end
 *
 * Used block:
 * xstg_size_t prevFreeBlock <- not used as that:
 *  - bit 0 indicates whether the previous block is free (1) or not (0)
 *  - the rest MUST be 0 to indicate an occupied block.
 * xstg_size_t sizeBlock
 *
 * Note that prevFreeBlock and nextFreeBlock are actually pointers
 * but we only consider the lower xstg_size_t bits. The rest are zero
 * since the total size of the memory is small.
 * */
typedef struct header_t {
    xstg_size_t prevFreeBlock; // __attribute__ (( aligned (ALIGN_BLOCK_BYTES) ));
    xstg_size_t sizeBlock;

    xstg_size_t nextFreeBlock; // __attribute__ (( aligned (ALIGN_BYTES) ));
} header_t;

// Union to hold both pointer to header and xstg_size_t which
// are used inter-changeably
typedef union header_addr_t {
    header_t *address;
    xstg_size_t value;
} header_addr_t;

// Some assertions to make sure things are OK
xstg_static_assert(2*sizeof(xstg_size_t) >= ELEMENT_SIZE_BYTES);
xstg_static_assert(offsetof(header_t, nextFreeBlock) % ELEMENT_SIZE_BYTES == 0);
xstg_static_assert(ZERO_LIST_SIZE == SL_COUNT);
xstg_static_assert(sizeof(char) == 1);

/* The pool (containing the bitmaps and the lists of
 * free blocks.
 */
typedef struct pool_t {
    xstg_size_t flAvailOrNot;                        // bitmap that indicates the presence (1) or absence (0) of free blocks
                                                                            // in blocks[i][*]
    xstg_size_t slAvailOrNot[FL_COUNT];  // Second level bitmaps

    xstg_size_t blocks[FL_COUNT][SL_COUNT];  // Pointers to starts of free-lists
    header_t nullBlock __attribute__ ((aligned (ELEMENT_SIZE_BYTES) )); // This will be used to mark "null" blocks
                                                                            // Use this so that we have a valid address for prev/next free blocks
                                                                            // in particular since prev can never be NULL since that
                                                                            // would indicate a busy block

    header_t mainBlock __attribute__ ((aligned (ALIGN_BLOCK_BYTES) ));  // Main memory block
} pool_t;

static pool_t *globalPool = _NULL;
static BOOL didInit = FALSE;

static const xstg_size_t GoffsetToStart = offsetof(header_t, nextFreeBlock);
static const xstg_size_t GusedBlockOverhead = GoffsetToStart >> ELEMENT_SIZE_LOG2;

static const xstg_size_t GminBlockRealSize = (sizeof(header_t)+sizeof(xstg_size_t) + ELEMENT_SIZE_BYTES - 1) >> ELEMENT_SIZE_LOG2;
static const xstg_size_t GmaxBlockRealSize = (xstg_size_t)((1<<FL_MAX_LOG2) - 1);

/* If we are running on x86, we can definitely
 * make no assumption on being given memory that
 * starts at 0x0 as is assumed here to shorten
 * the fields used to store addresses. We therefore
 * use a GbaseOfMemory to compute proper offsets
 */
typedef uint64_t ptrToInt_t;

#ifdef AFL
static header_t* GbaseOfMemory = _NULL;
#define GET_ADDRESS(header_addr) ((header_t*)((ptrToInt_t)GbaseOfMemory + ((ptrToInt_t)(header_addr.value) << ELEMENT_SIZE_LOG2)))
#define SET_VALUE(header_addr, addr) do { (header_addr).value = ((ptrToInt_t)(addr) - (ptrToInt_t)GbaseOfMemory) >> ELEMENT_SIZE_LOG2; } while(0)
#define SET_NULL(header_addr) SET_VALUE(header_addr, GbaseOfMemory)
#define IS_NULL(addr) ((addr) == GbaseOfMemory)
#else
#define GET_ADDRESS(header_addr) ((header_t*)((ptrToInt_t)(header_addr.value) << ELEMENT_SIZE_LOG2))
#define SET_VALUE(header_addr, addr) do { (header_addr).value = (ptrToInt_t)(addr) >> ELEMENT_SIZE_LOG2; } while(0)
#define SET_NULL(header_addr) SET_VALUE(header_addr, 0)
#define IS_NULL(addr) ((addr) == 0)
#endif

/* Bit functions:
 * fls: Find last set: position of the MSB set to 1
 * ffs: find first set: position of the LSB set to 1
 */
// IMPORTANT: CHANGE THESE FUNCTIONS IF xstg_size_t CHANGES
#if SIZE_XSTG_SIZE == 16
int fls(xstg_size_t val) {
  int bit = 15;

  if(!(val & 0xff00)) { val <<= 8; bit -= 8; }
  if(!(val & 0xf000)) { val <<= 4; bit -= 4; }
  if(!(val & 0xc000)) { val <<= 2; bit -= 2; }
  if(!(val & 0x8000)) { val <<= 1; bit -= 1; }

  return bit;
}
#elif SIZE_XSTG_SIZE == 32
int fls(xstg_size_t val) {
  int bit = 31;

  if(!(val & 0xffff0000)) { val <<= 16; bit -= 16; }
  if(!(val & 0xff000000)) { val <<= 8; bit -= 8; }
  if(!(val & 0xf0000000)) { val <<= 4; bit -= 4; }
  if(!(val & 0xc0000000)) { val <<= 2; bit -= 2; }
  if(!(val & 0x80000000)) { val <<= 1; bit -= 1; }

  return bit;
}
#else
#error "Unknown size for xstg_size_t"
#endif


int ffs(xstg_size_t val) {
  return fls(val & (~val + 1));
}

/* Utility functions for blocks. All these functions that
 * a "me" block pointer as their first argument
 */

void* addressForBlock(header_t* me) {
    return (void*)((char*)me + GoffsetToStart);
}

header_addr_t blockForAddress(void* me) {
    header_addr_t res;

    SET_VALUE(res, ((char*)me - GoffsetToStart));
    return res;
}

static BOOL isBlockFree(const header_t* me) {
    return (me->prevFreeBlock & ~0x1); // Checks if there is a non-null bit (ie: a valid pointer)
                                                                             // past the first bit
}

static BOOL isPrevBlockFree(const header_t* me) {
    if(isBlockFree(me)) {
        return FALSE;
    } else {
        return (me->prevFreeBlock & 0x1);
    }
}

static header_addr_t getPrevBlock(const header_t* me) {
    header_addr_t res;
    ASSERT(isPrevBlockFree(me));
    xstg_size_t offset = *((xstg_size_t*)(me) - 1) + GusedBlockOverhead; // Gets sizeBlock2 in
                                                                                                    // previous block (which is free)

    SET_VALUE(res, ((const char*)me - (offset << ELEMENT_SIZE_LOG2)));
    return res;
}

static header_addr_t getNextBlock(const header_t* me) {
    header_addr_t res;
    xstg_size_t offset = me->sizeBlock + GusedBlockOverhead;

    SET_VALUE(res, ((const char*)me + (offset << ELEMENT_SIZE_LOG2)));
    return res;
}

static header_addr_t getPrevFreeBlock(const header_t* me) {
    header_addr_t res;
    ASSERT(isBlockFree(me) && "getPrevFreeBlock not called on free block");

    res.value = me->prevFreeBlock;
    return res;
}

static header_addr_t getNextFreeBlock(const header_t* me) {
    header_addr_t res;
    ASSERT(isBlockFree(me) && "getNextFreeBlock not called on free block");

    res.value = me->nextFreeBlock;
    return res;
}

static void linkFreeBlocks(header_addr_t first, header_addr_t second) {
    ASSERT(isBlockFree(GET_ADDRESS(first)) && "linkFreeBlocks arg1 not free");
    ASSERT(isBlockFree(GET_ADDRESS(second)) && "linkFreeBlocks arg2 not free");
    ASSERT(getNextBlock(GET_ADDRESS(first)).value != second.value && "Consecutive blocks cannot be free");
    ASSERT(((ptrToInt_t)GET_ADDRESS(first) & ((1<<ELEMENT_SIZE_LOG2) - 1)) == 0 && "linkFreeBlocks arg1 misaligned");
    ASSERT(((ptrToInt_t)GET_ADDRESS(second) & ((1<<ELEMENT_SIZE_LOG2) - 1)) == 0 && "linkFreeBlocks arg2 misaligned");

    GET_ADDRESS(first)->nextFreeBlock = second.value;
    GET_ADDRESS(second)->prevFreeBlock = first.value;
}

// The previous block of a free block is *ALWAYS* used
// (otherwise, it would be coalesced)
static void markPrevBlockUsed(header_t* me) {
    ASSERT(!isBlockFree(me) && "markPrevBlockUsed can only be called on used block");
    me->prevFreeBlock = 0;
}

static void markPrevBlockFree(header_t* me) {
    ASSERT(!isBlockFree(me) && "markPrevBlockFree can only be called on used block");
    me->prevFreeBlock = 1;
}

static void markBlockUsed(header_t* me) {
    me->prevFreeBlock = (xstg_size_t)0;

    header_addr_t nextBlock = getNextBlock(me);
    if(!isBlockFree(GET_ADDRESS(nextBlock)))
        markPrevBlockUsed(GET_ADDRESS(nextBlock));
}

static void markBlockFree(header_t* me) {
    // To mark a block as free, we need to duplicate it's size
    // at the end of the block so that the next block can find the beginning
    // of the free block.
    xstg_size_t *locationToWrite = (xstg_size_t*)((char*)me +
        ((me->sizeBlock + GusedBlockOverhead) << ELEMENT_SIZE_LOG2) - sizeof(xstg_size_t));
    *locationToWrite = me->sizeBlock;

    me->prevFreeBlock = (xstg_size_t)0xBEEF; // Some value that is not 0 or 1 for now
                                            // will be updated when me
                                            // free block is actually inserted

    header_t *nextBlockPtr = GET_ADDRESS(getNextBlock(me));
    if(!isBlockFree(nextBlockPtr))  // Usually, no two free blocks follow each other but it may happen temporarily
        markPrevBlockFree(nextBlockPtr);
}


/*
 * Allocation helpers (size and alignement constraints)
 */
static xstg_size_t getRealSizeOfRequest(size_t size) {
    if(size >> ELEMENT_SIZE_LOG2 > GmaxBlockRealSize)
        return 0; // Too big to allocate

    if(size < ((GminBlockRealSize - GusedBlockOverhead) << ELEMENT_SIZE_LOG2)) {
        return GminBlockRealSize - GusedBlockOverhead;
    }

    // We want this to be aligned so that the total w/ the header is ALIGN_BYTES but
    // since the header may be smaller than ALIGN_BYTES, we do this weird computation
    // to take it into account
    size = ((size + ALIGN_BYTES - GoffsetToStart - 1) & ~(ALIGN_BYTES - 1))
        + GoffsetToStart;

    ASSERT(size % ELEMENT_SIZE_BYTES == 0);

    size >>= ELEMENT_SIZE_LOG2;
    ASSERT(size >= (GminBlockRealSize - GusedBlockOverhead));

    return size;
}

/* Two-level function to determine indices. This is pretty much
 * taken straight from the specs
 */
static void mappingInsert(xstg_size_t realSize, int* flIndex, int* slIndex) {
    int tf, ts;
    if(realSize < ZERO_LIST_SIZE) {
        tf = 0;
        ts = realSize;
    } else {
        tf = fls(realSize);
        ts = (realSize >> (tf - SL_COUNT_LOG2)) - (SL_COUNT);
        tf -= (FL_COUNT_SHIFT - 1);
    }
    *flIndex = tf;
    *slIndex = ts;
}

/* For allocations, we want to round-up to the next block size
 * so that we are sure that any block will work so we can
 * pick it in constant time.
 */
static void mappingSearch(xstg_size_t realSize, int* flIndex, int* slIndex) {
    if(realSize >= ZERO_LIST_SIZE) {
        // If not, we don't need to do anything since the mappingInsert
        // will return the correct thing
        realSize += (1 << (fls(realSize) - SL_COUNT_LOG2)) - 1;
    }
    mappingInsert(realSize, flIndex, slIndex);
}

/* Search for a suitable free block:
 *  - first search for a block in the fl/sl indicated.
 *  - if not found, search for higher sl (bigger) with same fl
 *  - if not found, search for higher fl and sl in that
 *
 *  Returns the header of a free block as well as flIndex and slIndex
 *  that block was taken from.
 */
static header_addr_t findFreeBlockForRealSize(pool_t* poolToUse, xstg_size_t realSize,
    int* flIndex, int* slIndex) {

    int tf, ts;
    header_addr_t res;
    xstg_size_t flBitMap, slBitMap;

    mappingSearch(realSize, flIndex, slIndex);
    tf = *flIndex;
    ts = *slIndex;

    if(tf >= FL_COUNT) {
        SET_NULL(res); // This means the alloc size is too large
        return res;
    }

    slBitMap = poolToUse->slAvailOrNot[tf] & (~0 << ts); // This takes all SL bins bigger or equal to ts
    if(slBitMap == 0) {
        // We don't have any non-zero block here so we look at the flAvailOrNot map
        flBitMap = poolToUse->flAvailOrNot & (~0 << (tf + 1));
        if(flBitMap == 0) {
            SET_NULL(res);
            return res;
        }

        // Look for the first bit that is a one
        tf = ffs(flBitMap);
        ASSERT(tf > *flIndex);
        *flIndex = tf;

        // Now we get the slBitMap. There is definitely a 1 in there since there is a 1 in tf
        slBitMap = poolToUse->slAvailOrNot[tf];
    }

    ASSERT(slBitMap != 0);

    ts = ffs(slBitMap);
    *slIndex = ts;

    res.value = poolToUse->blocks[tf][ts];
    return res;
}


/* Helpers to manipulate the free lists */
static void removeFreeBlock(pool_t* poolToUse, header_addr_t freeBlock) {
    header_t *freeBlockPtr = GET_ADDRESS(freeBlock);
    ASSERT(isBlockFree(freeBlockPtr));

    int flIndex, slIndex;
    mappingInsert(freeBlockPtr->sizeBlock, &flIndex, &slIndex);

    // First remove it from the chain of free blocks
    header_addr_t prevFreeBlock = getPrevFreeBlock(freeBlockPtr);
    header_addr_t nextFreeBlock = getNextFreeBlock(freeBlockPtr);

    ASSERT(prevFreeBlock.value !=0 && isBlockFree(GET_ADDRESS(prevFreeBlock)));
    ASSERT(nextFreeBlock.value !=0 && isBlockFree(GET_ADDRESS(nextFreeBlock)));

    linkFreeBlocks(prevFreeBlock, nextFreeBlock);

    // Check if we need to change the head of the blocks list
    if(poolToUse->blocks[flIndex][slIndex] == freeBlock.value) {
        poolToUse->blocks[flIndex][slIndex] = nextFreeBlock.value;

        // If the new head is nullBlock then we clear the bitmaps to
        // indicate that we have no more blocks available here
        if(GET_ADDRESS(nextFreeBlock) == &poolToUse->nullBlock) {
            poolToUse->slAvailOrNot[flIndex] &= ~(1<<slIndex); // Clear me bit

            // Check if slAvailOrNot is now 0
            if(poolToUse->slAvailOrNot[flIndex] == 0) {
                poolToUse->flAvailOrNot &= ~(1<<flIndex);
            }
        }
    }
}

void addFreeBlock(pool_t* poolToUse, header_addr_t freeBlock) {
    header_t *freeBlockPtr = GET_ADDRESS(freeBlock);
    int flIndex, slIndex;
    mappingInsert(freeBlockPtr->sizeBlock, &flIndex, &slIndex);

    header_addr_t currentHead, temp;

    currentHead.value = poolToUse->blocks[flIndex][slIndex];

    ASSERT(GET_ADDRESS(currentHead));
    ASSERT(freeBlockPtr);
    ASSERT(freeBlockPtr != &(poolToUse->nullBlock));

    // Set the links properly
    SET_VALUE(temp, &(poolToUse->nullBlock));

    freeBlockPtr->prevFreeBlock = temp.value;
    linkFreeBlocks(freeBlock, currentHead);

    freeBlockPtr->prevFreeBlock = temp.value;
    freeBlockPtr->nextFreeBlock = currentHead.value;
    GET_ADDRESS(currentHead)->prevFreeBlock = freeBlock.value; // Does not matter is the head was the
                                                          // nullBlock, that value is ignored anyways

    // Update the bitmaps
    poolToUse->blocks[flIndex][slIndex] = freeBlock.value;
    poolToUse->flAvailOrNot |= (1<<flIndex);
    poolToUse->slAvailOrNot[flIndex] |= (1<<slIndex);
}

/* Split origBlock so that origBlock is realSize long and
 * returns a pointer to the remaining block which is free
 */
static header_addr_t splitBlock(header_addr_t origBlock, xstg_size_t realSize) {
    header_addr_t remainingBlock;

    header_t *origBlockPtr = GET_ADDRESS(origBlock);
    xstg_size_t origBlockSize = origBlockPtr->sizeBlock;
    ASSERT(origBlockSize > realSize + GminBlockRealSize);

    xstg_size_t remainingSize = origBlockSize - realSize - GusedBlockOverhead;

    SET_VALUE(remainingBlock, ((char*)origBlockPtr + GoffsetToStart + (realSize << ELEMENT_SIZE_LOG2)));

    // Take care of the remaining block (set its size and mark it as free)
    GET_ADDRESS(remainingBlock)->sizeBlock = remainingSize;
    markBlockFree(GET_ADDRESS(remainingBlock));

    // Set the size of the original block properly
    origBlockPtr->sizeBlock = realSize;

    return remainingBlock;
}

/* Absorbs nextBlock into freeBlock making one much
 * larger freeBlock. nextBlock must be physically next
 * to freeBlock
 */
static void absorbNext(header_addr_t freeBlock, header_addr_t nextBlock) {
    header_t *freeBlockPtr = GET_ADDRESS(freeBlock), *nextBlockPtr = GET_ADDRESS(nextBlock);
    ASSERT(isBlockFree(freeBlockPtr));
    ASSERT(isBlockFree(nextBlockPtr));
    ASSERT(getNextBlock(freeBlockPtr).value == nextBlock.value);

    freeBlockPtr->sizeBlock += nextBlockPtr->sizeBlock + GusedBlockOverhead;
    markBlockFree(freeBlockPtr); // will update the other field
}

/* Merge with the previous block if it is free (toBeFreedBlock must
 * be used so isPrevBlockFree can return true)
 * Returns the resulting free block (either toBeFreedBlock marked as free
 * or the larger block)
 */
static header_addr_t mergePrevious(pool_t *poolToUse, header_addr_t toBeFreedBlock) {
    header_t *toBeFreedBlockPtr = GET_ADDRESS(toBeFreedBlock);
    ASSERT(!isBlockFree(toBeFreedBlockPtr));
    if(isPrevBlockFree(toBeFreedBlockPtr)) {
        // Get the previous block
        header_addr_t prevBlock = getPrevBlock(toBeFreedBlockPtr);
        // Remove it from the free-lists (since it will be made bigger)
        removeFreeBlock(poolToUse, prevBlock);

        // Now we mark the toBeFreedBlock as free and merge it with the other one
        markBlockFree(toBeFreedBlockPtr);
        absorbNext(prevBlock, toBeFreedBlock);
        toBeFreedBlock.value = prevBlock.value;
    } else {
        markBlockFree(toBeFreedBlockPtr);
    }

    ASSERT(isBlockFree(toBeFreedBlockPtr));
    return toBeFreedBlock;
}

/* Merges a block with its block if that one is free as well. The input
 * block must be free to start with
 */
static header_addr_t mergeNext(pool_t *poolToUse, header_addr_t freeBlock) {
    ASSERT(isBlockFree(GET_ADDRESS(freeBlock)));
    header_addr_t nextBlock = getNextBlock(GET_ADDRESS(freeBlock));
    if(isBlockFree(GET_ADDRESS(nextBlock))) {
        removeFreeBlock(poolToUse, nextBlock);
        absorbNext(freeBlock, nextBlock);
    }

    return freeBlock;
}

/* Pool construction */
static void initializePool(pool_t* poolToUse, xstg_size_t poolRealSize) {

    header_addr_t mainBlockAddr, nullBlockAddr;

    SET_VALUE(nullBlockAddr, &(poolToUse->nullBlock));
    SET_VALUE(mainBlockAddr, &(poolToUse->mainBlock));

    // Initialize the bitmaps to 0
    poolToUse->flAvailOrNot = 0;
    for(int i=0; i<FL_COUNT; ++i) {
        poolToUse->slAvailOrNot[i] = 0;
        for(int j=0; j<SL_COUNT; ++j) {
            poolToUse->blocks[i][j] = nullBlockAddr.value;
        }
    }

    // Initialize the main block properly
    // We do something a little special in the main block: we take a little bit
    // of it at the end to create a zero-sized block that is used
    // to act as a sentinel. We therefore need GusedBlockOverhead space in the
    // end (the 2* is because we also account for our own overhead)
    poolToUse->mainBlock.sizeBlock = poolRealSize - 2*GusedBlockOverhead;

    poolToUse->mainBlock.prevFreeBlock = poolToUse->mainBlock.nextFreeBlock
        = nullBlockAddr.value;

    markBlockFree(&(poolToUse->mainBlock));

    // Add the sentinel
    header_t *sentinel = (header_t*)((char*)&(poolToUse->mainBlock) + ((poolRealSize - GusedBlockOverhead) << ELEMENT_SIZE_LOG2));
    markBlockUsed(sentinel);
    sentinel->sizeBlock = 0;
    markPrevBlockFree(sentinel);

    // Initialize the nullBlock properly
    poolToUse->nullBlock.sizeBlock = 0;
    poolToUse->nullBlock.prevFreeBlock = poolToUse->nullBlock.nextFreeBlock
        = nullBlockAddr.value;

    // Add the big free block
    addFreeBlock(poolToUse, mainBlockAddr);
}

#ifndef AFL
static
#endif
unsigned int initializeMemoryManager(void* location, xstg_size_t realSize) {
    /* The memory will be layed out as follows:
     *  - at location: the pool structure is used
     *  - the first free block starts right after that (aligned)
     */
#ifdef AFL
    GbaseOfMemory = (header_t*)location;
#endif

    globalPool = (pool_t*)location;

    size_t poolHeaderSize = sizeof(pool_t) + ALIGN_BLOCK_BYTES - 1;
    poolHeaderSize = (poolHeaderSize & ~(ALIGN_BLOCK_BYTES - 1));
    // Now we have a poolHeaderSize that is big enough to contain the pool
    // and right after it, we can start the big block.

    size_t poolRealSize = realSize - ((poolHeaderSize + ELEMENT_SIZE_BYTES - 1) >> ELEMENT_SIZE_LOG2);

    if(poolRealSize < (size_t)GminBlockRealSize || poolRealSize > (size_t)GmaxBlockRealSize) {
        return 0;
    }

    initializePool(globalPool, poolRealSize);
    didInit = TRUE;
    return globalPool->mainBlock.sizeBlock << ELEMENT_SIZE_LOG2;
}

static unsigned int initializeFromSP() {
    // This is an allocation routine for scratchpads
    extern void _end;   // Symbol defined by the linker. This is
                        // where the heap should start
    void* locationToStart = &_end;
    size_t endLocation = 0;
    // This is not very pretty but has to do with the fact
    // that I don't know if inline asm handles rfbase well
    // FIXME
    __asm__(
        "bitop1 r0, r64, 0x0, OR, 64;\n\t"
        "bitop1 %0, r48, 0x0, OR, 64;\n\t"
        : "=r" (endLocation) /* outputs */
        : /* inputs */
        : "r48" /* clobber */
    );

    // We are going to allow the heap to only to the bottom of the
    // reserved stack to prevent any stack/heap overflowgrow all the way
    // to the top so programmers can happily shoot themselves in the
    // foot (and all over for that matter)
    xstg_size_t realSizeForPool = (endLocation - (size_t)locationToStart) >> ELEMENT_SIZE_LOG2;

    return initializeMemoryManager(locationToStart, realSizeForPool);
}

void* tlsf_malloc(RARG size_t size) {
#ifndef AFL
    if(!didInit) {
        if(!initializeFromSP()) {
            errno = ENOMEM;
            return _NULL;
        }
    }
#endif
    ASSERT(didInit);
    xstg_size_t allocSize, returnedSize;
    int flIndex, slIndex;
    header_addr_t freeBlock, remainingBlock;

    allocSize = getRealSizeOfRequest(size);

    freeBlock = findFreeBlockForRealSize(globalPool, allocSize, &flIndex, &slIndex);

    header_t *freeBlockPtr = GET_ADDRESS(freeBlock);

    if(IS_NULL(freeBlockPtr)) {
        errno = ENOMEM;
        return _NULL;
    }

    removeFreeBlock(globalPool, freeBlock);
    returnedSize = freeBlockPtr->sizeBlock;

    if(returnedSize > allocSize + GminBlockRealSize) {
        remainingBlock = splitBlock(freeBlock, allocSize);
        addFreeBlock(globalPool, remainingBlock);
    }

    markBlockUsed(freeBlockPtr);

    return addressForBlock(freeBlockPtr);
}

void tlsf_free(RARG void* ptr) {
    if(!didInit)
        return; // We don't have a memory system to speak of
    header_addr_t b;

    b = blockForAddress(ptr);

    b = mergePrevious(globalPool, b);
    b = mergeNext(globalPool, b);
    addFreeBlock(globalPool, b);
}

void* tlsf_realloc(RARG void* ptr, size_t size) {
    // First deal with corner cases:
    //  - non-zero size with null pointer is like malloc
    //  - zero size with ptr like free
    if(ptr && size == 0) {
        tlsf_free(reent_ptr, ptr);
        return _NULL;
    }
    if(!ptr) {
        return tlsf_malloc(reent_ptr, size);
    }

    if(!didInit)
        return _NULL;

    void* result = _NULL;
    header_addr_t curBlock, nextBlock;
    header_t *blockAddr, *nextBlockAddr;
    xstg_size_t realReqSize, realAvailSize;

    curBlock = blockForAddress(ptr);
    blockAddr = GET_ADDRESS(curBlock);
    nextBlock = getNextBlock(blockAddr);
    nextBlockAddr = GET_ADDRESS(nextBlock);

    realAvailSize = blockAddr->sizeBlock + nextBlockAddr->sizeBlock + GusedBlockOverhead;
    realReqSize = getRealSizeOfRequest(size);

    if(realReqSize > blockAddr->sizeBlock || (!isBlockFree(nextBlockAddr) || realReqSize > realAvailSize)) {
        // We need to reallocate and copy
        result = tlsf_malloc(reent_ptr, size);
        if(result) {
            size_t sizeToCopy = blockAddr->sizeBlock<realReqSize?blockAddr->sizeBlock:realReqSize;
            sizeToCopy <<= ELEMENT_SIZE_LOG2;
            MALLOC_COPY(result, ptr, sizeToCopy);
            tlsf_free(reent_ptr, ptr); // Free the original block
        }
    } else {
        if(realReqSize > blockAddr->sizeBlock) {
            // This means we need to extend to the other block
            removeFreeBlock(globalPool, nextBlock);
            blockAddr->sizeBlock = realAvailSize;
            markBlockUsed(blockAddr);
        } else {
            realAvailSize = blockAddr->sizeBlock;
        }
        // We can trim to just the size used to create a new
        // free block and reduce internal fragmentation
        if(realAvailSize > realReqSize + GminBlockRealSize) {
            header_addr_t remainingBlock = splitBlock(curBlock, realReqSize);
            addFreeBlock(globalPool, remainingBlock);
        }
        result = ptr;
    }
    return result;
}

/* Some debug functions to see what is going on */


static void printBlock(void *block_, void* extra) {
    header_t *curBlock = (header_t*)block_;
    int *count = (int*)extra;
    printf("\tBlock %d starts at 0x%p (user: 0x%p) of size %d (user: %d) %s\n",
        *count, curBlock, (char*)curBlock + (GusedBlockOverhead << ELEMENT_SIZE_LOG2),
        curBlock->sizeBlock, (curBlock->sizeBlock << ELEMENT_SIZE_LOG2),
        isBlockFree(curBlock)?"free":"used");
    *count += 1;
}

typedef struct flagVerifier_t {
    int countConsecutiveFrees;
    BOOL isPrevFree;
    int countErrors;
} flagVerifier_t;

static void verifyFlags(void *block_, void* extra) {
    header_t *curBlock = (header_t*)block_;
    flagVerifier_t* verif = (flagVerifier_t*)extra;

    if(isPrevBlockFree(curBlock) != verif->isPrevFree) {
        verif->countErrors += 1;
        printf("Mismatch in free flag for block 0x%p\n", curBlock);
    }
    if(isBlockFree(curBlock)) {
        verif->countConsecutiveFrees += 1;
        if(verif->countConsecutiveFrees > 1) {
            printf("Blocks did not coalesce (count of %d at 0x%p)\n",
                verif->countConsecutiveFrees, curBlock);
            verif->countErrors += 1;
        }
        verif->isPrevFree = TRUE;
    } else {
        verif->countConsecutiveFrees = 0;
        verif->isPrevFree = FALSE;
    }
}

void walkHeap(walkerAction action, void* extra) {
    if(action == _NULL) {
        action = printBlock;
    }
    header_t *curBlock = &(globalPool->mainBlock);

    while(curBlock && !(curBlock->sizeBlock == 0)) {
        action(curBlock, extra);
        curBlock = GET_ADDRESS(getNextBlock(curBlock));
    }
}

/* Will check to make sure the heap is properly maintained
 *
 * Return the number of errors
 */
int checkHeap(unsigned int *freeRemaining) {
    int errCount = 0;
    unsigned int countFree = 0;
    flagVerifier_t verif = {0, FALSE, 0};

    walkHeap(verifyFlags, &verif);
    if(verif.countErrors) {
        errCount += verif.countErrors;
        printf("Mismatch in free flags or coalescing\n");
    }

    // Check the bitmaps
    for(int i=0; i < FL_COUNT; ++i) {
        BOOL hasFlAvail = globalPool->flAvailOrNot & (1 << i);
        for(int j=0; j < SL_COUNT; ++j) {
            BOOL hasSlAvail = globalPool->slAvailOrNot[i] & (1 << j);

            header_addr_t blockHead;
            blockHead.value = globalPool->blocks[i][j];

            if(!hasFlAvail && hasSlAvail) {
                ++errCount;
                printf("FL and SL lists do not match for (%d, %d)\n", i, j);
            }

            if(!hasSlAvail && (GET_ADDRESS(blockHead) != &(globalPool->nullBlock))) {
                ++errCount;
                printf("Empty list does not start with nullBLock for (%d, %d)\n", i, j);
            }
            if(hasSlAvail) {
                // We now check all the blocks
                header_t *blockHeadPtr = GET_ADDRESS(blockHead);
                while(blockHeadPtr != &(globalPool->nullBlock)) {
                    if(!isBlockFree(blockHeadPtr)) {
                        printf("Block 0x%p should be free for (%d, %d)\n", blockHeadPtr, i, j);
                        ++errCount;
                    }
                    if(isBlockFree(GET_ADDRESS(getNextBlock(blockHeadPtr)))) {
                        printf("Block 0x%p should have coalesced with next for (%d, %d)\n", blockHeadPtr, i, j);
                        ++errCount;
                    }
                    if(!isPrevBlockFree(GET_ADDRESS(getNextBlock(blockHeadPtr)))) {
                        printf("Block 0x%p is not prevFree for (%d, %d)\n", blockHeadPtr, i, j);
                        ++errCount;
                    }
                    if(blockHeadPtr != GET_ADDRESS(getPrevBlock(GET_ADDRESS(getNextBlock(blockHeadPtr))))) {
                        printf("Block 0x%p cannot be back-reached for (%d, %d)\n", blockHeadPtr, i, j);
                        ++errCount;
                    }
                    if(blockHeadPtr->sizeBlock < (GminBlockRealSize - GusedBlockOverhead) || blockHeadPtr->sizeBlock > GmaxBlockRealSize) {
                        printf("Block 0x%p has illegal size for (%d, %d)\n", blockHeadPtr, i, j);
                        ++errCount;
                    }

                    int tf, ts;
                    mappingInsert(blockHeadPtr->sizeBlock, &tf, &ts);
                    if(tf != i || ts != j) {
                        printf("Block 0x%p is in wrong bucket for (%d, %d)\n", blockHeadPtr, i, j);
                        ++errCount;
                    }
                    countFree += blockHeadPtr->sizeBlock;
                    blockHead = getNextFreeBlock(blockHeadPtr);
                    blockHeadPtr = GET_ADDRESS(blockHead);
                }
            }
        }
    }
    *freeRemaining = countFree << ELEMENT_SIZE_LOG2;
    return errCount;
}

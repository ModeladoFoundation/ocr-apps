{% import "common_macros.inc.c" as util with context -%}
{{ util.auto_file_banner() }}

{% set defname = "_CNCOCR_INTERNAL_H_" -%}
#ifndef {{defname}}
#define {{defname}}

#include "cncocr.h"

#if CNCOCR_x86
extern void *memcpy(void *dest, const void *src, size_t n);
#    ifndef HAL_X86_64
#    define hal_memCopy(dest, src, len, bg) memcpy(dest, src, len)
#    define hal_fence() __sync_synchronize()
#    endif
#elif CNCOCR_TG
// some HAL definitions (apps/lib/tg/include)
#    include "misc.h"
#else
#    warning UNKNOWN PLATFORM (possibly unsupported)
#endif

#if CNCOCR_TG
#define CNC_TABLE_SIZE 16
#else
#define CNC_TABLE_SIZE 512
#endif

#define SIMPLE_DBCREATE(guid, ptr, sz) ocrDbCreate(guid, ptr, sz, DB_PROP_NONE, NULL_GUID, NO_ALLOC)

bool _cncPut(ocrGuid_t item, unsigned char *tag, int tagLength, cncItemCollection_t collection, bool isSingleAssignment);
bool _cncPutSingleton(ocrGuid_t item, ocrGuid_t collection, bool isSingleAssignment);

void _cncGet(unsigned char *tag, int tagLength, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, cncItemCollection_t collection);
void _cncGetSingleton(ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode, ocrGuid_t collection);

void *_cncRangedInputAlloc(u32 n, u32 dims[], size_t itemSize, ocrEdtDep_t *out);

// XXX - hack to work around broken lockable db provider
// functions copied from ocr-hal-fsim-xe.h
#if CNCOCR_TG && CNC_USE_HAL_LOCKS

/**
 * @brief Compare and swap (32 bit)
 *
 * The semantics are as follows (all operations performed atomically):
 *     - if location is cmpValue, atomically replace with
 *       newValue and return cmpValue
 *     - if location is *not* cmpValue, return value at location
 *
 * @param atomic        u32*: Pointer to the atomic value (location)
 * @param cmpValue      u32: Expected value of the atomic
 * @param newValue      u32: Value to set if the atomic has the expected value
 *
 * @return Old value of the atomic
 */
#define hal_cmpswap32(atomic, cmpValue, newValue)                       \
    ({                                                                  \
        u32 __tmp;                                                      \
        __asm__ __volatile__("cmpxchg32.any %0, %1, %2\n\t"             \
                             : "=r" (__tmp)                             \
                             : "r" (atomic),                            \
                               "r" (cmpValue),                          \
                               "0" (newValue));                         \
        __tmp;                                                          \
    })

/**
 * @brief Convenience function that basically implements a simple
 * lock
 *
 * This will usually be a wrapper around cmpswap32. This function
 * will block until the lock can be acquired
 *
 * @param lock      Pointer to a 32 bit value
 */
#define hal_lock32(lock)                        \
    while(hal_cmpswap32(lock, 0, 1))

/**
 * @brief Convenience function to implement a simple
 * unlock
 *
 * @param lock      Pointer to a 32 bit value
 */
#define hal_unlock32(lock)                      \
    *(u32 *)(lock) = 0

#define CNCTGL_ONLY(x) x

#else
#define CNCTGL_ONLY(x)
#endif /* CNCOCR_TG && CNC_USE_HAL_LOCKS */

typedef struct {
    CNCTGL_ONLY(volatile u32 lock;)
    ocrGuid_t blocks[1];
} _cncBucketHead_t;

#endif /*{{defname}}*/

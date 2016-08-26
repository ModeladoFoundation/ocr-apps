#ifndef _MISC_H_
#define _MISC_H_

#include "xstg-map.h"

#define BLOB_START (SR_IPM_BASE + 0x1F400000 + 0x3400) // Hardcoded, in IPM, to be fixed (bug #139)

/* This is basically lifted from OCR, but for the apps */

#define hal_fence()                                     \
    do { __asm__ __volatile__("fence 0xF, B\n\t"); } while(0)

#define hal_memCopy(destination, source, size, isBackground)            \
    do { __asm__ __volatile__("lea %0, %0\n\t"                          \
                              "lea %1, %1\n\t"                          \
                              "dma.copy %0, %1, %2, 0, 8\n\t"           \
                              :                                         \
                              : "r" ((void *)(destination)),            \
                                "r" ((void *)(source)),                 \
                                "r" (size)                              \
                              : "0", "1", "memory");                    \
        if (!isBackground) __asm__ __volatile__("fence 0x8, B\n\t" : : : "memory");    \
    } while(0)

#endif // _MISC_H_

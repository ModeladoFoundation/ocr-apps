#ifndef _MISC_H_
#define _MISC_H_

#define BLOB_START (0x240000000000 + 0x1F400000 + 0x3000) // Hardcoded, to be fixed (bug #139)

/* This is basically lifted from OCR, but for the apps */

#define hal_fence()                                     \
    do { __asm__ __volatile__("fence 0xF, B\n\t"); } while(0)

#define hal_memCopy(destination, source, size, isBackground)            \
    do { __asm__ __volatile__("dma.copy %0, %1, %2, 0, 8\n\t"           \
                              :                                         \
                              : "r" ((void *)(destination)),            \
                                "r" ((void *)(source)),                 \
                                "r" (size));                            \
        if (!isBackground) __asm__ __volatile__("fence 0xF, B\n\t");    \
    } while(0)

#endif // _MISC_H_

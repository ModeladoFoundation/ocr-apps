#ifndef STDIO_H
#define STDIO_H

/* degenerate stdio.h */

#include "ocr.h"

#ifdef TG_ARCH

#define printf( ... )          ocrPrintf(__VA_ARGS__)
#define fprintf(filedesc, ... ) ocrPrintf(__VA_ARGS__)
#define fflush(filedesc)

#endif

#endif

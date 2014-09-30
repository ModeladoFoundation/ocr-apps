#ifndef SDTLIB_H
#define SDTLIB_H

/* degenerate stdlib.h */

#include "ocr.h"

#ifdef TG_ARCH

#define exit(code) { PRINTF("exit(%d)\n",code); ocrShutdown(); }

#endif

#endif

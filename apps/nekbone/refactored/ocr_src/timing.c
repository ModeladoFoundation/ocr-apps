#ifndef NEKBONE_TIMING_H
#include "timing.h"
#endif

#ifdef NEKO_USE_TIMING
#include <sys/time.h>
#endif

#include "ocr.h" //PRINTF

TimeMark_t nekbone_getTime(void)
{
#   ifdef NEKO_USE_TIMING
        const unsigned long million = 1000000;
        struct timeval ptime;
        gettimeofday(&ptime, (struct timezone *)NULL);
        const unsigned long t = ptime.tv_usec + ptime.tv_sec*million;
        return t;
#   else
        return 0;
#   endif
}


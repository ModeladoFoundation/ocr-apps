#ifndef TIMING_H
#include "timing.h"
#endif

#ifdef USE_TIMING
#   include <sys/time.h>
//#   include "ocr.h" //PRINTF
#endif

TimeMark_t getTime(void)
{
#   ifdef USE_TIMING
        const unsigned long million = 1000000;
        struct timeval ptime;
        gettimeofday(&ptime, (struct timezone *)0);
        const unsigned long t = ptime.tv_usec + ptime.tv_sec*million;
        return t;
#   else
        return 0;
#   endif
}


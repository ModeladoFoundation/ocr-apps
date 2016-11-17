#ifndef NEKBONE_TIMING_H
#define NEKBONE_TIMING_H

//Define the following in order to actually get a time measurement
#define NEKO_USE_TIMING

//Define the following in order to enable the printing of timings.
#define NEKO_PRINT_TIMING

//Define the following in order to capture timings within the CG iterations.
#define NEKO_CG_TIMING

//Define the following in order to get the timing for a single CG iterations
//2016nov13: This measure has proven itself to be very flaky for some reason.  So left unused.
//#define NEKO_get_CGLOOP

#ifdef TG_ARCH
#   undef NEKO_USE_TIMING
#   undef NEKO_PRINT_TIMING
#endif // TG_ARCH

#ifndef NEKO_USE_TIMING
#undef NEKO_PRINT_TIMING
#undef NEKO_CG_TIMING
#endif

#define TIMEF "%ld"
typedef long TimeMark_t; //In micro-seconds as provided by gettimeofday()

TimeMark_t nekbone_getTime(void);

#ifdef NEKO_PRINT_TIMING
#   define TIMEPRINT1(txt,a)           PRINTF(txt,a)
#   define TIMEPRINT2(txt,a,b)         PRINTF(txt,a,b)
#   define TIMEPRINT3(txt,a,b,c)       PRINTF(txt,a,b,c)
#   define TIMEPRINT4(txt,a,b,c,d)     PRINTF(txt,a,b,c,d)
#   define TIMEPRINT5(txt,a,b,c,d,e)   PRINTF(txt,a,b,c,d,e)
#   define TIMEPRINT6(txt,a,b,c,d,e,f) PRINTF(txt,a,b,c,d,e,f)
#else
#   define TIMEPRINT1(txt,a)           do{}while(0)
#   define TIMEPRINT2(txt,a,b)         do{}while(0)
#   define TIMEPRINT3(txt,a,b,c)       do{}while(0)
#   define TIMEPRINT4(txt,a,b,c,d)     do{}while(0)
#   define TIMEPRINT5(txt,a,b,c,d,e)   do{}while(0)
#   define TIMEPRINT6(txt,a,b,c,d,e,f) do{}while(0)
#endif // NEKO_PRINT_TIMING

#ifdef NEKO_CG_TIMING
#define NEKO_CG_TIMFCN(x) x
#else
#define NEKO_CG_TIMFCN(x)
#endif

#endif // NEKBONE_TIMING_H

#ifndef TIMING_H
#define TIMING_H

//Define the following in order to actually get a time measurement
#define USE_TIMING

//Define the following in order to enable the printing of timings.
#define PRINT_TIMING

#ifdef TG_ARCH
#   undef USE_TIMING
#   undef PRINT_TIMING
#endif // TG_ARCH

#ifndef USE_TIMING
#undef PRINT_TIMING
#endif

#define TIMEF "%ld"  //Format to use for TimeMark_t within printf/PRINTF statements.
typedef long TimeMark_t; //In micro-seconds as provided by gettimeofday()

TimeMark_t getTime(void);

#ifdef PRINT_TIMING
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
#endif // PRINT_TIMING

#endif // TIMING_H

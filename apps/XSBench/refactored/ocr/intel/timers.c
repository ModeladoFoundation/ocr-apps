#ifndef TG_ARCH
#include <sys/time.h>
#include <inttypes.h>
#endif
#include <ocr.h>

#include "timers.h"

#define TICK (1.0e-6)

static u64 getTime(void)
{
#ifndef TG_ARCH
   struct timeval ptime;
   gettimeofday(&ptime, (struct timezone *)NULL);
   return ((u64)1000000)*(u64)ptime.tv_sec + (u64)ptime.tv_usec;
#else
   return 0;
#endif
}

void profile_start(const enum timer_name handle, void* t)
{
   timer* timers = (timer*) t;
   timers[handle].start = getTime();
}

void profile_stop(const enum timer_name handle, void* t)
{
   timer* timers = (timer*) t;
   ++timers[handle].count;
   u64 delta = getTime() - timers[handle].start;
   timers[handle].total += delta;
   timers[handle].elapsed += delta;
}

double get_elapsed_time(const enum timer_name handle, void* t)
{
   timer* timers = (timer*) t;
   double etime = TICK * (double)timers[handle].elapsed;
   timers[handle].elapsed = 0;
   return etime;
}

#include <stdio.h>
#ifndef TG_ARCH
#include <sys/time.h>
#include <inttypes.h>
#endif
#include <ocr.h>

#include "simulation.h"
#include "timers.h"

#define TICK (1.0e-6)

char* timer_name[number_of_timers] = {
   "total",
   "loop",
   "timestep",
   "  position",
   "  velocity",
   "  redistribute",
   "    atomHalo",
   "  force",
   "    eamHalo",
   "commHalo",
   "commReduce"
};

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

void print_timers(void* t, u32 atoms)
{
   timer* timers = (timer*) t;
   double loop_time = timers[loop_timer].total*TICK;

   PRINTF("\n\nTimings\n");
   PRINTF("        Timer        # Calls    Avg/Call (s)   Total (s)    %% Loop\n");
   PRINTF("___________________________________________________________________\n");
   for (int ii=0; ii<number_of_timers; ++ii)
   {
      double total = timers[ii].total*TICK;
      if (timers[ii].count > 0)
#ifdef TG_ARCH
         PRINTF("%16s%12lu     %8.4f      %8.4f    %8.2f\n",
#else
         printf("%-16s%12"PRIu64"     %8.4f      %8.4f    %8.2f\n",
#endif
                 timer_name[ii], timers[ii].count, total/(double)timers[ii].count,
                 total, total/loop_time*100.0);
   }
   real_t rate = timers[compute_force_timer].total / (atoms * timers[compute_force_timer].count);
   PRINTF("\n---------------------------------------------------\n");
   PRINTF(" Average atom update rate: %6.2f us/atom/task\n", rate);
   PRINTF("---------------------------------------------------\n\n");
}

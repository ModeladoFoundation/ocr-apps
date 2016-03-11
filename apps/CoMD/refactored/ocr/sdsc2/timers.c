#include <ocr.h>

#include "timers.h"

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

void print_timers(mdtimer_t* timers_p, u32 atoms, u32 steps)
{
  double loop_time = timers_p[total_timer].total*TICK;

  PRINTF("\n\nTimings\n");
  PRINTF("        Timer        # Calls    Avg/Call (s)   Total (s)    %% Loop\n");
  PRINTF("___________________________________________________________________\n");
  for (int ii=0; ii<number_of_timers; ++ii)
  {
    double total = timers_p[ii].total*TICK;
    if(timers_p[ii].count > 0)
#ifndef TG_ARCH
      printf("%-16s%12llu     %8.4f      %8.4f    %8.2f\n",
#else
      PRINTF("%16s %11llu     %8.4f      %8.4f    %8.2f\n",
#endif
             timer_name[ii], timers_p[ii].count, total/(double)timers_p[ii].count,
             total, total/loop_time*100.0);
  }
  double rate = (double) timers_p[total_timer].total / (atoms * steps);
  //double rate = 1; //FIXME
  PRINTF("\n---------------------------------------------------\n");
  PRINTF(" Average atom update rate: %6.2f us/atom/task\n", rate);
  PRINTF("---------------------------------------------------\n\n");
}

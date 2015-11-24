#ifndef TIMERS_H
#define TIMERS_H

#ifndef TG_ARCH
#include <stdio.h>
#include <sys/time.h>
#endif
#include <ocr.h>

#define TICK (1.0e-6)

enum timer_name {
total_timer, loop_timer, timestep_timer, position_timer, velocity_timer,
redistribute_timer, atom_halo_timer, compute_force_timer, eam_halo_timer,
comm_halo_timer, comm_reduce_timer, number_of_timers};

typedef struct
{
  u64 start;
  u64 total;
  u64 count;
  u64 elapsed;
} mdtimer_t;

static inline u64 getTime(void)
{
#ifdef TG_ARCH
  return 0;
#else
  struct timeval ptime;
  gettimeofday(&ptime, (struct timezone *)NULL);
  return ((u64)1000000)*(u64)ptime.tv_sec + (u64)ptime.tv_usec;
#endif
}

static inline void profile_start(const enum timer_name handle, mdtimer_t* timers_p)
{
  timers_p[handle].start = getTime();
}

static inline void profile_stop(const enum timer_name handle, mdtimer_t* timers_p)
{
  ++timers_p[handle].count;
  u64 delta = getTime() - timers_p[handle].start;
  timers_p[handle].total += delta;
  timers_p[handle].elapsed += delta;
}

static inline double get_elapsed_time(const enum timer_name handle, mdtimer_t* timers_p)
{
  double etime = TICK * (double)timers_p[handle].elapsed;
  timers_p[handle].elapsed = 0;
  return etime;
}

static inline double get_total_time(const enum timer_name handle, mdtimer_t* timers_p)
{
  return TICK * (double)timers_p[handle].total;
}

void print_timers(mdtimer_t* timers_p, u32 atoms);

#endif

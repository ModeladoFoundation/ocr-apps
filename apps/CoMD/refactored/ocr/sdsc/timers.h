#ifndef TIMERS_H
#define TIMERS_H

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
} timer;

void profile_start(const enum timer_name name, void* t);

void profile_stop(const enum timer_name name, void* t);

double get_elapsed_time(const enum timer_name name, void* t);

void print_timers(void* t, u32 atoms);

#endif

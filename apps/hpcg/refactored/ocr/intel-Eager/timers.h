#ifndef TIMERS_H
#define TIMERS_H

enum timer_name {
total_timer, number_of_timers};

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

u64 getTime(void);

#endif

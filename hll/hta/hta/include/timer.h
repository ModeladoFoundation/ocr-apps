#ifndef __TIMER_H__
#define __TIMER_H__
#include <sys/time.h>
#include <assert.h>

#define timer_t double
#define NUM_TIMERS (32)
#define CHECK_VALID_TIMER_ID  (assert(id < NUM_TIMERS && id >= 0))

timer_t hta_timers[NUM_TIMERS];

inline double get_cur_time() {
    struct timeval t;
    gettimeofday(&t, (void*)0);
    return t.tv_sec + 1.0e-6*t.tv_usec;
}

inline void hta_timer_start(int id) {
    CHECK_VALID_TIMER_ID;
    hta_timers[id] = get_cur_time();
}

inline void hta_timer_stop(int id) {
    hta_timers[id] = get_cur_time() - hta_timers[id];
    CHECK_VALID_TIMER_ID;
}

double timer_get_sec(int id) {
    CHECK_VALID_TIMER_ID;
    return hta_timers[id];
}
double get_cur_time();
void hta_timer_start(int id);
void hta_timer_stop(int id);
#endif

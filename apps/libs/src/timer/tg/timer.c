#include "timer.h"


double usec_to_sec (long usec) {
    return 0.0;
}

void get_time(struct timeval * t_time) {
}

long elapsed_usec(struct timeval * t_start, struct timeval * t_stop) {
    return 0;
}

double elapsed_sec(struct timeval * t_start, struct timeval * t_stop) {
   return 0.0;
}

void print_throughput(char * timer_name, unsigned long long nb_instances, double time_sec) {
}

void print_throughput_custom(char * timer_name, unsigned long long nb_instances, double time_sec, double throughput) {
}

void print_elapsed(double time_sec) {
}

void print_elapsed_usec(long time_usec) {
}

void summary_throughput_timer(timestamp_t * start_t, timestamp_t * stop_t, unsigned long long instances) {
}

void summary_throughput_dbl(double secs, unsigned long long instances) {
}

double avg_usec(long * array, int length) {
    return 0.0;
}

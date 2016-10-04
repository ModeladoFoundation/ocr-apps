#include "timer.h"

#include "ocr.h"

#define UNIT_WORKLOAD_STR ("unit")
#define UNIT_DURATION_STR ("s")
#define UNIT_THROUGHPUT_STR ("op/s")

double usec_to_sec (long usec) {
    return 0.0;
}

void get_time(timestamp_t * t_time) {
    return;
}

long elapsed_usec(timestamp_t * t_start, timestamp_t * t_stop) {
    return 0;
}

double elapsed_sec(timestamp_t * t_start, timestamp_t * t_stop) {
   return 0.0;
}

void print_throughput_custom_name(char * timer_name,
                             char * workloadUnit, unsigned long long workload,
                             char * durationUnit, double duration,
                             char * throughputUnit, double throughput) {
    PRINTF("Timer Name        : %s\n", timer_name);
    PRINTF("Workload    (%s): %llu\n", (workloadUnit ? workloadUnit : UNIT_WORKLOAD_STR), workload);
    PRINTF("Duration    (%s)   : %f\n", (durationUnit ? durationUnit : UNIT_DURATION_STR), duration);
    PRINTF("Throughput  (%s): %f\n", (throughputUnit ? throughputUnit : UNIT_THROUGHPUT_STR), throughput);
}

void print_throughput_custom(char * timer_name, unsigned long long nb_instances, double duration_sec, double throughput) {
    print_throughput_custom_name(timer_name,
                                 NULL, nb_instances,
                                 NULL, duration_sec,
                                 NULL, throughput);
}

void print_throughput(char * timer_name, unsigned long long nb_instances, double duration_sec) {
    print_throughput_custom(timer_name, nb_instances, duration_sec, (nb_instances/duration_sec));
}

void print_elapsed(double duration_sec) {
    PRINTF("Elapsed        (s): %f\n", duration_sec);
}

void print_elapsed_usec(long duration_sec) {
    PRINTF("Elapsed       (us): %ld\n", duration_sec);
}

void summary_throughput_timer(timestamp_t * start_t, timestamp_t * stop_t, unsigned long long instances) {
    double secs = elapsed_sec(start_t, stop_t);
    summary_throughput_dbl(secs, instances);
}

void summary_throughput_dbl(double secs, unsigned long long instances) {
    print_elapsed(secs);
    print_throughput("Summary", instances, secs);
}

double avg_usec(long * array, int length) {
    int i = 0;
    long acc = 0;
    while (i < length) {
        acc += array[i];
        i++;
    }
    double res = (((double)acc)/length);
    return res;
}

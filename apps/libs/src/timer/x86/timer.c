#include "timer.h"


#define UNIT_WORKLOAD_STR ("unit")
#define UNIT_DURATION_STR ("s")
#define UNIT_THROUGHPUT_STR ("op/s")

double usec_to_sec (long usec) {
    double res = ((double)usec) / 1000000;
    return res;
}

void get_time(struct timeval * t_time) {
    gettimeofday(t_time, NULL);
}

long elapsed_usec(struct timeval * t_start, struct timeval * t_stop) {
    long start_usec = t_start->tv_sec*1000000+t_start->tv_usec;
    long stop_usec = t_stop->tv_sec*1000000+t_stop->tv_usec;
    long elapsed = stop_usec - start_usec;
    return elapsed;
}

double elapsed_sec(struct timeval * t_start, struct timeval * t_stop) {
    long elapsed = elapsed_usec(t_start, t_stop);
    return (((double)elapsed)/1000000);
}

void print_throughput_custom_name(char * timer_name,
                             char * workloadUnit, unsigned long long workload,
                             char * durationUnit, double duration,
                             char * throughputUnit, double throughput) {
    printf("Timer Name        : %s\n", timer_name);
    printf("Workload    (%s): %llu\n", (workloadUnit ? workloadUnit : UNIT_WORKLOAD_STR), workload);
    printf("Duration    (%s)   : %f\n", (durationUnit ? durationUnit : UNIT_DURATION_STR), duration);
    printf("Throughput  (%s): %f\n", (throughputUnit ? throughputUnit : UNIT_THROUGHPUT_STR), throughput);
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
    printf("Elapsed        (s): %f\n", duration_sec);
}

void print_elapsed_usec(long duration_sec) {
    printf("Elapsed       (us): %ld\n", duration_sec);
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

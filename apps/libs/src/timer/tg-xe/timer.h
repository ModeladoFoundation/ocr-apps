#ifndef __TIMER_H_
#define __TIMER_H_


#define timestamp_t void *

void get_time(timestamp_t t_time);

double elapsed_sec(timestamp_t * start, timestamp_t * stop);

long elapsed_usec(timestamp_t * start, timestamp_t * stop);

double avg_usec(long * array, int length);


/* Conversion functions */

double usec_to_sec (long usec);


/* Printing functions */

void print_throughput(char * timer_name, unsigned long long nb_instances, double time_sec);

void print_throughput_custom(char * timer_name, unsigned long long nb_instances, double time_sec, double throughput);

void summary_throughput_timer(timestamp_t  * start_t, timestamp_t * stop_t, unsigned long long instances);

void summary_throughput_dbl(double secs, unsigned long long instances);

#endif /* __TIMER_H_ */
#ifndef PIL_RSTREAM_H
#define PIL_RSTREAM_H

#define X 1024
#define Y 1024
#define Z 1024

void main_function(int *target_id, int argc, char **argv, int *end, double A[X][Z], double B[Z][Y], double C[X][Y], double *kernel_start_time);
void parallel_multiply_function(int *target_id, int i, double A[X][Z], double B[Z][Y], double C[X][Y]);
void finalize_function(int *target_id, double C[X][Y], double *kernel_start_time);

#endif /* PIL_RSTREAM_H */

#include <stdint.h>
#include <stdio.h>
//#include "benchmark.h"
#include "example300.h"

void main_function(int *target_id, int argc, char **argv, int *end, double A[X][Z], double B[Z][Y], double C[X][Y], double *kernel_start_time) {
	int32_t i, j, k;
	struct timer init_timer;

	*end = X-1;

	printf("example300  x: %d  y: %d  z: %d\n", X, Y, Z);

	//print_num_threads();

	/* Initialize Arrays */
	printf("Initialization: ");
	start_timer(&init_timer);
	// TODO: should i malloc aoffsets and A and remalloc them later, or pil_alloc them once and pass them?
	/*
	A = (double (*)[X]) malloc(X * Z * sizeof(double));
	B = (double (*)[Z]) malloc(Z * Y * sizeof(double));
	C = (double (*)[X]) malloc(X * Y * sizeof(double));
	*/
	for (i = 0; i < X; i++)
	{
		for (k = 0; k < Z; k++)
		{
			A[i][k] = 1.0;
		}
		for (j = 0; j < Y; j++)
		{
			C[i][j] = 0.0;
		}
	}
	for (k = 0; k < Z; k++)
	{
		for (j = 0; j < Y; j++)
		{
			B[k][j] = 1.0;
		}
	}
	stop_timer(&init_timer);
	printf("%11.3lf ms\n", init_timer.elapsed);

	/* Kernel */
	printf("Kernel: ");
	*kernel_start_time = omp_get_wtime();

	*target_id = 2;
}

#pragma rstream map
void parallel_multiply_function(int *target_id, int i, double A[X][Z], double B[Z][Y], double C[X][Y]) {
	int j, k;

	for (j=0; j<Y; j++)
	{
		//cc = 0.0;
		for (k=0; k<Z; k++)
		{
			C[i][j] += A[i][k] * B[k][j];
			//cc += A[i][k] * B[k][j];
		}
		//C[i][j] = cc;
	}

	*target_id = 3;
}

void finalize_function(int *target_id, double C[X][Y], double *kernel_start_time) {
	struct timer verification_timer;
	double kernel_time = omp_get_wtime() - *kernel_start_time;
	bool correct = TRUE;
	printf("        %11.3lf sec\n", kernel_time);

	/* Verify Output */
	printf("Verification: ");
	start_timer(&verification_timer);
	if ((X==Y) && (X==X))
	{
		int i, j;
		for (i=0; i<X; i++)
		{
			for (j=0; j<Y; j++)
			{
				if (C[i][j] != X)
					correct = FALSE;
			}
		}
	}
	stop_timer(&verification_timer);
	printf("  %11.3lf ms\n", verification_timer.elapsed);
	print_correct(correct);
	
	*target_id = 0;
}

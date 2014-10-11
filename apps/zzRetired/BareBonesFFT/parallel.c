#include "parallel.h"

int main(int argc, char *argv[])
{
	numThreads = 2;
	point *dataPoints = hardCoded();
	origPointer = dataPoints;
	point* origPoints = copyPoints(dataPoints, numPoints, 1);

	int ret = 0x10000;

	point* fftPoints = FourierTransform(dataPoints, numPoints, 1);
	point* invertedFFTPoints = FourierTransform(fftPoints, numPoints, -1);
	if(checkPoints(origPoints, invertedFFTPoints, numPoints) == 0)
	{
		printf("f-(f(x)) differs from x! badtransformation!\n");
		printTwoPoints(origPoints, dataPoints, numPoints);
	}
	else
	{
		printf("Transformation completed successfully\n");

	}
	freeAll(fftPoints, invertedFFTPoints, origPoints, dataPoints);
	return ret ;
}

//void *malloc(unsigned long x) {return (void *) 0x0;} //because sesc wraps malloc
//void free(void *x) {} //because sesc wraps malloc

/*
 *	Wrapper to Fourier Transformation
 */
point* FourierTransform(point* x, int N, int inverse)
{
	twiddle = buildTwiddle(N, inverse);

	pthread_t thread;
	StartData td;
	td.x = x;
	td.N = N;
	td.stride = 1;
	td.inverse = inverse;
	td.thread_id = 0;
	td.depth = 1;
	pthread_create(&thread, NULL, &parallelFastFourierTransform, &(td));
	void* v_fft;
	pthread_join(thread, &v_fft);
	//point *fft = (point *) (((EndData *) v_fft).X);
	point *fft = (point *) v_fft;

	if(inverse == -1)
	{
		int k;
		for(k = 0; k < N; k++)
		{
			fft[k].real /= (double) N;
			fft[k].imaginary /= (double) N;
		}
	}

	return fft;
}

/**
 *	Recursive definition of Cooley-Tukey, where subtree exploration are run in parallel
 *	refactored to expose essential steps of algorithm
 */
void* parallelFastFourierTransform(void *v_td)
{
	StartData td = *((StartData *) v_td);
	point *x = td.x;
	int N = td.N;
	int inverse = td.N;
	int stride = td.stride;


	point* X = (point *) malloc(N*sizeof(point));
	if (N == 1) {X[0] = x[0]; return X;}

	point *even = NULL, *odd = NULL;

	if(stride < numThreads)
	{
		recurseParallel(x, &even, &odd, N, stride, inverse, td.thread_id, td.depth);
		runParallel(N, numThreads / stride, even, odd, X, stride, N/2);
	}
	else
	{
		even = serialFastFourierTransform(x, N/2, stride*2, inverse);
		odd = serialFastFourierTransform(x+stride, N/2, stride*2, inverse);
		runSerial(even, odd, stride, N/2, X);
	}

	free(even);
	free(odd);
	return X;
}

/*
 *	For the higher level of the trees we can perform the recursion in parallel
 *	(up until we run out of threads or incur to much overhead)
 */
void recurseParallel(point * x, point **even, point **odd, int N, int stride, int inverse, int thread_id, int depth)
{
	pthread_t thread_even, thread_odd;
	StartData td_even, td_odd;
	td_even.x = x; 				td_odd.x = x+stride;
	td_even.N = N/2; 			td_odd.N = N/2;
	td_even.stride = stride*2; 		td_odd.stride = stride*2;
	td_even.inverse = inverse; 		td_odd.inverse = inverse;
	td_even.thread_id = 2*thread_id; 	td_odd.thread_id = 2*thread_id + 1;
	td_even.depth = depth*2; 		td_odd.depth = depth*2;

	pthread_create(&thread_even, NULL, &parallelFastFourierTransform, &(td_even));
	pthread_create(&thread_odd, NULL, &parallelFastFourierTransform, &(td_odd));

	void* v_fft;
	pthread_join(thread_even, &v_fft);
	(*even) = (point *) v_fft;
	pthread_join(thread_odd, &v_fft);
	(*odd) = (point *) v_fft;
}

/*
 *	For the higher level of the trees we can perform the computation in parallel
 */
void runParallel(int N, int thread_count, point *even, point *odd, point *X, int stride, int offset)
{
	int k;
	//task-centric version of for loop
	pthread_t threads[thread_count];
	DFTData threadData[thread_count];
	for(k = 0; k < thread_count; k++)
	{
		threadData[k].start = k*N/thread_count;
		threadData[k].end = (k+1)*N/thread_count;
		threadData[k].even = even;
		threadData[k].odd = odd;
		threadData[k].X = X;
		threadData[k].stride = stride;
		threadData[k].offset = N/2;

		if(k < thread_count/2)
			pthread_create(&(threads[k]), NULL, &plus_wrapper, &(threadData[k]));
		else
			pthread_create(&(threads[k]), NULL, &minus_wrapper, &(threadData[k]));
	}

	for(k = 0; k < thread_count; k++)
	{
		pthread_join(threads[k], NULL);
	}
}

/*
 *	Once our depth has exceeded the number of threads, run in serial
 */
void runSerial(point *even, point *odd, int stride, int offset, point * X)
{
	int k;
	for(k = 0; k < offset; k ++)
	{
		X[k] = plus(even[k], (mult(twiddle[k*stride], odd[k])));
	}
	for(k = offset; k < offset*2; k ++)
	{
		X[k] = minus(even[k - offset], (mult(twiddle[(k - offset)*stride], odd[k - offset])));
	}
}

/*
 *	have threads either perform the addition or subtraction
 *	of the subcomponents generated recursively
 */
void *plus_wrapper(void *v_td)
{
	DFTData td = *((DFTData *) v_td);
	int start = td.start;
        int end = td.end;
        point *even = td.even;
        point *odd = td.odd;
        point *X = td.X;
        int stride = td.stride;

	int k;
	for(k = start; k < end; k++)
	{
		X[k] = plus(even[k], (mult(twiddle[k*stride], odd[k])));
	}

	return NULL;
}

/*
 *	have threads either perform the addition or subtraction
 *	of the subcomponents generated recursively
 */
void *minus_wrapper(void *v_td)
{
	DFTData td = *((DFTData *) v_td);
	int start = td.start;
        int end = td.end;
        point *even = td.even;
        point *odd = td.odd;
        point *X = td.X;
        int stride = td.stride;
        int offset = td.offset;

	int k;
	for(k = start; k < end; k++)
	{
		X[k] = minus(even[k - offset], (mult(twiddle[(k - offset)*stride], odd[k - offset])));
	}

	return NULL;
}

/*
 *	Recursive definition of Cooley-Tukey
 */
point* serialFastFourierTransform(point* x, int N, int stride, int inverse)
{
	point* X = (point *) malloc(N*sizeof(point));
	if (N == 1) {X[0] = x[0]; return X;}

	int k;

	point* even = serialFastFourierTransform(x, N/2, stride*2, inverse);
	point* odd = serialFastFourierTransform(x+stride, N/2, stride*2, inverse);

	int offset = N/2;

	for(k = 0; k < N/2; k ++)
	{
		X[k] = plus(even[k], (mult(twiddle[k*stride], odd[k])));
	}
	for(k = N/2; k < N; k ++)
	{
		X[k] = minus(even[k - offset], (mult(twiddle[(k - offset)*stride], odd[k - offset])));
	}

	free(even);
	free(odd);

	return X;
}

/*
 *	Copy over a list of points and return a pointer to that structure
 */
point* copyPoints(point* a, int numPoints, int stride)
{
	point* b = (point*) malloc(numPoints*sizeof(point));
	int k = 0;
	int count = 0;
	for(; k < numPoints; k+=stride)
	{
		b[count].real = a[k].real;
		b[count].imaginary = a[k].imaginary;
		count++;
	}

	return b;
}

/*
 *	returns non-zero if two lists are the same, zero otherwise
 */
int checkPoints(point* a, point* b, int numPoints)
{
	int k = 0;
	for(; k < numPoints; k++)
	{
		if((a[k].real - b[k].real) > 0.001 || (a[k].imaginary - b[k].imaginary) > 0.001
		|| (a[k].real - b[k].real) < -0.001 || (a[k].imaginary - b[k].imaginary) < -0.001
		)
		{
			//printf("First differ at index: %d\n", k);
			printPoint(a[k]);
			printPoint(b[k]);

			if(a[k].real != b[k].real) //printf("real: %f\n", a[k].real - b[k].real);
			if(a[k].imaginary != b[k].imaginary) //printf("imaginary: %f\n", a[k].imaginary - b[k].imaginary);
			return 0;
		}
	}

	return 1;
}

#include "helper.h"

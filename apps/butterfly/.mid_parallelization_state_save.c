#include "parallel.h"
#include "helper.c"

int main(int argc, char *argv[])
{
	char *file = (char *) "Input/default.txt";
	if(argc > 1) file = argv[1];
	point* dataPoints = buildDataPoints(file);
	origPointer = dataPoints;
	point* origPoints = copyPoints(dataPoints, numPoints, 1);

	struct timeval startTime, endTime;
	gettimeofday(&startTime, NULL);

	point* fftPoints = FourierTransform(dataPoints, numPoints, 1);

	gettimeofday(&endTime, NULL);
	int BASE_TO_MICRO = 1000000;
	double tS = startTime.tv_sec*BASE_TO_MICRO + startTime.tv_usec;
	double tE = endTime.tv_sec*BASE_TO_MICRO + endTime.tv_usec;
	printf("Total processor usage: %f seconds\n", (tE-tS)/BASE_TO_MICRO);

	char *outputFile = outfileName(file);
	writeData(fftPoints, outputFile);

	point* invertedFFTPoints = FourierTransform(fftPoints, numPoints, -1);
	if(checkPoints(origPoints, invertedFFTPoints, numPoints) == 0)
		printf("f-(f(x)) differs from x! bad transformation!\n");
	else
		printf("Transformation completed successfully.\n");

	freeAll(fftPoints, invertedFFTPoints, origPoints, dataPoints);
	return 0;
}

point *fastFourierTransform(point *x, int N)
{
	point *X = (point *) malloc(N*sizeof(point));
	point *Y = (point *) malloc(N*sizeof(point));
	point *Xstart, *Xp, *odd, *even;

	even = x;

	int k = 0, m = 1;
	int stride = N/2;
	int offset = N/2;
	int flag = 1;

	for(; m < N; m *= 2)
	{
		Xstart = flag? Y : X;
		Xp = Xstart;
		odd = even + stride;
		for(k = 0; k < N/2; k++)
		{
			odd[k] = mult(twiddle[(k/stride)*stride], odd[k]);
			Xp[k] = plus(even[k], odd[k]);
			Xp[k + offset] = minus(even[k], odd[k]);
			if((k+1) % stride == 0)
			{
				even += stride;
				odd = even + stride;
			}
		}

		stride /= 2;
		even = Xstart;
		flag = flag? 0 : 1;
	}

	if(flag)
	{	free(Y);
		return X;
	}
	
	free(X);
	return Y;
}

void * parallelWorker(void *input)
{
	FFT_t fft = *((FFT_t *) input);
	int N = fft.N;
	point *X = (fft.X);
	point *Y = (fft.Y);
	point *even = (fft.x);	
	int start = (fft.start);
	int end = (fft.end);

	point *Xstart, *Xp, *odd;

	int k, m = 1, stride = N/2, offset = N/2, flag = 1;

	for(; m < N; m *= 2)
	{
		Xstart = flag? Y : X;
		Xp = Xstart;
		even += stride*();
		odd = even + stride; //TODO
		for(k = start; k < end; k++)
		{
			odd[k] = mult(twiddle[(k/stride)*stride], odd[k]); //TODO make sure not to duplicate
			Xp[k] = plus(even[k], odd[k]);
			Xp[k + offset] = minus(even[k], odd[k]); //TODO : split this pairing for spatial locality purposes (within a thread it probably matters)

			if((k+1) % stride == 0) //may be able to do this more intelligently if we constrain ourselves to threads which are powers of two
			{
				even += stride;
				odd = even + stride;
			}
		}
	
		stride /= 2;
		even = Xstart;
		flag = flag? 0 : 1;
		pthread_barrier_wait(&barr);
	}

	return NULL;
}

//first I will make an iterative algorithm
point *FourierTransform(point *x, int N, int inverse)
{
	twiddle = buildTwiddle(N, inverse);

	point *fft = fastFourierTransform(x, N);

	if(inverse == -1)
		invert(fft, N);

	//free
	return fft;
}


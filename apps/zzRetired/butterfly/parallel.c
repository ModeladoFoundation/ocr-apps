#include "parallel.h"
#include "helper.c"

int main(int argc, char *argv[])
{
	char *file = (char *) "Input/default.txt";
	numThreads = 2;
	if(argc > 1) file = argv[1];
	if(argc > 2) numThreads = atoi(argv[2]);
	point* dataPoints = buildDataPoints(file);
	//point* dataPoints = hardCoded();
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

	//char *outputFile = outfileName(file);
	//writeData(fftPoints, outputFile);

	//printPoints(fftPoints, numPoints);
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

	pthread_barrier_init(&barr, NULL, numThreads);
	pthread_t threads[numThreads];
	FFT_t threadData[numThreads];
	int k = 0;
	for(; k < numThreads; k++)
	{
		threadData[k].N = N;
		threadData[k].X = X;
		threadData[k].Y = Y;
		threadData[k].x = x;
		threadData[k].start = (k*(N/numThreads))/2;
		threadData[k].end = threadData[k].start + ((N/numThreads)/2);
		threadData[k].thread_id = k;
		if(k == numThreads - 1) threadData[k].end = N/2;

		pthread_create(&(threads[k]), NULL, &parallelWorker, &(threadData[k]));
	}

	for(k = 0; k < numThreads; k++)
		pthread_join(threads[k], NULL);

	if(logTwo(N) % 2 == 0)
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
	point *X = fft.X;
	point *Y = fft.Y;
	point *even = fft.x;
	int start = fft.start;
	int end = fft.end;
	int thread_id = fft.thread_id;
	point *Xstart, *Xp, *odd;
	int k, m = 1, stride = N/2, offset = N/2, flag = 1;

	for(; m < N; m *= 2)
	{
		Xstart = flag? Y : X;
		Xp = Xstart;
		//even += (stride*((thread_id) * (m/2)));
		//even += thread_id * stride * (N/(end-start)) * ((m/2) / );
		even += stride * (int) (((float) thread_id) * (((float) m) / ((float) numThreads)));
//constrain to powers of two by this statement (currently)
		odd = even + stride; //TODO
		for(k = start; k < end; k++)
		{
			odd[k] = mult(twiddle[(k/stride)*stride], odd[k]); //TODO make sure not to duplicate
			Xp[k] = plus(even[k], odd[k]);
			Xp[k + offset] = minus(even[k], odd[k]); //TODO : split this pairing for spatial locality purposes (within a thread it probably matters)

			if((k+1) % stride == 0) //may be able to do this more intelligently if we constrain ourselves to threads which are powers of two
			{
				//printf("start: %d end: %d thread: %d m: %d k: %d\n", start, end, thread_id, m, k);
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


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

	//freeAll(fftPoints, invertedFFTPoints, origPoints, dataPoints);
	return 0;
}

point *fastFourierTransform(point *x, int N, int inverse)
{
	point *X = (point *) malloc(N*sizeof(point));
	point *Y = copyPoints(x, N, 1);	

	int k = 0;
	int stride = N/2;
	int offset = 1;
	//LEFT OFF: NEED TO ITERATE THROUGH A BINARY NUMBER TREE. FOCUS AND DO IT.
	int flag = 1;

//	for(; offset < N; offset *= 2)
		for(; stride > 0; stride /= 2)
		{
			for(k = 0; k < N/2; k++)
			{
				if(flag)
				{
			printf("Y N/2: %d stride: %d k: %d\n", offset, stride, k);
			printPoint(Y[k * offset]);
			printPoint(Y[(k * offset) + stride]);
					X[k * stride] = plus(Y[k * offset], mult(twiddle[(k/stride)*stride], Y[(k*offset) + stride]));
					X[(k * stride) + offset] = minus(Y[k * offset], (mult(twiddle[(k/stride)*stride], Y[(k*offset) + stride])));
			printPoint(X[k * stride]);
			printPoint(X[(k * stride) + offset]);
printf("--------------------------------\n");
			}
				else
				{
			printf("Y N/2: %d stride: %d k: %d\n", offset, stride, k);
			printPoint(X[k]);
			printPoint(X[(k * offset) + stride]);
					Y[k * stride] = plus(X[k * offset], mult(twiddle[(k/stride)*stride], X[(k*offset) + stride]));
					Y[(k * stride) + offset] = minus(X[k * offset], (mult(twiddle[(k/stride)*stride], X[(k*offset) + stride])));
			printPoint(Y[k * stride]);
			printPoint(Y[(k * stride) + offset]);
printf("--------------------------------\n");
				}
			}

			offset *= 2;
			flag = !flag;
		}

	if(flag)
		return Y;
	else
		return X;
}

//first I will make an iterative algorithm
point *FourierTransform(point *x, int N, int inverse)
{
	twiddle = buildTwiddle(N, inverse);

	point *fft = fastFourierTransform(x, N, inverse);

	if(inverse == -1)
		invert(fft, N);

	//free
	return fft;
}


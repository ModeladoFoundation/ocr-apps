#include "serial.h"
#include "helper.c"

int main(int argc, char *argv[])
{
	char *file = (char *) "Input/default.txt";
	if(argc > 1) file = argv[1];
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

//input must be a power of two! this goes unchecked!
point *fastFourierTransform(point *x, int N)
{
	point *X = (point *) malloc(N*sizeof(point));
	point *Y = (point *) malloc(N*sizeof(point));
	point *Xstart, *Xp, *odd, *even;

	even = x;

	int k = 0, m = 0;
	int stride = N/2;
	int offset = 1;
	int flag = 1;

	for(; offset < N; offset *= 2)
	{
		Xstart = flag? Y : X;
		Xp = Xstart;
		for(k = 0; k < offset; k++)
		{
			for(m = 0; m < stride; m++)
			{
				odd = even + stride;
				*odd = mult(twiddle[k*stride], *odd);
				*Xp = plus(*even, *odd);
				Xp[N/2] = minus(*even, *odd);
				Xp++;
				even++;
			}

			even += stride;
		}

		stride /= 2;
		even = Xstart;
		flag = flag? 0 : 1;
	}

	//if(logTwo(N) % 2 == 0)
	if(flag)
	{
		free(Y);
		return X;
	}

	free(X);
	return Y;
}

//input must be a power of two! this goes unchecked!
point *FourierTransform(point *x, int N, int inverse)
{
	twiddle = buildTwiddle(N, inverse);

	point *fft = fastFourierTransform(x, N);

	if(inverse == -1)
		invert(fft, N);

	//free
	return fft;
}


#include "x86_serial.h"

int main(int argc, char *argv[])
{
	char *file = (char *) "Input/default.txt";
	if(argc > 1) file = argv[1];
	//point* dataPoints = buildDataPoints(file);
	point *dataPoints = hardCoded();
	origPointer = dataPoints;
	point* origPoints = copyPoints(dataPoints, numPoints, 1);

	point* fftPoints = FourierTransform(dataPoints, numPoints, 1);

	//char *outputFile = outfileName(file);
	//writeData(fftPoints, outputFile);

	int ret = 0x10000;

	point* invertedFFTPoints = FourierTransform(fftPoints, numPoints, -1);
	if(checkPoints(origPoints, invertedFFTPoints, numPoints) == 0)
	{
		printf("f-(f(x)) differs from x! bad transformation!\n");
		//write(stdout, (void *) &("f-(f(x)) differs from x! bad transformation!\n"), sizeof("f-(f(x)) differs from x! bad transformation!\n"));
		printTwoPoints(origPoints, dataPoints, numPoints);
		ret = 0x3;
	}
	else
	{
		//printTwoPoints(invertedFFTPoints, dataPoints, numPoints);
		printf("transform completed succesfully.\n");
		ret = 0x10;
	}

	freeAll(fftPoints, invertedFFTPoints, origPoints, dataPoints);
	return ret;
}

//void *malloc(unsigned long x) {return (void *) 0x0;} //because sesc wraps malloc
//void free(void *x) {} //because sesc wraps malloc

/*
 *	Wrapper to Fourier Transformation
 */
point* FourierTransform(point* x, int N, int inverse)
{
	twiddle = buildTwiddle(N, inverse);
	point* fft = fastFourierTransform(x, N, 1, inverse);
	//printf("----------------------------\n");

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
 *	Recursive definition of Cooley-Tukey
 */
point* fastFourierTransform(point* x, int N, int stride, int inverse)
{
	point* X = (point *) malloc(N*sizeof(point));
	if (N == 1) {X[0] = x[0]; return X;}

	int k;

	point* even = fastFourierTransform(x, N/2, stride*2, inverse);
	point* odd = fastFourierTransform(x+stride, N/2, stride*2, inverse);

	int offset = N/2;
	for(k = 0; k < N/2; k ++)
	{
		X[k] = plus(even[k], (mult(twiddle[k*stride], odd[k])));
		X[k + offset] = minus(even[k], (mult(twiddle[k*stride], odd[k])));
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
			printf("First differ at index: %d\n", k);
			printPoint(a[k]);
			printPoint(b[k]);

			if(a[k].real != b[k].real) printf("real: %f\n", a[k].real - b[k].real);
			if(a[k].imaginary != b[k].imaginary) printf("imaginary: %f\n", a[k].imaginary - b[k].imaginary);
			return 0;
		}
	}

	return 1;
}

#include "x86_helper.c"

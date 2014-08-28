#include "serial.h"
#include "helper.c"

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


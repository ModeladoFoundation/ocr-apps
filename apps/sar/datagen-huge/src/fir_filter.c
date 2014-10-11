#include "common.h"

void fir_filter(float *x, float *h, float *y, int nh, int nr)
{
	int n, k;
	float sum;

	for(n=0; n<nr; n++)
	{
		sum = 0;
		for(k=0; k<nh; k++) {
			sum += h[k] * x[n+k];
		}
		y[n] = sum;
	}
}
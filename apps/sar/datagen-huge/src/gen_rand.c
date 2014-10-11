#include "common.h"

void gen_rand(float *x, float max, float min, float scale, int nx)
{
	int n;

	for(n=0; n<nx; n++) {
		x[n] = scale * (((float)rand() * ((max - min) / (float)RAND_MAX)) + min);
	}
}
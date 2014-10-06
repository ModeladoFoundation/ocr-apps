#include "common.h"

float sinc(float x)
{
	if(x == 0.0f) {
		return 1.0f;
	} else {
		const float arg_x = M_PI*x;
		const float sin_x = sinf(arg_x);
		return sin_x/arg_x;
	}
}

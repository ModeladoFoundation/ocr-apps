#include "common.h"

float sinc(float x)
{
#if RAG_PURE_FLOAT
	const float flt_m_pi = (float)M_PI;
	if( x == 0.0f ) {
		return 1.0f;
	} else {
		const float flt_arg_x = flt_m_pi*x;
		const float flt_sine_x = sinf(flt_arg_x);
		return flt_sine_x/flt_arg_x;
	}
#else
	if(x == 0.0) {
		return 1.0;
	}
	else {
		return sinf(M_PI*x)/(M_PI*x);
	}
#endif
}

#include "ocr.h"
#include "rag_ocr.h"
#include "common.h"

float sinc(float x)
{
#ifdef RAG_PURE_FLOAT
	if( x == 0.0f ) {
		return 1.0f;
	} else {
		const float flt_arg_x = ((float)M_PI)*x;
		return sinf(flt_arg_x)/flt_arg_x;
	}
#else
	if(x == 0.0) {
		return 1.0;
	}
	else {
		const double dbl_arg_x = M_PI*x;
		return sinf(dbl_arg_x)/(dbl_arg_x);
	}
#endif
}

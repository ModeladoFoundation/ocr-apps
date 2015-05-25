#ifndef _CNCOCR_CHOLESKY_COMMON_H_
#define _CNCOCR_CHOLESKY_COMMON_H_

#if CNCOCR_x86
#include <sys/time.h>
#else
struct timeval { u64 t; };
#endif

#endif /*_CNCOCR_CHOLESKY_COMMON_H_*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "RAG.h"
#include "FP.h"

#if CILK
extern "C" {
#endif

/* double */

#if defined(FSIM) || defined(OCR)

#include <math.h>

double __ieee754_sqrt(double arg);

double SQRT(double arg) { return __ieee754_sqrt(arg); }

double CBRT(double arg) { return cbrt(arg); }

double FABS(double arg) {
  if(arg < 0) {
   return -arg;
  } else {
   return arg;
  };
}

double FMAX(double arg1, double arg2) {
  if ( arg1 <= arg2 ) {
    return (arg2);        // both ordered and arg2 largest
  } else if ( arg1 >= arg1 ) {
    return (arg1);        // both ordered and arg1 largest
  };
  return (arg1+arg2);     // unordered so propagate a NaN
}

double FMIN(double arg1, double arg2) {
  if ( arg1 <= arg2 ) {
    return (arg1);        // both ordered and arg1 smallest
  } else if ( arg2 >= arg1 ) {
    return (arg2);        // both ordered and arg2 smallest
  };
  return (arg1+arg2);     // unordered so propagate a NaN
}

#endif

#if CILK
} // extern "C"
#endif

#ifdef CILK
extern "C" {
#endif

/* double */

#if defined(FSIM) || defined(OCR)

double SQRT(double arg);
double CBRT(double arg);
double FABS(double arg);
double FMAX(double arg1, double arg2);
double FMIN(double arg1, double arg2);

#else // NOT FSIM or OCR

#include <math.h>

#define SQRT(arg) sqrt(arg)
#define CBRT(arg) cbrt(arg)
#define FABS(arg) fabs(arg)
#define FMAX(arg1,arg2) fmax((arg1),(arg2))
#define FMIN(arg1,arg2) fmin((arg1),(arg2))

#endif // FSIM or OCR

#ifdef CILK
} // extern "C"
#endif

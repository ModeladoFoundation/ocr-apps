#ifndef INCLUSION_HELLOWORLD_H
#define INCLUSION_HELLOWORLD_H

#include "../../tools/app_ocr_err_util.h"

Err_t setupTheParabola(int * io_parabola);
Err_t solveForRoots(int * in_parabola, double * o_roots);
Err_t deduplicateRoots(double * io_roots);
Err_t printRoots(double * io_roots);

#endif

/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

//#define BLAS_TYPE__single
#define BLAS_TYPE__double
//#define BLAS_TYPE__complex
//#define BLAS_TYPE__complex_double

#include "test_gemm.c"
#include "test_getrf.c"
#include "test_gesv.c"
#include "test_potrf.c"
#include "test_posv.c"
#include "gemm.c"
#include "trsm.c"
#include "gesv.c"
#include "getrf.c"
#include "getf2.c"
#include "getrs.c"
#include "posv.c"
#include "potrf.c"
#include "potf2.c"
#include "potrs.c"
#include "syrk.c"
#include "typedutils.c"


/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

//#define TEST ctest_gemm
//#define TEST ztest_getrf
//#define TEST ctest_gesv
//#define TEST ztest_potrf
//#define TEST stest_posv
#define TEST ztest_spmmm

#include "ocr.h"
#include "extensions/ocr-legacy.h"
#include <stdio.h>
#include <stdlib.h>

void TEST();

ocrGuid_t   legacyContext;

int main(int argc, char * argv[]) {

    ocrConfig_t ocrConfig;
    ocrConfig.userArgv = argv;
    ocrConfig.iniFile = getenv("OCR_CONFIG");
    ocrLegacyInit (&legacyContext, &ocrConfig);  // Init the runtime to support calling the OCR topology for spmmm.
    TEST();
printf ("line = %ld TEST returned to test_blas.c\n", (u64) __LINE__);
    ocrLegacyFinalize(legacyContext, false);
printf ("line = %ld TEST back from ocrLegacyFinalize.  main functiond ends, returning zero\n", (u64) __LINE__);
    return 0;
}
#undef TEST

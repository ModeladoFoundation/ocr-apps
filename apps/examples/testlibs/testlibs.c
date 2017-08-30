#include "ocr.h"
#include <math.h>
#include <stdio.h>
#include <complex.h>
#include <string.h>
#include <stdlib.h>

static int
cmpint(const void *p1, const void *p2)
{
    if(*(int*)p1 < *(int*)p2) return -1;
    if(*(int*)p1 == *(int*)p2) return 0; else return 1;
}

void qsort_test(void)
{
    int i;
    int array[] = {3, 6, 2, 7, 8, 5, 9, 1, 4, 6};

    qsort(array, sizeof(array)/sizeof(int), sizeof(int), cmpint);

    for(i = 0; i<sizeof(array)/sizeof(int); i++) ocrPrintf("%d ", array[i]);
    ocrPrintf("\n");
}

ocrGuid_t mainEdt ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {

    ocrPrintf("Testing creal, atan, cimag, cexp\n");

    double pi = 4*atan(1.0);
    double complex z = cexp(I * pi);

    ocrPrintf("%f +i%f\n", creal(z), cimag(z));

    ocrPrintf("Testing erfc %f\n", erfc(1.0));

    ocrPrintf("Testing sqrt %f\n", sqrt(2.0));

    ocrPrintf("Testing exp %f\n", exp(2.0));

    ocrPrintf("Testing sin %f\n", sin(pi/4));

    ocrPrintf("Testing cos %f\n", cos(pi/4));

    ocrPrintf("Testing ceil %f\n", ceil(1.2));

    ocrPrintf("Testing floor %f\n", floor(1.2));

    char str1[32] = "hello";
    char str2[] = "world";

    ocrPrintf("Testing strcat %s %s ", str1, str2);
    ocrPrintf(" %s\n", strcat(str1, str2));

    ocrPrintf("Testing strlen of %s is %d\n", str1, strlen(str1));

    ocrPrintf("Testing strcmp of %s and %s is %d\n", str1, str2, strcmp(str1, str2));
    ocrPrintf("Testing strcmp of %s and %s is %d\n", str1, str1, strcmp(str1, str1));
    ocrPrintf("Testing strcmp of %s and %s is %d\n", str2, str1, strcmp(str2, str1));

    ocrPrintf("Testing strcpy of %s and %s is ", str1, str2);
    ocrPrintf("%s\n", strcpy(str1, str2));

    char str[] = "hello 42 world 1.0";
    int ival;
    float fval;
    sscanf(str, "%s %d %s %f", (char *)&str1, &ival, (char *)&str2, &fval);
    ocrPrintf("Testing sscanf on %s\n", str);
    ocrPrintf("%s %d %s %f\n", str1, ival, str2, fval);


    ocrPrintf("Testing qsort\n");
    qsort_test();


    ocrPrintf("Testing complete, shutting down\n");
    ocrShutdown();
    return NULL_GUID;
}

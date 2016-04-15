/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */
/* Based on stream benchmark at https://www.cs.virginia.edu/stream/FTP/Code/stream.c */

#define _GNU_SOURCE
#include "ocr.h"
#include <stdlib.h>
#include <stdio.h>

# include <math.h>
# include <float.h>
#include "pthread.h"

#define STREAM_ARRAY_SIZE 9000000
#define NUM_THREADS  32

#define FLAGS DB_PROP_NONE
#define PROPERTIES EDT_PROP_NONE

# ifndef MIN
# define MIN(x,y) ((x)<(y)?(x):(y))
# endif
# ifndef MAX
# define MAX(x,y) ((x)>(y)?(x):(y))
# endif

# define HLINE "-------------------------------------------------------------\n"

#ifndef STREAM_TYPE
#define STREAM_TYPE double
#endif

#define MOD NUM_THREADS
#define NTIMES 1000

#define OFFSET 0

#define PER_THREAD_SIZE (STREAM_ARRAY_SIZE/NUM_THREADS)

STREAM_TYPE scalar = 3.0;

#include <sys/time.h>

double mysecond()
{
        struct timeval tp;
        struct timezone tzp;
        int i;

        i = gettimeofday(&tp,&tzp);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

void checkSTREAMresults (void);
void preamble(void);
double times[NUM_THREADS][NTIMES];
ocrGuid_t tmp_copy, tmp_scale, tmp_add, tmp_triad, tmp_loop, tmp_finalize;
ocrGuid_t edt_finalize;
ocrGuid_t evt_finalize[NUM_THREADS];

ocrGuid_t copy(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    double *a = (double *)depv[0].ptr;
    double *c = (double *)depv[1].ptr;
    u32 i;

    for(i = 0; i<PER_THREAD_SIZE; i++) c[i] = a[i];
    return depv[1].guid;
}

ocrGuid_t scale(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    double *a = (double *)depv[0].ptr;
    double *b = (double *)depv[1].ptr;
    u32 i;

    for(i = 0; i<PER_THREAD_SIZE; i++) b[i] = scalar*a[i];

    return depv[1].guid;
}

ocrGuid_t add(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    double *a = (double *)depv[0].ptr;
    double *b = (double *)depv[1].ptr;
    double *c = (double *)depv[2].ptr;
    u32 i;

    for(i = 0; i<PER_THREAD_SIZE; i++) c[i] = a[i] + b[i];

    return depv[2].guid;
}

ocrGuid_t triad(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
    double *a = (double *)depv[0].ptr;
    double *b = (double *)depv[1].ptr;
    double *c = (double *)depv[2].ptr;
    u32 i;

    for(i = 0; i<PER_THREAD_SIZE; i++) a[i] = b[i] + scalar*c[i];

    times[paramv[1]][paramv[0]] = mysecond() - times[paramv[1]][paramv[0]];
    return depv[0].guid;
}

ocrGuid_t finalize(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{
  double *ptr = (double *)depv[0].ptr;
  printTimes();
  ocrShutdown();
  return NULL_GUID;
}

ocrGuid_t loop(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{

  ocrGuid_t db_a = depv[0].guid;
  ocrGuid_t db_b = depv[1].guid;
  ocrGuid_t db_c = depv[2].guid;
  u64 iter = paramv[0];
  u64 param[2] = { iter, paramv[1] };

  times[paramv[1]][iter] = mysecond();
  ocrGuid_t copy_output, scale_output, add_output, triad_output;
  ocrGuid_t edt_copy, edt_scale, edt_add, edt_triad;

  ocrEdtCreate(&edt_copy, tmp_copy, EDT_PARAM_DEF, param, EDT_PARAM_DEF, NULL, PROPERTIES, NULL_HINT, &copy_output);
  ocrEdtCreate(&edt_scale, tmp_scale, EDT_PARAM_DEF, param, EDT_PARAM_DEF, NULL, PROPERTIES, NULL_HINT, &scale_output);
  ocrEdtCreate(&edt_add, tmp_add, EDT_PARAM_DEF, param, EDT_PARAM_DEF, NULL, PROPERTIES, NULL_HINT, &add_output);
  ocrEdtCreate(&edt_triad, tmp_triad, EDT_PARAM_DEF, param, EDT_PARAM_DEF, NULL, PROPERTIES, NULL_HINT, &triad_output);

  ocrAddDependence(db_a, edt_triad, 0, DB_MODE_RW);
  ocrAddDependence(add_output, edt_triad, 1, DB_MODE_RO);
  ocrAddDependence(scale_output, edt_triad, 2, DB_MODE_RO);

  ocrAddDependence(db_a, edt_add, 0, DB_MODE_RO);
  ocrAddDependence(scale_output, edt_add, 1, DB_MODE_RO);
  ocrAddDependence(copy_output, edt_add, 2, DB_MODE_RW);

  ocrAddDependence(db_a, edt_scale, 0, DB_MODE_RO);
  ocrAddDependence(db_b, edt_scale, 1, DB_MODE_RW);

  ocrAddDependence(db_a, edt_copy, 0, DB_MODE_RO);
  ocrAddDependence(db_c, edt_copy, 1, DB_MODE_RW);


  iter++;
  param[0] = iter;
  if (iter < NTIMES) {  // Spawn another set
    ocrGuid_t edt_loop;
    ocrEdtCreate(&edt_loop, tmp_loop, EDT_PARAM_DEF, param, EDT_PARAM_DEF, NULL, PROPERTIES, NULL_HINT, NULL);

    ocrAddDependence(triad_output, edt_loop, 0, DB_MODE_RW);
    ocrAddDependence(scale_output, edt_loop, 1, DB_MODE_RW);
    ocrAddDependence(add_output, edt_loop, 2, DB_MODE_RW);
  } else {
      u64 tid = paramv[1];
      ocrAddDependence(triad_output, evt_finalize[tid], 0, DB_DEFAULT_MODE);
  }

  return NULL_GUID;

}

ocrGuid_t mainLet(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    // Create DBs
    ocrGuid_t db_a, db_b, db_c;

    double *a;
    double *b;
    double *c;
    u32 i;
    ocrGuid_t edt_loop, output_evt;

    ocrDbCreate(&db_a, (void **)&a, sizeof(double)*PER_THREAD_SIZE,
                             FLAGS, NULL_HINT, NO_ALLOC);

    ocrDbCreate(&db_b, (void **)&b, sizeof(double)*PER_THREAD_SIZE,
                             FLAGS, NULL_HINT, NO_ALLOC);

    ocrDbCreate(&db_c, (void **)&c, sizeof(double)*PER_THREAD_SIZE,
                             FLAGS, NULL_HINT, NO_ALLOC);

    // Init them
    for(i = 0; i<PER_THREAD_SIZE; i++) {
        a[i] = 2.0;
        b[i] = 2.0;
        c[i] = 0.0;
    }

    u64 param[2] = { 0, paramv[0] };
    ocrEdtCreate(&edt_loop, tmp_loop, EDT_PARAM_DEF, param, EDT_PARAM_DEF, NULL, EDT_PROP_FINISH, NULL_HINT, &output_evt);

    ocrAddDependence(db_a, edt_loop, 0, DB_MODE_RW);
    ocrAddDependence(db_b, edt_loop, 1, DB_MODE_RW);
    ocrAddDependence(db_c, edt_loop, 2, DB_MODE_RW);

    return output_evt;
}

ocrGuid_t mainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[])
{

    preamble();

    // Create NUM_THREADS sets of datablocks (3/each)
    // Initialize them
    // Spawn the following tasks in a loop
    // 1. Copy
    // 2. Scale
    // 3. Add
    // 4. Triad

    ocrGuid_t tmp_mainLet;
    ocrGuid_t edt_mainLet;
    u64 i;
    ocrHint_t hint_disperse;

    if(ocrHintInit(&hint_disperse, OCR_HINT_EDT_T )) PRINTF("Error initializing hint\n");
    if(ocrSetHintValue(&hint_disperse, OCR_HINT_EDT_DISPERSE, OCR_HINT_EDT_DISPERSE_NEAR)) PRINTF("Error setting hint\n");

    ocrEdtTemplateCreate(&tmp_mainLet, mainLet, 1, 0);

    // Spawn the threads
    ocrEdtTemplateCreate(&tmp_loop, loop, 2, 3);
    ocrEdtTemplateCreate(&tmp_copy, copy, 2, 2);
    ocrEdtTemplateCreate(&tmp_scale, scale, 2, 2);
    ocrEdtTemplateCreate(&tmp_add, add, 2, 3);
    ocrEdtTemplateCreate(&tmp_triad, triad, 2, 3);

    ocrEdtTemplateCreate(&tmp_finalize, finalize, 0, NUM_THREADS);
    ocrEdtCreate(&edt_finalize, tmp_finalize, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, PROPERTIES, NULL_HINT, NULL);

    for(i = 0; i<NUM_THREADS; i++) {
        ocrEventCreate(&evt_finalize[i], OCR_EVENT_ONCE_T, true);
        ocrAddDependence(evt_finalize[i], edt_finalize, i, DB_MODE_RO);
        ocrEdtCreate(&edt_mainLet, tmp_mainLet, EDT_PARAM_DEF, &i, EDT_PARAM_DEF, NULL, PROPERTIES, &hint_disperse, NULL);
    }

    return NULL_GUID;
}

void preamble(void)
{
    int                 BytesPerWord;
    int j;
    double t;

    /* --- SETUP --- determine precision and check timing --- */

    PRINTF(HLINE);
    PRINTF("STREAM version $Revision: 5.10 $\n");
    PRINTF(HLINE);
    BytesPerWord = sizeof(STREAM_TYPE);
    PRINTF("This system uses %d bytes per array element.\n",
        BytesPerWord);

    PRINTF(HLINE);
#ifdef N
    PRINTF("*****  WARNING: ******\n");
    PRINTF("      It appears that you set the preprocessor variable N when compiling this code.\n");
    PRINTF("      This version of the code uses the preprocesor variable STREAM_ARRAY_SIZE to control the array size\n");
    PRINTF("      Reverting to default value of STREAM_ARRAY_SIZE=%llu\n",(unsigned long long) STREAM_ARRAY_SIZE);
    PRINTF("*****  WARNING: ******\n");
#endif

    PRINTF("Array size = %llu (elements), Offset = %d (elements)\n" , (unsigned long long) STREAM_ARRAY_SIZE, OFFSET);
    PRINTF("Memory per array = %.1f MiB (= %.1f GiB).\n",
        BytesPerWord * ( (double) STREAM_ARRAY_SIZE / 1024.0/1024.0),
        BytesPerWord * ( (double) STREAM_ARRAY_SIZE / 1024.0/1024.0/1024.0));
    PRINTF("Total memory required = %.1f MiB (= %.1f GiB).\n",
        (3.0 * BytesPerWord) * ( (double) STREAM_ARRAY_SIZE / 1024.0/1024.),
        (3.0 * BytesPerWord) * ( (double) STREAM_ARRAY_SIZE / 1024.0/1024./1024.));
    PRINTF("Each kernel will be executed %d times.\n", NTIMES);
    PRINTF(" The *best* time for each kernel (excluding the first iteration)\n");
    PRINTF(" will be used to compute the reported bandwidth.\n");

    PRINTF(HLINE);

}

int printTimes(void)
{
    double avgtime = 0.0;
    double mintime = FLT_MAX;
    double maxtime = 0.0;
    int j, k;

    for (k=1; k<NTIMES; k++) /* note -- skip first iteration */
        {
        for (j=0; j<NUM_THREADS; j++)
            {
            avgtime = avgtime + times[j][k];
            mintime = MIN(mintime, times[j][k]);
            maxtime = MAX(maxtime, times[j][k]);
            }
        }

    PRINTF(HLINE);
    PRINTF("%f MB/s %f %f %f\n", 1.0E-06*10*sizeof(STREAM_TYPE)*PER_THREAD_SIZE/mintime, avgtime/((NTIMES-1)*NUM_THREADS), mintime, maxtime);
    PRINTF(HLINE);

    return 0;

}

/* A gettimeofday routine to give access to the wall
 *    clock timer on most UNIX-like systems.  */
#if 0
#include <sys/time.h>

#ifndef abs
#define abs(a) ((a) >= 0 ? (a) : -(a))
#endif
void checkSTREAMresults ()
{
        STREAM_TYPE aj,bj,cj,scalar;
        STREAM_TYPE aSumErr,bSumErr,cSumErr;
        STREAM_TYPE aAvgErr,bAvgErr,cAvgErr;
        double epsilon;
        ssize_t j;
        int     k,ierr,err;

    /* reproduce initialization */
        aj = 1.0;
        bj = 2.0;
        cj = 0.0;
    /* a[] is modified during timing check */
        aj = 2.0E0 * aj;
    /* now execute timing loop */
        scalar = 3.0;
        for (k=0; k<NTIMES; k++)
        {
            cj = aj;
            bj = scalar*cj;
            cj = aj+bj;
            aj = bj+scalar*cj;
        }

    /* accumulate deltas between observed and expected results */
        aSumErr = 0.0;
        bSumErr = 0.0;
        cSumErr = 0.0;
        for (j=0; j<STREAM_ARRAY_SIZE; j++) {
                aSumErr += abs(a[j] - aj);
                bSumErr += abs(b[j] - bj);
                cSumErr += abs(c[j] - cj);
        }
        aAvgErr = aSumErr / (STREAM_TYPE) STREAM_ARRAY_SIZE;
        bAvgErr = bSumErr / (STREAM_TYPE) STREAM_ARRAY_SIZE;
        cAvgErr = cSumErr / (STREAM_TYPE) STREAM_ARRAY_SIZE;

        if (sizeof(STREAM_TYPE) == 4) {
                epsilon = 1.e-6;
        }
        else if (sizeof(STREAM_TYPE) == 8) {
                epsilon = 1.e-13;
        }
        else {
                PRINTF("WEIRD: sizeof(STREAM_TYPE) = %lu\n",sizeof(STREAM_TYPE));
                epsilon = 1.e-6;
        }

        err = 0;
        if (abs(aAvgErr/aj) > epsilon) {
                err++;
                PRINTF ("Failed Validation on array a[], AvgRelAbsErr > epsilon (%e)\n",epsilon);
                PRINTF ("     Expected Value: %e, AvgAbsErr: %e, AvgRelAbsErr: %e\n",aj,aAvgErr,abs(aAvgErr)/aj);
                ierr = 0;
                for (j=0; j<STREAM_ARRAY_SIZE; j++) {
                        if (abs(a[j]/aj-1.0) > epsilon) {
                                ierr++;
#ifdef VERBOSE
                                if (ierr < 10) {
                                        PRINTF("         array a: index: %ld, expected: %e, observed: %e, relative error: %e\n",
                                                j,aj,a[j],abs((aj-a[j])/aAvgErr));
                                }
#endif
                        }
                }
                PRINTF("     For array a[], %d errors were found.\n",ierr);
        }
        if (abs(bAvgErr/bj) > epsilon) {
                err++;
                PRINTF ("Failed Validation on array b[], AvgRelAbsErr > epsilon (%e)\n",epsilon);
                PRINTF ("     Expected Value: %e, AvgAbsErr: %e, AvgRelAbsErr: %e\n",bj,bAvgErr,abs(bAvgErr)/bj);
                PRINTF ("     AvgRelAbsErr > Epsilon (%e)\n",epsilon);
                ierr = 0;
                for (j=0; j<STREAM_ARRAY_SIZE; j++) {
                        if (abs(b[j]/bj-1.0) > epsilon) {
                                ierr++;
#ifdef VERBOSE
                                if (ierr < 10) {
                                        PRINTF("         array b: index: %ld, expected: %e, observed: %e, relative error: %e\n",
                                                j,bj,b[j],abs((bj-b[j])/bAvgErr));
                                }
#endif
                        }
                }
                PRINTF("     For array b[], %d errors were found.\n",ierr);
        }
        if (abs(cAvgErr/cj) > epsilon) {
                err++;
                PRINTF ("Failed Validation on array c[], AvgRelAbsErr > epsilon (%e)\n",epsilon);
                PRINTF ("     Expected Value: %e, AvgAbsErr: %e, AvgRelAbsErr: %e\n",cj,cAvgErr,abs(cAvgErr)/cj);
                PRINTF ("     AvgRelAbsErr > Epsilon (%e)\n",epsilon);
                ierr = 0;
                for (j=0; j<STREAM_ARRAY_SIZE; j++) {
                        if (abs(c[j]/cj-1.0) > epsilon) {
                                ierr++;
#ifdef VERBOSE
                                if (ierr < 10) {
                                        PRINTF("         array c: index: %ld, expected: %e, observed: %e, relative error: %e\n",
                                                j,cj,c[j],abs((cj-c[j])/cAvgErr));
                                }
#endif
                        }
                }
                PRINTF("     For array c[], %d errors were found.\n",ierr);
        }
        if (err == 0) {
                PRINTF ("Solution Validates: avg error less than %e on all three arrays\n",epsilon);
        }
#ifdef VERBOSE
        PRINTF ("Results Validation Verbose Results: \n");
        PRINTF ("    Expected a(1), b(1), c(1): %f %f %f \n",aj,bj,cj);
        PRINTF ("    Observed a(1), b(1), c(1): %f %f %f \n",a[1],b[1],c[1]);
        PRINTF ("    Rel Errors on a, b, c:     %e %e %e \n",abs(aAvgErr/aj),abs(bAvgErr/bj),abs(cAvgErr/cj));
#endif
}
#endif

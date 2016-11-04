/*
Author Jesmin Jahan Tithi
Copyright Intel Corporation 2016
*/
//export OCR_ASAN=yes
#define ENABLE_EXTENSION_LABELING

#include "ocr.h"
#include "extensions/ocr-labeling.h" //currently needed for labeled guids
#include "extensions/ocr-affinity.h" //needed for affinity


#ifdef USE_PROFILER
#include "extensions/ocr-profiler.h"
#endif
#include <stdio.h>
#include "string.h"
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#define PRINT
#define CHECK_RESULTS

// User defined libraries
#include "cilktime.h" // timing library

#define MAX(a, b) ((a>b)?a:b)
#define GAP_PENALTY 0 //for LCS 0, for edit distance 1 and use min, for Smith waterman depends on the blosum matrix

typedef struct {
      long N, base, xi, xj, n;  // input size, top-left corner index, and size of the current
	  ocrGuid_t s_guid, t_guid, score_guid; // s_label, t_label, score_matrix_labels
} LCS_task_params;

typedef struct {
      long N, xi, xj, n;  // input and base case size, top-left corner index
} LCS_base_params;


void serial_lcs(int* X, int *S, int *T, long int xi, long int xj, long int n){
    long int N = n;
    long int Nplus1 = N + 1;

    long int endi = n;//not needed
    long int endj = n;//not needed

    // Diagonal by diagonal computation.


    // First half of the diagonals
    for (int t = 0; t < endi; t++)
    {
        for(int i = 0; i <= t;i++)
        {
            int j = t - i;
            long int idx = (N+(xj+j-xi-i));
            //Optimize
            int del = X[idx - 1];
            int ins = X[idx + 1];
            int del_ins = (del > ins) ? (del + GAP_PENALTY):(ins + GAP_PENALTY);
            int match = X[idx] + (S[xi+i] != T[xj+j]);
            X[idx]= (match> del_ins)?match:del_ins;

        }
    }

	int n_minus1 = endi - 1;
	int nx2 = n_minus1 + endi;

    // Second half of the diagonals
    for (int t = endi; t < nx2; t++) {
        for(int i = n_minus1; i >= (t - n_minus1);i--)
        {
            int j = t - i;
            long int idx = (N+(xj+j-xi-i));
            //Optimize
            int del = X[idx - 1];
            int ins = X[idx + 1];
            int del_ins = (del > ins) ? (del + GAP_PENALTY):(ins + GAP_PENALTY);
            int match = X[idx] + (S[xi+i] != T[xj+j]);
            X[idx]= (match> del_ins)?match:del_ins;
        }
    }

    return;
}

//base case kernel
void inline serial_lcs_1D(long* X, int *S, int *T, long N, long int xi, long int xj, long int n){
    long Nplus1 = N + 1;
    long int endi = (xi + n > N) ? (Nplus1  - xi) : n;//not needed
    long int endj = (xj + n > N) ? (Nplus1  - xj) : n;//not needed

    // Diagonal by diagonal computation.



    // First half of the diagonals
    for (int t = 0; t < endi; t++)
    {
        for(int i = 0; i <= t;i++)
        {
            int j = t - i;
            long int idx = (N+(xj+j-xi-i));
            //Optimize
            int del = X[idx - 1];
            int ins = X[idx + 1];
            int del_ins = (del > ins) ? (del + GAP_PENALTY):(ins + GAP_PENALTY);
            int match = X[idx] + (S[xi+i] != T[xj+j]);
            X[idx]= MAX(match, del_ins);

        }

    }
    // Second half of the diagonals


    int n_minus1 = endi - 1;
	int nx2 = n_minus1 + endi;
    for (int t = endi; t < nx2; t++) {
        for(int i = n_minus1; i >= (t - n_minus1);i--)
        {
            int j = t - i;
            long int idx = (N+(xj+j-xi-i));
            //Optimize
            int del = X[idx - 1];
            int ins = X[idx + 1];
            int del_ins = (del > ins) ? (del + GAP_PENALTY):(ins + GAP_PENALTY);
            int match = X[idx] + (S[xi+i] != T[xj+j]);
            X[idx]= MAX(match, del_ins);
        }
    }
}

// OCR libraries


ocrGuid_t seqLCSEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){
#ifdef USE_PROFILER
   START_PROFILE(seqLCSEdt_profile);
#endif
   //Take the passcoresed arg and convert it back to the original type
   LCS_base_params* p = (LCS_base_params *)paramv;

   int *S = depv[0].ptr;
   int *T = depv[1].ptr;


   long *score = depv[2].ptr;
    // Call the base task
   serial_lcs_1D(score, S, T, p->N, p->xi, p->xj, p->n);
#ifdef USE_PROFILER
RETURN_PROFILE(NULL_GUID);
#endif
   return NULL_GUID;
}
ocrGuid_t recLCSEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]){

	// Cast the parameter to the original parameter format.
    LCS_task_params* p = (LCS_task_params *)paramv;

	long base = p->base;
    long n = p->n;

    if(n<=base)
    {

		//PRINTF("Calling basecase N: %lu, base: %lu, xi: %lu, xj: %lu, n: %lu \n", p->N, base, p->xi, p->xj, p->n);

		// First get the data blocks you need by using their guids
        //inline serial_lcs_1D(long* X, int *S, int *T, long int xi, long int xj, long int n)
	    //serial_lcs_1D(X, S, T, p->N, p->xi, p->xj, n);

		LCS_base_params p1;

	    // you can just pass n
		p1.N = p->N;
		p1.xi = p->xi;
		p1.xj = p->xj;
		p1.n = n;

		// How to I access the pointers S, T and score?
		// Question: Can I create a global template?
		// Can I have global variables?


		// Create the task template
		ocrGuid_t base_template;
		ocrEdtTemplateCreate(&base_template, seqLCSEdt, sizeof(p1)/sizeof(u64) , 3 );

		// Create the task
		ocrGuid_t baseEdt, baseoutputEventGuidOnce;
		ocrEventCreate(&baseEdt, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

		ocrEdtCreate(&baseEdt,
					 base_template,
					 EDT_PARAM_DEF,
					 (u64*) &p1,
					 EDT_PARAM_DEF,
					 NULL,
					 EDT_PROP_NONE,
					 NULL_HINT,
					 &baseoutputEventGuidOnce);

		ocrAddDependence( p->s_guid, baseEdt, 0, DB_MODE_RO );
		ocrAddDependence( p->t_guid, baseEdt, 1, DB_MODE_RO );

		//////////////////////////////////////////////////
		ocrAddDependence( p->score_guid, baseEdt, 2, DB_MODE_RW );

		return NULL_GUID;
		return NULL_GUID;

    }
    else
    {

        long nn = n >> 1;

		//PRINTF("Working on size: %lu\n", nn);

		//---------------------------Call for x11-------------------------------------------
		LCS_task_params p1;

		p1.N = p->N;
		p1.base = base;
		p1.xi = p->xi;
		p1.xj = p->xj;
		p1.n = nn;
		p1.s_guid = p->s_guid;
		p1.t_guid = p->t_guid;
		p1.score_guid = p->score_guid;

		// Create the task template
		ocrGuid_t LCSGuidTmp_X11;
		ocrEdtTemplateCreate(&LCSGuidTmp_X11, recLCSEdt, sizeof(p1)/sizeof(u64) , 1);

		// Create the task
		ocrGuid_t x11Edt, x11outputEventGuid, x11outputEventGuidOnce;
		ocrEventCreate(&x11outputEventGuid, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

		ocrEdtCreate(&x11Edt,
					  LCSGuidTmp_X11,
					  EDT_PARAM_DEF,
					  (u64*) &p1,
					  EDT_PARAM_DEF,
					  NULL,
					  EDT_PROP_FINISH,
					  NULL_HINT,
					  &x11outputEventGuidOnce);
		ocrAddDependence( x11outputEventGuidOnce, x11outputEventGuid, 0, DB_MODE_NULL );
		ocrAddDependence( NULL_GUID, x11Edt, 0, DB_MODE_NULL );

		//---------------------------Call for x12-------------------------------------------
        ocrGuid_t LCSGuidTmp_X12_x21;
		ocrEdtTemplateCreate(&LCSGuidTmp_X12_x21, recLCSEdt, sizeof(p1)/sizeof(u64) , 1 );

		ocrGuid_t x12Edt, x12outputEventGuid, x12outputEventGuidOnce;
		ocrEventCreate(&x12outputEventGuid, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

	    p1.xi = p->xi + nn, p1.xj = p->xj;

		ocrEdtCreate(&x12Edt,
					  LCSGuidTmp_X12_x21,
					  EDT_PARAM_DEF,
					  (u64*) &p1,
					  EDT_PARAM_DEF,
					  &x11outputEventGuid,
					  EDT_PROP_FINISH,
					  NULL_HINT,
					  &x12outputEventGuidOnce);

	    ocrAddDependence( x12outputEventGuidOnce, x12outputEventGuid, 0, DB_MODE_NULL );

		//---------------------------Call for x21-------------------------------------------
		ocrGuid_t x21Edt, x21outputEventGuid, x21outputEventGuidOnce;
		ocrEventCreate(&x21outputEventGuid, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

	    p1.xi = p->xi, p1.xj = p->xj+nn;

		ocrEdtCreate(&x21Edt,
					  LCSGuidTmp_X12_x21,
					  EDT_PARAM_DEF,
					  (u64*) &p1,
					  EDT_PARAM_DEF,
					  &x11outputEventGuid,
					  EDT_PROP_FINISH,
					  NULL_HINT,
					  &x21outputEventGuidOnce);

		ocrAddDependence( x21outputEventGuidOnce, x21outputEventGuid, 0, DB_MODE_NULL );

		//---------------------------Call for x22-------------------------------------------

		ocrGuid_t LCSGuidTmp_X22;
		ocrEdtTemplateCreate(&LCSGuidTmp_X22, recLCSEdt, sizeof(p1)/sizeof(u64) , 2 );

		ocrGuid_t x22Edt, x22outputEventGuid;
		ocrEventCreate(&x22outputEventGuid, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

	    p1.xi = p->xi + nn, p1.xj = p->xj+nn;

		ocrEdtCreate(&x22Edt,
					  LCSGuidTmp_X22,
					  EDT_PARAM_DEF,
					  (u64*) &p1,
					  EDT_PARAM_DEF,
					  NULL,
					  EDT_PROP_FINISH,
					  NULL_HINT,
					  &x22outputEventGuid);


		ocrAddDependence(x21outputEventGuid, x22Edt, 0, DB_DEFAULT_MODE);
		ocrAddDependence(x12outputEventGuid, x22Edt, 1, DB_DEFAULT_MODE);

    }
	return NULL_GUID;
}



ocrGuid_t shutDownEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    long *score = depv[1].ptr;
	long true_value = (long)paramv[1];

	long end = cilk_getticks();
	PRINTF("runtime: %f\n",cilk_ticks_to_seconds(end-(long)paramv[2]));

	assert(true_value==score[paramv[0]]);

    PRINTF("\nShutting down OCR runtime\n");

	ocrShutdown(); // This is the last EDT to execute, terminate
    return NULL_GUID;
}

/*Generate Random Input*/
void genRandInput(int *X, int *Y, int n)
{
    char a = 'A';
    for(int i =0; i<n ; i ++)
    {
        X[i]=rand()%4+a;
    }
    for(int i = 0; i<n ; i ++)
    {
        Y[i]=rand()%4+a;
    }
}
//----------------------------------------------------- Main EDT --------------------------//
ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
	// Collect command line arguments.
    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;
    void * PTR_cmdLineArgs = depv[0].ptr;
	u32 argc = getArgc(PTR_cmdLineArgs);

	// Input params and default values.
    long N = 1024; // string length
    long base = 256;	// basecase
	int num_workers = 16; // maximum number of workers

	if(argc<1)
	{
		PRINTF("USAGE: please run the program as follows:\n"
		     "executable input_string_length number_of_cores basecase\n");
		exit(1);
	}

	if(argc > 1)
	{
		N = (long) atol(getArgv(PTR_cmdLineArgs, 1));
	}
	if(argc > 2)
	{
		base = (long) atol(getArgv(PTR_cmdLineArgs, 2));
	}
	if(argc > 3)
	{
		num_workers = (s64) atol(getArgv(PTR_cmdLineArgs, 3));
	}


	// base can not be > N.
    if(N < base) base = N;

	// Actual allocation size


    PRINTF("\n");
    PRINTF("Running LCS.\nStrings len: %ld # workers: %d, basecase: %ld\n", N, num_workers, base);

    s64 Nplus1 = N + 1;
	#ifdef CHECK_RESULTS


	int *_S = ( int * ) malloc(Nplus1 *  sizeof( int ));
    int *_T = ( int * ) malloc(Nplus1 *  sizeof( int ));

    _S[0]=_T[0]=32;


    // Linear space array.
    int * score = ( int * ) malloc( (2 * Nplus1) *  sizeof( int ) );

	// Initialize the matrix.
    for ( int i = 0; i < Nplus1; i++ )
    {
        score[N - i] = i;
    }
    for ( int j = 0; j < Nplus1; j++ )
    {
        score[N + j] = j;
    }
	#endif


	////////////////////////Initialize score ///////////////////////

	ocrGuid_t score_guid;
	int *S, *T;
	s64 *lcs_score;

	// Create the data block with the guid
	ocrDbCreate(&score_guid,
		       (void**) &lcs_score,
				sizeof(long) * 2 * Nplus1,
				GUID_PROP_NONE,
				NULL_HINT,
				NO_ALLOC
				);

	for ( int i = 0; i < Nplus1; i++ )
    {
        lcs_score[N - i] = i;
    }
    for ( int j = 0; j < Nplus1; j++ )
    {
        lcs_score[N + j] = j;
    }

	ocrGuid_t S_guid, T_guid;

	ocrDbCreate(&S_guid,
		       (void**) &S,
				sizeof(int) * Nplus1,
				GUID_PROP_NONE,
				NULL_HINT,
				NO_ALLOC);

	ocrDbCreate(&T_guid,
		       (void**) &T,
				sizeof(int) *  Nplus1,
				GUID_PROP_NONE,
				NULL_HINT,
				NO_ALLOC);

    genRandInput(S, T, Nplus1);

	long true_value;
	#ifdef CHECK_RESULTS
	//PRINTF("\n_S: ");
	for(int i = 0; i <Nplus1; i++)
    {
	    _S[i] = S[i];
		//PRINTF("%ld ", _S[i]);

	}
    //PRINTF("\n_T: ");
	for(int i = 0; i <Nplus1; i++)
    {
	    _T[i]=T[i];
		//PRINTF("%ld ", _T[i]);

	}
	//PRINTF("\n");

	serial_lcs(score, _S, _T, 1, 1, N);

	//PRINTF("Sane value: %ld \n", score[N]);
	true_value = score[N];

	free(_S);
	free(_T);
	free(score);
	#endif

	// Print values for debug purpose
	//Create a new finish EDT
	//params are N, base, xi, xj, and the guids.

	LCS_task_params p;

	p.N = N;
	p.base = base;
	p.xi = 1;
	p.xj = 1;
	p.n = N;
	p.s_guid = S_guid;
	p.t_guid = T_guid;
	p.score_guid = score_guid;

	unsigned long start;
	start = cilk_getticks();

	// Create the task template
	ocrGuid_t recLCSGuid;
    ocrEdtTemplateCreate(&recLCSGuid, recLCSEdt, sizeof(p)/sizeof(u64) , 3 );

	// Create the task
	ocrGuid_t recLCSEdt, outputEventGuid, outputEventGuidOnce;
	ocrEventCreate(&outputEventGuid, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

    ocrEdtCreate(&recLCSEdt,
				  recLCSGuid,
				  EDT_PARAM_DEF,
				  (u64*) &p,
				  EDT_PARAM_DEF,
				  NULL,
                  EDT_PROP_FINISH,
				  NULL_HINT,
				  &outputEventGuidOnce);

	ocrAddDependence( outputEventGuidOnce, outputEventGuid, 0, DB_MODE_NULL );
	ocrAddDependence( S_guid, recLCSEdt, 0, DB_MODE_RO );
	ocrAddDependence( T_guid, recLCSEdt, 1, DB_MODE_RO );
	ocrAddDependence( score_guid, recLCSEdt, 2, DB_MODE_RW );
	// Create the task template
	ocrGuid_t shutDown_tmp;

	u64 paramv2[3];
	paramv2[0]=N;
	paramv2[1]=true_value ;
	paramv2[2]=start;
    ocrEdtTemplateCreate(&shutDown_tmp, shutDownEdt, sizeof(paramv2)/sizeof(u64) , 2);

	// Create the task
	ocrGuid_t shutDown_taskGuid;
    ocrEdtCreate(&shutDown_taskGuid,
				  shutDown_tmp,
				  EDT_PARAM_DEF,
				  (u64*) paramv2,
				  EDT_PARAM_DEF,
				  NULL,
                  EDT_PROP_NONE,
				  NULL_HINT,
				  NULL);


    ocrAddDependence(outputEventGuid, shutDown_taskGuid, 0, DB_MODE_NULL );
	ocrAddDependence(score_guid, shutDown_taskGuid, 1, DB_MODE_RO );

	return NULL_GUID;
}

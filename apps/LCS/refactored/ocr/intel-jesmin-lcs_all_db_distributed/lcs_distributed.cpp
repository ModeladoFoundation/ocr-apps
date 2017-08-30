
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

// User defined libraries
#include "cilktime.h" // timing library

//#define PRINT
//#define CHECK_RESULTS

#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)
#define GAP_PENALTY 0
//
typedef struct {
    int N, base, xi, xj, n; // input size, top-left corner index, and size of the current
    ocrGuid_t s_labels, t_labels, score_matrix_labels; // s_label, t_label, score_matrix_labels
} LCS_task_params;

typedef struct {
    int N, xi, xj, n; // input and base case size, top-left corner index
} LCS_base_params;

void serial_lcs(int* X, int* S, int* T, int xi, int xj, int n)
{
    int N = n;
    int Nplus1 = N + 1;
    // Finish computation

    // Conditions to handle non powers of 2.
    long int endi = (xi + n > N) ? (Nplus1 - xi) : n;
    long int endj = (xj + n > N) ? (Nplus1 - xj) : n;

    endi = xi + endi;
    endj = xj + endj;

    for (int i = xi; i < endi; i++) {
        for (int j = xj; j < endj; j++) {
            long int idx = (i - 1) * Nplus1;
            //Optimize
            int del = X[idx + j];
            int ins = X[idx + Nplus1 + (j - 1)];
            int del_ins = MAX(del, ins) + GAP_PENALTY;
            int match = X[idx + (j - 1)] + (S[i] != T[j]);
            X[idx + Nplus1 + j] = MIN(match, del_ins);
        }
    }
    // Copy back to original location
}

//base case kernel
void serial_lcs_1D(int* current, int* left, int* above, int* diag,
    int* S, int* T, int N, int xi, int xj, int n)
{
    int my_i_size = (xi + n > N) ? (N + 1 - xi) : n;
    int my_j_size = (xj + n > N) ? (N + 1 - xj) : n;

    int neighbor_i_size = (xi == 1) ? 1 : n;
    int neighbor_j_size = (xj == 1) ? 1 : n;

#ifdef PRINT
    ocrPrintf("Calling seq basecase: xi: %d, xj: %d, n: %d \n", xi, xj, n);
#endif
    //copy left to a seperate location.

    //CASE 1: i=0, j=0;

    int match = diag[(neighbor_i_size - 1) * neighbor_j_size
                    + (neighbor_j_size - 1)]
        + (S[0] != T[0]);
    int del = left[(neighbor_j_size - 1)];
    int ins = above[(neighbor_i_size - 1) * n];
    current[0] = MIN(match, (MAX(del, ins) + GAP_PENALTY));

    //CASE 2: i=0, j>0; both diagonal and above will come from above neigh
    int i = 0, j = 0;

    int neighbor_row = (neighbor_i_size - 1) * n;

    for (j = 1; j < my_j_size; j++) {
        int ins = above[neighbor_row + j];
        int match = above[neighbor_row + (j - 1)] + (S[0] != T[j]);
        int del = current[(j - 1)];
        current[j] = MIN(match, (MAX(del, ins) + GAP_PENALTY));
    }

    //CASE 3: i>0, j=0; both diagonal and left will come from left neigh
    j = 0;
    for (i = 1; i < my_i_size; i++) {
        int match = left[(i - 1) * neighbor_j_size + (neighbor_j_size - 1)] + (S[i] != T[0]);
        int del = left[(i)*neighbor_j_size + (neighbor_j_size - 1)];
        int ins = current[(i - 1) * my_j_size];
        current[i * my_j_size] = MIN(match, (MAX(del, ins) + GAP_PENALTY));

        //neighbor_col = neighbor_col+ neighbor_j_size;			//next row col
    }

    //CASE 4: i>0, j>0;
    for (i = 1; i < my_i_size; i++) {
        for (j = 1; j < my_j_size; j++) {

            int match = current[(i - 1) * my_j_size + (j - 1)] + (S[i] != T[j]);
            int ins = current[(i - 1) * my_j_size + j];
            int del = current[(i)*my_j_size + (j - 1)];
            current[i * my_j_size + (j)] = MIN(match, (MAX(del, ins) + GAP_PENALTY));
        }
    }
#ifdef PRINT
    for (i = 0; i < my_i_size; i++) {
        for (j = 0; j < my_j_size; j++) {
            ocrPrintf("%d ", current[i * my_j_size + (j)]);
        }
        ocrPrintf("\n");
    }
#endif
    return;
}

// OCR libraries
ocrGuid_t seqLCSEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    //Take the passcoresed arg and convert it back to the original type
    LCS_base_params* p = (LCS_base_params*)paramv;

    int* S = (int*)depv[0].ptr;
    int* T = (int*)depv[1].ptr;

    S = (p->xi == 1) ? S + 1 : S;
    T = (p->xj == 1) ? T + 1 : T;

    int* current = (int*)depv[2].ptr;
    int* left = (int*)depv[3].ptr;
    int* above = (int*)depv[4].ptr;
    int* diag = (int*)depv[5].ptr;

    serial_lcs_1D(current, left, above, diag, S, T, p->N, p->xi, p->xj, p->n);

    return NULL_GUID;
}
ocrGuid_t recLCSEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{

    // Cast the parameter to the original parameter format.
    LCS_task_params* p = (LCS_task_params*)paramv;

    int base = p->base;
    int n = p->n;

    if (n <= base) {
        //Setup data dependency

        LCS_base_params p1;
        p1.N = p->N;
        p1.xi = p->xi;
        p1.xj = p->xj;
        p1.n = n;

        // Create the task template
        ocrGuid_t base_template;
        ocrEdtTemplateCreate(&base_template, seqLCSEdt, sizeof(LCS_base_params) / sizeof(u64), 6);

        ocrGuid_t baseEdt, baseoutputEventGuidOnce;
        ocrEventCreate(&baseEdt, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

        ocrEdtCreate(&baseEdt,
            base_template,
            EDT_PARAM_DEF,
            (u64*)&p1,
            EDT_PARAM_DEF,
            NULL,
            EDT_PROP_NONE,
            NULL_HINT,
            &baseoutputEventGuidOnce);

        ocrGuid_t S, T;

        // Optimization effort.

        ocrGuidFromIndex(&S, p->s_labels, (p->xi - 1) / base);
        ocrGuidFromIndex(&T, p->t_labels, (p->xj - 1) / base);

        ocrAddDependence(S, baseEdt, 0, DB_MODE_RO);
        ocrAddDependence(T, baseEdt, 1, DB_MODE_RO);

        int block_i_end = (p->xi - 1) / base + 1;
        int block_j_end = (p->xj - 1) / base + 1;
        int block_grid_size = (p->N / base + 1);

        int current_block = (block_i_end * block_grid_size + block_j_end);
        int left_block = (block_i_end * block_grid_size + block_j_end - 1);
        int above_block = ((block_i_end - 1) * block_grid_size + block_j_end);
        int diag_block = ((block_i_end - 1) * block_grid_size + block_j_end - 1);

        ocrGuid_t current, left, above, diag;
        ocrGuidFromIndex(&current, p->score_matrix_labels, current_block);
        ocrAddDependence(current, baseEdt, 2, DB_MODE_RW);

        ocrGuidFromIndex(&left, p->score_matrix_labels, left_block);
        ocrAddDependence(left, baseEdt, 3, DB_MODE_RO);

        ocrGuidFromIndex(&above, p->score_matrix_labels, above_block);
        ocrAddDependence(above, baseEdt, 4, DB_MODE_RO);

        ocrGuidFromIndex(&diag, p->score_matrix_labels, diag_block);
        ocrAddDependence(diag, baseEdt, 5, DB_MODE_RO);

        return NULL_GUID;
    }
    else {

        int nn = n >> 1;

        //ocrPrintf("Working on size: %lu\n", nn);

        //--Call for x11-------------------------------------------
        LCS_task_params p1;

        p1.N = p->N;
        p1.base = base;
        p1.xi = p->xi;
        p1.xj = p->xj;
        p1.n = nn;
        p1.s_labels = p->s_labels;
        p1.t_labels = p->t_labels;
        p1.score_matrix_labels = p->score_matrix_labels;

        //ocrPrintf("Calling rec case N: %lu, xi: %lu, xj: %lu, n: %lu \n", p->N, p->xi, p->xj, p->n);
        // Create the task template
        ocrGuid_t LCSGuidTmp_X11;
        ocrEdtTemplateCreate(&LCSGuidTmp_X11, recLCSEdt, sizeof(p1) / sizeof(u64), 1);

        // Create the task
        ocrGuid_t x11Edt, x11outputEventGuid, x11outputEventGuidOnce;
        ocrEventCreate(&x11outputEventGuid, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

        ocrEdtCreate(&x11Edt,
            LCSGuidTmp_X11,
            EDT_PARAM_DEF,
            (u64*)&p1,
            EDT_PARAM_DEF,
            NULL,
            EDT_PROP_FINISH,
            NULL_HINT,
            &x11outputEventGuidOnce);
        ocrAddDependence(x11outputEventGuidOnce, x11outputEventGuid, 0, DB_MODE_NULL);
        ocrAddDependence(NULL_GUID, x11Edt, 0, DB_MODE_NULL);

        //---Call for x12-------------------------------------------
        ocrGuid_t LCSGuidTmp_X12_x21;
        ocrEdtTemplateCreate(&LCSGuidTmp_X12_x21, recLCSEdt, sizeof(p1) / sizeof(u64), 1);

        ocrGuid_t x12Edt, x12outputEventGuid, x12outputEventGuidOnce;
        ocrEventCreate(&x12outputEventGuid, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

        p1.xi = p->xi + nn, p1.xj = p->xj;

        ocrEdtCreate(&x12Edt,
            LCSGuidTmp_X12_x21,
            EDT_PARAM_DEF,
            (u64*)&p1,
            EDT_PARAM_DEF,
            &x11outputEventGuid,
            EDT_PROP_FINISH,
            NULL_HINT,
            &x12outputEventGuidOnce);

        ocrAddDependence(x12outputEventGuidOnce, x12outputEventGuid, 0, DB_MODE_NULL);

        //---------------------------Call for x21-------------------------------------------
        ocrGuid_t x21Edt, x21outputEventGuid, x21outputEventGuidOnce;
        ocrEventCreate(&x21outputEventGuid, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

        p1.xi = p->xi, p1.xj = p->xj + nn;

        ocrEdtCreate(&x21Edt,
            LCSGuidTmp_X12_x21,
            EDT_PARAM_DEF,
            (u64*)&p1,
            EDT_PARAM_DEF,
            &x11outputEventGuid,
            EDT_PROP_FINISH,
            NULL_HINT,
            &x21outputEventGuidOnce);

        ocrAddDependence(x21outputEventGuidOnce, x21outputEventGuid, 0, DB_MODE_NULL);

        //---------------------------Call for x22-------------------------------------------

        ocrGuid_t LCSGuidTmp_X22;
        ocrEdtTemplateCreate(&LCSGuidTmp_X22, recLCSEdt, sizeof(p1) / sizeof(u64), 2);

        ocrGuid_t x22Edt, x22outputEventGuid;
        ocrEventCreate(&x22outputEventGuid, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

        p1.xi = p->xi + nn, p1.xj = p->xj + nn;

        ocrEdtCreate(&x22Edt,
            LCSGuidTmp_X22,
            EDT_PARAM_DEF,
            (u64*)&p1,
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

ocrGuid_t shutDownEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    int* score = (int*)depv[1].ptr;
    int len = (int)paramv[0];

    long end = cilk_getticks();
    ocrPrintf("runtime: %f\n", cilk_ticks_to_seconds(end - (long)paramv[2]));

#ifdef CHECK_RESULTS
    if (score[len * len - 1] != (int)paramv[1])
        printf("Result did not match! :(\n");
    else
        printf("Result matched! We are good :)\n");
#endif

    ocrPrintf("\nShutting down OCR runtime\n");

    ocrShutdown(); // This is the last EDT to execute, terminate
    return NULL_GUID;
}
ocrGuid_t randInitEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    int* Ti = (int*)depv[0].ptr;
    int len = (int)paramv[0];
    int base = (int)paramv[1];
    if (len == base + 1) {
        Ti[0] = 32;
        //ocrPrintf("%d ", Ti[0]);
        Ti++;
    }

    for (int l = 0; l < len; l++) {
        Ti[l] = rand() % 4 + 'A'; // Initialize T
        //ocrPrintf("%d ", Ti[l]);
    }
    return NULL_GUID;
}

ocrGuid_t scoreInitEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    int* score = (int*)depv[0].ptr;
    int len = (int)paramv[0];
    int start = (int)paramv[1];

    if (len == 1) {
        score[0] = start;
        //ocrPrintf("%d ", score[0]);
    }
    else {

        for (int l = 0; l < len; l++) {
            score[l] = start + l; // Initialize T
            //ocrPrintf("%d ", score[l]);
        }
    }
    return NULL_GUID;
}

typedef struct
    {
    ocrGuid_t s_labels, t_labels, score_matrix_labels;
    int base, num_labels;
} init_param;

ocrGuid_t InitEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    ocrGuid_t s_labels, t_labels, score_matrix_labels;

    int** S = (int**)depv[0].ptr;
    int** T = (int**)depv[1].ptr;
    int** lcs_score = (int**)depv[2].ptr;

    init_param* p = (init_param*)paramv;
    s_labels = p->s_labels;
    t_labels = p->t_labels;
    score_matrix_labels = p->score_matrix_labels;
    int base = p->base;
    int num_labels = p->num_labels;

    // Create the DBs for S, T and lcs_score matrix and initialize as needed.
    // Use DBcreate for these as well.

    int grid_block_size = num_labels + 1;
    int num_blocks = (grid_block_size) * (grid_block_size);

    ////////////////////////Initialize T ///////////////////////

    //case0:
    ocrGuid_t T_ij;
    ocrGuidFromIndex(&T_ij, t_labels, 0);

    int block_size = base + 1;
    // Create the data block with the guid
    ocrDbCreate(&T_ij,
        (void**)&T[0],
        sizeof(int) * (block_size),
        GUID_PROP_IS_LABELED,
        NULL_HINT,
        NO_ALLOC);

    assert(T[0] != NULL);

    //create randInitEDT
    //TODO: Move this part to a initializer EDT.
    //Now initialize it
    //Create the task
    ocrGuid_t randInitTmp;
    ocrEdtTemplateCreate(&randInitTmp, randInitEdt, 2, 1);

    u64 string_param[2];
    string_param[0] = block_size;
    string_param[1] = base;

    ocrGuid_t randInitTEdt;
    ocrEdtCreate(&randInitTEdt,
        randInitTmp,
        EDT_PARAM_DEF,
        (u64*)&string_param,
        EDT_PARAM_DEF,
        NULL,
        EDT_PROP_NONE,
        NULL_HINT,
        NULL);
    ocrAddDependence(T_ij, randInitTEdt, 0, DB_MODE_RW);

    //case2: block size = base;
    block_size = base;

    for (int j = 1; j < num_labels; j++) {
        ocrGuid_t T_ij;
        // First get the already created guids
        ocrGuidFromIndex(&T_ij, t_labels, j);

        // Create the data block with the guid
        ocrDbCreate(&T_ij,
            (void**)&T[j],
            sizeof(int) * block_size,
            GUID_PROP_IS_LABELED,
            NULL_HINT,
            NO_ALLOC);

        assert(T[j] != NULL);
        // TODO: Move this part to a initializer EDT.
        // Now initialize it

        ocrGuid_t randInitTEdt;

        string_param[0] = block_size;

        ocrEdtCreate(&randInitTEdt,
            randInitTmp,
            EDT_PARAM_DEF,
            (u64*)&string_param,
            EDT_PARAM_DEF,
            NULL,
            EDT_PROP_NONE,
            NULL_HINT,
            NULL);

        ocrAddDependence(T_ij, randInitTEdt, 0, DB_MODE_RW);
    }

    //ocrPrintf("\n S:");
    ////////////////////////////////////////////////////////

    //////////////////Initialize S/////////////////////////

    // First get the already created guids
    ocrGuid_t S_ij;
    ocrGuidFromIndex(&S_ij, s_labels, 0);
    block_size = base + 1;
    // Create the data block with the guid and size
    ocrDbCreate(&S_ij,
        (void**)&S[0],
        sizeof(int) * (block_size),
        GUID_PROP_IS_LABELED,
        NULL_HINT,
        NO_ALLOC);

    assert(S[0] != NULL);
    // Now initialize it
    // TODO: Move this part to a initializer EDT.
    srand(84388311);

    ocrGuid_t randInitSEdt;

    string_param[0] = block_size;

    ocrEdtCreate(&randInitSEdt,
        randInitTmp,
        EDT_PARAM_DEF,
        (u64*)&string_param,
        EDT_PARAM_DEF,
        NULL,
        EDT_PROP_NONE,
        NULL_HINT,
        NULL);

    ocrAddDependence(S_ij, randInitSEdt, 0, DB_MODE_RW);

    block_size = base;

    for (int i = 1; i < num_labels; i++) {
        ocrGuid_t S_ij;
        // First get the already created guids
        ocrGuidFromIndex(&S_ij, s_labels, i);

        // Create the data block with the guid and size
        ocrDbCreate(&S_ij,
            (void**)&S[i],
            sizeof(int) * block_size,
            GUID_PROP_IS_LABELED,
            NULL_HINT,
            NO_ALLOC);

        assert(S[i] != NULL);
        // Now initialize it

        string_param[0] = block_size;

        ocrGuid_t randInitSEdt;

        ocrEdtCreate(&randInitSEdt,
            randInitTmp,
            EDT_PARAM_DEF,
            (u64*)&string_param,
            EDT_PARAM_DEF,
            NULL,
            EDT_PROP_NONE,
            NULL_HINT,
            NULL);

        ocrAddDependence(S_ij, randInitSEdt, 0, DB_MODE_RW);
    }

    ////////////////////////Initialize score ///////////////////////
    int data_block_size = (base * base);

    //CASE1: i=0;j=0
    ocrGuid_t lcs_score_ij;
    block_size = 1;
    // First get the guid
    ocrGuidFromIndex(&lcs_score_ij, score_matrix_labels, 0);
    // Create the data block with the guid
    ocrDbCreate(&lcs_score_ij,
        (void**)&lcs_score[0],
        sizeof(int) * 1,
        GUID_PROP_IS_LABELED,
        NULL_HINT,
        NO_ALLOC);

    assert(lcs_score[0] != NULL);

    //Create the task
    ocrGuid_t scoreInitTmp;
    ocrEdtTemplateCreate(&scoreInitTmp, scoreInitEdt, 2, 1);

    ocrGuid_t scoreEdt;

    u64 score_param[2];
    score_param[0] = block_size;
    score_param[1] = 0; //start
    ocrEdtCreate(&scoreEdt,
        scoreInitTmp,
        EDT_PARAM_DEF,
        (u64*)&score_param,
        EDT_PARAM_DEF,
        NULL,
        EDT_PROP_NONE,
        NULL_HINT,
        NULL);

    ocrAddDependence(lcs_score_ij, scoreEdt, 0, DB_MODE_RW);

    //CASE2: i = 0, j > 0
    int i = 0, j = 0;
    block_size = base;
    int t;
    for (j = 1; j < grid_block_size; j++) {
        t = j;
        // First get the guid
        ocrGuidFromIndex(&lcs_score_ij, score_matrix_labels, t);
        // Create the data block with the guid
        ocrDbCreate(&lcs_score_ij,
            (void**)&lcs_score[t],
            sizeof(int) * block_size,
            GUID_PROP_IS_LABELED,
            NULL_HINT,
            NO_ALLOC);

        assert(lcs_score[t] != NULL);

        ocrGuid_t scoreEdt;

        score_param[0] = block_size;
        score_param[1] = (j - 1) * base + 1;
        ocrEdtCreate(&scoreEdt,
            scoreInitTmp,
            EDT_PARAM_DEF,
            (u64*)&score_param,
            EDT_PARAM_DEF,
            NULL,
            EDT_PROP_NONE,
            NULL_HINT,
            NULL);

        ocrAddDependence(lcs_score_ij, scoreEdt, 0, DB_MODE_RW);
    }

    //CASE3: j-0, i>1
    j = 0;

    for (i = 1; i < grid_block_size; i++) {
        t = i * grid_block_size;
        ocrGuidFromIndex(&lcs_score_ij, score_matrix_labels, t);
        ocrDbCreate(&lcs_score_ij,
            (void**)&lcs_score[t],
            sizeof(int) * base,
            GUID_PROP_IS_LABELED,
            NULL_HINT,
            NO_ALLOC);

        assert(lcs_score[t] != NULL);

        ocrGuid_t scoreEdt;

        score_param[0] = block_size;
        score_param[1] = (i - 1) * base + 1;
        ocrEdtCreate(&scoreEdt,
            scoreInitTmp,
            EDT_PARAM_DEF,
            (u64*)&score_param,
            EDT_PARAM_DEF,
            NULL,
            EDT_PROP_NONE,
            NULL_HINT,
            NULL);

        ocrAddDependence(lcs_score_ij, scoreEdt, 0, DB_MODE_RW);
    }

    //CASE4: i>0, j>0
    for (int i = 1; i < grid_block_size; i++) {
        for (int j = 1; j < grid_block_size; j++) {
            int t = (i * grid_block_size + j);

            ocrGuid_t lcs_score_ij;
            // First get the guid
            ocrGuidFromIndex(&lcs_score_ij, score_matrix_labels, t);

            // Create the data block with the guid
            ocrDbCreate(&lcs_score_ij,
                (void**)&lcs_score[t],
                sizeof(int) * data_block_size,
                GUID_PROP_IS_LABELED,
                NULL_HINT,
                NO_ALLOC);
            assert(lcs_score[t] != NULL);
        }
        // initialize data
    }
    return NULL_GUID;
}

//----------------------------------------------------- Main EDT --------------------------//
extern "C" ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    // Collect command line arguments.
    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;
    void* PTR_cmdLineArgs = depv[0].ptr;
    u32 argc = ocrGetArgc(PTR_cmdLineArgs);

    // Input params and default values.
    int N = 1024; // string length
    int base = 256; // basecase
    int num_workers = 16; // maximum number of workers

    if (argc < 1) {
        ocrPrintf("USAGE: please run the program as follows:\n"
               "executable input_string_length number_of_cores basecase\n");
        exit(1);
    }

    if (argc > 1) {
        N = (int)atol(ocrGetArgv(PTR_cmdLineArgs, 1));
    }
    if (argc > 2) {
        base = (int)atol(ocrGetArgv(PTR_cmdLineArgs, 2));
    }
    if (argc > 3) {
        num_workers = (int)atol(ocrGetArgv(PTR_cmdLineArgs, 3));
    }

    // base can not be > N.
    if (N < base)
        base = N;

    ocrPrintf("\n");
    ocrPrintf("Running LCS.\nStrings len: %ld  basecase: %ld\n", N, base);
    fflush(stdout);

    // Create mapguids for S, T and lcs_score matrix.
    ocrGuid_t s_labels, t_labels, score_matrix_labels;
    int num_labels = N / base;
    int grid_block_size = num_labels + 1;
    int num_blocks = (grid_block_size) * (grid_block_size);

    ocrGuidRangeCreate(&s_labels, num_labels, GUID_USER_DB);
    ocrGuidRangeCreate(&t_labels, num_labels, GUID_USER_DB);
    ocrGuidRangeCreate(&score_matrix_labels, num_blocks, GUID_USER_DB);

    int **S, **T;
    int** lcs_score;

    ocrGuid_t S_guid, T_guid, score_guid;

    ocrDbCreate(&S_guid,
        (void**)&S,
        sizeof(int*) * num_labels,
        DB_PROP_NONE,
        NULL_HINT,
        NO_ALLOC);

    assert(S != NULL);
    ocrDbCreate(&T_guid,
        (void**)&T,
        sizeof(int*) * num_labels,
        DB_PROP_NONE,
        NULL_HINT,
        NO_ALLOC);

    assert(T != NULL);
    // Using quadratic space.
    ocrDbCreate(&score_guid,
        (void**)&lcs_score,
        sizeof(int*) * num_blocks,
        DB_PROP_NONE,
        NULL_HINT,
        NO_ALLOC);

    assert(lcs_score != NULL);

    // Initialize the data
    //ocrPrintf("%d HERE\n", );

    init_param pv;

    pv.s_labels = s_labels;
    pv.t_labels = t_labels;
    pv.score_matrix_labels = score_matrix_labels;
    pv.base = base;
    pv.num_labels = num_labels;

    // Create the task template
    ocrGuid_t initGuidTmp;
    ocrEdtTemplateCreate(&initGuidTmp, InitEdt, sizeof(init_param) / sizeof(u64), 3);
    // Create the task
    ocrGuid_t initDataEdt, outputEventGuid, outputEventGuidOnce;
    ocrEventCreate(&outputEventGuid, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

    ocrEdtCreate(&initDataEdt,
        initGuidTmp,
        EDT_PARAM_DEF,
        (u64*)&pv,
        EDT_PARAM_DEF,
        NULL,
        EDT_PROP_FINISH,
        NULL_HINT,
        &outputEventGuidOnce);

    ocrAddDependence(outputEventGuidOnce, outputEventGuid, 0, DB_MODE_NULL);
    ocrAddDependence(S_guid, initDataEdt, 0, DB_MODE_RW);
    ocrAddDependence(T_guid, initDataEdt, 1, DB_MODE_RW);
    ocrAddDependence(score_guid, initDataEdt, 2, DB_MODE_RW);

#ifdef CHECK_RESULTS
    int Nplus1 = N + 1;

    int* _S = (int*)malloc(Nplus1 * sizeof(int));
    int* _T = (int*)malloc(Nplus1 * sizeof(int));

    _S[0] = _T[0] = 32;

    // Linear space array. Computation is going to happen row by row.
    int* score = (int*)malloc((Nplus1 * Nplus1) * sizeof(int));

    // Initialize the matrix.
    for (int i = 0; i < Nplus1; i++) {
        score[i] = i;
    }
    //score[Nplus1]=1; //first value for the second row
    for (int i = 0; i < Nplus1; i++) {
        score[i * Nplus1] = i;
    }
#endif

#ifdef CHECK_RESULTS
    int* Ti = T[0];
    for (int l = 1; l < block_size; l++) {
        _T[l] = Ti[l];
    }
#endif

#ifdef CHECK_RESULTS
    int* _Ti = _T + block_size;
#endif

#ifdef CHECK_RESULTS
    Ti = T[j];
    for (int l = 0; l < block_size; l++) {
        _Ti[l] = Ti[l];
    }
    _Ti = _Ti + block_size;
#endif

#ifdef CHECK_RESULTS
    int* Si = S[0];
    Si[0] = 32;
    for (int l = 1; l < block_size; l++) {
        _S[l] = Si[l];
    }
#endif

#ifdef CHECK_RESULTS
    int* _Si = _S + block_size;
#endif

#ifdef CHECK_RESULTS
    Si = S[i];
    for (int l = 0; l < block_size; l++) {
        _Si[l] = Si[l];
    }
    _Si = _Si + block_size;
#endif

    int final_value = 0;
////////////////////////////////////////////////////////////
// Print values for debug purpose
#ifdef PRINT
    ocrPrintf("\nS: ");
    for (int i = 0; i < num_labels; i++) {
        int* Si = S[i];
        int block_size = (i == 0) ? (base + 1) : base;
        for (int j = 0; j < block_size; j++) {
            ocrPrintf("%ld ", Si[j]);
        }
    }
    ocrPrintf("\nT: ");
    for (int i = 0; i < num_labels; i++) {
        int* Ti = T[i];
        int block_size = (i == 0) ? (base + 1) : base;
        for (int j = 0; j < block_size; j++) {
            ocrPrintf("%ld ", Ti[j]);
        }
    }

    ocrPrintf("\n");
#endif

#ifdef CHECK_RESULTS
#ifdef PRINT
    ocrPrintf("\n_S: ");
    for (int i = 0; i < Nplus1; i++) {

        ocrPrintf("%ld ", _S[i]);
    }
    ocrPrintf("\n_T: ");
    for (int i = 0; i < Nplus1; i++) {

        ocrPrintf("%ld ", _T[i]);
    }
    ocrPrintf("\n");
#endif

    serial_lcs(score, _S, _T, 1, 1, N);

#ifdef PRINT
    for (int i = 1; i < Nplus1; i++) {
        for (int j = 1; j < Nplus1; j++) {
            ocrPrintf("%d ", score[i * Nplus1 + (j)]);
        }
        ocrPrintf("\n");
    }
#endif

    final_value = score[Nplus1 * Nplus1 - 1];

    free(_S);
    free(_T);
    free(score);
#endif

    //Create a new finish EDT
    //params are N, base, xi, xj, and the guids.

    LCS_task_params p;

    p.N = N;
    p.base = base;
    p.xi = 1;
    p.xj = 1;
    p.n = N;
    p.s_labels = s_labels;
    p.t_labels = t_labels;
    p.score_matrix_labels = score_matrix_labels;

    unsigned long start;
    start = cilk_getticks();

    // Create the task template
    ocrGuid_t recLCSGuid;
    ocrEdtTemplateCreate(&recLCSGuid, recLCSEdt, sizeof(LCS_task_params) / sizeof(u64), 1);
    //ocrPrintf("%d HERE\n", sizeof(LCS_task_params)/sizeof(u64) );

    // Create the task
    ocrGuid_t recLCSEdt, outputEventGuid2, outputEventGuidOnce2;
    ocrEventCreate(&outputEventGuid2, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

    ocrEdtCreate(&recLCSEdt,
        recLCSGuid,
        EDT_PARAM_DEF,
        (u64*)&p,
        EDT_PARAM_DEF,
        NULL,
        EDT_PROP_FINISH,
        NULL_HINT,
        &outputEventGuidOnce2);

    ocrAddDependence(outputEventGuidOnce2, outputEventGuid2, 0, DB_MODE_NULL);
    ocrAddDependence(outputEventGuid, recLCSEdt, 0, DB_MODE_NULL);

    // Create the task template
    ocrGuid_t shutDown_tmp;
    u64 paramv2[3];
    paramv2[0] = base;
    paramv2[1] = final_value;
    paramv2[2] = start;

    ocrEdtTemplateCreate(&shutDown_tmp, shutDownEdt, 3, 2);

    // Create the task
    ocrGuid_t shutDown_taskGuid;
    ocrEdtCreate(&shutDown_taskGuid,
        shutDown_tmp,
        EDT_PARAM_DEF,
        (u64*)paramv2,
        EDT_PARAM_DEF,
        NULL,
        EDT_PROP_NONE,
        NULL_HINT,
        NULL);

    ocrGuid_t lcs_score_ij;
    ocrGuidFromIndex(&lcs_score_ij, score_matrix_labels, num_blocks - 1);

    ocrAddDependence(outputEventGuid2, shutDown_taskGuid, 0, DB_MODE_NULL);
    ocrAddDependence(lcs_score_ij, shutDown_taskGuid, 1, DB_MODE_RO);

    printf("shutting down OCR\n");
    //	ocrShutdown();
    return NULL_GUID;
}

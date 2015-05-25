#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>
#include <mkl.h>
#include <mkl_lapacke.h>

#include "HTA.h"
#include "Comm.h"
#include "HTA_operations.h"
#include "Tuple.h"
#include "test.h"

#define BILLION  1000000000UL

char uplo = 'L';
int nBlocks, nEntries;
int lnt, lmt;
uint64_t startS, startNS, endS, endNS;
uint64_t potrfstartS, potrfstartNS, potrfendS, potrfendNS, potrfelapsedNS=0;
uint64_t trsmstartS, trsmstartNS, trsmendS, trsmendNS, trsmelapsedNS=0;
uint64_t gemmstartS, gemmstartNS, gemmendS, gemmendNS, gemmelapsedNS=0;
char prcs; // precision
int mThreads;
int info;
DIST_TYPE dist_type = DIST_ROW_CYCLIC;

double *sAMatrix;

static inline double
random_value_ (double a, double b)
{
	return (a > b) ? random_value_ (b, a) : (a + ((b-a)*drand48 ()));
}
// *************************************************************
// * Timing functions to start and stop the timer.
// ************************************************************/
void timerstart(uint64_t *startS, uint64_t *startNS)
{
	struct timespec tsTime;
	if (clock_gettime(CLOCK_REALTIME, &tsTime) == -1) {
		perror("clock gettime");
		exit(EXIT_FAILURE);
	} else {
		*startS = tsTime.tv_sec;
		*startNS = tsTime.tv_nsec;
	}
}

void timerend(uint64_t *endS, uint64_t *endNS)
{
	struct timespec tsTime;
	if (clock_gettime(CLOCK_REALTIME, &tsTime) == -1) {
		perror("clock gettime");
		exit(EXIT_FAILURE);
	} else {
		*endS = tsTime.tv_sec;
		*endNS = tsTime.tv_nsec;
	}
}

uint64_t timerelapsed(uint64_t startS, uint64_t startNS, uint64_t endS, uint64_t endNS)
{
    return ((endS - startS) * BILLION) + endNS - startNS;
}

void TRSM(HTA* hAik, HTA* hAkk) {
    ASSERT(hAkk);
    ASSERT(hAik);
    double *adata = HTA_get_ptr_raw_data(hAkk);
    double *bbdata = HTA_get_ptr_raw_data(hAik);
    int mRowInTile = hAkk->flat_size.values[1];
    ASSERT(mRowInTile == hAkk->flat_size.values[0]); // works for square tiles only for now
    cblas_dtrsm(CblasColMajor, CblasRight, CblasLower,
                CblasTrans, CblasNonUnit, mRowInTile, mRowInTile, 1.0,
                adata, mRowInTile, bbdata, mRowInTile);
}

void SYRK_OR_DGEMM(HTA* hAij, HTA* hAik, HTA* hAjk) {
    ASSERT(hAij);
    ASSERT(hAik);
    ASSERT(hAjk);
    int i = hAij->nd_rank.values[1];
    int j = hAij->nd_rank.values[0];
    int mRowInTile = hAij->flat_size.values[1];
    ASSERT(mRowInTile == hAij->flat_size.values[0]); // works for square tiles only for now
    if(i == j) {
        double *aadata = HTA_get_ptr_raw_data(hAij);
        double *bbdata = HTA_get_ptr_raw_data(hAik);
        cblas_dsyrk(CblasColMajor, CblasLower, CblasNoTrans,
                 mRowInTile, mRowInTile, -1.0, bbdata, mRowInTile, 1.0,
                 aadata, mRowInTile);
    } else {
        double *cdata = HTA_get_ptr_raw_data(hAij);
        double *acdata = HTA_get_ptr_raw_data(hAjk);
        double *bcdata = HTA_get_ptr_raw_data(hAik);
        cblas_dgemm(CblasColMajor, CblasNoTrans,
                 CblasTrans, mRowInTile, mRowInTile, mRowInTile,
                 -1.0, bcdata, mRowInTile, acdata, mRowInTile, 1.0,
                 cdata, mRowInTile);
    }
}

void POTRF(HTA* hAkk) {
    ASSERT(hAkk);
    double *adata = HTA_get_ptr_raw_data(hAkk);
    int mRowInTile = hAkk->flat_size.values[1];
    ASSERT(mRowInTile == hAkk->flat_size.values[0]); // works for square tiles only for now
    dpotrf(&uplo, &mRowInTile, HTA_get_ptr_raw_data(hAkk), &mRowInTile, &info); // serial cholesky factorization on diagonal tile
}

int hta_main(int argc, char** argv, int pid)
{

    SET_SPMD_PID(pid)
    MASTER_EXEC_START
    /* We will use the same number of entries for each block */
    if (argc < 5) {
        fprintf(stderr, "Format: %s S|D #blocks #elements #numThreads #dist\n", *argv);
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Initializing...");
    fflush(stderr);

    /* Read params */
    prcs = *(argv[1]);
    ASSERT(prcs == 'D' && "only supports double precision for now");
    nBlocks = strtoul(argv[2], NULL, 10);
    nEntries = strtoul(argv[3], NULL, 10);
    mThreads = strtoul(argv[4], NULL, 10);
    if(argc > 5) {
        dist_type = strtoul(argv[5], NULL, 10);
        assert(dist_type >= 0 && dist_type < DIST_TYPE_MAX);
    }
    printf("precision: %c, nBlocks = %d, nEntries = %d, mThreads = %d, dist_type = %d\n", prcs, nBlocks, nEntries, mThreads, dist_type);

    mkl_set_num_threads(1);

    // Initialize the matrix
    lmt = nBlocks * nEntries;
    lnt = lmt;
    printf("Allocating flat matrix of size %dx%d\n", lmt, lnt);
    sAMatrix = memalign(32, lmt * lnt * sizeof(double));
    ASSERT(sAMatrix);
    timerstart(&startS, &startNS);
    double *tptr = sAMatrix;
    for (int i = 0; i < lmt; ++i) {
            for (int j = 0; j < i; ++j) {
                    double x = random_value_ (-1.0, 1.0) ;
                    tptr[j + i* lmt] = tptr[i + j* lmt] = x;
            }
            tptr[i + i*lmt] = lmt + 1;
    }
    MASTER_EXEC_SYNC_END
    double *AMatrix = memalign(32, lmt * lnt * sizeof(double));

    HTA_SCALAR_TYPE stype = ((prcs == 's') || (prcs == 'S')) ? HTA_SCALAR_TYPE_FLOAT : HTA_SCALAR_TYPE_DOUBLE;
    ASSERT(stype != HTA_SCALAR_TYPE_FLOAT && "single precision not supported");
    Tuple t0 = Tuple_create(2, nBlocks, nBlocks);
    Tuple flat_size = Tuple_create(2, nBlocks*nEntries, nBlocks*nEntries);
    Tuple mesh = HTA_get_vp_mesh(2);
    printf("Processor mesh: "); Tuple_print(&mesh);
    Dist dist;
    Dist_init(&dist, dist_type, &mesh);
    //Dist_init(&dist, DIST_BLOCK, &mesh);
    HTA *hA = HTA_create_impl(pid, NULL, 2, 2, &flat_size, 0, &dist, stype, 1, &t0);
    HTA_init_with_array(hA, sAMatrix); // HTA library uses row-major, so the matrix is transposed after initialization

    // Warm up for MKL library function calls
    double *kk = memalign(32, sizeof(double) * nEntries *nEntries);
    double *jj = memalign(32, sizeof(double) * nEntries *nEntries);
    double *ll = memalign(32, sizeof(double) * nEntries *nEntries);

    dpotrf(&uplo, &nEntries, kk, &nEntries, &info); // serial cholesky factorization on diagonal tile
    cblas_dsyrk(CblasColMajor, CblasLower, CblasNoTrans,
             nEntries, nEntries, -1.0, kk, nEntries, 1.0,
             jj, nEntries);
    cblas_dtrsm(CblasColMajor, CblasRight, CblasLower,
                CblasTrans, CblasNonUnit, nEntries, nEntries, 1.0,
                kk, nEntries, jj, nEntries);
    cblas_dgemm(CblasColMajor, CblasNoTrans,
             CblasTrans, nEntries, nEntries, nEntries,
             -1.0, kk, nEntries, jj, nEntries, 1.0,
             ll, nEntries);

    // preallocate dependence arrays
    Tuple * sel1 = malloc(nBlocks * sizeof(Tuple));
    Tuple * sel2 = malloc(nBlocks * sizeof(Tuple));
    int maxNumTasks = (nBlocks*(nBlocks-1)/2); // same as first iteration numDGEMMS calculation
    Tuple * sel3 = malloc( maxNumTasks * sizeof(Tuple));
    Tuple * sel4 = malloc( maxNumTasks * sizeof(Tuple));
    Tuple * sel5 = malloc( maxNumTasks * sizeof(Tuple));

    HTA_barrier(pid);
    MASTER_EXEC_START
    timerend(&endS, &endNS);
    endS -= startS;
    endNS += endS * BILLION;
    endNS -= startNS;
    fprintf(stderr, "done.\n");
    fprintf(stderr, "MKL LARNV and GEMM initialization,   Matrix size %d is done in %6ld msec\n", lmt, (endNS)/(1000*1000));
    fprintf(stderr, "Running HTA Cholesky...");
    fflush(stderr);
    timerstart(&startS, &startNS);
    MASTER_EXEC_END
    for(int k = 0; k < nBlocks; k++) {
        int numDGEMMS = ((nBlocks-(k))*(nBlocks-(k)-1)/2);
        // Serially process diagonal tile (potrf)
        //printf("order of matrix = %d, lda = %d\n", nEntries, nEntries);
        timerstart(&potrfstartS, &potrfstartNS);
        Tuple tidx = Tuple_create(2, k, k);
        HTA *hAkk = HTA_pick_one_tile(hA, &tidx);
#if !defined(TRACING)
        if(hAkk->home == pid) // only master thread will perform this task in fork-join mode
            dpotrf(&uplo, &nEntries, HTA_get_ptr_raw_data(hAkk), &nEntries, &info); // serial cholesky factorization on diagonal tile
#else
        HTA_map_h1(HTA_LEAF_LEVEL(hAkk), POTRF, hAkk);
#endif
        timerend(&potrfendS, &potrfendNS);
        potrfelapsedNS += timerelapsed(potrfstartS, potrfstartNS, potrfendS, potrfendNS);
        //printf("k = %d, dpotrf result %d\n", k, info);

        // Process column k tiles using map_hsel (trsm)
        // Generate Tuple arrays to select paired tiles
        timerstart(&trsmstartS, &trsmstartNS);
        int numColTiles = nBlocks - k - 1;
        if(numColTiles > 0) {
            //Tuple sel1[numColTiles]; // For Akk
            //Tuple sel2[numColTiles]; // For Aik
            //printf("k = %d, TRSM in parallel: %d tiles\n", k, numColTiles);
            for(int i = k+1; i < nBlocks; i++) {
                sel1[i - k - 1] = Tuple_create(2, k, i);
                //sel2[i - k - 1] = Tuple_create(2, i, k);
                sel2[i - k - 1] = Tuple_create(2, k, k); // because it's transposed while being read to HTA
                //printf("k = %d, Call TRSM for pair A(%d, %d), A(%d, %d)\n", k, k, k, k, i);
            }
            HTA_cmap_h2(HTA_LEAF_LEVEL(hA), TRSM, numColTiles, hA, sel1, hA, sel2, k);
            // implicit barrier here in fork join mode
            //printf("k = %d, TRSM finished\n", k); fflush(stdout);
        }
        timerend(&trsmendS, &trsmendNS);
        trsmelapsedNS += timerelapsed(trsmstartS, trsmstartNS, trsmendS, trsmendNS);
        // Process the submatrix using map_hsel (syrk & gemm)
        timerstart(&gemmstartS, &gemmstartNS);
        if(numDGEMMS > 0) {
            //printf("k = %d, nBlocks = %d, GEMM in parallel: %d tiles\n", k, nBlocks, numDGEMMS);
            //Tuple sel3[numDGEMMS]; // for Aij
            //Tuple sel4[numDGEMMS]; // for Aik
            //Tuple sel5[numDGEMMS]; // for Ajk
            int c = 0;
            for(int i = k+1; i < nBlocks; i++) {
                for(int j = k+1; j <= i; j++) {
                    //sel3[c] = Tuple_create(2, i, j);
                    //sel4[c] = Tuple_create(2, i, k);
                    //sel5[c] = Tuple_create(2, j, k);
                    sel3[c] = Tuple_create(2, j, i);
                    sel4[c] = Tuple_create(2, k, i);
                    sel5[c] = Tuple_create(2, k, j);
                    c++;
                }
            }
            HTA_cmap_h3(HTA_LEAF_LEVEL(hA), SYRK_OR_DGEMM, numDGEMMS, hA, sel3, hA, sel4, hA, sel5, k);
            // implicit barrier here in fork join mode
        }
        timerend(&gemmendS, &gemmendNS);
        gemmelapsedNS += timerelapsed(gemmstartS, gemmstartNS, gemmendS, gemmendNS);
    }
    HTA_barrier(pid);
    MASTER_EXEC_START
    timerend(&endS, &endNS);
    endS -= startS;
    endNS += endS * BILLION;
    endNS -= startNS;
    fprintf(stderr, "done.\n");
    fprintf(stderr, "Tiled %6s Cholesky decomposition, Matrix size %d is done in %6ld msec\n", "HTA", lmt, (endNS)/(1000*1000));
    fprintf(stderr, "POTRF %6ld msec (%4.2lf)\n", (potrfelapsedNS)/(1000*1000), potrfelapsedNS/(endNS/100.0));
    fprintf(stderr, "TRSM  %6ld msec (%4.2lf)\n", (trsmelapsedNS)/(1000*1000), trsmelapsedNS/(endNS/100.0));
    fprintf(stderr, "GEMM  %6ld msec (%4.2lf)\n", (gemmelapsedNS)/(1000*1000), gemmelapsedNS/(endNS/100.0));
    MASTER_EXEC_END
#if !defined(TRACING) && defined(VERIFY)
    MASTER_EXEC_START
    fprintf(stderr, "running mkl with %d threads...", mThreads);
    fflush(stderr);
    mkl_set_num_threads(mThreads);
    // *******************************************************************************
    // * Call prototype mkl functions
    // *******************************************************************************
    timerstart(&startS, &startNS);
    //		dpotf2(uplo, &N, sAMatrix->data, &(AMatrix->lmt), &info);
    LAPACKE_dpotrf(LAPACK_COL_MAJOR, uplo, lnt,
                    sAMatrix, lmt);
    timerend(&endS, &endNS);
    endS -= startS;
    endNS += endS * BILLION;
    endNS -= startNS;
    fprintf(stderr, "done.\n");
    fprintf(stderr, "MKL Un-Tiled Cholesky decomposition, Matrix size %d is done in %6ld msec\n", lmt, (endNS)/(1000*1000));
    fflush(stderr);
    MASTER_EXEC_END

    // After flattening, the matrix will be transpose back to column major format to be compared with the golden result
    HTA_flatten(AMatrix, NULL, NULL, hA);

    MASTER_EXEC_START
    fprintf(stderr, "Checking data...");
    fflush(stderr);
    int i, j, k;
    double *adata = sAMatrix;
    double *bbdata = AMatrix;

    for (k = 0; k < lmt; ++k) {
            for (j = 0; j < lnt; ++j) {
                    i = j * lmt + k;
                    double err = fabs(1.0 - bbdata[i] / adata[i]);
                    double err2 = fabs( bbdata[i] - adata[i]);
                    if ((err > 5.e-8) && (err2 > 5.e-8))
                            printf("%d:%fx%f, ", i, err2, err);
            }
    }
    fprintf(stderr, "done.\n");
    free(sAMatrix);
    MASTER_EXEC_END

    free(sel1);
    free(sel2);
    free(sel3);
    free(sel4);
    free(sel5);
    free(AMatrix);
    HTA_destroy(hA);
#endif

    return 0;
}

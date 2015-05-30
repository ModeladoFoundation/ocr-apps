/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

#include "ocr.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include "mkl.h"
#include <stdbool.h>
#include <time.h>

#ifndef TG_ARCH

// Uncomment the below two lines to include EDT profiling information
// Also see OCR_ENABLE_EDT_PROFILING in common.mk
//#include "app-profiling.h"
//#include "db-weights.h"


#define FLAGS DB_PROP_NONE
#define PROPERTIES EDT_PROP_NONE
#define INTEL_BB 64 // Define Intel Byte-Boundary for mkl_alloc

static double** readMatrix( u32 matrixSize, FILE* in );
#endif

// This task computes the Cholesky factorization of a symmetric positive definite matrix...
// This is the first step in the tile Cholesky factorization.
ocrGuid_t lapacke_dpotrf_task ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u32 info;

    u64 *func_args = paramv;
    u32 k = (u32) func_args[0];
    u32 tileSize = (u32) func_args[1];
    ocrGuid_t out_lkji_kkkp1_event_guid = (ocrGuid_t) func_args[2];

    double* aBlock = (double*) (depv[0].ptr);

//    PRINTF("RUNNING sequential_cholesky %d with 0x%llx to satisfy\n", k, (u64)(out_lkji_kkkp1_event_guid));

    ocrGuid_t out_lkji_kkkp1_db_guid;
    ocrGuid_t out_lkji_kkkp1_db_affinity = NULL_GUID;

    info = LAPACKE_dpotrf(LAPACK_ROW_MAJOR, 'L', tileSize, aBlock, tileSize );

    if (info != 0)
    {
        if (info > 0) PRINTF("Matrix A is not Symmetric Positive Definite (SPD)");
        else PRINTF("i-th parameter had an illegal value.");
        ASSERT(0);
        ocrShutdown();
        return NULL_GUID;
    }

    ocrEventSatisfy(out_lkji_kkkp1_event_guid, depv[0].guid);

    return NULL_GUID;
}

// This task solves a triangular matrix equation, which is the
// second step in the tiled Cholesky factorization
ocrGuid_t cblas_dtrsm_task ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 *func_args = paramv;
    u32 k = (u32) func_args[0];
    u32 j = (u32) func_args[1];
    u32 tileSize = (u32) func_args[2];
    ocrGuid_t out_lkji_jkkp1_event_guid = (ocrGuid_t) func_args[3];

//    PRINTF("RUNNING trisolve (%d, %d)\n", k, j);

    double* aBlock = (double*) (depv[0].ptr);
    double* liBlock = (double*) (depv[1].ptr);

    ocrGuid_t out_lkji_jkkp1_db_guid;
    ocrGuid_t out_lkji_jkkp1_db_affinity = NULL_GUID;

    // Solve Ax = alpha B where alpha = 1 and A is liBlock triangular matrix output from
    // dpotrf and B is the next block to solve in factorization
    cblas_dtrsm(CblasColMajor, CblasLeft, CblasUpper, CblasTrans, CblasNonUnit, tileSize, tileSize, 1.0,
                liBlock, tileSize, // A matrix to solve for x in A x = B
                aBlock, tileSize); // B matrix, solution put here

    ocrEventSatisfy(out_lkji_jkkp1_event_guid, depv[0].guid);

    return NULL_GUID;
}

// This task performs a symmetric rank-k update, which is the
// third step in the tiled Cholesky factorization
ocrGuid_t cblas_dsyrk_task ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 *func_args = paramv;
    u32 k = (u32) func_args[0];
    u32 j = (u32) func_args[1];
    u32 i = (u32) func_args[2];
    u32 tileSize = (u32) func_args[3];
    ocrGuid_t out_lkji_jjkp1_event_guid = (ocrGuid_t) func_args[4];

//    PRINTF("RUNNING update_diagonal (%d, %d, %d)\n", k, j, i);

    double* aBlock = (double*) (depv[0].ptr);
    double* l2Block = (double*) (depv[1].ptr);

    // Symmetric Rank-k Update C = alpha AA' + beta C, where alpha = -1,  beta = 1
    cblas_dsyrk(CblasRowMajor, CblasLower, CblasNoTrans, tileSize, tileSize, -1.0,
                l2Block, tileSize, 1.0, // A matrix
                aBlock, tileSize); // C matrix, solution put here in lower triangle

    ocrEventSatisfy(out_lkji_jjkp1_event_guid, depv[0].guid);

    return NULL_GUID;
}

// This computes a matrix-matrix product to complete the last step in
// the Cholesky factorization prior to the next iteration.
ocrGuid_t cblas_dgemm_task ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 *func_args = paramv;
    u32 k = (u32) func_args[0];
    u32 j = (u32) func_args[1];
    u32 i = (u32) func_args[2];
    u32 tileSize = (u32) func_args[3];
    ocrGuid_t out_lkji_jikp1_event_guid = (ocrGuid_t) func_args[4];

//    PRINTF("RUNNING update_nondiagonal (%d, %d, %d)\n", k, j, i);

    double* aBlock = (double*) (depv[0].ptr);
    double* l1Block = (double*) (depv[1].ptr);
    double* l2Block = (double*) (depv[2].ptr);

    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasTrans, tileSize, tileSize, tileSize, -1.0,
                l1Block, tileSize, // A matrix
                l2Block, tileSize, 1.0, // B matrix
                aBlock, tileSize); // C matrix, solution put here

    ocrEventSatisfy(out_lkji_jikp1_event_guid, depv[0].guid);

    return NULL_GUID;
}

ocrGuid_t wrap_up_task ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u32 i, j, i_b, j_b;
    double* temp;

    u64 *func_args = paramv;
    u32 numTiles = (u32) func_args[0];
    u32 tileSize = (u32) func_args[1];
    u32 outSelLevel = (u32) func_args[2];

    struct timeval a;
    if(outSelLevel == 5)
    {
        FILE* outCSV = fopen("ocr_mkl_cholesky_stats.csv", "a");
        if( !outCSV ) {
            PRINTF("Cannot find file: %s\n", "ocr_mkl_cholesky_stats.csv");
            ocrShutdown();
            return NULL_GUID;
        }

        gettimeofday(&a, 0);
        fprintf(outCSV, "%f\n", (a.tv_sec*1000000+a.tv_usec)*1.0/1000000);
        fclose(outCSV);
        outSelLevel = 2;
    }

    FILE* out = fopen("ocr_mkl_cholesky.out", "w");

    for ( i = 0; i < numTiles; ++i ) {
        for( i_b = 0; i_b < tileSize; ++i_b) {
            for ( j = 0; j <= i; ++j ) {
                temp = (double*) (depv[i*(i+1)/2+j].ptr);

                if(i != j) {
                    for(j_b = 0; j_b < tileSize; ++j_b) {
                        switch(outSelLevel)
                        {
                        case 0:
                            printf("%lf ", temp[i_b*tileSize+j_b]);
                            break;
                        case 1:
                            fprintf(out, "%lf ", temp[i_b*tileSize+j_b]);
                            break;
                        case 2:
                            fwrite(&temp[i_b*tileSize+j_b], sizeof(double), 1, out);
                            break;
                        case 3:
                            fprintf(out, "%lf ", temp[i_b*tileSize+j_b]);
                            printf("%lf ", temp[i_b*tileSize+j_b]);
                            break;
                        case 4:
                            fwrite(&temp[i_b*tileSize+j_b], sizeof(double), 1, out);
                            printf("%lf ", temp[i_b*tileSize+j_b]);
                            break;
                        }
                    }
                }
                else {
                    for(j_b = 0; j_b <= i_b; ++j_b) {
                        switch(outSelLevel)
                        {
                        case 0:
                            printf("%lf ", temp[i_b*tileSize+j_b]);
                            break;
                        case 1:
                            fprintf(out, "%lf ", temp[i_b*tileSize+j_b]);
                            break;
                        case 2:
                            fwrite(&temp[i_b*tileSize+j_b], sizeof(double), 1, out);
                            break;
                        case 3:
                            fprintf(out, "%lf ", temp[i_b*tileSize+j_b]);
                            printf("%lf ", temp[i_b*tileSize+j_b]);
                            break;
                        case 4:
                            fwrite(&temp[i_b*tileSize+j_b], sizeof(double), 1, out);
                            printf("%lf ", temp[i_b*tileSize+j_b]);
                            break;
                        }
                    }
                }
            }
        }
    }
    fclose(out);

    ocrShutdown();

    return NULL_GUID;
}

inline static void lapacke_dpotrf_task_prescriber (ocrGuid_t edtTemp, u32 k,
                                                   u32 tileSize, ocrGuid_t*** lkji_event_guids) {
    ocrGuid_t seq_cholesky_task_guid;

    u64 func_args[3];
    func_args[0] = k;
    func_args[1] = tileSize;
    func_args[2] = (u64)(lkji_event_guids[k][k][k+1]);

    ocrGuid_t affinity = NULL_GUID;
    ocrEdtCreate(&seq_cholesky_task_guid, edtTemp, 3, func_args, 1, NULL, PROPERTIES, affinity, NULL);

    ocrAddDependence(lkji_event_guids[k][k][k], seq_cholesky_task_guid, 0, DB_MODE_RW);
}

inline static void cblas_dtrsm_task_prescriber ( ocrGuid_t edtTemp, u32 k, u32 j, u32 tileSize,
                                                 ocrGuid_t*** lkji_event_guids) {
    ocrGuid_t cblas_dtrsm_task_guid;

    u64 func_args[4];
    func_args[0] = k;
    func_args[1] = j;
    func_args[2] = tileSize;
    func_args[3] = (u64)(lkji_event_guids[j][k][k+1]);


    ocrGuid_t affinity = NULL_GUID;
    ocrEdtCreate(&cblas_dtrsm_task_guid, edtTemp, 4, func_args, 2, NULL, PROPERTIES, affinity, NULL);

    ocrAddDependence(lkji_event_guids[j][k][k], cblas_dtrsm_task_guid, 0, DB_MODE_RW);
    ocrAddDependence(lkji_event_guids[k][k][k+1], cblas_dtrsm_task_guid, 1, DB_MODE_RW);
}

inline static void cblas_dgemm_task_prescriber ( ocrGuid_t edtTemp, u32 k, u32 j, u32 i,
                                                 u32 tileSize, ocrGuid_t*** lkji_event_guids) {
    ocrGuid_t cblas_dgemm_task_guid;

    u64 func_args[5];
    func_args[0] = k;
    func_args[1] = j;
    func_args[2] = i;
    func_args[3] = tileSize;
    func_args[4] = (u64)(lkji_event_guids[j][i][k+1]);

    ocrGuid_t affinity = NULL_GUID;
    ocrEdtCreate(&cblas_dgemm_task_guid, edtTemp, 5, func_args, 3, NULL, PROPERTIES, affinity, NULL);

    ocrAddDependence(lkji_event_guids[j][i][k], cblas_dgemm_task_guid, 0, DB_MODE_RW);
    ocrAddDependence(lkji_event_guids[j][k][k+1], cblas_dgemm_task_guid, 1, DB_MODE_RW);
    ocrAddDependence(lkji_event_guids[i][k][k+1], cblas_dgemm_task_guid, 2, DB_MODE_RW);
}


inline static void cblas_dsyrk_task_prescriber ( ocrGuid_t edtTemp, u32 k, u32 j, u32 i,
                                                 u32 tileSize, ocrGuid_t*** lkji_event_guids) {
    ocrGuid_t cblas_dsyrk_task_guid;

    u64 func_args[5];
    func_args[0] = k;
    func_args[1] = j;
    func_args[2] = i;
    func_args[3] = tileSize;
    func_args[4] = (u64)(lkji_event_guids[j][j][k+1]);

    ocrGuid_t affinity = NULL_GUID;
    ocrEdtCreate(&cblas_dsyrk_task_guid, edtTemp, 5, func_args, 2, NULL, PROPERTIES, affinity, NULL);

    ocrAddDependence(lkji_event_guids[j][j][k], cblas_dsyrk_task_guid, 0, DB_MODE_RW);
    ocrAddDependence(lkji_event_guids[j][k][k+1], cblas_dsyrk_task_guid, 1, DB_MODE_RW);
}

inline static void wrap_up_task_prescriber ( ocrGuid_t edtTemp, u32 numTiles, u32 tileSize, u32 outSelLevel,
                                             ocrGuid_t*** lkji_event_guids ) {
    u32 i,j,k;
    ocrGuid_t wrap_up_task_guid;

    u64 func_args[3];
    func_args[0]=(u32)numTiles;
    func_args[1]=(u32)tileSize;
    func_args[2]=(u32)outSelLevel;

    ocrGuid_t affinity = NULL_GUID;
    ocrEdtCreate(&wrap_up_task_guid, edtTemp, 3, func_args, (numTiles+1)*numTiles/2, NULL, PROPERTIES, affinity, NULL);

    u32 index = 0;
    for ( i = 0; i < numTiles; ++i ) {
        k = 1;
        for ( j = 0; j <= i; ++j ) {
            ocrAddDependence(lkji_event_guids[i][j][k], wrap_up_task_guid, index++, DB_MODE_RW);
            ++k;
        }
    }
}

inline static ocrGuid_t*** allocateCreateEvents ( u32 numTiles ) {
    u32 i,j,k;
    ocrGuid_t*** lkji_event_guids;
    void* lkji_event_guids_db = NULL;
    ocrGuid_t lkji_event_guids_db_guid;

    ocrDbCreate(&lkji_event_guids_db_guid, &lkji_event_guids_db, sizeof(ocrGuid_t **)*numTiles,
                FLAGS, NULL_GUID, NO_ALLOC);

    lkji_event_guids = (ocrGuid_t ***) (lkji_event_guids_db);
    for( i = 0 ; i < numTiles ; ++i ) {
        ocrDbCreate(&lkji_event_guids_db_guid, (void *)&lkji_event_guids[i],
                    sizeof(ocrGuid_t *)*(i+1),
                    FLAGS, NULL_GUID, NO_ALLOC);
        for( j = 0 ; j <= i ; ++j ) {
            ocrDbCreate(&lkji_event_guids_db_guid, (void *)&lkji_event_guids[i][j],
                        sizeof(ocrGuid_t)*(numTiles+1),
                        FLAGS, NULL_GUID, NO_ALLOC);
            for( k = 0 ; k <= numTiles ; ++k ) {
                ocrEventCreate(&(lkji_event_guids[i][j][k]), OCR_EVENT_STICKY_T, TRUE);
            }
        }
    }

    return lkji_event_guids;
}

#ifdef TG_ARCH
inline static void satisfyInitialTiles(u32 numTiles, u32 tileSize,
                                       ocrGuid_t*** lkji_event_guids) {
    u32 i,j;
    u32 T_i, T_j;
    ocrGuid_t db_guid;
    ocrGuid_t db_affinity;
    void* temp_db;
    FILE *fin;

    fin = fopen("inputfile", "r");
    if(fin == NULL) PRINTF("Error opening input file\n");
    for( i = 0 ; i < numTiles ; ++i ) {
        for( j = 0 ; j <= i ; ++j ) {
            ocrDbCreate(&db_guid, &temp_db, sizeof(double)*tileSize*tileSize,
                        FLAGS, db_affinity, NO_ALLOC);

            fread(temp_db, sizeof(double)*tileSize*tileSize, 1, fin);
            ocrEventSatisfy(lkji_event_guids[i][j][0], db_guid);
            ocrDbRelease(db_guid);
        }
    }
    hal_fence();
    fclose(fin);

}
#else
inline static void satisfyInitialTiles(u32 numTiles, u32 tileSize, double** matrix,
                                       ocrGuid_t*** lkji_event_guids) {
    u32 i,j,index;
    u32 A_i, A_j, T_i, T_j;

    for( i = 0 ; i < numTiles ; ++i ) {
        for( j = 0 ; j <= i ; ++j ) {
            ocrGuid_t db_guid;
            ocrGuid_t db_affinity = NULL_GUID;
            void* temp_db;
            ocrGuid_t tmpdb_guid;
            ocrDbCreate(&db_guid, &temp_db, sizeof(double)*tileSize*tileSize,
                        FLAGS, db_affinity, NO_ALLOC);

            double* temp = (double*) temp_db;
            double** temp2D;

            ocrDbCreate(&tmpdb_guid, (void *)&temp2D, sizeof(double*)*tileSize,
                        FLAGS, NULL_GUID, NO_ALLOC);

            for( index = 0; index < tileSize; ++index )
                temp2D [index] = &(temp[index*tileSize]);

            // Split the matrix u32o tiles and write it u32o the item space at time 0.
            // The tiles are indexed by tile indices (which are tag values).
            for( A_i = i*tileSize, T_i = 0 ; T_i < tileSize; ++A_i, ++T_i ) {
                for( A_j = j*tileSize, T_j = 0 ; T_j < tileSize; ++A_j, ++T_j ) {
                    temp2D[ T_i ][ T_j ] = matrix[ A_i ][ A_j ];
                }
            }
            ocrEventSatisfy(lkji_event_guids[i][j][0], db_guid);
            ocrDbDestroy(tmpdb_guid);
        }
    }
}
#endif

ocrGuid_t mainEdt(u32 paramc, u64 *paramv, u32 depc, ocrEdtDep_t depv[]) {
    u32 matrixSize = -1;
    u32 tileSize = -1;
    u32 numTiles = -1;
    u32 i, j, k, c;
    u32 outSelLevel = 2;
    double **matrix, ** temp;
    u64 argc;

    void *programArg = depv[0].ptr;
    argc = getArgc(programArg);

    char *nparamv[argc];

    char *fileNameIn, *fileNameOut = "ocr_mkl_cholesky.out";

    for (i=0; i< argc; i++)
    {
        nparamv[i] = getArgv(programArg, i);
    }

    if ( argc == 1) {
        PRINTF("OCR-MKL Cholesky\n");
        PRINTF("__________________________________________________________________________________________________\n");
        PRINTF("Solves an OCR version of a Tiled Cholesky Decomposition with all math kernels using Intel MKL only\n\n");
        PRINTF("Usage:\n");
        PRINTF("\tocr_mkl_cholesky.exe {Arguments}\n\n");
        PRINTF("Arguments:\n");
        PRINTF("\t--ds -- Specify the Size of the Input Matrix\n");
        PRINTF("\t--ts -- Specify the Tile Size\n");
        PRINTF("\t--fi -- Specify the Input File Name of the Matrix\n");
//        PRINTF("\t--fo -- Specify an Output File Name (default: ocr_mkl_cholesky.out)\n");
        PRINTF("\t--ol -- Output Selection Level:\n");
        PRINTF("\t\t0: Print solution to stdout\n");
        PRINTF("\t\t1: Write solution to text file\n");
        PRINTF("\t\t2: Write solution to binary file (default)\n");
        PRINTF("\t\t3: Write solution to text file and print to stdout\n");
        PRINTF("\t\t4: Write solution to binary file and print to stdout\n");
        PRINTF("\t\t5: Write algorithm timing data to ocr_mkl_cholesky_stats.csv and write solution to binary file\n");

        ocrShutdown();
        return NULL_GUID;
    }
    else
    {
        // Reads in 4 arguments, input matrix file name, output matrix filename, datasize, and tilesize
        while (1)
        {
            static struct option long_options[] =
                {
                    {"ds", required_argument, 0, 'a'},
                    {"ts", required_argument, 0, 'b'},
                    {"fi", required_argument, 0, 'c'},
                    {"fo", required_argument, 0, 'd'},
                    {"ol", required_argument, 0, 'e'},
                    {0, 0, 0, 0}
                };

            u32 option_index = 0;

            c = getopt_long(argc, nparamv, "a:b:c:d:e", long_options, &option_index);

            if (c == -1) // Detect the end of the options
                break;
            switch (c)
            {
            case 'a':
                //PRINTF("Option a: matrixSize with value '%s'\n", optarg);
                matrixSize = (u32) atoi(optarg);
                break;
            case 'b':
                //PRINTF("Option b: tileSize with value '%s'\n", optarg);
                tileSize = (u32) atoi(optarg);
                break;
            case 'c':
                //PRINTF("Option c: fileNameIn with value '%s'\n", optarg);
                fileNameIn = optarg;
                break;
            case 'd':
                //PRINTF("Option d: fileNameOut with value '%s'\n", optarg);
                fileNameOut = (char*) mkl_realloc(fileNameOut, sizeof(optarg));
                strcpy(fileNameOut, optarg);
                break;
            case 'e':
                //PRINTF("Option e: outSelLevel with value '%s'\n", optarg);
                outSelLevel = (u32) atoi(optarg);
                break;
            default:
                PRINTF("ERROR: Invalid argument switch\n\n");
                PRINTF("OCR-MKL Cholesky\n");
                PRINTF("__________________________________________________________________________________________________\n");
                PRINTF("Solves an OCR version of a Tiled Cholesky Decomposition with all math kernels using Intel MKL only\n\n");
                PRINTF("Usage:\n");
                PRINTF("\tocr_mkl_cholesky.exe {Arguments}\n\n");
                PRINTF("Arguments:\n");
                PRINTF("\t--ds -- Specify the Size of the Input Matrix\n");
                PRINTF("\t--ts -- Specify the Tile Size\n");
                PRINTF("\t--fi -- Specify the Input File Name of the Matrix\n");
//                PRINTF("\t--fo -- Specify an Output File Name (default: ocr_mkl_cholesky.out)\n");
                PRINTF("\t--ol -- Output Selection Level:\n");
                PRINTF("\t\t0: Print solution to stdout\n");
                PRINTF("\t\t1: Write solution to text file\n");
                PRINTF("\t\t2: Write solution to binary file (default)\n");
                PRINTF("\t\t3: Write solution to text file and print to stdout\n");
                PRINTF("\t\t4: Write solution to binary file and print to stdout\n");
                PRINTF("\t\t5: Write algorithm timing data to ocr_mkl_cholesky_stats.csv and write solution to binary file\n");

                ocrShutdown();
                return NULL_GUID;
            }
        }
    }

    if(matrixSize == -1 || tileSize == -1)
    {
        PRINTF("Must specify matrix size and tile size\n");
        ocrShutdown();
        return NULL_GUID;
    }
    else if(matrixSize % tileSize != 0)
    {
        PRINTF("Incorrect tile size %d for the matrix of size %d \n", tileSize, matrixSize);
        ocrShutdown();
        return NULL_GUID;
    }

    numTiles = matrixSize/tileSize;

    PRINTF("Matrixsize %d, tileSize %d\n", matrixSize, tileSize);

#ifndef TG_ARCH
    struct timeval a;
    if(outSelLevel == 5)
    {
        FILE* outCSV = fopen("ocr_mkl_cholesky_stats.csv", "r");
        if( !outCSV )
        {

            outCSV = fopen("ocr_mkl_cholesky_stats.csv", "w");

            if( !outCSV ) {
                PRINTF("Cannot find file: %s\n", "ocr_mkl_cholesky_stats.csv");
                ocrShutdown();
                return NULL_GUID;
            }

            fprintf(outCSV, "MatrixSize,TileSize,NumTile,PreAllocTime,PreAlgorithmTime,PostAlgorithmTime\n");
        }
	else
        {
            outCSV = fopen("ocr_mkl_cholesky_stats.csv", "a+");
        }

        fprintf(outCSV, "%d,%d,%d,", matrixSize, tileSize, numTiles);
        gettimeofday(&a, 0);
        fprintf(outCSV, "%f,", (a.tv_sec*1000000+a.tv_usec)*1.0/1000000);
        fclose(outCSV);
    }

    FILE *in;
    in = fopen(fileNameIn, "r");
    if( !in ) {
        PRINTF("Cannot find file: %s\n", fileNameIn);
        ocrShutdown();
        return NULL_GUID;
    }

    matrix = readMatrix(matrixSize, in);

    if(outSelLevel == 5)
    {
        FILE* outCSV = fopen("ocr_mkl_cholesky_stats.csv", "a");
        if( !outCSV ) {
            PRINTF("Cannot find file: %s\n", "ocr_mkl_cholesky_stats.csv");
            ocrShutdown();
            return NULL_GUID;
        }
        gettimeofday(&a, 0);
        fprintf(outCSV, "%f,", (a.tv_sec*1000000+a.tv_usec)*1.0/1000000);
        fclose(outCSV);
    }
#endif


    ocrGuid_t*** lkji_event_guids = allocateCreateEvents(numTiles);

    ocrGuid_t templateSeq, templateTrisolve,
        templateUpdateNonDiag, templateUpdate, templateWrap;

    ocrEdtTemplateCreate(&templateSeq, lapacke_dpotrf_task, 3, 1);
    ocrEdtTemplateCreate(&templateTrisolve, cblas_dtrsm_task, 4, 2);
    ocrEdtTemplateCreate(&templateUpdateNonDiag, cblas_dgemm_task, 5, 3);
    ocrEdtTemplateCreate(&templateUpdate, cblas_dsyrk_task, 5, 2);
    ocrEdtTemplateCreate(&templateWrap, wrap_up_task, 3, (numTiles+1)*numTiles/2);

//    PRINTF("Going to satisfy initial tiles\n");
#ifdef TG_ARCH
    satisfyInitialTiles(numTiles, tileSize, lkji_event_guids);
#else
    satisfyInitialTiles(numTiles, tileSize, matrix, lkji_event_guids);
#endif

    for ( k = 0; k < numTiles; ++k ) {
//        PRINTF("Prescribing sequential task %d\n", k);
        lapacke_dpotrf_task_prescriber(templateSeq, k, tileSize, lkji_event_guids);

        for( j = k + 1 ; j < numTiles ; ++j ) {
            cblas_dtrsm_task_prescriber (templateTrisolve,
                                          k, j, tileSize, lkji_event_guids);

            for( i = k + 1 ; i < j ; ++i ) {
                cblas_dgemm_task_prescriber (templateUpdateNonDiag,
                                              k, j, i, tileSize, lkji_event_guids);
            }
            cblas_dsyrk_task_prescriber (templateUpdate,
                                          k, j, i, tileSize, lkji_event_guids);
        }
    }

    wrap_up_task_prescriber (templateWrap, numTiles, tileSize,
                             outSelLevel, lkji_event_guids);

//    PRINTF("Wrapping up mainEdt\n");
    return NULL_GUID;
}

#ifndef TG_ARCH
static double** readMatrix( u32 matrixSize, FILE* in ) {
    u32 i,j;
    double **A = (double**) mkl_malloc(sizeof(double*)*matrixSize, INTEL_BB);

    for( i = 0; i < matrixSize; ++i)
        A[i] = (double*) mkl_malloc(sizeof(double)*matrixSize, INTEL_BB);

    for( i = 0; i < matrixSize; ++i ) {
        for( j = 0; j < matrixSize-1; ++j )
            fscanf(in, "%lf ", &A[i][j]);
        fscanf(in, "%lf\n", &A[i][j]);
    }
    return A;
}
#endif

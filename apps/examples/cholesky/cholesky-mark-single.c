/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

#define __OCR__
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/time.h>
#include <ocr.h>

/* Single-node Cholesky example.  Based on the stock OCR example, this
 * version reduces the number of Events, parallelizes the startup costs,
 * and avoids passing pointers in paramv[] (which won't work across shared
 * memory domains).
 *
 * NOTE: Distributed OCR on x86 is (at time of writing) optimizing for SSA-
 * like semantics for data block accesses.  To convert this application to
 * operate in a strict SSA-like fashion, uncomment the following define.
 *
 * Without this, the application is still loosely SSA-like, in the sense
 * that only "finalized" data crosses tile boundaries.  If locality is
 * assigned according to the tile [row,column] tuple, tiles will always be
 * fully computed before they cross the network boundary.  That's enough for
 * static workload assignment, but won't be enough for automatic task
 * migration. */
//#define SSA_SEMANTICS

struct timeval a,b;

#define FLAGS DB_PROP_SINGLE_ASSIGNMENT
#define PROPERTIES EDT_PROP_NONE

ocrGuid_t shutdown_template;

/* number of entries in an array */
#define LENGTH(a) (sizeof((a))/sizeof((a)[0]))

/*
 * The global guid array is formatted as follows:
 * * [0] self-reference (the guid of the global guid array)
 * * [1] the sequential solver edt template guid
 * * [2] the triangle solver edt template guid
 * * [3] the diagonal update edt template guid
 * * [4] the non-diagonal update edt template guid
 * * [5+] a packed array of tile-related guids
 *
 * This array is passed to all of the tile operation EDTs as the first entry in
 * depv[].  It is used for scheduling the next task, and specifying the input
 * dependencies.
 *
 * For each tile, there is one Event GUID which is depended upon by other
 * compute nodes.  The Event GUID is satisfied with a DB GUID once the tile is
 * finalized (as determined by task_done()).
 *
 * Note that GUID hashes are a good substitute for this; once they are
 * introduced in OCR, this global guid array can go away.
 */
enum { GLOBAL_SELF, GLOBAL_SEQ, GLOBAL_TRISOLVE, GLOBAL_UPDATE, GLOBAL_UPDATE_NON_DIAG, _GLOBAL_PACKED_OFFSET };
#define GLOBALIDX(i,j) ((((i)+1)*(i)/2 + (j)) + _GLOBAL_PACKED_OFFSET)
#define GLOBAL(E,i,j) ((E)[GLOBALIDX(i,j)])

static void task_done(int i, int j, int k, int tileSize, const ocrGuid_t *globals, const ocrGuid_t data_guid) {
    ocrGuid_t task_guid, affinity = NULL_GUID;
    u64 func_args[4];
    func_args[0] = i;
    func_args[1] = j;
    func_args[2] = k+1;
    func_args[3] = tileSize;
//    printf("task_done globals=%p globals_guid=%ld\n", globals, data_guid);

    if(k == j) {
        /* tile is finalized, post the OUT event */
//        printf("tile %d,%d finalized at version %d\n", i, j, k+1);
        ocrEventSatisfy(GLOBAL(globals,i,j), data_guid);
        return;
    } else
    if(k == j-1) {
        /* Next up is the final event for this tile */
        if(i == j) {
//            printf("tile %d,%d next up: potrf template: %ld\n", i, j, globals[GLOBAL_SEQ]);
            ocrEdtCreate(&task_guid, globals[GLOBAL_SEQ], 4, func_args, 2, NULL, PROPERTIES, affinity, NULL);
        } else {
//            printf("tile %d,%d next up: trsm template: %ld\n", i, j, globals[GLOBAL_TRISOLVE]);
            ocrEdtCreate(&task_guid, globals[GLOBAL_TRISOLVE], 4, func_args, 3, NULL, PROPERTIES, affinity, NULL);
            ocrAddDependence(GLOBAL(globals,k+1,k+1), task_guid, 2, DB_MODE_RO);  /* up */
        }
    } else {
        /* Next up is an intermediate computation */
        if(i == j) {
//            printf("tile %d,%d next up: syrk template: %ld\n", i, j, globals[GLOBAL_UPDATE]);
            ocrEdtCreate(&task_guid, globals[GLOBAL_UPDATE], 4, func_args, 3, NULL, PROPERTIES, affinity, NULL);
            ocrAddDependence(GLOBAL(globals,i,k+1), task_guid, 2, DB_MODE_RO); /* left */
        } else {
//            printf("tile %d,%d next up: gemm template: %ld\n", i, j, globals[GLOBAL_UPDATE_NON_DIAG]);
            ocrEdtCreate(&task_guid, globals[GLOBAL_UPDATE_NON_DIAG], 4, func_args, 4, NULL, PROPERTIES, affinity, NULL);
            ocrAddDependence(GLOBAL(globals,i,k+1), task_guid, 2, DB_MODE_RO); /* left */
            ocrAddDependence(GLOBAL(globals,j,k+1), task_guid, 3, DB_MODE_RO); /* diagonal */
        }
    }
    ocrAddDependence(data_guid,task_guid,0,DB_MODE_ITW);
    ocrAddDependence(globals[GLOBAL_SELF],task_guid,1,DB_MODE_RO);
}

static ocrGuid_t sequential_cholesky_task ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    /* LAPACK: DPOTF2 */
    assert(paramc == 4);
    assert(depc   == 2);

    int i = (int)paramv[0];
    int j = (int)paramv[1];
    int k = (int)paramv[2];
    int tileSize = (int)paramv[3];

    int iB = 0, jB = 0, kB = 0;
    double* aBlock = (double*) (depv[0].ptr);
    const ocrGuid_t *globals = (ocrGuid_t*)(depv[1].ptr);
    PRINTF("RUNNING sequential_cholesky %d\n", k);

    ocrGuid_t out_guid = depv[0].guid;
    double *out = aBlock;
#ifdef SSA_SEMANTICS
    ocrDbCreate(&out_guid, (void**)&out, tileSize*tileSize*sizeof(double), FLAGS, NULL_GUID, NO_ALLOC);
    memcpy(out, aBlock, tileSize*tileSize*sizeof(double));
#endif /* SSA_SEMANTICS */

    for( kB = 0 ; kB < tileSize ; ++kB ) {
        if( aBlock[kB*tileSize+kB] <= 0 ) {
            PRINTF("Not a symmetric positive definite (SPD) matrix\n");
            ASSERT(0);
            ocrShutdown();
            return NULL_GUID;
        } else {
            out[kB*tileSize+kB] = sqrt(out[kB*tileSize+kB]);
        }

        for(jB = kB + 1; jB < tileSize ; ++jB ) {
            out[jB*tileSize+kB] = out[jB*tileSize+kB]/out[kB*tileSize+kB];
        }

        for(jB = kB + 1; jB < tileSize ; ++jB ) {
            for(iB = jB ; iB < tileSize ; ++iB ) {
                out[iB*tileSize+jB] -= out[iB*tileSize+kB] * out[jB*tileSize+kB];
            }
        }
    }

    task_done(i, j, k, tileSize, globals, out_guid);

    return NULL_GUID;
}

static ocrGuid_t trisolve_task ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    /* LAPACK: DTRSM */
    assert(paramc == 4);
    assert(depc   == 3);

    int i = (int)paramv[0];
    int j = (int)paramv[1];
    int k = (int)paramv[2];
    int tileSize = (int)paramv[3];

    int iB, jB, kB;
    double* aBlock = (double*) (depv[0].ptr);
    const ocrGuid_t *globals = (ocrGuid_t*)(depv[1].ptr);
    const double* liBlock = (double*) (depv[2].ptr);
//    PRINTF("RUNNING trisolve (%d, %d, %d)\n", i, j, k);

    ocrGuid_t out_guid = depv[0].guid;
    double *out = aBlock;
#ifdef SSA_SEMANTICS
    ocrDbCreate(&out_guid, (void**)&out, tileSize*tileSize*sizeof(double), FLAGS, NULL_GUID, NO_ALLOC);
    memcpy(out, aBlock, tileSize*tileSize*sizeof(double));
#endif /* SSA_SEMANTICS */

    for( kB = 0; kB < tileSize ; ++kB ) {
        for( iB = 0; iB < tileSize ; ++iB ) {
            out[iB*tileSize+kB] /= liBlock[kB*tileSize+kB];
        }

        for( jB = kB + 1 ; jB < tileSize; ++jB ) {
            for( iB = 0; iB < tileSize; ++iB ) {
                out[iB*tileSize+jB] -= liBlock[jB*tileSize+kB] * out[iB*tileSize+kB];
            }
        }
    }

    task_done(i, j, k, tileSize, globals, out_guid);

    return NULL_GUID;
}

static ocrGuid_t update_diagonal_task ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    /* LAPACK: DSYRK */
    assert(paramc == 4);
    assert(depc   == 3);

    int i = (int)paramv[0];
    int j = (int)paramv[1];
    int k = (int)paramv[2];
    int tileSize = (int)paramv[3];

    int iB, jB, kB;
    double* aBlock = (double*) (depv[0].ptr);
    const ocrGuid_t *globals = (ocrGuid_t*)(depv[1].ptr);
    const double* l2Block = (double*) (depv[2].ptr);
//    PRINTF("RUNNING update_diagonal (%d, %d, %d)\n", k, j, i);

    ocrGuid_t out_guid = depv[0].guid;
    double *out = aBlock;
#ifdef SSA_SEMANTICS
    ocrDbCreate(&out_guid, (void**)&out, tileSize*tileSize*sizeof(double), FLAGS, NULL_GUID, NO_ALLOC);
    memcpy(out, aBlock, tileSize*tileSize*sizeof(double));
#endif /* SSA_SEMANTICS */

    for( jB = 0; jB < tileSize ; ++jB ) {
        for( kB = 0; kB < tileSize ; ++kB ) {
            double temp = 0 - l2Block[jB*tileSize+kB];
            for( iB = jB; iB < tileSize; ++iB ) {
                out[iB*tileSize+jB] += temp * l2Block[iB*tileSize+kB];
            }
        }
    }

    task_done(i, j, k, tileSize, globals, out_guid);

    return NULL_GUID;
}

static ocrGuid_t update_nondiagonal_task ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    /* LAPACK: DGEMM */
    assert(paramc == 4);
    assert(depc   == 4);

    int i = (int)paramv[0];
    int j = (int)paramv[1];
    int k = (int)paramv[2];
    int tileSize = (int)paramv[3];

    int jB, kB, iB;
    double* aBlock = (double*) (depv[0].ptr);
    const ocrGuid_t *globals = (ocrGuid_t*)(depv[1].ptr);
    const double* l1Block = (double*) (depv[2].ptr);
    const double* l2Block = (double*) (depv[3].ptr);
//    PRINTF("RUNNING update_nondiagonal (%d, %d, %d)\n", k, j, i);

    ocrGuid_t out_guid = depv[0].guid;
    double *out = aBlock;
#ifdef SSA_SEMANTICS
    ocrDbCreate(&out_guid, (void**)&out, tileSize*tileSize*sizeof(double), FLAGS, NULL_GUID, NO_ALLOC);
    memcpy(out, aBlock, tileSize*tileSize*sizeof(double));
#endif /* SSA_SEMANTICS */

    for( jB = 0; jB < tileSize ; ++jB ) {
        for( kB = 0; kB < tileSize ; ++kB ) {
            double temp = 0 - l2Block[jB*tileSize+kB];
            for(iB = 0; iB < tileSize; ++iB)
                out[iB*tileSize+jB] += temp * l1Block[iB*tileSize+kB];
        }
    }

    task_done(i, j, k, tileSize, globals, out_guid);

    return NULL_GUID;
}

static ocrGuid_t wrap_up_task ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    int i, j, i_b, j_b;
    FILE *out = fopen("cholesky.out", "w");

    u64 *func_args = paramv;
    int numTiles = (int) func_args[0];
    int tileSize = (int) func_args[1];

    for ( i = 0; i < numTiles; ++i ) {
        for( i_b = 0; i_b < tileSize; ++i_b) {
            for ( j = 0; j <= i; ++j ) {
                int idx = i*(i+1)/2+j+1;
                const double *temp = (double*) (depv[idx].ptr);
                if(i != j) {
                    for(j_b = 0; j_b < tileSize; ++j_b) {
                        fprintf( out, "%lf ", temp[i_b*tileSize+j_b]);
                    }
                } else {
                    for(j_b = 0; j_b <= i_b; ++j_b) {
                        fprintf( out, "%lf ", temp[i_b*tileSize+j_b]);
                    }
                }
            }
        }
    }
    fclose(out);

    gettimeofday(&b,0);
    PRINTF("The computation took %f seconds\r\n", ((b.tv_sec - a.tv_sec)*1000000+(b.tv_usec - a.tv_usec))*1.0/1000000);

    ocrShutdown();
    return NULL_GUID;
}

inline static void wrap_up_task_prescriber (int numTiles, int tileSize, ocrGuid_t* globals) {
    int i, j;
    ocrGuid_t wrap_up_task_guid;

    u64 func_args[3];
    func_args[0]=(int)numTiles;
    func_args[1]=(int)tileSize;

    ocrGuid_t template;
    ocrEdtTemplateCreate(&template, wrap_up_task, 2, (numTiles+1)*numTiles/2+1);


    ocrEdtCreate(&wrap_up_task_guid, template, 2, func_args, (numTiles+1)*numTiles/2+1, NULL, PROPERTIES, NULL_GUID, NULL);

    int index = 1;
    for ( i = 0; i < numTiles; ++i ) {
        for ( j = 0; j <= i; ++j ) {
            ocrAddDependence(GLOBAL(globals,i,j), wrap_up_task_guid, index++, DB_MODE_RO);
        }
    }
    ocrAddDependence(globals[0], wrap_up_task_guid, 0, DB_MODE_RO);
}


static ocrGuid_t satisfy_initial_tile(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);
static ocrGuid_t readMatrix(int matrixSize, FILE* in);

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    int matrixSize = -1;
    int tileSize = -1;
    int numTiles = -1;
    int i, j;
    u64 argc;

    const void *programArg = depv[0].ptr;
    u64* dbAsU64 = (u64*)programArg;
    argc = dbAsU64[0];

    if ( argc !=  4 ) {
        printf("Usage: ./cholesky matrixSize tileSize fileName (found %"PRIu64" args)\n", argc);
        return 1;
    }

    u64* offsets = (u64*)MALLOC(argc*sizeof(u64));
    for (i=0; i< argc; i++)
        offsets[i] = dbAsU64[i+1];
    char *dbAsChar = (char*)programArg;

    matrixSize = atoi(dbAsChar+offsets[1]);
    tileSize = atoi(dbAsChar+offsets[2]);

    if ( matrixSize % tileSize != 0 ) {
        PRINTF("Incorrect tile size %d for the matrix of size %d \n", tileSize, matrixSize);
        ocrShutdown();
        return NULL_GUID;
    }

    numTiles = matrixSize/tileSize;

    ocrGuid_t *globals = NULL, db_globals = 0;

    char *fn = dbAsChar+offsets[3];

    ocrDbCreate(&db_globals, (void**)&globals, sizeof(ocrGuid_t)*GLOBALIDX(numTiles-1,numTiles-1),
             FLAGS, NULL_GUID, NO_ALLOC);
    memset(globals, 0, sizeof(ocrGuid_t)*GLOBALIDX(numTiles-1,numTiles-1));
    for( i = 0 ; i < numTiles ; ++i ) {
        for( j = 0 ; j <= i ; ++j ) {
            ocrEventCreate(&GLOBAL(globals,i,j), OCR_EVENT_STICKY_T, 1);
        }
    }

    FILE *in = fopen(fn, "r");
    if( !in ) {
        PRINTF("Cannot find file %s\n", fn);
        ocrShutdown();
        return NULL_GUID;
    }
    ocrGuid_t matrix = readMatrix( tileSize*numTiles, in );
    fclose(in);

    globals[GLOBAL_SELF] = db_globals;

    ocrEdtTemplateCreate(&globals[GLOBAL_SEQ]            , sequential_cholesky_task, 4, 2);
    ocrEdtTemplateCreate(&globals[GLOBAL_TRISOLVE]       , trisolve_task           , 4, 3);
    ocrEdtTemplateCreate(&globals[GLOBAL_UPDATE]         , update_diagonal_task    , 4, 3);
    ocrEdtTemplateCreate(&globals[GLOBAL_UPDATE_NON_DIAG], update_nondiagonal_task , 4, 4);

    ocrGuid_t satisfy_initial_template;
    ocrEdtTemplateCreate(&satisfy_initial_template, satisfy_initial_tile, 4, 2);
    for(i = 0; i < numTiles; i++) {
        for(j = 0; j <= i; j++) {
            ocrGuid_t initial;
            u64 args[] = {numTiles, tileSize, i, j};
            ocrGuid_t deps[] = {db_globals, matrix};
            ocrEdtCreate(&initial, satisfy_initial_template, LENGTH(args), args, 2, deps, PROPERTIES, NULL_GUID, NULL);
        }
    }

    gettimeofday(&a,0);

    wrap_up_task_prescriber (numTiles, tileSize, globals);

    return NULL_GUID;
}

static ocrGuid_t satisfy_initial_tile(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    assert(paramc == 4);
    assert(depc   == 2);
    int numTiles = (int)paramv[0];
    int tileSize = (int)paramv[1];
    int Tilei    = (int)paramv[2];
    int Tilej    = (int)paramv[3];
    const ocrGuid_t *globals = (ocrGuid_t *)depv[0].ptr;
    const double    *in      = (double*)    depv[1].ptr;

    int matrixSize = numTiles * tileSize;
    int Li, Lj, i, j, DBi = 0;

    ocrGuid_t out_guid;
    double *out;
    ocrDbCreate(&out_guid, (void**)&out, tileSize*tileSize*sizeof(double), FLAGS, NULL_GUID, NO_ALLOC);

    /* Extract the tile */
    for(i = Tilei*tileSize, Li = 0; Li < tileSize; i++, Li++) {
        for(j = Tilej*tileSize, Lj = 0 ; Lj < tileSize; j++, Lj++) {
            out[DBi++] = in[i*matrixSize + j];
        }
    }
    /* Spawn the first task on it (indicate that task number -1 is done) */
    task_done(Tilei, Tilej, -1, tileSize, globals, out_guid);
    return NULL_GUID;
}

static ocrGuid_t readMatrix( int matrixSize, FILE* in ) {
    int i,j,index;
    ocrGuid_t rv = 0, affinity = 0;
    double *buffer;
    ocrDbCreate(&rv, (void**)&buffer, sizeof(double)*matrixSize*matrixSize, FLAGS, affinity, NO_ALLOC);
    printf("readMatrix: rv=%#lx buffer=%p\n", rv, buffer);
    if(!rv || !buffer) {
        printf("Failed to alloc DB of %zd bytes.\n"
               "Please fix your OCR config file.\n", sizeof(double)*matrixSize*matrixSize);
        exit(1);
    }
    for(index = 0, i = 0; i < matrixSize; ++i ) {
        for( j = 0; j < matrixSize; ++j )
            fscanf(in, "%lf ", &buffer[index++]);
        fscanf(in, "%lf\n", &buffer[index++]);
    }
    return rv;
}

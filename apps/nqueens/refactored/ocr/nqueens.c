// \file nqueens.c
// \author Jorge Bellon <jorge.bellon.castro@intel.com>
//

#include "nqueens.h"

#include <ocr.h>
#include <stdlib.h>

u32 solutions = 0U;

ocrGuid_t findTemplate;
ocrGuid_t shutdownTemplate;

static inline void create_task( struct nqueens_args* args, ocrGuid_t* output )
{
    u32 paramc = sizeof(struct nqueens_args)/sizeof(u64)+1;
    u64* paramv = (u64*)args;

    ocrGuid_t edt;
    u8 err = ocrEdtCreate( &edt, findTemplate, paramc, paramv, 0, NULL,
                  EDT_PROP_FINISH, NULL_HINT, output );
    ASSERT( err == 0 );
}

static inline void find_solutions( const struct nqueens_args* args, u8 final )
{
    if( args->cols != args->all ) {
        u32 available = ~( args->ldiag | args->cols | args->rdiag ) & args->all;
        u32 spot = available & (-available);

        struct nqueens_args arguments;
        arguments.all = args->all;
        arguments.max_set = args->max_set;

        while( spot != 0 ) {
            arguments.ldiag   = (args->ldiag|spot)<<1;
            arguments.cols    = (args->cols |spot);
            arguments.rdiag   = (args->rdiag|spot)>>1;

            if( final ) {
                find_solutions( &arguments, final );
            } else {
                create_task( &arguments, NULL );
            }

            available = available - spot;
            spot = available & (-available);
        }
    } else {
        // A solution was found!
        solution_found();
    }
}

// Find solutions: recursive EDT
ocrGuid_t findSolutionsEdt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    // Decode EDT paramv
    const struct nqueens_args* args = (struct nqueens_args*)paramv;
    const u8 final = NumberOfSetBits(args->cols) > args->max_set;

    find_solutions( args, final );

    return NULL_GUID;
}

void solve_nqueens( u32 n, u32 cutoff )
{
    struct nqueens_args arguments = {
        .max_set = n-cutoff, .all = (1 << n) - 1,
        .ldiag = 0, .cols = 0, .rdiag = 0 };

    ocrGuid_t outEvent;
    create_task( &arguments, &outEvent );

    ocrGuid_t shutdownEdt;
    u64 paramv = n;
    u8 err = ocrEdtCreate( &shutdownEdt, shutdownTemplate,
                  1, &paramv, 1, &outEvent,
                  EDT_PROP_NONE, NULL_HINT, NULL );
    ASSERT( err == 0 );
}

ocrGuid_t shutdown( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    u32 found = get_solution_number();
    PRINTF( "%d-queens; %dx%d; sols: %d\n",
            paramv[0], paramv[0], paramv[0], found );

    u8 err;
    err = ocrEdtTemplateDestroy( findTemplate );
    ASSERT( err == 0 );

    err = ocrEdtTemplateDestroy( shutdownTemplate );
    ASSERT( err == 0 );

    ocrShutdown();
}

ocrGuid_t mainEdt ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    if( getArgc(depv[0].ptr) != 3 ) {
        PRINTF("Usage %s size cutoff", getArgv(depv[0].ptr,0) );
        ocrAbort(EXIT_FAILURE);
    }

    u32 n = atoi( getArgv(depv[0].ptr,1) );
    u32 cutoff = atoi( getArgv(depv[0].ptr,2) );
    ASSERT( 0 < n && n < 31 );
    ASSERT( cutoff < n );

    u8 err;
    err = ocrEdtTemplateCreate( &findTemplate, findSolutionsEdt,
                          sizeof(struct nqueens_args)/sizeof(u64)+1, 0 );
    ASSERT( err == 0 );

    err = ocrEdtTemplateCreate( &shutdownTemplate, shutdown, 1, 1 );
    ASSERT( err == 0 );

    solve_nqueens( n, cutoff );
    return NULL_GUID;
}


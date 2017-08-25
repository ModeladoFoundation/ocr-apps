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

    u8 err;
    ocrGuid_t edt;
    if( output ) {
        err  = ocrEdtCreate( &edt, findTemplate, paramc, paramv, 0, NULL,
                  EDT_PROP_FINISH|EDT_PROP_OEVT_VALID, NULL_HINT, output );
        ocrAssert( err == 0 );
    } else {
        err  = ocrEdtCreate( &edt, findTemplate, paramc, paramv, 0, NULL,
                  EDT_PROP_FINISH, NULL_HINT, output );
        ocrAssert( err == 0 );
    }
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
    u8 err;
    ocrGuid_t outEvent;
    ocrGuid_t shutdownEdt;
    struct nqueens_args app_args = {
        .max_set = n-cutoff, .all = (1 << n) - 1,
        .ldiag = 0, .cols = 0, .rdiag = 0 };
    struct shutdown_args shutdown_args = { .n = n };

    get_time(&shutdown_args.start);

    err = ocrEventCreate( &outEvent, OCR_EVENT_ONCE_T, EVT_PROP_NONE );
    ocrAssert( err == 0 );

    err = ocrEdtCreate( &shutdownEdt, shutdownTemplate,
                  sizeof(shutdown_args)/sizeof(u64)+1, (u64*)&shutdown_args,
                  1, &outEvent,
                  EDT_PROP_NONE, NULL_HINT, NULL );
    ocrAssert( err == 0 );

    create_task( &app_args, &outEvent );
}

ocrGuid_t shutdown( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    timestamp_t stop;
    get_time(&stop);
    u32 found = get_solution_number();

    struct shutdown_args* args = (struct shutdown_args*)paramv;
    ocrPrintf( "%d-queens; %dx%d; sols: %d\n",
            args->n, args->n, args->n, found );
    summary_throughput_timer(&args->start,&stop,1);

    u8 err;
    err = ocrEdtTemplateDestroy( findTemplate );
    ocrAssert( err == 0 );

    err = ocrEdtTemplateDestroy( shutdownTemplate );
    ocrAssert( err == 0 );

    ocrShutdown();

    return NULL_GUID;
}

ocrGuid_t mainEdt ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    if( ocrGetArgc(depv[0].ptr) != 3 ) {
        ocrPrintf("Usage %s size cutoff", ocrGetArgv(depv[0].ptr,0) );
        ocrAbort(EXIT_FAILURE);
    }

    u32 n = atoi( ocrGetArgv(depv[0].ptr,1) );
    u32 cutoff = atoi( ocrGetArgv(depv[0].ptr,2) );
    ocrAssert( 0 < n && n < 31 );
    ocrAssert( cutoff < n );

    u8 err;
    err = ocrEdtTemplateCreate( &findTemplate, findSolutionsEdt,
                          sizeof(struct nqueens_args)/sizeof(u64)+1, 0 );
    ocrAssert( err == 0 );

    err = ocrEdtTemplateCreate( &shutdownTemplate, shutdown,
                          sizeof(struct shutdown_args)/sizeof(u64)+1, 1 );
    ocrAssert( err == 0 );

    solve_nqueens( n, cutoff );
    return NULL_GUID;
}


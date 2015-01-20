#include "Stencil1D_context.h"
#include "Stencil1D_defs.h"
#include<cnc/debug.h>

int Stencil1D_main( Stencil1DArgs * args, int NX, int NITER )
{
#ifdef _DIST_
    CnC::dist_cnc_init< Stencil1D_context > _dinit;
#endif

    // create our context
    Stencil1D_context ctxt( NX, NITER );
    CnC::debug::trace_all(ctxt);

    // Put initial items

    float  val0_handle [NX + 1]/* = DONE: array handle? */;



    for ( int i0 = 1; i0 < ctxt.NX-1; ++i0 ) {
        val0_handle [i0] = 0. /* = DONE: computation/initialization */;

        ctxt.val.put( val_key_t(i0, 0 ), val0_handle [i0] ); /* DONE: array handle? */
    }
    float  val1_handle [NITER + 1] /* = DONE: array handle? */;

    for ( int i1 = 0; i1 <= ctxt.NITER; ++i1 ) {
        val1_handle [i1] = 1.0/* = DONE: computation/initialization */;

        ctxt.val.put( val_key_t(0, i1 ), val1_handle [i1] ); /* DONE: array handle? */
    }
    float  val2_handle [NITER + 1] /* = DONE: array handle? */;

    for ( int i1 = 0; i1 <= ctxt.NITER; ++i1 ) {
        val2_handle [i1] = 1. /* = DONE: computation/initialization */;

        ctxt.val.put( val_key_t(ctxt.NX-1, i1 ), val2_handle [i1] ); /* DONE: array handle? */
    }

    for ( int i0 = 1; i0 < ctxt.NX-1; ++i0 )
    for ( int i1 = 0; i1 < ctxt.NITER; ++i1 )
            ctxt.updateStep.put( updateStep_tag_t(i0, i1 ) );


    // wait for completetion
    ctxt.wait();

    {
        // get the results
        float  results_handle /* = DONE: array handle? */;

    for ( int i0 = 0; i0 < ctxt.NX; ++i0 ) {

        ctxt.val.get( val_key_t(i0, ctxt.NITER ), results_handle ); /* DONE: use it */
        printf ( "Result at %d is %f\n", i0, results_handle );
    }
    }
    return 0;
}

int main( const int argc, const char** argv )
{
    /* DONE: parse arguments etc. */



    // initialize context parameters

    int NX = atoi(argv [1]); /* DONE: Initialize */
    int NITER = atoi (argv [2]); /* DONE: Initialize */


    Stencil1DArgs args /* = DONE: Initialize args struct */;

    return Stencil1D_main( &args, NX, NITER );
}

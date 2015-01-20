#include "Stencil1D.h"


void Stencil1D_init(Stencil1DArgs *args, Stencil1DCtx *ctx) {


    { // Put "val0" items
        s64 _v;
        for (_v = 1; _v < ctx->NX-1; _v++) {
            float *val0 = cncCreateItem_val(/* DONE: count=*/1);
            /* DONE: Initialize val0 */
            *val0 = 0.;
            cncPut_val(val0, _v, 0, ctx);
        }
    }

    { // Put "val1" items
        s64 _t;
        for (_t = 0; _t <= ctx->NITER; _t++) {
            float *val1 = cncCreateItem_val(/* DONE: count=*/1);
            /* DONE: Initialize val1 */
            *val1 = 1.;
            cncPut_val(val1, 0, _t, ctx);
        }
    }

    { // Put "val2" items
        s64 _t;
        for (_t = 0; _t <= ctx->NITER; _t++) {
            float *val2 = cncCreateItem_val(/* DONE: count=*/1);
            /* DONE: Initialize val2 */
            *val2 = 1.;
            cncPut_val(val2, ctx->NX-1, _t, ctx);
        }
    }

    { // Prescribe "updateStep" steps
        s64 _v, _t;
        for (_v = 1; _v < ctx->NX-1; _v++) {
            for (_t = 0; _t < ctx->NITER; _t++) {
                cncPrescribe_updateStep(_v, _t, ctx);
            }
        }
    }

    // Set finalizer function's tag
    Stencil1D_await(ctx);

}


void Stencil1D_finalize(float **results, Stencil1DCtx *ctx) {

    { // Access "results" inputs
        s64 _v;
        for (_v = 0; _v < ctx->NX; _v++) {
            /* DONE: Do something with results[_v] */
            //if ( _v % 5 == 0 ) printf ( "Result at %lu is %f\n", _v, *results[_v] );
            printf ( "Result at %lu is %f\n", _v, *results[_v] );
        }
    }
}


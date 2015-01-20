#include "Stencil1D.h"

/**
 * Step function defintion for "updateStep"
 */
void updateStep(cncTag_t v, cncTag_t t, float *center, float *left, float *right, Stencil1DCtx *ctx) {

    //
    // OUTPUTS
    //

    // Put "new" items
    float *new = cncCreateItem_val(/* DONE: count=*/1);
    /* DONE: Initialize new */
    *new  = *center  +  0.5 * ( *right - *left );
    cncPut_val(new, v, t + 1, ctx);
}

#include "SmithWaterman.h"

/**
 * Step function defintion for "initLeftStep"
 */
void SmithWaterman_initLeftStep(SmithWatermanCtx *ctx) {
    const int th = ctx->th;
    const int nth = ctx->nth;
    s64 i, ii;
    for (i = 0; i < nth; i++) {
        int *left = cncItemAlloc(sizeof(*left) * (th+1));
        for (ii=0; ii <= th; ii++) {
            // XXX - Why isn't this just 0?
            left[ii] = GAP_PENALTY*(i*th+ii);
        }
        cncPut_left(left, i, 0, ctx);
    }
}

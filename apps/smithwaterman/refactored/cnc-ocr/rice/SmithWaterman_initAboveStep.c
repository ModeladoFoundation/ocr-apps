#include "SmithWaterman.h"

/**
 * Step function defintion for "initAboveStep"
 */
void SmithWaterman_initAboveStep(SmithWatermanCtx *ctx) {
    const int tw = ctx->tw;
    const int ntw = ctx->ntw;
    s64 j, jj;
    for (j = 0; j < ntw; j++) {
        int *above = cncItemAlloc(sizeof(*above) * (tw+1));
        for (jj=0; jj <= tw; jj++) {
            // XXX - Why isn't this just 0?
            above[jj] = GAP_PENALTY*(j*tw+jj);
        }
        cncPut_above(above, 0, j, ctx);
    }
}

#include "taskbomb.h"

/**
 * Step function defintion for "quencher"
 */
void taskbomb_quencher(cncTag_t i, cncTag_t j, cncTag_t k, taskbombCtx *ctx) {

    //
    // OUTPUTS
    //
    volatile u64 x = 0;
    while (++x < 20000L);

    if (i==0 && j==X && k==Y) {
        // Put "done" items
        void *done = cncItemAlloc(sizeof(*done));
        /* TODO: Initialize done */
        cncPut_done(done, ctx);
    }

}

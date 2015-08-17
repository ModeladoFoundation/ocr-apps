#include "nqueens.h"


void nqueens_cncInitialize(nqueensArgs *args, nqueensCtx *ctx) {


    // Prescribe "placeQueen" steps
    cncPrescribe_placeQueen(0, 0, 0, 0, 0, ctx);

    // Set finalizer function's tag
    nqueens_await(ctx);

}


void nqueens_cncFinalize(u64 **solutions, nqueensCtx *ctx) {
    printf("Found %u solutions\n", ctx->n);
}



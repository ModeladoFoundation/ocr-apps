#include "SmithWaterman.h"


void SmithWaterman_cncInitialize(SeqData *data, SmithWatermanCtx *ctx) {

    // Put sequence data
    cncPut_data(data, ctx);

    // Record starting time
    struct timeval *startTime = cncItemAlloc(sizeof(*startTime));
    gettimeofday(startTime, 0);
    cncPut_startTime(startTime, ctx);

    // Seed edges
    cncPrescribe_initAboveStep(ctx);
    cncPrescribe_initLeftStep(ctx);

    { // Prescribe "swStep" steps
        s64 _i, _j;
        for (_i = 0; _i < ctx->nth; _i++) {
            for (_j = 0; _j < ctx->ntw; _j++) {
                cncPrescribe_swStep(_i, _j, ctx);
            }
        }
    }

    // Set finalizer function's tag
    SmithWaterman_await(ctx);
}


void SmithWaterman_cncFinalize(struct timeval *startTime, int *above, SmithWatermanCtx *ctx) {
    struct timeval endTime;
    gettimeofday(&endTime, 0);
    double secondsRun = endTime.tv_sec - startTime->tv_sec;
    secondsRun += (endTime.tv_usec - startTime->tv_usec) / 1000000.0;
    printf("The computation took %f seconds\n", secondsRun);
    // Print the result
    printf("score: %d\n", above[ctx->tw]);
}


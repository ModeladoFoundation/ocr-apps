#include "nqueens.h"

int cncMain(int argc, char *argv[]) {
    CNC_REQUIRE(argc == 3, "USAGE: %s BOARD_DIM NUM_SOLUTIONS\n", argv[0]);
    // Create a new graph context
    nqueensCtx *context = nqueens_create();
    context->dim = atoi(argv[1]); // board side dimension (or number of queens)
    context->n = atoi(argv[2]);   // desired number of solutions

    CNC_REQUIRE(context->dim <= 16, "Max board dim is 16\n");

    // Launch the graph for execution
    nqueens_launch(NULL, context);

    // Exit when the graph execution completes
    CNC_SHUTDOWN_ON_FINISH(context);

    return 0;
}

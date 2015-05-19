#include "Cholesky.h"
#include <string.h>

#if CHOLESKY_USE_FILE && CNCOCR_TG
#error "Only generated input supported in CnC-OCR Cholesky on FSim"
#endif

int cncMain(int argc, char *argv[]) {

#if CHOLESKY_USE_FILE
    CNC_REQUIRE(argc==4,
            "Usage: ./Cholesky matrixSize tileSize fileName (found %d args)\n", argc);
#else
    CNC_REQUIRE(argc==3,
            "Usage: ./Cholesky matrixSize tileSize (found %d args)\n", argc);
#endif

    // Create a new graph context
    CholeskyCtx *context = Cholesky_create();

    // Parse matrix dim info
    int matrixCols = atoi(argv[1]);
    int tileSize = atoi(argv[2]);
    int numTiles = matrixCols / tileSize;
    CNC_REQUIRE(matrixCols % tileSize == 0,
            "Incompatible tile size %d for the matrix of size %d\n", tileSize, matrixCols);

    // Set up arguments for new graph instantiation
    CholeskyArgs args;

#if CHOLESKY_USE_FILE
    // Matrix read from input file
    char *lastChar = args.inFile + sizeof(args.inFile) - 1;
    *lastChar = '\0';
    strncpy(args.inFile, argv[3], sizeof(args.inFile));
    CNC_REQUIRE(*lastChar == '\0',
            "Input path is longer than %d characters.\n", sizeof(args.inFile));
#endif

    // Set graph parameters
    context->numTiles = numTiles;
    context->tileSize = tileSize;

    // Launch the graph for execution
    Cholesky_launch(&args, context);

    // Exit when the graph execution completes
    CNC_SHUTDOWN_ON_FINISH(context);

    return 0;
}

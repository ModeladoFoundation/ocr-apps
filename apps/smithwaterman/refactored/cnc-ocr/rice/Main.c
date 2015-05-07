#include "SmithWaterman.h"
#include <string.h>

void copy_path(char *dest, const char *src, u32 dest_len) {
    strncpy(dest, src, dest_len);
    CNC_REQUIRE(dest[dest_len-1] == '\0', "Input path length too long (max %u)\n", dest_len);
}

int cncMain(int argc, char *argv[]) {
    CNC_REQUIRE(argc == 5, "Usage: %s tileWidth tileHeight fileName1 fileName2\n", argv[0]);

    // Tile width and height
    int tw = atoi(argv[1]);
    int th = atoi(argv[2]);
    PRINTF("Tile width:  %d\n", tw);
    PRINTF("Tile height: %d\n", th);

    // Create a new graph context
    SmithWatermanCtx *context = SmithWaterman_create();

    // Set up arguments for new graph instantiation
    SmithWatermanArgs args;
    args.tw = tw;
    args.th = th;
    copy_path(args.inpath1, argv[3], sizeof(args.inpath1));
    copy_path(args.inpath2, argv[4], sizeof(args.inpath2));

    // Exit when the graph execution completes
    CNC_SHUTDOWN_ON_FINISH(context);

    // Launch the graph for execution
    SmithWaterman_launch(&args, context);

    return 0;
}

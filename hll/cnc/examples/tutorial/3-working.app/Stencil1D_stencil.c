#include "Stencil1D.h"

#define STENCIL(left, center, right) (0.5f*(center) + 0.25f*((left) + (right)))

/**
 * Step function defintion for "stencil"
 */
void Stencil1D_stencil(cncTag_t i, cncTag_t t, float *tile, float *fromLeft, float *fromRight, Stencil1DCtx *ctx) {

    // Put "newTile" items
    s32 j;
    const s32 lastJ = TILE_SIZE - 1;
    float *newTile = cncItemCreateVector_tile(TILE_SIZE);

    // first (conditional, default=1)
    const float first = fromLeft ? *fromLeft : 1;
    newTile[0] = STENCIL(first, tile[0], tile[1]);
    // inner
    for (j=1; j<lastJ; j++) {
        newTile[j] = STENCIL(tile[j-1], tile[j], tile[j+1]);
    }
    // last (conditional, default=1)
    const float last = fromRight ? *fromRight : 1;
    newTile[lastJ] = STENCIL(tile[lastJ-1], tile[lastJ], last);

    cncPut_tile(newTile, i, t, ctx);

    // Put "toRight" items
    float *toRight = cncItemCreate_fromLeft();
    *toRight = newTile[lastJ];
    cncPut_fromLeft(toRight, i+1, t, ctx);

    // Put "toLeft" items
    float *toLeft = cncItemCreate_fromRight();
    *toLeft = newTile[0];
    cncPut_fromRight(toLeft, i-1, t, ctx);

}

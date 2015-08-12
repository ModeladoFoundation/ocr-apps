#include "nqueens.h"

// board is made up of 4 8x8 blocks
#define BLOCK_DIM 8
#define BLOCK_MASK 0x0FF

static volatile s32 solutionsFound;

static inline u64 getRow(u64 nw, u64 ne, u64 sw, u64 se, u8 r) {
    u64 w, e;
    if (r < BLOCK_DIM) {
        w = sw;
        e = se;
    }
    else {
        w = nw;
        e = ne;
    }
    const u8 j = r%BLOCK_DIM;
    w = (w >> j*BLOCK_DIM) & BLOCK_MASK;
    e = (e >> j*BLOCK_DIM) & BLOCK_MASK;
    const u64 val = (e << BLOCK_DIM) | w;
    return val;
}

static inline void setRow(u64 *nw, u64 *ne, u64 *sw, u64 *se, u8 r, u8 i) {
    u64 v = (1UL << (i%BLOCK_DIM)) << ((r%BLOCK_DIM) * BLOCK_DIM);
    if (r < BLOCK_DIM) {
        if (i < BLOCK_DIM) { *sw |= v; }
        else { *se |= v; }
    }
    else {
        if (i < BLOCK_DIM) { *nw |= v; }
        else { *ne |= v; }
    }
}

static inline bool rowsConflict(u8 a, u64 rowA, u8 b, u64 rowB, u32 dim) {
    assert(b > a && "rows should be in ascending order");
    assert(rowA && "rows should be non-empty");
    assert(rowB && "rows should be non-empty");
    u8 diff = b - a;
    assert(diff < dim && "rows should both be on the board");
    // shifting the rows up lets me check the diagonals more easily
    rowA <<= dim;
    rowB <<= dim;
    // check if b is north of a
    bool n = (rowA ^ rowB) == 0;
    // check if b is north-west of a
    bool nw = ((rowA << diff) ^ rowB) == 0;
    // check if b is north-east of a
    bool ne = ((rowA >> diff) ^ rowB) == 0;
    return n || nw || ne;
}

static inline bool isSafe(u64 nw, u64 ne, u64 sw, u64 se, u8 r, u8 i, u32 dim) {
    u8 j;
    u64 rowB = 1UL << i;
    for (j=0; j<r; j++) {
        u64 rowA = getRow(nw, ne, sw, se, j);
        if (rowsConflict(j, rowA, r, rowB, dim)) {
            return false;
        }
    }
    return true;
}

static void handleGoodMove(u8 row, u8 i, cncTag_t nw, cncTag_t ne, cncTag_t sw, cncTag_t se, nqueensCtx *ctx) {
    const u32 lastRow = ctx->dim - 1;
    // record the move on the board
    setRow((u64*)&nw, (u64*)&ne, (u64*)&sw, (u64*)&se, row, i);
    // record the solution if we're done
    if (row == lastRow) {
        s32 solutionIndex = __sync_fetch_and_add(&solutionsFound, 1);
        u64 *solution = cncItemAlloc(sizeof(*solution) * 4);
        solution[0] = nw;
        solution[1] = ne;
        solution[2] = sw;
        solution[3] = se;
        cncPut_solutions(solution, solutionIndex, ctx);
    }
    // otherwise, find moves for next row
    else {
        cncPrescribe_placeQueen(row+1, nw, ne, sw, se, ctx);
    }
}

/**
 * Step function defintion for "placeQueen"
 */
void nqueens_placeQueen(cncTag_t row, cncTag_t nw, cncTag_t ne, cncTag_t sw, cncTag_t se, nqueensCtx *ctx) {

    // all done! early abort
    if (solutionsFound >= ctx->n) return;

    const u8 r = (u8)row;
    u8 i;
    u32 rowDim = ctx->dim;

    for (i=0; i<rowDim; i++) {
        // queen is OK, keep searching
        if (isSafe(nw, ne, sw, se, r, i, rowDim)) {
            handleGoodMove(r, i, nw, ne, sw, se, ctx);
        }
    }

}

#include "SimpleGraph.h"

/* Put an item to the Y collection */
void SimpleGraph_SY(cncTag_t y, SimpleGraphCtx *ctx) {
    int *Y = cncItemCreate_Y();
    *Y = y;
    cncPut_Y(Y, ctx);
}

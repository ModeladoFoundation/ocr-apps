#include "Config.h"
#include "Debug.h"

CFG_ITEM CFG_LIST[CFG_LIST_LENGTH] =
{
    {"CFG_DBG_LOG", 0},
    {"CFG_ALLOW_SHARED_DATA", 0},
    {"CFG_OPT_ASYNC_SEND", 1},
    {"CFG_OPT_MEMCPY_AVOIDANCE", 1},
    {"CFG_ASYNC_BCAST", 0},
    {"CFG_ASYNC_ALLREDUCE", 1},
    {"CFG_OPT_AGGREGATED_ASSIGN", 0},
    {"CFG_CMAP_CHECK_BEFORE_COMM", 1},
    {"CFG_CMAP_CREATE_CODELETS", 1}
};

int CFG_set(CFG_TYPE item_idx, int val) {
    switch (item_idx) {
        // You can add validity check here for values being set
        // For boolean values, val is either 0 or 1
        case(CFG_DBG_LOG):
        case(CFG_ALLOW_SHARED_DATA):
        case(CFG_OPT_ASYNC_SEND):
        case(CFG_OPT_MEMCPY_AVOIDANCE):
        case(CFG_ASYNC_BCAST):
        case(CFG_ASYNC_ALLREDUCE):
        case(CFG_OPT_AGGREGATED_ASSIGN):
        case(CFG_CMAP_CHECK_BEFORE_COMM):
        case(CFG_CMAP_CREATE_CODELETS):
            if(!(val == 0 || val == 1)) return 0;
        default:
            CFG_LIST[item_idx].val = val;
            return 1;
    }
}

int CFG_get(CFG_TYPE item_idx) {
    ASSERT(item_idx < CFG_LIST_LENGTH);
    return CFG_LIST[item_idx].val;
}


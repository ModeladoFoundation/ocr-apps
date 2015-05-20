#ifndef __CONFIG_H__
#define __CONFIG_H__
/*==================================================
 * For configurations that are switched on/off
 * by compile options. After changing recompilation
 * of the library is required for the change to be
 * effective.
 *================================================== */
// REUSE_GPP_ARRAYS enables optimizations about gpp arrays in Operation_util.h
// and Operation.pil
#define REUSE_GPP_ARRAYS (0)

// ENABLE_PREPACKING enables pre-packing for all parallel operations
// when the data blocks of HTA is not changed to avoid redundant
// packing steps in order to speed up parallel operation invocations
#define ENABLE_PREPACKING (1)

// Partial reduction optimizations
#define PARTIAL_REDUCE_PARALLEL_MERGE (1 << 0)
#define PARTIAL_REDUCE_SCALAR_OPT (1 << 1)
//#define REDUCE_OPT (PARTIAL_REDUCE_PARALLEL_MERGE | PARTIAL_REDUCE_SCALAR_OPT)
#define REDUCE_OPT (PARTIAL_REDUCE_SCALAR_OPT)

/*==================================================
 * For configurations that can be switch on/off
 * by setting values to shell environment variables
 * (No recompilation is required)
 *
 * To add new configurations, add a #define for the
 * index of the configuration item in the list of
 * configurations, and also change CFG_LIST_LENGTH += 1.
 * Next, go to Config.c and add the (name, default_val)
 * pair to the CFG_LIST. The name must be the same as
 * the enviroment variable name desired.
 *================================================== */

// Optimization through environment variables
typedef struct cfglist_item {
   const char* name;
   int         val;
} CFG_ITEM;

// define the index of the CFG item here
typedef enum {
    CFG_DBG_LOG             = 0,
    CFG_ALLOW_SHARED_DATA   = 1, // ON: pointers to leaf tile raw data is accessable on a processes
    CFG_OPT_ASYNC_SEND      = 2, // ON: optimization of communication operations assuming asynchronous sends are supported
    CFG_OPT_MEMCPY_AVOIDANCE = 3, // ON: avoid memcpy in communication operations
    CFG_ASYNC_BCAST          = 4, // ON: use async send for BCAST
    CFG_ASYNC_ALLREDUCE      = 5, // ON: use async send for allreduce
    CFG_OPT_AGGREGATED_ASSIGN = 6,
    CFG_CMAP_CHECK_BEFORE_COMM = 7,
    CFG_CMAP_CREATE_CODELETS = 8,
    CFG_LIST_LENGTH
} CFG_TYPE;

extern CFG_ITEM CFG_LIST[CFG_LIST_LENGTH];

int CFG_set(CFG_TYPE item_idx, int val);
int CFG_get(CFG_TYPE item_idx);

#endif

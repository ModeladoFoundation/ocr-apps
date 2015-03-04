#ifndef _CNCOCR_COMD_TYPES_H_
#define _CNCOCR_COMD_TYPES_H_

typedef struct comdArguments {
    /* TODO: Add struct members.
     * Use this struct to pass all arguments for
     * graph initialization. This should not contain any
     * pointers (unless you know you'll only be executing
     * in shared memory and thus passing pointers is safe).
     */
} comdArgs;

#include "string.h"
#include <math.h>
#ifndef CNCOCR_TG
#include <sys/time.h>
#include "strings.h"
#else
// Manu: placeholder till functions in time.h is implemented
struct timeval {
 float t;
};
#endif

typedef struct box *BItem;
typedef int TBoxesItem;
typedef struct atomInfo *AtomInfoItem;
typedef struct eamPot *EAMPOTItem;
typedef struct myReduction *redcItem;
typedef int ITItem;

#include "CoMDTypes.h"

#endif /*_CNCOCR_COMD_TYPES_H_*/

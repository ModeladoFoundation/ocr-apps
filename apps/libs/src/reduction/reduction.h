/*
 Author: David S Scott
 Copyright Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

/*
an OCR "library" for computing global reductions using labeled GUIDs.


The library implements a reduction tree invoking the reduction operator at each stage.
Node zero returns the result by satisfying the event with a separate datablock

See README for more details

*/

#include "ocr.h"
#include "extensions/ocr-labeling.h"

#define ARITY 10

typedef enum {
    REDUCTION_F8_ADD,
    REDUCTION_F8_MULTIPLY,
    REDUCTION_F8_MAX,
    REDUCTION_F8_MIN,
    REDUCTION_U8_ADD,
    REDUCTION_U8_MULTIPLY,
    REDUCTION_U8_MAX,
    REDUCTION_U8_MIN,
    REDUCTION_S8_MAX,
    REDUCTION_S8_MIN,
    REDUCTION_U8_BITAND,
    REDUCTION_U8_BITOR,
    REDUCTION_U8_BITXOR,
    REDUCTION_F4_ADD,
    REDUCTION_F4_MULTIPLY,
    REDUCTION_F4_MAX,
    REDUCTION_F4_MIN,
    REDUCTION_U4_ADD,
    REDUCTION_U4_MULTIPLY,
    REDUCTION_U4_MAX,
    REDUCTION_U4_MIN,
    REDUCTION_S4_MAX,
    REDUCTION_S4_MIN,
    REDUCTION_U4_BITAND,
    REDUCTION_U4_BITOR,
    REDUCTION_U4_BITXOR
    } reductionOperator_t;

typedef struct {
    u64 nrank;
    u64 myrank;
    u64 ndata;      //number of elements to be reduced
/*
0 only used when new is true
1 receiving from below
2 sending up
3 receiving from above
4 sending down (and returning answer)
*/
    u64 new; //should be set to true on first call or if any parameters change
    u64 all; //should be 1 for ALL-REDUCE and 0 for only rank 0 returns
    u64 up;
    reductionOperator_t reductionOperator; //which reduction to do (in reduction.h)
    ocrGuid_t rangeGUID; //nrank-1 labeled STICKY GUIDs (used only once to set up channels)
    ocrGuid_t sendUpEVT;
    ocrGuid_t recvUpEVT[ARITY];
    ocrGuid_t sendDownEVT[ARITY];
    ocrGuid_t recvDownEVT;
    ocrGuid_t returnEVT; //ONCE event to return the result, different for each rank
    ocrGuid_t reductionTML;    //initialized to NULL_GUID
    ocrHint_t myAffinity;
} reductionPrivate_t;

//prototypes

u64 reductionsizeof(reductionOperator_t operator);

void reductionLaunch(reductionPrivate_t * rbPTR, ocrGuid_t reductionPrivateGUID, void * mydataPTR);

ocrGuid_t reductionEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

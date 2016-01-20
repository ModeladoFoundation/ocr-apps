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

#define ENABLE_EXTENSION_LABELING
#include "ocr.h"
#include "extensions/ocr-labeling.h"

#define ARITY 7

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
    u64 new; //should be set to true on first call or if any parameters change
    u64 extra; //used for rank 0 passing something to all of the ranks (along with answer).  Typically a new once event if not using labeled GUIDs.
    reductionOperator_t reductionOperator; //which reduction to do (in reduction.h)
    ocrGuid_t rangeGUID; //nrank-1 labeled STICKY GUIDs
    ocrGuid_t sendEVT;
    ocrGuid_t recvEVT[ARITY];
    ocrGuid_t returnEVT; //ONCE event to return the result, provided by the user (presumably labeled)
    ocrGuid_t reductionTML;    //initialized to NULL_GUID
    ocrGuid_t returnDBK;  //initialized to NULL_GUID
    u64 sizeOfreturnDBK;  //doesn’t need to be initialized
} reductionPrivate_t;

//prototypes

u64 reductionsizeof(reductionOperator_t operator);

void reductionLaunch(reductionPrivate_t * rbPTR, ocrGuid_t reductionPrivateGUID, ocrGuid_t mydataGUID);

ocrGuid_t reductionEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

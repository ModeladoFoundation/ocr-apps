/*
 Author: David S Scott
 Copyright Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

/*
an OCR "library" for computing global reductions using labeled GUIDs.

This library can be called by a set of "numnodes" compute EDTs to compute a global reduction and return
result to the clones of the compute EDTs.
The EDTs must be numbered from 0 to numnodes-1

There are four initialization steps:

Before going parallel:


create a temporary datablock with a call to


    ocrGuid_t reductionCreateShared();


pass the datablock down to another EDT which initializes it with a call to


    void reductionSerialInit(u64 numnodes, u64 length, reductionOperator_t operator, reductionShared_t * shared, ocrGuid_t reductionSharedGuid);

numnodes is the number of participating EDTs.

length is the number of local elements to be reduced

reductionOperator is an enum type (which currently only contains REDUCTION_DOUBLE_ADD)

shared is the pointer to the shared block

reductionSharedGuid is the guid of the shared block (needed to release the initialized values)



After going parallel (a set of numnodes "compute" EDTs each of which gets the shared block and must know mynode), create a datablock with a call to

    ocrGuid_t reductionCreatePrivate();

Pass the shared and private blocks down to a new EDT which initializes the private block with a call to

    void reductionParallelInit(u64 mynode, reductionShared_t * shared, reductionPrivate_t * private, ocrGuid_t reductionPrivateGuid);

mynode is the "rank" of the particular EDT (between 0 and numnodes-1)
shared is a pointer to the shared block
private is a pointer to "my" private block
reductionPrivateGuid is the guid of my private block (needed to release the initialized values)

Note that the shared block is no longer needed after this call, but it is not easy to safely destroy it.



Now the reduction library can be called by the NUMNODES compute EDTs using two calls:

Create the return event with a call to

    ocrGuid_t reductionGetOnceEvent(reductionPrivate_t * private, ocrGuid_t reductionPrivateGuid);


Create the clone of the computeEDT and have it depend on the Once event to receive the result of the reduction

Compute the local contribution to be reduced, then call


void reductionLaunch(reductionPrivate_t * private, ocrGuid_t reductionPrivateGuid, ocrGuid_t mydataGuid);


The library implements a reduction tree invoking the reduction operator at each stage.  Node zero returns the result by satisfying the event with a separate datablock


*/

#define ENABLE_EXTENSION_LABELING
#include "ocr.h"
#include "extensions/ocr-labeling.h"

#define ARITY 2

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

typedef struct{
    u64 numnodes;
    u64 length;
    reductionOperator_t operator;
    ocrGuid_t Template;
    ocrGuid_t onceRange;
    ocrGuid_t stickyRange;
    } reductionShared_t;


typedef struct{
    u64 numnodes;
    u64 mynode;
    u64 phase;  //alternates between 0 and 1 to pick the final send GUID
    u64 length; //how many elements in the reduction vector
    u64 size;   //how big are the elements?
    reductionOperator_t operator;
    ocrGuid_t Template; //used for cloning
    ocrGuid_t finalSendBlock;  //used only by node zero
    ocrGuid_t finalSendGuid[2]; //used only by node zero
    ocrGuid_t sendGuid;
    ocrGuid_t recvGuid[ARITY];
    } reductionPrivate_t;

//prototypes


u64 reductionsizeof(reductionOperator_t operator);

ocrGuid_t reductionCreateShared();
void reductionSerialInit(u64 numnodes, u64 length, reductionOperator_t operator, reductionShared_t * shared, ocrGuid_t reductionSharedGuid);
ocrGuid_t reductionCreatePrivate();
void reductionParallelInit(u64 mynode, reductionShared_t * shared, reductionPrivate_t * private, ocrGuid_t reductionPrivateGuid);

ocrGuid_t reductionGetOnceEvent(reductionPrivate_t * private, ocrGuid_t reductionPrivateGuid);
void reductionLaunch(reductionPrivate_t * private, ocrGuid_t reductionPrivateGuid, ocrGuid_t mydataGuid);

ocrGuid_t reductionEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

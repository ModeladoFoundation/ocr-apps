/*
 Author: David S Scott
 Copyright Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

/*
an OCR "library" for computing global reductions using labeled GUIDs.

See reduction.h for details

The code implements an ARITY tree to reduce a set of local values to a single "global" set

ARITY is defined in reduction.h it can be changed and recompiled if desired.

*/

#define ENABLE_EXTENSION_LABELING
#include "ocr.h"
#include "extensions/ocr-labeling.h"
#include "math.h"   //for min and max
#include "string.h"   //for memcpy
#include "stdio.h"   //for debug print

#include "reduction.h"


u64 reductionsizeof(reductionOperator_t operator) {
    if(operator == REDUCTION_F8_ADD) return(8);
    if(operator == REDUCTION_F8_MULTIPLY) return(8);
    if(operator == REDUCTION_F8_MAX) return(8);
    if(operator == REDUCTION_F8_MIN) return(8);
    if(operator == REDUCTION_U8_ADD) return(8);
    if(operator == REDUCTION_U8_MULTIPLY) return(8);
    if(operator == REDUCTION_U8_MAX) return(8);
    if(operator == REDUCTION_U8_MIN) return(8);
    if(operator == REDUCTION_S8_MAX) return(8);
    if(operator == REDUCTION_S8_MIN) return(8);
    if(operator == REDUCTION_U8_BITAND) return(8);
    if(operator == REDUCTION_U8_BITOR) return(8);
    if(operator == REDUCTION_U8_BITXOR) return(8);
    if(operator == REDUCTION_F4_ADD) return(4);
    if(operator == REDUCTION_F4_MULTIPLY) return(4);
    if(operator == REDUCTION_F4_MAX) return(4);
    if(operator == REDUCTION_F4_MIN) return(4);
    if(operator == REDUCTION_U4_ADD) return(4);
    if(operator == REDUCTION_U4_MULTIPLY) return(4);
    if(operator == REDUCTION_U4_MAX) return(4);
    if(operator == REDUCTION_U4_MIN) return(4);
    if(operator == REDUCTION_S4_MAX) return(4);
    if(operator == REDUCTION_S4_MIN) return(4);
    if(operator == REDUCTION_U4_BITAND) return(4);
    if(operator == REDUCTION_U4_BITOR) return(4);
    if(operator == REDUCTION_U4_BITXOR) return(4);
PRINTF("UNSUPPORTED reductionOperator\n");
ocrShutdown();
}

void reductionOperation(u64 length, void * a, void * b, reductionOperator_t operator){
    u64 i;
    double * af8p, * bf8p;
    float  * af4p, * bf4p;
    u64    * au8p, * bu8p;
    s64    * as8p, * bs8p;
    u32    * au4p, * bu4p;
    s32    * as4p, * bs4p;

    switch(operator) {

        case REDUCTION_F8_ADD:
        af8p = (double *) a;
        bf8p = (double *) b;
        for(i=0;i<length;i++) *af8p++ += *bf8p++;
        return;

        case REDUCTION_F8_MULTIPLY:
        af8p = (double *) a;
        bf8p = (double *) b;
        for(i=0;i<length;i++) *af8p++ *= *bf8p++;
        return;

        case REDUCTION_F8_MAX:
        af8p = (double *) a;
        bf8p = (double *) b;
        for(i=0;i<length;i++) if(*af8p++ < *bf8p++) *(af8p-1) = *(bf8p-1);
        return;

        case REDUCTION_F8_MIN:
        af8p = (double *) a;
        bf8p = (double *) b;
        for(i=0;i<length;i++) if(*af8p++ > *bf8p++) *(af8p-1) = *(bf8p-1);
        return;

        case REDUCTION_U8_ADD:
        au8p = (u64 *) a;
        bu8p = (u64 *) b;
        for(i=0;i<length;i++) *au8p++ += *bu8p++;
        return;

        case REDUCTION_U8_MULTIPLY:
        au8p = (u64 *) a;
        bu8p = (u64 *) b;
        for(i=0;i<length;i++) *au8p++ *= *bu8p++;
        return;

        case REDUCTION_U8_MAX:
        au8p = (u64 *) a;
        bu8p = (u64 *) b;
        for(i=0;i<length;i++) if(*au8p++ < *bu8p++) *(au8p-1) = *(bu8p-1);
        return;

        case REDUCTION_U8_MIN:
        au8p = (u64 *) a;
        bu8p = (u64 *) b;
        for(i=0;i<length;i++) if(*au8p++ > *bu8p++) *(au8p-1) = *(bu8p-1);
        return;

        case REDUCTION_S8_MAX:
        as8p = (u64 *) a;
        bs8p = (u64 *) b;
        for(i=0;i<length;i++) if(*as8p++ < *bs8p++) *(as8p-1) = *(bs8p-1);
        return;

        case REDUCTION_S8_MIN:
        as8p = (s64 *) a;
        bs8p = (s64 *) b;
        for(i=0;i<length;i++) if(*as8p++ > *bs8p++) *(as8p-1) = *(bs8p-1);
        return;

        case REDUCTION_U8_BITAND:
        au8p = (u64 *) a;
        bu8p = (u64 *) b;
        for(i=0;i<length;i++) *au8p++ &= *bu8p++;
        return;

        case REDUCTION_U8_BITOR:
        au8p = (u64 *) a;
        bu8p = (u64 *) b;
        for(i=0;i<length;i++) *au8p++ |= *bu8p++;
        return;

        case REDUCTION_U8_BITXOR:
        au8p = (u64 *) a;
        bu8p = (u64 *) b;
        for(i=0;i<length;i++) *au8p++ ^= *bu8p++;
        return;

        case REDUCTION_F4_ADD:
        af4p = (float *) a;
        bf4p = (float *) b;
        for(i=0;i<length;i++) *af4p++ += *bf4p++;
        return;

        case REDUCTION_F4_MULTIPLY:
        af4p = (float *) a;
        bf4p = (float *) b;
        for(i=0;i<length;i++) *af4p++ *= *bf4p++;
        return;

        case REDUCTION_F4_MAX:
        af4p = (float *) a;
        bf4p = (float *) b;
        for(i=0;i<length;i++) if(*af4p++ < *bf4p++) *(af4p-1) = *(bf4p-1);
        return;

        case REDUCTION_F4_MIN:
        af4p = (float *) a;
        bf4p = (float *) b;
        for(i=0;i<length;i++) if(*af4p++ > *bf4p++) *(af4p-1) = *(bf4p-1);
        return;

        case REDUCTION_U4_ADD:
        au4p = (u32 *) a;
        bu4p = (u32 *) b;
        for(i=0;i<length;i++) *au4p++ += *bu4p++;
        return;

        case REDUCTION_U4_MULTIPLY:
        au4p = (u32 *) a;
        bu4p = (u32 *) b;
        for(i=0;i<length;i++) *au4p++ *= *bu4p++;
        return;

        case REDUCTION_U4_MAX:
        au4p = (u32 *) a;
        bu4p = (u32 *) b;
        for(i=0;i<length;i++) if(*au4p++ < *bu4p++) *(au4p-1) = *(bu4p-1);
        return;

        case REDUCTION_U4_MIN:
        au4p = (u32 *) a;
        bu4p = (u32 *) b;
        for(i=0;i<length;i++) if(*au4p++ > *bu4p++) *(au4p-1) = *(bu4p-1);
        return;

        case REDUCTION_S4_MAX:
        as4p = (u32 *) a;
        bs4p = (u32 *) b;
        for(i=0;i<length;i++) if(*as4p++ < *bs4p++) *(as4p-1) = *(bs4p-1);
        return;

        case REDUCTION_S4_MIN:
        as4p = (s32 *) a;
        bs4p = (s32 *) b;
        for(i=0;i<length;i++) if(*as4p++ > *bs4p++) *(as4p-1) = *(bs4p-1);
        return;

        case REDUCTION_U4_BITAND:
        au4p = (u32 *) a;
        bu4p = (u32 *) b;
        for(i=0;i<length;i++) *au4p++ &= *bu4p++;
        return;

        case REDUCTION_U4_BITOR:
        au4p = (u32 *) a;
        bu4p = (u32 *) b;
        for(i=0;i<length;i++) *au4p++ |= *bu4p++;
        return;

        case REDUCTION_U4_BITXOR:
        au4p = (u32 *) a;
        bu4p = (u32 *) b;
        for(i=0;i<length;i++) *au4p++ ^= *bu4p++;
        return;


        }

        PRINTF("unsupported operator type %d \n", operator);
    return;
}


ocrGuid_t reductionCreateShared() {
    u64 dummy;
    ocrGuid_t shared;
    ocrDbCreate(&shared, (void**) &dummy, sizeof(reductionShared_t), 0, NULL_GUID, NO_ALLOC);
    return shared;
    }


void reductionSerialInit(u64 numnodes, u64 length, reductionOperator_t operator, reductionShared_t * shared, ocrGuid_t reductionSharedGuid){
    shared->numnodes = numnodes;
    shared->length = length;
    shared->operator = operator;
    ocrEdtTemplateCreate(&(shared->Template), reductionEdt, 0, EDT_PARAM_UNK);
    ocrGuidRangeCreate(&(shared->onceRange),(u64) 2, GUID_USER_EVENT_ONCE);
    ocrGuidRangeCreate(&(shared->stickyRange), numnodes-1, GUID_USER_EVENT_STICKY);
    ocrDbRelease(reductionSharedGuid);
    return;
}

ocrGuid_t reductionCreatePrivate() {
    u64 dummy;
    ocrGuid_t private;
    ocrDbCreate(&private, (void**) &dummy, sizeof(reductionPrivate_t), 0, NULL_GUID, NO_ALLOC);
    return private;
    }

void reductionParallelInit(u64 mynode, reductionShared_t * shared, reductionPrivate_t * private, ocrGuid_t reductionPrivateGuid) {
    u64 source, dummy;
    private->numnodes = shared->numnodes;
    private->mynode = mynode;
    private->phase = 0;
    private->length = shared->length;
    private->operator = shared->operator;
    private->Template = shared->Template;
    ocrGuidFromIndex(&(private->finalSendGuid[0]), shared->onceRange, 0);
    ocrGuidFromIndex(&(private->finalSendGuid[1]), shared->onceRange, 1);
    if(mynode != 0) ocrGuidFromIndex(&(private->sendGuid), shared->stickyRange, mynode-1);
      else
        ocrDbCreate(&(private->finalSendBlock), (void**) &dummy, private->length*reductionsizeof(private->operator), 0, NULL_GUID, NO_ALLOC);

//set receive GUIDs
    u64 i;
    for(i=0;i<ARITY;i++) {
        source = mynode*ARITY+i+1;
        if(source >= private->numnodes) {
            private->recvGuid[i] = NULL_GUID;
        }else{
            ocrGuidFromIndex(&(private->recvGuid[i]), shared->stickyRange, source-1);
        }
    }

    ocrDbRelease(reductionPrivateGuid);
    return;
}


ocrGuid_t reductionGetOnceEvent(reductionPrivate_t * private, ocrGuid_t reductionPrivateGuid){
    ocrGuid_t temp;
    private->phase ^= 1;
    temp = private->finalSendGuid[private->phase];
    u64 errno = ocrEventCreate(&temp, OCR_EVENT_ONCE_T, GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);

    ocrDbRelease(reductionPrivateGuid);
    return(temp);
}

void reductionLaunch(reductionPrivate_t * private, ocrGuid_t reductionPrivateGuid, ocrGuid_t mydataGuid){
    ocrGuid_t reduction;
    ocrEdtCreate(&reduction, private->Template, EDT_PARAM_DEF, NULL, 2, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
    ocrAddDependence(reductionPrivateGuid, reduction, 0, DB_MODE_RO);
    ocrAddDependence(mydataGuid, reduction, 1, DB_MODE_RW);
    return;
}

ocrGuid_t reductionEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

/*
depv
0: private block
1: mydata
2: yourdata

2-ARITY+1 your datablocks for receivers
ARITY+2 final send block (node 0 only)
*/

    reductionPrivate_t * private = depv[0].ptr;
    void * mydata = (void *) depv[1].ptr;

    u64 mynode = private->mynode;

//if(depc > 3) printf("M%d P%d start depv3 %lx \n", mynode, phase, depv[3].guid);


    void * yourdata;
    ocrGuid_t reduction;
    ocrGuid_t srcEvent, destEvent;
    u64 errno, i, src, dest, nrecv, ndep;

//check whether need to receive by cloning
        if(depc == 2 && (mynode == 0 || private->recvGuid[0] != NULL_GUID)) {
        ndep = ARITY+2;
        if(mynode == 0) ndep++;
        ocrEdtCreate(&reduction, private->Template, EDT_PARAM_DEF, NULL, ndep, NULL, EDT_PROP_NONE, NULL_GUID, NULL_GUID);
        ocrAddDependence(depv[0].guid, reduction, 0 , DB_MODE_RO);
        ocrAddDependence(depv[1].guid, reduction, 1 , DB_MODE_RW);
        for(i=0;i<ARITY;i++) {
            if(private->recvGuid[i] == NULL_GUID) {
                 ocrAddDependence(NULL_GUID, reduction,i+2, DB_MODE_RO);
            } else {
                errno = ocrEventCreate(&(private->recvGuid[i]), OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
                ocrAddDependence(private->recvGuid[i], reduction, i+2, DB_MODE_RO);
            }
        }
        if(mynode == 0) ocrAddDependence(private->finalSendBlock, reduction, ndep-1, DB_MODE_RW);
        return NULL_GUID;
    }

        if(mynode == 0 || private->recvGuid[0] != NULL_GUID){
//I have received
//printf("M%d myvalue %f \n", mynode, *((float *) mydata));
        for(i=0;i<ARITY;i++) {
            if(private->recvGuid[i] != NULL_GUID) {
                ocrEventDestroy(private->recvGuid[i]);
                yourdata = (void *) depv[i+2].ptr;
//printf("M%d before add your value %f \n", mynode, *((float *) yourdata));
                reductionOperation(private->length, mydata, yourdata, private->operator);
//printf("M%d after add myvalue %f \n", mynode, *((float *) mydata));
            }
        }
    }

//send
    if(mynode == 0) {
//printf("M0 P%d nrecv %d\n", phase, nrecv);
        memcpy(depv[depc-1].ptr, mydata, private->length*reductionsizeof(private->operator));
        ocrDbRelease(depv[depc-1].guid);

//printf("M%d P%d satisfy Guid %lx with block %lx\n", mynode, phase, onceEvent, depv[nrecv+1].guid);
        ocrEventSatisfy(private->finalSendGuid[private->phase], depv[depc-1].guid);
        return NULL_GUID;
    }
//printf("M%d P%d send\n",mynode, phase);
    errno = ocrEventCreate(&(private->sendGuid), OCR_EVENT_STICKY_T, GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
//printf("M%d P%d event guid %lx errno %d \n", mynode, phase, destEvent, errno);
    ocrDbRelease(depv[1].guid);
//printf("M%d P%d satisfy Guid %lx with %lx \n", mynode, phase, destEvent, depv[0].guid);
    ocrEventSatisfy(private->sendGuid, depv[1].guid);
    return NULL_GUID;
}


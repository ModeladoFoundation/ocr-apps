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

March 2016: modify to use channel events
April 2016: modify to send results back down tree
May 2016: fixed bug to use local downDBK (rather than everyone using the one from rank 0)
June 2016: fixed a race condition and added a release to reductionLaunch

*/

#include <string.h>   //for memcpy
#include <stdio.h>   //for memcpy

#include "ocr.h"
#include "extensions/ocr-labeling.h"
#include "extensions/ocr-affinity.h"
#include "macros.h"

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
    return (u64)-1; // Let's keep the compiler happy
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
        for(i=0;i<length;i++) {
            *af8p++ += *bf8p++;
        }
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
        as8p = (s64 *) a;
        bs8p = (s64 *) b;
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
        as4p = (s32 *) a;
        bs4p = (s32 *) b;
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
    ocrShutdown();
    return;
}


typedef struct{
 ocrEdtDep_t reductionPrivate;
 ocrEdtDep_t buffer[1];
} reductionSendChannelDEPV_t;


ocrGuid_t reductionSendChannelEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

    DEPVDEF(reductionSendChannel);
    ocrGuid_t rpDBK = DEPV(reductionSendChannel,reductionPrivate,guid);
    reductionPrivate_t * rpPTR = DEPV(reductionSendChannel,reductionPrivate,ptr);

    ocrGuid_t bufferDBK = DEPVARRAY(reductionSendChannel,buffer,0,guid);
    ocrGuid_t * bufferPTR = (ocrGuid_t *) DEPVARRAY(reductionSendChannel,buffer,0,ptr);


//create channel events
    ocrGuid_t channelUpEVT, channelDownEVT;
    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen = 2;
    params.EVENT_CHANNEL.nbSat = 1;
    params.EVENT_CHANNEL.nbDeps = 1;
    ocrEventCreateParams(&channelUpEVT, OCR_EVENT_CHANNEL_T, false, &params);
    rpPTR->sendUpEVT = channelUpEVT;
    ocrEventCreateParams(&channelDownEVT, OCR_EVENT_CHANNEL_T, false, &params);
    rpPTR->recvDownEVT = channelDownEVT;

//send channel events
    ocrGuid_t sendEVT;
    ocrGuidFromIndex(&sendEVT, rpPTR->rangeGUID, rpPTR->myrank-1);
    u64 errno = ocrEventCreate(&sendEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
    bufferPTR[0] = channelUpEVT;
    bufferPTR[1] = channelDownEVT;
    ocrDbRelease(bufferDBK);
    ocrEventSatisfy(sendEVT, bufferDBK);

//return (to receive or reduction)
    ocrDbRelease(rpDBK);
    return rpDBK;
}


typedef struct{
 ocrEdtDep_t reductionPrivate;
 ocrEdtDep_t buffer[ARITY];
} reductionRecvChannelDEPV_t;


ocrGuid_t reductionRecvChannelEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

    DEPVDEF(reductionRecvChannel);
    ocrGuid_t rpDBK = DEPV(reductionRecvChannel,reductionPrivate,guid);
    reductionPrivate_t * rpPTR = DEPV(reductionRecvChannel,reductionPrivate,ptr);

    u64 nrank = rpPTR->nrank;
    u64 myrank = rpPTR->myrank;

    u64 i, src;

    for(i=0;i<ARITY;i++)
        if(DEPVARRAY(reductionRecvChannel,buffer,i,ptr) != NULL){
            rpPTR->recvUpEVT[i] =  ((ocrGuid_t *) DEPVARRAY(reductionRecvChannel,buffer,i,ptr))[0];
            rpPTR->sendDownEVT[i] =  ((ocrGuid_t *) DEPVARRAY(reductionRecvChannel,buffer,i,ptr))[1];
            ocrDbDestroy(DEPVARRAY(reductionRecvChannel,buffer,i,guid));
        } else {
            rpPTR->recvUpEVT[i] = NULL_GUID;
            rpPTR->sendDownEVT[i] = NULL_GUID;
        }

//return to reduction)
        ocrDbRelease(rpDBK);
        return rpDBK;
    }


    typedef struct{
     ocrEdtDep_t reductionPrivate;
     ocrEdtDep_t mydata;
     ocrEdtDep_t yourdata[1];
 } reductionDEPV_t;


    void reductionSendUp(reductionPrivate_t * rpPTR, void * data) {
        ocrGuid_t localDBK;
        void * localPTR;
        ocrDbCreate(&(localDBK), (void**) &localPTR, rpPTR->size, 0, NULL_HINT, NO_ALLOC);
        memcpy(localPTR, data, rpPTR->size);
        ocrDbRelease(localDBK);
        ocrEventSatisfy(rpPTR->sendUpEVT, localDBK);
        return;
    }


    void reductionRecvUp(reductionPrivate_t * rpPTR, ocrGuid_t reductionPrivateDBK, ocrGuid_t mydataDBK) {

        u32 i;
        ocrGuid_t reductionEDT;
        ocrEdtCreate(&reductionEDT, rpPTR->reductionTML, EDT_PARAM_DEF, NULL, ARITY+2, NULL, EDT_PROP_NONE, &rpPTR->myAffinity, NULL);
        ocrAddDependence(mydataDBK, reductionEDT, SLOT(reduction,mydata), DB_MODE_RW);
        for(i=0;i<ARITY;i++) ocrAddDependence(rpPTR->recvUpEVT[i], reductionEDT, SLOTARRAY(reduction,yourdata,i), DB_MODE_RO);
        rpPTR->phase = 1;
        ocrDbRelease(reductionPrivateDBK);
        ocrAddDependence(reductionPrivateDBK, reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
        return;
    }

    void reductionRecvDown(reductionPrivate_t * rpPTR, ocrGuid_t reductionPrivateDBK) {

        u32 i;
        ocrGuid_t reductionEDT;
        ocrEdtCreate(&reductionEDT, rpPTR->reductionTML, EDT_PARAM_DEF, NULL, 2, NULL, EDT_PROP_NONE, &rpPTR->myAffinity, NULL);
        ocrAddDependence(rpPTR->recvDownEVT, reductionEDT, SLOT(reduction,mydata), DB_MODE_RO);
        rpPTR->phase = 2;
        ocrDbRelease(reductionPrivateDBK);
        ocrAddDependence(reductionPrivateDBK, reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
        return;
    }

    void reductionSendDown(reductionPrivate_t * rpPTR, void * data) {
        ocrGuid_t localDBK;
        void * localPTR;
        u32 i;


        for(i=0;i<ARITY;i++) {
            if(!ocrGuidIsNull(rpPTR->sendDownEVT[i])){
                ocrDbCreate(&(localDBK), (void**) &localPTR, rpPTR->size, 0, NULL_HINT, NO_ALLOC);
                memcpy(localPTR, data, rpPTR->size);
                ocrDbRelease(localDBK);
                ocrEventSatisfy(rpPTR->sendDownEVT[i], localDBK);
            }
        }
        return;
    }






/*
 * C function callable from an SPMD set of EDTs to execute a reduction tree and return the answer.
 * Requires setting certain parameters in a "reductionPrivate" block
 */
ocrGuid_t reductionEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

    /*
    depv
    0: private block
    1: mydata
    2 to ARITY+1: the datablocks from your children
    going down, yourdata[0] is the result to send back and down
    */

    DEPVDEF(reduction);
    ocrGuid_t rpDBK = DEPV(reduction,reductionPrivate,guid);
    reductionPrivate_t * rpPTR = DEPV(reduction,reductionPrivate,ptr);

    u64 nrank = rpPTR->nrank;
    u64 myrank = rpPTR->myrank;
    u64 ndata = rpPTR->ndata;

    ocrGuid_t returnEVT = rpPTR->returnEVT;

    if(nrank == 1) { // nothing to do
        if(!ocrGuidIsNull(rpPTR->returnEVT)) {
            ocrEventSatisfy(rpPTR->returnEVT, DEPV(reduction,mydata,guid));
        }
        return NULL_GUID;
    }


    void * downdataPTR, *yourdataPTR, *mydataPTR;
    mydataPTR = DEPV(reduction,mydata,ptr);

    double dummy;

    ocrGuid_t reductionEDT;
    ocrGuid_t destEVT, recvEVT;
    u64 errno, i, src, dest, nrecv, ndep;

    if(rpPTR->new == 1) { //create channel events and use labeled GUIDs to initialize tree
        rpPTR->new = 0;
        rpPTR->phase = 0;

//initialize to NULL GUIDs
        rpPTR->sendUpEVT = NULL_GUID;
        rpPTR->recvDownEVT = NULL_GUID;

        for(i=0;i<ARITY;i++) {
            rpPTR->sendDownEVT[i] = NULL_GUID;
            rpPTR->recvUpEVT[i] = NULL_GUID;
        }

//create clone to continue after channel events are installed

        ocrEdtCreate(&reductionEDT, rpPTR->reductionTML, EDT_PARAM_DEF, NULL, 2, NULL, EDT_PROP_NONE, &rpPTR->myAffinity, NULL);
        ocrAddDependence(DEPV(reduction,mydata,guid), reductionEDT, SLOT(reduction,mydata), DB_MODE_RW);

        ocrGuid_t SendOutputEVT = NULL_GUID;
        ocrGuid_t reductionSendChannelTML, reductionSendChannelEDT, bufferDBK;

//create send
        if(myrank !=0) {
            ocrEdtTemplateCreate(&reductionSendChannelTML, reductionSendChannelEdt, 0, DEPVNUM(reductionSendChannel));
            ocrEdtCreate(&reductionSendChannelEDT, reductionSendChannelTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &rpPTR->myAffinity, &SendOutputEVT);
            ocrDbCreate(&bufferDBK, (void**) &dummy, 2*sizeof(ocrGuid_t), 0, NULL_HINT, NO_ALLOC);
            ocrDbRelease(bufferDBK);
            ocrAddDependence(bufferDBK, reductionSendChannelEDT, SLOT(reductionSendChannel,buffer), DB_MODE_RW);
        }

//create Receive
        ocrGuid_t RecvOutputEVT = NULL_GUID;
        ocrGuid_t reductionRecvChannelTML, reductionRecvChannelEDT;
        if(myrank*ARITY+1 < nrank) {
            ocrEdtTemplateCreate(&reductionRecvChannelTML, reductionRecvChannelEdt, 0, DEPVNUM(reductionRecvChannel));
            ocrEdtCreate(&reductionRecvChannelEDT, reductionRecvChannelTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &rpPTR->myAffinity, &RecvOutputEVT);
            for(i=0;i<ARITY;i++) {
                src = myrank*ARITY+i+1;
                if(src < nrank) {
                    ocrGuidFromIndex(&recvEVT, rpPTR->rangeGUID, src-1);
                    errno = ocrEventCreate(&recvEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
                    ocrAddDependence(recvEVT, reductionRecvChannelEDT, SLOTARRAY(reductionRecvChannel,buffer,i), DB_MODE_RO);
                } else {
                    ocrAddDependence(NULL_GUID, reductionRecvChannelEDT, SLOTARRAY(reductionRecvChannel,buffer,i), DB_MODE_RO);
                }
            }
        }

//launch all
    ocrDbRelease(DEPV(reduction,reductionPrivate,guid));
    if(ocrGuidIsNull(SendOutputEVT)) //no send
        if(ocrGuidIsNull(RecvOutputEVT)){ //neither
            ocrAddDependence(DEPV(reduction,reductionPrivate,guid), reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
        } else { //only Recv
            ocrAddDependence(RecvOutputEVT, reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
            ocrAddDependence(DEPV(reduction,reductionPrivate,guid), reductionRecvChannelEDT, SLOT(reductionRecvChannel,reductionPrivate), DB_MODE_RW);
        }
    else
        if(ocrGuidIsNull(RecvOutputEVT)) {//only send
            ocrAddDependence(SendOutputEVT, reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
            ocrAddDependence(DEPV(reduction,reductionPrivate,guid), reductionSendChannelEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
        } else { //all 3
            ocrAddDependence(SendOutputEVT, reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
            ocrAddDependence(RecvOutputEVT, reductionSendChannelEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
            ocrAddDependence(DEPV(reduction,reductionPrivate,guid), reductionRecvChannelEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
        }
        return NULL_GUID; //done with initialization
    }

//channels in place


    ocrGuid_t localDBK;
    void * localPTR;
    ocrGuid_t reductionPrivateDBK = DEPV(reduction,reductionPrivate,guid);


    rpPTR->size = rpPTR->ndata;
    if(rpPTR->type != BROADCAST) rpPTR->size =  rpPTR->ndata*reductionsizeof(rpPTR->reductionOperator);

    switch (rpPTR->phase) {


    case 0: //first real call, figure out what to do

        switch(rpPTR->type) {

        case BROADCAST:
            if(rpPTR->myrank == 0)
                reductionSendDown(rpPTR, mydataPTR);
              else reductionRecvDown(rpPTR, DEPV(reduction, reductionPrivate, guid));
            return NULL_GUID;

        case REDUCE:
            if(!ocrGuidIsNull(rpPTR->recvUpEVT[0])) {
                reductionRecvUp(rpPTR, DEPV(reduction, reductionPrivate, guid), DEPV(reduction, mydata, guid));
                }
              else reductionSendUp(rpPTR, mydataPTR);
            return NULL_GUID;

        case ALLREDUCE:
            if(!ocrGuidIsNull(rpPTR->recvUpEVT[0])) {
                reductionRecvUp(rpPTR,DEPV(reduction, reductionPrivate,guid), DEPV(reduction,mydata,guid));
            }
              else {
                reductionSendUp(rpPTR, mydataPTR);
                reductionRecvDown(rpPTR, DEPV(reduction,reductionPrivate,guid));
            }
            return NULL_GUID;
        }

    case 1: //I have received up
        for(i=0;i<ARITY;i++) {
            if(!ocrGuidIsNull(DEPVARRAY(reduction,yourdata,i,guid))) {
                yourdataPTR = DEPVARRAY(reduction,yourdata,i,ptr);
                reductionOperation(rpPTR->ndata,mydataPTR, yourdataPTR, rpPTR->reductionOperator);
                ocrDbDestroy(DEPVARRAY(reduction,yourdata,i,guid));
            }
        }
        if(myrank !=0) {
            reductionSendUp(rpPTR, mydataPTR);
            if(rpPTR->type == ALLREDUCE) reductionRecvDown(rpPTR, DEPV(reduction, reductionPrivate, guid));
            return NULL_GUID;
        }
          else {
            if(rpPTR->type == ALLREDUCE) reductionSendDown(rpPTR, mydataPTR);
            rpPTR->phase = 0;
            ocrDbRelease(reductionPrivateDBK);
            ocrDbRelease(DEPV(reduction,mydata,guid));
            ocrEventSatisfy(returnEVT, DEPV(reduction,mydata,guid));
            return NULL_GUID;
        } // This seemed to be missing

    case 2: //I have received down
        if(!ocrGuidIsNull(rpPTR->sendDownEVT[0])) reductionSendDown(rpPTR,mydataPTR);
        rpPTR->phase = 0;
        ocrDbRelease(reductionPrivateDBK);
        ocrDbRelease(DEPV(reduction,mydata,guid));
        ocrEventSatisfy(returnEVT, DEPV(reduction,mydata,guid));
        return NULL_GUID;
        // } // This seemed to be extra
    } // end outermost switch
}






/*
 * C function callable from an SPMD set of EDTs to execute a reduction tree and return the answer.
 * Requires setting certain parameters in a "reductionPrivate" block
 */
void reductionLaunch(reductionPrivate_t * rpPTR, ocrGuid_t reductionPrivateDBK, void * mydataPTR){
    ocrGuid_t reductionEDT, localDBK;
    void * localPTR;
    if(rpPTR->new) {
        ocrEdtTemplateCreate(&(rpPTR->reductionTML), reductionEdt, 0, EDT_PARAM_UNK);
        ocrHintInit(&rpPTR->myAffinity,OCR_HINT_EDT_T);
#ifdef ENABLE_EXTENSION_AFFINITY
        ocrGuid_t myAffinity;
        ocrAffinityGetCurrent(&(myAffinity));
        ocrSetHintValue(&rpPTR->myAffinity, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(myAffinity));
#endif
    }

    ocrEdtCreate(&reductionEDT, rpPTR->reductionTML, EDT_PARAM_DEF, NULL, 2, NULL, EDT_PROP_NONE, &rpPTR->myAffinity, NULL);
    ocrDbCreate(&(localDBK), (void**) &localPTR, rpPTR->ndata*reductionsizeof(rpPTR->reductionOperator), 0, NULL_HINT, NO_ALLOC);
    memcpy(localPTR, mydataPTR, rpPTR->ndata*reductionsizeof(rpPTR->reductionOperator));
    ocrDbRelease(localDBK);
    ocrAddDependence(localDBK, reductionEDT, SLOT(reduction,mydata), DB_MODE_RW);
    ocrDbRelease(reductionPrivateDBK);
    ocrAddDependence(reductionPrivateDBK, reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
    return;
}


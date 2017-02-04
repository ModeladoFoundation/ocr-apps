/*;
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

#include "reductionEager.h"

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
    ocrGuid_t returnEVT; //used to return to reductionEdt
} channelRecvPRM_t;

typedef struct{
    ocrEdtDep_t reductionPrivate;
    ocrEdtDep_t downBuffer;
    ocrEdtDep_t upBuffer[ARITY];
} channelRecvDEPV_t;


ocrGuid_t channelRecvEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

    PRMDEF(channelRecv);
    ocrGuid_t returnEVT = PRM(channelRecv,returnEVT);

    DEPVDEF(channelRecv);
    ocrGuid_t rpDBK = DEPV(channelRecv,reductionPrivate,guid);
    reductionPrivate_t * rpPTR = DEPV(channelRecv,reductionPrivate,ptr);
    ocrGuid_t bufferDBK = DEPV(channelRecv,downBuffer,guid);
    ocrGuid_t * bufferPTR = DEPV(channelRecv,downBuffer,ptr);

    u64 i, dummy;

//Recv from above

    if(bufferPTR != NULL) {
        rpPTR->sendUpEVT[0] = bufferPTR[0];
        rpPTR->sendUpEVT[1] = bufferPTR[1];
        ocrDbDestroy(bufferDBK);
        ocrDbCreate(&(rpPTR->sendUpDBK[0]), (void**) &dummy, rpPTR->size, 0, &rpPTR->myDbkAffinityHNT, NO_ALLOC);
        ocrDbCreate(&(rpPTR->sendUpDBK[1]), (void**) &dummy, rpPTR->size, 0, &rpPTR->myDbkAffinityHNT, NO_ALLOC);
      } else {
        rpPTR->sendUpEVT[0] = NULL_GUID;
        rpPTR->sendUpEVT[1] = NULL_GUID;
        rpPTR->sendUpDBK[0] = NULL_GUID;
        rpPTR->sendUpDBK[1] = NULL_GUID;
        }

//Recv from below

     for(i=0;i<ARITY;i++) {
         bufferDBK = DEPVARRAY(channelRecv,upBuffer,i,guid);
         bufferPTR = (ocrGuid_t *) DEPVARRAY(channelRecv,upBuffer,i,ptr);
         if(bufferPTR != NULL) {
            rpPTR->sendDownEVT[i][0] = bufferPTR[0];
            rpPTR->sendDownEVT[i][1] = bufferPTR[1];
            ocrDbDestroy(bufferDBK);
            ocrDbCreate(&(rpPTR->sendDownDBK[i][0]), (void**) &dummy, rpPTR->size, 0, &rpPTR->myDbkAffinityHNT, NO_ALLOC);
            ocrDbCreate(&(rpPTR->sendDownDBK[i][1]), (void**) &dummy, rpPTR->size, 0, &rpPTR->myDbkAffinityHNT, NO_ALLOC);
          } else {
            rpPTR->sendDownEVT[i][0] = NULL_GUID;
            rpPTR->sendDownEVT[i][1] = NULL_GUID;
            rpPTR->sendDownDBK[i][0] = NULL_GUID;
            rpPTR->sendDownDBK[i][1] = NULL_GUID;
            }
        }
    ocrDbRelease(rpDBK);
    ocrEventSatisfy(returnEVT, rpDBK);
    return NULL_GUID;
}





    typedef struct{
     ocrEdtDep_t reductionPrivate;
     ocrEdtDep_t mydata;
     ocrEdtDep_t yourdata[1];
 } reductionDEPV_t;




    void reductionRecvUp(ocrGuid_t reductionPrivateDBK, reductionPrivate_t * rpPTR, ocrGuid_t mydataDBK) {

        u32 i;
        ocrGuid_t reductionEDT;
        ocrEdtCreate(&reductionEDT, rpPTR->reductionTML, EDT_PARAM_DEF, NULL, ARITY+2, NULL, EDT_PROP_NONE, &rpPTR->myEdtAffinityHNT, NULL);
        ocrAddDependence(mydataDBK, reductionEDT, SLOT(reduction,mydata), DB_MODE_RW);
        for(i=0;i<ARITY;i++) {
            ocrAddDependence(rpPTR->recvUpEVT[i][rpPTR->toggle], reductionEDT, SLOTARRAY(reduction,yourdata,i), DB_MODE_RO);
        }
        rpPTR->phase = 1;
        ocrDbRelease(reductionPrivateDBK);
        ocrAddDependence(reductionPrivateDBK, reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
        return;
    }

    void reductionRecvDown(ocrGuid_t reductionPrivateDBK, reductionPrivate_t * rpPTR, ocrGuid_t mydataDBK) {

        u32 i;
        ocrGuid_t reductionEDT;
        ocrEdtCreate(&reductionEDT, rpPTR->reductionTML, EDT_PARAM_DEF, NULL, 3, NULL, EDT_PROP_NONE, &rpPTR->myEdtAffinityHNT, NULL);
        ocrAddDependence(rpPTR->recvDownEVT[rpPTR->toggle], reductionEDT, SLOTARRAY(reduction,yourdata,0), DB_MODE_RO);
        rpPTR->phase = 2;
        ocrDbRelease(reductionPrivateDBK);
        ocrAddDependence(reductionPrivateDBK, reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
        ocrAddDependence(mydataDBK, reductionEDT, SLOT(reduction,mydata), DB_MODE_RW);
        return;
    }





typedef struct{
    ocrEdtDep_t reductionPrivate;
    ocrEdtDep_t mydata;
    ocrEdtDep_t buffer[ARITY];
} reductionSendDownAndBackDEPV_t;


ocrGuid_t reductionSendDownAndBackEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

    DEPVDEF(reductionSendDownAndBack);
    reductionPrivate_t * rpPTR = DEPV(reductionSendDownAndBack,reductionPrivate,ptr);

    void * mydataPTR = DEPV(reductionSendDownAndBack,mydata,ptr);
    ocrGuid_t bufferDBK;
    void * bufferPTR;

    u64 i;

    for(i=0;i<ARITY;i++) {
        if(ocrGuidIsNull(rpPTR->sendDownEVT[i][0]))break;

        bufferPTR = DEPVARRAY(reductionSendDownAndBack,buffer,i,ptr);
        bufferDBK = DEPVARRAY(reductionSendDownAndBack,buffer,i,guid);
        memcpy(bufferPTR, mydataPTR, rpPTR->size);
        ocrDbRelease(bufferDBK);
        ocrEventSatisfy(rpPTR->sendDownEVT[i][rpPTR->toggle], bufferDBK);
        }
    if(rpPTR->myrank !=0 || rpPTR->type != BROADCAST){ //rank0 should not send back for a broadcast
        ocrGuid_t returnEVT = rpPTR->returnEVT;
        ocrDbRelease(DEPV(reductionSendDownAndBack,mydata,guid));

        ocrEventSatisfy(returnEVT, DEPV(reductionSendDownAndBack,mydata,guid));
    }

    return NULL_GUID;
    }


    void reductionSendDownAndBack(ocrGuid_t reductionPrivateDBK, reductionPrivate_t * rpPTR, ocrGuid_t mydataDBK) {

        u32 i;
        ocrGuid_t reductionSendDownAndBackEDT;
        ocrEdtCreate(&reductionSendDownAndBackEDT, rpPTR->reductionSendDownAndBackTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &rpPTR->myEdtAffinityHNT, NULL);


        for(i=0;i<ARITY;i++) {
            if(!ocrGuidIsNull(rpPTR->sendDownEVT[i][rpPTR->toggle])) {
                ocrAddDependence(rpPTR->sendDownDBK[i][rpPTR->toggle], reductionSendDownAndBackEDT, SLOTARRAY(reductionSendDownAndBack,buffer,i), DB_MODE_RW);

            } else {
                ocrAddDependence(NULL_GUID, reductionSendDownAndBackEDT, SLOTARRAY(reductionSendDownAndBack,buffer,i), DB_MODE_RW);
            }

        }
        ocrDbRelease(reductionPrivateDBK);
        ocrAddDependence(reductionPrivateDBK, reductionSendDownAndBackEDT, SLOT(reductionSendDownAndBack,reductionPrivate), DB_MODE_RO);
        ocrAddDependence(mydataDBK, reductionSendDownAndBackEDT, SLOT(reductionSendDownAndBack,mydata), DB_MODE_RW);


        return;
    }


typedef struct{
    ocrEdtDep_t reductionPrivate;
    ocrEdtDep_t mydata;
    ocrEdtDep_t buffer;
} reductionSendUpDEPV_t;

ocrGuid_t reductionSendUpEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

    DEPVDEF(reductionSendUp);
    ocrGuid_t rpDBK = DEPV(reductionSendUp,reductionPrivate,guid);
    reductionPrivate_t * rpPTR = DEPV(reductionSendUp,reductionPrivate,ptr);

    void * mydataPTR = DEPV(reductionSendUp,mydata,ptr);
    ocrGuid_t bufferDBK = DEPV(reductionSendUp,buffer,guid);
    void * bufferPTR = DEPV(reductionSendUp,buffer,ptr);


    memcpy(bufferPTR, mydataPTR, rpPTR->size);

    ocrDbRelease(bufferDBK);
    ocrEventSatisfy(rpPTR->sendUpEVT[rpPTR->toggle], bufferDBK);

    return NULL_GUID;
    }


    void reductionSendUp(ocrGuid_t reductionPrivateDBK, reductionPrivate_t * rpPTR, ocrGuid_t mydataDBK) {

        ocrGuid_t reductionSendUpEDT;


        ocrEdtCreate(&reductionSendUpEDT, rpPTR->reductionSendUpTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &rpPTR->myEdtAffinityHNT, NULL);

        ocrAddDependence(reductionPrivateDBK, reductionSendUpEDT, SLOT(reductionSendUp,reductionPrivate), DB_MODE_RO);
        ocrAddDependence(mydataDBK, reductionSendUpEDT, SLOT(reductionSendUp,mydata), DB_MODE_RW);
        ocrAddDependence(rpPTR->sendUpDBK[rpPTR->toggle], reductionSendUpEDT, SLOT(reductionSendUp,buffer), DB_MODE_RW);

        return;
    }




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
    ocrGuid_t mydataDBK = DEPV(reduction,mydata,guid);
    void * mydataPTR = DEPV(reduction,mydata,ptr);

    u64 nrank = rpPTR->nrank;
    u64 myrank = rpPTR->myrank;
    u64 ndata = rpPTR->ndata;

    ocrGuid_t returnEVT = rpPTR->returnEVT;

    if(nrank == 1) { // nothing to do
        if(!ocrGuidIsNull(rpPTR->returnEVT)) ocrEventSatisfy(rpPTR->returnEVT, DEPV(reduction,mydata,guid));
        return NULL_GUID;
    }


    void * downdataPTR, * yourdataPTR;
    mydataPTR = DEPV(reduction,mydata,ptr);

    double dummy;

    ocrGuid_t reductionEDT;
    ocrGuid_t sendEVT, recvEVT, bufferDBK, * bufferPTR;
    u64 errno, i, src, dest, nrecv, ndep;
    if(rpPTR->new == 1) {
//create channel events and use labeled GUIDs to initialize tree
//to support EAGER datablocks, the receiver will create the channel event
//but can be reversed by defining SENDER_OWNS_CHANNEL_EVENTS
        rpPTR->new = 0;
        rpPTR->phase = 0;

        ocrEventParams_t params;
        params.EVENT_CHANNEL.maxGen = 2;
        params.EVENT_CHANNEL.nbSat = 1;
        params.EVENT_CHANNEL.nbDeps = 1;

//create clone to continue after channel events are installed
        ocrGuid_t returnEVT;
        ocrEventCreate(&returnEVT, OCR_EVENT_ONCE_T, true);
        ocrEdtCreate(&reductionEDT, rpPTR->reductionTML, EDT_PARAM_DEF, NULL, 2, NULL, EDT_PROP_NONE, &rpPTR->myEdtAffinityHNT, NULL);
        ocrAddDependence(DEPV(reduction,mydata,guid), reductionEDT, SLOT(reduction,mydata), DB_MODE_RW);
        ocrAddDependence(returnEVT, reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);


        ocrGuid_t sendEVT, channelRecvEDT, channelRecvTML;

        ocrEdtTemplateCreate(&channelRecvTML, channelRecvEdt, PRMNUM(channelRecv), DEPVNUM(channelRecv));
        ocrEdtCreate(&channelRecvEDT, channelRecvTML, EDT_PARAM_DEF, (u64 *) &returnEVT, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &rpPTR->myEdtAffinityHNT, NULL);

//send up and recv down

        if(myrank != 0) {
            ocrDbCreate(&bufferDBK, (void**) &bufferPTR, 2*sizeof(ocrGuid_t), 0, NULL_HINT, NO_ALLOC);
            ocrDbCreate(&(rpPTR->sendUpDBK[0]), (void**) &dummy, rpPTR->size, 0, &rpPTR->myDbkAffinityHNT, NO_ALLOC);
            ocrDbCreate(&(rpPTR->sendUpDBK[0]), (void**) &dummy, rpPTR->size, 0, &rpPTR->myDbkAffinityHNT, NO_ALLOC);
            ocrEventCreateParams(&(rpPTR->recvDownEVT[0]), OCR_EVENT_CHANNEL_T, false, &params);
            ocrEventCreateParams(&(rpPTR->recvDownEVT[1]), OCR_EVENT_CHANNEL_T, false, &params);
            bufferPTR[0] = rpPTR->recvDownEVT[0];
            bufferPTR[1] = rpPTR->recvDownEVT[1];
            ocrGuidFromIndex(&sendEVT, rpPTR->rangeGUID, myrank);
            errno = ocrEventCreate(&sendEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
            ocrDbRelease(bufferDBK);
            ocrEventSatisfy(sendEVT, bufferDBK);

            ocrGuidFromIndex(&recvEVT, rpPTR->rangeGUID, nrank + myrank);
            errno = ocrEventCreate(&recvEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
            ocrAddDependence(recvEVT, channelRecvEDT, SLOT(channelRecv,downBuffer), DB_MODE_RO);

          } else {
            ocrAddDependence(NULL_GUID, channelRecvEDT, SLOT(channelRecv,downBuffer), DB_MODE_RO);
            rpPTR->sendUpDBK[0] = NULL_GUID;
            rpPTR->sendUpDBK[1] = NULL_GUID;
            rpPTR->sendUpEVT[0] = NULL_GUID;
            rpPTR->sendUpEVT[1] = NULL_GUID;
            rpPTR->recvDownEVT[0] = NULL_GUID;
            rpPTR->recvDownEVT[1] = NULL_GUID;
            }

//send down and recv up

        for(i=0;i<ARITY;i++) {
            src = myrank*ARITY+i+1;
            if(src < nrank) {
                ocrDbCreate(&bufferDBK, (void**) &bufferPTR, 2*sizeof(ocrGuid_t), 0, NULL_HINT, NO_ALLOC);
                ocrDbCreate(&(rpPTR->sendDownDBK[i][0]), (void**) &dummy, rpPTR->size, 0, &rpPTR->myDbkAffinityHNT, NO_ALLOC);
                ocrDbCreate(&(rpPTR->sendDownDBK[i][0]), (void**) &dummy, rpPTR->size, 0, &rpPTR->myDbkAffinityHNT, NO_ALLOC);
                ocrEventCreateParams(&(rpPTR->recvUpEVT[i][0]), OCR_EVENT_CHANNEL_T, false, &params);
                ocrEventCreateParams(&(rpPTR->recvUpEVT[i][1]), OCR_EVENT_CHANNEL_T, false, &params);
                bufferPTR[0] = rpPTR->recvUpEVT[i][0];
                bufferPTR[1] = rpPTR->recvUpEVT[i][1];
                ocrGuidFromIndex(&sendEVT, rpPTR->rangeGUID, nrank+src);
                errno = ocrEventCreate(&sendEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
                ocrDbRelease(bufferDBK);
                ocrEventSatisfy(sendEVT, bufferDBK);

                ocrGuidFromIndex(&recvEVT, rpPTR->rangeGUID, src);
                errno = ocrEventCreate(&recvEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
                ocrAddDependence(recvEVT, channelRecvEDT, SLOTARRAY(channelRecv,upBuffer,i), DB_MODE_RO);

              } else {
                ocrAddDependence(NULL_GUID, channelRecvEDT, SLOTARRAY(channelRecv,upBuffer,i), DB_MODE_RO);
                rpPTR->sendDownDBK[i][0] = NULL_GUID;
                rpPTR->sendDownDBK[i][1] = NULL_GUID;
                rpPTR->sendDownEVT[i][0] = NULL_GUID;
                rpPTR->sendDownEVT[i][1] = NULL_GUID;
                rpPTR->recvUpEVT[i][0] = NULL_GUID;
                rpPTR->recvUpEVT[i][1] = NULL_GUID;
                }
            }
        ocrAddDependence(rpDBK, channelRecvEDT, SLOT(channelRecv,reductionPrivate), DB_MODE_RW);
        return NULL_GUID;
        }



//channels in place




    //printf("R%d P%d type%d mydatablock "GUIDF" \n", myrank, rpPTR->phase, rpPTR->type,GUIDA(DEPV(reduction,mydata,guid)));

    switch (rpPTR->phase) {


    case 0: //first real call, figure out what to do
    //printf("R%d P0 type%d \n", myrank, rpPTR->type);

        rpPTR->toggle ^= 1;

        switch(rpPTR->type) {

        case BROADCAST:
            if(rpPTR->myrank == 0)
                reductionSendDownAndBack(rpDBK, rpPTR, mydataDBK);
              else reductionRecvDown(rpDBK, rpPTR, mydataDBK);
            return NULL_GUID;

        case REDUCE:
            if(!ocrGuidIsNull(rpPTR->recvUpEVT[0][rpPTR->toggle])) {
                reductionRecvUp(rpDBK, rpPTR, mydataDBK);
                }
              else reductionSendUp(rpDBK, rpPTR, mydataDBK);
            return NULL_GUID;

        case ALLREDUCE:
            if(!ocrGuidIsNull(rpPTR->recvUpEVT[0][rpPTR->toggle])) {
                reductionRecvUp(rpDBK, rpPTR, mydataDBK);
            }
              else {
                reductionSendUp(rpDBK, rpPTR, mydataDBK);
                reductionRecvDown(rpDBK, rpPTR, mydataDBK);
            }
            return NULL_GUID;
        }

    case 1: //I have received up
    //printf("R%d P1 type%d \n", myrank, rpPTR->type);
        for(i=0;i<ARITY;i++) {
            if(!ocrGuidIsNull(DEPVARRAY(reduction,yourdata,i,guid))) {
                yourdataPTR = DEPVARRAY(reduction,yourdata,i,ptr);
                reductionOperation(rpPTR->ndata,mydataPTR, yourdataPTR, rpPTR->reductionOperator);
            }
        }
        if(myrank !=0) {
            reductionSendUp(rpDBK, rpPTR, mydataDBK);
            if(rpPTR->type == ALLREDUCE) reductionRecvDown(rpDBK, rpPTR, mydataDBK);
            return NULL_GUID;
        }
        if(rpPTR->type == ALLREDUCE) {
            reductionSendDownAndBack(DEPV(reduction,reductionPrivate,guid), rpPTR, DEPV(reduction,mydata,guid));
            return NULL_GUID;
            }

        returnEVT = rpPTR->returnEVT;
        ocrDbRelease(DEPV(reduction,mydata,guid));
        ocrEventSatisfy(returnEVT, DEPV(reduction,mydata,guid));

        return NULL_GUID;

    case 2: //I have received down
    //printf("R%d P2 type%d \n", myrank, rpPTR->type);
        memcpy(DEPV(reduction,mydata,ptr), DEPVARRAY(reduction,yourdata,0,ptr), rpPTR->size);
        if(!ocrGuidIsNull(rpPTR->sendDownEVT[0][rpPTR->toggle])){
            reductionSendDownAndBack(DEPV(reduction,reductionPrivate,guid), rpPTR, DEPV(reduction,mydata,guid));
            }
          else {
            ocrGuid_t returnEVT = rpPTR->returnEVT;


            ocrDbRelease(rpDBK);
            ocrDbRelease(DEPV(reduction,mydata,guid));
            ocrEventSatisfy(returnEVT, DEPV(reduction,mydata,guid));
            }
        return NULL_GUID;

        }
}






/*
 * C function callable from an SPMD set of EDTs to execute a reduction tree and return the answer.
 * Requires setting certain parameters in a "reductionPrivate" block
 */
void reductionLaunch(reductionPrivate_t * rpPTR, ocrGuid_t reductionPrivateDBK, void * mydataPTR){
    ocrGuid_t reductionEDT, localDBK;
    void * localPTR;
    rpPTR->size = rpPTR->ndata;
//printf("RL new %d \n", rpPTR->new);
    if(rpPTR->type != BROADCAST) rpPTR->size =  rpPTR->ndata*reductionsizeof(rpPTR->reductionOperator);
    if(rpPTR->new) {
        ocrEdtTemplateCreate(&(rpPTR->reductionTML), reductionEdt, 0, EDT_PARAM_UNK);
        ocrEdtTemplateCreate(&(rpPTR->reductionSendDownAndBackTML), reductionSendDownAndBackEdt, 0, ARITY+2);
        ocrEdtTemplateCreate(&(rpPTR->reductionSendUpTML), reductionSendUpEdt, 0, 3);
        ocrHintInit(&rpPTR->myEdtAffinityHNT,OCR_HINT_EDT_T);
        ocrHintInit(&rpPTR->myDbkAffinityHNT,OCR_HINT_DB_T);

        rpPTR->toggle = 1;
#ifdef ENABLE_EXTENSION_AFFINITY
        ocrGuid_t myAffinity;
        ocrAffinityGetCurrent(&(myAffinity));
        ocrSetHintValue(&rpPTR->myEdtAffinityHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(myAffinity));
#endif

#define ENABLE_EAGER_DATABLOCKS

#ifdef ENABLE_EAGER_DATABLOCKS
        ocrSetHintValue(&rpPTR->myDbkAffinityHNT, OCR_HINT_DB_EAGER, 1);
#endif
    }

    ocrEdtCreate(&reductionEDT, rpPTR->reductionTML, EDT_PARAM_DEF, NULL, 2, NULL, EDT_PROP_NONE, &rpPTR->myEdtAffinityHNT, NULL);
    ocrDbCreate(&(localDBK), (void**) &localPTR, rpPTR->size, 0, NULL_HINT, NO_ALLOC);
    memcpy(localPTR, mydataPTR, rpPTR->size);
//PRINTF("RL%d localDBK "GUIDF" \n",rpPTR->myrank,GUIDA(localDBK));
    ocrDbRelease(localDBK);
    ocrAddDependence(localDBK, reductionEDT, SLOT(reduction,mydata), DB_MODE_RW);
    rpPTR->phase = 0;
    ocrDbRelease(reductionPrivateDBK);
    ocrAddDependence(reductionPrivateDBK, reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
    return;
}


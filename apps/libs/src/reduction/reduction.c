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

*/

#define ENABLE_EXTENSION_LABELING
#define ENABLE_EXTENSION_AFFINITY
#include "ocr.h"
#include "extensions/ocr-labeling.h"
#include "extensions/ocr-affinity.h"
#include "math.h"   //for min and max
#include "string.h"   //for memcpy
#include "stdio.h"   //for debug print
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
//PRINTF("add %f %f \n", *af8p, *bf8p);
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

//PRINTF("Send %d Start \n", rpPTR->myrank);
    ocrGuid_t bufferDBK = DEPVARRAY(reductionSendChannel,buffer,0,guid);
    ocrGuid_t * bufferPTR = (ocrGuid_t *) DEPVARRAY(reductionSendChannel,buffer,0,ptr);

//create channel event
    ocrGuid_t channelUpEVT, channelDownEVT;
    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen = 2;
    params.EVENT_CHANNEL.nbSat = 1;
    params.EVENT_CHANNEL.nbDeps = 1;
    ocrEventCreateParams(&channelUpEVT, OCR_EVENT_CHANNEL_T, false, &params);
    rpPTR->sendUpEVT = channelUpEVT;
    ocrEventCreateParams(&channelDownEVT, OCR_EVENT_CHANNEL_T, false, &params);
    rpPTR->recvDownEVT = channelDownEVT;
//PRINTF("Send %d channelUpEVT "GUIDF" channelDownEVT "GUIDF"\n", rpPTR->myrank, GUIDA(channelUpEVT), GUIDA(channelDownEVT));


//send channel events
    ocrGuid_t sendEVT;
    ocrGuidFromIndex(&sendEVT, rpPTR->rangeGUID, rpPTR->myrank-1);
//PRINTF("S%d range "GUIDF" sendEVT "GUIDF" \n", rpPTR->myrank, GUIDA(rpPTR->rangeGUID), GUIDA(sendEVT));
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
//TODO destroy labeled events (!)

    DEPVDEF(reductionRecvChannel);
    ocrGuid_t rpDBK = DEPV(reductionRecvChannel,reductionPrivate,guid);
    reductionPrivate_t * rpPTR = DEPV(reductionRecvChannel,reductionPrivate,ptr);

    u64 nrank = rpPTR->nrank;
    u64 myrank = rpPTR->myrank;

//PRINTF("Recv %d Start depv0 guid "GUIDF" depv1 guid "GUIDF"\n", rpPTR->myrank, depv[0].guid, depv[1].guid);

    u64 i, src;

    for(i=0;i<ARITY;i++)
        if(DEPVARRAY(reductionRecvChannel,buffer,i,ptr) != NULL){
            rpPTR->recvUpEVT[i] =  ((ocrGuid_t *) DEPVARRAY(reductionRecvChannel,buffer,i,ptr))[0];
//PRINTF("R%d i%d recvUpEVT "GUIDF" \n", myrank, i, rpPTR->recvUpEVT[i]);
            rpPTR->sendDownEVT[i] =  ((ocrGuid_t *) DEPVARRAY(reductionRecvChannel,buffer,i,ptr))[1];
//PRINTF("R%d i%d sendDownEVT "GUIDF" \n", myrank, i, rpPTR->sendDownEVT[i]);
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

ocrGuid_t reductionEdt(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

/*
depv
0: private block
1: mydata
2-ARITY+1 your datablocks from your children
ARITY+2 final send block (rank 0 only)
*/


    DEPVDEF(reduction);
    ocrGuid_t rpDBK = DEPV(reduction,reductionPrivate,guid);
    reductionPrivate_t * rpPTR = DEPV(reduction,reductionPrivate,ptr);

    u64 nrank = rpPTR->nrank;
    u64 myrank = rpPTR->myrank;
    u64 ndata = rpPTR->ndata;

    void * downdataPTR, *yourdataPTR, *mydataPTR;
    mydataPTR = DEPV(reduction,mydata,ptr);

    double dummy;

    ocrGuid_t reductionEDT;
    ocrGuid_t destEVT, recvEVT;
    u64 errno, i, src, dest, nrecv, ndep;
//PRINTF("Red %d depc %d", myrank, depc);
//for(i=0;i<depc;i++) PRINTF(" "GUIDF" ",GUIDA(depv[i].guid));
//PRINTF(" \n");

    if(rpPTR->new == 1) { //create channel events and use labeled GUIDs to initialize tree
        rpPTR->new = 0;

//initialize to NULL GUIDs

        rpPTR->sendUpEVT = NULL_GUID;
        rpPTR->recvDownEVT = NULL_GUID;

        for(i=0;i<ARITY;i++) {
            rpPTR->sendDownEVT[i] = NULL_GUID;
            rpPTR->recvUpEVT[i] = NULL_GUID;
        }

//create clone to continue after channel events are installed

//PRINTF("Red %d create clone to initialize\n", myrank);
        ocrEdtCreate(&reductionEDT, rpPTR->reductionTML, EDT_PARAM_DEF, NULL, 2, NULL, EDT_PROP_NONE, &rpPTR->myAffinity, NULL);
        ocrAddDependence(DEPV(reduction,mydata,guid), reductionEDT, SLOT(reduction,mydata), DB_MODE_RW);

//INTF("Red %d after clone\n", myrank);

//create Send

        ocrGuid_t SendOutputEVT = NULL_GUID;
        ocrGuid_t reductionSendChannelTML, reductionSendChannelEDT, bufferDBK;


        if(myrank !=0) {
            ocrEdtTemplateCreate(&reductionSendChannelTML, reductionSendChannelEdt, 0, DEPVNUM(reductionSendChannel));
            ocrEdtCreate(&reductionSendChannelEDT, reductionSendChannelTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &rpPTR->myAffinity, &SendOutputEVT);
//PRINTF("Red %d create SendChannel output "GUIDF" \n", myrank, SendOutputEVT);
            ocrDbCreate(&bufferDBK, (void**) &dummy, 2*sizeof(ocrGuid_t), 0, NULL_HINT, NO_ALLOC);
            ocrAddDependence(bufferDBK, reductionSendChannelEDT, SLOT(reductionSendChannel,buffer), DB_MODE_RW);
        }



//create Receive

        ocrGuid_t RecvOutputEVT = NULL_GUID;
        ocrGuid_t reductionRecvChannelTML, reductionRecvChannelEDT;

        if(myrank*ARITY+1 < nrank) {
            ocrEdtTemplateCreate(&reductionRecvChannelTML, reductionRecvChannelEdt, 0, DEPVNUM(reductionRecvChannel));
            ocrEdtCreate(&reductionRecvChannelEDT, reductionRecvChannelTML, EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL, EDT_PROP_NONE, &rpPTR->myAffinity, &RecvOutputEVT);
//PRINTF("Red %d create RecvChannel guid "GUIDF" slots %d output "GUIDF" \n", myrank, reductionRecvChannelEDT, DEPVNUM(reductionRecvChannel), RecvOutputEVT);
            for(i=0;i<ARITY;i++) {
                src = myrank*ARITY+i+1;
                if(src < nrank) {
                    ocrGuidFromIndex(&recvEVT, rpPTR->rangeGUID, src-1);
                    errno = ocrEventCreate(&recvEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
                    ocrAddDependence(recvEVT, reductionRecvChannelEDT, SLOTARRAY(reductionRecvChannel,buffer,i), DB_MODE_RO);
//PRINTF("add dep %d recvEVT guid "GUIDF" errno %d\n", SLOTARRAY(reductionRecvChannel,buffer,i), GUIDA(reductionRecvChannelEDT), errno);
              } else {
                    ocrAddDependence(NULL_GUID, reductionRecvChannelEDT, SLOTARRAY(reductionRecvChannel,buffer,i), DB_MODE_RO);
                }
            }
        }


//launch all

//PRINTF("Launch %d Send "GUIDF" Recv "GUIDF" \n", myrank, GUIDA(SendOutputEVT), GUIDA(RecvOutputEVT));
        ocrDbRelease(DEPV(reduction,reductionPrivate,guid));
        if(ocrGuidIsNull(SendOutputEVT)) //no send
            if(ocrGuidIsNull(RecvOutputEVT)){ //neither
                ocrAddDependence(DEPV(reduction,reductionPrivate,guid), reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
//PRINTF("Red %d neither \n", myrank);

            } else { //only Recv
                ocrAddDependence(RecvOutputEVT, reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
                ocrAddDependence(DEPV(reduction,reductionPrivate,guid), reductionRecvChannelEDT, SLOT(reductionRecvChannel,reductionPrivate), DB_MODE_RW);
//PRINTF("Red %d Recv only guid "GUIDF" add dep %d\n", myrank, GUIDA(reductionRecvChannelEDT), SLOT(reductionRecvChannel,reductionPrivate));
            }
          else if(ocrGuidIsNull(RecvOutputEVT)) {//only send
//PRINTF("Red %d send only\n", myrank);
                ocrAddDependence(SendOutputEVT, reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
                ocrAddDependence(DEPV(reduction,reductionPrivate,guid), reductionSendChannelEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
            } else { //all 3
//PRINTF("Red %d both\n", myrank);
                ocrAddDependence(SendOutputEVT, reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
                ocrAddDependence(RecvOutputEVT, reductionSendChannelEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
                ocrAddDependence(DEPV(reduction,reductionPrivate,guid), reductionRecvChannelEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
            }

        return NULL_GUID; //done with initialization
    }



ocrGuid_t localDBK;
void * localPTR;
        if(depc == 2) { //first "real" call, heading up
//PRINTF("Red %d depc 2 recvUp "GUIDF" \n", myrank, GUIDA(rpPTR->recvUpEVT[0]));
            if(!ocrGuidIsNull(rpPTR->recvUpEVT[0])) { //need to receive
                rpPTR->up = 1;
                ocrEdtCreate(&reductionEDT, rpPTR->reductionTML, EDT_PARAM_DEF, NULL, ARITY+2, NULL, EDT_PROP_NONE, &rpPTR->myAffinity, NULL);
//PRINTF("Red %d create clone for first real time\n", myrank);
                ocrDbRelease(DEPV(reduction,mydata,guid));  //should not be needed since mydata hasn't changed
                ocrAddDependence(DEPV(reduction,mydata,guid), reductionEDT, SLOT(reduction,mydata), DB_MODE_RW);
                for(i=0;i<ARITY;i++) ocrAddDependence(rpPTR->recvUpEVT[i], reductionEDT, SLOTARRAY(reduction,yourdata,i), DB_MODE_RO);
                ocrDbRelease(DEPV(reduction,reductionPrivate,guid));
                ocrAddDependence(DEPV(reduction,reductionPrivate,guid), reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
                return NULL_GUID;
            }


            if(ocrGuidIsNull(rpPTR->sendUpEVT)) { //must be the only active rank...just return the input
                ocrDbRelease(DEPV(reduction,mydata,guid));
//PRINTF("Red %d satisfying local\n", myrank);
                ocrEventSatisfy(rpPTR->returnEVT,DEPV(reduction,mydata,guid));
                return NULL_GUID;
            }

//send up
//PRINTF("Red %d send up "GUIDF" \n", myrank, GUIDA(rpPTR->sendUpEVT));

            ocrDbRelease(DEPV(reduction,mydata,guid));
            ocrEventSatisfy(rpPTR->sendUpEVT, DEPV(reduction,mydata,guid));

//clone to receive down
            if(rpPTR->all) {
                rpPTR->up = 0;
                ocrEdtCreate(&reductionEDT, rpPTR->reductionTML, EDT_PARAM_DEF, NULL, 3, NULL, EDT_PROP_NONE, &rpPTR->myAffinity, NULL);
//PRINTF("Red %d create clone for receive down\n", myrank);
                ocrAddDependence(DEPV(reduction,mydata,guid), reductionEDT, SLOT(reduction,mydata), DB_MODE_RW);
                ocrAddDependence(rpPTR->recvDownEVT, reductionEDT, SLOTARRAY(reduction,yourdata,0), DB_MODE_RO);
                ocrDbRelease(DEPV(reduction,reductionPrivate,guid));
                ocrAddDependence(DEPV(reduction,reductionPrivate,guid), reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
            }
            return NULL_GUID;
        }


u64 j;

        if(rpPTR->up == true){ //I have received up

//PRINTF("Red %d I have received \n", myrank);

//combine values
            for(i=0;i<ARITY;i++) {
                if(!ocrGuidIsNull(DEPVARRAY(reduction,yourdata,i,guid))) {
                    yourdataPTR = DEPVARRAY(reduction,yourdata,i,ptr);
//for(j=0;j<rpPTR->ndata;j++)  PRINTF("Red %d i %d j %d yourdata %f \n", myrank, i, j, ((double *) yourdataPTR)[j]);
                    reductionOperation(rpPTR->ndata,mydataPTR, yourdataPTR, rpPTR->reductionOperator);
//for(j=0;j<rpPTR->ndata;j++)  PRINTF("Red %d i %d j %d mydata %f \n", myrank, i, j, ((double *) mydataPTR)[j]);
                }
            }

//for(j=0;j<rpPTR->ndata;j++)  PRINTF("Red %d i %d j %d mydata %f \n", myrank, i, j, ((double *) mydataPTR)[j]);
            ocrDbRelease(DEPV(reduction,mydata,guid));

            if(ocrGuidIsNull(rpPTR->sendUpEVT)) { //top of the tree
//PRINTF("Red %d local satisfy "GUIDF" \n", myrank, GUIDA(rpPTR->returnEVT));

//send back
                ocrEventSatisfy(rpPTR->returnEVT,DEPV(reduction,mydata,guid));
                if(!rpPTR->all) return NULL_GUID; //not going down

//send down

                for(i=0;i<ARITY;i++) {
//PRINTF("Red %d sendDownEVT "GUIDF" block "GUIDF" \n", myrank, GUIDA(rpPTR->sendDownEVT[i]), GUIDA(DEPV(reduction,mydata,guid)));
                    if(!ocrGuidIsNull(rpPTR->sendDownEVT[i])){
                        ocrDbCreate(&(localDBK), (void**) &localPTR, rpPTR->ndata*reductionsizeof(rpPTR->reductionOperator), 0, NULL_HINT, NO_ALLOC);
                        memcpy(localPTR, mydataPTR, rpPTR->ndata*reductionsizeof(rpPTR->reductionOperator));
                        ocrDbRelease(localDBK);
                        ocrEventSatisfy(rpPTR->sendDownEVT[i], localDBK);
                    }
                }

//PRINTF("Red %d satisfying local\n", myrank);
                return NULL_GUID;
//PRINTF("Red %d sendUpEVT "GUIDF" block "GUIDF" \n", myrank, GUIDA(rpPTR->sendUpEVT), GUIDA(DEPV(reduction,mydata,guid)));
        }

//send up
            ocrEventSatisfy(rpPTR->sendUpEVT, DEPV(reduction,mydata,guid));

            if(!rpPTR->all) return NULL_GUID;  //done if only going up

//clone to receive down
            rpPTR->up = false;
            ocrEdtCreate(&reductionEDT, rpPTR->reductionTML, EDT_PARAM_DEF, NULL, 3, NULL, EDT_PROP_NONE, &rpPTR->myAffinity, NULL);
//PRINTF("Red %d create clone for receive down\n", myrank);
            ocrAddDependence(DEPV(reduction,mydata,guid), reductionEDT, SLOT(reduction,mydata), DB_MODE_RW);
            ocrAddDependence(rpPTR->recvDownEVT, reductionEDT, SLOTARRAY(reduction,yourdata,0), DB_MODE_RO);
            ocrDbRelease(DEPV(reduction,reductionPrivate,guid));  //should not be needed since mydata hasn't changed
            ocrAddDependence(DEPV(reduction,reductionPrivate,guid), reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
            return NULL_GUID;
        }

// must be on the way down, copy, send down and return

//PRINTF("Red %d on way down\n", myrank);
//for(j=0;j<rpPTR->ndata;j++)  PRINTF("Red recv down and out %d j %d mydata %f \n", myrank, j, ((double *) yourdataPTR)[j]);



memcpy(mydataPTR,DEPVARRAY(reduction,yourdata,0,ptr), rpPTR->ndata*reductionsizeof(rpPTR->reductionOperator));
//for(j=0;j<rpPTR->ndata;j++)  PRINTF("Red send down and out %d j %d mydata %f \n", myrank, j, ((double *) mydataPTR)[j]);
ocrDbDestroy(DEPVARRAY(reduction,yourdata,0,guid));

        for(i=0;i<ARITY;i++) {
//PRINTF("Red %d sendDownEVT "GUIDF" block "GUIDF" \n", myrank, GUIDA(rpPTR->sendDownEVT[i]), GUIDA(DEPV(reduction,mydata,guid)));
            if(!ocrGuidIsNull(rpPTR->sendDownEVT[i])){
                ocrDbCreate(&(localDBK), (void**) &localPTR, rpPTR->ndata*reductionsizeof(rpPTR->reductionOperator), 0, NULL_HINT, NO_ALLOC);
                memcpy(localPTR, mydataPTR, rpPTR->ndata*reductionsizeof(rpPTR->reductionOperator));
                ocrDbRelease(localDBK);
                ocrEventSatisfy(rpPTR->sendDownEVT[i], localDBK);
            }
        }

//PRINTF("Red %d local satisfy "GUIDF" \n", myrank, GUIDA(rpPTR->returnEVT));

        ocrDbRelease(DEPV(reduction,mydata,guid));
        ocrEventSatisfy(rpPTR->returnEVT, DEPV(reduction,mydata,guid));
        return NULL_GUID;
    }

/*
C function callable from an SPMD set of EDTs to execute a reduction tree and return the answer.
Requires setting certain parameters in a "reductionPrivate" block
*/

void reductionLaunch(reductionPrivate_t * rpPTR, ocrGuid_t reductionPrivateGUID, void * mydataPTR){
    ocrGuid_t reductionEDT, localDBK;
    double * localPTR;
    if(rpPTR->new) {
        ocrEdtTemplateCreate(&(rpPTR->reductionTML), reductionEdt, 0, EDT_PARAM_UNK);
        ocrHintInit(&rpPTR->myAffinity,OCR_HINT_EDT_T);
#ifdef ENABLE_EXTENSION_AFFINITY
        ocrGuid_t myAffinity;
        ocrAffinityGetCurrent(&(myAffinity));
//PRINTF("R%d affinity "GUIDF" \n", rpPTR->myrank, myAffinity);
        ocrSetHintValue(&rpPTR->myAffinity, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(myAffinity));
#else
    rpPTR->myAffinity = NULL_HINT;
#endif
    }

    ocrEdtCreate(&reductionEDT, rpPTR->reductionTML, EDT_PARAM_DEF, NULL, 2, NULL, EDT_PROP_NONE, &rpPTR->myAffinity, NULL);
    ocrAddDependence(reductionPrivateGUID, reductionEDT, SLOT(reduction,reductionPrivate), DB_MODE_RW);
    ocrDbCreate(&(localDBK), (void**) &localPTR, rpPTR->ndata*reductionsizeof(rpPTR->reductionOperator), 0, NULL_HINT, NO_ALLOC);
    memcpy(localPTR, mydataPTR, rpPTR->ndata*reductionsizeof(rpPTR->reductionOperator));

    ocrAddDependence(localDBK, reductionEDT, SLOT(reduction,mydata), DB_MODE_RW);
    return;
}


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
        ocrShutdown();
    return;
}



typedef struct{
   ocrEdtDep_t reductionPrivate;
   ocrEdtDep_t mydata;
   ocrEdtDep_t yourdata[1];
} reductionDEPV_t;

ocrGuid_t reductionEDT(u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

/*
depv
0: private block
1: mydata
2-ARITY+1 your datablocks from your children
ARITY+2 final send block (rank 0 only)
*/


    reductionDEPV_t * reductionDEPV = (reductionDEPV_t *) depv;
    reductionPrivate_t * rpPTR = reductionDEPV->reductionPrivate.ptr;

    u64 nrank = rpPTR->nrank;
    u64 myrank = rpPTR->myrank;
    u64 ndata = rpPTR->ndata;

    void * yourdataPTR, *mydataPTR;
    mydataPTR = DEPV(reduction,mydata,ptr);

//PRINTF("R%ld start nrank %ld ndata %ld depc %ld \n", myrank, nrank, ndata, depc);

//PRINTF("R%ld GUIDs %lx %lx", myrank, depv[0].guid, depv[1].guid);
//if(depc > 2) PRINTF("%lx \n", depv[2].guid);
//if(depc > 3) PRINTF("%lx \n", depv[3].guid);
//PRINTF("\n");

//PRINTF("R%ld ARITY %ld ndata %ld reductionOperator %ld \n", myrank, ARITY, ndata, rpPTR->reductionOperator);
//PRINTF("R%ld mydata %f \n", myrank, *((double *)mydataPTR));
//if(yourdataPTR != NULL) PRINTF("R%ld yourdata %f \n", myrank, *((double *)yourdataPTR));

    double dummy;

    ocrGuid_t reductionGUID;
    ocrGuid_t destEVT;
    u64 errno, i, src, dest, nrecv, ndep;
    if(rpPTR->new) {
        rpPTR->new = 0;
        if(myrank == 0) ocrDbCreate(&rpPTR->returnDBK, (void**) &dummy, ndata*reductionsizeof(rpPTR->reductionOperator)+rpPTR->extra, 0, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC);
        if(myrank > 0) ocrGuidFromIndex(&(rpPTR->sendEVT), rpPTR->rangeGUID, myrank-1);
//PRINTF("M%d SendEVT %lx \n", myrank, rpPTR->sendEVT);
        for(i=0;i<ARITY;i++) {
            src = myrank*ARITY+1+i;
//PRINTF("M%d i %d mysrc %d \n", myrank, i, src);
            if(src<nrank ){
                ocrGuidFromIndex(&(rpPTR->recvEVT[i]), rpPTR->rangeGUID, src-1);
//PRINTF("M%d i%d mysrc %d recvEVT %lx\n", myrank, i, src, rpPTR->recvEVT[i]);
            }
            else rpPTR->recvEVT[i] = NULL_GUID;
        }
    }

//check whether need to receive by cloning
        if(depc == 2 && ((myrank == 0) || (myrank*ARITY+1 < nrank))) {
        ndep = ARITY + 2;
        if(myrank == 0) ndep++;
//PRINTF("R%ld cloneing ndep %ld \n", myrank, ndep);
        ocrEdtCreate(&reductionGUID, rpPTR->reductionTML, EDT_PARAM_DEF, NULL, ndep, NULL, EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL);
        ocrAddDependence(DEPV(reduction,reductionPrivate,guid), reductionGUID, SLOT(reduction,reductionPrivate), DB_MODE_RO);
        ocrAddDependence(DEPV(reduction,mydata,guid), reductionGUID, SLOT(reduction,mydata), DB_MODE_RW);
        for(i=0;i<ARITY;i++) {
            if( !IS_GUID_NULL(rpPTR->recvEVT[i]) ) errno = ocrEventCreate(&(rpPTR->recvEVT[i]), OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
            ocrAddDependence(rpPTR->recvEVT[i], reductionGUID, i+2, DB_MODE_RO);
        }

        if(myrank == 0) ocrAddDependence(rpPTR->returnDBK, reductionGUID, ndep-1, DB_MODE_RW);
        return NULL_GUID;

    }

        if(myrank == 0 || depc > 2){
//I have received
        for(i=0;i<ARITY;i++) {
//PRINTF("M%ld myvalue %f mySrc %ld \n", myrank, *((double *) mydataPTR), myrank*ARITY+1+i);
            if( !IS_GUID_NULL(DEPV(reduction,yourdata[i],guid)) ) {
//PRINTF("M%ld i %d guid %lx \n", myrank, i, DEPV(reduction,yourdata[i],guid));
                ocrEventDestroy(rpPTR->recvEVT[i]);
                yourdataPTR = DEPV(reduction,yourdata[i],ptr);
//printf("M%d before add your value %f \n", myrank, *((float *) yourdata));
                reductionOperation(rpPTR->ndata,mydataPTR, yourdataPTR, rpPTR->reductionOperator);
//printf("M%d after add myvalue %f \n", myrank, *((float *) mydata));
            }
        }
    }

//send
    if(myrank == 0) {
//printf("M0 P%d nrecv %d\n", phase, nrecv);

//yourdata[ARITY] exists only on rank 0 and is the return block which was added as a dependence when rank0 cloned itself.
        memcpy(DEPV(reduction,yourdata[ARITY],ptr), mydataPTR, rpPTR->ndata*reductionsizeof(rpPTR->reductionOperator) + rpPTR->extra);
        ocrDbRelease(rpPTR->returnDBK);

//printf("M%d P%d satisfy Guid %lx with block %lx\n", myrank, phase, onceEvent, depv[nrecv+1].guid);
        ocrEventSatisfy(rpPTR->returnEVT, rpPTR->returnDBK);
        return NULL_GUID;
    }
    destEVT = rpPTR->sendEVT;
    errno = ocrEventCreate(&destEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG);
//printf("M%d depc %d  event guid %lx errno %d \n", myrank, depc, destEVT, errno);
    ocrDbRelease(DEPV(reduction,mydata,guid));
//printf("M%d P%d satisfy Guid %lx with %lx \n", myrank, phase, destEvent, depv[0].guid);
//printf("M%d depc %d  event guid %lx errno %d \n", myrank, depc, destEVT, errno);
    ocrEventSatisfy(destEVT, DEPV(reduction,mydata,guid));
    return NULL_GUID;
}


void reductionLaunch(reductionPrivate_t * rpPTR, ocrGuid_t reductionPrivateGUID, ocrGuid_t mydataGUID){
    ocrGuid_t reductionGUID;
    if(IS_GUID_NULL(rpPTR->reductionTML)) ocrEdtTemplateCreate(&(rpPTR->reductionTML), reductionEDT, 0, EDT_PARAM_UNK);
    ocrEdtCreate(&reductionGUID, rpPTR->reductionTML, EDT_PARAM_DEF, NULL, 2, NULL, EDT_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NULL);
    ocrAddDependence(reductionPrivateGUID, reductionGUID, 0, DB_MODE_RW);
    ocrAddDependence(mydataGUID, reductionGUID, 1, DB_MODE_RW);
    return;
}


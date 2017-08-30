#ifndef PTEST_DAVEREDUCTION_H
#include "ptest_daveReduction.h"
#endif

#define ENABLE_EXTENSION_LABELING  // For labeled GUIDs
#include "extensions/ocr-labeling.h"  // For labeled GUIDs

#ifdef REDUCTION_EAGER
#include "reductionEager.h"
#else
#include "reduction.h"
#endif

#include "ptest_FORforkjoin.h" //FFJ_Ledger_t

int init_DRshared(DRshared_t * io)
{
    return clear_DRshared(io);
}
int clear_DRshared(DRshared_t * io)
{
    GUID_ASSIGN_VALUE(io->reductionRangeGUID , NULL_GUID);
    return 0;
}
int destroy_DRshared(DRshared_t * io)
{
    int err=0;
    if( ! IS_GUID_NULL(io->reductionRangeGUID)){
        err = ocrGuidMapDestroy( io->reductionRangeGUID);
    }
    if(err) return err;
    return clear_DRshared(io);
}
int copy_DRshared(DRshared_t * in_from, DRshared_t * o_target)
{
    GUID_ASSIGN_VALUE(o_target->reductionRangeGUID, in_from->reductionRangeGUID);
    return 0;
}
void print_DRshared(DRshared_t * in)
{
    PRINTF("DRshared> reductRguid="GUIDF"\n", GUIDA(in->reductionRangeGUID) );
}

int DRmainEdt_fcn(DRshared_t * o_sharedRef, DRshared_t * o_shared, unsigned long in_nrank)
{
    int err = 0;
    while(!err){
        err = init_DRshared(o_sharedRef); IFEB;
        err = init_DRshared(o_shared); IFEB;

        ocrGuid_t reductionRangeGUID = NULL_GUID;
        unsigned long rangeCount;
#ifdef REDUCTION_EAGER
        //Note: *2 is for eager reduction library double buffering I think
        rangeCount = in_nrank*2;
#else
        rangeCount = in_nrank;
#endif
        err = ocrGuidRangeCreate(&reductionRangeGUID, rangeCount, GUID_USER_EVENT_STICKY); IFEB;

        GUID_ASSIGN_VALUE(o_sharedRef->reductionRangeGUID, reductionRangeGUID);
        GUID_ASSIGN_VALUE(o_shared->reductionRangeGUID, reductionRangeGUID);

#       ifdef DR_ENABLE_REDUCTION_A
            PRINTF("INFO: DR: Reduction A is enabled.\n");
#       else
            PRINTF("INFO: DR: Reduction A is off.\n");
#       endif
#       ifdef DR_ENABLE_REDUCTION_B
            PRINTF("INFO: DR: Reduction B is enabled.\n");
#       else
            PRINTF("INFO: DR: Reduction B is off.\n");
#       endif

        break;
    }
    return err;
}

int DRFinalEdt_fcn(DRshared_t * o_sharedRef)
{
    int err = 0;
    err = destroy_DRshared(o_sharedRef);
    return err;
}

int DRinit_fcn(FFJ_Ledger_t * in_ledger, DRshared_t * in_shared, reductionPrivate_t * io_reducPrivate)
{
    int err = 0;
    while(!err){
        //DBG> PRINTF("INFO: DBG: at DRinit_fcn rankid=%u\n", in_ledger->rankid);

        io_reducPrivate->nrank  = in_ledger->nrank;
        io_reducPrivate->myrank = in_ledger->rankid;
        io_reducPrivate->ndata  = 1; // 1 means that the object we are going to allReduce will be a scalar.
        io_reducPrivate->reductionOperator = REDUC_OPERATOR;
        io_reducPrivate->rangeGUID = in_shared->reductionRangeGUID;
        io_reducPrivate->new = 1;
        io_reducPrivate->type = REDUC_OPERATION_TYPE;

        ocrEventParams_t params;
        params.EVENT_CHANNEL.maxGen = 2; //2 for channel exchange
        params.EVENT_CHANNEL.nbSat = 1;
        params.EVENT_CHANNEL.nbDeps = 1;
        err = ocrEventCreateParams(&(io_reducPrivate->returnEVT), OCR_EVENT_CHANNEL_T, false, &params); IFEB;

        err = clear_DRshared(in_shared); IFEB;

        TimeMark_t t = getTime();
        in_ledger->at_DRinit_fcn = t;
        break;
    }
    return err;
}

int DR_reduxA_start_fcn(FFJ_Ledger_t * in_ledger, unsigned int in_multiplier,
                        ocrGuid_t io_reducPrivateGuid, reductionPrivate_t * io_reducPrivate,
                        unsigned int in_destSlot, ocrGuid_t in_destinationGuid)
{
    int err = 0;
    while(!err){
        ReducSum_t sum = 0;

        const unsigned int val = in_ledger->rankid + 1U;
        sum = val * in_multiplier;

#       ifdef DR_ENABLE_REDUCTION_A
            err = ocrAddDependence(io_reducPrivate->returnEVT, in_destinationGuid, in_destSlot, DB_MODE_RO); IFEB;
            reductionLaunch(io_reducPrivate, io_reducPrivateGuid, &sum);
#       else
            ocrGuid_t gd_sum = NULL_GUID;
            ReducSum_t * o_sum = NULL;
            err = ocrDbCreate( &gd_sum, (void**)&o_sum, 1*sizeof(ReducSum_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC); IFEB;
            *o_sum = sum;
            err = ocrDbRelease(gd_sum); IFEB;
            err = ocrXHookup(OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG, in_destinationGuid, in_destSlot, DB_MODE_RO, gd_sum); IFEB;
            err = ocrDbRelease(io_reducPrivateGuid); IFEB; //This is to imitate the release done by reductionLaunch().
#       endif

        TimeMark_t t = getTime();
        in_ledger->at_DR_reduxA_start_fcn = t;

        break;
    }
    return err;
}

int DR_reduxA_stop_fcn(unsigned int in_multiplier, FFJ_Ledger_t * in_ledger, ReducSum_t * in_sum, ocrGuid_t in_sum_guid)
{
    int err = 0;
    while(!err){
        //TODO: Find a way to reclaim the memory and objects used by io_reducPrivate and reduction algorithm.
        TimeMark_t t = getTime();
        in_ledger->at_DR_reduxA_stop_fcn = t;

        ReducSum_t x = *in_sum;
        err = ocrDbDestroy( in_sum_guid ); IFEB;

        ReducSum_t expected = 0;

#       ifdef DR_ENABLE_REDUCTION_A
            expected = in_ledger->nrank * (in_ledger->nrank + 1) /2 * in_multiplier;
#       else
            const unsigned int val = in_ledger->rankid + 1U;
            expected = val * in_multiplier;
#       endif

        //DBG> PRINTF("INFO: DR_redux_stop> rank=%u sum=%24.14E expected=%24.14E\n", in_ledger->rankid, (double)x, (double)expected);

        double delta = x - expected;
        if(delta<0) delta = -delta;

        if(delta > 1e-14){
            PRINTF("ERROR: DR_reduxA_stop> rank=%u> Too much difference found: sum=%24.14E expected=%24.14E\n",
                    in_ledger->rankid, (double)x, (double)expected);
            err = __LINE__; break;
        }
        break;
    }
    return err;
}

/**
 * io_reducPrivateGuid: where we want the reduction answer to be written to
 * reductionPrivate_t: reduction configuration data-structure
 */
int DR_reduxB_start_fcn(FFJ_Ledger_t * in_ledger, unsigned int in_multiplier,
                        ocrGuid_t io_reducPrivateGuid, reductionPrivate_t * io_reducPrivate,
                        unsigned int in_destSlot, ocrGuid_t in_destinationGuid)
{
    int err = 0;
    while(!err){
        ReducSum_t sum = 0;

        const unsigned int val = in_ledger->rankid + 1U;
        sum = val * in_multiplier;

#       ifdef DR_ENABLE_REDUCTION_B
#ifdef DR_ENABLE_REDUCTION_EVENT
            err = ocrAddDependence(io_reducPrivate->returnEVT, in_destinationGuid, in_destSlot, DB_MODE_RO); IFEB;
#else
            err = ocrAddDependence(io_reducPrivate->returnEVT, in_destinationGuid, in_destSlot, DB_MODE_RO); IFEB;
            reductionLaunch(io_reducPrivate, io_reducPrivateGuid, &sum);
#endif
#       else
            ocrGuid_t gd_sum = NULL_GUID;
            ReducSum_t * o_sum = NULL;
            err = ocrDbCreate( &gd_sum, (void**)&o_sum, 1*sizeof(ReducSum_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC); IFEB;
            *o_sum = sum;
            err = ocrDbRelease(gd_sum); IFEB;
            err = ocrXHookup(OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG, in_destinationGuid, in_destSlot, DB_MODE_RO, gd_sum); IFEB;
            err = ocrDbRelease(io_reducPrivateGuid); IFEB; //This is to imitate the release done by reductionLaunch().
#       endif

        TimeMark_t t = getTime();
        in_ledger->at_DR_reduxB_start_fcn = t;

        break;
    }
    return err;
}

int DR_reduxB_stop_fcn(unsigned int in_multiplier, FFJ_Ledger_t * in_ledger, ReducSum_t * in_sum, ocrGuid_t in_sum_guid)
{
    int err = 0;
    while(!err){
        //TODO: Find a way to reclaim the memory and objects used by io_reducPrivate and reduction algorithm.

        TimeMark_t t = getTime();
        in_ledger->at_DR_reduxB_stop_fcn = t;
        TimeMark_t dt = t - in_ledger->at_DR_reduxB_start_fcn;
        if(dt<0) dt=-dt; //Just to make sure we get something that is monotonously increasing.
        in_ledger->cumulsum_DR_reduxB_stop_fcn += dt;

        ReducSum_t x = *in_sum;
        err = ocrDbDestroy( in_sum_guid ); IFEB;

        ReducSum_t expected = 0;

#       ifdef DR_ENABLE_REDUCTION_B
            expected = in_ledger->nrank * (in_ledger->nrank + 1) /2 * in_multiplier;
#       else
            const unsigned int val = in_ledger->rankid + 1U;
            expected = val * in_multiplier;
#       endif

        //DBG> PRINTF("INFO: DR_redux_stop> rank=%u sum=%24.14E expected=%24.14E\n", in_ledger->rankid, (double)x, (double)expected);

        double delta = x - expected;
        if(delta<0) delta = -delta;

        if(delta > 1e-14){
            PRINTF("ERROR: DR_reduxB_stop> rank=%u> Too much difference found: sum=%24.14E expected=%24.14E\n",
                    in_ledger->rankid, (double)x, (double)expected);
            err = __LINE__; break;
        }
        break;
    }
    return err;
}

void print_DR_Ledger_timings(FFJ_Ledger_t * in)
{
    TimeMark_t t_reduxA = in->at_DR_reduxA_stop_fcn - in->at_DR_reduxA_start_fcn;
    TimeMark_t t_reduxB = in->at_DR_reduxB_stop_fcn - in->at_DR_reduxB_start_fcn;

    TIMEPRINT5("INFO: TIME: DR rank=%u/%u reduxA,reduxB,cumulReduxB="TIMEF","TIMEF","TIMEF"\n",
               in->rankid,in->nrank, t_reduxA, t_reduxB, in->cumulsum_DR_reduxB_stop_fcn);
}


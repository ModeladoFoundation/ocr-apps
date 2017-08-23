#ifndef PTEST_ReductionEvent_H
#include "ptest_reductionEvent.h"
#endif

#define ENABLE_EXTENSION_LABELING  // For labeled GUIDs
#include "extensions/ocr-labeling.h"  // For labeled GUIDs

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
        err = ocrGuidRangeCreate(&reductionRangeGUID, 1, GUID_USER_EVENT_REDUCTION); IFEB;

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
        ocrGuid_t evtGuid;
        ocrGuidFromIndex(&evtGuid, in_shared->reductionRangeGUID, 0);
        ocrEventParams_t params;
        params.EVENT_REDUCTION.maxGen        = TEST_MAXGEN;
        params.EVENT_REDUCTION.nbContribs    = in_ledger->nrank;
        params.EVENT_REDUCTION.nbContribsPd  = (in_ledger->nrank / in_ledger->OCR_affinityCount);
        params.EVENT_REDUCTION.nbDatum       = TEST_NBDATUM;
        params.EVENT_REDUCTION.arity         = TEST_ARITY;
        params.EVENT_REDUCTION.op            = TEST_OP;
        params.EVENT_REDUCTION.type          = TEST_TYPE;
        params.EVENT_REDUCTION.reuseDbPerGen = TEST_REUSEDBPERGEN;
        ocrEventCreateParams(&evtGuid, OCR_EVENT_REDUCTION_T, GUID_PROP_IS_LABELED, &params);
        io_reducPrivate->evtGuid = evtGuid;
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
            err = ocrAddDependenceSlot(io_reducPrivate->evtGuid, (u32) in_ledger->rankid, in_destinationGuid, in_destSlot, DB_MODE_RO); IFEB;
            err = ocrEventReductionSatisfySlot(io_reducPrivate->evtGuid, &sum, (u32) in_ledger->rankid); IFEB;
            // reductionLaunch(io_reducPrivate, io_reducPrivateGuid, &sum);
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
        TimeMark_t t = getTime();
        in_ledger->at_DR_reduxA_stop_fcn = t;

        ReducSum_t x = *in_sum;
        // err = ocrDbDestroy( in_sum_guid ); IFEB;
        err = ocrDbRelease( in_sum_guid ); IFEB;

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

int DR_reduxB_start_fcn(FFJ_Ledger_t * in_ledger, unsigned int in_multiplier,
                        ocrGuid_t io_reducPrivateGuid, reductionPrivate_t * io_reducPrivate,
                        unsigned int in_destSlot, ocrGuid_t in_destinationGuid)
{
    int err = 0;
    while(!err){
        ReducSum_t sum = 0;
        const unsigned int rankid = in_ledger->rankid;
        const unsigned int val = rankid + 1U;
        sum = val * in_multiplier;

#       ifdef DR_ENABLE_REDUCTION_B
            err = ocrAddDependenceSlot(io_reducPrivate->evtGuid, (u32) in_ledger->rankid, in_destinationGuid, in_destSlot, DB_MODE_RO); IFEB;
            err = ocrEventReductionSatisfySlot(io_reducPrivate->evtGuid, &sum, (u32) in_ledger->rankid); IFEB;
            // err = ocrAddDependence(io_reducPrivate->returnEVT, in_destinationGuid, in_destSlot, DB_MODE_RO); IFEB;
            // reductionLaunch(io_reducPrivate, io_reducPrivateGuid, &sum);
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
        TimeMark_t t = getTime();
        in_ledger->at_DR_reduxB_stop_fcn = t;
        TimeMark_t dt = t - in_ledger->at_DR_reduxB_start_fcn;
        if(dt<0) dt=-dt; //Just to make sure we get something that is monotonously increasing.
        in_ledger->cumulsum_DR_reduxB_stop_fcn += dt;

        ReducSum_t x = *in_sum;
        err = ocrDbRelease( in_sum_guid ); IFEB;
        // err = ocrDbDestroy( in_sum_guid ); IFEB;

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


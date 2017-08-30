#ifndef PTEST_REDUCTION_H
#include "ptest_FORforkjoin.h"
#endif

#define XMEMSET(SRC, CHARC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)SRC+xmIT)=CHARC;}
#define XMEMCPY(DEST, SRC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)DEST+xmIT)=*((char*)SRC+xmIT);}

#define IFEB if(err) break

#ifndef RECURB_ITER_COUNT
#   define RECURB_ITER_COUNT 10000
#endif

int init_FFJ_Ledger(unsigned int in_rankid, unsigned int in_nrank, FFJ_Ledger_t * io)
{
    XMEMSET(io,0,sizeof(FFJ_Ledger_t));
    io->rankid = in_rankid;
    io->nrank  = in_nrank;
    io->nb_iteration_for_recurB = RECURB_ITER_COUNT;
    PRINTF("INFO: FFJ_Ledger: Number of iterationB requested= %ld\n", io->nb_iteration_for_recurB);
    return 0;
}
int clear_FFJ_Ledger(FFJ_Ledger_t * io)
{
    XMEMSET(io,0,sizeof(FFJ_Ledger_t));
    return 0;
}
int destroy_FFJ_Ledger(FFJ_Ledger_t * io)
{
    int err=0;
    XMEMSET(io,0,sizeof(FFJ_Ledger_t));
    return err;
}
int copy_FFJ_Ledger(FFJ_Ledger_t * in_from, FFJ_Ledger_t * o_target)
{
    if( !in_from || !o_target) return __LINE__;
    XMEMCPY(o_target, in_from, sizeof(FFJ_Ledger_t));
    return 0;
}
void  print_FFJ_Ledger(FFJ_Ledger_t * in)
{
    PRINTF("INFO: FFJ_Ledger>[myrank,nrank,affinity,pdID]= %u %u %ld %lu\n",
           in->rankid, in->nrank, in->OCR_affinityCount, in->pdID
           );
}
void print_FFJ_Ledger_timings(FFJ_Ledger_t * in)
{
    TimeMark_t t0 = in->at_setup;
    TimeMark_t t_scatter = in->at_FOR_transit_start - t0;
    TimeMark_t t_FOR_transit_start_stop = in->at_FOR_transit_stop - in->at_FOR_transit_start;

    TIMEPRINT4("INFO: TIME: rank=%u/%u scatter,FORtransit= "TIMEF","TIMEF"\n",
               in->rankid,in->nrank, t_scatter, t_FOR_transit_start_stop);
}

int ffjMainEdt_fcn(ocrGuid_t * in_gDone, ocrGuid_t * o_gDone)
{
    int err = 0;
    while(!err){
        GUID_ASSIGN_VALUE(*o_gDone, *in_gDone);
        TimeMark_t t = getTime();
        TIMEPRINT1("INFO: TIME: ffjMainEdt_fcn_time_mark=  "TIMEF"\n", t);
        break;
    }
    return err;
}

int ffjFinalEdt_fcn(void)
{
    int err = 0;
    while(!err){
        PRINTF("INFO: ffjFinalEdt_fcn reached.\n");
        TimeMark_t t = getTime();
        TIMEPRINT1("INFO: TIME: ffjFinalEdt_fcn_time_mark= "TIMEF"\n", t);
        break;
    }
    return err;
}

int ffjSetup_fcn(unsigned int in_nrank, FFJ_Ledger_t * o_ffjLedger,
                 ocrGuid_t * in_gDoneFOR, ocrGuid_t * o_gDoneFOR)
{
    int err = 0;
    while(!err){
        err = init_FFJ_Ledger(FFJ_NOTA_RANK, in_nrank, o_ffjLedger); IFEB;
        GUID_ASSIGN_VALUE(*o_gDoneFOR, *in_gDoneFOR);

        TimeMark_t t = getTime();
        o_ffjLedger->at_setup = t;

        o_ffjLedger->OCR_affinityCount = 0;

#       ifdef FFJ_ENABLE_AFFINITIES
            u64 affinityCount;
            err = ocrAffinityCount( AFFINITY_PD, &affinityCount ); IFEB;
            o_ffjLedger->OCR_affinityCount = affinityCount;
            if(o_ffjLedger->OCR_affinityCount < 0) {err = __LINE__; IFEB;}
            PRINTF("INFO: Affinities are in use. Strategy = %d.\n", (int) FFJ_AFFINITY_STRATEGY);
#       else
            PRINTF("INFO: Affinities are not used.\n");
#       endif

        //DBG> print_FFJ_Ledger(o_ffjLedger);
        break;
    }
    return err;
}

int ffjFOR_fcn(int in_rank, unsigned long in_pdID,
               FFJ_Ledger_t * in_ffjLedger, FFJ_Ledger_t * o_ffjLedger,
               ocrGuid_t * in_gDoneFOR, ocrGuid_t * o_gDoneFOR)
{
    int err = 0;
    while(!err){
        copy_FFJ_Ledger(in_ffjLedger, o_ffjLedger);
        o_ffjLedger->rankid = in_rank;
        o_ffjLedger->pdID = in_pdID;

        //DBG> print_FFJ_Ledger(o_ffjLedger);

        GUID_ASSIGN_VALUE(*o_gDoneFOR, *in_gDoneFOR);

        TimeMark_t t = getTime();
        o_ffjLedger->at_FORbegin = t;

        break;
    }
    return err;
}

int ffjFOR_Transist_start_fcn(FFJ_Ledger_t * io_ffjLedger)
{
    int err = 0;
    while(!err){
        TimeMark_t t = getTime();
        io_ffjLedger->at_FOR_transit_start = t;

        //DBG> PRINTF("INFO: DBG: at ffjFOR_Transist_start_fcn rankid=%u\n", io_ffjLedger->rankid);

        break;
    }
    return err;
}

int ffjFOR_Transist_stop_fcn(FFJ_Ledger_t * io_ffjLedger)
{
    int err = 0;
    while(!err){
        TimeMark_t t = getTime();
        io_ffjLedger->at_FOR_transit_stop = t;

        print_FFJ_Ledger(io_ffjLedger);
        print_FFJ_Ledger_timings(io_ffjLedger);
        break;
    }
    return err;
}

int ffjROF_fcn(void)
{
    int err = 0;
    while(!err){
        break;
    }
    return err;
}

int ocrXgetEdtHint(unsigned long in_pdID, ocrHint_t * io_HNT, ocrHint_t ** o_pHNT)
{
    int err = 0;
    while(!err){
        *o_pHNT = NULL_HINT;
#       ifdef FFJ_ENABLE_AFFINITIES
            err = ocrHintInit( io_HNT, OCR_HINT_EDT_T ); IFEB;

            ocrGuid_t currentAffinity = NULL_GUID;
            if(in_pdID == FFJ_USE_CURRENT_PD){
                ocrAffinityGetCurrent( &currentAffinity );
            }else{
                ocrAffinityGetAt( AFFINITY_PD, in_pdID, &currentAffinity );
            }

            ocrSetHintValue( io_HNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue( currentAffinity ) );
            *o_pHNT = io_HNT;
#       endif
        break;
    }
    return err;
}

int ocrXgetDbkHint(unsigned long in_pdID, ocrHint_t * io_HNT, ocrHint_t ** o_pHNT)
{
    int err = 0;
    while(!err){
        *o_pHNT = NULL_HINT;
#       ifdef FFJ_ENABLE_AFFINITIES
            err = ocrHintInit( io_HNT, OCR_HINT_DB_T ); IFEB;

            ocrGuid_t currentAffinity = NULL_GUID;
            if(in_pdID == FFJ_USE_CURRENT_PD){
                ocrAffinityGetCurrent( &currentAffinity );
            }else{
                ocrAffinityGetAt( AFFINITY_PD, in_pdID, &currentAffinity );
            }

            ocrSetHintValue( io_HNT, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue( currentAffinity ) );
            *o_pHNT = io_HNT;
#       endif
        break;
    }
    return err;
}

unsigned long calculate_pid0(unsigned int in_rankid, long in_affinityCount, unsigned int in_nrank)
{
    return 0;
}

unsigned long calculate_pid1(unsigned int in_rankid, long in_affinityCount, unsigned int in_nrank)
{
    unsigned long pd =0;
    const unsigned long nb_rankPerPolicydomain = in_nrank / in_affinityCount;

    //DBG> PRINTF("INFO: DBG: r=%u nb_rankPerPolicydomain=%lu\n", in_rankid, nb_rankPerPolicydomain);

    if(nb_rankPerPolicydomain == 0) {
        //There are more policy domain than ranks.
        const unsigned long minNb_ranks_per_pd = 8;

        pd = in_nrank / minNb_ranks_per_pd;
    } else {
        pd = in_rankid / nb_rankPerPolicydomain;
        if(pd >= in_affinityCount){
            //Making sure the tail excess goes toward the last domain.
            pd = in_affinityCount - 1;
        }
    }

    return pd;
}

unsigned long calculate_pid(unsigned int in_rankid, long in_affinityCount, unsigned int in_nrank)
{
    unsigned long x = 0;

    const int choice = FFJ_AFFINITY_STRATEGY;
    switch(choice){
    default:
    case 1:
        x = calculate_pid1(in_rankid, in_affinityCount, in_nrank);
        break;
    case 0:
        x = calculate_pid0(in_rankid, in_affinityCount, in_nrank);
        break;
    }

    return x;
}

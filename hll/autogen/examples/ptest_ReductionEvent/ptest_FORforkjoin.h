#ifndef PTEST_REDUCTION_H
#define PTEST_REDUCTION_H

#include "ocr.h"
#include "ocr-std.h"
#include "extensions/ocr-affinity.h"

#include "timing.h"

#ifndef COUNT_FOR_FORKJOIN
#define COUNT_FOR_FORKJOIN 8  //The number of "ranks" generated by the FOR task.
#endif

typedef struct FFJ_Ledger {
    unsigned int rankid;  //This will also be used to determined the PD: PD = rankid / RANK_PER_PDOMAIN
    unsigned int nrank; //The number of simulated ranks
    unsigned long pdID;

    long nb_iteration_for_recurB;

    TimeMark_t at_setup;
    TimeMark_t at_FORbegin;
    TimeMark_t at_FOR_transit_start;
    TimeMark_t at_FOR_transit_stop;

    TimeMark_t at_DRinit_fcn;
    TimeMark_t at_DR_reduxA_start_fcn;
    TimeMark_t at_DR_reduxA_stop_fcn;
    TimeMark_t at_DR_reduxB_start_fcn;
    TimeMark_t at_DR_reduxB_stop_fcn;
    TimeMark_t cumulsum_DR_reduxB_stop_fcn;

    long OCR_affinityCount;

} FFJ_Ledger_t;
#define FFJ_NOTA_RANK ((unsigned int)(-1))

#define FFJ_ENABLE_AFFINITIES
#ifndef ENABLE_EXTENSION_AFFINITY
#   undef FFJ_ENABLE_AFFINITIES
#endif

//In order to find out which strategies are available, see calculate_pid().

#ifndef FFJ_AFFINITY_STRATEGY
#define FFJ_AFFINITY_STRATEGY 1
#endif

#define FFJ_USE_CURRENT_PD ((unsigned long)-2)

//These functions return zero upon success.
int init_FFJ_Ledger(unsigned int in_rankid, unsigned int in_nrank, FFJ_Ledger_t * io);
int clear_FFJ_Ledger(FFJ_Ledger_t * io);
int destroy_FFJ_Ledger(FFJ_Ledger_t * io);
int copy_FFJ_Ledger(FFJ_Ledger_t * in_from, FFJ_Ledger_t * o_target);
void print_FFJ_Ledger(FFJ_Ledger_t * in);
void print_FFJ_Ledger_timings(FFJ_Ledger_t * in);

int ffjMainEdt_fcn(ocrGuid_t * in_gDone, ocrGuid_t * o_gDone);
int ffjFinalEdt_fcn(void);
int ffjSetup_fcn(unsigned int in_nrank, FFJ_Ledger_t * o_ffjLedger,
                 ocrGuid_t * in_gDoneFOR, ocrGuid_t * o_gDoneFOR);
int ffjFOR_fcn(int in_rank, unsigned long in_pdID,
               FFJ_Ledger_t * in_ffjLedger, FFJ_Ledger_t * o_ffjLedger,
               ocrGuid_t * in_gDoneFOR, ocrGuid_t * o_gDoneFOR);
int ffjFOR_Transist_start_fcn(FFJ_Ledger_t * io_ffjLedger);
int ffjFOR_Transist_stop_fcn(FFJ_Ledger_t * io_ffjLedger);
int ffjROF_fcn(void);

unsigned long calculate_pid(unsigned int in_rankid, long in_affinityCount, unsigned int in_nrank);

int ocrXgetEdtHint(unsigned long in_pdID, ocrHint_t * io_HNT, ocrHint_t ** o_pHNT);
int ocrXgetDbkHint(unsigned long in_pdID, ocrHint_t * io_HNT, ocrHint_t ** o_pHNT);

#endif // PTEST_REDUCTION_H
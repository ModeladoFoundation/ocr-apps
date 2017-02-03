#ifndef PTEST_DAVEREDUCTION_H
#define PTEST_DAVEREDUCTION_H

#include "ocr.h"
#include "ocr-std.h"

#define IFEB if(err) break

#define DR_ENABLE_REDUCTION_A //Comment this out in order to disable the allreduce operation in reduxA
#define DR_ENABLE_REDUCTION_B //Comment this out in order to disable the allreduce operation in reduxB

#define SLOT4REDUCTION 4

typedef double ReducSum_t;
#define REDUC_OPERATION_TYPE REDUCTION_F8_ADD

#define DR_MULTIPLIER_A 1
#define DR_MULTIPLIER_B 2

typedef struct reductionPrivateBase reductionPrivate_t; //Forward declaration
typedef struct FFJ_Ledger FFJ_Ledger_t; //Forward declaration

typedef struct DRshared {
    ocrGuid_t reductionRangeGUID;   //The ranged Guids set, of size nrank
} DRshared_t;

//These functions return zero upon success.
int init_DRshared(DRshared_t * io);
int clear_DRshared(DRshared_t * io);
int destroy_DRshared(DRshared_t * io);
int copy_DRshared(DRshared_t * in_from, DRshared_t * o_target);
void print_DRshared(DRshared_t * in);

int DRmainEdt_fcn(DRshared_t * o_sharedRef, DRshared_t * o_shared, unsigned long in_nrank);
int DRFinalEdt_fcn(DRshared_t * o_sharedRef);

int DRinit_fcn(FFJ_Ledger_t * in_ledger, DRshared_t * in_shared, reductionPrivate_t * io_reducPrivate);

int DR_reduxA_start_fcn(FFJ_Ledger_t * in_ledger, unsigned int in_multiplier,
                        ocrGuid_t io_reducPrivateGuid, reductionPrivate_t * io_reducPrivate,
                        unsigned int in_destSlot, ocrGuid_t in_destinationGuid);
int DR_reduxA_stop_fcn(unsigned int in_multiplier, FFJ_Ledger_t * in_ledger, ReducSum_t * in_sum, ocrGuid_t in_sum_guid);

int DR_reduxB_start_fcn(FFJ_Ledger_t * in_ledger, unsigned int in_multiplier,
                        ocrGuid_t io_reducPrivateGuid, reductionPrivate_t * io_reducPrivate,
                        unsigned int in_destSlot, ocrGuid_t in_destinationGuid);
int DR_reduxB_stop_fcn(unsigned int in_multiplier, FFJ_Ledger_t * in_ledger, ReducSum_t * in_sum, ocrGuid_t in_sum_guid);

void print_DR_Ledger_timings(FFJ_Ledger_t * in);

#endif // PTEST_DAVEREDUCTION_H

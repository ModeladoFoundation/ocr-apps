#ifndef PTEST_REDUCTIONEVENT_H
#define PTEST_REDUCTIONEVENT_H

#define ENABLE_EXTENSION_REDUCTION_EVT 1
#define ENABLE_MULTI_OUTPUT_SLOT 1
#define ENABLE_EXTENSION_PARAMS_EVT 1

#include "ocr.h"
#include "ocr-std.h"

#define IFEB if(err) break

#define DR_ENABLE_REDUCTION_A //Comment this out in order to disable the allreduce operation in reduxA
#define DR_ENABLE_REDUCTION_B //Comment this out in order to disable the allreduce operation in reduxB

#define SLOT4REDUCTION_A 4
#define SLOT4REDUCTION_B 5  //This includes the slot for reduction.

typedef double ReducSum_t;
#ifndef REDUC_OPERATOR
#define REDUC_OPERATOR REDOP_F8_ADD
#endif
#ifndef REDUC_OPERATION_TYPE
#define REDUC_OPERATION_TYPE REDOP_ALLREDUCE
#endif

#ifndef TEST_MAXGEN
#define TEST_MAXGEN 1
#endif
#ifndef TEST_NBCONTRIBSPD
#define TEST_NBCONTRIBSPD 2
#endif
#ifndef TEST_NBDATUM
#define TEST_NBDATUM 1
#endif
#ifndef TEST_ARITY
#define TEST_ARITY ARITY
#endif
#ifndef TEST_OP
#define TEST_OP REDUC_OPERATOR
#endif
#ifndef TEST_TYPE
#define TEST_TYPE REDUC_OPERATION_TYPE
#endif
#ifndef TEST_REUSEDBPERGEN
#define TEST_REUSEDBPERGEN true
#endif

#define DR_MULTIPLIER_A 1
#define DR_MULTIPLIER_B 2

typedef struct reductionPrivate_t {
    ocrGuid_t evtGuid;
} reductionPrivate_t;

struct FFJ_Ledger; //Forward declaration
// typedef struct FFJ_Ledger FFJ_Ledger_t; //Forward declaration
typedef struct DRshared {
    ocrGuid_t reductionRangeGUID;   //The range GUID holding the labeled reduction event's GUID
} DRshared_t;

//These functions return zero upon success.
int init_DRshared(DRshared_t * io);
int clear_DRshared(DRshared_t * io);
int destroy_DRshared(DRshared_t * io);
int copy_DRshared(DRshared_t * in_from, DRshared_t * o_target);
void print_DRshared(DRshared_t * in);

int DRmainEdt_fcn(DRshared_t * o_sharedRef, DRshared_t * o_shared, unsigned long in_nrank);
int DRFinalEdt_fcn(DRshared_t * o_sharedRef);

int DRinit_fcn(struct FFJ_Ledger * in_ledger, DRshared_t * in_shared, reductionPrivate_t * io_reducPrivate);

int DR_reduxA_start_fcn(struct FFJ_Ledger * in_ledger, unsigned int in_multiplier,
                        ocrGuid_t io_reducPrivateGuid, reductionPrivate_t * io_reducPrivate,
                        unsigned int in_destSlot, ocrGuid_t in_destinationGuid);
int DR_reduxA_stop_fcn(unsigned int in_multiplier, struct FFJ_Ledger * in_ledger, ReducSum_t * in_sum, ocrGuid_t in_sum_guid);

int DR_reduxB_start_fcn(struct FFJ_Ledger * in_ledger, unsigned int in_multiplier,
                        ocrGuid_t io_reducPrivateGuid, reductionPrivate_t * io_reducPrivate,
                        unsigned int in_destSlot, ocrGuid_t in_destinationGuid);
int DR_reduxB_stop_fcn(unsigned int in_multiplier, struct FFJ_Ledger * in_ledger, ReducSum_t * in_sum, ocrGuid_t in_sum_guid);

void print_DR_Ledger_timings(struct FFJ_Ledger * in);

#endif // PTEST_REDUCTIONEVENT_H

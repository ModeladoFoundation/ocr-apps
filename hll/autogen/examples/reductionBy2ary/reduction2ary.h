#ifndef REDUCTION_2ARY_H
#define REDUCTION_2ARY_H

#include "ocr.h" //ocrGuid_t, PRINTF
#include "../../tools/app_ocr_err_util.h"
#include "../../tools/app_ocr_debug_util.h"

#include "bintreeForkjoin.h"
#include "global_data.h"

//=The model for the reduction facility is that one has <edtCount> EDTs, each
// of which has a value <value> to reduce and then distribute back to all the
// initial EDTs.
//=It is assumed that starting EDTs have been bracketed:
//      Assume the name of one of the starting EDTs is "A".
//      Then "A" bracketed itself by creating itself "a":
//
//          --> A --------> a ----> elsewhere
// And the reduction facility is inserted in parallel, and "A" and "a" have
// been modified to accept the reduction flow:
//
//          --> A --------------------> a ----> elsewhere
//              |                       ^
//       ledger |                       | R2aryValue_t
//              |-----reduction---------|
//
//  On the branch to the reduction facility, both the guid of "a" and <value>
//  The data sent to the reduction facility is of type LedgerReduc2ary_t.
//  The data returned by the reduction facility is of type R2aryValue_t.

typedef struct R2aryValue{
    double x;
} R2aryValue_t;

void init_R2aryValue(R2aryValue_t * io_this);  //Assumed to be the same as
void copy_R2aryValue(R2aryValue_t * io_source, R2aryValue_t * io_destination);
void add_R2aryValue(R2aryValue_t * in_val1, R2aryValue_t * io_val2);  //val2 += val1
void print_R2aryValue(OA_DEBUG_ARGUMENT, R2aryValue_t * io_this, const char * in_text);

typedef struct LedgerReduc2ary
{
    unsigned long edtCount; //The number of EDTs the reduction will be handling.
                           //In the MPI world, this would be what MPI_COMM_SIZE returns.

    unsigned long edtID; //This is number associated with the EDT.
                         //    In the MPI world, this would be what MPI_COMM_RANK returns.
    unsigned long leafID; //When given to the reduction facility, this->edtID
                          //will be converted to its corresponding leafID

    ocrGuid_t labelEDTmap;  //This points to a range of labeled EDTs.
                            //There are this->edtCount EDT guids in that range.

    ocrGuid_t whenDone;  //This is the guid where one is expected to go when
                         //done with current EDT.  Initialize it the bracketing
                         //EDT when starting the facility

    R2aryValue_t value; //Before entering the algorithm, set this to the value
                        // you want to add_R2aryValue
} LedgerReduc2ary_t;

Err_t create_reduction2ary_ledger(OA_DEBUG_ARGUMENT,
                                  LedgerReduc2ary_t * io_ledger_r2ary,
                                  unsigned long in_edtCount,
                                  unsigned long in_edtID,
                                  ocrGuid_t * in_labelEDTmap,
                                  ocrGuid_t * in_whenDone,
                                  R2aryValue_t * in_value
                                  );
Err_t destroy_reduction2ary_ledger(OA_DEBUG_ARGUMENT, LedgerReduc2ary_t * io_ledger_r2ary);
void copy_reduction2ary_ledger(LedgerReduc2ary_t * in_ledger, LedgerReduc2ary_t * o_ledger);
void print_reduction2ary_ledger(OA_DEBUG_ARGUMENT, LedgerReduc2ary_t * io_ledger_r2ary, const char * in_text);

typedef struct R2aryWhenDone
{
    ocrGuid_t wdone[2];
    unsigned long stopCriteria;  //If zero, then this is the last node
                                 // before the R2aryConclusion EDT.
                                 //It is set in R2aryJoinIF(),  and once in setup_reduction2ary()
} R2aryWhenDone_t;

void init_R2aryWhenDone(R2aryWhenDone_t * io_this);
void copy_R2aryWhenDone(R2aryWhenDone_t * io_source, R2aryWhenDone_t * io_destination);
void print_R2aryWhenDone(OA_DEBUG_ARGUMENT, R2aryWhenDone_t * io_this, const char * in_text);


Err_t btForkTransitStart(OA_DEBUG_ARGUMENT, TFJiterate_t * in_FJiterate,
                         GlobalData_t * in_globals, ocrGuid_t * in_whenDone,
                         LedgerReduc2ary_t * o_ledgerR2ary);

Err_t R2ary_ValidateValue_in_btForkTransit_stop(OA_DEBUG_ARGUMENT,
                                                GlobalData_t * in_globals,
                                                R2aryValue_t * in_r2aryValue,
                                                TFJiterate_t * in_FJiterate);


Err_t setup_reduction2ary(OA_DEBUG_ARGUMENT,
                          LedgerReduc2ary_t * o_ledgerR2ary,
                          R2aryWhenDone_t * o_whenDone,
                          LedgerReduc2ary_t * in_ledgerR2ary,
                          ocrGuid_t * in_whenDone,
                          ocrEdt_t in_funcPtr,
                          ocrGuid_t * io_g4nextEdt,
                          unsigned int * io_dataSlot2use
                          );

int conditionR2aryJoinIF(OA_DEBUG_ARGUMENT,
                         LedgerReduc2ary_t * in_ledgerA,
                         LedgerReduc2ary_t * in_ledgerB);//Return 1 upon success; zero otherwise.

Err_t R2aryJoinIF_fcn(OA_DEBUG_ARGUMENT,
                  ocrGuid_t * in_whereToNext,
                  LedgerReduc2ary_t * in_ledgerA,
                  LedgerReduc2ary_t * in_ledgerB,
                  LedgerReduc2ary_t * o_ledger,
                  R2aryWhenDone_t * o_r2aWdone,
                  ocrEdt_t in_funcPtr,
                  ocrGuid_t * io_g4nextEdt,
                  unsigned int * io_dataSlot2use
                  );

Err_t R2aryJoinELSE_fcn(OA_DEBUG_ARGUMENT,
                        LedgerReduc2ary_t * in_ledgerA,
                        LedgerReduc2ary_t * in_ledgerB,
                        R2aryValue_t * o_value,
                        R2aryWhenDone_t * o_r2aWdone);

int conditionR2aryForkIF(OA_DEBUG_ARGUMENT,
                         R2aryWhenDone_t * in_r2aWdone);//Return 1 upon success; zero otherwise.

Err_t R2aryForkIF_FOR_body(OA_DEBUG_ARGUMENT,
                           int in_index, R2aryWhenDone_t * in_r2aWdone,
                           R2aryValue_t * in_value, R2aryValue_t * o_value,
                           ocrGuid_t * o_whereTonext);

Err_t R2aryForkELSE_FOR_body(OA_DEBUG_ARGUMENT,
                             int in_index, R2aryWhenDone_t * in_r2aWdone,
                             R2aryValue_t * in_value, R2aryValue_t * o_value,
                             ocrGuid_t * o_whereTonext);

#endif //REDUCTION_2ARY_H

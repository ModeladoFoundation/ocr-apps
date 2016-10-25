#ifndef REDUCTION_2ARY_H
#include "reduction2ary.h"
#endif

#include "reduction2ary_indices.h"

#define ENABLE_EXTENSION_LABELING  // For labeled EDTs
#include "extensions/ocr-labeling.h"  // For labeled EDTs

#define XMEMSET(SRC, CHARC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)SRC+xmIT)=CHARC;}
#define XMEMCPY(DEST, SRC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)DEST+xmIT)=*((char*)SRC+xmIT);}

#define REDUCTION2ARY_VERBOSE //Define this macro in order to get many PRINTF activated.
                              //Otherwise, to keep silent, undefine the macro.

void init_R2aryValue(R2aryValue_t * io_this)
{
    io_this->x = 0;
}
void copy_R2aryValue(R2aryValue_t * io_source, R2aryValue_t * io_destination)
{
    io_destination->x = io_source->x;
}
void add_R2aryValue(R2aryValue_t * in_val1, R2aryValue_t * io_val2)
{
    io_val2->x += in_val1->x;
}
void print_R2aryValue(OA_DEBUG_ARGUMENT, R2aryValue_t * io_this, const char * in_text)
{
#   ifdef REDUCTION2ARY_VERBOSE
        const char * defaultt = "";
        const char * t = in_text;
        if(!t){
            t = defaultt;
        }

        PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" REDUC2ARY VALUE x=%e %s\n",
               in_edtType, GUIDA(in_thisEDT),
               io_this->x,
               t
               );
#   endif
}

Err_t create_reduction2ary_ledger(OA_DEBUG_ARGUMENT, LedgerReduc2ary_t * io_ledger_r2ary,
                                  unsigned long in_edtCount,
                                  unsigned long in_edtID,
                                  ocrGuid_t * in_labelEDTmap,
                                  ocrGuid_t * in_whenDone,
                                  R2aryValue_t * in_value
                                  )
{
    LedgerReduc2ary_t * L = io_ledger_r2ary;
    Err_t err=0;
    while(!err){
        if( in_edtCount ==0 ) {
            PRINTF("ERROR: create_reduction2ary_ledger: in_edtCount ==0 is true.\n");
            err=__LINE__; IFEB;
        }
        if( in_edtID >= in_edtCount ) {
            PRINTF("ERROR: create_reduction2ary_ledger: in_edtID >= in_edtCount is true.\n");
            err=__LINE__; IFEB;
        }

        GUID_ASSIGN_VALUE(L->labelEDTmap, NULL_GUID); //No default constructor.  So this will have to do.

        L->edtCount = in_edtCount;
        L->edtID = in_edtID;
        L->leafID = reduc2ary_calculate_leafID(in_edtCount, in_edtID);

        GUID_ASSIGN_VALUE(L->labelEDTmap, *in_labelEDTmap);
        GUID_ASSIGN_VALUE(L->whenDone, *in_whenDone);

        init_R2aryValue(&io_ledger_r2ary->value);
        copy_R2aryValue(in_value, &io_ledger_r2ary->value);

        print_reduction2ary_ledger(OA_DEBUG_INVARS, io_ledger_r2ary, "CREATE");
        break;  //  while(!err)
    }
    return err;
}

Err_t destroy_reduction2ary_ledger(OA_DEBUG_ARGUMENT, LedgerReduc2ary_t * io_ledger_r2ary)
{
    LedgerReduc2ary_t * L = io_ledger_r2ary;
    Err_t err=0;
    while(!err){
        print_reduction2ary_ledger(OA_DEBUG_INVARS, L, "DESTROY");
        XMEMSET(L, 0, sizeof(LedgerReduc2ary_t));
        GUID_ASSIGN_VALUE(L->labelEDTmap, NULL_GUID); // No guarantee that NULL_GUID is 0x0
        init_R2aryValue(&io_ledger_r2ary->value);
        break;  //  while(!err)
    }
    return err;
}

void copy_reduction2ary_ledger(LedgerReduc2ary_t * in_ledger, LedgerReduc2ary_t * o_ledger)
{
    XMEMCPY(o_ledger, in_ledger, sizeof(LedgerReduc2ary_t));
    GUID_ASSIGN_VALUE(o_ledger->labelEDTmap, in_ledger->labelEDTmap);
    GUID_ASSIGN_VALUE(o_ledger->whenDone,    in_ledger->whenDone);
    copy_R2aryValue(&in_ledger->value, &o_ledger->value);
}

void print_reduction2ary_ledger(OA_DEBUG_ARGUMENT, LedgerReduc2ary_t * io_ledger_r2ary,
                                const char * in_text)
{
#   ifdef REDUCTION2ARY_VERBOSE
        const char * defaultt = "";
        const char * t = in_text;
        if(!t){
            t = defaultt;
        }

        ocrGuid_t emap = NULL_GUID;
        ocrGuid_t wdone = NULL_GUID;
        if( ! ocrGuidIsNull(in_thisEDT)){
            GUID_ASSIGN_VALUE(emap, io_ledger_r2ary->labelEDTmap);
            GUID_ASSIGN_VALUE(wdone, io_ledger_r2ary->whenDone);
        }

        PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" REDUC2ARY LEDGER edtCount=%lu edtID=%lu leafID=%lu, edtMap="GUIDF" whenDone="GUIDF" %s\n",
                in_edtType, GUIDA(in_thisEDT),
                io_ledger_r2ary->edtCount,
                io_ledger_r2ary->edtID,
                io_ledger_r2ary->leafID,
                GUIDA(emap),
                GUIDA(wdone),
                t
               );
        print_R2aryValue(OA_DEBUG_INVARS, &io_ledger_r2ary->value, in_text);
#   endif
}

void init_R2aryWhenDone(R2aryWhenDone_t * io_this)
{
    GUID_ASSIGN_VALUE(io_this->wdone[0], NULL_GUID);
    GUID_ASSIGN_VALUE(io_this->wdone[1], NULL_GUID);
}
void copy_R2aryWhenDone(R2aryWhenDone_t * io_source, R2aryWhenDone_t * io_destination)
{
    GUID_ASSIGN_VALUE(io_destination->wdone[0], io_source->wdone[0]);
    GUID_ASSIGN_VALUE(io_destination->wdone[1], io_source->wdone[1]);
}
void print_R2aryWhenDone(OA_DEBUG_ARGUMENT, R2aryWhenDone_t * io_this, const char * in_text)
{
#   ifdef REDUCTION2ARY_VERBOSE
        const char * defaultt = "";
        const char * t = in_text;
        if(!t){
            t = defaultt;
        }

        ocrGuid_t leftg = NULL_GUID;
        ocrGuid_t rightg = NULL_GUID;
        if( ! ocrGuidIsNull(in_thisEDT)){
            GUID_ASSIGN_VALUE(leftg, io_this->wdone[0]);
            GUID_ASSIGN_VALUE(rightg, io_this->wdone[1]);
        }

        PRINTF("TESTIO> REDUC2ARY WHENDONE leftg="GUIDF" rightg="GUIDF" %s\n",
                GUIDA(leftg),
                GUIDA(rightg),
                t
               );
#   endif
}

Err_t btForkTransitStart(OA_DEBUG_ARGUMENT, TFJiterate_t * in_FJiterate,
                         GlobalData_t * in_globals, ocrGuid_t * in_whenDone,
                         LedgerReduc2ary_t * o_ledgerR2ary)
{
    Err_t err=0;
    while(!err){
        if(in_FJiterate->low <= 0) {
            PRINTF("ERROR: btForkTransitStart: in_FJiterate->low is smaller or equal to zero.\n");
            err = __LINE__; IFEB;
        }
        if(in_FJiterate->low > in_globals->overall_mpi_count){
            PRINTF("ERROR: btForkTransitStart: in_FJiterate->low is larger than the overall_mpi_count.\n");
            err = __LINE__; IFEB;
        }

        unsigned long edtCount = in_globals->overall_mpi_count;
        unsigned long edtID = in_FJiterate->low -1; //That should gives  0<= x < in_globals->overall_mpi_count

        double x = edtID + 1;  //That should gives  1<= x <= in_globals->overall_mpi_count
                               //This is the starting point for the contribution of each EDt to the summation.
        R2aryValue_t val;
        init_R2aryValue(&val);
        val.x = x;

        err = create_reduction2ary_ledger(OA_DEBUG_INVARS, o_ledgerR2ary, edtCount, edtID,
                                  & in_globals->labelEDTmap,
                                  in_whenDone,
                                  & val
                                  ); IFEB;

        print_reduction2ary_ledger(OA_DEBUG_INVARS, o_ledgerR2ary, "btForkTransitStart");

        break;  //  while(!err)
    }
    return err;
}

Err_t R2ary_ValidateValue_in_btForkTransit_stop(OA_DEBUG_ARGUMENT,
                                                GlobalData_t * in_globals,
                                                R2aryValue_t * in_r2aryValue,
                                                TFJiterate_t * in_FJiterate)
{
    Err_t err=0;
    while(!err){

        unsigned long N = in_globals->overall_mpi_count;
        unsigned long edtID = in_FJiterate->low -1; //See btForkTransitStart for details.

        const double one = 1;
        const double two = 2;
        double ref = (N + one) * N /two;
        const double tiny = 1e-14;

        double diff = in_r2aryValue->x - ref;
        if(diff < 0) diff = -diff;

        if( diff < tiny ){
#           ifdef REDUCTION2ARY_VERBOSE
                PRINTF("TESTIO> R2ary_ValidateValue_in_btForkTransit_stop: Success.\n");
                PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" REDUC2ARY CHECK VALUE: edtCount=%u edtID=%u Value is OK.\n",
                       in_edtType, GUIDA(in_thisEDT),
                       N, edtID
                       );
#           endif
        } else {
            err = __LINE__;
#           ifdef REDUCTION2ARY_VERBOSE
                PRINTF("TESTIO> R2ary_ValidateValue_in_btForkTransit_stop: ERROR: Results do not match.\n");
                PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" REDUC2ARY CHECK VALUE: edtCount=%u edtID=%u ERROR ref=%e  val=%e\n",
                       in_edtType, GUIDA(in_thisEDT),
                       N, edtID, ref, in_r2aryValue->x
                       );
#           endif
            break;
        }

        break;  //  while(!err)
    }
    return err;
}

Err_t setup_reduction2ary(OA_DEBUG_ARGUMENT,
                          LedgerReduc2ary_t * o_ledgerR2ary,
                          R2aryWhenDone_t * o_whenDone,
                          LedgerReduc2ary_t * in_ledgerR2ary,
                          ocrGuid_t * in_whenDone,
                          ocrEdt_t in_funcPtr,
                          ocrGuid_t * io_g4nextEdt,
                          unsigned int * io_dataSlot2use
                          )
{
    Err_t err=0;
    while(!err){
        print_reduction2ary_ledger(OA_DEBUG_INVARS, in_ledgerR2ary, "setup_reduction2ary entry");
        GUID_ASSIGN_VALUE(o_whenDone->wdone[0], in_ledgerR2ary->whenDone);
        GUID_ASSIGN_VALUE(o_whenDone->wdone[1], NULL_GUID);
        o_whenDone->stopCriteria = NOTA_reduc2ary_leafID;

        if(in_ledgerR2ary->edtID >= in_ledgerR2ary->edtCount){
            PRINTF("ERROR: setup_reduction2ary: in_ledgerR2ary->edtID >= in_ledgerR2ary->edtCount\n");
            err=__LINE__; IFEB;
        }
        copy_reduction2ary_ledger(in_ledgerR2ary, o_ledgerR2ary);

        GUID_ASSIGN_VALUE(o_ledgerR2ary->whenDone, *in_whenDone);

        unsigned long parentIndex = reduc2ary_find_parent(in_ledgerR2ary->edtCount, in_ledgerR2ary->leafID);

#       ifdef REDUCTION2ARY_VERBOSE
            PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" REDUC2ARY SETUP parentDex=%lu leaf=%lu\n",
                   in_edtType, GUIDA(in_thisEDT),
                   parentIndex, o_ledgerR2ary->leafID
                   );
#       endif

        err = ocrXIndexedEdtCreate(in_funcPtr, 0,NULL,2, EDT_PROP_NONE, NULL_HINT,
                                   parentIndex, &in_ledgerR2ary->labelEDTmap,
                                   io_g4nextEdt, NULL);
        if(err){
            PRINTF("ERROR: setup_reduction2ary: Creation of parent EDT (=%lu) failed.\n", parentIndex);
            IFEB;
        }

        *io_dataSlot2use = reduc2ary_select_slot2use(in_ledgerR2ary->leafID, parentIndex);

#       ifdef REDUCTION2ARY_VERBOSE
            PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" REDUC2ARY SETUP parentDex=%lu dslot=%u parentGuid="GUIDF " leaf=%lu\n",
                   in_edtType, GUIDA(in_thisEDT),
                   parentIndex, *io_dataSlot2use,
                   GUIDA(*io_g4nextEdt),
                   o_ledgerR2ary->leafID
                   );
#       endif
        print_reduction2ary_ledger(OA_DEBUG_INVARS, o_ledgerR2ary, "setup_reduction2ary exit");

        break;  //  while(!err)
    }
    return err;
}

int conditionR2aryJoinIF(OA_DEBUG_ARGUMENT,
                         LedgerReduc2ary_t * in_ledgerA,
                         LedgerReduc2ary_t * in_ledgerB) //Return 1 upon success; zero otherwise.
{
    unsigned long parentDex = reduc2ary_find_parent(in_ledgerA->edtCount, in_ledgerA->leafID);
    int x = 1;
    if(parentDex == ROOT_reduc2ary_parentIndex){
        x = 0;
    }

#   ifdef REDUCTION2ARY_VERBOSE
        print_reduction2ary_ledger(OA_DEBUG_INVARS, in_ledgerA, "A-conditionR2aryJoinIF");
        print_reduction2ary_ledger(OA_DEBUG_INVARS, in_ledgerB, "B-conditionR2aryJoinIF");
        PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" conditionR2aryJoinIF parentDex=%lu x=%d\n",
                in_edtType, GUIDA(in_thisEDT), parentDex, x);
#   endif
    return x;
}

Err_t R2aryJoinIF_fcn(OA_DEBUG_ARGUMENT,
                      ocrGuid_t * in_whereToNext,
                      LedgerReduc2ary_t * in_ledgerA,
                      LedgerReduc2ary_t * in_ledgerB,
                      LedgerReduc2ary_t * o_ledger,
                      R2aryWhenDone_t * o_r2aWdone,
                      ocrEdt_t in_funcPtr,
                      ocrGuid_t * io_g4nextEdt,
                      unsigned int * io_dataSlot2use
                      )
{
    Err_t err=0;
    while(!err){
        print_reduction2ary_ledger(OA_DEBUG_INVARS, in_ledgerA, "in_ledgerA R2aryJoinIF");
        print_reduction2ary_ledger(OA_DEBUG_INVARS, in_ledgerB, "in_ledgerB R2aryJoinIF");

        if(in_ledgerA->edtCount != in_ledgerB->edtCount){
            PRINTF("ERROR: TaskTYPE=%d TaskID="GUIDF" R2aryJoinIF : edtCount differs.\n", in_edtType, in_thisEDT);
            err = __LINE__; IFEB;
        }
        unsigned long parentA = reduc2ary_find_parent(in_ledgerA->edtCount, in_ledgerA->leafID);
        unsigned long parentB = reduc2ary_find_parent(in_ledgerB->edtCount, in_ledgerB->leafID);
        if(parentA != parentB){
            PRINTF("ERROR: TaskTYPE=%d TaskID="GUIDF" R2aryJoinIF : parents differs: %lu, %lu\n",
                   in_edtType, in_thisEDT, parentA, parentB);
            err = __LINE__; IFEB;
        }

        copy_reduction2ary_ledger(in_ledgerA, o_ledger);

        init_R2aryValue(&o_ledger->value);
        add_R2aryValue(&in_ledgerA->value, &o_ledger->value);
        add_R2aryValue(&in_ledgerB->value, &o_ledger->value);

        GUID_ASSIGN_VALUE(o_ledger->whenDone, *in_whereToNext);

        GUID_ASSIGN_VALUE(o_r2aWdone->wdone[0], in_ledgerA->whenDone);
        GUID_ASSIGN_VALUE(o_r2aWdone->wdone[1], in_ledgerB->whenDone);

        unsigned long old_leaf = o_ledger->leafID;
        o_ledger->leafID = parentA;
        unsigned long new_parent = reduc2ary_find_parent(in_ledgerB->edtCount, parentA);

        unsigned long starting_leaf = reduc2ary_calculate_leafID(in_ledgerA->edtCount, in_ledgerA->edtID);
        if(starting_leaf == in_ledgerA->leafID){
            //This should happen only once (when leafID was originally calculated by setup_reduction2ary).
            //Setting to zero will tell BtJoinIFTHEN when to go to its ELSE clause.
            o_r2aWdone->stopCriteria = 0;
        } else {
            //After that, we just used it as a place holder.
            //Note that setting o_r2aWdone->stopCriteria to zero when leafID is zero
            //should not affect anything because by then we will be at the transition point.
            o_r2aWdone->stopCriteria = in_ledgerA->leafID;
        }

//        *io_dataSlot2use = reduc2ary_select_slot2use(old_leaf, parentA);
        *io_dataSlot2use = reduc2ary_select_slot2use(o_ledger->leafID, new_parent);

        err = ocrXIndexedEdtCreate(in_funcPtr, 0,NULL,2, EDT_PROP_NONE, NULL_HINT,
//                                   parentA, &o_ledger->labelEDTmap,
                                   new_parent, &o_ledger->labelEDTmap,
                                   io_g4nextEdt, NULL);
        if(err){
            PRINTF("ERROR: R2aryJoinIF_fcn: Creation of parent EDT (=%lu) failed.\n", parentA);
            IFEB;
        }

#       ifdef REDUCTION2ARY_VERBOSE
            PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" REDUC2ARY R2aryJoinIF parentDex=%lu dslot=%u parentGuid="GUIDF " old_leaf=%lu\n",
                   in_edtType, GUIDA(in_thisEDT),
                   parentA, *io_dataSlot2use,
                   GUIDA(*io_g4nextEdt),
                   old_leaf
                   );
#       endif
        print_reduction2ary_ledger(OA_DEBUG_INVARS, o_ledger, "o_ledger R2aryJoinIF");
        print_R2aryWhenDone(OA_DEBUG_INVARS, o_r2aWdone, "o_r2aWdone R2aryJoinIF");

        break;  //  while(!err)
    }
    return err;
}

Err_t R2aryJoinELSE_fcn(OA_DEBUG_ARGUMENT,
                        LedgerReduc2ary_t * in_ledgerA,
                        LedgerReduc2ary_t * in_ledgerB,
                        R2aryValue_t * o_value,
                        R2aryWhenDone_t * o_r2aWdone)
{
    Err_t err=0;
    while(!err){
        print_reduction2ary_ledger(OA_DEBUG_INVARS, in_ledgerA, "in_ledgerA R2aryJoinELSE");
        print_reduction2ary_ledger(OA_DEBUG_INVARS, in_ledgerB, "in_ledgerB R2aryJoinELSE");

        if(in_ledgerA->edtCount != in_ledgerB->edtCount){
            PRINTF("ERROR: TaskTYPE=%d TaskID="GUIDF" R2aryJoinIF : edtCount differs.\n",
                   in_edtType, GUIDA(in_thisEDT));
            err = __LINE__; IFEB;
        }
        unsigned long parentA = reduc2ary_find_parent(in_ledgerA->edtCount, in_ledgerA->leafID);
        unsigned long parentB = reduc2ary_find_parent(in_ledgerB->edtCount, in_ledgerB->leafID);
        if(parentA != parentB){
            PRINTF("ERROR: TaskTYPE=%d TaskID="GUIDF" R2aryJoinIF : parents differs: %lu, %lu\n",
                   in_edtType, GUIDA(in_thisEDT), parentA, parentB);
            err = __LINE__; IFEB;
        }

        if(parentA != ROOT_reduc2ary_parentIndex){
            PRINTF("ERROR: TaskTYPE=%d TaskID="GUIDF" R2aryJoinIF : Bad entry into ELSE EDT: parent=%lu\n",
                   in_edtType, GUIDA(in_thisEDT), parentA);
            err = __LINE__; IFEB;
        }

        init_R2aryValue(o_value);
        add_R2aryValue(&in_ledgerA->value, o_value);
        add_R2aryValue(&in_ledgerB->value, o_value);

        GUID_ASSIGN_VALUE(o_r2aWdone->wdone[0], in_ledgerA->whenDone);
        GUID_ASSIGN_VALUE(o_r2aWdone->wdone[1], in_ledgerB->whenDone);

        unsigned long leaf = reduc2ary_calculate_leafID(in_ledgerA->edtCount, in_ledgerA->edtID);

        if(leaf == in_ledgerA->leafID){
            //This should happen only once (when leafID was originally calculated by setup_reduction2ary).
            o_r2aWdone->stopCriteria = 0;
        } else {
            //After that, we just used it as a place holder.
            //Note that setting o_r2aWdone->stopCriteria to zero when leafID is zero
            //should not affect anything because by then we will be at the transition point.
            o_r2aWdone->stopCriteria = in_ledgerA->leafID;
        }

        print_R2aryValue(OA_DEBUG_INVARS, o_value, "o_value R2aryJoinELSE");
        print_R2aryWhenDone(OA_DEBUG_INVARS, o_r2aWdone, "o_r2aWdone R2aryJoinELSE");

        break;  //  while(!err)
    }
    return err;
}

int conditionR2aryForkIF(OA_DEBUG_ARGUMENT,
                         R2aryWhenDone_t * in_r2aWdone)//Return 1 upon success; zero otherwise.
{
    int x = 1;
    if(in_r2aWdone->stopCriteria == 0){
        //Time to stop joining and go to conclusion
        x = 0;
    }
#   ifdef REDUCTION2ARY_VERBOSE
        print_R2aryWhenDone(OA_DEBUG_INVARS, in_r2aWdone, "o_r2aWdone conditionR2aryForkIF");
        PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" conditionR2aryForkIF x=%d\n",
                in_edtType, GUIDA(in_thisEDT), x);
#   endif
    return x;
}

Err_t R2aryForkIF_FOR_body(OA_DEBUG_ARGUMENT,
                           int in_index, R2aryWhenDone_t * in_r2aWdone,
                           R2aryValue_t * in_value, R2aryValue_t * o_value,
                           ocrGuid_t * o_whereTonext)
{
    Err_t err=0;
    while(!err){
#   ifdef REDUCTION2ARY_VERBOSE
        PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" R2aryForkIF_FOR_body index=%d\n",
                in_edtType, GUIDA(in_thisEDT), in_index);
#   endif
        print_R2aryWhenDone(OA_DEBUG_INVARS, in_r2aWdone, "in_r2aWdone R2aryForkIF_FOR_body");
        print_R2aryValue(OA_DEBUG_INVARS, in_value, "in_value R2aryForkIF_FOR_body");

        GUID_ASSIGN_VALUE(*o_whereTonext, in_r2aWdone->wdone[in_index]);
        copy_R2aryValue(in_value, o_value);

        print_R2aryValue(OA_DEBUG_INVARS, o_value, "o_value R2aryForkIF_FOR_body");

        break;  //  while(!err)
    }
    return err;
}

Err_t R2aryForkELSE_FOR_body(OA_DEBUG_ARGUMENT,
                             int in_index, R2aryWhenDone_t * in_r2aWdone,
                             R2aryValue_t * in_value, R2aryValue_t * o_value,
                             ocrGuid_t * o_whereTonext)
{
    Err_t err=0;
    while(!err){
#   ifdef REDUCTION2ARY_VERBOSE
        PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" R2aryForkELSE_FOR_body index=%d\n",
                in_edtType, GUIDA(in_thisEDT), in_index);
#   endif
        print_R2aryWhenDone(OA_DEBUG_INVARS, in_r2aWdone, "in_r2aWdone R2aryForkELSE_FOR_body");
        print_R2aryValue(OA_DEBUG_INVARS, in_value, "in_value R2aryForkELSE_FOR_body");

        GUID_ASSIGN_VALUE(*o_whereTonext, in_r2aWdone->wdone[in_index]);
        copy_R2aryValue(in_value, o_value);

        print_R2aryValue(OA_DEBUG_INVARS, o_value, "o_value R2aryForkELSE_FOR_body");

        break;  //  while(!err)
    }
    return err;
}

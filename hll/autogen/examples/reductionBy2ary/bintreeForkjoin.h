#ifndef INCLUSION_BINTREE_FORKJOIN_H
#define INCLUSION_BINTREE_FORKJOIN_H

#include "ocr.h" //ocrGuid_t, PRINTF
#include "../../tools/app_ocr_err_util.h"
#include "global_data.h"

typedef unsigned long BTindex_t;
#define NOTA_BTindex ((unsigned long)(-1))
#define ROOT_BTindex 0
void get_children_indices(BTindex_t in_parent, BTindex_t * o_child1, BTindex_t * o_child2);
BTindex_t get_parent_index(BTindex_t in_index);
unsigned long get_level(BTindex_t in_index);

typedef unsigned long Value_t;
typedef Value_t (*testFunctPtr_t)(Value_t one, Value_t two);
typedef void (*testFunctPtr_zero_t)(Value_t * one);

Value_t add(Value_t one, Value_t two);
void    add_zero(Value_t * io); //This is the zero value for that operation,
                                // where add_zero(&y); then x == (x+y) is true,
                                // assuming that + is the operation of interest.

typedef struct TFJiterate
{
    BTindex_t btindex;

    //[low,hi] are the indices for the interval of interest
    unsigned int low;
    unsigned int hi;

    testFunctPtr_t funcp;
    testFunctPtr_zero_t fzerop;

    ocrGuid_t whereToGoWhenFalse;
} TFJiterate_t;

typedef struct Work
{
    BTindex_t btindex;
    testFunctPtr_t funcp;
    Value_t result;
} Work_t;

void print_iterate(int in_edtType, ocrGuid_t in_thisEDT, TFJiterate_t * it, const char * in_text);
void copy_iterate(TFJiterate_t * in, TFJiterate_t * out);

void print_work(int in_edtType, ocrGuid_t in_thisEDT, Work_t w, const char * in_text);
void copy_work(Work_t w, Work_t * out);

Err_t setupBtForkJoin(int in_edtType, ocrGuid_t in_thisEDT,
                      TFJiterate_t * io_iterate,
                      ocrGuid_t in_whereToGoWhenFalse,
                      testFunctPtr_t in_funcp,
                      testFunctPtr_zero_t in_func_zerop,
                      GlobalData_t * in_globals,
                      unsigned long * o_refvalue
                      );

int conditionBtFork(int in_edtType, ocrGuid_t in_thisEDT, TFJiterate_t * io_iterate); //Return 1 upon success; zero otherwise.

Err_t btForkThen(int in_edtType, ocrGuid_t in_thisEDT,
                 TFJiterate_t * in_iterate,
                 ocrGuid_t in_whenDone,
                 TFJiterate_t * o_iterate,
                 ocrGuid_t * o_gDone);

Err_t btForkFOR(int in_edtType, ocrGuid_t in_thisEDT, int in_foliationIndex, TFJiterate_t * in_iterate, TFJiterate_t * o_iterate);

Err_t btForkElse(int in_edtType, ocrGuid_t in_thisEDT,
                 TFJiterate_t * in_iterate, ocrGuid_t in_whenDone,
                 TFJiterate_t * o_iterate, ocrGuid_t * o_gDone);

Err_t transitionBTFork(int in_edtType, ocrGuid_t in_thisEDT,
                       TFJiterate_t * in_iterate, Work_t * o_work, Work_t * o_work2);

unsigned int btCalculateJoinIndex(BTindex_t in_index);
Err_t joinOperationIFTHEN(int in_edtType, ocrGuid_t in_thisEDT, Work_t in_left, Work_t in_right, Work_t * o_work);
Err_t joinOperationELSE(int in_edtType, ocrGuid_t in_thisEDT, Work_t in_left, Work_t in_right, Work_t * o_work);

Err_t concludeBtForkJoin(int in_edtType, ocrGuid_t in_thisEDT, unsigned long in_reference, unsigned long in_result);

#endif //INCLUSION_BINTREE_FORKJOIN_H

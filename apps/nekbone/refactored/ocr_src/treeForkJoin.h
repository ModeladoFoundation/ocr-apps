#ifndef INCLUSION_BINTREE_FORKJOIN_H
#define INCLUSION_BINTREE_FORKJOIN_H

#include "ocr.h" //ocrGuid_t, PRINTF
#include "app_ocr_err_util.h"
#include "spmd_global_data.h"

typedef unsigned long BTindex_t;
#define NOTA_BTindex ((unsigned long)(-1))
#define ROOT_BTindex 0
void get_children_indices(BTindex_t in_parent, BTindex_t * o_child1, BTindex_t * o_child2);
BTindex_t get_parent_index(BTindex_t in_index);
unsigned long get_level(BTindex_t in_index);

typedef unsigned long TChecksum_t;
TChecksum_t treeFJ_checksum_add(TChecksum_t one, TChecksum_t two);
void  treeFJ_checksum_add_zero(TChecksum_t * io); //This is the zero value for that operation,
                                         // where treeFJ_checksum_add_zero(&y); then x == (x+y) is true,
                                         // assuming that + is the operation of interest.
typedef struct TFJiterate
{
    BTindex_t btindex;

    //[low,hi] are the indices for the interval of interest
    unsigned int low;
    unsigned int hi;

    ocrGuid_t whereToGoWhenFalse;
} TFJiterate_t;

typedef struct TChecksum_work
{
    BTindex_t btindex;
    TChecksum_t result;
} TChecksum_work_t;

void print_iterate(int in_edtType, ocrGuid_t in_thisEDT, TFJiterate_t * it,
                   const char * in_text);
void copy_TFJiterate(TFJiterate_t * in, TFJiterate_t * out);

void print_TChecksum_work(int in_edtType, ocrGuid_t in_thisEDT,
                          TChecksum_work_t w, const char * in_text);
void copy_TChecksum_work(TChecksum_work_t w, TChecksum_work_t * out);

Err_t setupBtForkJoin(int in_edtType, ocrGuid_t in_thisEDT,
                      TFJiterate_t * io_iterate,
                      ocrGuid_t in_whereToGoWhenFalse,
                      SPMD_GlobalData_t * in_globalData,
                      unsigned long * o_refvalue
                      );

int conditionBtFork(int in_edtType, ocrGuid_t in_thisEDT, TFJiterate_t * io_iterate); //Return 1 upon success; zero otherwise.

Err_t btForkThen(int in_edtType, ocrGuid_t in_thisEDT,
                 TFJiterate_t * in_iterate,
                 ocrGuid_t in_whenDone,
                 TFJiterate_t * o_iterate,
                 ocrGuid_t * o_gDone);

Err_t btForkFOR(int in_edtType, ocrGuid_t in_thisEDT, int in_foliationIndex,
                TFJiterate_t * in_iterate, TFJiterate_t * o_iterate);

Err_t btForkElse(int in_edtType, ocrGuid_t in_thisEDT,
                 TFJiterate_t * in_iterate, ocrGuid_t in_whenDone,
                 TFJiterate_t * o_iterate, ocrGuid_t * o_gDone);

Err_t transitionBTFork(int in_edtType, ocrGuid_t in_thisEDT,
                       TFJiterate_t * in_iterate, TChecksum_work_t * o_TChecksum_work,
                       TChecksum_work_t * o_TChecksum_work2);

unsigned int btCalculateJoinIndex(BTindex_t in_index);
Err_t joinOperationIFTHEN(int in_edtType, ocrGuid_t in_thisEDT,
                          TChecksum_work_t in_left, TChecksum_work_t in_right,
                          TChecksum_work_t * o_TChecksum_work);
Err_t joinOperationELSE(int in_edtType, ocrGuid_t in_thisEDT,
                        TChecksum_work_t in_left, TChecksum_work_t in_right,
                        TChecksum_work_t * o_TChecksum_work);

Err_t concludeBtForkJoin(int in_edtType, ocrGuid_t in_thisEDT,
                         unsigned long in_reference, unsigned long in_result);

#endif //INCLUSION_BINTREE_FORKJOIN_H

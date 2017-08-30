#ifndef INCLUSION_TAILRECURSION_H
#define INCLUSION_TAILRECURSION_H

#include "ocr.h" //ocrGuid_t, ocrPrintf
#include "app_ocr_err_util.h"
#include "spmd_global_data.h"

typedef struct TailRecurIterate
{
    ocrGuid_t whereToGoWhenDone;
    long begin;
    long end; //One passed the last iterate
    long increment;
    long current;  // This is the current iteration value
} TailRecurIterate_t;

Err_t tailRecurInitialize(TailRecurIterate_t * io_iterate,
                          ocrGuid_t in_whereToGoWhenDone,
                          SPMD_GlobalData_t * in_globald);
int tailRecurCondition(TailRecurIterate_t * io_iterate); //Return 1 upon success; zero otherwise.
Err_t tailRecurIfThenClause(const TailRecurIterate_t * in_iterate);
Err_t tailRecurElseClause(const TailRecurIterate_t * in_iterate);
Err_t tailRecurConclude();

#endif

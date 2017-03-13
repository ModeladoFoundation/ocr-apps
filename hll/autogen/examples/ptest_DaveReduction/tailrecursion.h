#ifndef INCLUSION_TAILRECURSION_H
#define INCLUSION_TAILRECURSION_H

#include "ocr.h" //ocrGuid_t, PRINTF
#include "../../tools/app_ocr_err_util.h"

typedef struct TailRecurIterate
{
    ocrGuid_t whereToGoWhenDone;
    long begin;
    long end; //One passed the last iterate
    long increment;
    long current;  // This is the current iteration value
} TailRecurIterate_t;

Err_t tailRecurInitialize(long in_iteration_count, TailRecurIterate_t * io_iterate,
                          ocrGuid_t in_whereToGoWhenDone);

int tailRecurCondition(TailRecurIterate_t * io_iterate); //Return 1 upon success; zero otherwise.

Err_t tailRecurIfThenClause(const TailRecurIterate_t * in_iterate);
Err_t tailRecurElseClause(const TailRecurIterate_t * in_iterate);

#endif

#ifndef INCLUSION_TAILRECURSION_H
#define INCLUSION_TAILRECURSION_H

#include "ocr.h" //ocrGuid_t, PRINTF
#include "../../tools/app_ocr_err_util.h"

typedef struct Iterate
{
    ocrGuid_t whereToGoWhenFalse;
    long begin;
    long end; //One passed the last iterate
    long increment;
    long current;  // This is the current iteration value
} Iterate_t;

Err_t initializeIterate(Iterate_t * io_iterate, ocrGuid_t in_whereToGoWhenFalse);
int condition(Iterate_t * io_iterate); //Return 1 upon success; zero otherwise.
Err_t trueClause(const Iterate_t * in_iterate);
Err_t falseClause(const Iterate_t * in_iterate);
Err_t concludeIteration(int * in_work);

Err_t initializeWork(u64 in_workLength, int * io_work);
Err_t trueClauseWithWork(const Iterate_t * in_iterate, int * in_work);
Err_t falseClauseWithWork(const Iterate_t * in_iterate, int * in_work);

#endif

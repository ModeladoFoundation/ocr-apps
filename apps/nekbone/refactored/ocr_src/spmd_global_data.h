#ifndef SPMD_GLOBAL_DATA_H
#define SPMD_GLOBAL_DATA_H

#include "app_ocr_err_util.h"

typedef struct SPMD_GlobalData
{
    unsigned int overall_mpi_count;
    unsigned int iterationCountOnEachRank;

} SPMD_GlobalData_t;

void init_SPMDglobals(SPMD_GlobalData_t * io_gdata);
void copy_SPMDglobals(SPMD_GlobalData_t * in_source, SPMD_GlobalData_t * io_target);
void print_SPMDglobals(SPMD_GlobalData_t * in_gdata, const char * in_text);

#endif // SPMD_GLOBAL_DATA_H

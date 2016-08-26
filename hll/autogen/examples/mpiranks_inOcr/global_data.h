#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include "../../tools/app_ocr_err_util.h"

typedef struct GlobalData
{
    unsigned int overall_mpi_count;
    unsigned int iterationCountOnEachRank;

} GlobalData_t;

void init_globalData(GlobalData_t * io_gdata);
void copy_globalData(GlobalData_t * in_source, GlobalData_t * io_target);
void print_globalData(GlobalData_t * in_gdata, const char * in_text);

#endif // GLOBAL_DATA

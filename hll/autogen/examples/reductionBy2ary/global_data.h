#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include <ocr.h> //ocrGuid_t
#include "../../tools/app_ocr_err_util.h"
#include "../../tools/app_ocr_debug_util.h"

typedef struct GlobalData
{
    unsigned int overall_mpi_count;
    unsigned int iterationCountOnEachRank;

    ocrGuid_t labelEDTmap;  //This points to a range of labeled EDTs.
                            //There are this->overall_mpi_count EDT guids in that range.

} GlobalData_t;

Err_t create_globalData(OA_DEBUG_ARGUMENT, GlobalData_t * io_gdata);
Err_t destroy_globalData(OA_DEBUG_ARGUMENT, GlobalData_t * io_gdata);
Err_t clear_globalData(OA_DEBUG_ARGUMENT, GlobalData_t * io_gdata);  //This does no release of resources.
void copy_globalData(OA_DEBUG_ARGUMENT, GlobalData_t * in_source, GlobalData_t * io_target);
void print_globalData(OA_DEBUG_ARGUMENT, GlobalData_t * in_gdata, const char * in_text);

#endif // GLOBAL_DATA

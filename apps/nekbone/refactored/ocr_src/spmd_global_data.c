#ifndef SPMD_GLOBAL_DATA_H
#include "spmd_global_data.h"
#endif

//Removed for runs on TG> #include <string.h> //memset. memcpy

//#define GLOBAL_DATA_VERBOSE //Define this macro in order to get many PRINTF activated.
                            //Otherwise, to keep silent, undefine the macro.

void init_SPMDglobals(SPMD_GlobalData_t * io_gdata)
{
    //Removed for runs on TG> memset(io_gdata, 0, sizeof(SPMD_GlobalData_t));

    //See setup_SPMD_using_NEKOstatics() to see how these values will be overwritten.
    io_gdata->overall_mpi_count = 3;
    io_gdata->iterationCountOnEachRank = 2;

    print_SPMDglobals(io_gdata, "INIT");
}
void copy_SPMDglobals(SPMD_GlobalData_t * in_source, SPMD_GlobalData_t * io_target)
{
    print_SPMDglobals(in_source, "COPY SOURCE");
    unsigned long sz = sizeof(SPMD_GlobalData_t);
    //Removed for runs on TG> memcpy(io_target, in_source, sz);
    io_target->iterationCountOnEachRank = in_source->iterationCountOnEachRank;
    io_target->overall_mpi_count        = in_source->overall_mpi_count;
    print_SPMDglobals(io_target, "COPY TARGET");
}

void print_SPMDglobals(SPMD_GlobalData_t * in_gdata, const char * in_text)
{
#   ifdef GLOBAL_DATA_VERBOSE
        const char * defaultt = "";
        const char * t = in_text;
        if(!t){
            t = defaultt;
        }
        PRINTF("TESTIO> SPMD GLOBAL DATA overall_mpi_count=%u iterationCountOnEachRank=%u %s\n",
                in_gdata->overall_mpi_count,
                in_gdata->iterationCountOnEachRank,
                t
               );
#   endif
}

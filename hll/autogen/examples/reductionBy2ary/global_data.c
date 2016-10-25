#ifndef GLOBAL_DATA_H
#include "global_data.h"
#endif

#include "reduction2ary_indices.h"

#define ENABLE_EXTENSION_LABELING  // For labeled EDTs
#include "extensions/ocr-labeling.h"  // For labeled EDTs

#define XMEMSET(SRC, CHARC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)SRC+xmIT)=CHARC;}
#define XMEMCPY(DEST, SRC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)DEST+xmIT)=*((char*)SRC+xmIT);}

#define GLOBAL_DATA_VERBOSE //Define this macro in order to get many PRINTF activated.
                            //Otherwise, to keep silent, undefine the macro.

Err_t create_globalData(OA_DEBUG_ARGUMENT, GlobalData_t * io_gdata)
{
    GlobalData_t * GD = io_gdata;
    Err_t err=0;
    while(!err){
        GUID_ASSIGN_VALUE(GD->labelEDTmap, NULL_GUID); //No default constructor.  So this will have to do.

        io_gdata->iterationCountOnEachRank = 999999; //Not in use.

        unsigned long mpi_count_needed = 4;
        unsigned long topminleaf = reduc2ary_calculate_leafID(mpi_count_needed, 0);
        if( topminleaf == 0 ) {
            //This means that no reduction is required since there is only 1 rank.
            PRINTF("ERROR: ocrGuidRangeCreate: A range size of at least one is needed:%lu\n", topminleaf);
            err = __LINE__;
            break;
        }

        io_gdata->overall_mpi_count = mpi_count_needed; //This is an exact fit only for a full binary tree.

        err = ocrGuidRangeCreate( &GD->labelEDTmap, topminleaf, GUID_USER_EDT); IFEB;

        print_globalData(OA_DEBUG_INVARS, GD, "CREATE");
        break;  //  while(!err)
    }
    return err;
}

Err_t destroy_globalData(OA_DEBUG_ARGUMENT, GlobalData_t * io_gdata)
{
    GlobalData_t * GD = io_gdata;
    Err_t err=0;
    while(!err){
        print_globalData(OA_DEBUG_INVARS, GD, "DESTROY");
        if( ! ocrGuidIsEq(GD->labelEDTmap, NULL_GUID) ){
            err = ocrGuidMapDestroy(GD->labelEDTmap);
            GUID_ASSIGN_VALUE(GD->labelEDTmap, NULL_GUID);
        }

        XMEMSET(io_gdata, 0, sizeof(GlobalData_t));
        GUID_ASSIGN_VALUE(GD->labelEDTmap, NULL_GUID); //In case NULL_GUID != 0x0

        break;  //  while(!err)
    }
    return err;
}

Err_t clear_globalData(OA_DEBUG_ARGUMENT, GlobalData_t * io_gdata)
{
    GlobalData_t * GD = io_gdata;
    Err_t err=0;
    while(!err){
        print_globalData(OA_DEBUG_INVARS, GD, "CLEAR");
        XMEMSET(io_gdata, 0, sizeof(GlobalData_t));
        GUID_ASSIGN_VALUE(GD->labelEDTmap, NULL_GUID);
        break;  //  while(!err)
    }
    return err;
}

void copy_globalData(OA_DEBUG_ARGUMENT, GlobalData_t * in_source, GlobalData_t * io_target)
{
    print_globalData(OA_DEBUG_INVARS, in_source, "COPY SOURCE");
    unsigned long sz = sizeof(GlobalData_t);
    XMEMCPY(io_target, in_source, sz);
    print_globalData(OA_DEBUG_INVARS, io_target, "COPY TARGET");
}

void print_globalData(OA_DEBUG_ARGUMENT, GlobalData_t * in_gdata, const char * in_text)
{
#   ifdef GLOBAL_DATA_VERBOSE
        const char * defaultt = "";
        const char * t = in_text;
        if(!t){
            t = defaultt;
        }

        ocrGuid_t emap = NULL_GUID;
        if( ! ocrGuidIsNull(in_thisEDT)){
            GUID_ASSIGN_VALUE(emap, in_gdata->labelEDTmap);
        }

        PRINTF("TESTIO> TaskTYPE=%d TaskID="GUIDF" GLOBAL DATA overall_mpi_count=%u iterationCountOnEachRank=%u EDTmap="GUIDF" %s\n",
                in_edtType, GUIDA(in_thisEDT),
                in_gdata->overall_mpi_count,
                in_gdata->iterationCountOnEachRank,
                GUIDA(emap),
                t
               );
#   endif
}

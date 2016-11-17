#ifndef NEKBONE_REDUCTION_H
#include "neko_reduction.h"
#endif

#define ENABLE_EXTENSION_LABELING  // For labeled GUIDs
#include "extensions/ocr-labeling.h"  // For labeled GUIDs

#include "reduction.h"

#define XMEMSET(SRC, CHARC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)SRC+xmIT)=CHARC;}
#define XMEMCPY(DEST, SRC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)DEST+xmIT)=*((char*)SRC+xmIT);}

Err_t copy_Reduct_private(reductionPrivate_t * in_from, reductionPrivate_t * o_target)
{
    if( !in_from || !o_target) return __LINE__;
    XMEMCPY(o_target, in_from, sizeof(reductionPrivate_t));
    return 0;
}

Err_t init_Reduct_shared(unsigned long in_nrank,unsigned long in_ndata, Reduct_shared_t * io)
{
    XMEMSET(io,0,sizeof(Reduct_shared_t));
    io->nrank = in_nrank;
    io->ndata = in_ndata;
    io->reductionRangeGUID = NULL_GUID;
    return 0;
}
Err_t clear_Reduct_shared(Reduct_shared_t * io)
{
    XMEMSET(io,0,sizeof(Reduct_shared_t));
    return 0;
}
Err_t destroy_Reduct_shared(Reduct_shared_t * io)
{
    Err_t err=0;
    if( ! IS_GUID_NULL(io->reductionRangeGUID)){
        err = ocrGuidMapDestroy( io->reductionRangeGUID);
    }
    XMEMSET(io,0,sizeof(Reduct_shared_t));
    return err;
}
Err_t copy_Reduct_shared(Reduct_shared_t * in_from, Reduct_shared_t * o_target)
{
    if( !in_from || !o_target) return __LINE__;
    XMEMCPY(o_target, in_from, sizeof(Reduct_shared_t));
    return 0;
}
void  print_Reduct_shared(Reduct_shared_t * in)
{
    PRINTF("Reduct_shared>[myrank,nrank,ndata,reductRguid]= %lu %lu "GUIDF"\n",
           in->nrank, in->ndata, GUIDA(in->reductionRangeGUID)
           );
}

Err_t NEKO_mainEdt_reduction(unsigned long in_nrank,unsigned long in_ndata,
                           Reduct_shared_t * io_sharedRef, Reduct_shared_t * io_shared)
{
    Err_t err = 0;
    while(!err){
        err = init_Reduct_shared(in_nrank, in_ndata, io_sharedRef); IFEB;
        err = init_Reduct_shared(in_nrank, in_ndata, io_shared); IFEB;

        ocrGuid_t reductionRangeGUID = NULL_GUID;
        err = ocrGuidRangeCreate(&reductionRangeGUID, in_nrank, GUID_USER_EVENT_STICKY); IFEB;

        GUID_ASSIGN_VALUE(io_sharedRef->reductionRangeGUID, reductionRangeGUID);
        GUID_ASSIGN_VALUE(io_shared->reductionRangeGUID, reductionRangeGUID);
        break;
    }
    return err;
}

Err_t NEKO_finalEdt_reduction(Reduct_shared_t * io_sharedRef)
{
    Err_t err = 0;
    while(!err){
        err = destroy_Reduct_shared(io_sharedRef); IFEB;
        break;
    }
    return err;
}

Err_t NEKO_ForkTransit_reduction(unsigned int in_rankID, Reduct_shared_t * io_shared,
                                 reductionPrivate_t * io_reducPrivate)
{
    int err = 0;
    while(!err){
        io_reducPrivate->nrank  = io_shared->nrank;
        io_reducPrivate->myrank = in_rankID;
        io_reducPrivate->ndata  = io_shared->ndata;
        io_reducPrivate->reductionOperator = REDUC_OPERATION_TYPE;
        io_reducPrivate->rangeGUID = io_shared->reductionRangeGUID;
        io_reducPrivate->new = 1;
        io_reducPrivate->type = ALLREDUCE;

        ocrEventParams_t params;
        params.EVENT_CHANNEL.maxGen = 2; //2 for channel exchange
        params.EVENT_CHANNEL.nbSat = 1;
        params.EVENT_CHANNEL.nbDeps = 1;
        err = ocrEventCreateParams(&(io_reducPrivate->returnEVT), OCR_EVENT_CHANNEL_T, false, &params); IFEB;

        err = clear_Reduct_shared(io_shared); IFEB;

        if(0 == in_rankID){
#           ifdef REDUCTION_CGSTEP0
                PRINTF("INFO> Reduction in CGstep0_start       is active. Slot count used = %d.\n", (int)REDUC_SLOT_4CGstep0);
#           else
                PRINTF("INFO> Reduction in CGstep0_start       is    off. Slot count used = %d.\n", (int)REDUC_SLOT_4CGstep0);
#           endif
#           ifdef REDUCTION_BETA
                PRINTF("INFO> Reduction in nekbone_beta_start  is active. Slot count used = %d.\n", (int)REDUC_SLOT_4Beta);
#           else
                PRINTF("INFO> Reduction in nekbone_beta_start  is    off. Slot count used = %d.\n", (int)REDUC_SLOT_4Beta);
#           endif
#           ifdef REDUCTION_ALPHA
                PRINTF("INFO> Reduction in nekbone_alpha_start is active. Slot count used = %d.\n", (int)REDUC_SLOT_4Alpha);
#           else
                PRINTF("INFO> Reduction in nekbone_alpha_start is    off. Slot count used = %d.\n", (int)REDUC_SLOT_4Alpha);
#           endif
#           ifdef REDUCTION_RTR
                PRINTF("INFO> Reduction in nekbone_rtr_start   is active. Slot count used = %d.\n", (int)REDUC_SLOT_4Rtr);
#           else
                PRINTF("INFO> Reduction in nekbone_rtr_start   is    off. Slot count used = %d.\n", (int)REDUC_SLOT_4Rtr);
#           endif
        }
        break;
    }
    return err;
}





















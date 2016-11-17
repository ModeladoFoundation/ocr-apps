#ifndef UTEST_REDUCTION_H
#include "utest_reduction.h"
#endif

#define ENABLE_EXTENSION_LABELING  // For labeled GUIDs
#include "extensions/ocr-labeling.h"  // For labeled GUIDs

#include "reduction.h"

#define XMEMSET(SRC, CHARC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)SRC+xmIT)=CHARC;}
#define XMEMCPY(DEST, SRC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)DEST+xmIT)=*((char*)SRC+xmIT);}

#define IFEB if(err) break

int init_DReduct_shared(unsigned long in_nrank,unsigned long in_ndata, DReduct_shared_t * io)
{
    XMEMSET(io,0,sizeof(DReduct_shared_t));
    io->nrank = in_nrank;
    io->ndata = in_ndata;
    io->reductionRangeGUID = NULL_GUID;
    return 0;
}
int clear_DReduct_shared(DReduct_shared_t * io)
{
    XMEMSET(io,0,sizeof(DReduct_shared_t));
    return 0;
}
int destroy_DReduct_shared(DReduct_shared_t * io)
{
    int err=0;
    if( ! IS_GUID_NULL(io->reductionRangeGUID)){
        err = ocrGuidMapDestroy( io->reductionRangeGUID);
    }
    XMEMSET(io,0,sizeof(DReduct_shared_t));
    return err;
}
int copy_DReduct_shared(DReduct_shared_t * in_from, DReduct_shared_t * o_target)
{
    if( !in_from || !o_target) return __LINE__;
    XMEMCPY(o_target, in_from, sizeof(DReduct_shared_t));
    return 0;
}
void  print_DReduct_shared(DReduct_shared_t * in)
{
    PRINTF("DReduct_shared>[myrank,nrank,ndata,reductRguid]= %lu %lu "GUIDF"\n",
           in->nrank, in->ndata, GUIDA(in->reductionRangeGUID)
           );
}

int utest_mainedt(unsigned long in_nrank,unsigned long in_ndata,
                  unsigned int in_size, ReducValue_t * io_workLeft, ReducValue_t * io_workRight,
                  DReduct_shared_t * io_sharedG, DReduct_shared_t * io_sharedL,
                  DReduct_shared_t * io_sharedR)
{
    int err = 0;
    while(!err){
        unsigned int i;
        for(i=0; i < in_size; ++i){
            ReducValue_t u = i + 1;
            io_workLeft[i]  = u;
            io_workRight[i] = u + in_size;
        }

        err = init_DReduct_shared(in_nrank,in_ndata, io_sharedG); IFEB;
        err = init_DReduct_shared(in_nrank,in_ndata, io_sharedL); IFEB;
        err = init_DReduct_shared(in_nrank,in_ndata, io_sharedR); IFEB;

        ocrGuid_t reductionRangeGUID = NULL_GUID;
        err = ocrGuidRangeCreate(&reductionRangeGUID, in_nrank, GUID_USER_EVENT_STICKY); IFEB;

        GUID_ASSIGN_VALUE(io_sharedG->reductionRangeGUID, reductionRangeGUID);
        GUID_ASSIGN_VALUE(io_sharedL->reductionRangeGUID, reductionRangeGUID);
        GUID_ASSIGN_VALUE(io_sharedR->reductionRangeGUID, reductionRangeGUID);

        break;
    }
    return err;
}

int reduction_init(unsigned int in_rank, DReduct_shared_t * io_shared, reductionPrivate_t * io_reducPrivate)
{
    int err = 0;
    while(!err){
        io_reducPrivate->nrank  = io_shared->nrank;
        io_reducPrivate->myrank = in_rank;
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

        err = clear_DReduct_shared(io_shared); IFEB;
        break;
    }
    return err;
}

int compute_start(unsigned int in_multiplier,
                  ocrGuid_t io_reducPrivateGuid, reductionPrivate_t * io_reducPrivate,
                  unsigned int in_size, ReducValue_t * in_work,
                  unsigned int in_destSlot, ocrGuid_t in_destinationGuid)
{
    int err = 0;
    while(!err){
        ocrGuid_t gd_sum= NULL_GUID;
        ReducSum_t * o_sum=NULL;
        err = ocrDbCreate( &gd_sum, (void**)&o_sum, 1*sizeof(ReducSum_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC); IFEB;

        *o_sum = 0;
        unsigned int i;
        for(i=0; i<in_size; ++i){
            *o_sum += in_work[i] * in_multiplier;
        }

        err = ocrAddDependence(io_reducPrivate->returnEVT, in_destinationGuid, in_destSlot, DB_MODE_RO); IFEB;

        reductionLaunch(io_reducPrivate, io_reducPrivateGuid, o_sum);

        err = ocrDbRelease(gd_sum); IFEB;

        break;
    }
    return err;
}

int leftEDT_stop(reductionPrivate_t * io_reducPrivate, ReducSum_t * in_sum, ocrGuid_t in_sum_guid)
{
    int err = 0;
    while(!err){
        ReducSum_t check = 1;
        check /=2;
        if( check == 0 ) {
            //This is for integers.
            long x = *in_sum;
            PRINTF("LEFT> sum = %ld\n", x);
        } else {
            //This is for floating point numbers.
            double x = *in_sum;
            PRINTF("LEFT> sum = %24.14E\n", x);
        }

        err = ocrDbDestroy( in_sum_guid ); IFEB;

        break;
    }
    return err;
}

int rightEDT_stop(reductionPrivate_t * io_reducPrivate, ReducSum_t * in_sum, ocrGuid_t in_sum_guid)
{
    int err = 0;
    while(!err){
        ReducSum_t check = 1;
        check /=2;
        if( check == 0 ) {
            //This is for integers.
            long x = *in_sum;
            PRINTF("RIGHT> sum = %ld\n", x);
        } else {
            //This is for floating point numbers.
            double x = *in_sum;
            PRINTF("RIGHT> sum = %24.14E\n", x);
        }

        err = ocrDbDestroy( in_sum_guid ); IFEB;

        break;
    }
    return err;
}





















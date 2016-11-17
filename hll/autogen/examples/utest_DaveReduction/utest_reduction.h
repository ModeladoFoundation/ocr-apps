#ifndef UTEST_REDUCTION_H
#define UTEST_REDUCTION_H

#include "ocr.h"
#include "ocr-std.h"

typedef struct reductionPrivateBase reductionPrivate_t; //Forward declaration

//IMPORTANT: Try to pick an appropriate choice of REDUC_OPERATION_TYPE that matches the type of ReducSum_t.
//           Typically integer-to-integer, float-to-float, 4bytes type-to-4bytes type, etc.
//           The proper choices for REDUC_OPERATION_TYPE are one of the

typedef unsigned short ReducValue_t;
typedef unsigned long ReducSum_t;
#define REDUC_OPERATION_TYPE REDUCTION_F8_ADD

//typedef double ReducValue_t;
//typedef double ReducSum_t;
//#define REDUC_OPERATION_TYPE REDUCTION_F8_ADD

#define SLOT4REDUCTION 4

#define DReduct_shared_NOTA_RANK ((unsigned long)(-1))
typedef struct DReduct_shared {
    unsigned long nrank;            //The number of simulated ranks
    unsigned long ndata;            //This is the cardinal count of things to contain.
    ocrGuid_t reductionRangeGUID;   //The ranged Guids set, of size nrank
} DReduct_shared_t;

//These functions return zero upon success.
int init_DReduct_shared(unsigned long in_nrank,unsigned long in_ndata, DReduct_shared_t * io);
int clear_DReduct_shared(DReduct_shared_t * io);
int destroy_DReduct_shared(DReduct_shared_t * io);
int copy_DReduct_shared(DReduct_shared_t * in_from, DReduct_shared_t * o_target);
void  print_DReduct_shared(DReduct_shared_t * in);


int utest_mainedt(unsigned long in_nrank,unsigned long in_ndata,
                  unsigned int in_size, ReducValue_t * io_workLeft, ReducValue_t * io_workRight,
                  DReduct_shared_t * io_sharedG, DReduct_shared_t * io_sharedL,
                  DReduct_shared_t * io_sharedR);

int reduction_init(unsigned int in_rank, DReduct_shared_t * in_shared, reductionPrivate_t * io_reducPrivate);

int compute_start(unsigned int in_multiplier,
                  ocrGuid_t io_reducPrivateGuid, reductionPrivate_t * io_reducPrivate,
                  unsigned int in_size, ReducValue_t * in_work,
                  unsigned int in_destSlot, ocrGuid_t in_destinationGuid);

int leftEDT_stop(reductionPrivate_t * io_reducPrivate, ReducSum_t * in_sum, ocrGuid_t in_sum_guid);
int rightEDT_stop(reductionPrivate_t * io_reducPrivate, ReducSum_t * in_sum, ocrGuid_t in_sum_guid);

#endif // UTEST_REDUCTION_H

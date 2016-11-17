#ifndef NEKBONE_REDUCTION_H
#define NEKBONE_REDUCTION_H

#include "app_ocr_err_util.h"
#include "ocr.h"
#include "ocr-std.h"


//Comment out any of these lines in order to disable specific use of Reduction
#define REDUCTION_CGSTEP0  // For Reduction done in nekbone_cg.h::nekbone_CGstep0_start()
#define REDUCTION_BETA     // For Reduction done in nekbone_cg.h::nekbone_beta_start()
#define REDUCTION_ALPHA    // For Reduction done in nekbone_cg.h::nekbone_alpha_start()
#define REDUCTION_RTR      // For Reduction done in nekbone_cg.h::nekbone_rtr_start()

//=The following are slots count and position overrides used for reduction
// IMPORTANT: If they are changed also change their values in nekbone_inOCR.py
//=These still needs to be used even if Reduction is not in use.
#define REDUC_SLOT_4CGstep0 8
#define REDUC_SLOT_4Beta    19
#define REDUC_SLOT_4Alpha   19
#define REDUC_SLOT_4Rtr     19

typedef struct reductionPrivateBase reductionPrivate_t; //Forward declaration
Err_t copy_Reduct_private(reductionPrivate_t * in_from, reductionPrivate_t * o_target);

typedef double ReducValue_t;
typedef double ReducSum_t;
#define REDUC_OPERATION_TYPE REDUCTION_F8_ADD

typedef struct Reduct_shared {
    unsigned long nrank;            //The number of simulated ranks
    unsigned long ndata;            //This is the cardinal count of things to contain. For here, 1.
    ocrGuid_t reductionRangeGUID;   //The ranged Guids set, of size nrank
} Reduct_shared_t;

//These functions return zero upon success.
Err_t init_Reduct_shared(unsigned long in_nrank,unsigned long in_ndata, Reduct_shared_t * io);
Err_t clear_Reduct_shared(Reduct_shared_t * io);
Err_t destroy_Reduct_shared(Reduct_shared_t * io);
Err_t copy_Reduct_shared(Reduct_shared_t * in_from, Reduct_shared_t * o_target);
void  print_Reduct_shared(Reduct_shared_t * in);

Err_t NEKO_mainEdt_reduction(unsigned long in_nrank,unsigned long in_ndata,
                             Reduct_shared_t * io_sharedRef, Reduct_shared_t * io_shared);

Err_t NEKO_finalEdt_reduction(Reduct_shared_t * io_sharedRef);

Err_t NEKO_ForkTransit_reduction(unsigned int in_rankID, Reduct_shared_t * in_shared,
                                 reductionPrivate_t * io_reducPrivate);

#endif // NEKBONE_REDUCTION_H

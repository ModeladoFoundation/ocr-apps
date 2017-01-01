#ifndef NEKBONE_CG_H
#define NEKBONE_CG_H

#include "app_ocr_err_util.h"
#include "neko_globals.h"
#include "nbn_setup.h"
#include "ocr.h"

#include "neko_reduction.h"
typedef struct reductionPrivateBase reductionPrivate_t; //Forward declaration

//This struct contains all the scalar values thatvare used in the NEkbone
//CG parts.
//In as much as possible, the original names were preserved.
typedef struct NEKO_CGscalars
{
    long currentCGiteration;
    NBN_REAL rtz1, rtz2;
    NBN_REAL beta, pap, alpha, alphm, rtr;
    NBN_REAL rnorm;

    //Variables in Nekbone but not in use there nor here: NBN_REAL rlim2, rtr0

    TimeMark_t at_TailRecusionStart;
} NEKO_CGscalars_t;

Err_t init_NEKO_CGscalars(NEKO_CGscalars_t * io);
Err_t destroy_NEKO_CGscalars(NEKO_CGscalars_t * io);
Err_t copy_NEKO_CGscalars(NEKO_CGscalars_t * in_from, NEKO_CGscalars_t * o_target);
void  print_NEKO_CGscalars(NEKO_CGscalars_t * in);

//This only times the Nekbone specific computation done within each EDTs
//involved in the CG iteration, not the OCR framework part.
//All times are cumulative times of the time spent only in a given section/EDT.
typedef struct NEKO_CGtimings
{
    TimeMark_t cumu_tailRecurTransitBEGIN;
    TimeMark_t cumu_nekCG_solveMi;

    TimeMark_t cumu_nekCG_beta_start;
    TimeMark_t cumu_nekCG_beta_stop;
    TimeMark_t at_nekCG_beta_start; //Used to calculate the total duration of beta
    TimeMark_t cumu_nekCG_beta_transit; //This is transit time between nekCG_beta_start & nekCG_beta_stop.

    TimeMark_t cumu_nekCG_axi_start;
    TimeMark_t cumu_nekCG_axi_stop;
    TimeMark_t at_nekCG_axi_start; //Used to calculate the total duration of axi
    TimeMark_t cumu_nekCG_axi_transit; //This is transit time between nekCG_axi_start & nekCG_axi_stop.

    TimeMark_t cumu_nekCG_alpha_start;
    TimeMark_t cumu_nekCG_alpha_stop;
    TimeMark_t at_nekCG_alpha_start; //Used to calculate the total duration of alpha
    TimeMark_t cumu_nekCG_alpha_transit; //This is transit time between nekCG_alpha_start & nekCG_alpha_stop.

    TimeMark_t cumu_nekCG_rtr_start;
    TimeMark_t cumu_nekCG_rtr_stop;
    TimeMark_t at_nekCG_rtr_start; //Used to calculate the total duration of rtr
    TimeMark_t cumu_nekCG_rtr_transit; //This is transit time between nekCG_rtr_start & nekCG_rtr_stop.

} NEKO_CGtimings_t;

Err_t init_NEKO_CGtimings(NEKO_CGtimings_t * io);
Err_t destroy_NEKO_CGtimings(NEKO_CGtimings_t * io);
Err_t copy_NEKO_CGtimings(NEKO_CGtimings_t * in_from, NEKO_CGtimings_t * o_target);
void  print_NEKO_CGtimings(NEKO_CGtimings_t * in);

Err_t nekbone_setupTailRecusion(NEKOglobals_t * in_NEKOglobals,
                                NEKO_CGscalars_t * in_CGstats,
                                NEKO_CGscalars_t * io_CGstats);

Err_t nekbone_CGstep0_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                            NBN_REAL * in_F, NBN_REAL * in_C, NBN_REAL * io_R,
                            ocrGuid_t io_reducPrivateGuid, reductionPrivate_t * io_reducPrivate,
                            unsigned int in_destSlot, ocrGuid_t in_destinationGuid
                           );

Err_t nekbone_CGstep0_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                          NBN_REAL *io_X, NBN_REAL *io_W, NBN_REAL *io_P,NBN_REAL *io_Z,
                          NEKO_CGscalars_t * io_CGscalars, NEKO_CGtimings_t * io_CGtimes,
                          ocrGuid_t in_sum_guid, ReducSum_t * in_sum);

Err_t nekbone_tailRecursionELSE(NEKO_CGtimings_t * io_CGtimes);

Err_t nekbone_tailTransitBegin(long in_currentIteration,
                               NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats,
                               NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes);

Err_t nekbone_solveMi(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                      NBN_REAL *in_R, NBN_REAL *io_Z,
                      NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes);

Err_t nekbone_beta_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                         NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats,
                         NBN_REAL *io_R, NBN_REAL *io_C, NBN_REAL *io_Z,
                         NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes,
                         ocrGuid_t io_reducPrivateGuid, reductionPrivate_t * io_reducPrivate,
                         unsigned int in_destSlot, ocrGuid_t in_destinationGuid
                         );

Err_t nekbone_beta_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                        NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats,
                        NBN_REAL *in_P, NBN_REAL *io_Z, NBN_REAL *io_P,
                        NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes,
                        ocrGuid_t in_sum_guid, ReducSum_t * in_sum
                        );

Err_t nekbone_alpha_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                          NBN_REAL *io_W, NBN_REAL *io_C, NBN_REAL *io_P,
                          NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes,
                          ocrGuid_t io_reducPrivateGuid, reductionPrivate_t * io_reducPrivate,
                          unsigned int in_destSlot, ocrGuid_t in_destinationGuid
                         );

Err_t nekbone_alpha_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                         NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats,
                         NBN_REAL *in_X, NBN_REAL *in_P, NBN_REAL *io_X,
                         NBN_REAL *in_R, NBN_REAL *in_W, NBN_REAL *io_R,
                         NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes,
                         ocrGuid_t in_sum_guid, ReducSum_t * in_sum
                         );

Err_t nekbone_rtr_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                        NBN_REAL *io_R, NBN_REAL *io_C,
                        NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes,
                        ocrGuid_t io_reducPrivateGuid, reductionPrivate_t * io_reducPrivate,
                        unsigned int in_destSlot, ocrGuid_t in_destinationGuid
                        );

Err_t nekbone_rtr_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                       NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats,
                       NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes,
                       ocrGuid_t in_sum_guid, ReducSum_t * in_sum);

Err_t nekbone_ai_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                       NBN_REAL *io_W, NBN_REAL *io_P,
                       NBN_REAL * io_UR, NBN_REAL * io_US, NBN_REAL * io_UT,
                       NBN_REAL * in_G1, NBN_REAL * in_G4, NBN_REAL * in_G6,
                       NBN_REAL * in_dxm1, NBN_REAL * in_dxTm1,
                       NBN_REAL * io_AItemp,
                       NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes
                       );

Err_t nekbone_ai_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                      NBN_REAL *in_P, NBN_REAL *io_W,
                      NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes);

Err_t nekbone_tailRecurTransitEND(long in_current_iteration,
                                  unsigned int in_rankID,
                                  NEKO_CGscalars_t * in_CGstats,
                                  NEKO_CGscalars_t * io_CGstats);

Err_t nekbone_BtForkTransition_Stop(NEKOglobals_t * in_NEKOglobals);

#endif // NEKBONE_CG_H

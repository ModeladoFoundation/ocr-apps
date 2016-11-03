#ifndef NEKBONE_CG_H
#define NEKBONE_CG_H

#include "app_ocr_err_util.h"
#include "neko_globals.h"
#include "nbn_setup.h"

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
} NEKO_CGscalars_t;

Err_t init_NEKO_CGscalars(NEKO_CGscalars_t * io);
Err_t destroy_NEKO_CGscalars(NEKO_CGscalars_t * io);
Err_t copy_NEKO_CGscalars(NEKO_CGscalars_t * in_from, NEKO_CGscalars_t * o_target);
void  print_NEKO_CGscalars(NEKO_CGscalars_t * in);

Err_t nekbone_CGstep0_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                            NBN_REAL * in_F, NBN_REAL * in_C, NBN_REAL * io_R );

Err_t nekbone_CGstep0_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                          NBN_REAL *io_X, NBN_REAL *io_W, NBN_REAL *io_P,NBN_REAL *io_Z,
                          NEKO_CGscalars_t * io_CGscalars);

Err_t nekbone_tailTransitBegin(long in_currentIteration,
                               NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats);

Err_t nekbone_solveMi(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                          NBN_REAL *in_R, NBN_REAL *io_Z);

Err_t nekbone_beta_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                         NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats,
                         NBN_REAL *io_R, NBN_REAL *io_C, NBN_REAL *io_Z,
                         NBN_REAL * o_partial_sum_rcz
                         );

Err_t nekbone_beta_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                        NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats,
                        NBN_REAL * in_sum_rcz,
                        NBN_REAL *in_P, NBN_REAL *io_Z, NBN_REAL *io_P
                        );

Err_t nekbone_alpha_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                          NBN_REAL *io_W, NBN_REAL *io_C, NBN_REAL *io_P,
                          NBN_REAL * o_partial_sum_pap
                         );

Err_t nekbone_alpha_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                         NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats,
                         NBN_REAL * in_sum_pap,
                         NBN_REAL *in_X, NBN_REAL *in_P, NBN_REAL *io_X,
                         NBN_REAL *in_R, NBN_REAL *in_W, NBN_REAL *io_R
                         );

Err_t nekbone_rtr_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                        NBN_REAL *io_R, NBN_REAL *io_C, NBN_REAL * o_partial_sum_rtr );

Err_t nekbone_rtr_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                       NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats,
                       NBN_REAL * in_sum_rtr);

Err_t nekbone_ai_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                       NBN_REAL *io_W, NBN_REAL *io_P,
                       NBN_REAL * io_UR, NBN_REAL * io_US, NBN_REAL * io_UT,
                       NBN_REAL * in_G1, NBN_REAL * in_G4, NBN_REAL * in_G6,
                       NBN_REAL * in_dxm1, NBN_REAL * in_dxTm1,
                       NBN_REAL * io_AItemp);

Err_t nekbone_ai_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                      NBN_REAL *in_W, NBN_REAL *in_P, NBN_REAL *io_W);

#endif // NEKBONE_CG_H

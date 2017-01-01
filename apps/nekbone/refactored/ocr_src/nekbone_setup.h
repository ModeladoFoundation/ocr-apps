#ifndef NEKBONE_SETUP_H
#define NEKBONE_SETUP_H

#include "app_ocr_err_util.h"
#include "neko_globals.h"
#include "nbn_setup.h"

#include "blas.h"

Err_t nekbone_setup(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * io_NEKOglobals,
                    unsigned int * io_lglel, unsigned long * io_glo_num);

Err_t nekbone_set_multiplicity_start(NEKOglobals_t * in_NEKOglobals, NBN_REAL * io_C);
Err_t nekbone_set_multiplicity_stop(NEKOglobals_t * in_NEKOglobals, NBN_REAL * io_C);

//I'll prefix with a 'h' all variables used only for a hop, i.e. locally used.
//I'll try to keep the same confusing variable naming as used in Nekbone.
Err_t nekbone_proxy_setup(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                          NBN_REAL *io_wA, NBN_REAL *io_wC, NBN_REAL *io_wD,
                          NBN_REAL *io_wB, NBN_REAL *io_wZ, NBN_REAL *io_wW,
                          double * io_wZd, double * io_wWd,
                          NBN_REAL * io_G1, NBN_REAL * io_G4, NBN_REAL * io_G6,
                          BLAS_REAL_TYPE * io_dxm1, BLAS_REAL_TYPE * io_dxTm1
                          );

Err_t nekbone_set_f_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                          NBN_REAL * io_f);

Err_t nekbone_set_f_stop(NEKOglobals_t * in_NEKOglobals,
                         NBN_REAL * in_C, NBN_REAL * io_f);


#endif // NEKBONE_SETUP_H

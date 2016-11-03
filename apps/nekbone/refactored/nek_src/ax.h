#ifndef NEKBONE_AX_H
#define NEKBONE_AX_H

#include "nbn_setup.h"
#include "blas3.h"

int nbb_ax_e(unsigned int in_pDOF, NBN_REAL * io_W, NBN_REAL * io_U,
             nbb_matrix2_t in_D, nbb_matrix2_t in_Dt,
             NBN_REAL * in_g1, NBN_REAL * in_g4, NBN_REAL * in_g6,
             NBN_REAL * io_wUR, NBN_REAL * io_wUS, NBN_REAL * io_wUT,
             NBN_REAL * io_temp
             );

int nbb_axi_before(unsigned int in_Ecount, unsigned int in_pDOF,
                   NBN_REAL * io_W, NBN_REAL * io_P,
                   nbb_matrix2_t in_D, nbb_matrix2_t in_Dt,
                   NBN_REAL * in_g1, NBN_REAL * in_g4, NBN_REAL * in_g6,
                   NBN_REAL * io_wUR, NBN_REAL * io_wUS, NBN_REAL * io_wUT,
                   NBN_REAL * io_temp );

#endif // NEKBONE_AX_H


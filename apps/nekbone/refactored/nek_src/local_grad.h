#ifndef NEKBONE_LOCAL_GRAD_H
#define NEKBONE_LOCAL_GRAD_H

#include "blas.h"
#include "blas3.h"

int nbb_local_grad3(nbb_matrix3_t in_u,
                    nbb_matrix2_t in_D,
                    nbb_matrix2_t in_Dt,
                    nbb_matrix3_t * o_ur,
                    nbb_matrix3_t * o_us,
                    nbb_matrix3_t * o_ut
                    );

int nbb_local_grad3_t(nbb_matrix3_t in_u,
                    nbb_matrix2_t in_D,
                    nbb_matrix2_t in_Dt,
                    nbb_matrix3_t * o_ur,
                    nbb_matrix3_t * o_us,
                    nbb_matrix3_t * o_ut
                    );

#endif // NEKBONE_LOCAL_GRAD_H

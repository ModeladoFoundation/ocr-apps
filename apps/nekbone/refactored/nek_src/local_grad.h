#ifndef NEKBONE_LOCAL_GRAD_H
#define NEKBONE_LOCAL_GRAD_H

#include "blas3.h"

int nbb_local_grad3(nbb_matrix2_t in_D, nbb_matrix2_t in_Dt,
                    nbb_matrix3_t * io_ur, nbb_matrix3_t * io_us,
                    nbb_matrix3_t * io_ut, nbb_matrix3_t * io_u);

int nbb_local_grad3_t(nbb_matrix2_t in_D, nbb_matrix2_t in_Dt,
                      nbb_matrix3_t * io_ur, nbb_matrix3_t * io_us,
                      nbb_matrix3_t * io_ut, nbb_matrix3_t * io_u,
                      nbb_matrix3_t * io_temp);

#endif // NEKBONE_LOCAL_GRAD_H

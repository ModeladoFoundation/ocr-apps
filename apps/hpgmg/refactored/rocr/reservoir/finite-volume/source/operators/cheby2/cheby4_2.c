#include "kernels2.h"

#pragma rstream map
void cheby_mappable_kernel4_2(
        int H2INV,
        int s, int ss, int ghosts, int ghostsToOperateOn, int dim,
        const double chebyshev_c1[CHEBYSHEV_DEGREE],
        const double chebyshev_c2[CHEBYSHEV_DEGREE],
        double x_np1_0[4][4][4],       double x_np1_1[4][4][4],
        const double rhs[4][4][4],    const double alpha[4][4][4],
        const double beta_i[4][4][4],  const double beta_j[4][4][4],
        const double beta_k[4][4][4],  const double Dinv[4][4][4],
        const double valid[4][4][4]) {
    int i; int j; int k;
    KERNEL_2;
}
#undef h2inv

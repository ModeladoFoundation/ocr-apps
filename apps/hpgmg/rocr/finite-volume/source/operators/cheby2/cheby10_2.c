#include "kernels2.h"

#pragma rstream map
void cheby_mappable_kernel10_2(
        int H2INV,
        int s, int ss, int ghosts, int ghostsToOperateOn, int dim,
        const double chebyshev_c1[CHEBYSHEV_DEGREE],
        const double chebyshev_c2[CHEBYSHEV_DEGREE],
        double x_np1_0[10][10][10],       double x_np1_1[10][10][10],
        const double rhs[10][10][10],    const double alpha[10][10][10],
        const double beta_i[10][10][10],  const double beta_j[10][10][10],
        const double beta_k[10][10][10],  const double Dinv[10][10][10],
        const double valid[10][10][10]) {
    int i; int j; int k;
    KERNEL_2;
}
#undef h2inv

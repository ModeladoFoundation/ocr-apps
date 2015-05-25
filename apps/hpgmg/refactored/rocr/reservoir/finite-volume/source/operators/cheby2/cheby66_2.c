#include "kernels2.h"

#pragma rstream map
void cheby_mappable_kernel66_2(
        int H2INV,
        int s, int ss, int ghosts, int ghostsToOperateOn, int dim,
        const double chebyshev_c1[CHEBYSHEV_DEGREE],
        const double chebyshev_c2[CHEBYSHEV_DEGREE],
        double x_np1_0[66][66][66],       double x_np1_1[66][66][66],
        const double rhs[66][66][66],    const double alpha[66][66][66],
        const double beta_i[66][66][66],  const double beta_j[66][66][66],
        const double beta_k[66][66][66],  const double Dinv[66][66][66],
        const double valid[66][66][66]) {
    int i; int j; int k;
    KERNEL_2;
}
#undef h2inv

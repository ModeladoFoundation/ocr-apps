#include "kernels2.h"

#pragma rstream map
void cheby_mappable_kernel34_2(
        int H2INV,
        int s, int ss, int ghosts, int ghostsToOperateOn, int dim,
        const double chebyshev_c1[CHEBYSHEV_DEGREE],
        const double chebyshev_c2[CHEBYSHEV_DEGREE],
        double x_np1_0[34][34][34],       double x_np1_1[34][34][34],
        const double rhs[34][34][34],    const double alpha[34][34][34],
        const double beta_i[34][34][34],  const double beta_j[34][34][34],
        const double beta_k[34][34][34],  const double Dinv[34][34][34],
        const double valid[34][34][34]) {
    int i; int j; int k;
    KERNEL_2;
}
#undef h2inv

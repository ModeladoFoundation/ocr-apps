#include "kernels2.h"

#pragma rstream map
void cheby_mappable_kernel258_1(
        int H2INV,
        int s, int ss, int ghosts, int ghostsToOperateOn, int dim,
        const double chebyshev_c1[CHEBYSHEV_DEGREE],
        const double chebyshev_c2[CHEBYSHEV_DEGREE],
        double x_np1_0[258][258][258],       double x_np1_1[258][258][258],
        const double rhs[258][258][258],    const double alpha[258][258][258],
        const double beta_i[258][258][258],  const double beta_j[258][258][258],
        const double beta_k[258][258][258],  const double Dinv[258][258][258],
        const double valid[258][258][258]) {
    int i; int j; int k;
    KERNEL_1;
}
#undef h2inv

#include "kernels2.h"

#pragma rstream map
void cheby_mappable_kernel130_2(
        int H2INV,
        int s, int ss, int ghosts, int ghostsToOperateOn, int dim,
        const double chebyshev_c1[CHEBYSHEV_DEGREE],
        const double chebyshev_c2[CHEBYSHEV_DEGREE],
        double x_np1_0[130][130][130],       double x_np1_1[130][130][130],
        const double rhs[130][130][130],    const double alpha[130][130][130],
        const double beta_i[130][130][130],  const double beta_j[130][130][130],
        const double beta_k[130][130][130],  const double Dinv[130][130][130],
        const double valid[130][130][130]) {
    int i; int j; int k;
    KERNEL_2;
}
#undef h2inv

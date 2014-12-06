#include "kernels2.h"

#pragma rstream map
void cheby_mappable_kernel18_1(
        int H2INV,
        int s, int ss, int ghosts, int ghostsToOperateOn, int dim,
        const double chebyshev_c1[CHEBYSHEV_DEGREE],
        const double chebyshev_c2[CHEBYSHEV_DEGREE],
        double x_np1_0[18][18][18],       double x_np1_1[18][18][18],
        const double rhs[18][18][18],    const double alpha[18][18][18],
        const double beta_i[18][18][18],  const double beta_j[18][18][18],
        const double beta_k[18][18][18],  const double Dinv[18][18][18],
        const double valid[18][18][18]) {
    int i; int j; int k;
    KERNEL_1;
}
#undef h2inv

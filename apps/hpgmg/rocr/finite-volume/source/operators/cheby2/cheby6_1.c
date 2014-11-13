#include "kernels2.h"

#pragma rstream map
void cheby_mappable_kernel6_1(
        int H2INV,
        int s, int ss, int ghosts, int ghostsToOperateOn, int dim,
        const double chebyshev_c1[CHEBYSHEV_DEGREE],
        const double chebyshev_c2[CHEBYSHEV_DEGREE],
        double x_np1_0[6][6][6],       double x_np1_1[6][6][6],
        const double rhs[6][6][6],    const double alpha[6][6][6],
        const double beta_i[6][6][6],  const double beta_j[6][6][6],
        const double beta_k[6][6][6],  const double Dinv[6][6][6],
        const double valid[6][6][6]) {
    int i; int j; int k;
    KERNEL_1;
}
#undef h2inv

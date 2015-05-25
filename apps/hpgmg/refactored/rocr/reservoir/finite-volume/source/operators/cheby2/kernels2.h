#ifndef CHEBYSHEV_DEGREE
#define CHEBYSHEV_DEGREE 4
#endif
#define h2inv ((double)H2INV) \

#define rstream_calculate_Dinv()                                              \
(                                                                             \
  1.0 / (a*alpha[k][j][i] - b*h2inv*(                                         \
           + beta_i[k][j][i]  *( valid[k][j][i-1] - 2.0 )                     \
           + beta_j[k][j][i]  *( valid[k][j-1][i] - 2.0 )                     \
           + beta_k[k][j][i]  *( valid[k-1][j][i] - 2.0 )                     \
           + beta_i[k][j][i+1]*( valid[k][j][i+1] - 2.0 )                     \
           + beta_j[k][j+1][i]*( valid[k][j+1][i] - 2.0 )                     \
           + beta_k[k+1][j][i]*( valid[k+1][j][i] - 2.0 )                     \
        ))                                                                    \
)
#define rstream_Dinv_ijk() rstream_calculate_Dinv()
#define rstream_apply_op_ijk(x)                                                               \
  (                                                                                           \
    a*alpha[k][j][i]*x[k][j][i] - b*h2inv*(                                                   \
      + beta_i[k][j][i]  *( valid[k][j][i-1]*( x[k][j][i] + x[k][j][i-1] ) - 2.0*x[k][j][i] ) \
      + beta_j[k][j][i]  *( valid[k][j-1][i]*( x[k][j][i] + x[k][j-1][i] ) - 2.0*x[k][j][i] ) \
      + beta_k[k][j][i]  *( valid[k-1][j][i]*( x[k][j][i] + x[k-1][j][i] ) - 2.0*x[k][j][i] ) \
      + beta_i[k][j][i+1]*( valid[k][j][i+1]*( x[k][j][i] + x[k][j][i+1] ) - 2.0*x[k][j][i] ) \
      + beta_j[k][j+1][i]*( valid[k][j+1][i]*( x[k][j][i] + x[k][j+1][i] ) - 2.0*x[k][j][i] ) \
      + beta_k[k+1][j][i]*( valid[k+1][j][i]*( x[k][j][i] + x[k+1][j][i] ) - 2.0*x[k][j][i] ) \
    )                                                                                         \
  )

        // These are the original statements for c1 and c2. For reference data
        // 0 <= ss < CHEBYSHEV_DEGREE at all times. Thus the R-Stream unfriendly
        // mod in the array access function is removed.
        //
        //const double c1 = chebyshev_c1[ss%CHEBYSHEV_DEGREE];                  \
        //const double c2 = chebyshev_c2[ss%CHEBYSHEV_DEGREE];                  \


// if (ss % 2 == 0)
#define KERNEL_1                                                              \
do {                                                                          \
double a = 0.0;                                                               \
double b = 1.0;                                                               \
const double c1 = chebyshev_c1[ss];                                           \
const double c2 = chebyshev_c2[ss];                                           \
for(k=0-ghostsToOperateOn;k<dim+ghostsToOperateOn;k++){                       \
    for(j=0-ghostsToOperateOn;j<dim+ghostsToOperateOn;j++){                   \
        for(i=0-ghostsToOperateOn;i<dim+ghostsToOperateOn;i++){               \
            x_np1_0[k][j][i] = x_np1_1[k][j][i] +                             \
                           c1*(x_np1_1[k][j][i]-x_np1_0[k][j][i]) +           \
                           c2*rstream_Dinv_ijk()*                             \
                           (rhs[k][j][i]-rstream_apply_op_ijk(x_np1_1));      \
        }}}                                                                   \
} while (0)                                                                   \

// else if (ss % 2 != 0)
#define KERNEL_2                                                              \
do {                                                                          \
double a = 0.0;                                                               \
double b = 1.0;                                                               \
const double c1 = chebyshev_c1[ss];                                           \
const double c2 = chebyshev_c2[ss];                                           \
for(k=0-ghostsToOperateOn;k<dim+ghostsToOperateOn;k++){                       \
    for(j=0-ghostsToOperateOn;j<dim+ghostsToOperateOn;j++){                   \
        for(i=0-ghostsToOperateOn;i<dim+ghostsToOperateOn;i++){               \
            x_np1_1[k][j][i] = x_np1_0[k][j][i] +                             \
                           c1*(x_np1_0[k][j][i]-x_np1_1[k][j][i]) +           \
                           c2*rstream_Dinv_ijk()*                             \
                           (rhs[k][j][i]-rstream_apply_op_ijk(x_np1_0));      \
        }}}                                                                   \
}                                                                             \
while (0)


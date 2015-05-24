#include "gsrb_operator_defines.h"

#ifndef bSIZE
	#error Please specify the sizes of the data arrays using the bSIZE preprocessor variable
#endif

#define b 1.0

// #ifndef H2INV
// 	#error Please specify h2inv variable
// #endif

#pragma rstream map
void gsrb_mappable_kernel_red( const int dim,
							double phi[bSIZE][bSIZE][bSIZE],
                            const double Dinv[bSIZE][bSIZE][bSIZE],
							const double rhs[bSIZE][bSIZE][bSIZE],
							const double a,
                            const int H,
							const double alpha[bSIZE][bSIZE][bSIZE],
							const double beta_i[bSIZE][bSIZE][bSIZE],
							const double beta_j[bSIZE][bSIZE][bSIZE],
							const double beta_k[bSIZE][bSIZE][bSIZE],
							const double valid[bSIZE][bSIZE][bSIZE] )
{

#define h2inv ((double)H)

    int i, j, k;
        for ( k = 0 ; k<dim ; k+=2 ) {
            for ( j = 0 ; j<dim ; j+=2 ) {
                for ( i = 1 ; i<dim ; i+=2 ) {
                    double Ax     = apply_op_ijk_rstream(phi,k,j,i);
                    double lambda = Dinv_ijk_rstream(k,j,i);
                    phi[k][j][i] = phi[k][j][i] + lambda*(rhs[k][j][i]-Ax);
                }
                for ( i = 0 ; i<dim ; i+=2 ) {
                    double Ax     = apply_op_ijk_rstream(phi,k,(j+1),i);
                    double lambda = Dinv_ijk_rstream(k,(j+1),i);
                    phi[k][j+1][i] = phi[k][j+1][i] + lambda*(rhs[k][j+1][i]-Ax);
                }
                for ( i = 0 ; i<dim ; i+=2 ) {
                    double Ax     = apply_op_ijk_rstream(phi,(k+1),j,i);
                    double lambda = Dinv_ijk_rstream((k+1),j,i);
                    phi[k+1][j][i] = phi[k+1][j][i] + lambda*(rhs[k+1][j][i]-Ax);
                }
                for ( i = 1 ; i<dim ; i+=2 ) {
                    double Ax     = apply_op_ijk_rstream(phi,(k+1),(j+1),i);
                    double lambda = Dinv_ijk_rstream((k+1),(j+1),i);
                    phi[k+1][j+1][i] = phi[k+1][j+1][i] + lambda*(rhs[k+1][j+1][i]-Ax);
                }
            }
        }
#undef h2inv
}

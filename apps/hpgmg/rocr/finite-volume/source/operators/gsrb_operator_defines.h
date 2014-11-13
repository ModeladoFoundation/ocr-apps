
#ifndef GSRB_OPERATOR_DEFINES_H
#define GSRB_OPERATOR_DEFINES_H

// #define GSRB_FP
// #define GSRB_STRIDE2

#define STENCIL_VARIABLE_COEFFICIENT_RSTREAM
#define STENCIL_FUSE_DINV_RSTREAM
#define STENCIL_FUSE_BC_RSTREAM

#define GSRB_R_STREAM_EXEC_1(C) \
	if ( jStride == 4 ) { \
		double (*phiM)[4][4] = (double (*)[4][4]) phi; \
		const double (*DinvM)[4][4] = (const double (*)[4][4]) Dinv; \
		const double (*rhsM)[4][4] = (const double (*)[4][4]) rhs; \
		const double (*alphaM)[4][4] = (const double (*)[4][4]) alpha; \
		const double (*beta_iM)[4][4] = (const double (*)[4][4]) beta_i; \
		const double (*beta_jM)[4][4] = (const double (*)[4][4]) beta_j; \
		const double (*beta_kM)[4][4] = (const double (*)[4][4]) beta_k; \
		const double (*validM)[4][4] = (const double (*)[4][4]) valid; \
		gsrb_mapped_kernel4_##C(dim,phiM,DinvM,rhsM,a, (int)h2inv, \
								alphaM,beta_iM,beta_jM,beta_kM,validM); \
	}

#define GSRB_R_STREAM_EXEC_2(C) \
	GSRB_R_STREAM_EXEC_1(C) \
	else if ( jStride == 6 ) { \
		double (*phiM)[6][6] = (double (*)[6][6]) phi; \
		const double (*DinvM)[6][6] = (const double (*)[6][6]) Dinv; \
		const double (*rhsM)[6][6] = (const double (*)[6][6]) rhs; \
		const double (*alphaM)[6][6] = (const double (*)[6][6]) alpha; \
		const double (*beta_iM)[6][6] = (const double (*)[6][6]) beta_i; \
		const double (*beta_jM)[6][6] = (const double (*)[6][6]) beta_j; \
		const double (*beta_kM)[6][6] = (const double (*)[6][6]) beta_k; \
		const double (*validM)[6][6] = (const double (*)[6][6]) valid; \
		gsrb_mapped_kernel6_##C(dim,phiM,DinvM,rhsM,a, (int)h2inv, \
									alphaM,beta_iM,beta_jM,beta_kM,validM); \
	}

#define GSRB_R_STREAM_EXEC_3(C) \
	GSRB_R_STREAM_EXEC_2(C) \
	else if ( jStride == 10 ) { \
		double (*phiM)[10][10] = (double (*)[10][10]) phi; \
		const double (*DinvM)[10][10] = (const double (*)[10][10]) Dinv; \
		const double (*rhsM)[10][10] = (const double (*)[10][10]) rhs; \
		const double (*alphaM)[10][10] = (const double (*)[10][10]) alpha; \
		const double (*beta_iM)[10][10] = (const double (*)[10][10]) beta_i; \
		const double (*beta_jM)[10][10] = (const double (*)[10][10]) beta_j; \
		const double (*beta_kM)[10][10] = (const double (*)[10][10]) beta_k; \
		const double (*validM)[10][10] = (const double (*)[10][10]) valid; \
			gsrb_mapped_kernel10_##C(dim,phiM,DinvM,rhsM,a, (int)h2inv, \
									alphaM,beta_iM,beta_jM,beta_kM,validM); \
	}

#define GSRB_R_STREAM_EXEC_4(C) \
	GSRB_R_STREAM_EXEC_3(C) \
	else if ( jStride == 18 ) { \
		double (*phiM)[18][18] = (double (*)[18][18]) phi; \
		const double (*DinvM)[18][18] = (const double (*)[18][18]) Dinv; \
		const double (*rhsM)[18][18] = (const double (*)[18][18]) rhs; \
		const double (*alphaM)[18][18] = (const double (*)[18][18]) alpha; \
		const double (*beta_iM)[18][18] = (const double (*)[18][18]) beta_i; \
		const double (*beta_jM)[18][18] = (const double (*)[18][18]) beta_j; \
		const double (*beta_kM)[18][18] = (const double (*)[18][18]) beta_k; \
		const double (*validM)[18][18] = (const double (*)[18][18]) valid; \
		gsrb_mapped_kernel18_##C(dim,phiM,DinvM,rhsM,a, (int)h2inv, \
									alphaM,beta_iM,beta_jM,beta_kM,validM); \
	}

#define GSRB_R_STREAM_EXEC_5(C) \
	GSRB_R_STREAM_EXEC_4(C) \
	else if ( jStride == 34 ) { \
		double (*phiM)[34][34] = (double (*)[34][34]) phi; \
		const double (*DinvM)[34][34] = (const double (*)[34][34]) Dinv; \
		const double (*rhsM)[34][34] = (const double (*)[34][34]) rhs; \
		const double (*alphaM)[34][34] = (const double (*)[34][34]) alpha; \
		const double (*beta_iM)[34][34] = (const double (*)[34][34]) beta_i; \
		const double (*beta_jM)[34][34] = (const double (*)[34][34]) beta_j; \
		const double (*beta_kM)[34][34] = (const double (*)[34][34]) beta_k; \
		const double (*validM)[34][34] = (const double (*)[34][34]) valid; \
		gsrb_mapped_kernel34_##C(dim,phiM,DinvM,rhsM,a, (int)h2inv, \
									alphaM,beta_iM,beta_jM,beta_kM,validM); \
	}

#define GSRB_R_STREAM_EXEC_6(C) \
	GSRB_R_STREAM_EXEC_5(C) \
	else if ( jStride == 66 ) { \
		double (*phiM)[66][66] = (double (*)[66][66]) phi; \
		const double (*DinvM)[66][66] = (const double (*)[66][66]) Dinv; \
		const double (*rhsM)[66][66] = (const double (*)[66][66]) rhs; \
		const double (*alphaM)[66][66] = (const double (*)[66][66]) alpha; \
		const double (*beta_iM)[66][66] = (const double (*)[66][66]) beta_i; \
		const double (*beta_jM)[66][66] = (const double (*)[66][66]) beta_j; \
		const double (*beta_kM)[66][66] = (const double (*)[66][66]) beta_k; \
		const double (*validM)[66][66] = (const double (*)[66][66]) valid; \
		gsrb_mapped_kernel66_##C(dim,phiM,DinvM,rhsM,a, (int)h2inv, \
									alphaM,beta_iM,beta_jM,beta_kM,validM); \
	}

#define GSRB_R_STREAM_EXEC_7(C) \
	GSRB_R_STREAM_EXEC_6(C) \
	else if ( jStride == 130 ) { \
		double (*phiM)[130][130] = (double (*)[130][130]) phi; \
		const double (*DinvM)[130][130] = (const double (*)[130][130]) Dinv; \
		const double (*rhsM)[130][130] = (const double (*)[130][130]) rhs; \
		const double (*alphaM)[130][130] = (const double (*)[130][130]) alpha; \
		const double (*beta_iM)[130][130] = (const double (*)[130][130]) beta_i; \
		const double (*beta_jM)[130][130] = (const double (*)[130][130]) beta_j; \
		const double (*beta_kM)[130][130] = (const double (*)[130][130]) beta_k; \
		const double (*validM)[130][130] = (const double (*)[130][130]) valid; \
		gsrb_mapped_kernel130_##C(dim,phiM,DinvM,rhsM,a, (int)h2inv, \
									alphaM,beta_iM,beta_jM,beta_kM,validM); \
	}


#define GSRB_R_STREAM_EXEC(C,V) \
	GSRB_R_STREAM_EXEC_##V(C) \
	else { \
		printf("This level is not supported. Please optimize GSRB with bSIZE=%d \n", jStride); \
	}

//------------------------------------------------------------------------------------------------------------------------------
// calculate Dinv?
#ifdef STENCIL_VARIABLE_COEFFICIENT_RSTREAM
  #define calculate_Dinv_rstream(K,J,I)                                      \
  (                                                             \
    1.0 / (a*alpha[K][J][I] - b*h2inv*(                             \
             + beta_i[K][J][I	 ]*( valid[K][J][I-1] - 2.0 ) \
             + beta_j[K][J][I]*( valid[K][J-1][I] - 2.0 ) \
             + beta_k[K][J][I]*( valid[K-1][J][I] - 2.0 ) \
             + beta_i[K][J][I+1]*( valid[K][J][I+1] - 2.0 ) \
             + beta_j[K][J+1][I]*( valid[K][J+1][I] - 2.0 ) \
             + beta_k[K+1][J][I]*( valid[K+1][J][I] - 2.0 ) \
          ))                                                    \
  )
#else // constant coefficient case...
  #define calculate_Dinv_rstream(K,J,I)          \
  (                                 \
    1.0 / (a - b*h2inv*(            \
             + valid[K][J][I-1]   \
             + valid[K][J-1][I]   \
             + valid[K-1][J][I]   \
             + valid[K][J][I+1]   \
             + valid[K][J+1][I]   \
             + valid[K+1][J][I]   \
             - 12.0                 \
          ))                        \
  )
#endif

#if defined(STENCIL_FUSE_DINV_RSTREAM) && defined(STENCIL_FUSE_BC_RSTREAM)
#define Dinv_ijk_rstream(K,J,I) calculate_Dinv_rstream(K,J,I) // recalculate it
#else
#define Dinv_ijk_rstream(K,J,I) Dinv[K][J][I]        // simply retriev it rather than recalculating it
#endif
//------------------------------------------------------------------------------------------------------------------------------
#ifdef STENCIL_FUSE_BC_RSTREAM

  #ifdef STENCIL_VARIABLE_COEFFICIENT_RSTREAM
    #define apply_op_ijk_rstream(x,K,J,I)                                                                     \
    (                                                                                         \
      a*alpha[K][J][I]*x[K][J][I] - b*h2inv*(                                                         \
        + beta_i[K][J][I]*( valid[K][J][I-1]*( x[K][J][I] + x[K][J][I-1] ) - 2.0*x[K][J][I] ) \
        + beta_j[K][J][I]*( valid[K][J-1][I]*( x[K][J][I] + x[K][J-1][I] ) - 2.0*x[K][J][I] ) \
        + beta_k[K][J][I]*( valid[K-1][J][I]*( x[K][J][I] + x[K-1][J][I] ) - 2.0*x[K][J][I] ) \
        + beta_i[K][J][I+1]*( valid[K][J][I+1]*( x[K][J][I] + x[K][J][I+1] ) - 2.0*x[K][J][I] ) \
        + beta_j[K][J+1][I]*( valid[K][J+1][I]*( x[K][J][I] + x[K][J+1][I] ) - 2.0*x[K][J][I] ) \
        + beta_k[K+1][J][I]*( valid[K+1][J][I]*( x[K][J][I] + x[K+1][J][I] ) - 2.0*x[K][J][I] ) \
      )                                                                                       \
    )
  #else  // constant coefficient case...
    #define apply_op_ijk_rstream(x,K,J,I)                                \
    (                                                    \
      a*x[K][J][I] - b*h2inv*(                               \
        + valid[K][J][I-1]*( x[K][J][I] + x[K][J][I-1] ) \
        + valid[K][J-1][I]*( x[K][J][I] + x[K][J-1][I] ) \
        + valid[K-1][J][I]*( x[K][J][I] + x[K-1][J][I] ) \
        + valid[K][J][I+1]*( x[K][J][I] + x[K][J][I+1] ) \
        + valid[K][J+1][I]*( x[K][J][I] + x[K][J+1][I] ) \
        + valid[K+1][J][I]*( x[K][J][I] + x[K+1][J][I] ) \
                       -12.0*( x[K][J][I]                  ) \
      )                                                  \
    )
  #endif // variable/constant coefficient

#endif


//------------------------------------------------------------------------------------------------------------------------------
#ifndef STENCIL_FUSE_BC_RSTREAM

  #ifdef STENCIL_VARIABLE_COEFFICIENT_RSTREAM
    #define apply_op_ijk_rstream(x,K,J,I)                                 \
    (                                                     \
      a*alpha[K][J][I]*x[K][J][I] - b*h2inv*(                     \
        + beta_i[K][J][I+1]*( x[K][J][I+1] - x[K][J][I] ) \
        + beta_i[K][J][I]*( x[K][J][I-1] - x[K][J][I] ) \
        + beta_j[K][J+1][I]*( x[K][J+1][I] - x[K][J][I] ) \
        + beta_j[K][J][I]*( x[K][J-1][I] - x[K][J][I] ) \
        + beta_k[K+1][J][I]*( x[K+1][J][I] - x[K][J][I] ) \
        + beta_k[K][J][I]*( x[K-1][J][I] - x[K][J][I] ) \
      )                                                   \
    )
  #else  // constant coefficient case...
    #define apply_op_ijk_rstream(x,K,J,I)            \
    (                                \
      a*x[K][J][I] - b*h2inv*(           \
        + x[K][J][I+1]             \
        + x[K][J][I-1]             \
        + x[K][J+1][I]             \
        + x[K][J-1][I]             \
        + x[K+1][J][I]             \
        + x[K-1][J][I]             \
        - x[K][J][I]*6.0         \
      )                              \
    )
  #endif // variable/constant coefficient

#endif // BCs

#endif





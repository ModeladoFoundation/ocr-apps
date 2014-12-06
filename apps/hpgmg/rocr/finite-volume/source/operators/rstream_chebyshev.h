#ifndef RSTREAM_CHEBY_H
#define RSTREAM_CHEBY_H

// Multidimensional array declarations and mapped function calls at levels 1--8
        //const int ghostsM = s + (ghosts / 2); \

#define RSTREAM_CHEBY_EXEC_1 \
    if (jStride == 4) { \
              double (*x_np1_0M)[4][4] = (double (*)[4][4])x_np1_0;\
              double (*x_np1_1M)[4][4] = (double (*)[4][4])x_np1_1;\
        const double (*rhsM)[4][4]     = (const double (*)[4][4])rhs;\
        const double (*alphaM)[4][4]   = (const double (*)[4][4])alpha;\
        const double (*beta_iM)[4][4]  = (const double (*)[4][4])beta_i;\
        const double (*beta_jM)[4][4]  = (const double (*)[4][4])beta_j;\
        const double (*beta_kM)[4][4]  = (const double (*)[4][4])beta_k;\
        const double (*DinvM)[4][4]    = (const double (*)[4][4])Dinv;\
        const double (*validM)[4][4]   = (const double (*)[4][4])valid;\
        const int ghostsM = ghosts; \
        if ((ss & 1) == 0) {\
            cheby_mappable_kernel4_1((int)h2inv, s, ss, ghostsM, ghostsToOperateOn, dim, \
                chebyshev_c1, chebyshev_c2, \
                x_np1_0M, x_np1_1M, \
                rhsM, alphaM, \
                beta_iM, beta_jM, beta_kM, \
                DinvM, validM);\
        } else { \
            cheby_mappable_kernel4_2((int)h2inv, s, ss, ghostsM, ghostsToOperateOn, dim, \
                chebyshev_c1, chebyshev_c2, \
                x_np1_0M, x_np1_1M, \
                rhsM, alphaM, \
                beta_iM, beta_jM, beta_kM, \
                DinvM, validM);\
        } \
    }\

#define RSTREAM_CHEBY_EXEC_2 \
    RSTREAM_CHEBY_EXEC_1 \
    else if (jStride == 6) { \
              double (*x_np1_0M)[6][6]= (double (*)[6][6])x_np1_0;\
              double (*x_np1_1M)[6][6]= (double (*)[6][6])x_np1_1;\
        const double (*rhsM)[6][6]    = (const double (*)[6][6])rhs;\
        const double (*alphaM)[6][6]  = (const double (*)[6][6])alpha;\
        const double (*beta_iM)[6][6] = (const double (*)[6][6])beta_i;\
        const double (*beta_jM)[6][6] = (const double (*)[6][6])beta_j;\
        const double (*beta_kM)[6][6] = (const double (*)[6][6])beta_k;\
        const double (*DinvM)[6][6]   = (const double (*)[6][6])Dinv;\
        const double (*validM)[6][6]  = (const double (*)[6][6])valid;\
        const int ghostsM = ghosts; \
        if ((ss & 1) == 0) {\
        cheby_mappable_kernel6_1((int)h2inv, s, ss, ghostsM, ghostsToOperateOn, dim, \
            chebyshev_c1, chebyshev_c2, \
            x_np1_0M, x_np1_1M, \
            rhsM, alphaM, \
            beta_iM, beta_jM, beta_kM, \
            DinvM, validM);\
        } else { \
        cheby_mappable_kernel6_2((int)h2inv, s, ss, ghostsM, ghostsToOperateOn, dim, \
            chebyshev_c1, chebyshev_c2, \
            x_np1_0M, x_np1_1M, \
            rhsM, alphaM, \
            beta_iM, beta_jM, beta_kM, \
            DinvM, validM);\
        } \
    } \

#define RSTREAM_CHEBY_EXEC_3 \
    RSTREAM_CHEBY_EXEC_2 \
    else if (jStride == 10) { \
        double (*x_np1_0M)[10][10]      = (double (*)[10][10])x_np1_0;\
        double (*x_np1_1M)[10][10]      = (double (*)[10][10])x_np1_1;\
        const double (*rhsM)[10][10]    = (const double (*)[10][10])rhs;\
        const double (*alphaM)[10][10]  = (const double (*)[10][10])alpha;\
        const double (*beta_iM)[10][10] = (const double (*)[10][10])beta_i;\
        const double (*beta_jM)[10][10] = (const double (*)[10][10])beta_j;\
        const double (*beta_kM)[10][10] = (const double (*)[10][10])beta_k;\
        const double (*DinvM)[10][10]   = (const double (*)[10][10])Dinv;\
        const double (*validM)[10][10]  = (const double (*)[10][10])valid;\
        const int ghostsM = ghosts; \
        if ((ss & 1) == 0) {\
        cheby_mappable_kernel10_1((int)h2inv, s, ss, ghostsM, ghostsToOperateOn, dim, \
            chebyshev_c1, chebyshev_c2, \
            x_np1_0M, x_np1_1M, \
            rhsM, alphaM, \
            beta_iM, beta_jM, beta_kM, \
            DinvM, validM);\
        } else { \
        cheby_mappable_kernel10_2((int)h2inv, s, ss, ghostsM, ghostsToOperateOn, dim, \
            chebyshev_c1, chebyshev_c2, \
            x_np1_0M, x_np1_1M, \
            rhsM, alphaM, \
            beta_iM, beta_jM, beta_kM, \
            DinvM, validM);\
        } \
    } \

#define RSTREAM_CHEBY_EXEC_4 \
    RSTREAM_CHEBY_EXEC_3 \
    else if (jStride == 18) { \
        double (*x_np1_0M)[18][18]      = (double (*)[18][18])x_np1_0;\
        double (*x_np1_1M)[18][18]      = (double (*)[18][18])x_np1_1;\
        const double (*rhsM)[18][18]    = (const double (*)[18][18])rhs;\
        const double (*alphaM)[18][18]  = (const double (*)[18][18])alpha;\
        const double (*beta_iM)[18][18] = (const double (*)[18][18])beta_i;\
        const double (*beta_jM)[18][18] = (const double (*)[18][18])beta_j;\
        const double (*beta_kM)[18][18] = (const double (*)[18][18])beta_k;\
        const double (*DinvM)[18][18]   = (const double (*)[18][18])Dinv;\
        const double (*validM)[18][18]  = (const double (*)[18][18])valid;\
        const int ghostsM = ghosts; \
        if ((ss & 1) == 0) {\
        cheby_mappable_kernel18_1((int)h2inv, s, ss, ghostsM, ghostsToOperateOn, dim, \
            chebyshev_c1, chebyshev_c2, \
            x_np1_0M, x_np1_1M, \
            rhsM, alphaM, \
            beta_iM, beta_jM, beta_kM, \
            DinvM, validM);\
        } else { \
        cheby_mappable_kernel18_2((int)h2inv, s, ss, ghostsM, ghostsToOperateOn, dim, \
            chebyshev_c1, chebyshev_c2, \
            x_np1_0M, x_np1_1M, \
            rhsM, alphaM, \
            beta_iM, beta_jM, beta_kM, \
            DinvM, validM);\
        } \
    } \

#define RSTREAM_CHEBY_EXEC_5 \
    RSTREAM_CHEBY_EXEC_4 \
    else if (jStride == 34) { \
              double (*x_np1_0M)[34][34]= (double (*)[34][34])x_np1_0;\
              double (*x_np1_1M)[34][34]= (double (*)[34][34])x_np1_1;\
        const double (*rhsM)[34][34]    = (const double (*)[34][34])rhs;\
        const double (*alphaM)[34][34]  = (const double (*)[34][34])alpha;\
        const double (*beta_iM)[34][34] = (const double (*)[34][34])beta_i;\
        const double (*beta_jM)[34][34] = (const double (*)[34][34])beta_j;\
        const double (*beta_kM)[34][34] = (const double (*)[34][34])beta_k;\
        const double (*DinvM)[34][34]   = (const double (*)[34][34])Dinv;\
        const double (*validM)[34][34]  = (const double (*)[34][34])valid;\
        const int ghostsM = ghosts; \
        if ((ss & 1) == 0) {\
        cheby_mappable_kernel34_1((int)h2inv, s, ss, ghostsM, ghostsToOperateOn, dim, \
            chebyshev_c1, chebyshev_c2, \
            x_np1_0M, x_np1_1M, \
            rhsM, alphaM, \
            beta_iM, beta_jM, beta_kM, \
            DinvM, validM);\
        } else { \
        cheby_mappable_kernel34_2((int)h2inv, s, ss, ghostsM, ghostsToOperateOn, dim, \
            chebyshev_c1, chebyshev_c2, \
            x_np1_0M, x_np1_1M, \
            rhsM, alphaM, \
            beta_iM, beta_jM, beta_kM, \
            DinvM, validM);\
        } \
    } \

#define RSTREAM_CHEBY_EXEC_6 \
    RSTREAM_CHEBY_EXEC_5 \
    else if (jStride == 66) { \
              double (*x_np1_0M)[66][66]= (double (*)[66][66])x_np1_0;\
              double (*x_np1_1M)[66][66]= (double (*)[66][66])x_np1_1;\
        const double (*rhsM)[66][66]    = (const double (*)[66][66])rhs;\
        const double (*alphaM)[66][66]  = (const double (*)[66][66])alpha;\
        const double (*beta_iM)[66][66] = (const double (*)[66][66])beta_i;\
        const double (*beta_jM)[66][66] = (const double (*)[66][66])beta_j;\
        const double (*beta_kM)[66][66] = (const double (*)[66][66])beta_k;\
        const double (*DinvM)[66][66]   = (const double (*)[66][66])Dinv;\
        const double (*validM)[66][66]  = (const double (*)[66][66])valid;\
        const int ghostsM = ghosts; \
        if ((ss & 1) == 0) {\
        cheby_mappable_kernel66_1((int)h2inv, s, ss, ghostsM, ghostsToOperateOn, dim, \
            chebyshev_c1, chebyshev_c2, \
            x_np1_0M, x_np1_1M, \
            rhsM, alphaM, \
            beta_iM, beta_jM, beta_kM, \
            DinvM, validM);\
        } else { \
        cheby_mappable_kernel66_2((int)h2inv, s, ss, ghostsM, ghostsToOperateOn, dim, \
            chebyshev_c1, chebyshev_c2, \
            x_np1_0M, x_np1_1M, \
            rhsM, alphaM, \
            beta_iM, beta_jM, beta_kM, \
            DinvM, validM);\
        } \
    } \

#define RSTREAM_CHEBY_EXEC_7 \
    RSTREAM_CHEBY_EXEC_6 \
    else if (jStride == 130) { \
              double (*x_np1_0M)[130][130]= (double (*)[130][130])x_np1_0;\
              double (*x_np1_1M)[130][130]= (double (*)[130][130])x_np1_1;\
        const double (*rhsM)[130][130]    = (const double (*)[130][130])rhs;\
        const double (*alphaM)[130][130]  = (const double (*)[130][130])alpha;\
        const double (*beta_iM)[130][130] = (const double (*)[130][130])beta_i;\
        const double (*beta_jM)[130][130] = (const double (*)[130][130])beta_j;\
        const double (*beta_kM)[130][130] = (const double (*)[130][130])beta_k;\
        const double (*DinvM)[130][130]   = (const double (*)[130][130])Dinv;\
        const double (*validM)[130][130]  = (const double (*)[130][130])valid;\
        const int ghostsM = ghosts; \
        if ((ss & 1) == 0) {\
        cheby_mappable_kernel130_1((int)h2inv, s, ss, ghostsM, ghostsToOperateOn, dim, \
            chebyshev_c1, chebyshev_c2, \
            x_np1_0M, x_np1_1M, \
            rhsM, alphaM, \
            beta_iM, beta_jM, beta_kM, \
            DinvM, validM);\
        } else { \
        cheby_mappable_kernel130_2((int)h2inv, s, ss, ghostsM, ghostsToOperateOn, dim, \
            chebyshev_c1, chebyshev_c2, \
            x_np1_0M, x_np1_1M, \
            rhsM, alphaM, \
            beta_iM, beta_jM, beta_kM, \
            DinvM, validM);\
        } \
    } \

//#define RSTREAM_CHEBY_EXEC_8 \
//    RSTREAM_CHEBY_EXEC_7 \
//    else if (jStride == 258) { \
//              double (*x_np1_0M)[258][258]= (double (*)[258][258])x_np1_0;\
//              double (*x_np1_1M)[258][258]= (double (*)[258][258])x_np1_1;\
//        const double (*rhsM)[258][258]    = (const double (*)[258][258])rhs;\
//        const double (*alphaM)[258][258]  = (const double (*)[258][258])alpha;\
//        const double (*beta_iM)[258][258] = (const double (*)[258][258])beta_i;\
//        const double (*beta_jM)[258][258] = (const double (*)[258][258])beta_j;\
//        const double (*beta_kM)[258][258] = (const double (*)[258][258])beta_k;\
//        const double (*DinvM)[258][258]   = (const double (*)[258][258])Dinv;\
//        const double (*validM)[258][258]  = (const double (*)[258][258])valid;\
//        const int ghostsM = s + (ghosts / 2); \
//        cheby_mappable_kernel258((int)h2inv, s, ss, ghostsM, ghostsToOperateOn, dim, \
//            chebyshev_c1, chebyshev_c2, \
//            x_np1_0M, x_np1_1M, \
//            rhsM, alphaM, \
//            beta_iM, beta_jM, beta_kM, \
//            DinvM, validM);\
//    } \

#define RSTREAM_CHEBY_EXEC(V)                                                 \
    for (ss=s; ss<s+ghosts;ss++,ghostsToOperateOn--) {                        \
        RSTREAM_CHEBY_EXEC_##V                                                \
        else {                                                                \
            printf("Unsupported level. jStride = %d\n", jStride);             \
        }                                                                     \
    }                                                                         \

#endif // RSTREAM_CHEBY_H

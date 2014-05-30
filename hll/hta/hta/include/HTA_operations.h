#ifndef __HTA_OPS__
#define __HTA_OPS__

#include "HTA.h"
#include "Tuple.h"
#include "CSR.h"

void REDUCE_PRODUCT(HTA_SCALAR_TYPE stype, void* result, void* elem);
void REDUCE_SUM(HTA_SCALAR_TYPE stype, void* result, void* elem);
void REDUCE_SUMSQUARE(HTA_SCALAR_TYPE stype, void* result, void* elem);
void REDUCE_MAX(HTA_SCALAR_TYPE stype, void* result, void* elem);

/// Increment all elements within dense HTA 
void H1_INC(HTA * dest);
void H1S1_MUL(HTA * h, void* scalar);
void H1S1_INIT(HTA * h, void* scalar);

/// Read from src and add 1, store the result to dest
void H2_INC(HTA * dest, HTA * src);
void H2_COPY(HTA * dest, HTA * src);
void H2_ABS(HTA * dest, HTA * src);

/// Apply pow to all elements from src HTA and save the dest HTA
/// It works for double and float only
void H2S1_POW(HTA * dest, HTA * src, void* s);

void H3_PWADD(HTA * dest, HTA * src1, HTA * src2);
void H3_PWSUB(HTA * dest, HTA * src1, HTA * src2);
/// Dense matrix pointwise multiplication
void H3_PWMUL(HTA * dest, HTA * src1, HTA * src2);
void H3_MATMUL(HTA * dest, HTA * src1, HTA * src2);
/// Sparse matrix-dense matrix pointwise multiplication
void H3_SDPWMUL(HTA * hr, HTA * hs, HTA * hd);
/// sparse matrix dense vector multiplication
void H3_SDMV(HTA * hr, HTA * hs, HTA * hd);

#endif

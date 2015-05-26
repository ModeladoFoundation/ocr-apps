#ifndef __HTA_BLAS_H__
#define __HTA_BLAS_H__

#define HTAblasColMajor (0)
#define HTAblasRowMajor (1)
#define HTAblasNoTrans (0)
#define HTAblasTrans (1)

#ifdef __cplusplus
extern "C" {
#endif
void HTAblas_dgemm (const int order,
            const int transA,
            const int transB,
            const int M,
            const int N,
            const int K,
            const double alpha,
            double *A,
            const int lda,
            double *B,
            const int ldb,
            const double beta,
            double *C,
            const int ldc);

#ifdef __cplusplus
}
#endif
#endif

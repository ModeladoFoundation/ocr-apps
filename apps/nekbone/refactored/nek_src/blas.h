#ifndef NEKBONE_BLAS_H
#define NEKBONE_BLAS_H

#define BLAS_REAL_TYPE double
#define BLAS_UINT_TYPE unsigned int

#define NEK_CHECKS_BLAS1 1   //Comments this out in order to disable BLAS1 checks
#define NEK_CHECKS_BLAS3 1   //Comments this out in order to disable BLAS3 checks

//In NEKbone, a vector is simply a C array with a length to be kept separately.

//In NEKbone, a 2D matrix is simply a C array with length = rows*cols, and a specify
//access pattern.

#endif // NEKBONE_BLAS_H

#ifndef NEKBONE_BLAS1_H
#define NEKBONE_BLAS1_H

#include "blas.h"

//----- For integer numbers
typedef struct nbb_ivector {
    BLAS_UINT_TYPE length; //The number of entry in the vector
    BLAS_UINT_TYPE * v;  //Use the accessor functions at() and get() to access an entry
} nbb_ivector_t;

unsigned long nbb_byte_sizeofiv(nbb_ivector_t in_v); //Returns the number of byte needed to
                                                     // store the entire vector for given length.
                                                     //So set the length before use.

BLAS_UINT_TYPE  nbb_getiv(nbb_ivector_t in_v, BLAS_UINT_TYPE in_offset);
BLAS_UINT_TYPE * nbb_ativ(nbb_ivector_t io_v, BLAS_UINT_TYPE in_offset); //Returns address to value
                                                                         //Returns NULL upon failure.
//----- For real numbers
typedef struct nbb_rvector {
    BLAS_UINT_TYPE length; //The number of entry in the vector
    BLAS_REAL_TYPE * v;  //Use the accessor functions at() and get() to access an entry
} nbb_rvector_t;

unsigned long nbb_byte_sizeofrv(nbb_rvector_t in_v); //Returns the number of byte needed to
                                                     // store the entire vector for given length.
                                                     //So set the length before use.

BLAS_REAL_TYPE  nbb_getrv(nbb_rvector_t in_v, BLAS_UINT_TYPE in_offset);
BLAS_REAL_TYPE * nbb_atrv(nbb_rvector_t io_v, BLAS_UINT_TYPE in_offset); //Returns address to value
                                                                         //Returns NULL upon failure.
//----- General methods -- Returns zero upon success
int nbb_rzero(nbb_rvector_t io_v);

int nbb_rzeroi(nbb_ivector_t io_a, BLAS_UINT_TYPE in_start, BLAS_UINT_TYPE in_last);

int nbb_rone(nbb_rvector_t io_v);


int nbb_copy(nbb_rvector_t io_a, nbb_rvector_t in_b);

int nbb_copyi(nbb_rvector_t io_a, nbb_rvector_t in_b,
               BLAS_UINT_TYPE in_start, BLAS_UINT_TYPE in_last);

int nbb_add2(nbb_rvector_t io_a, nbb_rvector_t in_b);

int nbb_col2(nbb_rvector_t io_a, nbb_rvector_t in_b);

int nbb_add2s1i(nbb_rvector_t io_a, nbb_rvector_t in_b, BLAS_REAL_TYPE in_c1,
                BLAS_UINT_TYPE in_start, BLAS_UINT_TYPE in_last);

#endif // NEKBONE_BLAS1_H

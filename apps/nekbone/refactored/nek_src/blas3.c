#ifndef NEKBONE_BLAS3_H
#include "blas3.h"
#endif

//===== Assorted index hashing ================================================
unsigned long hash_columnMajor3(int if_onebased_array_use1,
                               unsigned long Ni, unsigned long Nj, unsigned long Nk,
                               unsigned long i, unsigned long j, unsigned long k)
{
    unsigned int offset = 0;
    if(1 == if_onebased_array_use1){
        //For 1-based array
        --i;
        --j;
        --k;
        offset = 1;
    }

    const unsigned long o_h = (i + Ni * (j + Nj*k) + offset);
    return o_h;
}

void dehash_columnMajor3(int if_onebased_array_use1,
                        unsigned long Ni, unsigned long Nj, unsigned long Nk,
                        unsigned long h, unsigned long * o_i,
                        unsigned long * o_j, unsigned long * o_k)
{
    if(1 == if_onebased_array_use1){
        --h;
    }

    unsigned int u = h / Ni;
    *o_k = u / Nj;
    *o_j = u - Nj * (*o_k);
    *o_i = h - Ni*( (*o_j) + Nj * (*o_k));

    if(1 == if_onebased_array_use1){
        ++(*o_k);
        ++(*o_j);
        ++(*o_i);
    }
}

//===== 2D matrices ===========================================================
unsigned long nbb_byte_sizeof2(nbb_matrix2_t in_m)
{
    unsigned long sz = sizeof(nbb_matrix2_t);
    sz += (unsigned long)in_m.sz_cols * in_m.sz_rows * sizeof(BLAS_REAL_TYPE);
    return sz;
}

BLAS_REAL_TYPE  nbb_get2(nbb_matrix2_t in_m, BLAS_UINT_TYPE in_row, BLAS_UINT_TYPE in_col)
{
    //Column major
    return in_m.m[in_row + in_m.sz_rows * in_col];
}
BLAS_REAL_TYPE * nbb_at2(nbb_matrix2_t io_m, BLAS_UINT_TYPE in_row, BLAS_UINT_TYPE in_col) //Returns address to value
{
#   ifdef NEK_CHECKS_BLAS3
        if( ! io_m.m) return 0;
        if( in_row >= io_m.sz_rows || in_col >= io_m.sz_cols) return 0;
#   endif
    //Column major
    return io_m.m + (in_row + io_m.sz_rows * in_col);
}

int nbb_transpose2(nbb_matrix2_t *o_a, nbb_matrix2_t in_b)
{
#   ifdef NEK_CHECKS_BLAS3
        if( ! o_a) return __LINE__;
        if( ! in_b.m || !o_a->m ) return __LINE__;
        if(in_b.sz_cols * in_b.sz_rows == 0) return __LINE__;
        if( o_a->sz_cols * o_a->sz_rows < in_b.sz_cols * in_b.sz_rows) return __LINE__;
#   endif

    o_a->sz_cols = in_b.sz_rows;
    o_a->sz_rows = in_b.sz_cols;

    BLAS_UINT_TYPE i,j;

    for(i=0; i<in_b.sz_rows; ++i){
        for(j=0; j<in_b.sz_cols; ++j){
            BLAS_REAL_TYPE * p = nbb_at2(*o_a,j,i);
#           ifdef NEK_CHECKS_BLAS3
                if(!p) return __LINE__;
#           endif
            *p = nbb_get2(in_b,i,j);
        }
    }

    return 0;
}

int nbb_mxm2(nbb_matrix2_t in_a, nbb_matrix2_t in_b, nbb_matrix2_t * o_c) //Calculate C= A*B
{
#   ifdef NEK_CHECKS_BLAS3
        if( ! o_c) return __LINE__;
        if( ! in_a.m || ! in_b.m || !o_c->m ) return __LINE__;
        if(in_a.sz_cols * in_a.sz_rows == 0 || in_b.sz_cols * in_b.sz_rows == 0) return __LINE__;
        if( in_a.sz_cols !=  in_b.sz_rows ) return __LINE__;
        if( o_c->sz_cols * o_c->sz_rows < in_a.sz_rows * in_b.sz_cols ) return __LINE__;
#   endif

    o_c->sz_rows = in_a.sz_rows;
    o_c->sz_cols = in_b.sz_cols;

    BLAS_UINT_TYPE i;
    for(i=0; i<in_a.sz_rows; ++i){
        BLAS_UINT_TYPE j;
        for(j=0; j<in_b.sz_cols; ++j){
            BLAS_UINT_TYPE k;
            BLAS_REAL_TYPE sum = 0;
            for(k=0; k<in_a.sz_cols; ++k){
                sum += nbb_get2(in_a,i,k) * nbb_get2(in_b,k,j);
            }
            BLAS_REAL_TYPE * p = nbb_at2(*o_c,i,j);
#           ifdef NEK_CHECKS_BLAS3
                if(!p) return __LINE__;
#           endif
            *p = sum;
        }
    }

    return 0;
}

//===== 3D matrices ===========================================================
unsigned long nbb_byte_sizeof3(nbb_matrix3_t in_m)
{
    unsigned long sz = sizeof(nbb_matrix3_t);
    sz += (unsigned long)in_m.sz_cols * in_m.sz_rows * in_m.sz_depth * sizeof(BLAS_REAL_TYPE);
    return sz;
}

BLAS_REAL_TYPE  nbb_get3(nbb_matrix3_t in_m, BLAS_UINT_TYPE in_row, BLAS_UINT_TYPE in_col, BLAS_UINT_TYPE in_depth)
{
    //Column major
    return in_m.m[in_row + in_m.sz_rows * in_col + in_m.sz_rows * in_m.sz_cols * in_depth];
}

BLAS_REAL_TYPE * nbb_at3(nbb_matrix3_t io_m, BLAS_UINT_TYPE in_row, BLAS_UINT_TYPE in_col, BLAS_UINT_TYPE in_depth)
{
    //Column major
#   ifdef NEK_CHECKS_BLAS3
        if( ! io_m.m) return 0;
        if( in_row >= io_m.sz_rows || in_col >= io_m.sz_cols || in_depth >= io_m.sz_depth) return 0;
#   endif
    //Column major
    return io_m.m + (in_row + io_m.sz_rows * in_col + io_m.sz_rows * io_m.sz_cols * in_depth);
}

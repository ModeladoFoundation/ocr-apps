#ifndef NEKBONE_BLAS1_H
#include "blas1.h"
#endif

unsigned long nbb_byte_sizeofiv(nbb_ivector_t in_v)
{
    unsigned long sz = sizeof(nbb_ivector_t);
    sz += in_v.length * sizeof(BLAS_REAL_TYPE);
    return sz;
}

BLAS_UINT_TYPE nbb_getiv(nbb_ivector_t in_v, BLAS_UINT_TYPE in_offset)
{
#   ifdef NEK_CHECKS_BLAS1
        if( ! in_v.v) return ((BLAS_UINT_TYPE)-1);
        if( in_offset >= in_v.length ) return ((BLAS_UINT_TYPE)-1);
#   endif
    return in_v.v[in_offset];
}

BLAS_UINT_TYPE * nbb_ativ(nbb_ivector_t io_v, BLAS_UINT_TYPE in_offset)
{
#   ifdef NEK_CHECKS_BLAS1
        if( ! io_v.v) return 0;
        if( in_offset >= io_v.length ) return 0;
#   endif
    return io_v.v + in_offset;
}

unsigned long nbb_byte_sizeofrv(nbb_rvector_t in_v)
{
    unsigned long sz = sizeof(nbb_rvector_t);
    sz += in_v.length * sizeof(BLAS_REAL_TYPE);
    return sz;
}

BLAS_REAL_TYPE  nbb_getrv(nbb_rvector_t in_v, BLAS_UINT_TYPE in_offset)
{
#   ifdef NEK_CHECKS_BLAS1
        if( ! in_v.v) return -1e38;
        if( in_offset >= in_v.length ) return -1e38;
#   endif
    return in_v.v[in_offset];
}

BLAS_REAL_TYPE * nbb_atrv(nbb_rvector_t io_v, BLAS_UINT_TYPE in_offset)
{
#   ifdef NEK_CHECKS_BLAS1
        if( ! io_v.v) return 0;
        if( in_offset >= io_v.length ) return 0;
#   endif
    return io_v.v + in_offset;
}

int nbb_rzero(nbb_rvector_t io_v)
{
#   ifdef NEK_CHECKS_BLAS1
        if(!io_v.v) return __LINE__;
        if(io_v.length==0) return __LINE__;
#   endif
    BLAS_UINT_TYPE i;
    for(i=0; i<io_v.length; ++i){
        io_v.v[i] = 0;
    }
    return 0;
}

int nbb_rzeroi(nbb_ivector_t io_a, BLAS_UINT_TYPE in_start, BLAS_UINT_TYPE in_last)
{
#   ifdef NEK_CHECKS_BLAS1
        if( ! io_a.v) return __LINE__;
        if( in_start >= io_a.length || in_last >= io_a.length) return __LINE__;
        if(in_start > in_last) return __LINE__;
#   endif
    BLAS_UINT_TYPE i;
    for(i=in_start; i<=in_last; ++i){
        io_a.v[i] = 0;
    }
    return 0;
}

int nbb_rone(nbb_rvector_t io_v)
{
#   ifdef NEK_CHECKS_BLAS1
        if( ! io_v.v) return __LINE__;
        if( io_v.length == 0) return __LINE__;
#   endif
    BLAS_UINT_TYPE i;
    for(i=0; i<io_v.length; ++i){
        io_v.v[i] = 1;
    }
    return 0;
}

int nbb_copy(nbb_rvector_t io_a, nbb_rvector_t in_b)
{
#   ifdef NEK_CHECKS_BLAS1
        if( !io_a.v || !in_b.v) return __LINE__;
        if(io_a.length==0 || in_b.length > io_a.length) return __LINE__;
#   endif
    BLAS_UINT_TYPE i;
    for(i=0; i<in_b.length; ++i){
        *nbb_atrv(io_a,i) = nbb_getrv(in_b,i);
    }
    return 0;
}

int nbb_copyi(nbb_rvector_t io_a, nbb_rvector_t in_b,
               BLAS_UINT_TYPE in_start, BLAS_UINT_TYPE in_last)
{
#   ifdef NEK_CHECKS_BLAS1
        if( !io_a.v || !in_b.v) return __LINE__;
        if(io_a.length==0 || in_b.length > io_a.length) return __LINE__;
        if(in_last >= io_a.length || in_start >= io_a.length) return __LINE__;
#   endif
    BLAS_UINT_TYPE i;
    for(i=in_start; i<=in_last; ++i){
        *nbb_atrv(io_a,i) = nbb_getrv(in_b,i);
    }
    return 0;
}

int nbb_add2(nbb_rvector_t io_a, nbb_rvector_t in_b)
{
#   ifdef NEK_CHECKS_BLAS1
        if( !io_a.v || !in_b.v) return __LINE__;
        if(io_a.length==0 || in_b.length > io_a.length) return __LINE__;
#   endif

    BLAS_UINT_TYPE i;
    for(i=0; i<in_b.length; ++i){
        *nbb_atrv(io_a,i) += nbb_getrv(in_b,i);
    }
    return 0;
}

int nbb_col2(nbb_rvector_t io_a, nbb_rvector_t in_b)
{
#   ifdef NEK_CHECKS_BLAS1
        if( !io_a.v || !in_b.v) return __LINE__;
        if(io_a.length==0 || in_b.length > io_a.length) return __LINE__;
#   endif

    BLAS_UINT_TYPE i;
    for(i=0; i<in_b.length; ++i){
        *nbb_atrv(io_a,i) *= nbb_getrv(in_b,i);
    }
    return 0;
}

int nbb_add2s1i(nbb_rvector_t io_a, nbb_rvector_t in_b, BLAS_REAL_TYPE in_c1,
                 BLAS_UINT_TYPE in_start, BLAS_UINT_TYPE in_last)
{
#   ifdef NEK_CHECKS_BLAS1
        if( !io_a.v || !in_b.v) return __LINE__;
        if(io_a.length==0 || in_b.length > io_a.length) return __LINE__;
        if(in_last >= io_a.length || in_start >= io_a.length) return __LINE__;
#   endif
    BLAS_UINT_TYPE i;
    for(i=in_start; i<=in_last; ++i){
        BLAS_REAL_TYPE t = nbb_getrv(io_a,i);
        *nbb_atrv(io_a,i) = in_c1 * t + nbb_getrv(in_b,i);
    }
    return 0;
}







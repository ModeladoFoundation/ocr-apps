
//#ifdef SIMD

#define SIMD_WIDTH  4

#define _MM_AND(A,B)            _mm_and_si128(A,B)
#define _MM_NOT(A)              _mm_xor_si128(A,xmm_all_ones)
#define _MM_STORE(A, B)         _mm_store_si128((__m128i *)(A), B)
#define _MM_STOREU(A, B)        _mm_storeu_si128((__m128i *)(A), B)
#define _MM_LOAD(A)            _mm_load_si128((__m128i *)(A))
#define _MM_LOADU(A)            _mm_loadu_si128((__m128i *)(A))
#define _MM_GATHER(A, B, X)     { A[0] = X[B[0]]; A[1] = X[B[1]]; A[2] = X[B[2]]; A[3] = X[B[3]]; }
#define _MM_SET1(val)           _mm_set1_epi32(val)
#define _MM_CMP_EQ(A,B)         _mm_cmpeq_epi32(A,B)
#define _MM_CMP_LT(A,B)         _mm_cmplt_epi32(A,B)
#define _MM_CMP_GT(A,B)         _mm_cmpgt_epi32(A,B)
#define _MM_SCATTER(A, val, X)     { X[A[0]] = val; X[A[1]] = val; X[A[2]] = val; X[A[3]] = val; }
#define _MM_SHUFFLE_EPI8(A, mask)   _mm_shuffle_epi8(A, mask)

#define ASSEMBLE_TO_REGISTERS(val0, val1, val2, val3, Trep, xmm_reg) { Trep[0] = val0; Trep[1] = val1; Trep[2] = val2; Trep[3] = val3; xmm_reg = _MM_LOAD(Trep); }


__inline __m128 _mm_gather_ps(float* base, __m128i regidx)
{
    __m128 output, x1, x2, x3;
    unsigned __int64 index;
    unsigned long idx0,idx2;
    unsigned long long idx1,idx3;

    index=_mm_cvtsi128_si64(regidx);
    idx0 = index;
    idx1 = index>>32; 
    
    index=_mm_cvtsi128_si64(_mm_castps_si128(_mm_movehl_ps(_mm_castsi128_ps(regidx),_mm_castsi128_ps(regidx))));
    idx2 = index;
    idx3 = index>>32; 

    output = _mm_load_ss(&base[idx0]);
    x1 = _mm_load_ss(&base[idx1]);
    x2 = _mm_load_ss(&base[idx2]);
    x3 = _mm_load_ss(&base[idx3]);
    
   output = _mm_unpacklo_ps(output, x2);
   x2 = _mm_unpacklo_ps(x1, x3);
   output = _mm_unpacklo_ps(output, x2);
   return (output);
}


__inline __m128i _mm_gather_epi32(float* base, __m128i regidx)
{
    __m128 output, x1, x2, x3;
    unsigned __int64 index;
    unsigned int idx0,idx2;
    unsigned int idx1,idx3;
                                                                                                                                                                         
    index = _mm_cvtsi128_si64(regidx);
    idx0 =  index;
    idx1 =  index>>32;
                                                                                                                                                                                     
    index = _mm_cvtsi128_si64(_mm_castps_si128(_mm_movehl_ps(_mm_castsi128_ps(regidx),_mm_castsi128_ps(regidx))));
    idx2 =  index;
    idx3 =  index>>32;
                                                                                                                                                                                                 
    output = _mm_load_ss(&base[idx0]);
    x1 = _mm_load_ss(&base[idx1]);
    x2 = _mm_load_ss(&base[idx2]);
    x3 = _mm_load_ss(&base[idx3]);
    
   output = _mm_unpacklo_ps(output, x2);
   x2 = _mm_unpacklo_ps(x1, x3);
   output = _mm_unpacklo_ps(output, x2);
   return (_mm_castps_si128(output));
}


__inline __m128i _mm_gather_epi32_naive(int * base, int idx0, int idx1, int idx2, int idx3)
{
    __declspec(align(64)) int Trep[4];
    Trep[0] = base[idx0];
    Trep[1] = base[idx1];
    Trep[2] = base[idx2];
    Trep[3] = base[idx3];
    return ( _mm_load_si128 ((__m128i *)Trep));
}




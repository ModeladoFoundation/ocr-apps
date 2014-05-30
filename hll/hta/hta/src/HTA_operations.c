#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "HTA.h"
#include "Debug.h"
#include "Tuple.h"
#include "CSR.h"
#include "HTA_operations.h"
#include "util.h"

#define REDUCE_SUM_CASE(type) \
{ \
    type *r = (type*) result; \
    type d = *((type*) buf); \
    *r += d; \
}

void REDUCE_SUM(HTA_SCALAR_TYPE stype, void* result, void* buf) {
    ASSERT(result && buf);

    switch(stype)
    {
        case(HTA_SCALAR_TYPE_INT32):
            REDUCE_SUM_CASE(int32_t)
        break;
        case(HTA_SCALAR_TYPE_INT64):
            REDUCE_SUM_CASE(int64_t)
        break;
        case(HTA_SCALAR_TYPE_UINT32):
            REDUCE_SUM_CASE(uint32_t)
        break;
        case(HTA_SCALAR_TYPE_UINT64):
            REDUCE_SUM_CASE(uint64_t)
        break;
        case(HTA_SCALAR_TYPE_FLOAT):
            REDUCE_SUM_CASE(float)
        break;
        case(HTA_SCALAR_TYPE_DOUBLE):
            REDUCE_SUM_CASE(double)
        break;
        case(HTA_SCALAR_TYPE_DCOMPLEX):
            { 
                dcomplex *r = (dcomplex*) result; 
                dcomplex d = *((dcomplex*) buf); 
                *r = dcmplx_add(*r, d); 
            }
        break;
        default:
            ASSERT(0 && "Unimplemented");
    }
}

#define REDUCE_PRODUCT_CASE(type) \
{ \
    type *r = (type*) result; \
    type d = *((type*) buf); \
    *r *= d; \
}

void REDUCE_PRODUCT(HTA_SCALAR_TYPE stype, void* result, void* buf) {
    ASSERT(result && buf);

    switch(stype)
    {
        case(HTA_SCALAR_TYPE_INT32):
            REDUCE_PRODUCT_CASE(int32_t)
        break;
        case(HTA_SCALAR_TYPE_INT64):
            REDUCE_PRODUCT_CASE(int64_t)
        break;
        case(HTA_SCALAR_TYPE_UINT32):
            REDUCE_PRODUCT_CASE(uint32_t)
        break;
        case(HTA_SCALAR_TYPE_UINT64):
            REDUCE_PRODUCT_CASE(uint64_t)
        break;
        case(HTA_SCALAR_TYPE_FLOAT):
            REDUCE_PRODUCT_CASE(float)
        break;
        case(HTA_SCALAR_TYPE_DOUBLE):
            REDUCE_PRODUCT_CASE(double)
        break;
        default:
            ASSERT(0 && "Unimplemented");
    }
}

#define REDUCE_MAX_CASE(type) \
{ \
    type *r = (type*) result; \
    type d = *((type*) buf); \
    *r = max(*r, d); \
}

void REDUCE_MAX(HTA_SCALAR_TYPE stype, void* result, void* buf) {
    ASSERT(result && buf);

    switch(stype)
    {
        case(HTA_SCALAR_TYPE_INT32):
            REDUCE_MAX_CASE(int32_t)
        break;
        case(HTA_SCALAR_TYPE_INT64):
            REDUCE_MAX_CASE(int64_t)
        break;
        case(HTA_SCALAR_TYPE_UINT32):
            REDUCE_MAX_CASE(uint32_t)
        break;
        case(HTA_SCALAR_TYPE_UINT64):
            REDUCE_MAX_CASE(uint64_t)
        break;
        case(HTA_SCALAR_TYPE_FLOAT):
            REDUCE_MAX_CASE(float)
        break;
        case(HTA_SCALAR_TYPE_DOUBLE):
            REDUCE_MAX_CASE(double)
        break;
        default:
            ASSERT(0 && "Unimplemented");
    }
}
/// Increment all scalar elements
#define H1_INC_CASE(type) \
    { \
        type *base = (type*) HTA_get_ptr_raw_data(dest); \
        for(int i = 0; i < num_elems; i++) \
            base[i]++; \
    }

void H1_INC(HTA * dest)
{
    ASSERT(dest->type == HTA_TYPE_DENSE);
    ASSERT(dest->height == 1);

    int num_elems = dest->leaf.num_elem;

    switch (dest->scalar_type) 
    {
        case(HTA_SCALAR_TYPE_INT32):
            H1_INC_CASE(int32_t)
        break;
        case(HTA_SCALAR_TYPE_INT64):
            H1_INC_CASE(int64_t)
        break;
        case(HTA_SCALAR_TYPE_UINT32):
            H1_INC_CASE(uint32_t)
        break;
        case(HTA_SCALAR_TYPE_UINT64):
            H1_INC_CASE(uint64_t)
        break;
        default:
            ASSERT(0 && "Unimplemented");
    }
}

/// Increment all scalar elements
#define H2_INC_CASE(type) \
    { \
        type *dest_base = (type*) HTA_get_ptr_raw_data(dest); \
        type *src_base = (type*) HTA_get_ptr_raw_data(src); \
        for(int i = 0; i < num_elems; i++) \
            dest_base[i] = src_base[i] + 1; \
    }

void H2_INC(HTA * dest, HTA * src)
{
    ASSERT(dest->type == HTA_TYPE_DENSE
            && src->type == HTA_TYPE_DENSE);
    ASSERT(dest->height == 1 && src->height == 1);
    int num_elems = dest->leaf.num_elem;

    switch (dest->scalar_type) 
    {
        case(HTA_SCALAR_TYPE_INT32):
            H2_INC_CASE(int32_t)
        break;
        case(HTA_SCALAR_TYPE_INT64):
            H2_INC_CASE(int64_t)
        break;
        case(HTA_SCALAR_TYPE_UINT32):
            H2_INC_CASE(uint32_t)
        break;
        case(HTA_SCALAR_TYPE_UINT64):
            H2_INC_CASE(uint64_t)
        break;
        default:
            ASSERT(0 && "Unimplemented");
    }
}

#define H2_COPY_CASE(type) \
    { \
        type *dest_base = (type*) HTA_get_ptr_raw_data(dest); \
        type *src_base = (type*) HTA_get_ptr_raw_data(src); \
        memcpy(dest_base, src_base, num_elems * sz); \
    }

void H2_COPY(HTA * dest, HTA * src)
{
    ASSERT(dest->type == HTA_TYPE_DENSE
            && src->type == HTA_TYPE_DENSE);
    ASSERT(dest->height == 1 && src->height == 1);
    int num_elems = dest->leaf.num_elem;
    size_t sz = HTA_get_scalar_size(dest);

    switch (dest->scalar_type) 
    {
        case(HTA_SCALAR_TYPE_INT32):
            H2_COPY_CASE(int32_t)
        break;
        case(HTA_SCALAR_TYPE_INT64):
            H2_COPY_CASE(int64_t)
        break;
        case(HTA_SCALAR_TYPE_UINT32):
            H2_COPY_CASE(uint32_t)
        break;
        case(HTA_SCALAR_TYPE_UINT64):
            H2_COPY_CASE(uint64_t)
        break;
        case(HTA_SCALAR_TYPE_FLOAT):
            H2_COPY_CASE(float)
        break;
        case(HTA_SCALAR_TYPE_DOUBLE):
            H2_COPY_CASE(double)
        break;
        default:
            ASSERT(0 && "Unimplemented");
    }
}

#define H2_ABS_CASE(type) \
    { \
        type *dest_base = (type*) HTA_get_ptr_raw_data(dest); \
        type *src_base = (type*) HTA_get_ptr_raw_data(src); \
        for(int i = 0; i < num_elems; i++) \
            dest_base[i] = abs(src_base[i]); \
    }
#define H2_FABS_CASE(type) \
    { \
        type *dest_base = (type*) HTA_get_ptr_raw_data(dest); \
        type *src_base = (type*) HTA_get_ptr_raw_data(src); \
        for(int i = 0; i < num_elems; i++) \
            dest_base[i] = fabs(src_base[i]); \
    }

void H2_ABS(HTA * dest, HTA * src)
{
    ASSERT(dest->type == HTA_TYPE_DENSE
            && src->type == HTA_TYPE_DENSE);
    ASSERT(dest->height == 1 && src->height == 1);
    int num_elems = dest->leaf.num_elem;

    switch (dest->scalar_type) 
    {
        case(HTA_SCALAR_TYPE_INT32):
            H2_ABS_CASE(int32_t)
        break;
        case(HTA_SCALAR_TYPE_INT64):
            H2_ABS_CASE(int64_t)
        break;
        case(HTA_SCALAR_TYPE_UINT32):
            H2_ABS_CASE(uint32_t)
        break;
        case(HTA_SCALAR_TYPE_UINT64):
            H2_ABS_CASE(uint64_t)
        break;
        case(HTA_SCALAR_TYPE_FLOAT):
            H2_FABS_CASE(float)
        break;
        case(HTA_SCALAR_TYPE_DOUBLE):
            H2_FABS_CASE(double)
        break;
        default:
            ASSERT(0 && "Unimplemented");
    }
}

#define H2S1_POW_CASE(type) \
    { \
        type *dest_base = (type*) HTA_get_ptr_raw_data(dest); \
        type *src_base = (type*) HTA_get_ptr_raw_data(src); \
        for(int i = 0; i < num_elems; i++)  \
            dest_base[i] = pow(src_base[i], *(type*)s); \
    }

void H2S1_POW(HTA * dest, HTA * src, void* s)
{
    ASSERT(dest->type == HTA_TYPE_DENSE
            && src->type == HTA_TYPE_DENSE);
    ASSERT(dest->height == 1 && src->height == 1);
    int num_elems = dest->leaf.num_elem;

    switch (dest->scalar_type) 
    {
        case(HTA_SCALAR_TYPE_FLOAT):
            H2S1_POW_CASE(float)
        break;
        case(HTA_SCALAR_TYPE_DOUBLE):
            H2S1_POW_CASE(double)
        break;
        default:
            ASSERT(0 && "Unimplemented");
    }
}

#define H3_PWADD_CASE(type) \
{ \
    type* dest_ptr = (type*) HTA_get_ptr_raw_data(dest); \
    type* src1_ptr = (type*) HTA_get_ptr_raw_data(src1); \
    type* src2_ptr = (type*) HTA_get_ptr_raw_data(src2); \
    for(int i = 0; i < num_elems; i++) \
    { \
        dest_ptr[i] = src1_ptr[i] + src2_ptr[i]; \
    } \
}

void H3_PWADD(HTA * dest, HTA * src1, HTA * src2)
{
    ASSERT(dest->type == HTA_TYPE_DENSE
            && src1->type == HTA_TYPE_DENSE
            && src2->type == HTA_TYPE_DENSE);
    ASSERT(dest->height == 1 && src1->height == 1 && src2->height == 1);

    int num_elems = dest->leaf.num_elem;
    switch (dest->scalar_type) 
    {
        case(HTA_SCALAR_TYPE_INT32):
            H3_PWADD_CASE(int32_t)
        break;
        case(HTA_SCALAR_TYPE_INT64):
            H3_PWADD_CASE(int64_t)
        break;
        case(HTA_SCALAR_TYPE_UINT32):
            H3_PWADD_CASE(uint32_t)
        break;
        case(HTA_SCALAR_TYPE_UINT64):
            H3_PWADD_CASE(uint64_t)
        break;
        case(HTA_SCALAR_TYPE_FLOAT):
            H3_PWADD_CASE(float)
        break;
        case(HTA_SCALAR_TYPE_DOUBLE):
            H3_PWADD_CASE(double)
        break;
        default:
            ASSERT(0 && "Unimplemented");
    }
}

#define H3_PWSUB_CASE(type) \
{ \
    type* dest_ptr = (type*) HTA_get_ptr_raw_data(dest); \
    type* src1_ptr = (type*) HTA_get_ptr_raw_data(src1); \
    type* src2_ptr = (type*) HTA_get_ptr_raw_data(src2); \
    for(int i = 0; i < num_elems; i++) \
    { \
        dest_ptr[i] = src1_ptr[i] - src2_ptr[i]; \
    } \
}

void H3_PWSUB(HTA * dest, HTA * src1, HTA * src2)
{
    ASSERT(dest->type == HTA_TYPE_DENSE
            && src1->type == HTA_TYPE_DENSE
            && src2->type == HTA_TYPE_DENSE);
    ASSERT(dest->height == 1 && src1->height == 1 && src2->height == 1);

    int num_elems = dest->leaf.num_elem;
    switch (dest->scalar_type) 
    {
        case(HTA_SCALAR_TYPE_INT32):
            H3_PWSUB_CASE(int32_t)
        break;
        case(HTA_SCALAR_TYPE_INT64):
            H3_PWSUB_CASE(int64_t)
        break;
        case(HTA_SCALAR_TYPE_UINT32):
            H3_PWSUB_CASE(uint32_t)
        break;
        case(HTA_SCALAR_TYPE_UINT64):
            H3_PWSUB_CASE(uint64_t)
        break;
        case(HTA_SCALAR_TYPE_FLOAT):
            H3_PWSUB_CASE(float)
        break;
        case(HTA_SCALAR_TYPE_DOUBLE):
            H3_PWSUB_CASE(double)
        break;
        default:
            ASSERT(0 && "Unimplemented");
    }
}

#define H3_PWMUL_CASE(type) \
{ \
    type* dest_ptr = (type*) HTA_get_ptr_raw_data(dest); \
    type* src1_ptr = (type*) HTA_get_ptr_raw_data(src1); \
    type* src2_ptr = (type*) HTA_get_ptr_raw_data(src2); \
    for(int i = 0; i < num_elems; i++) \
    { \
        dest_ptr[i] = src1_ptr[i] * src2_ptr[i]; \
    } \
}

void H3_PWMUL(HTA * dest, HTA * src1, HTA * src2)
{
    ASSERT(dest->type == HTA_TYPE_DENSE
            && src1->type == HTA_TYPE_DENSE
            && src2->type == HTA_TYPE_DENSE);
    ASSERT(dest->height == 1 && src1->height == 1 && src2->height == 1);

    int num_elems = dest->leaf.num_elem;
    switch (dest->scalar_type) 
    {
        case(HTA_SCALAR_TYPE_INT32):
            H3_PWMUL_CASE(int32_t)
        break;
        case(HTA_SCALAR_TYPE_INT64):
            H3_PWMUL_CASE(int64_t)
        break;
        case(HTA_SCALAR_TYPE_UINT32):
            H3_PWMUL_CASE(uint32_t)
        break;
        case(HTA_SCALAR_TYPE_UINT64):
            H3_PWMUL_CASE(uint64_t)
        break;
        case(HTA_SCALAR_TYPE_FLOAT):
            H3_PWMUL_CASE(float)
        break;
        case(HTA_SCALAR_TYPE_DOUBLE):
            H3_PWMUL_CASE(double)
        break;
        default:
            ASSERT(0 && "Unimplemented");
    }
}

#define H3_MATMUL_CASE(type) \
{ \
    type* dest_ptr = (type*) HTA_get_ptr_raw_data(dest); \
    type* src1_ptr = (type*) HTA_get_ptr_raw_data(src1); \
    type* src2_ptr = (type*) HTA_get_ptr_raw_data(src2); \
    for(int i = 0; i < M; i++) \
        for(int j = 0; j < O; j++) \
        { \
            type sum = 0; \
            for(int k = 0; k < N; k++) \
            { \
                sum += src1_ptr[i * N + k] * src2_ptr[k * N + j]; \
            } \
            dest_ptr[i * N + j] = sum; \
        } \
}

// sequentially matrix multiply src tiles and store the result to dest tile
void H3_MATMUL(HTA * dest, HTA * src1, HTA * src2)
{
    ASSERT(dest->type == HTA_TYPE_DENSE
            && src1->type == HTA_TYPE_DENSE
            && src2->type == HTA_TYPE_DENSE);
    ASSERT(dest->height == 1 && src1->height == 1 && src2->height == 1);
    ASSERT(src1->flat_size.values[1] == src2->flat_size.values[0]);

    int M = src1->flat_size.values[0];
    int N = src1->flat_size.values[1];
    int O = src2->flat_size.values[1];
    switch (dest->scalar_type) 
    {
        case(HTA_SCALAR_TYPE_INT32):
            H3_MATMUL_CASE(int32_t)
        break;
        case(HTA_SCALAR_TYPE_INT64):
            H3_MATMUL_CASE(int64_t)
        break;
        case(HTA_SCALAR_TYPE_UINT32):
            H3_MATMUL_CASE(uint32_t)
        break;
        case(HTA_SCALAR_TYPE_UINT64):
            H3_MATMUL_CASE(uint64_t)
        break;
        case(HTA_SCALAR_TYPE_FLOAT):
            H3_MATMUL_CASE(float)
        break;
        case(HTA_SCALAR_TYPE_DOUBLE):
            H3_MATMUL_CASE(double)
        break;
        default:
            ASSERT(0 && "Unimplemented");
    }
}

#define H1S1_MUL_CASE(type) \
{ \
    type* h_base = (type*) HTA_get_ptr_raw_data(h); \
    type s = *((type*) scalar); \
    for(int i = 0; i < num_elems; i++) \
    { \
        h_base[i] *= s; \
    } \
}
void H1S1_MUL(HTA * h, void* scalar) {
    ASSERT(h->type == HTA_TYPE_DENSE && scalar); // TODO: add support for SPARSE HTA
    ASSERT(h->height == 1);
   
    int num_elems = h->leaf.num_elem;
    switch (h->scalar_type) 
    {
        case(HTA_SCALAR_TYPE_INT32):
            H1S1_MUL_CASE(int32_t)
        break;
        case(HTA_SCALAR_TYPE_INT64):
            H1S1_MUL_CASE(int64_t)
        break;
        case(HTA_SCALAR_TYPE_UINT32):
            H1S1_MUL_CASE(uint32_t)
        break;
        case(HTA_SCALAR_TYPE_UINT64):
            H1S1_MUL_CASE(uint64_t)
        break;
        case(HTA_SCALAR_TYPE_FLOAT):
            H1S1_MUL_CASE(float)
        break;
        case(HTA_SCALAR_TYPE_DOUBLE):
            H1S1_MUL_CASE(double)
        break;
        default:
            ASSERT(0 && "Unimplemented");
    }
}

#define H1S1_INIT_CASE(type) \
{ \
    type* h_base = (type*) HTA_get_ptr_raw_data(h); \
    type s = *((type*) scalar); \
    for(int i = 0; i < num_elems; i++) \
    { \
        h_base[i] = s; \
    } \
}

void H1S1_INIT(HTA * h, void* scalar) {
    ASSERT(h->type == HTA_TYPE_DENSE && scalar); // TODO: add support for SPARSE HTA
    ASSERT(h->height == 1);
   
    int num_elems = h->leaf.num_elem;
    switch (h->scalar_type) 
    {
        case(HTA_SCALAR_TYPE_INT32):
            H1S1_INIT_CASE(int32_t)
        break;
        case(HTA_SCALAR_TYPE_INT64):
            H1S1_INIT_CASE(int64_t)
        break;
        case(HTA_SCALAR_TYPE_UINT32):
            H1S1_INIT_CASE(uint32_t)
        break;
        case(HTA_SCALAR_TYPE_UINT64):
            H1S1_INIT_CASE(uint64_t)
        break;
        case(HTA_SCALAR_TYPE_FLOAT):
            H1S1_INIT_CASE(float)
        break;
        case(HTA_SCALAR_TYPE_DOUBLE):
            H1S1_INIT_CASE(double)
        break;
        case(HTA_SCALAR_TYPE_DCOMPLEX):
            H1S1_INIT_CASE(dcomplex)
        break;
        default:
            ASSERT(0 && "Unimplemented");
    }
}

#define H3_SDPWMUL_CASE(type) \
{ \
    type *hd_val = (type*) HTA_get_ptr_raw_data(hd); \
    type *hr_val = (type*) HTA_get_ptr_raw_data(hr); \
    void *raw = HTA_get_ptr_raw_data(hs); \
    type *val_ptr = (type*) raw; \
    int* col_ind = (int*)(raw + nnz*sz); \
    int* row_ptr = (int*)(raw + nnz*sz + nnz*sizeof(int)); \
    for(int i = 0; i < num_rows; i++) \
    { \
        for(int j = row_ptr[i]; j < row_ptr[i+1]; j++) \
        { \
            int col = col_ind[j]; \
            hr_val[i*num_cols + col] = val_ptr[j] * hd_val[i*num_cols + col]; \
        } \
    } \
}

/// The operation takes three leaf HTA tiles and perform pointwise multiplication
/// @param hr The resulting dense matrix tile
/// @param hs The source sparse matrix tile
/// @param hd The source dense matrix tile
void H3_SDPWMUL(HTA * hr, HTA * hs, HTA * hd)
{
    ASSERT(hr->type == HTA_TYPE_DENSE
            && hs->type == HTA_TYPE_SPARSE
            && hd->type == HTA_TYPE_DENSE);
    ASSERT(hr->height == 1 && hs->height == 1 && hd->height == 1);
    ASSERT(hr->scalar_type == hs->scalar_type && hs->scalar_type == hd->scalar_type);

    Leaf* leaf = &hs->leaf;
    int nnz = leaf->num_elem;
    
    if(nnz == 0) // all zero tile
        return; 

    size_t sz = HTA_get_scalar_size(hs);
    int num_rows = hd->flat_size.values[0];
    int num_cols = hd->flat_size.values[1];
    switch (hr->scalar_type) 
    {
        case(HTA_SCALAR_TYPE_INT32):
            H3_SDPWMUL_CASE(int32_t)
        break;
        case(HTA_SCALAR_TYPE_INT64):
            H3_SDPWMUL_CASE(int64_t)
        break;
        case(HTA_SCALAR_TYPE_UINT32):
            H3_SDPWMUL_CASE(uint32_t)
        break;
        case(HTA_SCALAR_TYPE_UINT64):
            H3_SDPWMUL_CASE(uint64_t)
        break;
        case(HTA_SCALAR_TYPE_FLOAT):
            H3_SDPWMUL_CASE(float)
        break;
        case(HTA_SCALAR_TYPE_DOUBLE):
            H3_SDPWMUL_CASE(double)
        break;
        default:
            ASSERT(0 && "Unimplemented");
    }
}

#define H3_SDMV_CASE(type) \
{ \
    type *hd_val = (type*) HTA_get_ptr_raw_data(hd); \
    type *hr_val = (type*) HTA_get_ptr_raw_data(hr); \
    void *raw = HTA_get_ptr_raw_data(hs); \
    type *val_ptr = (type*) raw; \
    int* col_ind = (int*)(raw + nnz*sz); \
    int* row_ptr = (int*)(raw + nnz*sz + nnz*sizeof(int)); \
    for(int i = 0; i < num_rows; i++) \
    { \
        type sum = 0.0; \
        for(int j = row_ptr[i]; j < row_ptr[i+1]; j++) \
        { \
            int col = col_ind[j]; \
            sum += val_ptr[j] * hd_val[col]; \
        } \
        hr_val[i] = sum; \
    } \
}
void H3_SDMV(HTA * hr, HTA * hs, HTA * hd)
{
    ASSERT(hr->type == HTA_TYPE_DENSE
            && hs->type == HTA_TYPE_SPARSE
            && hd->type == HTA_TYPE_DENSE);
    ASSERT(hr->height == 1 && hs->height == 1 && hd->height == 1);
    ASSERT(hr->scalar_type == hs->scalar_type && hs->scalar_type == hd->scalar_type);

    Leaf* leaf = &hs->leaf;
    int nnz = leaf->num_elem;
    
    if(nnz == 0) // all zero tile
        return; 

    size_t sz = HTA_get_scalar_size(hs);
    int num_rows = hs->flat_size.values[0];

    switch (hr->scalar_type) 
    {
        case(HTA_SCALAR_TYPE_INT32):
            H3_SDMV_CASE(int32_t)
        break;
        case(HTA_SCALAR_TYPE_INT64):
            H3_SDMV_CASE(int64_t)
        break;
        case(HTA_SCALAR_TYPE_UINT32):
            H3_SDMV_CASE(uint32_t)
        break;
        case(HTA_SCALAR_TYPE_UINT64):
            H3_SDMV_CASE(uint64_t)
        break;
        case(HTA_SCALAR_TYPE_FLOAT):
            H3_SDMV_CASE(float)
        break;
        case(HTA_SCALAR_TYPE_DOUBLE):
            H3_SDMV_CASE(double)
        break;
        default:
            ASSERT(0 && "Unimplemented");
    }
}

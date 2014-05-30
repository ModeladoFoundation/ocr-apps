#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "Debug.h"
#include "Region.h"
#include "Accessor.h"
#include "Tuple.h"

//====================================================
//  Region APIs
//====================================================
//Region Region_create(int dim, ...) {
//    ASSERT(dim > 0);
//    int n = dim;
//    va_list argp;
//
//    Region r;
//    r.accessor_type = ACCESSOR_REGION;
//    r.dim = dim;
//
//    // iteratively storing numbers into the tuple
//    va_start(argp, dim);
//    while(n > 0)
//    {
//        r.ranges[dim-n] = va_arg(argp, Range);
//        n--;
//    }
//    va_end(argp);
//    return r;
//}

void Region_init(Region *r, int dim, ...) {
    ASSERT(r);
    ASSERT(dim > 0);
    int n = dim;
    va_list argp;

    r->accessor_type = ACCESSOR_REGION;
    r->dim = dim;

    // iteratively storing numbers into the tuple
    va_start(argp, dim);
    while(n > 0)
    {
        r->ranges[dim-n] = va_arg(argp, Range);
        n--;
    }
    va_end(argp);
}

//void Region_destroy(Region r) {
//    ASSERT(r);
//    ASSERT(r->ranges);
//
//    free(r->ranges);
//    free(r);
//}

unsigned int Region_cardinality(Region *r) {
    unsigned int c = 1;
    for(int i = 0; i < r->dim; i++)
        c *= r->ranges[i].card;
    return c;
}

Tuple Region_get_dimension(Region *r) {
    ASSERT(r);
    Tuple new_tuple;
    new_tuple.dim = r->dim;
    new_tuple.height = 1;
    
    for(int i = 0; i < r->dim; i++) {
        new_tuple.values[i] = r->ranges[i].card;
    }
    return new_tuple;
}

int Region_idx_to_tile_idx(Region *r, Tuple *t, int idx) {
    int indices[5];
    ASSERT(r->dim < 4 && "Only support at most 3d for now");

    int dim = r->dim;

    // Compute the region indices
    for(int j = dim - 1; j >= 0; j--) {
        indices[j] = idx % r->ranges[j].card;
        idx = (idx - indices[j]) / r->ranges[j].card;
    }
    //printf("region indices: %d %d\n", indices[0], indices[1]);

    // Map region indices to tile indices
    for(int j = dim - 1; j >= 0; j--) {
       indices[j] = Range_idx_to_linear_idx(&r->ranges[j], indices[j]); 
    }
    //printf("tile indices: %d %d\n", indices[0], indices[1]);

    // Compute the linear index within tile
    int ret = 0;
    int mul = 1;
    for(int i = dim-1; i>= 0; i--)
    {
	ret += indices[i] * mul;
        mul *= t->values[i];
    }
    return ret;
}

void Region_print(Region *r) {
    printf("Region info:");
    printf("dim: %d, ", r->dim);
    printf("cardinality: %u\n", Region_cardinality(r));
    for(int i = 0; i < r->dim; i++)
        Range_print(&r->ranges[i]);
}

//====================================================
//  Range APIs
//====================================================


void Range_print(Range *r) {
    printf("Range low: %d, high: %d, step: %d, mod: %d, cardinality:%d\n", 
            r->low, r->high, r->step, r->mod, r->card);
}

//unsigned int Range_cardinality(Range r) {
//    return (r.high - r.low + 1)/r.step;
//}

int Range_idx_to_linear_idx(Range *r, int idx) {
    return r->low + r->step * idx;
}

void Range_init(Range *r, int l, int h, int s, int m)       
{
    r->low = l;                                        
    r->high = h;                                      
    r->step= s;                                       
    r->mod = m;                                        
    r->card = (h - l)/s + 1;                     
}

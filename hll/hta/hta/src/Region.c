#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "Debug.h"
#include "Region.h"
#include "Accessor.h"
#include "Tuple.h"

#define UNDETERMINED (-1)
//====================================================
//  Region APIs
//====================================================

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

// get the cardinality of the whole region
int Region_cardinality(Region *r) {
    unsigned int c = 1;
    for(int i = 0; i < r->dim; i++) {
        if(r->ranges[i].card == UNDETERMINED)
            return UNDETERMINED;
        c *= r->ranges[i].card;
    }
    return c;
}

// get the cardinality of specified by the range of each dimension
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

// It takes a linear index within a region and convert it to linear
// index within a tile (size specified by t)
int Region_idx_to_tile_idx(Region *r, Tuple *t, int idx) {
    int indices[5];
    ASSERT(r->dim < 5 && "Only support at most 3d for now");

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

Tuple Region_idx_to_tile_nd_idx(Region *r, int idx) {
    int indices[5];
    ASSERT(r->dim < 4 && "Only support at most 3d for now");

    int dim = r->dim;
    Tuple ret;
    ret.dim = dim;

    // Compute the region indices
    for(int j = dim - 1; j >= 0; j--) {
        indices[j] = idx % r->ranges[j].card;
        idx = (idx - indices[j]) / r->ranges[j].card;
    }
    //printf("region indices: %d %d\n", indices[0], indices[1]);

    // Map region indices to tile indices
    for(int j = dim - 1; j >= 0; j--) {
       ret.values[j] = Range_idx_to_linear_idx(&r->ranges[j], indices[j]);
    }
    return ret;
}

// NOTICE: this is used only when for the "compact" region nd idx (i.e.
// the region_nd_idx is generated using the region's cardinality) and
// this is not for nd_idx generated using region iterator since those
// are nd idx with the corresponding tile
Tuple Region_nd_idx_to_tile_nd_idx(Region *r, Tuple* region_nd_idx) {
    int indices[5];
    ASSERT(r->dim < 4 && "Only support at most 3d for now");

    int dim = r->dim;
    Tuple ret;
    ret.dim = dim;
    // Map region indices to tile indices
    for(int j = dim - 1; j >= 0; j--) {
       ret.values[j] = Range_idx_to_linear_idx(&r->ranges[j], region_nd_idx->values[j]);
    }
    return ret;
}

// It converts a region selection into equivalent boolean array
// representation. The programmer has to allocate memory space for the boolean array
void Region_to_boolean_array(Region *r, Tuple *t, int* sel) {
    ASSERT(r);
    ASSERT(t);
    ASSERT(sel);

    // unset the selection array
    int num_elem = Tuple_product(t);
    for(int i = 0; i < num_elem; i++) {
        sel[i] = 0;
    }

    // set the points selected by the region
    int rc = Region_cardinality(r);
    for(int i = 0; i < rc; i++) {
        int idx = Region_idx_to_tile_idx(r, t, i);
        sel[idx] = 1;
    }
}

#if 0
// It converts a region selection into equivalent boolean array
// representation. The programmer has to allocate memory space for the boolean array
void Region_chain_to_boolean_array(int length, Region *r, Tuple *t, int* sel) {
    ASSERT(r);
    ASSERT(t);
    ASSERT(sel);
    ASSERT(length > 0);

    int num_elem = 1;      // number of total elements
    int selected_elem = 1; // number of selected elements by the region chain
    int rc[length];        // cardinality at each level
    int r_iter[length];    // the linear index of each region

    for(int l = 0; l < length; l++) {
        num_elem *= Tuple_product(t);
        ASSERT(num_elem > 0);              // overflow check

        rc[l] = Region_cardinality(&r[l]);  // get cardinality at each level
        selected_elem *= rc[l];
        ASSERT(selected_elem > 0);         // overflow check

        r_iter[l] = 0;                     // initialize r_iter
    }

    // unset the selection array
    for(int i = 0; i < num_elem; i++) {
        sel[i] = 0;
    }

    // iterate over all selected element
    Tuple t_iter[length]; // a chain of selected tile nd index
    for(int i = 0; i < selected_elem; i++) {
        for(int l = 0; l < length; l++) {
            t_iter[l] = Region_idx_to_tile_nd_idx(&r[l], r_iter[l]);
        }

        // TODO: convert t_iter to flattened linear index in order to set sel
         ASSERT(0 && "complete implementation first");
        //sel[Tuple_chain_to_linear_idx(length, t_iter, t)] = 1;

        if(i == selected_elem - 1)
            break;
        // advance r_iter
        int done_adv_iter = 0;
        int to_adv = length - 1;
        do {
            ASSERT(to_adv >= 0);
            r_iter[to_adv]++;
            if(r_iter[to_adv] == rc[to_adv]) {
                if(to_adv == 0) { // last element
                    done_adv_iter = 1;
                    ASSERT(0); // should never reach here
                }
                else {
                    r_iter[to_adv] = 0;
                    to_adv--;
                }

            } else {
                done_adv_iter = 1;
            }
        } while(!done_adv_iter);
    }

    for(int i = 0; i < rc; i++) {
        sel[Region_idx_to_tile_idx(r, t, i)] = 1;
    }
}
#endif

// For a given number of dimensions, create a region that selects all elements
// The ranges being created use relative selection and thus should be modified with HTA_set_abs_region_with_hta before actual use
Region Region_create_full_elem_selection(int dim) {
    Region r;
    r.accessor_type = ACCESSOR_REGION;
    r.dim = dim;
    for(int i = 0; i < dim; i++) {
        Range range;
        Range_init(&range, 0, -1, 1, 0);
        r.ranges[i] = range;
    }
    return r;
}


// Initialize region iterator tuple according to the spec of the region
void Region_iterator_init(Tuple *iter, Region *rgn) {
    ASSERT(iter);
    ASSERT(rgn);
    ASSERT(Region_cardinality(rgn) > 0); // Check whether the region has abs ranges

    int dim = rgn->dim;
    iter->dim = dim;
    iter->height = 1; // FIXME: for now only 1 level iterator is supported
    for(int i = 0; i < dim; i++) {
        iter->values[i] = rgn->ranges[i].low;
    }
}

// Increment iterator. The return value indicates whether the iterator
// reaches the end of the region and should always be checked
int Region_iterator_next(Tuple *iter, Region *rgn) {
    ASSERT(iter);
    ASSERT(rgn);
    ASSERT(Region_cardinality(rgn) > 0); // Check whether the region has abs ranges

    int dim = rgn->dim;
    ASSERT(iter->dim == dim);

    for(int i = dim-1; i >= 0; i--) { // start from the least insignificant dim
        Range *dim_range = &rgn->ranges[i];
        int    dim_idx   = iter->values[i];
        int le_high = (dim_idx <= dim_range->high) ? 1:0; // memorize whether dim_idx is less than or equal to high bound before increment

        dim_idx += dim_range->step;
        if(le_high && dim_idx > dim_range->high) { // was <= high but now exceeds higher bound
                                                   // this indicates higher dim must also increment
            iter->values[i] = dim_range->low;      // reset to lowest idx of this dimension
            if(i == 0) // highest dim can't increment more (iterator reaches the end)
                return 0;
        } else {
            iter->values[i] = dim_idx % dim_range->mod;  // keep going until the previous condition happens
            break; // break out of the for loop since higher dim don't need changing
        }
    }
    return 1;
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

int Range_idx_to_linear_idx(Range *r, int idx) {
    return r->low + r->step * idx;
}

void Range_init(Range *r, int l, int h, int s, int m)
{
    r->low = l;
    r->high = h;
    r->step= s;
    r->mod = m;
    r->card = (h < 0 || l < 0 || m <= 0 ) ? (UNDETERMINED) : ((h - l)/s + 1);
}

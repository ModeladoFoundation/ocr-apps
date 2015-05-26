#ifndef __REGION_H__
#define __REGION_H__
#include <stdint.h>
#include "Tuple.h"

/// range is a range of integers in the closed interval [low, end], with optional step
struct range {
    int low;   // lower bound
    int high;   // higher bound
    int step;   // step
    int mod;   // for circular shift?
    int card;
};

/// Range is passed by value
typedef struct range Range;

/// region is an n-dimensional rectangular index space spanned by n ranges.
struct region {
    /// This field is used for runtime introspection in HTA_access
    int accessor_type;
    int dim;
    Range ranges[HTA_MAX_DIM];
};

/// Region is passed by reference
typedef struct region Region;

/// Initialize a Region structure with <<dim>> ranges
void Region_init(Region *r, int dim, ...);

/// The total number of elements in a region is known as its cardinality.
int Region_cardinality(Region *r);

/// Take a linear index within a region and convert it to linear index
/// within a tile specified by the tuple t.
/// @param r Pointer to a region
/// @param t A tuple to specify the tile size
/// @param idx The linear index within the region
/// @return The converted linear index within the tile.
int Region_idx_to_tile_idx(Region *r, Tuple *t, int idx);

Tuple Region_nd_idx_to_tile_nd_idx(Region *r, Tuple* region_nd_idx);
// It converts a region selection into equivalent boolean array
// representation. The programmer has to allocate memory space for the boolean array
void Region_to_boolean_array(Region *r, Tuple *t, int* sel);

Region Region_create_full_elem_selection(int dim);

/// Create a Tuple of 1 level
Tuple Region_get_dimension(Region *r);

/// flatten is an alias of Region_get_dimension
#define Region_flatten(r) Region_get_dimension(r)

// Region iterator functions
void Region_iterator_init(Tuple *iter, Region *rgn);
int Region_iterator_next(Tuple *iter, Region *rgn);

void Region_print(Region *r);

void Range_print(Range *r);

void Range_init(Range *r, int l, int h, int s, int m);

int Range_idx_to_linear_idx(Range *r, int idx);
#endif

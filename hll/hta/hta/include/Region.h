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
    /// Cardinality 
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

//Region Region_create(int dim, ...);
void Region_init(Region *r, int dim, ...);
//void Region_destroy(Region r);

/// The total number of elements in a region is known as its cardinality.
unsigned int Region_cardinality(Region *r);

/// Take a linear index within a region and convert it to linear index
/// within a tile specified by the tuple t.
/// @param r Pointer to a region
/// @param t A tuple to specify the tile size
/// @param idx The linear index within the region
/// @return The converted linear index within the tile.
int Region_idx_to_tile_idx(Region *r, Tuple *t, int idx);

/// Create a Tuple of 1 level
Tuple Region_get_dimension(Region *r);

void Region_print(Region *r);

void Range_print(Range *r);

void Range_init(Range *r, int l, int h, int s, int m);

int Range_idx_to_linear_idx(Range *r, int idx);
#endif

#ifndef __MAPPING_H__
#define __MAPPING_H__
#include <stdint.h>
#include "Alloc.h"
#include "Tuple.h"
#include "Distribution.h"

struct mapping {
    /// Row-major, column-major or tile
    int order;              
    Dist dist;
    void** map_blocks;     // map_blocks is a data block which contains an array of REFs to all tiles allocated
    int num_blocks;     // total number of leaves    
    size_t scalar_size;
};

typedef struct mapping Mapping; 

/// Allocates data blocks for leaf tiles. The mapping information should be stored in leaf
/// tile HTAs and this Mapping should be discarded after the mapping information is recorded.
/// Thus, it's not a reference counted object.
/// @param order Row major, column major, or tile order
/// @param tiling The tiling information
/// @param dist Distribution of leaf tiles on processor nodes. Default is circular.
/// @param scalar_size The size in bytes of a scalar element
Mapping* Mapping_create(int dim, int levels, int order, const Tuple *flat_size, const Tuple* tiling, const Dist *dist, size_t scalar_size);

/// Destroy mapping information
/// @param m The mapping data structure to destroy.
void Mapping_destroy(Mapping *m);

/// Print mapping information
/// @param m The pointer to mapping data structure
void Mapping_print(Mapping *m);
#endif

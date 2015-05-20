#ifndef __MAPPING_H__
#define __MAPPING_H__
#include <stdint.h>
#include "Alloc.h"
#include "Tuple.h"
#include "Distribution.h"


enum ORDERING { ORDER_TILE = 0,
                ORDER_ROW = 1,
                ORDER_COL = 2 };

struct mapping {
    /// Row-major, column-major or tile
    int order;
    Dist dist;
    void** map_blocks;     // map_blocks is a data block which contains an array of REFs to all tiles allocated
    int num_blocks;     // total number of leaves
    size_t scalar_size;
    int* owners;
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

/// Same as Mapping_create but used in SPMD mode
Mapping* Mapping_create_with_pid(int pid, int dim, int levels, int order, const Tuple *flat_size, const Tuple* tiling, const Dist *dist, size_t scalar_size);

/// Same as Mapping_create except for an extra parameter to specify a preallocated memory region
Mapping* Mapping_create_with_ptr(int dim, int levels, int order, const Tuple *flat_size, const Tuple* tiling, const Dist *dist, size_t scalar_size, void* prealloc);

/// Most low-level option for creating a Mapping
Mapping* Mapping_create_impl(int pid, int owner_only, int dim, int levels, int order, const Tuple *flat_size, const Tuple* tiling, const Dist *dist, size_t scalar_size, void* prealloc);
/// Destroy mapping information
/// @param m The mapping data structure to destroy.
void Mapping_destroy(Mapping *m);

/// Print mapping information
/// @param m The pointer to mapping data structure
void Mapping_print(Mapping *m);
#endif

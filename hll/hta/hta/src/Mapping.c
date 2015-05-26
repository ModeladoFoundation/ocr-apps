#include <stdio.h>
#include <stdlib.h>
#include "Mapping.h"
#include "Debug.h"
#include "Comm.h"

// FIXME: find a better place for this

Mapping* Mapping_create(int dim, int levels, int order, const Tuple *flat_size, const Tuple* tiling, const Dist *dist, size_t scalar_size)
{
    return Mapping_create_impl(-1, 0, dim, levels, order, flat_size, tiling, dist, scalar_size, NULL);
}

Mapping* Mapping_create_with_pid(int pid, int dim, int levels, int order, const Tuple *flat_size, const Tuple* tiling, const Dist *dist, size_t scalar_size)
{
    return Mapping_create_impl(pid, 0, dim, levels, order, flat_size, tiling, dist, scalar_size, NULL);
}

Mapping* Mapping_create_with_ptr(int dim, int levels, int order, const Tuple *flat_size, const Tuple* tiling, const Dist *dist, size_t scalar_size,  void* prealloc)
{
    return Mapping_create_impl(-1, 0, dim, levels, order, flat_size, tiling, dist, scalar_size, NULL);
}

// When prealloc != NULL, the memory space is preallocated and this function will only calculate
// the pointer to the first element for each tile
Mapping* Mapping_create_impl(int pid, int owner_only, int dim, int levels, int order, const Tuple *flat_size, const Tuple* tiling, const Dist *dist, size_t scalar_size, void* prealloc)
{
    int i = 0;
    int num_tiles;
    //int mem_space_used = 0;

    ASSERT(scalar_size > 0);

    // get number of leaf tiles
    if(tiling)
        num_tiles = Tuple_count_elements(tiling, tiling->height);
    else
        num_tiles = 1;

    DBG(5, "Mapping_create(): allocate memory space for %d tiles\n", num_tiles);

    // allocate space for metadata
    Mapping *m = (Mapping*) Alloc_acquire_block(sizeof(Mapping));
    if(!owner_only)
        m->map_blocks = (void**) Alloc_acquire_block(num_tiles*sizeof(void*));
    else
        m->map_blocks = NULL;

    m->owners = (int*) Alloc_acquire_block(num_tiles*sizeof(int));

    DBG(5, "Mapping_create(): create an integer pointer array for blocks\n");
    m->order = order;
    m->dist = *dist;
    m->num_blocks = num_tiles;
    m->scalar_size = scalar_size;

    DBG(5, "Mapping_create(): allocate blocks for leaf tile\n");
#if 0
    if(num_tiles > 1)
#endif
    {
        // Allocate data blocks
        Tuple iter[levels-1]; // the number of tuples in tiling array is (levels-1) tuples
        Tuple_iterator_begin(dim, levels-1, iter);
        Tuple tile_size;
        Tuple_init_zero(&tile_size, dim); // Create a temporary tuple for computations

        if(order == ORDER_TILE) {
            ASSERT(prealloc == NULL);
            for(i=0;i<num_tiles;i++)
            {
                int owner = (pid == -1) ? (-1):(Dist_get_pid(dist, iter, tiling));
                if(!owner_only) {
                    Tuple_get_leaf_tile_size(flat_size, tiling, iter, &tile_size);
                    size_t block_size = scalar_size * Tuple_product(&tile_size);
                    DBG(5, "Mapping_create(): allocate %zd bytes for leaf tile %d\n", block_size, i);
                    if((pid == -1) || (owner == pid)) // owner of the tile // Dist_get_pid applied to the first level tiling for distribution
                        m->map_blocks[i] = Alloc_acquire_block(block_size);
                    else
                        m->map_blocks[i] = NULL;
                }
                m->owners[i] = owner;
                if(num_tiles >1) Tuple_iterator_next(tiling, iter);
            }
        } else { // ROW- or COL-major layout
            void* ptr;

            ASSERT(!owner_only);
            if(prealloc == NULL) {
                // 1. only one thread allocates a big block
                int num_elems = Tuple_product(flat_size);
                size_t total_size = scalar_size * num_elems; // NOTICE: for dense HTA only
                if(pid == 0 || pid == -1)
                    ptr = Alloc_acquire_block(total_size);

                // 2. broadcast the pointer to the block allocated to all threads
                if(pid != -1) // communicate to fetch base pointer to the block for SPMD mode
                    comm_bcast(pid, 0, &ptr, sizeof(void*));
            } else {
                ptr = prealloc;
            }

            // 3. evaluate and set pointers for leaf tiles
            Tuple nd_offset;
            nd_offset.height = 1;

            // iterate through the leaf tiles and get their logical index
            for(i=0;i<num_tiles;i++) {
                int owner = (pid == -1) ? (-1):(Dist_get_pid(dist, iter, tiling));
                Tuple_get_tile_start_offset(flat_size, tiling, iter, &nd_offset);
                // use the logical index to evaluate the location of the first element in the physical memory
                int offset = Tuple_nd_to_1d_index_by_order(order, &nd_offset, flat_size);
                m->map_blocks[i] = ptr + scalar_size * offset;
                m->owners[i] = owner;
                if(num_tiles > 1) Tuple_iterator_next(tiling, iter);
            }

        }
    }
    //DBG(5, "Mapping_create(): total memory space allocated = %d\n", mem_space_used);
    return m;
}

void Mapping_destroy(Mapping *m)
{
    if(m->map_blocks)
        Alloc_free_block(m->map_blocks);
    Alloc_free_block(m->owners);
    Alloc_free_block(m);
}

void Mapping_print(Mapping *m)
{
    printf("Mapping structure:\n");
    printf("order: %d\n", m->order);
    Dist_print(&m->dist);
    printf("scalar_size: %zd\n", m->scalar_size);
}

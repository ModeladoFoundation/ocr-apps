#include <stdio.h>
#include <stdlib.h>
//#include "HTA.h"
#include "Mapping.h"
#include "Debug.h"

Mapping* Mapping_create(int dim, int levels, int order, const Tuple *flat_size, const Tuple* tiling, const Dist *dist, size_t scalar_size)
{
    int i = 0;
    int num_tiles;
    int mem_space_used = 0;
    
    ASSERT(scalar_size > 0);

    // get number of leaf tiles
    if(tiling)
        num_tiles = Tuple_count_elements(tiling, tiling->height);  
    else
        num_tiles = 1;

    DBG(5, "Mapping_create(): allocate memory space for %d tiles\n", num_tiles);

    // allocate space for metadata
    Mapping *m = (Mapping*) Alloc_acquire_block(0, sizeof(Mapping));
    m->map_blocks = (void**) Alloc_acquire_block(0, num_tiles*sizeof(void*));
    //printf("size of mapping: %zd, Mapping: %p, map_blocks: %p\n", sizeof(Mapping), m, m->map_blocks);

    DBG(5, "Mapping_create(): create an integer pointer array for blocks\n");
    m->order = order;
    m->dist = *dist;
    m->num_blocks = num_tiles;
    m->scalar_size = scalar_size;

    DBG(5, "Mapping_create(): allocate blocks for leaf tile\n");

    if(num_tiles > 1)
    {
        // Allocate data blocks
        Tuple iter[levels-1]; // the number of tuples in tiling array is (levels-1) tuples
        Tuple_iterator_begin(dim, levels-1, iter);
        Tuple tile_size;
        Tuple_init_zero(&tile_size, dim); // Create a temporary tuple for computations
        for(i=0;i<num_tiles;i++)
        {
            Tuple_get_leaf_tile_size(flat_size, tiling, iter, &tile_size);
            size_t block_size = scalar_size * Tuple_product(&tile_size);
            DBG(5, "Mapping_create(): allocate %zd bytes for leaf tile %d\n", block_size, i);
            m->map_blocks[i] = Alloc_acquire_block(Dist_get_home(dist, i), block_size);
            mem_space_used += block_size;
            Tuple_iterator_next(tiling, iter);
        }
    }
    else
    {
        size_t block_size = scalar_size * Tuple_product(flat_size);
        DBG(5, "Mapping_create(): allocate %zd bytes for leaf tile %d\n", block_size, i);
        m->map_blocks[0] = Alloc_acquire_block(Dist_get_home(dist, i), block_size);
        mem_space_used += block_size;
    }

    DBG(5, "Mapping_create(): total memory space allocated = %d\n", mem_space_used);
    return m;
}

void Mapping_destroy(Mapping *m)
{
    Alloc_free_block(m->map_blocks);
    Alloc_free_block(m);
}

void Mapping_print(Mapping *m) 
{
    printf("Mapping structure:\n");
    printf("order: %d\n", m->order);
    Dist_print(&m->dist);
    //printf("num_blocks: %d\n", m->num_blocks);
    //for(int i = 0; i < m->num_blocks; i++)
    //{
    //    printf("map_blocks[%d]: %lu\n", i, m->map_blocks[i]);
    //}
    //printf("block_size: %zd\n", m->block_size);
    printf("scalar_size: %zd\n", m->scalar_size);
}

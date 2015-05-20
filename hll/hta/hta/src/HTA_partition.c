// ========================================================================
// Dynamic Partitioning
// ========================================================================
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/queue.h>
#include "HTA.h"
#include "Partition.h"

//#define DEVDBG

typedef SIMPLEQ_HEAD(queue_head_t, queue_entry_t) QHEAD;
typedef struct queue_entry_t {
    SIMPLEQ_ENTRY(queue_entry_t) entries;
    HTA* pHTA;
    Tuple idx;
} QENTRY;


Partition Partition_create(int num_parts, ...)
{
    int n = num_parts;
    va_list argp;
    Partition new_part;

    ASSERT(num_parts >= 0 && num_parts <= HTA_MAX_NUM_PART && "Invalid dimension! Recompile if larger dimension is needed");
    new_part.num_parts = num_parts;

    // iteratively storing numbers into the tuple
    va_start(argp, num_parts);
    while(n > 0)
    {
        int val = va_arg(argp, int);
        new_part.values[num_parts-n] = val;
        n--;
    }
    va_end(argp);
    return new_part;
}

// Computes the tiling of newly created tiles
static void _calculate_split_tiling(Tuple* old_tiling, int dim, int offset, Tuple* new_tiling0, Tuple* new_tiling1)
{
    ASSERT((offset > 0) && (offset < old_tiling->values[dim]) && "offset validity check"); // FIXME: allow empty tile partition?
    *new_tiling0 = *old_tiling;
    new_tiling0->values[dim] = offset;
    *new_tiling1 = *old_tiling;
    new_tiling1->values[dim] = old_tiling->values[dim] - offset;
}

// Reuse t0 and change the fields. Creates a new data block for t1 and
// returns the pointer.
static HTA* _split_HTA(HTA* t0, int dim, int offset) {
    HTA *t1;
    Tuple old_tiling;
    Tuple t0_new_tiling, t1_new_tiling;
    ASSERT((dim >= 0) && (dim < t0->dim) && "specified dimension can't be larger than tile dimensions");
    ASSERT(t0 && "pointer to HTA can't be NULL");
    ASSERT(t0->height > 1 && "Only support non-leaf for now");
#ifdef DEVDEBUG
    printf("_split_HTA at dim %d, offset %d\n", dim, offset);
    printf("original flat_size:");
    Tuple_print(&t0->flat_size);
#endif

    // compute new tiling for the two new HTAs
    Tuple_init_zero(&t0_new_tiling, t0->dim);
    Tuple_init_zero(&t1_new_tiling, t0->dim);
    old_tiling = t0->tiling;
    _calculate_split_tiling(&old_tiling, dim, offset, &t0_new_tiling, &t1_new_tiling);

    // allocate space for new tile t1
    t1 = (HTA*) Alloc_acquire_block(sizeof(HTA));
    *t1 = *t0; // copy most values from t0
    t1->num_tiles = Tuple_product(&t1_new_tiling);
    t1->tiling = t1_new_tiling;
    t1->tiles = (HTA**) Alloc_acquire_block(sizeof(HTA*) * t1->num_tiles);
    // FIXME: flat_size recompute
    t1->flat_size.values[dim] = 0; // reset flat_size for the partitioned dimension
    // FIXME: global rank, nd_rank, nd_tile_dimensions, nd_element_offset calculation
#if ENABLE_PREPACKING
    // reset prepacking array
    t1->prepacked = 0;
    t0->prepacked = 0;
#endif

    // allocate space for new t0
    int new_t0_num_tiles = Tuple_product(&t0_new_tiling);
    HTA** new_t0_tiles = (HTA**) Alloc_acquire_block(sizeof(HTA*) * t0->num_tiles);

    // distribute lower level tiles to t0 and t1
    Tuple iter[1];
    HTA* lt = HTA_iterator_begin(t0, iter);
#ifdef DEVDEBUG
    int t0_tiles_count = 0;
    int t1_tiles_count = 0;
#endif
    int accumulated[t1_new_tiling.values[dim]];
    for (int i = 0; i < t1_new_tiling.values[dim]; i++) accumulated[i] = 0;
    do {
        Tuple nd_idx = *iter;
        int linear_idx;
        if(nd_idx.values[dim] >= offset) { // belongs to t1, index has to change
            nd_idx.values[dim] -= offset;
            linear_idx = Tuple_nd_to_1d_index(&nd_idx, &t1_new_tiling);
            t1->tiles[linear_idx] = lt;
            if(!accumulated[nd_idx.values[dim]]) { // TODO: is this general enough for irregular HTAs?
                t1->flat_size.values[dim] += lt->flat_size.values[dim]; // accumulate flat_size for t1
                accumulated[nd_idx.values[dim]] = 1;
            }
#ifdef DEVDEBUG
            t1_tiles_count++;
#endif
        }
        else {
            linear_idx = Tuple_nd_to_1d_index(&nd_idx, &t0_new_tiling);
            new_t0_tiles[linear_idx] = lt;
#ifdef DEVDEBUG
            t0_tiles_count++;
#endif
        }
    } while ((lt = HTA_iterator_next(t0, iter)));


#ifdef DEVDEBUG
    // make sure the number of tiles redistributed is as expected
    ASSERT(t0_tiles_count == new_t0_num_tiles);
    ASSERT(t1_tiles_count == t1->num_tiles);
#endif
    // update t0 fields
    Alloc_free_block(t0->tiles);
    t0->tiles = new_t0_tiles;
    t0->num_tiles = new_t0_num_tiles;
    t0->tiling = t0_new_tiling;
    // FIXME: flat_size recompute
    t0->flat_size.values[dim] -= t1->flat_size.values[dim];
    // FIXME: global rank, nd_rank, nd_tile_dimensions, nd_element_offset calculation

#ifdef DEVDEBUG
    printf("----> Produces t0 of size: ");
    Tuple_print(&t0_new_tiling);
    printf("----> Produces t0 of flat size: ");
    Tuple_print(&t0->flat_size);
    printf("----> Produces t1 of size: ");
    Tuple_print(&t1_new_tiling);
    printf("----> Produces t1 of flat size: ");
    Tuple_print(&t1->flat_size);
#endif
    return t1;
}

static void _calculate_merge_tiling(Tuple* new_tiling, Tuple *t0_old, Tuple *t1_old, int dim) {
    *new_tiling = *t0_old;
    new_tiling->values[dim] += t1_old->values[dim];
}

// Merge whatever tiles t1 owns to t0 and destroy t1
static HTA* _merge_HTA(HTA* t0, HTA* t1, int dim) {
    ASSERT((dim >= 0) && (dim < t0->dim) && "specified dimension can't be larger than tile dimensions");
    ASSERT(t0 && "pointer to HTA can't be NULL");
    ASSERT(t0->height > 1 && "Only support non-leaf for now");

    Tuple t0_old_tiling = t0->tiling;
    HTA** t0_old_tiles = t0->tiles;

    // compute new tiling for t0 and overwrite original tiling
    _calculate_merge_tiling(&t0->tiling, &t0_old_tiling, &t1->tiling, dim);
#ifdef DEVDEBUG
    printf("Tiling before merging:\n");
    printf("t0: ");
    Tuple_print(&t0_old_tiling);
    printf("t1: ");
    Tuple_print(t1->tiling);
    printf("Tiling after merging:\n");
    Tuple_print(t0->tiling);
#endif

    // allocate new tiles array for t0
    t0->num_tiles = Tuple_product(&t0->tiling);
    t0->tiles = (HTA**) Alloc_acquire_block(sizeof(HTA*) * t0->num_tiles);
    t0->flat_size.values[dim] += t1->flat_size.values[dim];

    // for each tile in t0 assign them to the new location
    Tuple iter0[1];
    Tuple_iterator_begin(t0->dim, 1, iter0);
    do {
        int old_1d_idx = Tuple_nd_to_1d_index(iter0, &t0_old_tiling); // 1d_idx with the old tiling
        int new_1d_idx = Tuple_nd_to_1d_index(iter0, &t0->tiling); // compute 1d_idx with new tiling
        t0->tiles[new_1d_idx] = t0_old_tiles[old_1d_idx];
#ifdef DEVDEBUG
        printf("Movement:\n");
        printf(">> original t0 tile index:\n");
        Tuple_print(iter0);
        printf(">> new tile index:\n");
        Tuple_print(iter0);
        printf(">> new 1d idx: %d\n", new_1d_idx);
#endif
    } while (Tuple_iterator_next(&t0_old_tiling, iter0));

    // for each tile in t1 assign them to the new location
    Tuple iter1[1];
    Tuple_iterator_begin(t1->dim, 1, iter1);
    do {
        Tuple nd_idx = *iter1;
        nd_idx.values[dim] += t0_old_tiling.values[dim]; // add offset to the index of tiles in t1
        int old_1d_idx = Tuple_nd_to_1d_index(iter1, &t1->tiling);
        int new_1d_idx = Tuple_nd_to_1d_index(&nd_idx, &t0->tiling); // compute 1d_idx with new tiling
        t0->tiles[new_1d_idx] = t1->tiles[old_1d_idx];
#ifdef DEVDEBUG
        printf("Movement:\n");
        printf(">> original t1 tile index:\n");
        Tuple_print(iter1);
        printf(">> new tile index:\n");
        Tuple_print(&nd_idx);
        printf(">> new 1d idx: %d\n", new_1d_idx);
#endif
    } while (Tuple_iterator_next(&t1->tiling, iter1));

    // destroy t1
    Alloc_free_block(t1->tiles);
    //Alloc_free_block(&t1->tiling);
    Alloc_free_block(t1);

    // destroy useless blocks associated with old t0
    Alloc_free_block(t0_old_tiles);

    return t0;
}

// It does not increase the height of an HTA (unless it's for the leaf?)
HTA* HTA_part(HTA* h, const Partition* src_partition, const Partition* offset) {
    int dim = h->dim;

    ASSERT(h->dim <= 2 && "only support 2D for now");
    ASSERT(h->height > 1 && "leaf tile partitioning not supported");

#ifdef DEVDEBUG
    printf("Tiling before partitioning:");
    Tuple_print(&h->tiling);
    printf("Number of tiles: %d\n", h->num_tiles);
#endif

    // calculate the number of tiles after new partitioning
    Tuple new_tiling = h->tiling;
    Tuple orig_tiling = h->tiling;
    orig_tiling.height = 1; // NOTICE: to iterate at one level only, the height must be set to 1 (not very intuitive..)
    int new_num_tiles = 1;
    for(int i = 0; i < dim; i++) {
        if(src_partition->values[i] != PART_NONE) {
            new_tiling.values[i]++;
            new_num_tiles *= new_tiling.values[i];
        }
    }
#ifdef DEVDEBUG
    printf("New tiling after partitioning:");
    Tuple_print(&new_tiling);
    printf("New number of tiles: %d\n", new_num_tiles);
#endif

    // allocate a new tiles array for the new partitioning
    HTA** new_tiles = (HTA**) Alloc_acquire_block(sizeof(HTA*) * new_num_tiles);

    // Partitioning
    Tuple iter[1];
    Tuple_init_zero(iter, h->dim);
    // for each tile
    do {
        HTA* t = HTA_pick_one_tile(h, iter);
        Tuple action[1];
        Tuple new_tile_idx = iter[0];
        int to_split = 0;
        for(int i = 0; i < dim; i++) {
            if(iter->values[i] > src_partition->values[i]) {
                action->values[i] = PART_INC;
                new_tile_idx.values[i]++;
            }
            else if (iter->values[i] == src_partition->values[i]) {
                action->values[i] = PART_SPLIT;
                to_split++;
            }
            else {
                action->values[i] = PART_NO_CHANGE;
            }
        }
        // if not on the new partition lines
        // calculate new index and assign to tiles array
        if(!to_split) {
#ifdef DEVDEBUG
            printf("No split for this tile:");
            Tuple_print(iter);
#endif
            int new_1d_index = Tuple_nd_to_1d_index(&new_tile_idx, &new_tiling);
            new_tiles[new_1d_index] = t;
            // TODO: check if there is anything else to change in the tile HTA metadata
        }
        else {
            // Use a queue for the split tiles
            QHEAD qhead;
            QENTRY *qptr;
            SIMPLEQ_INIT(&qhead);
            qptr = malloc(sizeof(QENTRY));
            qptr->pHTA = t;
            qptr->idx = new_tile_idx;
            SIMPLEQ_INSERT_HEAD(&qhead, qptr, entries);
#ifdef DEVDEBUG
            printf("Split this tile:");
            Tuple_print(iter);
#endif
            // if on the new partition lines
            // split it to at most 2^dim tiles and store the new hta to tiles array
            // Iteratively split for each dimension, each time double the number of new tiles
            int entry_count = 1;
            for(int i = dim-1; i >= 0; i--) {
                if(action->values[i] == PART_SPLIT) { // check action in each dimension
                    // for each entry in the simple queue, split them
                    for(int j = 0; j < entry_count; j++) {
                        qptr = SIMPLEQ_FIRST(&qhead);
                        ASSERT(qptr != NULL);
                        SIMPLEQ_REMOVE_HEAD(&qhead, entries);

                        HTA* t0 = qptr->pHTA;
                        Tuple t0_idx = qptr->idx;
                        HTA* t1 = _split_HTA(t0, i, offset->values[i]);
                        ASSERT(t1 && "t1 must not be NULL");

                        Tuple t1_idx = t0_idx;
                        t1_idx.values[i]++;
                        // insert the modified t0
                        SIMPLEQ_INSERT_TAIL(&qhead, qptr, entries);
                        // create new queue entry for t1
                        qptr = malloc(sizeof(QENTRY));
                        qptr->pHTA = t1;
                        qptr->idx = t1_idx;
                        SIMPLEQ_INSERT_TAIL(&qhead, qptr, entries);
                    }
                    entry_count *= 2;
                }
            }
#ifdef DEVDEBUG
            printf("----> %d tiles created\n", entry_count);
#endif
            // distribute tile pointers and clean up the queue
#ifdef DEVDEBUG
            int count_inserted = 0;
#endif
            while(!SIMPLEQ_EMPTY(&qhead)) {
                qptr = SIMPLEQ_FIRST(&qhead);
                SIMPLEQ_REMOVE_HEAD(&qhead, entries);
                // some more change to the HTA fields
                // !! FIXME !! can all field changes be computed in _split_HTA??
                int new_1d_index = Tuple_nd_to_1d_index(&qptr->idx, &new_tiling);
                qptr->pHTA->rank = new_1d_index; // must assign new rank for the tile here
                new_tiles[new_1d_index] = qptr->pHTA;
#ifdef DEVDEBUG
                count_inserted++;
                printf("----> New split tile:");
                Tuple_print(&qptr->idx);
                printf("----> Inserted at linear index %d\n", new_1d_index);
#endif
                free(qptr);
            }
#ifdef DEVDEBUG
            printf("----> %d tiles are inserted for splitting this tile\n", count_inserted);
#endif
        }
    } while(Tuple_iterator_next(&orig_tiling, iter));

    // free tiles array and replace with new one
    h->tiling = new_tiling;
    Alloc_free_block(h->tiles);
    h->tiles = new_tiles;
    h->num_tiles = new_num_tiles;
    // TODO: other fields need to be changed?
    return h;
}

HTA* HTA_rmpart(HTA* h, const Partition* partition) {
    ASSERT(h->dim <= 2 && "only support 2D for now");
    ASSERT(h->height > 1 && "leaf tile partitioning not supported");

#ifdef DEVDEBUG
    printf("Tiling before removing partition:");
    Tuple_print(&h->tiling);
    printf("Number of tiles: %d\n", h->num_tiles);
#endif

    Tuple new_tiling = h->tiling; // new_tiling will eventually contain the final result
    Tuple orig_tiling = h->tiling; // orig_tiling changes in each iteration
    HTA** orig_tiles = h->tiles;
    HTA** new_tiles = NULL;
    // Merge one dimension at a time
    for (int i = 0; i < h->dim; i++) {
#ifdef DEVDEBUG
        printf("Merging tiles in dimension >>>>> %d <<<<<\n", i);
#endif
        // Generate the new tiling after merging in this dimension
        if(partition->values[i] != PART_NONE) {
            ASSERT(partition->values[i] > 0 && partition->values[i] < h->tiling.values[i] && "Partition validity check");
            new_tiling.values[i]--;
        }
        int new_num_tiles = Tuple_product(&new_tiling);

#ifdef DEVDEBUG
        printf("New tiling will become:\n");
        Tuple_print(&new_tiling);
#endif
        // allocate a new tiles array for the new partitioning
        new_tiles = (HTA**) Alloc_acquire_block(sizeof(HTA*) * new_num_tiles);
        Tuple iter[1];
        Tuple_init_zero(iter, h->dim);
        Tuple iter_space = new_tiling;
        iter_space.height = 1;
        // iterate through the index space of the new tiling
        do {
            Tuple action[1];
            int to_merge = 0;
            Tuple orig_tile_idx = iter[0];
            if(iter->values[i] == partition->values[i] - 1) {
                action->values[i] = PART_MERGE;
                orig_tile_idx.values[i]++;
                to_merge = 1;
            } else if (iter->values[i] >= partition->values[i]) {
                action->values[i] = PART_INC;
                orig_tile_idx.values[i]++;
            } else {
                action->values[i] = PART_NO_CHANGE;
            }

            // if not a merged tile in this dimension
            if(!to_merge) {
                HTA *t0 = orig_tiles[Tuple_nd_to_1d_index(&orig_tile_idx, &orig_tiling)];
#ifdef DEVDEBUG
                printf("No merging for this tile:");
                Tuple_print(&orig_tile_idx);
#endif
                int new_1d_index = Tuple_nd_to_1d_index(iter, &new_tiling);
                new_tiles[new_1d_index] = t0;
                // TODO: check if there is anything else to change in the tile HTA metadata
            } else {
                HTA *t0 = orig_tiles[Tuple_nd_to_1d_index(iter, &orig_tiling)];
                HTA *t1 = orig_tiles[Tuple_nd_to_1d_index(&orig_tile_idx, &orig_tiling)];
                t0 = _merge_HTA(t0, t1, i);
                int new_1d_index = Tuple_nd_to_1d_index(iter, &new_tiling);
                new_tiles[new_1d_index] = t0;
#ifdef DEVDEBUG
                printf("Merging tile:\n");
                Tuple_print(iter);
                Tuple_print(&orig_tile_idx);
                printf("It's 1d_idx = %d\n", new_1d_index);
#endif
            }
        } while(Tuple_iterator_next(&iter_space, iter));
        // prepare for the next dimension
        Alloc_free_block(orig_tiles);
        orig_tiles = new_tiles;
        orig_tiling = new_tiling;
    }
    // eventually new tiling is computed and assigned to the hta data structure
    h->tiling = new_tiling;
    h->tiles = new_tiles;
    h->num_tiles = Tuple_product(&h->tiling);
    // TODO: other fields need to be changed?
    return h;
}

HTA* HTA_part_matrix(int dim, void *matrix, Tuple* matrix_size, HTA_SCALAR_TYPE scalar_type, Partition* partitions) {
    ASSERT(dim == 2 && "only 2d is supported for now");

    // ================================
    // 1. Build the HTA metadata shell
    // ================================
    // for each partition at each dimension, compute the tiling at the
    // top level and the flat_size of the leaf levels
    Tuple tiling; // top level tiling
    tiling.dim = dim;
    Partition tile_sizes[dim]; // the leaf tile sizes after partitioning
    for(int i = 0; i < dim; i++) {
        int num_parts = partitions[i].num_parts;
        tiling.values[i] = (num_parts == 0) ? 1 : (num_parts + 1);
        tile_sizes[i].num_parts = tiling.values[i];

        if(num_parts == 0) { // only one partition
            tile_sizes[i].values[0] = matrix_size->values[i];
        } else {
            for(int j = 0; j < tiling.values[i]; j++) {
                // partition validity check
                if(j < num_parts)
                    ASSERT(partitions[i].values[j] > 0 && partitions[i].values[j] < matrix_size->values[i]);
                // FIXME: assume sorted numbers
                if(j == 0) // first partition
                    tile_sizes[i].values[j] = partitions[i].values[j];
                else if (j == tiling.values[i] - 1) // last partition
                    tile_sizes[i].values[j] = matrix_size->values[i] - partitions[i].values[j-1];
                else
                    tile_sizes[i].values[j] = partitions[i].values[j] - partitions[i].values[j-1];
            }
        }
    }
    tiling.height = 2;
#ifdef DEVDBG
    for(int i = 0; i < dim; i++) {
        printf("tile_sizes[%d]: ", i);
        for (int j = 0; j < tile_sizes[i].num_parts; j++)
            printf(" %d ", tile_sizes[i].values[j]);
        printf("\n");
    }
#endif

    // Create the meta data of leaf tiles
    Tuple iter[1];
    Tuple_init_zero(iter, dim);
    Tuple iter_space = tiling;
    iter_space.height = 1;
    int num_tiles = Tuple_product(&tiling);
    // TODO: support for distributed memory environment?
    HTA** tiles = Alloc_acquire_block(num_tiles * sizeof(HTA*));
    do {
        HTA* leaf_tile = Alloc_acquire_block(sizeof(HTA));
        Tuple flat_size;
        flat_size.height = 1;
        flat_size.dim = dim;
        for(int i = 0; i < dim; i++) {
            flat_size.values[i] = tile_sizes[i].values[iter->values[i]];
        }
        int num_elems = Tuple_product(&flat_size);
        void* raw_data = Alloc_acquire_block(num_elems* HTA_size_of_scalar_type(scalar_type));

        // Fill in other fields
        leaf_tile->type = HTA_TYPE_DENSE;
        leaf_tile->dim = dim;
        leaf_tile->height = 1;
        leaf_tile->scalar_type = scalar_type;
        leaf_tile->rank = Tuple_nd_to_1d_index(iter, &iter_space);
        // FIXME: fix these fields
        //h->nd_rank.dim = dim;
        //h->nd_tile_dimensions = nd_size_at_level[levels-1]; // it's actually size at "height"
        //Tuple_1d_to_nd_index(cur_idx, &h->nd_tile_dimensions, &h->nd_rank);
        //h->nd_element_offset = *element_offset;
        leaf_tile->is_selection= 0;
        leaf_tile->flat_size = flat_size;
        leaf_tile->num_tiles = 1;
        leaf_tile->tiles = NULL;
        //leaf_tile->tiling = NULL;
        leaf_tile->leaf.num_elem = num_elems;
        leaf_tile->leaf.raw = raw_data;
        // FIXME: fix these fields
        //h->home = Dist_get_home(&mmap->dist, cur_idx);
#if ENABLE_PREPACKING
	leaf_tile->prepacked = 0;
	leaf_tile->num_gpps = 0;
	leaf_tile->prepacked_data_array = NULL;
#endif

        // Assign new tile pointer to the tile array of root level
        int _1d_idx = Tuple_nd_to_1d_index(iter, &iter_space);
        tiles[_1d_idx] = leaf_tile;

    } while(Tuple_iterator_next(&iter_space, iter));

    // Fill in fields of top level HTA
    HTA *h =  Alloc_acquire_block(sizeof(struct hta));
    h->type = HTA_TYPE_DENSE;
    h->dim = dim;
    h->height = 2;
    h->scalar_type = scalar_type;
    h->rank = 0;
    h->nd_rank.dim = dim;
    // FIXME: fix these fields
    //h->nd_tile_dimensions = nd_size_at_level[levels-1]; // it's actually size at "height"
    //Tuple_1d_to_nd_index(cur_idx, &h->nd_tile_dimensions, &h->nd_rank);
    //h->nd_element_offset = *element_offset;
    h->is_selection= 0;
    h->num_tiles = num_tiles;
    h->leaf.num_elem = 0;
    h->leaf.raw = NULL;
    h->home = -1;
    h->tiling = tiling;
    h->tiles = tiles;
    h->flat_size = *matrix_size;
#if ENABLE_PREPACKING
    h->prepacked = 0;
    h->num_gpps = 0;
    h->prepacked_data_array = NULL;
#endif

    // =================================================
    // 2. Call HTA_init_with_array to initialize values
    // =================================================
    if(matrix != NULL)
        HTA_init_with_array(h, matrix);
    return h;
}

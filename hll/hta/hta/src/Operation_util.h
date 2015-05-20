#include "Config.h"
#include "Types.h"
#define MAX_SCALAR_SIZE (sizeof(dcomplex))

#if REUSE_GPP_ARRAYS
#if defined(PIL2SWARM) || defined(PIL2OCR)
gpp_t index_array;
gpp_t data_array;
#else
extern gpp_t index_array;
extern gpp_t data_array;
#endif
gpp_t* s_darray;
int index_array_pool_size = 0;
int data_array_pool_size = 0;
int scalar_array_pool_size = 0;
#endif

#if !REUSE_GPP_ARRAYS
    #define GPP_ARRAY_INIT  \
        gpp_t index_array, data_array;  \
        pil_init(&index_array, &data_array); \
        pil_alloc(&index_array, (bound+1)*sizeof(int)); \
        pil_alloc(&data_array, total_num_gpps*sizeof(gpp_t));

    #define GPP_SARRAY_INIT \
        gpp_t s_darray[bound]; \
        for(int i = 0; i < bound; i++) { \
            pil_alloc(&s_darray[i], sz); \
            memcpy(s_darray[i].ptr, s1, sz); \
        }

    #define GPP_ARRAY_FINALIZE \
        pil_free(data_array); \
        pil_free(index_array);

    #define GPP_SARRAY_FINALIZE\
        for(int i = 0; i < bound; i++) { \
            pil_free(s_darray[i]); \
        }
#else
    #define GPP_ARRAY_INIT  \
        if(index_array_pool_size < (bound+1)) { \
            if(index_array_pool_size != 0) { \
                pil_free(index_array); \
            } \
            pil_init(&index_array, &index_array); \
            pil_alloc(&index_array, (bound+1)*sizeof(int)); \
            index_array_pool_size = bound+1; \
        } \
        if(data_array_pool_size < total_num_gpps) { \
            if(data_array_pool_size != 0) { \
                pil_free(data_array); \
            } \
            pil_init(&data_array, &data_array); \
            pil_alloc(&data_array, total_num_gpps*sizeof(gpp_t)); \
            data_array_pool_size = total_num_gpps; \
        }

    #define GPP_SARRAY_INIT \
        if(scalar_array_pool_size < bound) \
        { \
            if(scalar_array_pool_size != 0) { \
                for(int i = 0; i < scalar_array_pool_size; i++) \
                    pil_free(s_darray[i]); \
                free(s_darray); \
            } \
            s_darray = malloc(sizeof(gpp_t) * bound); \
            for(int i = 0; i < bound; i++) { \
                pil_alloc(&s_darray[i], MAX_SCALAR_SIZE); \
            } \
            scalar_array_pool_size = bound; \
        } \
        for(int i = 0; i < bound; i++) { \
            memcpy(s_darray[i].ptr, s1, sz); \
        }

    #define GPP_ARRAY_FINALIZE ;
    #define GPP_SARRAY_FINALIZE ;
#endif

#define MATMUL_CASE(type) \
    for(j = 0; j < j_bound; j++) { \
	Tuple i_j; \
        Tuple_init(&i_j, 2, idx-1, j); \
	type* d = HTA_access_element(h1, &i_j); \
	for(k = 0; k < k_bound; k++) { \
	    Tuple i_k; \
            Tuple_init(&i_k, 2, idx-1, k); \
	    Tuple k_j; \
            Tuple_init(&k_j, 2, k, j); \
	    type* s1 = HTA_access_element(h2, &i_k);  \
	    type* s2 = HTA_access_element(h3, &k_j); \
	    *d += (*s1) * (*s2); \
	} \
    }

#ifdef SPMD_PIL
void _enter_80(gpp_t index_array, gpp_t data_array, int pid, ReduceOp fr_op, int dim_reduc);
void _enter_120(gpp_t index_array, gpp_t data_array, int pid, int level, H2Op h2op);
void _enter_130(gpp_t index_array, gpp_t data_array, int pid, int level, H3Op h3op);
#else
void _enter_80(gpp_t index_array, gpp_t data_array, int bound, int idx, ReduceOp fr_op, int dim_reduc);
void _enter_120(gpp_t index_array, gpp_t data_array, int bound, int idx, int level, H2Op h2op);
void _enter_130(gpp_t index_array, gpp_t data_array, int bound, int idx, int level, H3Op h3op);
#endif
void _enter_81(gpp_t index_array, gpp_t data_array, int bound, int idx, ReduceOp fr_op, int dim_reduc, int vec_size);
void _enter_1200(gpp_t index_array, gpp_t data_array, int pid, int *step, int np, size_t size, int offset, gpp_t buf, int *dest, gpp_t *sendbuf, int *src, gpp_t *recvbuf, HTA_SCALAR_TYPE stype, ReduceOp fr_op);

// For scan operation
void _scan_leaf_tile(HTA* t, ReduceOp op, void* sum);
void _local_scan(HTA* h, ReduceOp op, void* local_sum);
void _scan_subtree(HTA *h, ReduceOp op, void* sum);

void _final_scan_leaf_tile(HTA* h, ReduceOp op, void* sum);
void _final_scan_subtree(HTA *h, ReduceOp op, void* sum) ;
void _final_scan(HTA *h, ReduceOp op, void* ex_sum);

void _recursive_full_reduce(ReduceOp op, void* result, HTA* h)
{
    if(h->height == 1)
    {
        int num_elements = h->leaf.num_elem;
        void* ptr = HTA_get_ptr_raw_data(h);
        for(int i = 0; i < num_elements; i++) {
            op(h->scalar_type, result, ptr + HTA_get_scalar_size(h) * i);
        }
    }
    else
    {
        int num_tiles = h->num_tiles;
        for(int i = 0; i < num_tiles; i++)
        {
            _recursive_full_reduce(op, result, h->tiles[i]);
        }
    }
}

int _count_gpps(HTA * h)
{
    int total = 0;
    int multiplier = 1;
    ASSERT(h);

#if ENABLE_PREPACKING
    if(h->prepacked)
        return h->num_gpps;
#endif

    if(h->type == HTA_TYPE_DENSE)
    {
        // iteratively count dynamically allocated objects assuming regular tiles
        while(h->height >= 1) {
            total += 2 * multiplier;
            multiplier *= h->num_tiles;
            if(h->height > 1)
                h = h->tiles[0];
            else
                break;
        }
        return total;
    }

    if(h->height == 1) { // leaf
        if(h->leaf.num_elem != 0) {
            total += 2;
        }
        else {
            ASSERT(h->type != HTA_TYPE_DENSE);
            total = 1;
        }
    }
    else {
        total = 2;
        // recursively pack its children
        for(int i = 0; i < h->num_tiles; i++) {
            total += _count_gpps(h->tiles[i]);
        }
    }

    return total;
}

int _count_gpps_at_level(HTA *h, int level)
{
    for(int i = 0; i < level; i++)
        h = h->tiles[0];
    return _count_gpps(h);
}

int get_num_gpps(HTA *h, HTA **ha, int level, int bound) {
    int total_num_gpps = 0;
    if(h!= NULL && h->type == HTA_TYPE_DENSE) // faster computation assuming dense HTA is regular
        total_num_gpps = _count_gpps_at_level(h, level) * bound;
    else {
        for(int i = 0; i < bound; i++) {
            total_num_gpps += _count_gpps(ha[i]);
        }
    }
    return total_num_gpps;
}

// perform DFS on the input HTA tree to fill in the gpp array in prefix order
// returns the number of gpps processed
int _recursive_pack_HTA(gpp_t *g, HTA *h)
{
    int total = 0;

    // Fill in guid information of "this"
    g[0].guid = (guid_t) Alloc_get_block_id(h);
    // printf("Packing 0x%08x\n", g[0].guid);
    if(h->height == 1) { // leaf
        if(h->leaf.num_elem != 0) {
            if(h->leaf.order != ORDER_TILE) // FIXME: This may not work for OCR
                g[1].guid = (guid_t) (h->leaf.raw);
            else
                g[1].guid = (guid_t) Alloc_get_block_id(h->leaf.raw);

            // printf("Packing 0x%08x\n", g[1].guid);
            total = 2;
        }
        else {
            ASSERT(h->type != HTA_TYPE_DENSE);
            total = 1;
        }
    }
    else {
        //g[1].guid = (guid_t) Alloc_get_block_id(h->tiling);
        // printf("Packing 0x%08x\n", g[1].guid);
        g[1].guid = (guid_t) Alloc_get_block_id(h->tiles);
        // printf("Packing 0x%08x\n", g[2].guid);
        total = 2;
        // recursively pack its children
        for(int i = 0; i < h->num_tiles; i++) {
            total += _recursive_pack_HTA(g + total, h->tiles[i]);
        }
    }
    return total;
}

// recursively unpack the HTA tree and filled in the gpp array
// returns the number of gpps processed
#define gpp_get_ptr(g) ((g).ptr + sizeof(gpp_t))
int _unpack_HTA(gpp_t *g, HTA** h)
{
    int total;

    ASSERT(g && h);
    // printf("Unpacking 0x%08x\n", g[0].guid);
    ASSERT(gpp_get_ptr(g[0]));

    *h = gpp_get_ptr(g[0]);
    HTA *x = *h;
    if(x->height == 1) {
        if(x->leaf.num_elem != 0) {
            // printf("Unpacking 0x%08x\n", g[1].guid);
            if(x->leaf.order != ORDER_TILE) { // FIXME: This may not work for OCR
                x->leaf.raw = g[1].ptr;
            } else {
                ASSERT(gpp_get_ptr(g[1]));
                x->leaf.raw = gpp_get_ptr(g[1]);
            }
            total = 2;
        }
        else {
            ASSERT(x->type != HTA_TYPE_DENSE);
            total = 1;
        }
    }
    else {
        // printf("Unpacking 0x%08x\n", g[1].guid);
        ASSERT(gpp_get_ptr(g[1]));
        // printf("Unpacking 0x%08x\n", g[2].guid);
        ASSERT(gpp_get_ptr(g[2]));

        //x->tiling = gpp_get_ptr(g[1]);
        x->tiles =  gpp_get_ptr(g[1]);
        total = 2;

        for(int i = 0; i < x->num_tiles; i++) {
            total += _unpack_HTA(g + total, &x->tiles[i]);
        }
    }

    return total;
}

// A wrapper function to the recursive packing function
int _pack_HTA(gpp_t *g, HTA *h)
{
    int total = 0;
    ASSERT(h && g);
#if ENABLE_PREPACKING
    if(h->prepacked) { // copy directly from prepacked array instead of traversing
        guid_t* darray = (guid_t*) h->prepacked_data_array;
        int count = h->num_gpps;
        for(int i = 0; i < count; i++) {
            g[i].guid = darray[i];
        }
        return count;
    }
#endif

    total =  _recursive_pack_HTA(g, h);

#if ENABLE_PREPACKING
    ASSERT(!h->prepacked);
    guid_t* darray = (guid_t*) malloc(sizeof(guid_t) * total);
    for(int i = 0; i < total; i++) {
        darray[i] = g[i].guid;
    }
    h->prepacked = 1;
    h->prepacked_data_array = darray;
    h->num_gpps = total;
#endif

    return total;
}

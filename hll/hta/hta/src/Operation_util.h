
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

void _recursive_full_reduce(ReduceOp op, void* result, HTA* h) 
{ 
    if(h->height == 1) 
    { 
        //int num_elements = Tuple_count_elements(h->tiling, 1); 
        int num_elements = h->leaf.num_elem;
        void* ptr = HTA_get_ptr_raw_data(h); 
        for(int i = 0; i < num_elements; i++) { 
            op(h->scalar_type, result, ptr + HTA_get_scalar_size(h) * i); 
        } 
    } 
    else 
    { 
        int num_tiles = Tuple_count_elements(h->tiling, 1); 
        for(int i = 0; i < num_tiles; i++) 
        { 
            _recursive_full_reduce(op, result, h->tiles[i]); 
        } 
    } 
} 

int _count_gpps(HTA * h)
{
    int total = 0;
    ASSERT(h);

    if(h->type == HTA_TYPE_DENSE)
    {
        // iteratively count dynamically allocated objects assuming regular tiles
        for(int i = h->height - 1; i >= 0; i--)
        {
            if(i == h->height - 1)
                total = 2;
            else
                total = 3 + Tuple_product(&h->tiling[i]) * total;
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
        total = 3;
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
int _pack_HTA(gpp_t *g, HTA *h) 
{
    int total = 0;

    ASSERT(h && g);

    // Fill in guid information of "this" 
    g[0].guid = (guid_t) Alloc_get_block_id(h);
    // printf("Packing 0x%08x\n", g[0].guid);
    if(h->height == 1) { // leaf
        if(h->leaf.num_elem != 0) {
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
        g[1].guid = (guid_t) Alloc_get_block_id(h->tiling);
        // printf("Packing 0x%08x\n", g[1].guid);
        g[2].guid = (guid_t) Alloc_get_block_id(h->tiles);
        // printf("Packing 0x%08x\n", g[2].guid);
        total = 3;
        // recursively pack its children
        for(int i = 0; i < h->num_tiles; i++) {
            total += _pack_HTA(g + total, h->tiles[i]);
        }
    }

    return total;
}

// recursively unpack the HTA tree and filled in the gpp array
// returns the number of gpps processed
int _unpack_HTA(gpp_t *g, HTA** h)
{
    int total;

    ASSERT(g && h);
    // printf("Unpacking 0x%08x\n", g[0].guid);
    ASSERT(g[0].ptr);

    *h = g[0].ptr;
    HTA *x = *h;
    if(x->height == 1) {
        if(x->leaf.num_elem != 0) {
            // printf("Unpacking 0x%08x\n", g[1].guid);
            ASSERT(g[1].ptr);
            x->leaf.raw = g[1].ptr;
            total = 2;
        }
        else {
            ASSERT(x->type != HTA_TYPE_DENSE);
            total = 1;
        }
    }
    else {
        // printf("Unpacking 0x%08x\n", g[1].guid);
        ASSERT(g[1].ptr);
        // printf("Unpacking 0x%08x\n", g[1].guid);
        ASSERT(g[2].ptr);

        x->tiling = g[1].ptr;
        x->tiles = g[2].ptr;
        total = 3;

        for(int i = 0; i < x->num_tiles; i++) {
            total += _unpack_HTA(g + total, &x->tiles[i]);
        }
    }

    return total;
}

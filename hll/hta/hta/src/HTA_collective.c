#include <string.h>
#include <stdio.h>
#include "Tuple.h"
#include "HTA.h"

static void _get_shift_target(Tuple * target, Tuple * nd_idx, Tuple * nd_size, Tuple * direction) 
{
    ASSERT(target->dim == nd_idx->dim && nd_idx->dim == nd_size->dim && nd_size->dim == direction->dim);

    for(int i = 0; i < target->dim; i++){
        int size = nd_size->values[i];
        int dir = direction->values[i] % size;
        dir = (dir < 0) ? (size + dir) : dir; // make dir be positive
        target->values[i] = (nd_idx->values[i] + dir) % size;
    }
}

static void _recursive_rerank(HTA *h, int* rank) 
{
    if(h->height == 1) {
        h->rank = *rank;
        (*rank)++;
    }
    else {
        for(int i = 0; i < h->num_tiles; i++){
            _recursive_rerank(h->tiles[i], rank);
        }
    }
}

// FIXME: this implementation works for shared memory model only
void HTA_circshift(HTA *h, Tuple *dir) {
    ASSERT(h->dim == dir->dim);

    if(h->height == 1) // a leaf tile
        return;

    int dim = h->dim;
    HTA* newtiles[h->num_tiles]; // a temporary array for storing shifted pointers

    Tuple iter;
    Tuple target;
    Tuple_iterator_begin(dim, 1, &iter); // an iterator that walks at 1st level only
    Tuple_init_zero(&target, dim);
    do {
        _get_shift_target(&target, &iter, h->tiling, dir); // get the nd index of shift target position
        int target_idx = Tuple_nd_to_1d_index(&target, h->tiling);
        //ASSERT(newtiles[target_idx] == NULL); // must not be written any value yet
        newtiles[target_idx] = HTA_pick_one_tile(h, &iter);
    }while(Tuple_iterator_next(h->tiling, &iter));

    // overwrite original pointers
    for(int i = 0; i < h->num_tiles; i++) 
    {
        h->tiles[i] = newtiles[i];
    }

    // traverse the HTA and update the rank value for they will be used to infer the location
    HTA_rerank(h);
}

void HTA_rerank(HTA *h) 
{
    int rank = 0;
    _recursive_rerank(h, &rank);
}


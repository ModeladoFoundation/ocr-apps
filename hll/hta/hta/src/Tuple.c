#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "Tuple.h"
#include "Accessor.h"
#include "Debug.h"
// #include "RefCount.h"

// Create a tuple. It returns the whole data structure filled with initial
// value.
Tuple Tuple_create(int dim, ...)
{
    int n = dim;
    va_list argp;
    Tuple new_tuple;

    ASSERT(dim > 0 && dim <= HTA_MAX_DIM && "Invalid dimension! Recompile if larger dimension is needed");
    new_tuple.accessor_type = ACCESSOR_TUPLE;
    new_tuple.dim = dim;
    new_tuple.height = 1;
    
    // iteratively storing numbers into the tuple
    va_start(argp, dim);
    while(n > 0)
    {
        int val = va_arg(argp, int);
        new_tuple.values[dim-n] = val;
        n--;
    }
    va_end(argp);
    return new_tuple;
}

// Initialize an existing tuple storage. This should be used because it doesn't have to copy
// the whole Tuple structure
void Tuple_init(Tuple* new_tuple, int dim, ...)
{
    int n = dim;
    va_list argp;

    ASSERT(new_tuple);
    ASSERT(dim > 0 && dim <= HTA_MAX_DIM && "Invalid dimension! Recompile if larger dimension is needed");
    new_tuple->accessor_type = ACCESSOR_TUPLE;
    new_tuple->dim = dim;
    new_tuple->height = 1;
    
    // iteratively storing numbers into the tuple
    va_start(argp, dim);
    while(n > 0)
    {
        int val = va_arg(argp, int);
        new_tuple->values[dim-n] = val;
        n--;
    }
    va_end(argp);
}

void Tuple_fill_in_sequence(Tuple* t, int num_tuples, ...)
{
    ASSERT(num_tuples > 0);
    va_list argp;
    va_start(argp, num_tuples);
    for(int i = 0; i < num_tuples; i++)
    {
        t[i] = va_arg(argp, Tuple);
        t[i].height = num_tuples - i;
    }
    va_end(argp);
}

// Compute the product of numbers in a tuple
int Tuple_product(const Tuple *t)
{
    int product = 1;
    for(int i = 0; i < t->dim; i++)
    {
        product *= t->values[i]; // TODO: add overflow protection here for debug version
    }
    return product;
}

// Print Tuple content
void Tuple_print(const Tuple *t)
{
    printf("Tuple: ");
    printf("dim = %d, ", t->dim);
    printf("height = %d, (", t->height);
    for(int i=0; i < t->dim; i++)
    {
        printf((i!=t->dim-1)?"%d,":"%d)\n", t->values[i]);
    }
}

/// Count the number of total elements at a certain depth
/// @param depth The number of levels to go down
/// @param t The tuple sequence to count
int Tuple_count_elements(const Tuple* t, int depth)
{
    int size = 1;

    if(depth == 0) // root level
        return 1;
    
    ASSERT(t->height >= depth);

    for(int i = 0; i < depth; i++)
    {
        size *= Tuple_product(t+i); // FIXME: This can overflow very easily
    }

    return size;
}

//Tuple Tuple_create_empty(int dim)
//{
//    Tuple new_tuple;
//    ASSERT(dim > 0);
//    new_tuple.accessor_type = ACCESSOR_TUPLE;
//    new_tuple.dim = dim;
//    new_tuple.height = 1; 
//    // iteratively storing numbers into the tuple
//    for(int n = 0; n < dim; n++)
//        new_tuple.values[n] = 0;
//    return new_tuple;
//}

void Tuple_init_zero(Tuple *new_tuple, int dim)
{
    ASSERT(dim > 0);
    ASSERT(new_tuple);
    new_tuple->accessor_type = ACCESSOR_TUPLE;
    new_tuple->dim = dim;
    new_tuple->height = 1; 
    // iteratively storing numbers into the tuple
    for(int n = 0; n < dim; n++)
        new_tuple->values[n] = 0;
}

void Tuple_add_dimensions(Tuple *d, const Tuple *s) 
{
    ASSERT(d->dim == s->dim);
    for(int n = 0; n < d->dim; n++)
        d->values[n] += s->values[n];
}

void Tuple_overwrite_values(Tuple *d, const Tuple *s)
{
    ASSERT(d->dim == s->dim);
    for(int n = 0; n < d->dim; n++)
        d->values[n] = s->values[n];
}

void Tuple_clear_value(Tuple *t) 
{
    ASSERT(t);
    memset(t->values, 0, sizeof(int) * t->dim);
}

void Tuple_clone_array(Tuple *d, const Tuple *s) {
    ASSERT(d != NULL && s != NULL);
    //ASSERT(d->height == s->height);
    int height = s->height;

    for(int i = 0; i < height; i++)
        d[i] = s[i]; // copy every field in struct Tuple
}

void Tuple_iterator_begin(int dim, int num_tuples, Tuple* iter) {
    ASSERT(iter != NULL);
    for(int i = 0; i < num_tuples; i++){
        //iter[i] = Tuple_create_empty(dim);
        Tuple_init_zero(&iter[i], dim);
        iter[i].height = num_tuples - i;
    }
}

int Tuple_iterator_next(const Tuple* tiling, Tuple* iter) 
{
    int total_height = tiling->height;
    int total_dim = tiling->dim;
    ASSERT(total_height == iter->height && total_dim == iter->dim);

    int done = 0;
    for(int level = total_height - 1; level >= 0; level--) {
        Tuple* i = &iter[level];
        const Tuple* sz = &tiling[level];
        for(int dim = total_dim - 1; dim >= 0; dim--) {
            if(i->values[dim] + 1 < sz->values[dim])
            {
                i->values[dim]++;
                done = 1;
                break; // for(dim)
            }
            else
            {
                i->values[dim] = 0;
            }
        }
        if(done == 1)
            break;
    }
    if(done == 0)
        return 0; // invalid iterator increment
    else
        return 1; // next element found
}

/// Return the nd global offset of the specified tile
void Tuple_get_tile_start_offset(Tuple* flat_size, Tuple* tiling, Tuple *iter, Tuple *nd_offset) {
    int dim = flat_size->dim;
    ASSERT(flat_size && tiling && iter && nd_offset);
    ASSERT(tiling->height >= iter->height);
    ASSERT(flat_size->height == 1 && nd_offset->height == 1);

    Tuple cur_flat_size = *flat_size;
    Tuple_init_zero(nd_offset, dim);
    Tuple cur_offset;
    Tuple_init_zero(&cur_offset, dim);

    for(int i = 0; i < iter->height; i++) {
        Tuple_get_lower_tile_size(&iter[i], &tiling[i], &cur_flat_size, &cur_flat_size, &cur_offset);
        Tuple_add_dimensions(nd_offset, &cur_offset);
    }
}

/// tile_size has to be initialized to the same value as flat_size!
void Tuple_get_leaf_tile_size(const Tuple *flat_size, const Tuple* tiling, const Tuple* iter, Tuple* tile_size)
{
    ASSERT(tiling != NULL && iter != NULL);
    ASSERT(tiling->height == iter->height);

    Tuple cur_flat_size = *flat_size;

    for(int lvl = 0; lvl < tiling->height; lvl++)
    {
        Tuple_get_lower_tile_size(&iter[lvl], &tiling[lvl], &cur_flat_size, tile_size, NULL);
        cur_flat_size = *tile_size;
    }
} 

/// @param nd_idx the n-dimensional index to the lower level tile
/// @param nd_size the size of n-dimensions to the lower level 
/// @param flat_size the flattened size of scalar elements at this level
/// @param the result of the flattened size of the lower level tile (output)
void Tuple_get_lower_tile_size(const Tuple *nd_idx, const Tuple *nd_size, const Tuple *flat_size, Tuple* tile_size, Tuple* element_offset) 
{
    ASSERT(tile_size != NULL);

    for(int dim = 0; dim < flat_size->dim; dim++)
    {
        int start = (int) ((nd_idx->values[dim] * (long long)flat_size->values[dim]) / nd_size->values[dim]); // overflow prevention
        int end = (int)(((nd_idx->values[dim] + 1) * (long long)flat_size->values[dim]) / nd_size->values[dim]);
        ASSERT(end > start);
        tile_size->values[dim] = end - start;
        if(element_offset != NULL) {
            element_offset->values[dim] = start;
        }
    }
}

void Tuple_1d_to_nd_index(int orig_idx, const Tuple *nd_size, Tuple *nd_idx) 
{
    int dim = nd_size->dim;
    ASSERT(nd_idx->dim == nd_size->dim);

    int idx = orig_idx;
    for(int i = dim-1; i >= 0; i--) { // from the least significant dimension, compute the index 
        nd_idx->values[i] = idx % nd_size->values[i];
        idx /= nd_size->values[i];
    }
}

int Tuple_nd_to_1d_index(const Tuple *nd_idx, const Tuple *nd_size)
{
    int dim = nd_idx->dim;
    int mul = 1;
    int idx = 0;
    ASSERT(nd_idx->dim == nd_size->dim);

    for(int i = dim-1; i>=0; i--)
    {
	idx += nd_idx->values[i] * mul;
	mul *= nd_size->values[i];
        ASSERT(mul >= 1); // overflow detection
    }
    return idx;
}

// For partial reduction

void Tuple_set_tuples_dim(Tuple *t, int dim_reduc, int value)
{ 
  ASSERT(t);
  ASSERT(dim_reduc < t->dim && dim_reduc >= 0);
  
  t->values[dim_reduc] = value;
  
  for(int i = 0; i < t->height; i++)
  {
     t[i].values[dim_reduc] = value;
  }
}

void Tuple_get_size_of_dimensions(Tuple *t, int depth, Tuple *result)
{
    int i, j;
    
    ASSERT(t && result);
    ASSERT(t->height >= depth);
    ASSERT(t->dim == result->dim);
    
    for(j = 0; j < t->dim; j++) {
        int val = 1;
        for(i = 0; i < depth; i++)
            val *= t[i].values[j]; // FIXME: possible overflow
        result->values[j] = val;
    } 
}

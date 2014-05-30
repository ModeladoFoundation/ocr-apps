#ifndef __HTA_H__
#define __HTA_H__

/// @file HTA.h

#include "Tuple.h"
#include "Region.h"
#include "Distribution.h"
#include "Alloc.h"
#include "Mapping.h"
#include "Debug.h"
#include "Types.h"

#define HTA_ROOT_LEVEL(h) (0)
#define HTA_LEAF_LEVEL(h) (h->height - 1)
#define HTA_SCALAR_LEVEL(h) (h->height)

typedef enum {
    HTA_TYPE_UNDEF,
    HTA_TYPE_DENSE,
    HTA_TYPE_SPARSE,
    HTA_TYPE_MAX
} HTA_TYPE;

/// The primitive type of scalar elements in an HTA
typedef enum {
    HTA_SCALAR_TYPE_INT32,
    HTA_SCALAR_TYPE_INT64,
    HTA_SCALAR_TYPE_UINT32,
    HTA_SCALAR_TYPE_UINT64,
    HTA_SCALAR_TYPE_FLOAT,
    HTA_SCALAR_TYPE_DOUBLE,
    HTA_SCALAR_TYPE_DCOMPLEX,
    HTA_SCALAR_TYPE_MAX
} HTA_SCALAR_TYPE;

typedef struct leaf_t {
    /// number of elements
    int num_elem;
    /// for dense tile it's just an 1D array
    /// for sparse tile, it contains 3 parts: val, col_ind, and row_ptr (Compressed Sparse Row format)
    void *raw; // translates to (scalar_type *) for dense tile
} Leaf;

/// The data structure of HTA
typedef struct hta {
    /// type of HTA
    HTA_TYPE type;
    /// The scalar type of this HTA
    HTA_SCALAR_TYPE scalar_type;
    /// Number of  dimensions 
    int dim;              
    /// The height of current tile in the tree (i.e. the number of levels below this one)
    int height;
    /// The home location (processor node number) of this tile. -1 means this tile is not leaf   
    int home;
    /// The rank of it among all HTAs at the same level
    int rank;        
    /// The nd rank of the HTA (it only apply to leaf tiles
    Tuple nd_rank;
    /// global tile dimensions at the same level
    Tuple nd_tile_dimensions;
    /// Global element start offset
    Tuple nd_element_offset;
    /// The number of tiles immediately one level below
    int num_tiles;
    /// The dimension of flattened HTA
    Tuple flat_size;
    /// An array of <levels> Tuples
    Tuple* tiling; 
    /// tiles is a dynamically allocated array of pointers to all HTA metadata one level immediately below
    struct hta** tiles;  
    /// The block id of the data tile. It's 0 for non-leaf HTA tiles.
    /// The pointer to leaf tile metadata
    Leaf leaf;

    /// This flag is to indicate if the HTA metadata is created by selection
    /// if it is true, the destroy function free the metadata used by the root
    /// HTA only and will not go recursively destroying childrens
    int is_selection;

    /// !!!!!!! The following fields are for doing tricks !!!!!!!
    /// !!!!!!! These fields should not be used in general manipulation
    /// !!!!!!! Since they are not valid for HTAs acquired with HTA_select
    /// The level relative to the root during first initialization
    // int level; 
    /// The block id of it's original parent
    // struct hta* parent;
    /// The block id of it's original root
    // struct hta* root;
} HTA;

/// The new HTA_create takes the flatten HTA dimension size and the tiling of all levels 
/// except for the scalar level, which is computed automatically by the library
/// The flat_size field in each HTA instance is used to record the dimensions if
/// the HTA is flattened. If this field is null pointer, the HTA is an exact sized one.
/// HTA HTA_create(int dim, int levels, Tuple flat_size, Tuple tiling, int order, Dist dist, HTA_SCALAR_TYPE scalar_type);
HTA* HTA_create(int dim, int levels, const Tuple *flat_size, int order, Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, ...);
HTA* HTA_create_with_ts(int dim, int levels, const Tuple *flat_size, int order, Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, Tuple *ts);
/// This API destroys an HTA tree by recursively traverse and invoke RefCount_release() on
/// each node.
/// @param h The pointer to the top level HTA metadata
void HTA_destroy(HTA *h);

/// Print HTA information for debugging
/// @param h The pointer to the top level HTA metadata
/// @param recursively 1 means go down the HTA tree recursively. 0 means only current level.
void HTA_print(const HTA* h, int recursively);

// Initialize an empty HTA shell using data from existing matrix
void HTA_init_with_array(HTA *h, void* array);

// Flatten an HTA and write the data to an one dimensional array
void HTA_to_array(HTA *h, void* array);

// A sequential function that go through all elements in an HTA
// and set initial value to them
void HTA_init_all_scalars(HTA *h, void* initval);

/** 
 *  Get scalar element size of a certain HTA_SCALAR_TYPE
 *  @param scalar_t Some HTA_SCALAR_TYPE
 *  @return An integer number of the size in bytes
 */
size_t HTA_get_scalar_size(const HTA *h);

// recursively collects pointers to HTA data structure in the same level
void HTA_collect_tiles(int level, HTA *h, HTA** collect, int *count);

void* HTA_get_ptr_raw_data(const HTA *h);

// ------------------------------
// Accessors
// ------------------------------
/// This function selects a HTA tile specified by the
/// accessor tuples and return a pointer. It's different
/// from HTA_access_tile() in that it doesn't retain the
/// selected HTA metadata. Thus, HTA_destroy() should not be
/// used on the pointer returned.
/// @param h The top level HTA node
/// @param accessor A sequence of tuples to specify the tile to select
/// @return A pointer to selected HTA
/// @see HTA_access_tile(), HTA_destroy()     
HTA* HTA_select(HTA *h, void** accessor, int togo);
HTA* HTA_pick_one_tile(const HTA* h, const Tuple *nd_idx);

// Flat access operation
// accessor to certain scaler(s)
void* HTA_access_element(HTA* h, Tuple** acc);

void* HTA_flat_access(HTA *h, const Tuple *global_nd_idx);


// ------------------------------
// HTA operations
// ------------------------------
// HTA_map operator function interfaces
// Common operations are declared in HTA_operations.h
typedef void (*H1Op)(HTA* h1);
typedef void (*H2Op)(HTA* h1, HTA* h2);
typedef void (*H3Op)(HTA* h1, HTA* h2, HTA* h3);
typedef void (*H4Op)(HTA* h1, HTA* h2, HTA* h3, HTA* h4);
typedef void (*H5Op)(HTA* h1, HTA* h2, HTA* h3, HTA* h4, HTA* h5);

typedef void (*H1S1Op)(HTA* h1, void *s1);
typedef void (*H2S1Op)(HTA* h1, HTA* h2, void *s1);
typedef void (*H3S1Op)(HTA *h1, HTA *h2, HTA *h3, void *s1);
typedef void (*H4S1Op)(HTA *h1, HTA *h2, HTA *h3, HTA *h4, void *s1);
typedef void (*H5S1Op)(HTA *h1, HTA *h2, HTA *h3, HTA *h4, HTA *h5, void *s1);

// Non-destructive map functions
// The src HTAs are read-only and the result is stored to dest
void HTA_map_h1(int level, H1Op op, HTA* h1);
void HTA_map_h2(int level, H2Op op, HTA* h1, HTA* h2);
void HTA_map_h3(int level, H3Op op, HTA* h1, HTA* h2, HTA* h3);
void HTA_map_h4(int level, H4Op op, HTA* h1, HTA* h2, HTA* h3, HTA* h4);
void HTA_map_h1s1(int level, H1S1Op op, HTA* h1, void* s1);
void HTA_map_h2s1(int level, H2S1Op op, HTA* h1, HTA* h2, void* s1);
void HTA_map_h3s1(int level, H3S1Op op, HTA* h1, HTA* h2, HTA* h3, void* s1);
void HTA_map_h4s1(int level, H4S1Op op, HTA* h1, HTA* h2, HTA* h3, HTA* h4, void* s1);
void HTA_map_h5s1(int level, H5S1Op op, HTA* h1, HTA* h2, HTA* h3, HTA* h4, HTA* h5, void* s1);

// HTA_tile_to_hta
// goes down the hierarchiy of src1 to the specific level and 
// for all tiles t in that level, map the custom operator op(t, src2)
// in parallel
void HTA_tile_to_hta(int level, H3Op h3op, HTA* dest, HTA* src1, HTA* src2);

/// Reductions
typedef void (*ReduceOp)(HTA_SCALAR_TYPE stype, void* result, void* ptr);
void HTA_full_reduce(ReduceOp op, void* result, HTA *h);

// Useful for compute L2 norm
void HTA_reduce_h2(ReduceOp rop, H2S1Op h2s1op, void* result, 
        HTA *h1, HTA *h2);

// For partial reduction
HTA* HTA_iterator_to_hta(HTA *h, Tuple *it);
HTA* HTA_partial_reduce(ReduceOp scalar_op, HTA *h, int dim_reduc, 
        void* initval);
void HTA_sequential_partial_reduce(ReduceOp r_op, HTA* h, HTA* r, int dim_reduc);
HTA* HTA_allocate_partial_reduce_storage(HTA *h, int dim_reduc, void *initval);
HTA** HTA_allocate_partial_reduce_temporary(HTA *h, int dim_reduc, void *initval);
void HTA_merge_partial_reduce_results(ReduceOp scalar_op, HTA *r, HTA* h, 
        int dim_reduc, HTA** ha1, void * initval) ;

void HTA_circshift(HTA *h, Tuple *dir);
void HTA_rerank(HTA *h);

// ------------------------------
// Sparse HTAs
// ------------------------------
HTA* HTA_sparse_create(int dim, int levels, const Tuple *flat_size, 
        int order, Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, ...);
HTA* HTA_sparse_create_with_ts(int dim, int levels, const Tuple *flat_size, 
        int order, Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, 
        Tuple *ts);
void HTA_init_sparse_leaf(HTA *h, int nnz, void* val, int* col_ind, 
        int* row_ptr);
void HTA_init_with_sparse_matrix(HTA *hs, int num_nz, int num_rows, void* val, int* col_ind, 
        int* row_ptr);
// ------------------------------
// Index Calculation
// ------------------------------
void HTA_leaf_local_to_global_nd_index(HTA *h, Tuple *tiling_iter, 
        const Tuple *leaf_local_iter, Tuple *global_nd_index);
/// Returns a pointer to the leaf tile for a tuple of given ND global
/// indices
HTA* HTA_locate_leaf_for_nd_index(HTA *h, const Tuple *nd_index, 
        int *leaf_offset);
/// It takes a tuple of ND global element indices and convert
/// it to ND tile indices and ND element indices within the tile
void HTA_nd_global_to_tile_index(const Tuple *flat_size, 
        const Tuple *nd_num_tiles, const Tuple *nd_global_index, 
        Tuple *tile_index, Tuple *nd_local_index);
// ------------------------------
// for PILHTA
// ------------------------------
int hta_main(int argc, char** argv);
void hta_init();
#endif

#ifndef __HTA_H__
#define __HTA_H__

/// @file HTA.h
#ifdef __cplusplus
extern "C" {
#endif

#include "Tuple.h"
#include "Region.h"
#include "Distribution.h"
#include "Alloc.h"
#include "Mapping.h"
#include "Debug.h"
#include "Types.h"
#include "Config.h"
#include "SPMD_macros.h"

#define HTA_ROOT_LEVEL(h) (0)
#define HTA_LEAF_LEVEL(h) (h->height - 1)
#define HTA_SCALAR_LEVEL(h) (h->height)

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#define LOCAL (HTA_get_rank())

#define HTA_master_printf(...) ((HTA_is_distributed() ? ((HTA_get_rank() == 0)?(printf(__VA_ARGS__)):((void)0)): printf(__VA_ARGS__)))
#define HTA_pprintf(rank, ...) ((HTA_is_distributed() ? ((HTA_get_rank() == rank)?(printf(__VA_ARGS__)):((void)0)): printf(__VA_ARGS__)))

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
    int order;
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
    /// The pid of the process where the data structure is stored
    /// This is added for convenience since there is no MPI_comm_rank() equivalent in PIL
    int pid;
    /// The home location (processor node number) of this tile. -1 means this tile is not leaf
    int home;

    /// ----------GLOBAL information-------------
    /// The rank of it among all HTAs at the same level (global)
    int rank;
    /// The nd rank of the HTA (it only apply to leaf tiles
    Tuple nd_rank;
    /// global tile dimensions at the same level
    /// for example, (5, 5) meanss there are 5x5 tiles at this level
    /// in a regular HTA all 25 tiles have the same nd_tile_dimensions
    Tuple nd_tile_dimensions;
    /// Global element start offset
    Tuple nd_element_offset;
    /// The global dimensions of the whole HTA
    /// This is added in order to support different layout of leaf tile
    Tuple global_dimensions;

    Dist dist;
    /// ----------GLOBAL information-------------
    /// The number of tiles immediately one level below
    int num_tiles;
    /// The dimension of flattened HTA
    Tuple flat_size;
    /// An array of <levels> Tuples
    Tuple tiling;
    /// tiles is a dynamically allocated array of pointers to all HTA metadata one level immediately below
    struct hta** tiles;
    /// The block id of the data tile. It's 0 for non-leaf HTA tiles.
    /// The pointer to leaf tile metadata
    Leaf leaf;

    /// This flag is to indicate if the HTA metadata is created by selection
    /// if it is true, the destroy function free the metadata used by the root
    /// HTA only and will not go recursively destroying childrens
    int is_selection;

#if ENABLE_PREPACKING
    int prepacked;
    int num_gpps;
    void* prepacked_data_array;
#endif

} HTA;


/// The new HTA_create takes the flatten HTA dimension size and the tiling of all levels
/// except for the scalar level, which is computed automatically by the library
/// The flat_size field in each HTA instance is used to record the dimensions if
/// the HTA is flattened. If this field is null pointer, the HTA is an exact sized one.
/// HTA HTA_create(int dim, int levels, Tuple flat_size, Tuple tiling, int order, Dist dist, HTA_SCALAR_TYPE scalar_type);
HTA* HTA_create(int dim, int levels, const Tuple *flat_size, int order, Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, ...);
HTA* HTA_create_shell(void* prealloc, int dim, int levels, const Tuple *flat_size, int order, Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, ...);

HTA* HTA_create_with_pid(int pid, int dim, int levels, const Tuple *flat_size, int order, Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, ...);
HTA* HTA_create_shell_with_pid(int pid, void* prealloc, int dim, int levels, const Tuple *flat_size, int order, Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, ...);

HTA* HTA_create_impl(int pid, void* prealloc, int dim, int levels, const Tuple *flat_size, int order, Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, Tuple *ts);
/// This API destroys an HTA tree by recursively traverse and invoke RefCount_release() on
/// each node.
/// @param h The pointer to the top level HTA metadata
void HTA_destroy(HTA *h);

/// Print HTA information for debugging
/// @param h The pointer to the top level HTA metadata
/// @param recursively 1 means go down the HTA tree recursively. 0 means only current level.
void HTA_print(HTA* h, int recursively);

// Initialize an empty HTA shell using data from existing matrix
void HTA_init_with_array(HTA *h, const void* array);

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
size_t HTA_size_of_scalar_type(HTA_SCALAR_TYPE t);

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
// HTA_***_element is used while accessing using a sequence of tuples
void HTA_read_element(HTA* h, Tuple** acc, void* buf);
void HTA_write_element(HTA* h, Tuple** acc, void* buf);
/// HTA_flat_*** is used when the global index of the scalar element is used
void HTA_flat_read(HTA *h, const Tuple *global_nd_idx, void* buf);
void HTA_flat_write(HTA *h, const Tuple *global_nd_idx, void* val);


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

// Required by HTA_collect_set_tiles
typedef int (*SelecOp)(HTA *h1, Tuple s);
void HTA_collect_set_tiles(int level, HTA *h, HTA** collect, int *count, SelecOp sel_op, Tuple t);

// Non-destructive map functions
// The src HTAs are read-only and the result is stored to dest
void HTA_map_h1(int level, H1Op op, HTA* h1);
void HTA_map_h2(int level, H2Op op, HTA* h1, HTA* h2);
void HTA_map_h3(int level, H3Op op, HTA* h1, HTA* h2, HTA* h3);
void HTA_map_h4(int level, H4Op op, HTA* h1, HTA* h2, HTA* h3, HTA* h4);
void HTA_map_h5(int level, H5Op op, HTA* h1, HTA* h2, HTA* h3, HTA* h4, HTA* h5);
void HTA_map_h1s1(int level, H1S1Op op, HTA* h1, void* s1);
void HTA_map_h2s1(int level, H2S1Op op, HTA* h1, HTA* h2, void* s1);
void HTA_map_h3s1(int level, H3S1Op op, HTA* h1, HTA* h2, HTA* h3, void* s1);
void HTA_map_h4s1(int level, H4S1Op op, HTA* h1, HTA* h2, HTA* h3, HTA* h4, void* s1);
void HTA_map_h5s1(int level, H5S1Op op, HTA* h1, HTA* h2, HTA* h3, HTA* h4, HTA* h5, void* s1);

// Applied map function to a group of tiles
void HTA_map_h1sel(int level, H1Op op, HTA* h1, SelecOp sel_op, Tuple selec);
void HTA_map_h2sel(int level, H2Op op, HTA* h1, HTA* h2, SelecOp sel_op, Tuple selec);
void HTA_map_h3sel(int level, H3Op op, HTA* h1, HTA* h2, HTA* h3, SelecOp sel_op, Tuple selec);
void HTA_map_h4sel(int level, H4Op op, HTA* h1, HTA* h2, HTA* h3, HTA* h4, SelecOp sel_op, Tuple selec);
void HTA_map_h5sel(int level, H5Op op, HTA* h1, HTA* h2, HTA* h3, HTA* h4, HTA *h5, SelecOp sel_op, Tuple selec);

// Map with explicitly selected tiles
// NOTICE: h1 & sel1 specify the LHS and it's where the computation happens in SPMD mode
void HTA_cmap_h2(int level, H2Op op, int len, HTA* h1, Tuple* sel1, HTA* h2, Tuple* sel2, int iter);
void HTA_cmap_h3(int level, H3Op op, int len, HTA* h1, Tuple* sel1, HTA* h2, Tuple* sel2, HTA* h3, Tuple* sel3, int iter);

// HTA_tile_to_hta
// goes down the hierarchiy of src1 to the specific level and
// for all tiles t in that level, map the custom operator op(t, src2)
// in parallel
void HTA_tile_to_hta(int level, H3Op h3op, HTA* dest, HTA* src1, HTA* src2);
void HTA_tile_to_hta2(int level, H2Op h2op, HTA* dest, HTA* src1);

/// Reductions
typedef void (*ReduceOp)(HTA_SCALAR_TYPE stype, void* result, void* ptr);
void HTA_full_reduce(ReduceOp op, void* result, HTA *h);

// Useful for compute L2 norm
void HTA_reduce_h2(ReduceOp rop, H2S1Op h2s1op, void* result,
        HTA *h1, HTA *h2);

// For partial reduction
HTA* HTA_iterator_to_hta(HTA *h, Tuple *it);
HTA* HTA_iterator_begin(HTA* h, Tuple* iter);
HTA* HTA_iterator_next(HTA* h, Tuple* iter);
HTA* HTA_partial_reduce(ReduceOp scalar_op, HTA *h, int dim_reduc,
        void* initval);
void HTA_sequential_partial_reduce(ReduceOp r_op, HTA* h, HTA* r, int dim_reduc, void* initval);
HTA* HTA_allocate_partial_reduce_storage(HTA *h, int dim_reduc, void *initval);
HTA* HTA_allocate_partial_reduce_storage_spmd(HTA *h, int dim_reduc, void *initval);
HTA** HTA_allocate_partial_reduce_temporary(HTA *h, int dim_reduc, void *initval);
HTA* HTA_allocate_partial_reduce_temporary_spmd(HTA *h, int dim_reduc, void *initval);
void HTA_merge_partial_reduce_results(ReduceOp scalar_op, HTA *r, HTA* h,
        int dim_reduc, HTA** ha1, void * initval) ;
void HTA_merge_partial_reduce_results_spmd(ReduceOp scalar_op, HTA *r, HTA* h, int dim_reduc, HTA *ht, void * initval);
void HTA_reduce_two_tiles(ReduceOp op, HTA* dest, HTA* src);
void HTA_partial_reduce_with_preallocated(ReduceOp fr_op, HTA* h1, int dim_reduc, void* s1, HTA** ha, HTA *h2);
void HTA_partial_reduce_with_preallocated_pil(ReduceOp fr_op, HTA * h1, int dim_reduc, void* s1, HTA** ha, HTA* h2);

void HTA_circshift(HTA *h, Tuple *dir);
void HTA_rerank(HTA *h);
void HTA_scan(HTA *h, ReduceOp op, void* initval);
void HTA_transpose(HTA * xout, HTA * xin, int from_upper, int from_lower);

// assignment operations
void HTA_set_abs_region_with_hta(HTA* h, Region* elem_sel);
void HTA_assign(int* lhs_sel, Region* lhs_elem_sel, HTA* lhs_hta, int* rhs_sel, Region* rhs_elem_sel, HTA* rhs_hta);
void HTA_assign_with_region(Region* lhs_sel, Region* lhs_elem_sel, HTA* lhs_hta, Region* rhs_sel, Region* rhs_elem_sel, HTA* rhs_hta);
void HTA_flatten(void* out, Region* rhs_sel, Region* rhs_elem_sel, HTA* rhs_hta);

typedef int (*AssignOp)(Tuple *flat, Tuple *iter, int *idx);
void HTA_assign_custom(int* lhs_sel, AssignOp lhs_elem_sel, HTA* lhs_hta, int* rhs_sel, AssignOp rhs_elem_sel, HTA* rhs_hta);

// ------------------------------
// Sparse HTAs
// ------------------------------
HTA* HTA_sparse_create(int dim, int levels, const Tuple *flat_size,
        int order, Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, ...);
HTA* HTA_sparse_create_with_pid(int pid, int dim, int levels, const Tuple *flat_size,
        int order, Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, ...);
HTA* HTA_sparse_create_impl(int pid, int dim, int levels, const Tuple *flat_size,
        int order, Dist *dist, HTA_SCALAR_TYPE scalar_type, int num_tuples, Tuple *ts);
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
int hta_main(int argc, char** argv, int pid);
void hta_init();

// ------------------------------
// For distributed
// ------------------------------
int HTA_get_rank();
int HTA_get_num_processes();
int HTA_is_distributed();
void HTA_exit(int status);

void HTA_to_array_distributed(HTA *h, void* array);

// ------------------------------
// For shared memory SPMD
// ------------------------------
// Automatically compute a virtual process mesh shape for given dimension
Tuple HTA_get_vp_mesh(int dim);

// Fill boolean array selecting elements along direction "dim" for "iter" wavefront iteration
void HTA_fill_boolean_array(HTA *h, int dim, int type, int propagation, int iter, int *sel);
/// Global barrier of all processes (nop for fork join mode)
void HTA_barrier(int pid);
/// Make the leaf shared among all threads
void HTA_make_shared_leaf(HTA *h);
/// Make all the leaves shared among all threads
void HTA_make_shared_all_leaves(HTA *h);
/// Returns leaves owned by pid, and its number for the rest of processes
void HTA_collect_leaves_pid(int level, HTA *h, Leaf* collect, int *count, int pid);
/// Assign ptrs to leaves owned by pid
void HTA_set_ptr_to_leaves_pid(int level, HTA *h, Leaf* collect, int *count, int pid);

#ifdef __cplusplus
}
#endif
#endif

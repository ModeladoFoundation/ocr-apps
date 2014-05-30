#ifndef __TUPLE_H__
#define __TUPLE_H__

#ifndef HTA_MAX_DIM
#define HTA_MAX_DIM (4)  // Use static value for the maximum number of dimensions for HTA library
                         // to prevent the need of using dynamically allocated arrays
                         // Re-compilation is needed if larger number of dimensions is needed
#endif

/* A tuple is an n-dimensional index values (Bikshandi 2007) */
typedef struct tuple {
    /// This field is used for runtime introspection in HTA_access
    int accessor_type;
    int dim;        // dimension
    int height; 
    int values[HTA_MAX_DIM];   // pointer to an array of elements in the tuple    
} Tuple;

// ------------------------------------------
// Creation
// ------------------------------------------
// Tuple init functions should be preferred
void Tuple_init(Tuple* new_tuple, int dim, ...);
void Tuple_init_zero(Tuple *new_tuple, int dim );
Tuple Tuple_create(int dim, ...);
void Tuple_fill_in_sequence(Tuple* t, int num_tuples, ...);

// ------------------------------------------
// Utility functions
// ------------------------------------------
/// Compute the product of numbers in a tuple
int Tuple_product(const Tuple *t);

/// Print Tuple content
void Tuple_print(const Tuple *t);

/// Count the number of total elements at a certain depth
/// @param depth The number of levels to go down
/// @param t The tuple sequence to count
int Tuple_count_elements(const Tuple* t, int depth);

// count dimension sizes separately and save the result to a tuple
void Tuple_get_size_of_dimensions(Tuple *t, int depth, Tuple *result);
// ------------------------------------------
// Modifiers
// ------------------------------------------
/// Add values of s to d
void Tuple_add_dimensions(Tuple *d, const Tuple *s);
/// Overwrite values of d with values of s assuming the number of dimensions is the same
void Tuple_overwrite_values(Tuple *d, const Tuple *s);
/// Set all values to 0
void Tuple_clear_value(Tuple *t);
/// Copy the contents of tuple array s to tuple array t
void Tuple_clone_array(Tuple *d, const Tuple *s);

// ------------------------------------------
// Index computation
// ------------------------------------------

/// Convert ND index to 1D index
int Tuple_nd_to_1d_index(const Tuple *nd_idx, const Tuple *nd_size);

/// Convert 1D index to ND index for given ND dimension size
void Tuple_1d_to_nd_index(int idx, const Tuple *nd_size, Tuple *nd_idx);

/// To use iterator functions, create an array of tuples as a stack 
/// variable before calling Tuple_iterator_begin for best performance
void Tuple_iterator_begin(int dim, const int num_tuples, Tuple* iter);

/// Increment the iterator
int Tuple_iterator_next(const Tuple* tiling, Tuple* iter);

/// Return the nd global offset of the specified tile
void Tuple_get_tile_start_offset(Tuple* flat_size, Tuple* tiling, Tuple *iter, Tuple *nd_offset);

// ------------------------------------------
// Tile size computation
// ------------------------------------------
/// Compute leaf tile size using block decomposition algorithm
void Tuple_get_leaf_tile_size(const Tuple *flat_size, const Tuple* tiling, const Tuple* iter, Tuple* tile_size);
void Tuple_get_lower_tile_size(const Tuple *nd_idx, const Tuple *nd_size, const Tuple *flat_size, Tuple* tile_size, Tuple* element_offset);

// For partial reduction
// Set all t[i].value[dim_reduc] = value
void Tuple_set_tuples_dim(Tuple *t, int dim_reduc, int value);
#endif

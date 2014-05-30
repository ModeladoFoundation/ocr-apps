#ifndef __REFCOUNT_H__
#define __REFCOUNT_H__
#include <stdio.h>
/// @file RefCount.h

/// RefCount is simply a field that stores number of references
/// of an object allocated using RefCount_alloc().
/// @see RefCount_alloc()
typedef struct {
    unsigned int refcount;
} RefCount;

/// This function allocates memory space of specified size plus
/// a counter to count the references.
/// @param size The size of memory space needed
/// @return A memory pointer to the newly allocated memory object
void* RefCount_alloc(size_t size);

/// This function adds 1 to the counter of a reference counted object.
/// @param ptr Pointer to the reference counted object
/// @return Return value is the counter value after increment
int RefCount_retain(void *ptr);

/// This function decrement the reference counter by 1 and free memory
/// space if counter value reaches zero
/// @param ptr Pointer to the reference counted object
/// @return Returns 1 if the memory space is freed. Returns 0 otherwise. 
int RefCount_release(void *ptr);

/// Gets the reference counter value of the specified reference counted
/// object.
/// @param ptr Pointer to the reference counted object
/// @return Return value is the current reference counter value
int RefCount_get_counter(void *ptr);

/// The number of reference counted objects alive in the process.
/// This function is for debugging purposes.
/// @return The total number of reference counted objects in the process.
unsigned int RefCount_num_alive();

#endif

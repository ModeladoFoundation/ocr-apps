
#ifndef DBALLOCATOR_HPP
#define DBALLOCATOR_HPP

#include "common.h"
#include <memory>

namespace ompss {

template < typename T >
struct DatablockAllocator : public std::allocator<T> {
    T* allocate( size_t n ) {
        return ompss_malloc( n * sizeof(T) );
    }

    void deallocate( T* p, size_t n ) {
        ompss_free( p );
    }
};

} // namespace ompss

#endif


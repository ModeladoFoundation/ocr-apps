
#ifndef UTIL_H
#define UTIL_H

#include <cstdint>
#include <cstddef>

namespace mem {

template< typename T >
T* align( void* ptr ) {
    const uintptr_t p = reinterpret_cast<uintptr_t>(ptr);
    const uintptr_t aligned = (p + alignof(T) - 1u) & (-alignof(T));
    return reinterpret_cast<T*>(aligned);
}

template< typename T >
T* align( T* ptr ) {
    const uintptr_t p = reinterpret_cast<uintptr_t>(ptr);
    const uintptr_t aligned = (p + alignof(T) - 1u) & (-alignof(T));
    return reinterpret_cast<T*>(aligned);
}

// Returns the minimum value multiple of sizeof(T) bytes
// that can hold the memory contents between first and last
// pointers
template< typename T, typename U, typename V >
ptrdiff_t distance( const U* first, const V* last ) {
    ptrdiff_t distance = reinterpret_cast<const uint8_t*>(first)
                         - reinterpret_cast<const uint8_t*>(last);
    return distance/sizeof(T) + (distance%sizeof(T) > 0? 1: 0);
}

// Returns the size of a given type in terms of
// the size of another
template< typename T, typename U >
constexpr size_t sizeofInItems() {
    return sizeof(U)/sizeof(T) + (sizeof(U)%sizeof(T) > 0? 1: 0);
}

} // namespace mem

#endif // UTIL_H


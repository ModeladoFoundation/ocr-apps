
#ifndef LIFO_ARENA_H
#define LIFO_ARENA_H

#include <type_traits>

namespace buffered_alloc {

template< size_t alignment, size_t size >
struct lifo_allocator_arena {
    typedef typename std::aligned_storage<size,alignment>::type buffer_type;

    lifo_allocator_arena() :
        _storage(),
        _free( &_storage )
    {
    }

    size_t used_capacity() const {
        return reinterpret_cast<const uint8_t*>(_free)
            - reinterpret_cast<const uint8_t*>(&_storage);
    }

    size_t available_capacity() const {
        return sizeof(buffer_type) - used_capacity();
    }

    void* data() {
        return static_cast<void*>(&_storage);
    }

    // Assumes all memory is available again
    void clear() {
        _free = &_storage;
    }

    buffer_type _storage;
    void*       _free;
};

} // namespace buffered_alloc

#endif // ARENA_H


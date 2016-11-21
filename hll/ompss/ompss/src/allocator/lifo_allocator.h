
#ifndef LIFO_ALLOCATOR_H
#define LIFO_ALLOCATOR_H

#include "lifo_arena.h"

#include <cassert>
#include <memory>

namespace buffered_alloc {

template <typename Tp, size_t alignment, size_t size>
struct lifo_allocator {
    typedef Tp value_type;
    typedef lifo_allocator_arena<alignment,size> arena_type;

    template <typename T>
    struct rebind { typedef lifo_allocator<T,alignment,size> other; };

	lifo_allocator( arena_type& arena ) :
		_arena( arena )
	{
	}

    template <typename T>
    lifo_allocator( const lifo_allocator<T,alignment,size>& other ) :
        _arena( other._arena )
    {
    }

	Tp* allocate( std::size_t n, std::size_t align = alignof(Tp) ) noexcept {
        const ptrdiff_t align_req = std::max( align, alignof(ptrdiff_t) );
        const size_t    alloc_size = n * sizeof(Tp);

        Tp* result = nullptr;
        size_t space = _arena.available_capacity() - sizeof(ptrdiff_t);

        // Base pointer reserves enough space to store alignment offset
        void* base = reinterpret_cast<uint8_t*>(_arena._free) + sizeof(ptrdiff_t);
        // Align base pointer to alignof(Tp)
        void* aligned = base;
        if( std::align(align_req, alloc_size, aligned, space) ) {
            // Store alignment offset just before aligned buffer
            static_cast<ptrdiff_t*>(aligned)[-1] = (uint8_t*)aligned - (uint8_t*)base;
            // Update result pointer
            result = static_cast<Tp*>(aligned);
            // Update arena's free pointer
            _arena._free = static_cast<uint8_t*>(aligned) + alloc_size;
        }
        return result;
	}

#if 0 // Avoid usage of exceptions
    Tp* allocate( std::size_t n, std::size_t align = alignof(Tp) ) {
        Tp* result = allocate( std::nothrow, n, align );
        if( !result )
            throw std::bad_alloc();
        return result;
    }
#endif

	void deallocate( Tp* ptr, std::size_t n = 0u ) {
        ptrdiff_t offset = sizeof(ptrdiff_t) + reinterpret_cast<ptrdiff_t*>(ptr)[-1];
        // Update arena's free pointer
        _arena._free = reinterpret_cast<uint8_t*>(ptr) - offset;
	}

private:
    template< typename T, size_t al, size_t sz>
    friend struct lifo_allocator;

    arena_type& _arena;
};

} // namespace buffered_alloc

#endif // LIFO_ALLOCATOR_H


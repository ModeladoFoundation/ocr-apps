
#ifndef FALLBACK_ALLOCATOR_H
#define FALLBACK_ALLOCATOR_H

#include "memory/util.h"

#include <type_traits>
#include <ocr.h>

template <typename Tp>
class db_allocator {
public:
    typedef Tp value_type;

    template <typename T>
    struct rebind {
        typedef db_allocator<T> other;
    };

    db_allocator() = default;

    template <typename T>
    db_allocator( const db_allocator<T>& other )
    {
    }

	Tp* allocate( std::size_t n ) noexcept {
        ocrGuid_t* buffer;
        ocrGuid_t datablock;
        const size_t size = n*sizeof(Tp) + sizeof(ocrGuid_t) + alignof(ocrGuid_t) - 1u;
        u8 err = ocrDbCreate(&datablock, (void**)&buffer, size,
                         /*flags=*/0, /*loc=*/NULL_HINT, NO_ALLOC);
        ASSERT( err == 0 );

        buffer = mem::align( buffer );
        buffer[0] = datablock;
        return reinterpret_cast<Tp*>(&buffer[1]);
	}

	void deallocate( Tp* ptr, std::size_t n ) {
        ocrGuid_t* buffer = reinterpret_cast<ocrGuid_t*>(ptr);
        u8 err = ocrDbDestroy( buffer[-1] );
        ASSERT( err == 0 );
	}

};

#endif // FALLBACK_ALLOCATOR_H


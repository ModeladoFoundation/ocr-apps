
#ifndef FALLBACK_ALLOCATOR_H
#define FALLBACK_ALLOCATOR_H

#include "../common.h"

#include <type_traits>

template <typename Tp>
class db_allocator {
public:
    typedef Tp value_type;

    template <typename T>
    struct rebind {
        typedef db_allocator<T> other;
    };

    db_allocator()
    {
    }


    template <typename T>
    db_allocator( const db_allocator<T>& other )
    {
    }

	Tp* allocate( std::size_t n ) noexcept {
        return static_cast<Tp*>(
            ompss_malloc(n * sizeof(Tp) )
        );
	}

	void deallocate( Tp* ptr, std::size_t n ) {
        ompss_free(ptr);
	}

};

#endif // FALLBACK_ALLOCATOR_H


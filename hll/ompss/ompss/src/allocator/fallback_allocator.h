
#ifndef FALLBACK_ALLOCATOR_H
#define FALLBACK_ALLOCATOR_H

#include "firstfit_allocator.h"
#include <type_traits>

template <typename Tp, template<class> class Allocator = std::allocator >
class fallback_allocator {
public:
    typedef Tp value_type;

    template <typename T>
    struct rebind {
        typedef fallback_allocator<T,Allocator> other;
    };

	fallback_allocator( void* arena, size_t size, Allocator<Tp> fallback  = Allocator<Tp>() ) :
		_main(arena,size),
        _fallback(fallback),
        _begin(static_cast<uint8_t*>(arena)),
        _end(_begin+size)
	{
	}

    template <typename T>
    fallback_allocator( const fallback_allocator<T,Allocator>& other ) :
        _main( other._main ),
        _fallback( other._fallback ),
        _begin( other._begin ),
        _end( other._end )
    {
    }

	Tp* allocate( std::size_t n, const std::nothrow_t& tag ) noexcept {
        Tp* result = _main.allocate(n,tag);
        if( !result ) {
           result = _fallback.allocate(n,tag);
        }
        return result;
	}

#ifndef RELEASE
    Tp* allocate( std::size_t n ) {
        Tp* result = _main.allocate(n, std::nothrow);
        if( !result ) {
            result = _fallback.allocate(n);

            if( !result ) {
                throw std::bad_alloc();
            }
        }
        return result;
    }
#endif

	void deallocate( Tp* ptr, std::size_t n ) {
        if( fallback_allocated(ptr) ) {
            _fallback.deallocate(ptr,n);
        } else {
            _main.deallocate(ptr,n);
        }
	}

private:
    template < typename T, template <class> class A >
    friend struct fallback_allocator;

    bool fallback_allocated( Tp* ptr ) {
        // std::less<T*> required for memory order comparisons
        std::less<uint8_t*> lessThan;
        uint8_t* p = static_cast<uint8_t*>((void*)ptr);
        bool result = lessThan(p,_begin) || (!lessThan(p,_end));
        return result;
    }

    inplace::firstfit_allocator<Tp> _main;
    uint8_t*              _begin;
    uint8_t*              _end;
    Allocator<Tp>         _fallback;
};

#endif // FALLBACK_ALLOCATOR_H


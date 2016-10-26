
#ifndef FIRSTFIT_ALLOCATOR_H
#define FIRSTFIT_ALLOCATOR_H

#include "alloc_node.h"
#include "debug.h"

#include <cassert>

namespace inplace {

template <typename Tp>
struct firstfit_allocator {
    typedef Tp value_type;

    template <typename T>
    struct rebind { typedef firstfit_allocator<T> other; };

	firstfit_allocator(void* arena, size_t size) :
		_free(new (arena) free_node_base())
	{
        assert( size > sizeof(free_node_base) );

        size = size - sizeof(free_node_base) - sizeof(free_node);
        _free->insert_after( new(_free+1) free_node(size) );
	}

    template <typename T>
    firstfit_allocator( const firstfit_allocator<T>& other ) :
        _free( other._free )
    {
    }

	Tp* allocate( std::size_t n, const std::nothrow_t& tag ) noexcept {
		allocated_node* result = nullptr;
		free_node_base* current_node = _free;
		free_node* next_node = current_node->next();

		// Take in account node overheads. We must be able to restore
		// an allocated_node into a free_node of at least size = 0

		// newSize: minimum size to split a free_node in two
		const size_t newSize = n * sizeof(Tp) + sizeof(allocated_node);
		// replaceSize: minimum size to replace a free_node with an allocated one
		const size_t replaceSize = std::max( sizeof(free_node), newSize ) - sizeof(free_node);

		while (next_node && next_node->size() < replaceSize) {
			current_node = static_cast<free_node_base*>(next_node);
			next_node = next_node->next();
		}
		if (next_node) {
			// Found node
			if (next_node->size() >= newSize) {
				// Split when free space remains
				result = next_node->split(newSize);
			}
			else {
				// Replace when no space left on node
				current_node->remove_after();
				result = allocated_node::replace(next_node);
			}
		}
		return result? static_cast<Tp*>(result->data()) : nullptr;
	}

    Tp* allocate( std::size_t n ) {
        Tp* ptr = allocate(n, std::nothrow);
        if( !ptr )
            throw std::bad_alloc();
    }

	void deallocate( Tp* ptr, std::size_t n ) {
		std::less<free_node*> lowerThan;
		free_node* returned = free_node::replace(allocated_node::from_ptr(ptr));

		free_node* current = _free->next();
		if ( !current ) {
			_free->insert_after(returned);
		}
		else {
			free_node* next = current->next();
			// Must keep free list sorted
			while (next && lowerThan(next, returned)) {
				current = next;
				next = next->next();
			}
			current->try_join(returned);
		}
	}

private:
    template< typename T>
    friend struct firstfit_allocator;

    free_node_base* _free;
};

} // namespace inplace

#endif // FIRSTFIT_ALLOCATOR_H


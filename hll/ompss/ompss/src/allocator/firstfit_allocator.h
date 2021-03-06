
#ifndef FIRSTFIT_ALLOCATOR_H
#define FIRSTFIT_ALLOCATOR_H

#include "alloc_node.h"
#include "debug/fatal.h"
#include "debug/output.h"

#include <type_traits>
#include <cassert>

namespace buffered_alloc {

template< size_t alignment, size_t size >
struct firstfit_allocator_arena : public free_node_base {
    typedef typename std::aligned_storage<size,alignment>::type buffer_type;

    firstfit_allocator_arena() :
        free_node_base(),
        _storage()
    {
        log::verbose<log::Module::memory>( "Created arena {",
                  reinterpret_cast<void*>(&_storage), ", ",
                  reinterpret_cast<void*>((&_storage)+1), "}" );
        insert_after( new(&_storage) free_node(size_in_blocks(size)) );
    }

private:
    buffer_type _storage;
};

template <typename Tp>
struct firstfit_allocator {
    typedef Tp value_type;

    template< size_t alignment, size_t size >
    using arena_type = firstfit_allocator_arena<alignment,size>;

    template <typename T>
    struct rebind { typedef firstfit_allocator<T> other; };

    template< size_t alignment, size_t size >
	firstfit_allocator( firstfit_allocator_arena<alignment,size>& arena ) :
        _arena(arena)
	{
	}

    template <typename T>
    firstfit_allocator( const firstfit_allocator<T>& other ) :
        _arena( other._arena )
    {
    }

    // Intended only for debugging purposes.
    // Linear efficiency, so it is expensive...
    size_t available() {
        size_t amount = 0;
        free_node* next = _arena.next();
        while( next ) {
            amount += next->size() - 1;
            next = next->next();
        }
        return amount * sizeof(basic_block);
    }

    void dump_status() {
        free_node* next = _arena.next();
        while( next ) {
            log::verbose<log::Module::memory>( "   - { @:", static_cast<void*>(next),
                " [", next->data(), ", ", next->data(next->size()),
                "](", next->size(), ")}," );
            next = next->next();
        }
    }

	Tp* allocate( std::size_t n ) noexcept {
		allocated_node<Tp>* alloc_node = nullptr;
		free_node_base* current_node = &_arena;
		free_node* next_node = current_node->next();

        const size_t requested = size_in_blocks<Tp>(n);

		// Take in account node overheads. We must be able to restore
		// an allocated_node into a free_node of at least size = 0

		// replaceSize: minimum size to replace a free_node with an allocated one
		const size_t new_size = size_in_blocks<allocated_node<Tp>>() + requested;
		// newSize: minimum size to split a free_node in two
		const size_t split_size = new_size + size_in_blocks<free_node>();

		while (next_node && next_node->size() < new_size) {
			current_node = /*static_cast<free_node_base*>*/(next_node);
			next_node = next_node->next();
		}
		if (next_node) {
			// Found node
			if ( split_size <= next_node->size() ) {
				// Split when free space remains
				alloc_node = next_node->split<Tp>( new_size );
			} else {
				// Replace when no space left on node
				current_node->remove_after();
				alloc_node = allocated_node<Tp>::replace(next_node);
			}
		}

        dbg_check( alloc_node );
        Tp* result = alloc_node? alloc_node->data() : nullptr;

        log::verbose<log::Module::memory>( "Allocated node of size ", alloc_node->size(), " in address ", alloc_node );
        log::verbose<log::Module::memory>( "User data {",
            static_cast<void*>(result), ", ",
            static_cast<void*>(alloc_node->data(alloc_node->size())), " }" );
        log::verbose<log::Module::memory>( "Remaining size ", available() );

		return result;
	}

	void deallocate( Tp* ptr, std::size_t n ) {
        log::verbose<log::Module::memory>( "Deallocate addr ", static_cast<void*>(ptr) );

		free_node* returned = free_node::replace(allocated_node<Tp>::from_ptr(ptr));

		free_node* current = _arena.next();
		if ( !current ) {
			_arena.insert_after(returned);
		} else {
		    std::less<free_node*> lowerThan;
			free_node* next = current->next();
			// Must keep free list sorted
			while (next && lowerThan(next, returned)) {
				current = next;
				next = next->next();
			}
            current->insert_after(returned);
			current->try_join();
		}
        log::verbose<log::Module::memory>( "Remaining size ", available() );
        dump_status();
	}

private:
    template< typename T>
    friend struct firstfit_allocator;

    free_node_base& _arena;
};

} // namespace buffered_alloc

#endif // FIRSTFIT_ALLOCATOR_H


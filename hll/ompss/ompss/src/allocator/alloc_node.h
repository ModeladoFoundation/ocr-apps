
#ifndef ALLOC_NODE_H
#define ALLOC_NODE_H

#include "debug/fatal.h"
#include "memory/util.h"

#include <cstdint>
#include <cstddef>

#include <new>

namespace buffered_alloc {

template <typename T>
constexpr
inline bool is_aligned( const T* addr ) {
    return ( reinterpret_cast<uintptr_t>(addr) & (alignof(T)-1) ) == 0;
}

// A generic structure with
// alignof(void*) alignment
// restriction and size sizeof(void*)
class basic_block {
    void* _data;
};

template< typename T = uint8_t >
constexpr
inline size_t size_in_blocks( size_t n = 1 ) {
    return n == 0? 0 : (n*sizeof(T)-1)/sizeof(basic_block) + 1;
}

template<>
constexpr
inline size_t size_in_blocks<uint8_t>( size_t n ) {
    return n == 0? 0 : (n-1)/sizeof(basic_block) + 1;
}

struct free_node;

struct free_node_base {
	free_node_base() :
		_next(nullptr)
	{
	}

	free_node_base(free_node* next) :
		_next(next)
	{
	}

	free_node* next() {
		return _next;
	}

	void insert_after(free_node* n);

	void remove_after();

	free_node* _next;
};

template < typename T >
struct allocated_node {
    static_assert( alignof(basic_block) >= alignof(T), "Unsupported alignment restriction" );

    // Size must include allocated_node size
	allocated_node(size_t size) :
		_size(size)
	{
        dbg_check( is_aligned(this) );
	}

    // Returns the size in basic blocks of the
    // allocated_node plus the data buffer
	size_t size() const {
		return _size;
	}


	T* data( size_t offset ) {
		T* result = reinterpret_cast<T*>(
            reinterpret_cast<basic_block*>(this) + offset );

        dbg_check( is_aligned(result) );
        return result;
	}

	T* data() {
        T* result = reinterpret_cast<T*>(_data);

        dbg_check( is_aligned(result) );
		return result;
	}

	static allocated_node* from_ptr(void* ptr) {
		uintptr_t base = reinterpret_cast<uintptr_t>(ptr) - sizeof(allocated_node);
        allocated_node* result = reinterpret_cast<allocated_node*>(base);

        dbg_check( is_aligned(result) );
		return result;
	}

	//! Replaces a free node with an allocated node
	static allocated_node* replace(free_node* node);

	size_t      _size;
    basic_block _data[];
};

struct free_node : public free_node_base {
    // Size must include free_node size
    // and be specified in terms of basic blocks
	free_node(size_t size) :
		free_node_base(),
		_size(size)
	{
        dbg_check( is_aligned(this) );
	}

	free_node(size_t size, free_node* next) :
		free_node_base(next),
		_size(size)
	{
        dbg_check( is_aligned(this) );
	}

    // Returns the size in basic blocks of the
    // free_node plus the data buffer
	size_t size() const {
		return _size;
	}

    void* data() {
        return _data;
    }

	void* data( size_t offset ) {
		return reinterpret_cast<basic_block*>(this) + offset;
	}

	/*! Shrinks this node's size to create a new
	 * allocated node.
	 * Assumes requested size does not exceed
	 * free_node::_size (take into account allocated_node
	 * overhead).
	 * Make sure the amount of space requested is enough
	 * to replace this node with a free_node of size 0
	 */
    template< typename T >
	allocated_node<T>* split(size_t requested) {
        dbg_check( _size - size_in_blocks<free_node>() >= requested );

		_size -= requested;
		return new(data(_size)) allocated_node<T>(requested);
	}

	void try_join() {
        free_node* next_immediate = reinterpret_cast<free_node*>(data(_size));
        free_node* next = this->next();
        while (next == next_immediate) {
            // Remove node from list
            remove_after();
            // Increase this node size
			_size += next->_size;
            // and delete
			next->~free_node();

			// Keep propagating until no further joins are made
            next = this->next();
            next_immediate = reinterpret_cast<free_node*>(data(_size));
		}
	}

	static free_node* from_ptr(void* ptr) {
		uintptr_t base = reinterpret_cast<uintptr_t>(ptr) - sizeof(free_node);
        free_node* result = reinterpret_cast<free_node*>(base);

        dbg_check( is_aligned(result) );
		return result;
	}

	//! Replaces an allocated_node with a free_node
	template< typename T >
	static free_node* replace( allocated_node<T>* node );

	size_t      _size;
    basic_block _data[];
};

inline void free_node_base::insert_after(free_node* n) {
	n->_next = _next;
	_next = n;
}

inline void free_node_base::remove_after() {
	_next = _next->_next;
}

template < typename T >
inline allocated_node<T>* allocated_node<T>::replace(free_node* node) {
	size_t size = node->size();
	node->~free_node();
	return new(node) allocated_node<T>(size);
}

template< typename T >
inline free_node* free_node::replace(allocated_node<T>* node) {
	size_t size = node->size();
	node->~allocated_node<T>();
	return new(node) free_node(size);
}

} // namespace buffered_alloc

#endif // ALLOC_NODE_H



#ifndef ALLOC_NODE_H
#define ALLOC_NODE_H

#include "debug.h"

#include <cstdint>
#include <cstddef>

#include <new>

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

struct allocated_node {
	allocated_node(size_t size) :
		_size(size)
	{
	}

	size_t size() const {
		return _size;
	}

	void* data( size_t offset = 0 ) {
		return _data + offset;
	}

	static allocated_node* from_ptr(void* ptr) {
		char* base = static_cast<char*>(ptr) - sizeof(allocated_node);
		return (allocated_node*)base;
	}

	//! Replaces a free node with an allocated node
	static allocated_node* replace(free_node* node);

	size_t  _size;
    uint8_t _data[];
};

struct free_node : public free_node_base {
	free_node(size_t size) :
		free_node_base(),
		_size(size)
	{
	}

	free_node(size_t size, free_node* next) :
		free_node_base(next),
		_size(size)
	{
	}

	size_t size() const {
		return _size;
	}

	void* data( size_t offset = 0 ) {
		return _data + offset;
	}

	/*! Shrinks this node's size to create a new
	 * allocated node.
	 * Assumes requested size does not exceed
	 * free_node::_size (take into account allocated_node
	 * overhead).
	 * Make sure the amount of space requested is enough
	 * to replace this node with a free_node of size 0
	 */
	allocated_node* split(size_t requested) {
		_size -= requested + sizeof(allocated_node);
		return new(data(_size)) allocated_node(requested);
	}

	void try_join(free_node* node) {
		free_node* next_immediate = free_node::from_ptr(data(_size));
		if (node != next_immediate) {
			insert_after(node);
		} else {
			// Join is just increasing this node size
			// and destructing given node
			_size += node->_size + sizeof(free_node);
			node->~free_node();
			// Keep propagating until no further joins are made
			if( _next )
			    try_join(_next);
		}
	}

	static free_node* from_ptr(void* ptr) {
		char* base = static_cast<char*>(ptr) - sizeof(free_node);
		return (free_node*)base;
	}

	//! Replaces an allocated_node with a free_node
	static free_node* replace(allocated_node* node);

	size_t  _size;
    uint8_t _data[];
};

void free_node_base::insert_after(free_node* n) {
	n->_next = _next;
	_next = n;
}

void free_node_base::remove_after() {
	_next = _next->_next;
}

allocated_node* allocated_node::replace(free_node* node) {
	uint32_t new_size = node->size() + sizeof(free_node) - sizeof(allocated_node);
	node->~free_node();
	return new(node) allocated_node(new_size);
}

free_node* free_node::replace(allocated_node* node) {
	uint32_t new_size = node->size() + sizeof(allocated_node) - sizeof(free_node);
	node->~allocated_node();
	return new(node) free_node(new_size);
}

#endif // ALLOC_NODE_H

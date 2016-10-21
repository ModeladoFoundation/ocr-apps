
#ifndef SCRATCHPAD_H
#define SCRATCHPAD_H

#include <array>
#include <type_traits>

namespace mem {

struct node_base {
    node_base() :
        _next(nullptr)
    {
    }

    node_base( node_base* next ) :
        _next(next)
    {
    }

    void insert_after( node_base& node ) {
        node._next = _next;
        _next = &node;
    }

    void remove_after() {
        _next = _next->_next;
    }

    node_base* _next;
};

template < typename T >
struct node : public node_base {
    typedef typename std::aligned_storage<sizeof(T),alignof(T)>::type buffer_t;

    void* value() {
        return &_buffer;
    }

    buffer_t _buffer;
};

template < typename T, unsigned capacity >
struct scratchpad : public node_base {

    scratchpad() :
        node_base( &_memory[0] )
    {
        for( unsigned i = 0; i < capacity-1; i++ )
            _memory[i]._next = &_memory[i+1];
    }

    T* get_chunk() {
        void* chunk = nullptr;
        if( _next ) {
            chunk = static_cast<void*>(
                &static_cast<node<T>*>(_next)->_buffer );
            remove_after();
        }
        return static_cast<T*>(chunk);
    }

    void return_chunk( T* chunk ) {
        uintptr_t first = (uintptr_t)&_memory[0]._buffer;
        uintptr_t returned = (uintptr_t)chunk;
        unsigned item = (returned-first)/sizeof(node<T>);
        insert_after( _memory[item] );
    }

    std::array<node<T>,capacity> _memory;
};

} // namespace mem

#endif // SCRATCHPAD_H


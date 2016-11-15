
#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <iterator>

namespace mem {

struct Serializer {
    Serializer( void* base ) :
        _position(base)
    {
    }

    template<typename T>
    void write( const T& value ) {
        typedef T* pointer;
        typedef T  value_type;

        pointer destination = align<value_type>(_position);
        // Assign position to destination (alignment may introduce
        // some padding). Advance it one position.
        // Note that the size of any type is multiple of its alignment.
        // This is true even using -fpack-struct compiler flag.
        _position = destination + 1;

        std::uninitialized_copy( &value, (&value)+1, destination );
    }

    template<typename InputIterator>
    void write( InputIterator first, InputIterator last ) {
        typedef typename std::iterator_traits<InputIterator>::pointer pointer;
        typedef typename std::iterator_traits<InputIterator>::value_type value_type;
        typedef typename std::iterator_traits<InputIterator>::difference_type difference_type;

        pointer destination = align<value_type>(_position);
        difference_type elements = std::distance(first,last);

        if( first != last ) {
            // Assign position to last element
            // (alignment may introduce some padding).
            _position = destination + elements;

            std::uninitialized_copy( first, last, destination );
        }
    }

    template< typename T = uint8_t >
    void advance( size_t elements = 1 ) {
        _position = reinterpret_cast<T*>(_position) + elements;
    }

    const void* position() const {
        return _position;
    }

private:
    void* _position;
};

struct Deserializer {
    Deserializer( void* base ) :
        _position(base)
    {
    }

    template<typename T>
    T* read( size_t elements = 1 ) {
        typedef T* pointer;
        typedef T value_type;

        pointer source = align<value_type>(_position);
        if( elements > 0 ) {
            _position = source + elements;
        }

        return source;
    }

    template< typename T = uint8_t >
    void advance( size_t elements = 1 ) {
        _position = reinterpret_cast<T*>(_position) + elements;
    }

    const void* position() const {
        return _position;
    }

private:
    void* _position;
};

} // namespace mem

#endif // SERIALIZER_H


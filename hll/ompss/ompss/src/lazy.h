
#ifndef LAZY_H
#define LAZY_H

#include <memory>
#include <type_traits>

namespace ompss {

template < typename T >
struct IsGuidBased : public std::false_type {};

template<
    typename T,
    bool GuidBased = IsGuidBased<T>::value
>
class Lazy;

// Generic lazy initialization
// Relies on a boolean to record initialization status
// so it is not very efficient
template< typename T >
class Lazy<T,false> {
    typedef typename std::aligned_storage<alignof(T),sizeof(T)>::type Buffer;
public:
    Lazy() :
        _initialized(false)
    {
    }

    ~Lazy() {
        if( initialized() )
            reinterpret_cast<T*>(&_buffer)->~T();
    }

    T& operator=( const T& o ) {
        if( initialized() ) {
            (**this) = o;
        } else {
            new(&_buffer) T(o);
            _initialized = true;
        }
        return *this;
    }

    T& operator=( T&& o ) {
        if( initialized() ) {
            (**this) = std::forward(o);
        } else {
            new(&_buffer) T(std::forward(o));
            _initialized = true;
        }
        return *this;
    }

    template < typename... Args >
    void initialize( Args&&... args ) {
        ASSERT( !initialized() );
        new(&_buffer) T( std::forward(args)... );
        _initialized = true;
    }

    void reset() {
        if( initialized() )
            reinterpret_cast<T*>(&_buffer)->~T();
        _initialized = false;
    }

    operator T&() {
        return *reinterpret_cast<T*>(&_buffer);
    }

    operator const T&() const {
        return *reinterpret_cast<const T*>(&_buffer);
    }

    T& operator*() {
        return *reinterpret_cast<T*>(&_buffer);
    }

    const T& operator*() const {
        return *reinterpret_cast<const T*>(&_buffer);
    }

    T& operator->() {
        return *reinterpret_cast<T*>(&_buffer);
    }

    const T& operator->() const {
        return *reinterpret_cast<T*>(&_buffer);
    }

    bool initialized() const {
        return _initialized;
    }

private:
    Buffer _buffer;
    bool   _initialized;
};

// Lazy initialization for GUID based objects
// It relies on NULL_GUID special value to identify initialized
// objects, so it does not require use of booleans
// T must implement an explicit cast operator to ocrGuid_t
template< typename T >
class Lazy<T,true> {
    typedef typename std::aligned_storage<sizeof(T),alignof(T)>::type Buffer;
public:
    Lazy()
    {
        static_cast<T&>(*this).handle() = NULL_GUID;
    }

    ~Lazy() {
        if( initialized() )
            reinterpret_cast<T*>(&_buffer)->~T();
    }

    T& operator=( const T& o ) {
        if( initialized() ) {
            (**this) = o;
        } else {
            new(&_buffer) T(o);
        }
        return *this;
    }

    T& operator=( T&& o ) {
        if( initialized() ) {
            (**this) = std::forward<T>(o);
        } else {
            new(&_buffer) T(std::forward<T>(o));
        }
        return *this;
    }

    template < typename... Args >
    void initialize( Args&&... args ) {
        ASSERT( !initialized() );
        new(&_buffer) T( std::forward(args)... );
    }

    void reset() {
        if( initialized() )
            reinterpret_cast<T*>(&_buffer)->~T();
        static_cast<T&>(*this).handle() = NULL_GUID;
    }

    operator T&() {
        return *reinterpret_cast<T*>(&_buffer);
    }

    operator const T&() const {
        return *reinterpret_cast<const T*>(&_buffer);
    }

    T& operator*() {
        return *reinterpret_cast<T*>(&_buffer);
    }

    const T& operator*() const {
        return *reinterpret_cast<const T*>(&_buffer);
    }

    T& operator->() {
        return *reinterpret_cast<T*>(&_buffer);
    }

    const T& operator->() const {
        return *reinterpret_cast<T*>(&_buffer);
    }

    bool initialized() const {
        return !ocrGuidIsNull( static_cast<T>(*this).handle() );
    }

private:
    Buffer    _buffer;
};


} // namespace ompss

#endif


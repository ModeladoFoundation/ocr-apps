
#ifndef ACCESS_H
#define ACCESS_H

#include <cstdint>
#include <cstddef>

namespace ompss {

struct AccessBase {
    typedef uintptr_t Address;

    AccessBase( const void* base, size_t length ) :
        _begin( reinterpret_cast<Address>(base) ),
        _end( _begin + length )
    {
    }

    bool contains( Address addr ) const {
        return _begin <= addr && addr < _end;
    }

    bool overlaps( const AccessBase& other ) const {
        return _begin < other._end && other._begin < _end;
    }

    Address begin() const {
        return _begin;
    }

    Address end() const {
        return _end;
    }

private:
    Address _begin;
    Address _end;
};

enum class AccessTypes {
    read_only,
    read_write
};

template < AccessTypes type >
struct Access : public AccessBase {
    Access( const void* base, size_t length ) :
        AccessBase(base, length)
    {
    }
};

typedef Access<AccessTypes::read_only>  ReadAccess;
typedef Access<AccessTypes::read_write> WriteAccess;

} // namespace ompss

#endif // ACCESS_H


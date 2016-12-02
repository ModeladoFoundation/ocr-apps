
#ifndef DEPENDENCE_MAP_H
#define DEPENDENCE_MAP_H

#include "access.h"

#include "dependence_entry.h"

#include "allocator/db_allocator.h"
#include "allocator/firstfit_allocator.h"
#include "allocator/proxy_allocator.h"

#include <unordered_map>

namespace ompss {
// Dependence map
//#define TREE_TMP_ALLOC
//#define TREE_DB_ALLOC

typedef std::pair<AccessBase::Address,DependenceEntry> dependence_pair;
#if defined(TREE_TMP_ALLOC)
typedef buffered_alloc::firstfit_allocator<dependence_pair> tree_allocator;
#elif defined(TREE_DB_ALLOC)
typedef db_allocator<dependence_pair> tree_allocator;
#else
typedef std::allocator<dependence_pair> tree_allocator;
#endif

struct ExactMatchHash {
    // Discard least significant bits that don't conform a word
    size_t operator()( const AccessBase::Address& key ) const {
        return key>>4;
    }
};

typedef std::equal_to<AccessBase::Address> ExactMatchEqual;

typedef std::unordered_map<
    AccessBase::Address,
    DependenceEntry,
    ExactMatchHash,
    ExactMatchEqual,
    tree_allocator
> AccessMap;

// AccessMap/DependenceMap/whatever
struct AccessTracker {
    AccessTracker() :
        _accesses()
    {
    }

    template<AccessTypes type>
    void registerAccess( const Access<type>& newAccess, TaskDependences& deps );

    void clear() { _accesses.clear(); }

private:
    bool isConflict( const AccessBase& access, AccessMap::iterator& position ) {
        // If a {key,value} pair is found, there is a conflict
        return position != _accesses.end();
    }

    AccessMap::iterator findConflicts( const AccessBase& new_access ) {
        // Find conflicts using new_access begin address
        return _accesses.find( new_access.begin() );
    }

    DependenceEntry& getDependenceEntry( AccessMap::iterator position ) {
        return position->second;
    }

    template<AccessTypes type>
    AccessMap::iterator addEntry( AccessMap::iterator& position, const Access<type>& new_access ) {
        // Store entries using new_access' base address as key
        return _accesses.emplace_hint( position, std::piecewise_construct,
                            std::forward_as_tuple(new_access.begin()), std::forward_as_tuple<>() );
    }

    AccessMap _accesses;
};

template<AccessTypes type>
inline void AccessTracker::registerAccess( const Access<type>& new_access, TaskDependences& dependences ) {
    AccessMap::iterator conflict_iter = findConflicts(new_access);

    if( isConflict(new_access, conflict_iter) ) {
        DependenceEntry& entry = getDependenceEntry(conflict_iter);
        log::log<log::Module::dependences>( " +- Existing access @ ", &entry );

        entry.addDependence<type>( dependences );
        ++conflict_iter;
    } else {
        // Maybe open the section in DependenceEntry constructor?
        AccessMap::iterator position = addEntry( conflict_iter, new_access );
        DependenceEntry& entry = getDependenceEntry(position);

        log::log<log::Module::dependences>( " +- New access      @ ", &position->second );
        entry.openSection<type>( dependences );
    }
}

} // namespace ompss

#endif // DEPENDENCE_MAP_H


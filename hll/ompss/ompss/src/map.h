
#ifndef SET_H
#define SET_H

#include "dballocator.h"
#include "dependences_decl.h"

#include <map>

namespace ompss {

typedef uintptr_t        key_t;
typedef AccessDependence value_t;
typedef DatablockAllocator<std::pair<const key_t, value_t> > allocator_t;

typedef std::map< key_t, value_t, std::less<key_t>, allocator_t > hash_table_t;

//static inline value_t* hashTableGet( hash_table_t* table, key_t key )

static inline void newHashTable( hash_table_t* table ) {
    new(table) hash_table_t();
}

static inline void destructHashTable( hash_table_t* table ) {
    table->~hash_table_t();
}

} // namespace ompss

#endif


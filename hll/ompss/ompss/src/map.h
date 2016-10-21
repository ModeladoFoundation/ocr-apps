
#ifndef SET_H
#define SET_H

#include "dballocator.h"
#include "dependences_decl.h"

#include <map>

namespace ompss {

typedef uintptr_t        key_t;
typedef AccessDependence value_t;
typedef DatablockAllocator<std::pair<const key_t, value_t> > allocator_t;

typedef std::map< key_t, value_t, std::less<key_t>, allocator_t > DependenceMap;

} // namespace ompss

#endif


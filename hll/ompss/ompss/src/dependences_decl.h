
#ifndef DEPENDENCE_DECL_H
#define DEPENDENCE_DECL_H

#include "dballocator.h"
#include "event.h"
#include "lazy.h"

#include <ocr.h>

#include <vector>

namespace ompss {

struct AccessDependence {
    /*! Latch event that triggers when all read-only accesses are completed
     *  AND the read-only section is over (a write access
     *  has been registered). */
    Lazy<LatchEvent>  readCompleted;

    /*! Sticky event that triggers when a write-only or read-write access is
     *  completed. */
    Lazy<StickyEvent> writeCompleted;
};

typedef std::vector< ocrGuid_t,  DatablockAllocator<ocrGuid_t> >       GuidVector;
typedef std::vector< ocrEventTypes_t, DatablockAllocator<EventBase*> > TypeVector;

} // namespace ompss

#endif // DEPENDENCE_DECL_H


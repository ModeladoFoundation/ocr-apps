
#ifndef DEPENDENCE_ENTRY_DECL_H
#define DEPENDENCE_ENTRY_DECL_H

#include "task/task_fwd.h"

#include "dependences/task_dependences_decl.h"
#include "event.h"
#include "memory/lazy.h"

namespace ompss {

// Value of the Map
struct DependenceEntry {
    /*! Latch event that triggers when all read-only accesses are completed
     *  AND the read-only section is over (a write access
     *  has been registered). */
    mem::Lazy<ocr::LatchEvent>  readCompleted;

    /*! Sticky event that triggers when a write-only or read-write access is
     *  completed. */
    mem::Lazy<ocr::StickyEvent> writeCompleted;

    ~DependenceEntry();

    template <AccessTypes type>
    void openSection( TaskDependences& dependences );

    template <AccessTypes type>
    void addDependence( TaskDependences& dependences );
};

} // namespace ompss

#endif // DEPENDENCE_ENTRY_H



#ifndef DEPENDENCE_DECL_H
#define DEPENDENCE_DECL_H

#include "task_fwd.h"

#include "event.h"
#include "memory/lazy.h"

#include <nanos6_rt_interface.h>
#include <ocr.h>

#include <vector>

namespace ompss {

extern "C" typedef  void (*register_dep_funct_t)(void* handler, void* args_block);

struct TaskDependences {
    register_dep_funct_t   register_dependences;
    std::vector<ocrGuid_t> acquire;
    std::vector<ocrGuid_t> release;
    std::vector<uint8_t>   acq_satisfy;            // using a char avoids the vector overload for bool
    std::vector<uint8_t>   rel_destroy_not_satisfy;

    TaskDependences( nanos_task_info* task );
};

struct AccessDependence {
    /*! Latch event that triggers when all read-only accesses are completed
     *  AND the read-only section is over (a write access
     *  has been registered). */
    memory::Lazy<ocr::LatchEvent>  readCompleted;

    /*! Sticky event that triggers when a write-only or read-write access is
     *  completed. */
    memory::Lazy<ocr::StickyEvent> writeCompleted;

    void createReadSection();
    void readSectionAddReader( Task& task );
    void createWriteSection( Task& task );
    void addRAWDependence( Task& task );
    void addWAWDependence( Task& task );
    void addWARDependence( Task& task );
};


} // namespace ompss

#endif // DEPENDENCE_DECL_H



#ifndef OUTLINE_H
#define OUTLINE_H

#include "dependences.h"
#include "task.h"

extern ocrGuid_t taskOutlineTemplate;
extern ocrGuid_t cleanupTemplate;

ocrGuid_t edtOutlineWrapper( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] );

ocrGuid_t edtCleanup( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] );

ocrGuid_t edtShutdown( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] );

static inline void cleanUp( ompss::Task* task )
{
    // Release dependences
    releaseDependences( *task );

    // Check wether immediate clean-up is possible
    if( task->scope.flags.postponeCleanup ) {
        // Post-pone cleanup: other EDTs accesssing events
        ocrGuid_t cleanupDeps[2] = {
            getBufferDb(task), // Datablock containing Task
            getLocalScope().taskwaitEvent }; // Taskwait event

        ocrGuid_t cleanup_edt;
        u8 err = ocrEdtCreate( &cleanup_edt, cleanupTemplate,
                      0, NULL,
                      2, cleanupDeps,
                      EDT_PROP_NONE, NULL_HINT, NULL );
        ASSERT( err == 0);
    } else {
        ompss::Task::factory::destroy( task );
    }
}

#endif // OUTLINE_H


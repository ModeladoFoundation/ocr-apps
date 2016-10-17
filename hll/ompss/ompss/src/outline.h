
#ifndef OUTLINE_H
#define OUTLINE_H

#include "task.h"

extern ocrGuid_t taskOutlineTemplate;
extern ocrGuid_t cleanupTemplate;

ocrGuid_t edtOutlineWrapper( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] );

ocrGuid_t edtCleanup( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] );

ocrGuid_t edtShutdown( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] );

static inline void cleanUp( task_t* task )
{
    // Release dependences
    releaseDependences( task );

    // Check wether immediate clean-up is possible
    if( task->local_scope.flags.postpone_cleanup ) {
        // Post-pone cleanup: other EDTs accesssing events
        ocrGuid_t cleanupDeps[2] = {
            ((ocrGuid_t*)task)[-1],          // Datablock containing task_t
            getLocalScope()->taskwait_evt }; // Taskwait event

        ocrGuid_t cleanup_edt;
        u8 err = ocrEdtCreate( &cleanup_edt, cleanupTemplate,
                      0, NULL,
                      2, cleanupDeps,
                      EDT_PROP_NONE, NULL_HINT, NULL );
        ASSERT( err == 0);
    } else {
        destructTask( task );
    }
}

#endif // OUTLINE_H


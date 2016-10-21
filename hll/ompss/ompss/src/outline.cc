
#include <ocr.h>
#include <extensions/ocr-runtime-itf.h>

#include "dependences.h"
#include "hashtable.h"
#include "outline.h"
#include "task.h"
#include "task-local.h"

ocrGuid_t taskOutlineTemplate;
ocrGuid_t cleanupTemplate;

/*! \brief ocrEdt_t function wrapper
 * This function is used to encapsulate OmpSs compiler generated
 * outline functions into an EDT.
 * This EDT takes one parameter:
 *  - the outline function pointer
 * In addition, it takes at least one data dependence:
 *  - outline task's packed arguments passed using a datablock
 *
 *  At this point, no dependences are supported yet.
 */
ocrGuid_t edtOutlineWrapper( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    using namespace ompss;
    PROFILE_BLOCK;
    ASSERT( paramc == 0 );

    // Decode arguments and dependences
    Task* task = (Task*)getUserBuffer(depv[0].ptr);

    // Open taskwait region
    task->scope.taskwaitEvent++;

    // Store local scope in EDT local storage
    setLocalScope( task->scope );

    // Execute outline task
    task->definition.run( task->definition.arguments );

    // Close taskwait region
    task->scope.taskwaitEvent--;

    // Clean-up
    cleanUp( task );

    return NULL_GUID;
}

ocrGuid_t edtCleanup( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    using namespace ompss;
    PROFILE_BLOCK;
    // Decode arguments and dependences
    Task* task = (Task*)getUserBuffer(depv[0].ptr);
    Task::factory::destroy( task );
    return NULL_GUID;
}


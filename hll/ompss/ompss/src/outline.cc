
#include <ocr.h>
#include <extensions/ocr-runtime-itf.h>

#include "dependences.h"
#include "outline.h"
#include "task.h"
#include "task-local.h"

namespace ompss {

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

    // Decode arguments and dependences
    //Task* task = (Task*)getUserBuffer(depv[0].ptr);
    TaskDefinition* def = static_cast<TaskDefinition*>(
                            static_cast<void*>(paramv)
                          );

    // Store local scope in EDT local storage
    TaskScopeInfo scope;
    setLocalScope( scope );

    // Open taskwait region
    scope.taskwaitEvent++;

    // Execute outline task
    def->run( def->arguments.buffer );

    // Close taskwait region
    scope.taskwaitEvent--;

    // Clean-up
    cleanUp( NULL, scope );

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

} // namespace ompss


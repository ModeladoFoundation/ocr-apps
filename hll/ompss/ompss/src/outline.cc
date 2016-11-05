
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
ocrGuid_t edtOutlineWrapper( uint32_t paramc, uint64_t* paramv, uint32_t depc, ocrEdtDep_t depv[] )
{
    using namespace ompss;

    // Decode arguments and dependences
    std::tuple<TaskDefinition*,uint64_t,ocrGuid_t*,uint8_t*> args =
        Task::unpackParams( paramc, paramv );
    //Task* task = (Task*)getUserBuffer(depv[0].ptr);
    TaskDefinition* def = std::get<0>(args);

    // Store local scope in EDT local storage
    TaskScopeInfo scope;
    setLocalScope( scope );

    // Open taskwait region
    scope.taskwait.openRegion();

    // Execute outline task
    def->run( def->arguments.buffer );

    // Close taskwait region
    scope.taskwait.closeRegion();

    // Clean-up
    releaseDependences( std::get<1>(args), std::get<2>(args), std::get<3>(args) );

    return NULL_GUID;
}

ocrGuid_t edtCleanup( uint32_t paramc, uint64_t* paramv, uint32_t depc, ocrEdtDep_t depv[] )
{
    using namespace ompss;

    // Decode arguments and dependences
    Task* task = (Task*)getUserBuffer(depv[0].ptr);
    Task::factory::destroy( task );
    return NULL_GUID;
}

} // namespace ompss


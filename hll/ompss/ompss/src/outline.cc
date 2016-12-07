
#include "outline.h"
#include "task/task.h"

#include <ocr.h>
#include <extensions/ocr-runtime-itf.h>

namespace ompss {


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
    // Decode arguments and dependences
    std::tuple<TaskDefinition*,uint64_t,ocrGuid_t*,uint8_t*> args =
        Task::unpackParams( paramc, paramv );

    {
        // Store local scope in EDT local storage
        TaskScopeInfo scope;

        // Execute outline task
        TaskDefinition* def = std::get<0>(args);
        def->run( def->arguments.buffer );
    }

    // Clean-up
    releaseDependences( std::get<1>(args), std::get<2>(args), std::get<3>(args) );

    return NULL_GUID;
}

} // namespace ompss


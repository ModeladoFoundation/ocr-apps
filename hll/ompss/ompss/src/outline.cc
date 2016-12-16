
#include "debug/traceblock.h"
#include "outline.h"
#include "task/task.h"
#include "profile/profile.h"

#include <ocr.h>
#include <extensions/ocr-runtime-itf.h>

namespace ompss {

ocrGuid_t outlineTemplate;

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
ocrGuid_t outlineEdt( uint32_t paramc, uint64_t* paramv, uint32_t depc, ocrEdtDep_t depv[] )
{
    PROFILE_BLOCK( outlineEdt );

    // Decode arguments and dependences
    std::tuple<TaskDefinition*,TaskwaitEvent*,uint64_t,ocrGuid_t*> args =
        Task::unpackParams( paramc, paramv );

        TaskDefinition* def = std::get<0>(args);
    {
        // Store local scope in EDT local storage
        TaskScopeInfo scope( std::move(*std::get<1>(args)) );

        // Execute outline task
        PROFILE_BLOCK( ompss_user_code );
        def->run( def->arguments.buffer );
    }

    // Clean-up
    releaseDependences( std::get<2>(args), std::get<3>(args) );

    return NULL_GUID;
}

} // namespace ompss


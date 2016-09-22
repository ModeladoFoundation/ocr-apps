
#include <ocr.h>

#include "common.h"
#include "outline.h"
#include "task.h"

ocrGuid_t taskOutlineTemplate;

/*! \brief ocrEdt_t function wrapper
 * This function is used to encapsulate OmpSs compiler generated
 * outline functions into an EDT.
 * This EDT takes one parameter:
 *  - the outline function pointer
 * In addition, it takes at least one data dependence:
 *  - outline task's packed arguments passed using a datablock
 *
 *  At this point, no dependencies are supported yet.
 */
ocrGuid_t edtOutlineWrapper( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    ASSERT( paramc == 1 );

    // Decode arguments and dependencies
    task_definition_t* taskdef = (task_definition_t*)paramv[0];

    // Execute outline task
    taskdef->run_funct( taskdef->args_block );

    free( taskdef );
    return NULL_GUID;
}


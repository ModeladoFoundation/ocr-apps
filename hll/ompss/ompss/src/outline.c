
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
    ASSERT( paramc == 1 );

    // Decode arguments and dependences
    // First element of datablock is its own ocrGuid_t
    task_definition_t* taskdef = (task_definition_t*)(depv[0].ptr + sizeof(ocrGuid_t));

    // Create necessary edt-local data-structures
    // Note: we can store local dependences hash map
    // in the stack, since all the successor tasks will be
    // created during run_funct execution.
    setLocalDepMap( &taskdef->local_dependences );

    // Execute outline task
    taskdef->run_funct( taskdef->args_block );

    // Release dependences
    releaseDependences( taskdef );

    return NULL_GUID;
}

ocrGuid_t edtCleanup( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    // Decode arguments and dependences
    // First element of datablock is its own ocrGuid_t
    task_definition_t* taskdef = (task_definition_t*)(depv[0].ptr + sizeof(ocrGuid_t));

    destructTaskDefinition( taskdef );
    return NULL_GUID;
}


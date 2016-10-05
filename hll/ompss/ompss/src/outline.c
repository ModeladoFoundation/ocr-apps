
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
    task_t* task = (task_t*)(depv[0].ptr + sizeof(ocrGuid_t));

    // Create necessary edt-local data-structures
    // Note: we can store local dependences hash map
    // in the stack, since all the successor tasks will be
    // created during run_funct execution.
    setLocalScope( &task->local_scope );

    // Execute outline task
    task->definition.run( task->definition.arguments );

    // Release dependences
    releaseDependences( task );

    return NULL_GUID;
}

ocrGuid_t edtCleanup( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    // Decode arguments and dependences
    // First element of datablock is its own ocrGuid_t
    task_t* task = (task_t*)(depv[0].ptr + sizeof(ocrGuid_t));

    destructTask( task );
    return NULL_GUID;
}



#include "common.h"
#include "dependences.h"
#include "outline.h"
#include "task.h"
#include "task-local.h"

#include <nanos6_rt_interface.h>

#include <ocr.h>
#include <extensions/ocr-legacy.h>

/*! \brief Allocate space for a task and its parameters
 *
 *  This function creates a task and allocates space for its parameters.
 *  After calling it, the user code should fill out the block of data stored in args_block_pointer,
 *  and call nanos_submit_task with the contents stored in task_pointer.
 *
 *  \param[in] task_info a pointer to the nanos_task_info structure
 *  \param[in] task_invocation_info a pointer to the nanos_task_invocation_info structure
 *  \param[in] args_block_size size needed to store the paramerters passed to the task call
 *  \param[out] args_block_pointer a pointer to a location to store the pointer to the block of data that will contain the parameters of the task call
 *  \param[out] task_pointer a pointer to a location to store the task handler
 */
void nanos_create_task(
    nanos_task_info *task_info,
    nanos_task_invocation_info *task_invocation_info,
    size_t args_block_size,
    /* OUT */ void **args_block_pointer,
    /* OUT */ void **task_pointer
)
{
    task_t* task = newTask( task_info, args_block_size );

    *args_block_pointer = task->definition.arguments;
    *((task_t**)task_pointer) = task;
}


/*! \brief Submit a task
 *
 * This function should be called after filling out the block of parameters of the task. See nanos_create_task.
 *
 * \param[in] task The task handler
 */
void nanos_submit_task( void *handle )
{
    task_t* task = (task_t*)handle;

    // Create task EDT and its cleanup EDT
    ocrGuid_t edt, cleanup_edt;
    // Cleanup EDT will depend on task EDT
    // to be completed
    ocrGuid_t edt_finished_evt;

    // Register dependences
    task->definition.register_dependences( task, task->definition.arguments );
    u32 depc = 1 + getNumDependences( task );

    // Create EDT of finish type (does not return until
    // all its children EDTs are completed )
    u8 err = ocrEdtCreate( &edt, taskOutlineTemplate,
                  0, NULL,
                  depc, NULL,
                  EDT_PROP_FINISH, NULL_HINT, &edt_finished_evt );
    ASSERT( err == 0);

    // Feed EDT output event to taskwait latch event,
    // and increment latch's second pre-slot
    ocrGuid_t* taskwait_evt = &getLocalScope()->taskwait_evt;
    err = ocrEventSatisfySlot( *taskwait_evt, NULL_GUID, OCR_EVENT_LATCH_INCR_SLOT );
    ASSERT( err == 0 );
    err = ocrAddDependence( edt_finished_evt, *taskwait_evt,
                            OCR_EVENT_LATCH_DECR_SLOT, DB_DEFAULT_MODE );
    ASSERT( err == 0 );

    // Add edt dependences
    acquireDependences( edt, task );
}

/*! \brief Block the control flow of the current task until all of its children have finished
 *
 *  \param[in] invocation_source A string that identifies the source code location of the invocation
 */
void nanos_taskwait(char const *invocation_source)
{
    u8 err;
    // Prepare sticky event
    ocrGuid_t sticky_taskwait_evt;
    err = ocrEventCreate( &sticky_taskwait_evt, OCR_EVENT_STICKY_T, DB_PROP_NONE );
    ASSERT( err == 0 );

    // Get taskwait latch event and feed into sticky event
    ocrGuid_t* event = &getLocalScope()->taskwait_evt;
    err = ocrAddDependence( *event, sticky_taskwait_evt,
                            OCR_EVENT_LATCH_DECR_SLOT, DB_DEFAULT_MODE );
    ASSERT( err == 0 );

    // Close taskwait region
    err = ocrEventSatisfySlot( *event, NULL_GUID, OCR_EVENT_LATCH_DECR_SLOT );
    ASSERT( err == 0 );

    // Wait until all successors are completed
    err = ocrLegacyBlockProgress( sticky_taskwait_evt, NULL, NULL, NULL,
                                  LEGACY_PROP_NONE );
    ASSERT( err == 0 );

    // Destroy sticky event
    err = ocrEventDestroy( sticky_taskwait_evt );
    ASSERT( err == 0 );

    // Replace taskwait scope with a new one
    *event = NULL_GUID;
    err = ocrEventCreate( event, OCR_EVENT_LATCH_T, EVT_PROP_NONE );
    ASSERT( err == 0 );
    // Open next taskwait region
    err = ocrEventSatisfySlot( *event, NULL_GUID, OCR_EVENT_LATCH_INCR_SLOT );
    ASSERT( err == 0 );
}


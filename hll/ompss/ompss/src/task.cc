
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
    using namespace ompss;
    PROFILE_BLOCK;
    Task* task = Task::factory::construct( task_info, args_block_size );

    *args_block_pointer = task->definition->arguments.buffer;
    *((Task**)task_pointer) = task;
}


/*! \brief Submit a task
 *
 * This function should be called after filling out the block of parameters of the task. See nanos_create_task.
 *
 * \param[in] task The task handler
 */
void nanos_submit_task( void *handle )
{
    using namespace ompss;
    PROFILE_BLOCK;
    Task* task = (Task*)handle;

    // Create task EDT and its cleanup EDT
    ocrGuid_t edt;
    // Cleanup EDT will depend on task EDT
    // to be completed
    ocrGuid_t edtFinished;

    // Register dependences
    task->dependences.register_dependences( task, task->definition->arguments.buffer );

    uint32_t depc = task->dependences.acquire.size();
    ocrGuid_t* depv = task->dependences.acquire.data();

    // Create EDT of finish type (does not return until
    // all its children EDTs are completed )
    std::pair<uint32_t,uint64_t*> param = task->packParams();
    uint8_t err = ocrEdtCreate( &edt, taskOutlineTemplate,
                  param.first, param.second,
                  depc, depv,
                  EDT_PROP_FINISH, NULL_HINT, &edtFinished );
    ASSERT( err == 0);

    delete[] param.second;

    // Feed EDT output event to taskwait latch event,
    // and increment latch's second pre-slot
    ocr::LatchEvent& taskwaitEvent = getLocalScope().taskwaitEvent;
    taskwaitEvent++;
    taskwaitEvent.addDependence( edtFinished );

    // Add edt dependences
    acquireDependences( *task );

    Task::factory::destroy( task );
}

/*! \brief Block the control flow of the current task until all of its children have finished
 *
 *  \param[in] invocation_source A string that identifies the source code location of the invocation
 */
void nanos_taskwait(char const *invocation_source)
{
    using namespace ompss;
    PROFILE_BLOCK;
    // Prepare sticky event
    ocr::StickyEvent stickyTw;

    // Get taskwait latch event and feed into sticky event
    mem::Lazy<ocr::LatchEvent>& taskwaitEvent = getLocalScope().taskwaitEvent;
    stickyTw.addDependence( *taskwaitEvent );

    // Close taskwait region
    (*taskwaitEvent)--;

    // Wait until all successors are completed
    uint8_t err = ocrLegacyBlockProgress( stickyTw, NULL, NULL, NULL,
                                  LEGACY_PROP_NONE );
    ASSERT( err == 0 );

    // Replace taskwait scope with a new one
    taskwaitEvent.reset();
    // Open next taskwait region
    (*taskwaitEvent)++;
}


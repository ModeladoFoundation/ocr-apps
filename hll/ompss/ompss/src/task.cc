
#include "common.h"
#include "dependences.h"
#include "outline.h"
#include "task.h"
#include "task-local.h"
#include "taskwait.h"

#include <nanos6_rt_interface.h>

#include <ocr.h>

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

    Task* task = Task::factory::construct( task_info, args_block_size );

    *args_block_pointer = static_cast<void*>(task->definition.arguments.buffer);
    *reinterpret_cast<Task**>(task_pointer) = task;
}


/*! \brief Submit a task
 *
 * This function should be called after filling out the block of parameters of the task. See nanos_create_task.
 *
 * \param[in] task The task handler
 */
void nanos_submit_task( void *task )
{
    using namespace ompss;

    Task* new_task = static_cast<Task*>(task);

    // Create task EDT and its cleanup EDT
    ocrGuid_t edt;
    // Cleanup EDT will depend on task EDT
    // to be completed
    ocrGuid_t edtFinished;

    // Register dependences
    new_task->dependences.register_dependences(
        new_task,
        static_cast<void*>(new_task->definition.arguments.buffer) );

    uint32_t depc = new_task->dependences.acquire.size();
    ocrGuid_t* depv = new_task->dependences.acquire.data();

    // Create EDT of finish type (does not return until
    // all its children EDTs are completed )
    std::pair<uint32_t,uint64_t*> param = new_task->packParams();
    uint8_t err = ocrEdtCreate( &edt, taskOutlineTemplate,
                  param.first, param.second,
                  depc, depv,
                  EDT_PROP_FINISH, NULL_HINT, &edtFinished );
    ASSERT( err == 0);


    // Feed EDT output event to taskwait latch event,
    // and increment latch's second pre-slot
    getLocalScope().taskwait.registerEdt(edtFinished);

    // Add edt dependences
    acquireDependences( *new_task );

    Task::factory::destroy( new_task );
}

/*! \brief Block the control flow of the current task until all of its children have finished
 *
 *  \param[in] invocation_source A string that identifies the source code location of the invocation
 */
void nanos_taskwait(char const *invocation_source)
{
    using namespace ompss;

    // Wait until successors complete
    getLocalScope().taskwait.wait();

    // Clear dependence map
    getLocalScope().accesses.clear();
}


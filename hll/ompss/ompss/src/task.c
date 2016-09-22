
#include <nanos6_rt_interface.h>

#include <ocr.h>

#include "common.h"
#include "outline.h"
#include "task.h"

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
    task_definition_t* taskdef = newTaskDefinition( task_info, args_block_size );

    *args_block_pointer = taskdef->args_block;
    *((task_definition_t**)task_pointer) = taskdef;
}


/*! \brief Submit a task
 *
 * This function should be called after filling out the block of parameters of the task. See nanos_create_task.
 *
 * \param[in] task The task handler
 */
void nanos_submit_task(void *task)
{
    task_definition_t* taskdef = (task_definition_t*)task;

    // Register dependencies
    taskdef->dependencies_funct( task, taskdef->args_block );

    ocrGuid_t edt;
    // Create EDT
    u64 paramv[1];
    paramv[0] = (u64)task;
    u8 err = ocrEdtCreate( &edt, taskOutlineTemplate,
                  1, paramv,
                  1, &((ocrGuid_t*)taskdef)[-1],//taskdef->dependencies.size, (ocrGuid_t*)taskdef->dependencies.data,
                  EDT_PROP_NONE, NULL_HINT, NULL );
    ASSERT( err == 0);
}

/*! \brief Block the control flow of the current task until all of its children have finished
 *
 *  \param[in] invocation_source A string that identifies the source code location of the invocation
 */
void nanos_taskwait(char const *invocation_source)
{
}


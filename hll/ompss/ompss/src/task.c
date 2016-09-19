
#include <nanos6_rt_interface.h>

#include <assert.h>
#include <ocr.h>

#include "common.h"

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
    u8 err;

    // allocate args block
    ocrGuid_t args;
    *args_block_pointer = NULL;
    err = ocrDbCreate(&args, args_block_pointer, args_block_size,
                     /*flags=*/0, /*loc=*/NULL_HINT, NO_ALLOC);
    assert( err == 0 );
    assert( *args_block_pointer != NULL );

    // Task EDT parameters: function and arguments block
    u64 edtParams[1];
    edtParams[0] = (u64)task_info->run;

    ocrGuid_t edt;
    // Create EDT
    // 2 EDT arguments
    // 0 EDT dependencies (when supported, use EDT_PARAM_DEF)
    err = ocrEdtCreate( &edt, taskOutlineTemplate,
                  1, edtParams,
                  1, &args,
                  EDT_PROP_NONE, NULL_HINT, NULL );
    assert( err == 0);

    *((ocrGuid_t*)task_pointer) = edt;
}


/*! \brief Submit a task
 *
 * This function should be called after filling out the block of parameters of the task. See nanos_create_task.
 *
 * \param[in] task The task handler
 */
void nanos_submit_task(void *task)
{
}

/*! \brief Block the control flow of the current task until all of its children have finished
 *
 *  \param[in] invocation_source A string that identifies the source code location of the invocation
 */
void nanos_taskwait(char const *invocation_source)
{
}


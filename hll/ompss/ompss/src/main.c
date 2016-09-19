
#include <nanos6_rt_interface.h>

#include <assert.h>
#include <ocr.h>

#include "common.h"

extern int ompss_user_main( int argc, char* argv[] );

typedef void (*task_outline_funct_t)(void* args_block);

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
static ocrGuid_t edtOutlineWrapper( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    assert( paramc == 1 );
    assert( depc == 1 );

    // Decode arguments and dependencies
    task_outline_funct_t run_funct = (task_outline_funct_t)paramv[0];
    void* args_block = (void*)depv[0].ptr;

    // Execute outline task
    run_funct( args_block );

    ocrDbDestroy( depv[0].guid );
    return NULL_GUID;
}

//! \brief Initialize the runtime at least to the point that it will accept tasks
void nanos_preinit()
{
}

//! \brief Continue with the rest of the runtime initialization
void nanos_init()
{
    // Create EDT template
    u8 err = ocrEdtTemplateCreate( &taskOutlineTemplate, edtOutlineWrapper, 1, 1 );
    assert( err == 0);
}

//! \brief Wait until the the runtime has shut down
void nanos_wait_until_shutdown()
{
    // Cleanup. For the moment EDT templates are
    // used only once.
    u8 err = ocrEdtTemplateDestroy( taskOutlineTemplate );
    assert( err == 0);
}

//! \brief Notify the runtime that it can begin the shutdown process
void nanos_notify_ready_for_shutdown()
{
}

ocrGuid_t mainEdt(
            u32 paramc,
            u64* paramv,
            u32 depc,
            ocrEdtDep_t depv[] )
{
    nanos_preinit();
    nanos_init();

    void* program_args = depv[0].ptr;
    u64 argc = getArgc( program_args );
    char* argv[argc];
    for( u64 arg = 0; arg < argc; ++arg ) {
        argv[arg] = getArgv(program_args, arg);
    }

    int ret = ompss_user_main( (int)argc, argv );

    nanos_wait_until_shutdown();

    // If ret == 0, this is equivalent to ocrShutdown
    ocrAbort(ret);

    return NULL_GUID;
}


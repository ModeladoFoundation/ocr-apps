
#include <nanos6_rt_interface.h>

#include <ocr.h>

#include "common.h"
#include "outline.h"
#include "task.h"
#include "task-local.h"

extern int ompss_user_main( int argc, char* argv[] );

//! \brief Initialize the runtime at least to the point that it will accept tasks
void nanos_preinit()
{
}

//! \brief Continue with the rest of the runtime initialization
void nanos_init()
{
    // Create EDT template
    u8 err = ocrEdtTemplateCreate( &taskOutlineTemplate, edtOutlineWrapper, 1, EDT_PARAM_UNK );
    ASSERT( err == 0);
}

//! \brief Wait until the the runtime has shut down
void nanos_wait_until_shutdown()
{
    // Cleanup. For the moment EDT templates are
    // used only once.
    u8 err = ocrEdtTemplateDestroy( taskOutlineTemplate );
    ASSERT( err == 0);
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

    // Create necessary edt-local data-structures
    newHashTable( &main_data->local_dependences );
    setLocalDepMap( &main_data->local_dependences );

    int ret = ompss_user_main( (int)argc, argv );

    nanos_wait_until_shutdown();

    // If ret == 0, this is equivalent to ocrShutdown
    ocrAbort(ret);

    return NULL_GUID;
}


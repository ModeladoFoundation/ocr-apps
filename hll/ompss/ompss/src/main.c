
#include <nanos6_rt_interface.h>

#include <ocr.h>

#include "common.h"
#include "outline.h"
#include "task.h"
#include "task-local.h"

ocrGuid_t shutdownTemplate;
ocrGuid_t ompssMainTemplate;

extern int ompss_user_main( int argc, char* argv[] );

struct main_local_storage
{
    hash_table_t local_dependences;
    int return_value;
};

ocrGuid_t edtUserMain(
            u32 paramc,
            u64* paramv,
            u32 depc,
            ocrEdtDep_t depv[] )
{
    struct main_local_storage* main_data = depv[0].ptr;

    void* program_args = depv[1].ptr;
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

    return NULL_GUID;
}

ocrGuid_t edtShutdown(
            u32 paramc,
            u64* paramv,
            u32 depc,
            ocrEdtDep_t depv[] )
{
    // Decode task dependencies
    struct main_local_storage* tls = (struct main_local_storage*)(depv[0].ptr + sizeof(ocrGuid_t));
    int ret = tls->return_value;

    // Free task local data storage
    destructHashTable( &tls->local_dependences );
    ocrDbDestroy( depv[0].guid );

    // If ret == 0, this is equivalent to ocrShutdown
    ocrAbort(ret);

    return NULL_GUID;
}

ocrGuid_t mainEdt(
            u32 paramc,
            u64* paramv,
            u32 depc,
            ocrEdtDep_t depv[] )
{
    nanos_preinit();
    nanos_init();

    struct main_local_storage* tls =
        (struct main_local_storage*)malloc( sizeof(struct main_local_storage) );

    u8 err;
    ocrGuid_t userMainEdt, shutdownEdt;
    ocrGuid_t userMainDoneEvt;
    ocrGuid_t userMainDeps[2] = { ((ocrGuid_t*)tls)[-1] , depv[0].guid };
    err = ocrEdtCreate( &userMainEdt, ompssMainTemplate,
                  0, NULL,
                  2, userMainDeps,
                  EDT_PROP_FINISH, NULL_HINT, &userMainDoneEvt );
    ASSERT( err == 0 );

    ocrGuid_t shutdownDeps[2] = { ((ocrGuid_t*)tls)[-1], userMainDoneEvt };
    err = ocrEdtCreate( &shutdownEdt, shutdownTemplate,
                  0, NULL,
                  2, shutdownDeps,
                  EDT_PROP_NONE, NULL_HINT, NULL );
    ASSERT( err == 0 );

    return NULL_GUID;
}

//! \brief Initialize the runtime at least to the point that it will accept tasks
void nanos_preinit()
{
}

//! \brief Continue with the rest of the runtime initialization
void nanos_init()
{
    // Create EDT templates
    u8 err = ocrEdtTemplateCreate( &taskOutlineTemplate, edtOutlineWrapper, 0, EDT_PARAM_UNK );
    ASSERT( err == 0);

    err = ocrEdtTemplateCreate( &cleanupTemplate, edtCleanup, 0, 2 );
    ASSERT( err == 0);

    err = ocrEdtTemplateCreate( &ompssMainTemplate, edtUserMain, 0, 2 );
    ASSERT( err == 0);

    err = ocrEdtTemplateCreate( &shutdownTemplate, edtShutdown, 0, 2 );
    ASSERT( err == 0);
}

//! \brief Wait until the the runtime has shut down
void nanos_wait_until_shutdown()
{
}

//! \brief Notify the runtime that it can begin the shutdown process
void nanos_notify_ready_for_shutdown()
{
    // Do the shutdown right here
    u8 err = ocrEdtTemplateDestroy( taskOutlineTemplate );
    ASSERT( err == 0 );

    err = ocrEdtTemplateDestroy( cleanupTemplate );
    ASSERT( err == 0 );

    err = ocrEdtTemplateDestroy( ompssMainTemplate );
    ASSERT( err == 0 );

    err = ocrEdtTemplateDestroy( shutdownTemplate );
    ASSERT( err == 0 );
}


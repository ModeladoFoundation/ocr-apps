
#include <nanos6_rt_interface.h>

#include <ocr.h>

#include "common.h"
#include "outline.h"
#include "task.h"
#include "task-local.h"

ocrGuid_t shutdownTemplate;
ocrGuid_t ompssMainTemplate;

struct MainStorage {
    ompss::TaskScopeInfo scope;
    int                  exit_code;

    MainStorage() :
        scope(), exit_code(0)
    {
    }
};

extern "C" int ompss_user_main( int argc, char* argv[] );

ocrGuid_t edtUserMain(
            u32 paramc,
            u64* paramv,
            u32 depc,
            ocrEdtDep_t depv[] )
{
    PROFILE_BLOCK;

    void* program_args = depv[1].ptr;
    u64 argc = getArgc( program_args );
    char* argv[argc];
    for( u64 arg = 0; arg < argc; ++arg ) {
        argv[arg] = getArgv(program_args, arg);
    }

    // Initialize local scope
    MainStorage* main_data = new(getUserBuffer(depv[0].ptr)) MainStorage();

    // Open taskwait region
    main_data->scope.taskwaitEvent++;

    // Store local scope in EDT local storage
    setLocalScope( main_data->scope );

    // Call user's main function
    main_data->exit_code = ompss_user_main( (int)argc, argv );

    // Close taskwait region
    main_data->scope.taskwaitEvent--;

    nanos_wait_until_shutdown();

    return NULL_GUID;
}

ocrGuid_t edtShutdown(
            u32 paramc,
            u64* paramv,
            u32 depc,
            ocrEdtDep_t depv[] )
{
    PROFILE_BLOCK;
    // Decode task dependencies
    MainStorage* main_data = (MainStorage*)getUserBuffer(depv[0].ptr);
    int exit_code = main_data->exit_code;

    // Free task local data storage
    main_data->~MainStorage();
    ocrDbDestroy( depv[0].guid );

    // If exit_code == 0, this is equivalent to ocrShutdown
    ocrAbort( exit_code );

    return NULL_GUID;
}

// Nanos6 and OCR API implementations
extern "C" {

ocrGuid_t mainEdt(
            u32 paramc,
            u64* paramv,
            u32 depc,
            ocrEdtDep_t depv[] )
{
    PROFILE_BLOCK;
    nanos_preinit();
    nanos_init();


    u8 err;
    ocrGuid_t userMainEdt, shutdownEdt;
    ocrGuid_t userMainDoneEvt;
    ocrGuid_t userMainDeps[2] = { NULL_GUID, depv[0].guid };

    void* buffer;
    err = ocrDbCreate( &userMainDeps[0], &buffer, sizeof(MainStorage),
                  DB_PROP_NONE/*DB_PROP_NO_ACQUIRE*/, NULL_HINT, NO_ALLOC );
    ASSERT( err == 0 );

    err = ocrEdtCreate( &userMainEdt, ompssMainTemplate,
                  0, NULL,
                  2, userMainDeps,
                  EDT_PROP_FINISH, NULL_HINT, &userMainDoneEvt );
    ASSERT( err == 0 );

    ocrGuid_t shutdownDeps[2] = { userMainDeps[0], userMainDoneEvt };
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
    PROFILE_BLOCK;
}

//! \brief Continue with the rest of the runtime initialization
void nanos_init()
{
    PROFILE_BLOCK;
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
    PROFILE_BLOCK;
}

//! \brief Notify the runtime that it can begin the shutdown process
void nanos_notify_ready_for_shutdown()
{
    PROFILE_BLOCK;
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

} // extern "C"


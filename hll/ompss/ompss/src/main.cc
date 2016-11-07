
#include <nanos6_rt_interface.h>

#include <ocr.h>

#include "common.h"
#include "outline.h"
#include "task.h"
#include "task-local.h"

extern "C" int ompss_user_main( int argc, char* argv[] );

namespace ompss {

ocrGuid_t shutdownTemplate;
ocrGuid_t ompssMainTemplate;

struct MainStorage {
    int                  exit_code;

    MainStorage() :
        exit_code(0)
    {
    }
};

ocrGuid_t edtUserMain(
            uint32_t paramc,
            uint64_t* paramv,
            uint32_t depc,
            ocrEdtDep_t depv[] )
{
    using namespace ompss;

    void* program_args = depv[1].ptr;
    uint64_t argc = getArgc( program_args );
    char* argv[argc];
    for( uint64_t arg = 0; arg < argc; ++arg ) {
        argv[arg] = getArgv(program_args, arg);
    }

    // Initialize local scope
    TaskScopeInfo scope;
    MainStorage* main_data = new(getUserBuffer(depv[0].ptr)) MainStorage();

    // Open taskwait region
    scope.taskwait.openRegion();

    // Store local scope in EDT local storage
    setLocalScope( scope );

    // Call user's main function
    main_data->exit_code = ompss_user_main( (int)argc, argv );

    // Close taskwait region
    scope.taskwait.closeRegion();

    nanos_wait_until_shutdown();

    return NULL_GUID;
}

ocrGuid_t edtShutdown(
            uint32_t paramc,
            uint64_t* paramv,
            uint32_t depc,
            ocrEdtDep_t depv[] )
{
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

} // namespace ompss

// Nanos6 and OCR API implementations
extern "C" {

ocrGuid_t mainEdt(
            uint32_t paramc,
            uint64_t* paramv,
            uint32_t depc,
            ocrEdtDep_t depv[] )
{
    using namespace ompss;
    nanos_preinit();
    nanos_init();


    uint8_t err;
    ocrGuid_t userMainEdt, shutdownEdt;
    ocrGuid_t userMainDoneEvt;
    ocrGuid_t userMainDeps[2] = { NULL_GUID, depv[0].guid };

    // Create datablock that will contain
    // main function return value
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
}

//! \brief Continue with the rest of the runtime initialization
void nanos_init()
{
    using namespace ompss;

    // Create EDT templates
    uint8_t err = ocrEdtTemplateCreate( &taskOutlineTemplate, edtOutlineWrapper, EDT_PARAM_UNK, EDT_PARAM_UNK );
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
    using namespace ompss;
    // Do the shutdown right here
    uint8_t err = ocrEdtTemplateDestroy( taskOutlineTemplate );
    ASSERT( err == 0 );

    err = ocrEdtTemplateDestroy( cleanupTemplate );
    ASSERT( err == 0 );

    err = ocrEdtTemplateDestroy( ompssMainTemplate );
    ASSERT( err == 0 );

    err = ocrEdtTemplateDestroy( shutdownTemplate );
    ASSERT( err == 0 );
}

} // extern "C"



#include "common.h"
#include "memory/util.h"
#include "outline.h"
#include "task/task.h"

#include <nanos6_rt_interface.h>
#include <ocr.h>

extern "C" int ompss_user_main( int argc, char* argv[] );

namespace ompss {

ocrGuid_t shutdownTemplate;
ocrGuid_t ompssMainTemplate;

ocrGuid_t edtShutdown(
            uint32_t paramc,
            uint64_t* paramv,
            uint32_t depc,
            ocrEdtDep_t depv[] )
{
    // Decode task dependencies
    MainStorage* main_data = reinterpret_cast<MainStorage*>(paramv);
    int exit_code = main_data->exit_code;

    // Free task local data storage
    main_data->~MainStorage();

    nanos_notify_ready_for_shutdown();

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

    void* program_args = depv[0].ptr;
    uint64_t argc = getArgc( program_args );
    char* argv[argc];
    for( uint64_t arg = 0; arg < argc; ++arg ) {
        argv[arg] = getArgv(program_args, arg);
    }

    // Initialize local scope
    TaskScopeInfo scope;
    MainStorage main_data;

    // Open taskwait region
    scope.taskwait.openRegion();

    // Store local scope in EDT local storage
    setLocalScope( scope );

    // Call user's main function
    main_data.exit_code = ompss_user_main( static_cast<int>(argc), argv );

    // Create cleanup EDT
    ocrGuid_t shutdownEdt;
    uint8_t err = ocrEdtCreate( &shutdownEdt, shutdownTemplate,
        mem::sizeofInItems<uint64_t,MainStorage>(), reinterpret_cast<uint64_t*>(&main_data),
        1, const_cast<ocrGuid_t*>(&scope.taskwait.getEvent().handle()),
        EDT_PROP_NONE, NULL_HINT, NULL );
    ASSERT( err == 0 );

    // Close taskwait region
    scope.taskwait.closeRegion();

    nanos_wait_until_shutdown();

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

    err = ocrEdtTemplateCreate( &shutdownTemplate, edtShutdown, (mem::sizeofInItems<uint64_t,MainStorage>()), 1 );
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

    err = ocrEdtTemplateDestroy( shutdownTemplate );
    ASSERT( err == 0 );
}

} // extern "C"


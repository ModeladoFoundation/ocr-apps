
#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H

#include "vector.h"

#include "dependences_decl.h"
#include "task_decl.h"

/*! Initializes data_dependency_t data type */
static inline void initializeDataDependency( data_dependency_t* dep )
{
    dep->rd_is_done_evt = NULL_GUID;
    dep->wr_is_done_evt = NULL_GUID;
}

static inline void freeDataDependency( data_dependency_t* dep )
{
    if( !ocrGuidIsNull( dep->rd_is_done_evt ) )
        ocrEventDestroy( dep->rd_is_done_evt );

    if( !ocrGuidIsNull( dep->wr_is_done_evt ) )
        ocrEventDestroy( dep->wr_is_done_evt );
}

static inline void createReadSection( data_dependency_t* events )
{
    u8 err;
    // If read section event was not previously created
    if( ocrGuidIsNull( events->rd_is_done_evt ) ) {
        // Create latch event
        err = ocrEventCreate( &events->rd_is_done_evt,
                              OCR_EVENT_LATCH_T, EVT_PROP_NONE );
        ASSERT( err != 0 );

        // Increment latch event pre-slot counter
        // to "open" read-only section
        err = ocrEventSatisfySlot( events->rd_is_done_evt,
                                   NULL_GUID, OCR_EVENT_LATCH_INCR_SLOT );
        ASSERT( err != 0 );
    }
}

static inline void readSectionAddReader( task_definition_t* task, data_dependency_t* events )
{
    u8 err;
    err = ocrEventSatisfySlot( events->rd_is_done_evt, NULL_GUID, OCR_EVENT_LATCH_INCR_SLOT );
    ASSERT( err != 0 );

    struct _release_dep rel_action = {
        .event = events->rd_is_done_evt,
        .action = SATISFY };
    vectorPushBack( &task->release_deps, &rel_action );
}

static inline void createWriteSection( task_definition_t* task, data_dependency_t* events )
{
    // Replace previous write event regardless it existed or not
    u8 err = ocrEventCreate( &events->wr_is_done_evt, OCR_EVENT_STICKY_T, EVT_PROP_NONE );
    ASSERT( err != 0 );

    struct _release_dep rel_action = { .event = events->wr_is_done_evt,
                            .action  = SATISFY };
    vectorPushBack( &task->release_deps, &rel_action );
}

static inline void addDependencyRAW( task_definition_t* task, data_dependency_t* events )
{
    if( !ocrGuidIsNull( events->wr_is_done_evt ) ) {
        struct _acquire_dep acq_action = { .event = events->wr_is_done_evt,
                                .action  = DEPEND_ONLY };
        vectorPushBack( &task->acquire_deps, &acq_action );
    }
}

static inline void addDependencyWAW( task_definition_t* task, data_dependency_t* events )
{
    if( !ocrGuidIsNull( events->wr_is_done_evt ) ) {
        // Before EDT is executed: add dependency on event
        struct _acquire_dep acq_action = { .event = events->wr_is_done_evt,
                                .action  = DEPEND_ONLY };
        vectorPushBack( &task->acquire_deps, &acq_action );
        // After EDT is executed: destroy old write event
        struct _release_dep rel_action = { .event = events->wr_is_done_evt,
                                .action  = DESTROY };
        vectorPushBack( &task->release_deps, &rel_action );
    }
}

static inline void addDependencyWAR( task_definition_t* task, data_dependency_t* events )
{
    if( !ocrGuidIsNull( events->rd_is_done_evt ) ) {
        // Before EDT is executed: add dependency on event and close read section
        struct _acquire_dep acq_action = { .event = events->rd_is_done_evt,
                                .action  = DEPEND_AND_SATISFY };
        vectorPushBack( &task->acquire_deps, &acq_action );

        // Delete read section from dependency map
        events->rd_is_done_evt = NULL_GUID;
    }
}

static inline void acquireDependences( ocrGuid_t edt, task_definition_t* task )
{
    u8 err;
    u32 size = task->acquire_deps.size;
    struct _acquire_dep* actions = (struct _acquire_dep*)task->acquire_deps.data;
    for( u32 i = 0; i < size; ++i ) {
        u32 slot = i+1; // here slot refers to the position of the depv array
        err = ocrAddDependence( actions[i].event, edt, slot, DB_MODE_RW/*default mode*/ );
        ASSERT( err == 0 );

        if( actions[i].action == DEPEND_AND_SATISFY ) {
            err = ocrEventSatisfy( actions[i].event, NULL_GUID );
            ASSERT( err == 0 );
        }
    }
}

static inline void releaseDependences( task_definition_t* task )
{
    u8 err;
    u32 size = task->release_deps.size;
    struct _release_dep* actions = (struct _release_dep*)task->release_deps.data;
    for( u32 i = 0; i < size; ++i ) {
        switch( actions[i].action ) {
            case SATISFY:
                err = ocrEventSatisfy( actions[i].event, NULL_GUID );
                ASSERT( err == 0 );
                break;
            case DESTROY:
                err = ocrEventDestroy( actions[i].event );
                ASSERT( err == 0 );
                break;
            default:
                ASSERT( FALSE );
        }
    }
}

static inline u32 getNumDependences( task_definition_t* task )
{
    return task->acquire_deps.size;
}

#endif // DEPENDENCIES_H


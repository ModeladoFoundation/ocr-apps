
#ifndef TASK_H
#define TASK_H

#include "task_decl.h"

#include "dependences_decl.h"

#include "common.h"
#include "hashtable.h"
#include "task-local.h"
#include "vector.h"

#include <nanos6_rt_interface.h>

static inline void newLocalScope( task_scope_info_t* scope )
{
    // Create taskwait event and open taskwait region
    u8 err;
    err = ocrEventCreate( &scope->taskwait_evt,
                    OCR_EVENT_LATCH_T, EVT_PROP_NONE );
    ASSERT( err == 0 );
    err = ocrEventSatisfySlot( scope->taskwait_evt,
                    NULL_GUID, OCR_EVENT_LATCH_INCR_SLOT );
    ASSERT( err == 0 );

    // Initialize flags
    scope->flags.postpone_cleanup = 0;

    // Access map for dependence tracking
    newHashTable( &scope->accesses );
}

static inline void destructLocalScope( task_scope_info_t* self )
{
    // Close taskwait region
    u8 err;
    err = ocrEventSatisfySlot( self->taskwait_evt,
                               NULL_GUID, OCR_EVENT_LATCH_DECR_SLOT );
    ASSERT( err == 0 );

    // Free access map
    destructHashTable( &self->accesses );
}

static inline task_t* newTask( nanos_task_info* info, u64 args_size )
{
    task_t* task = (task_t*)ompss_malloc( sizeof(task_t) + args_size );

    task->definition.arguments = ((char*)task) + sizeof(task_t); // take care with alignment constraints
    task->definition.run       = info->run;
    task->definition.register_dependences = info->register_depinfo;

    // Dependence lists
    newVector( &task->dependences.acquire, sizeof(struct _acquire_dep) );
    newVector( &task->dependences.release, sizeof(struct _release_dep) );

    // Create necessary edt-local data-structures
    newLocalScope( &task->local_scope );

    return task;
}

static inline void destructTask( task_t* self )
{
    destructLocalScope( &self->local_scope );
    destructVector( &self->dependences.acquire );
    destructVector( &self->dependences.release );
    ompss_free(self);
}

#endif // TASK_H


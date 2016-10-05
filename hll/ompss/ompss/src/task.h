
#ifndef TASK_H
#define TASK_H

#include "task_decl.h"

#include "dependences_decl.h"

#include "common.h"
#include "hashtable.h"
#include "vector.h"

#include <nanos6_rt_interface.h>

static inline task_t* newTask( nanos_task_info* info, u64 args_size )
{
    task_t* task = (task_t*)malloc( sizeof(task_t) + args_size );

    task->definition.arguments = ((char*)task) + sizeof(task_t); // take care with alignment constraints
    task->definition.run       = info->run;
    task->definition.register_dependences = info->register_depinfo;

    // Dependence lists
    newVector( &task->dependences.acquire, sizeof(struct _acquire_dep) );
    newVector( &task->dependences.release, sizeof(struct _release_dep) );

    // Create necessary edt-local data-structures
    // Access map for dependence tracking
    newHashTable( &task->local_scope.accesses );
    // Create taskwait event and open taskwait region
    u8 err;
    err = ocrEventCreate( &task->local_scope.taskwait_evt,
                          OCR_EVENT_LATCH_T, EVT_PROP_NONE );
    ASSERT( err == 0 );
    err = ocrEventSatisfySlot( task->local_scope.taskwait_evt,
                               NULL_GUID, OCR_EVENT_LATCH_INCR_SLOT );
    ASSERT( err == 0 );

    return task;
}

static inline void destructTask( task_t* task )
{
    // Close taskwait region
    u8 err;
    err = ocrEventSatisfySlot( task->local_scope.taskwait_evt,
                               NULL_GUID, OCR_EVENT_LATCH_DECR_SLOT );
    ASSERT( err == 0 );

    destructVector( &task->dependences.acquire );
    destructVector( &task->dependences.release );
    destructHashTable( &task->local_scope.accesses );
    free(task);
}

#endif // TASK_H


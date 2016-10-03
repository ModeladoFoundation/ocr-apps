
#ifndef TASK_H
#define TASK_H

#include "task_decl.h"

#include "dependences_decl.h"

#include "common.h"
#include "hashtable.h"
#include "vector.h"

#include <nanos6_rt_interface.h>

static inline task_definition_t* newTaskDefinition( nanos_task_info* info, u64 args_size ) {
    task_definition_t* td = (task_definition_t*)malloc( sizeof(task_definition_t) + args_size );

    td->args_block = &td[1];
    td->run_funct = info->run;
    td->dependences_funct = info->register_depinfo;

    newVector( &td->acquire_deps, sizeof(struct _acquire_dep) );
    newVector( &td->release_deps, sizeof(struct _release_dep) );
    return td;
}

static inline void destructTaskDefinition( task_definition_t* td ) {
    destructVector( &td->acquire_deps );
    destructVector( &td->release_deps );
    free( td );
}

#endif // TASK_H


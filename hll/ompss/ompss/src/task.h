
#ifndef TASK_H
#define TASK_H

#include <nanos6_rt_interface.h>

#include "dependences_decl.h"
#include "common.h"
#include "vector.h"

typedef struct {
    void*                 args_block;
    run_funct_t           run_funct;
    register_dep_funct_t  dependences_funct;
    vector_t              acquire_deps;
    vector_t              release_deps;
} task_definition_t;

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


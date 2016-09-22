
#ifndef TASK_H
#define TASK_H

#include <nanos6_rt_interface.h>

#include "common.h"
#include "vector.h"

typedef struct {
    nanos_task_info*      info;
    void*                 args_block;
    run_funct_t           run_funct;
    register_dep_funct_t  dependencies_funct;
    vector_t dependencies;
} task_definition_t;

static inline task_definition_t* newTaskDefinition( nanos_task_info* info, u64 args_size ) {
    task_definition_t* td = (task_definition_t*)malloc( sizeof(task_definition_t) + args_size );

    td->info = info; // TODO: probably this is not needed any more
    td->args_block = td+1;
    td->run_funct = info->run;
    td->dependencies_funct = info->register_depinfo;

    newVector( &td->dependencies, sizeof(ocrGuid_t) );
    return td;
}

static inline void destructTaskDefinition( task_definition_t* td ) {
    destructVector( &td->dependencies );
    free( td );
}

#endif // TASK_H


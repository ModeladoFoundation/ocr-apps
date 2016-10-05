
#ifndef TASK_DECL_H
#define TASK_DECL_H

#include "common.h"
#include "hashtable_decl.h"
#include "vector_decl.h"

struct _task_definition {
    void*                     arguments;
    run_funct_t               run;
    register_dep_funct_t      register_dependences;
};

struct _task_dependences {
    vector_t                  acquire;
    vector_t                  release;
};

typedef struct {
    ocrGuid_t    taskwait_evt;
    hash_table_t accesses;
} task_scope_info_t;

typedef struct {
    struct _task_definition   definition;
    struct _task_dependences  dependences;
    task_scope_info_t         local_scope;
} task_t;

#endif // TASK_DECL_H


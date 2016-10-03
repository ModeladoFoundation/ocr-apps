
#ifndef TASK_DECL_H
#define TASK_DECL_H

#include "common.h"
#include "hashtable_decl.h"
#include "vector_decl.h"

typedef struct {
    void*                     args_block;
    run_funct_t               run_funct;
    register_dep_funct_t      dependences_funct;
    vector_t                  acquire_deps;
    vector_t                  release_deps;
    hash_table_t              local_dependences;
} task_definition_t;

#endif


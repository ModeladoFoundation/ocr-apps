
#ifndef HASHTABLE_DECL_H
#define HASHTABLE_DECL_H

#include "dependences_decl.h"

#include <ocr.h>

typedef void*             key_t;
typedef data_dependency_t value_t;

struct _ht_bucket;

typedef struct _ht_table {
    struct _ht_bucket** data;
    u32                size; // number of bins
} hash_table_t;

struct _ht_bucket {
    struct _ht_bucket* next;
    key_t              key;
    value_t            value;
};

#endif // HASHTABLE_DECL_H

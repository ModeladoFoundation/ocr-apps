
#ifndef VECTOR_DECL_H
#define VECTOR_DECL_H

#include <ocr.h>

typedef struct {
    u32 size;
    u32 capacity;
    u64 elem_size;
    void*  data;
} vector_t;

#endif // VECTOR_DECL_H


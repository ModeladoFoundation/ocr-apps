
#ifndef VECTOR_H
#define VECTOR_H

#include "common.h"

#include <string.h>

// FIXME look for an optimum value
#define ARRAY_RESIZE_ELEMENTS 16

typedef struct {
    u64 size;
    u64 elem_size;
    u64 capacity;
    void*  data;
} vector_t;

static inline void newVector( vector_t* vector, u64 elem_size )
{
    vector->capacity = 0;
    vector->size = 0;
    vector->elem_size = elem_size;
    vector->data = NULL;
}

static inline void destructVector( vector_t* vector )
{
    if( vector->capacity != 0 ) {
        free( vector->data );
    }
    vector->capacity = 0;
    vector->size = 0;
}

static inline void vectorResize( vector_t* vector,
                 u64 new_capacity )
{
    if( new_capacity != vector->capacity ) {
        vector->capacity = new_capacity;
        void* newData = malloc( new_capacity * vector->elem_size );

        memcpy( newData, vector->data,
                new_capacity * vector->elem_size );

        free( vector->data );
        vector->data = newData;
    }
}

static inline u8 vectorIsFull( vector_t* vector )
{
    return vector->size == vector->capacity? TRUE : FALSE;
}

static inline u8 vectorIsEmpty( vector_t* vector ) {
    return vector->size == 0? TRUE : FALSE;
}

static inline void vectorPushBack( vector_t* vector,
                   void* element )
{
    if( vectorIsFull(vector) ) {
        vectorResize( vector, vector->capacity + ARRAY_RESIZE_ELEMENTS );
    }

    void* pos = ((char*)vector->data) + vector->size*vector->elem_size;
    memcpy( pos, element, vector->elem_size );

    vector->size++;
}

#endif // VECTOR_H


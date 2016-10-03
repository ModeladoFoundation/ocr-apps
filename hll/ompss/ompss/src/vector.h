
#ifndef VECTOR_H
#define VECTOR_H

#include "vector_decl.h"

#include "common.h"

#include <string.h>

// FIXME look for an optimum value
#define ARRAY_RESIZE_ELEMENTS 16

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
                 u32 new_capacity )
{
    if( new_capacity != vector->capacity ) {
        void* new_data = malloc( new_capacity * vector->elem_size );

        memcpy( new_data, vector->data,
                vector->capacity * vector->elem_size );
        vector->capacity = new_capacity;

        free( vector->data );
        vector->data = new_data;
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


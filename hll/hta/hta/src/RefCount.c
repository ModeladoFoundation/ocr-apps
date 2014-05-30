#include "RefCount.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <omp.h>

static unsigned int _num_objects_alive = 0; // FIXME: could cause problem thread parallelism

void* RefCount_alloc(size_t size) {
    void* ptr;
    RefCount* obj = (RefCount*) malloc(sizeof(RefCount) + size);
    assert(obj);

    obj->refcount = 1;
    ptr = (void*) obj;
    ptr += sizeof(RefCount);

#pragma omp atomic
    _num_objects_alive++;

    return ptr;
}

int RefCount_retain(void *ptr) {
    RefCount* obj = (RefCount*) (ptr - sizeof(RefCount));
    obj->refcount++;
    return obj->refcount;
}

int RefCount_release(void *ptr) {
    RefCount* obj = (RefCount*) (ptr - sizeof(RefCount));
    obj->refcount--;
    if(obj->refcount == 0) {
        free(obj);
#pragma omp atomic
        _num_objects_alive--;
        return 1;
    }
    return 0;
}

int RefCount_get_counter(void *ptr) {
    RefCount* obj = (RefCount*) (ptr - sizeof(RefCount));
    return obj->refcount;
}

unsigned int RefCount_num_alive() {
    return _num_objects_alive;
}


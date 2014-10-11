#include <stdio.h>
#include <stdlib.h>
#include "common.h"

void *malloc_checked(size_t size, const char *file, int line)
{
    void *ptr = malloc(size);
    if (ptr == NULL)
    {
        fprintf(stderr, "Out of memory in call to malloc at line %d of file %s.\n",
            line, file);
        exit(1);
    }
    return ptr;
}

void *calloc_checked(size_t nelem, size_t elemsize, const char *file, int line)
{
    void *ptr = calloc(nelem, elemsize);
    if (ptr == NULL)
    {
        fprintf(stderr, "Out of memory in call to calloc at line %d of file %s.\n",
            line, file);
        exit(1);
    }
    return ptr;
}

void *fftwf_malloc_checked(size_t size, const char *file, int line)
{
    void *ptr = fftwf_malloc(size);
    if (ptr == NULL)
    {
        fprintf(stderr, "Out of memory in call to fftwf_malloc at line %d of file %s.\n",
            line, file);
        exit(1);
    }
    return ptr;
}

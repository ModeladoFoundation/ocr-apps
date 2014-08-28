#include <stdint.h>
#ifndef __INTVEC_H
#define __INTVEC_H

typedef struct IntVecTag
{

  uint32_t* arr; /* the dynamic array; */
  uint32_t num_elements;
  uint32_t num_allocated;
}IntVec;

/* vector of intvec */

typedef struct Vec_IntVecTag
{
  IntVec** arr;
  uint32_t num_elements;
  uint32_t num_allocated;

}Vec_IntVec;

IntVec* construct_IntVec();
IntVec* construct_IntVec_reserve(uint32_t isz);

void add_int(IntVec* avec, uint32_t elem);

void add_intvec(Vec_IntVec* a, IntVec* b);

void free_Vec_IntVec(Vec_IntVec* a);
void add_int_uniq(IntVec* avec, uint32_t elem);

void reserve_VecOfIntVec(Vec_IntVec* store, uint32_t alloc);

#endif

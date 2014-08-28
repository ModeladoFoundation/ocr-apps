

#include "int_vec.h"
#include <stdio.h>
#include <stdlib.h>

IntVec* construct_IntVec()
{
  IntVec* p = (IntVec*)malloc(sizeof(IntVec));
  p->arr = NULL;
  p->num_elements = 0;
  p->num_allocated = 0;
  return p;
}


IntVec* construct_IntVec_reserve(uint32_t isz)
{
  IntVec* p = (IntVec*)malloc(sizeof(IntVec));
  p->arr = (uint32_t*) malloc(isz * sizeof(uint32_t));
  p->num_elements = 0;

  p->num_allocated = isz;
  return p;
}



void add_int(IntVec* avec, uint32_t elem)
{

  if(avec->num_elements == avec->num_allocated)
    {


    if (avec->num_allocated == 0)
      avec->num_allocated = 3;
    else
      avec->num_allocated *= 2;
    void *_tmp = realloc(avec->arr, (avec->num_allocated * sizeof(uint32_t)));

    if (!_tmp)
      {
	fprintf(stderr, "ERROR: Couldn't realloc memory!\n");
	return;
      }

    if(avec->arr != _tmp &&  _tmp)
      {
	avec->arr = (uint32_t*) _tmp;

      }

  }
  avec->arr[avec->num_elements] = elem;
  ++avec->num_elements;
  return;
}

void add_int_uniq(IntVec* avec, uint32_t elem)
{
  uint32_t i;
  for(i = 0; i < avec->num_elements; i++)
    {
      if(avec->arr[i] == elem) return;
    }
  add_int(avec, elem);
}


void reserve_VecOfIntVec(Vec_IntVec* store, uint32_t alloc)
{
  store->arr = (IntVec**) malloc( alloc * sizeof(IntVec*));
  store->num_allocated = alloc;
  uint32_t k =0;
  for(; k < store->num_allocated; ++k)
    {
      store->arr[k] = NULL;
    }

}


void add_intvec(Vec_IntVec* avec, IntVec* elem)
{

  if(avec->num_elements == avec->num_allocated)
    {
      uint32_t k = avec->num_allocated;

      if (avec->num_allocated == 0)
	avec->num_allocated = 3;
      else
	avec->num_allocated *= 2;
      void *_tmp = realloc(avec->arr, (avec->num_allocated * sizeof(IntVec*)));

      if (!_tmp)
	{
	  fprintf(stderr, "ERROR: Couldn't realloc memory!\n");
	  return;
	}
      avec->arr = (IntVec**)_tmp;

      for(; k < avec->num_allocated; ++k)
	{
	  avec->arr[k] = NULL;
	}
    }
	avec->arr[avec->num_elements] = elem;

	++avec->num_elements;
	return;
}


void free_Vec_IntVec(Vec_IntVec* vvec)
{
  uint32_t i =0 ;
  for(; i < vvec->num_allocated; ++i)
    {
      if(vvec->arr[i])
	{
	if(vvec->arr[i]->arr)
	  free(vvec->arr[i]->arr);
	free(vvec->arr[i]);
	}
    }
  free(vvec->arr);

}

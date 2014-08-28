

#include "int_vec.h"
#include <stdio.h>
#include <stdlib.h>

struct IntVec* construct_IntVec()
{
  struct IntVec* p = (struct IntVec*)malloc(sizeof(struct IntVec));
  p->arr = NULL;
  p->num_elements = 0;
  p->num_allocated = 0;
  return p;
}

void add_int(struct IntVec* avec, int elem)
{

  if(avec->num_elements == avec->num_allocated)
    {


    if (avec->num_allocated == 0)
      avec->num_allocated = 3;
    else
      avec->num_allocated *= 2;
    void *_tmp = realloc(avec->arr, (avec->num_allocated * sizeof(int)));

    if (!_tmp)
      {
	fprintf(stderr, "ERROR: Couldn't realloc memory!\n");
	return;
      }

    if(avec->arr != _tmp &&  _tmp)
      {
	avec->arr = (int*) _tmp;

      }

  }
  avec->arr[avec->num_elements] = elem;
  ++avec->num_elements;
  return;
}


void add_intvec(struct Vec_IntVec* avec, struct IntVec* elem)
{

  if(avec->num_elements == avec->num_allocated) {


    if (avec->num_allocated == 0)
      avec->num_allocated = 3;
    else
      avec->num_allocated *= 2;
    void *_tmp = realloc(avec->arr, (avec->num_allocated * sizeof(struct IntVec*)));

		if (!_tmp)
		{
			fprintf(stderr, "ERROR: Couldn't realloc memory!\n");
			return;
		}
		avec->arr = (struct IntVec**)_tmp;
	}
	avec->arr[avec->num_elements] = elem;
	int k = avec->num_elements + 1;
	for(; k < avec->num_allocated; ++k)
	  {
	    avec->arr[k] = NULL;
	  }
	++avec->num_elements;
	return;
}


void free_Vec_IntVec(struct Vec_IntVec* vvec)
{
  int i =0 ;
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

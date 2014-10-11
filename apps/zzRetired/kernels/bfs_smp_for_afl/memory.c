#include <stdio.h>
#include "memory.h"
#include "AO_reroute.h"
#include <stdio.h>

struct node_t*  nstore;
/* struct node_t nstore[ssz]; */

uint32_t genRand(uint32_t M)
{
  double r = drand48();
  double x = M * r;
  uint32_t y = (uint32_t)x;
  return y;
}



uint32_t alloc_node()
{
  uint32_t retry = 0;
  /* generate a random number */
  while(retry < 200)
    {
      uint32_t idx =  genRand(ssz);
      uint32_t* ptr = &nstore[idx].tag;

      if(AO_compare_and_swap(ptr, available, inuse) !=0)
	{
	  return idx;
	}
      ++retry;
    }
  return (uint32_t) -1;
}


uint32_t alloc_node_ptr(struct node_t**  ptr)
{
  uint32_t retry = 0;
  /* generate a random number */
  while(retry < 1000)
    {
      uint32_t idx =  genRand(ssz);
      /* campare and swap with 1 */
      uint32_t* tptr = &nstore[idx].tag;
      if(AO_compare_and_swap(tptr, available, inuse) != 0)
	{

	  *ptr = &nstore[idx];
	  /* printf("alloc: %d\n",idx); */
	  return idx;
	}
      ++retry;
    }
  printf("Alloc failed : Probably out of memory \n");
  *ptr  = 0;
  return (uint32_t)-1;
}

uint32_t alloc_node_ptr_stat(struct node_t**  ptr, uint32_t* num_a)
{
  uint32_t retry = 0;
  /* generate a random number */
  while(retry < 1000)
    {
      uint32_t idx =  genRand(ssz);
      /* campare and swap with 1 */
      uint32_t* tptr = &nstore[idx].tag;
      *num_a = *num_a + 1;
      if(AO_compare_and_swap(tptr, available, inuse) != 0)
	{

	  *ptr = &nstore[idx];
	  nstore[idx].data = (uint32_t)-1;
	  nstore[idx].next = (uint32_t)-1;
	  /* printf("alloc: %d\n",idx); */
	  return idx;
	}
      ++retry;
    }
  printf("Alloc failed : Probably out of memory \n");
  *ptr  = 0;
  return (uint32_t)-1;
}

void init_store()
{
  nstore = (struct node_t*) malloc(ssz * sizeof(struct node_t));
  assert(nstore != 0);
  uint32_t i;
  for(i = 0; i < ssz; ++i)
    {
      nstore[i].tag = available;
      nstore[i].data = i;
      nstore[i].next = (uint32_t)-1;
    }
}

void reinit_store()
{

  uint32_t i;
  for(i = 0; i < ssz; ++i)
    {
      nstore[i].tag = available;
      nstore[i].data = i;

    }
}



uint32_t get_reference(uint32_t idx)
{
  uint32_t curr_ref = nstore[idx].tag;
  if(curr_ref == 0){
    return (uint32_t)-1;
  }
  uint32_t new_ref = curr_ref + 1;
  /* what if tag gets recycled i.e.
     the other thread releases
     and yet another thread picks it up.
     more abstractly the operation is get_reference to head.
     even if it is recycled we don't care as long   as head is head.
     the problem happens if exsisting refence is recycled which won't happen
   */

  if(AO_compare_and_swap(&nstore[idx].tag, curr_ref,new_ref) != 0)
    {
      return new_ref;
    }

  return (uint32_t)-1;
}


uint32_t get_reference_stat(uint32_t idx, uint32_t* num_a)
{
  uint32_t curr_ref = nstore[idx].tag;
  if(curr_ref == 0){
    return (uint32_t)-1;
  }
  uint32_t new_ref = curr_ref + 1;
  /* what if tag gets recycled i.e.
     the other thread releases
     and yet another thread picks it up.
     more abstractly the operation is get_reference to head.
     even if it is recycled we don't care as long   as head is head.
     the problem happens if exsisting refence is recycled which won't happen
   */
  *num_a = *num_a + 1;
  if(AO_compare_and_swap(&nstore[idx].tag, curr_ref,new_ref) != 0)
    {
      return new_ref;
    }

  return (uint32_t)-1;
}

uint32_t dec_reference(uint32_t idx)
{
  while(1)
    {
      uint32_t curr_ref = nstore[idx].tag;
      if(curr_ref == 0)
	{
	  return (uint32_t) -1;
	}
      uint32_t new_ref = curr_ref - 1;

      if(AO_compare_and_swap(&nstore[idx].tag, curr_ref, new_ref) != 0)
	{
	  return curr_ref;
	}
/*       printf("Cooool atomic failed: %ld %ld %ld\n", idx,  new_ref, curr_ref); */
    }
  return (uint32_t) -1;
}

uint32_t dec_reference_stat(uint32_t idx, uint32_t* num_a)
{
  while(1)
    {
      uint32_t curr_ref = nstore[idx].tag;
      if(curr_ref == 0)
	{
	  return (uint32_t) -1;
	}
      uint32_t new_ref = curr_ref - 1;
      *num_a = *num_a + 1;
      if(AO_compare_and_swap(&nstore[idx].tag, curr_ref, new_ref) != 0)
	{
	  return curr_ref;
	}
/*       printf("Cooool atomic failed: %ld %ld %ld\n", idx,  new_ref, curr_ref); */
    }
  return (uint32_t) -1;
}


uint32_t get_data(uint32_t idx)
{
  return nstore[idx].data;

}


uint32_t get_next(uint32_t idx)
{
  return nstore[idx].next;
}

void set_next(uint32_t idx,uint32_t loc)
{
  nstore[idx].next = loc;
  return;
}

void print_stack(uint32_t head)
{
  uint32_t cursor = head;
  char fn[80] = "stack_final_state.txt";
  FILE* fp = fopen(fn, "w");
  /*  fprintf(fp, "[");*/
  while(cursor != (uint32_t) -1)
    {
      fprintf(fp, "%d %d \n", cursor, nstore[cursor].data);
      cursor = nstore[cursor].next;
    }
  fprintf(fp, "-1 -1\n");
  fclose(fp);
}

void test_memory(void* arg)
{
  long id = (long) arg;
  /* printf("ID:%ld\n", id); */
  char fn[100];
  sprintf(fn, "pid_%ld.log", id);
  FILE* fp;
  fp = fopen(fn, "w");
  fprintf(fp, "ID=%ld\n", id);
  uint32_t loop_idx=0;

  for(loop_idx = 0; loop_idx < 3; ++loop_idx)
    {
      struct node_t* addr;
      uint32_t new_head = alloc_node_ptr(&addr);
      if(new_head != (uint32_t) -1)
	{
	  addr->data = id;
	  fprintf(fp, "alloc: addr=%p, pos=%d\n", addr, new_head);
	}
    }
  fclose(fp);
}



void dump_memory()
{
  uint32_t i = 0;
  printf("Dump\n");
  for(i = 0; i < ssz; ++i)
    {
      if(nstore[i].tag != 0)
	{
	  printf("idx=%d, tag=%d, next=%d, val=%d\n", i, nstore[i].tag, nstore[i].next, nstore[i].data);
	}
    }
}

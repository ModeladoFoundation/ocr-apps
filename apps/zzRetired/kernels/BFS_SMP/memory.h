#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>


struct node_t
{
  uint32_t tag;
  uint32_t next; /* pointer to next in the queue */
  payload_t data;
};

#define inuse 1
#define available 0
struct node_t*  nstore;

typedef struct node_t* nodeptr_t;

/* number of nodes to be managed */
//#define ssz  256
uint32_t ssz;


/* int genRand(int M); */
/* uint32_t genRand(uint32_t M); */
/* uint32_t alloc_node(); */
/* uint32_t alloc_node_ptr(struct node_t**  ptr); */
/* uint32_t alloc_node_ptr_stat(struct node_t**  ptr,uint32_t*); */
/* void dump_memory(); */
/* void test_memory(void* arg); */
void init_store(uint32_t mm_sz)
{
  ssz = mm_sz;
  nstore = (struct node_t*) malloc(ssz * sizeof(struct node_t));
  assert(nstore != 0);
  uint32_t i;
  for(i = 0; i < ssz; ++i)
    {
      nstore[i].tag = available;
      nstore[i].data = (uint32_t*)0;
      nstore[i].next = (uint32_t)-1;
    }

}
uint32_t genRand(uint32_t M)
{
  double r = drand48();
  double x = M * r;
  uint32_t y = (uint32_t)x;
  return y;
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
  exit(0);
  return (uint32_t)-1;
}

payload_t get_data(uint32_t idx)
{
  return nstore[idx].data;

}

uint32_t get_next(uint32_t idx)
{
  return nstore[idx].next;
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


/* uint32_t get_reference(uint32_t idx); */
/* uint32_t dec_reference(uint32_t idx); */
/* uint32_t get_reference_stat(uint32_t idx,uint32_t*); */
/* uint32_t dec_reference_stat(uint32_t idx,uint32_t*); */
/* uint32_t get_data(uint32_t idx); */
/* uint32_t get_next(uint32_t idx); */
/* void set_next(uint32_t idx,uint32_t loc); */
/* void print_stack(uint32_t head); */
/* void reinit_store(); */

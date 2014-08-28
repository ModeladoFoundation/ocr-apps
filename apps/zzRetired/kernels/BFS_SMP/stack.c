#include <stdio.h>
#include "memory.h"
#include "AO_reroute.h"
#include "myassert.h"

uint32_t push(uint32_t* head_ptr, payload_t dat)
{
  /* alloc a node */
  struct node_t* addr;

  uint32_t new_head = alloc_node_ptr(&addr);

  if(new_head != (uint32_t)-1)
    {
      uint32_t curr_head = *head_ptr;
      addr->data = dat;

      do
      	{
      	  curr_head = *head_ptr; /* what if aba happens: np as long as head is head*/
      	  addr->next = curr_head;
      	} while(AO_compare_and_swap(head_ptr, curr_head, new_head) == 0);
      /* fprintf(fp, "push:1 new_head=%lu, old_head=%lu, datum=%lu\n", new_head, curr_head, dat);  */
      return 1;
    }
   return (uint32_t)-1;
}




payload_t pop(uint32_t* head_ptr)
{
  while(1)
    {
      uint32_t curr_head  = *head_ptr;
      if(curr_head == (uint32_t)-1) return (uint32_t)-1;
      /* could head be recycled here? */
      /* A: yes, another thread frees it
	 and yet another thread recycles some new data */
      /* and therefore we incr tag of  curr_head using get_reference, so that no one free it but us*/

      if(get_reference(curr_head) != (uint32_t) -1)
	{
	  /* now we can safely read the contents of curr_head */
	  payload_t val = get_data(curr_head);
	  uint32_t next = get_next(curr_head);
	  if(AO_compare_and_swap(head_ptr, curr_head, next) != 0)
	    {
	      myassert(dec_reference(curr_head) != (uint32_t)-1);/* on behalf of what used to be head */
	      myassert(dec_reference(curr_head) != (uint32_t)-1); /* on behalf of curr_head */
	      return val;
	    }
	  else
	    {
	      /* curr_head is no longer the head, retry */

	    }
	}
      else
	{
	 /* curr_head is no longer head and is poped out, memory is freed */
	}
    }
}


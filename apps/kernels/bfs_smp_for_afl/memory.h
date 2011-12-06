#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>


struct node_t
{
  uint32_t tag;
  uint32_t next; /* pointer to next in the queue */
  uint32_t data;
};

#define inuse 1
#define available 0


typedef struct node_t* nodeptr_t;

/* number of nodes to be managed */
#define ssz 1000000000


/* int genRand(int M); */
uint32_t genRand(uint32_t M);
uint32_t alloc_node();
uint32_t alloc_node_ptr(struct node_t**  ptr);
uint32_t alloc_node_ptr_stat(struct node_t**  ptr,uint32_t*);
void dump_memory();
void test_memory(void* arg);
void init_store();
uint32_t get_reference(uint32_t idx);
uint32_t dec_reference(uint32_t idx);
uint32_t get_reference_stat(uint32_t idx,uint32_t*);
uint32_t dec_reference_stat(uint32_t idx,uint32_t*);
uint32_t get_data(uint32_t idx);
uint32_t get_next(uint32_t idx);
void set_next(uint32_t idx,uint32_t loc);
void print_stack(uint32_t head);
void reinit_store();

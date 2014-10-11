//#include "EvolvingDAG.h"

#include "dynamic_graph.h"
#define nref int32_t




uint32_t genRand(uint32_t M);
void grow_dag_init(int d);
int addNode_c(struct graphtree_idx* gptr, int, int);
nref findMatch_c();
void addNewNode(struct graphtree_idx* gptr);
void free_dag_generator();
void boot_strap_new_node(struct graphtree_idx* gptr);


/*
   gptr should not be modified expect by grow_dag routines.
   internal book-keeping of grow_dag is tightly coupled with graphtree_idx
*/

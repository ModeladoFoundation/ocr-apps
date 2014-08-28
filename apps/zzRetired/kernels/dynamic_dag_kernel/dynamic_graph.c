#include "dynamic_graph.h"
#include <assert.h>

/* void print_node(uint32_t adjid, struct graphtree_leaf* location) */
/* { */
/*   printf("%d ", adjid); */

/* } */

//adjs <-- adjacency of nodeid



/* void print_adjs(uint32_t nodeid, struct adjtree_idx* adjs) */
/* { */
/*   printf("N= %d : ", nodeid); */
/*   if(adjs) */
/*     adjtree_inorder_traversal(adjs, print_node); */
/*   printf("\n"); */

/* } */

uint8_t add_node(struct graphtree_idx* tree, uint32_t srcid)
{
  struct graphtree_leaf* srcPtr = find_graphtree_leaf(tree, srcid, 0);
  if(!srcPtr)
    {
      struct adjtree_idx* out_adj = construct_adjtree_idx();
      graphtree_insert_value(tree, srcid, out_adj, 0);
      return 0;
    }
  assert(0);
}




uint8_t add_edge(struct graphtree_idx* tree, uint32_t srcid, uint32_t desid)
{
  struct graphtree_leaf* desPtr = find_graphtree_leaf(tree, desid, 0);

  if(!desPtr){
    struct adjtree_idx* des_out_adj = 0;
    graphtree_insert_value(tree, desid, des_out_adj, 0);
    desPtr = find_graphtree_leaf(tree, desid, 0);
  }

  struct graphtree_leaf* srcPtr = find_graphtree_leaf(tree, srcid, 0);
  if(!srcPtr)
    {
      assert(0);
      struct adjtree_idx* out_adj = construct_adjtree_idx();
      adjtree_insert_value(out_adj, desid, desPtr,0);
      graphtree_insert_value(tree, srcid, out_adj, 0);
      return 1;
      //src is at depth 0;
    }
  else
    {
      if(!srcPtr->adjs) srcPtr->adjs = construct_adjtree_idx();
      /*printf("dynamic_graph:add_edge: srcid = %d, src_adjPtr=%p, desid =%d desPtr=%p %d\n", srcid, srcPtr->adjs, desid, desPtr, desPtr->nodeid);*/
      adjtree_insert_value(srcPtr->adjs, desid, desPtr, 0);
      if(desPtr->depth == 0) desPtr->depth = srcPtr->depth + 1;
      if(desPtr->depth > srcPtr->depth + 1)
	desPtr->depth = srcPtr->depth + 1;
      return desPtr->depth;
    }
}


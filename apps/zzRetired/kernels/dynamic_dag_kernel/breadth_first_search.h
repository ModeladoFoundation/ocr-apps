
#include "dynamic_graph.h"
#ifndef __BREADTH_FIRST_SEARCH_H
#define  __BREADTH_FIRST_SEARCH_H


struct bfs_search_env
{
  struct inode* gray_nodes;
  struct inode* discovered_nodes;
  struct inode* black_nodes;
  uint32_t  desID;
  uint8_t done;
  uint8_t res;
};

int reachabilitySearch(struct bfs_search_env* env);
void destruct_bfs_env(struct bfs_search_env* env);
struct bfs_search_env* construct_env(struct graphtree_leaf* src, uint32_t desid);



#endif

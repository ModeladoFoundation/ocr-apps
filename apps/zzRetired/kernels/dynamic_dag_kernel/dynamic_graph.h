#include <stdio.h>
#include <stdint.h>

#include "dynamic_graph_impl.h"

/*struct graphtree_idx; */

#ifndef __DYNAMIC_GRAPH_H
#define __DYNAMIC_GRAPH_H


uint8_t add_node(struct graphtree_idx* tree, uint32_t srcid);
uint8_t add_edge(struct graphtree_idx* tree, uint32_t srcid, uint32_t desid);

#endif

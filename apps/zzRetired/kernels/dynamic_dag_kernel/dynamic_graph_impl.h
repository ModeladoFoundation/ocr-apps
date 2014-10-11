#include <stdint.h>

#ifndef __DYNAMIC_GRAPH_IMPL_H
#define __DYNAMIC_GRAPH_IMPL_H

struct graphtree_idx;
struct graphtree_leaf;
struct adjtree_idx;
struct adjtree_leaf;

union graphtree_idx_child
{
  struct graphtree_idx* ichild;
  struct graphtree_leaf* lchild;
};

struct graphtree_idx
{
  struct graphtree_idx* parent;
  union graphtree_idx_child leftchild;
  union graphtree_idx_child rightchild;
  int left_type;
  int right_type; // to set the type of left and right 0 ==> inode 1==> vnode
};

union adjtree_idx_child
{
  struct adjtree_idx* ichild;
  struct adjtree_leaf* lchild;
};

struct adjtree_idx
{
  struct adjtree_idx* parent;
  union adjtree_idx_child leftchild;
  union adjtree_idx_child rightchild;
  int left_type;
  int right_type; // to set the type of left and right 0 ==> inode 1==> vnode
};
//stores adjacency for each node
struct graphtree_leaf
{
  uint32_t nodeid; //the actual id
  uint8_t depth;
  struct adjtree_idx* adjs;
  struct graphtree_idx* parent;

};

struct graphtree_idx* construct_graphtree_idx();
struct adjtree_idx* construct_adjtree_idx();
struct graphtree_leaf* construct_graphtree_leaf(struct graphtree_idx*p, uint32_t k, struct adjtree_idx* v);
struct adjtree_leaf* construct_adjtree_leaf(struct adjtree_idx* p, uint32_t k, struct graphtree_leaf* v);


struct adjtree_leaf
{
  //key,value
  uint32_t nodeid; //nodeid of the adjacent node
  struct graphtree_leaf* adjptr; //address of the nodeid

  struct adjtree_idx* parent;

};

void adjtree_inorder_traversal(void* env, struct adjtree_idx* root, void (*visitor)(void* env, uint32_t, struct graphtree_leaf*));


struct graphtree_leaf* find_graphtree_leaf(struct graphtree_idx* root, uint32_t key, uint8_t level);

uint8_t graphtree_insert_value(struct graphtree_idx* root, uint32_t new_key, struct adjtree_idx* new_value, uint8_t level);

uint8_t adjtree_insert_value(struct adjtree_idx* root, uint32_t new_key, struct graphtree_leaf* new_value, uint8_t level);


struct graphtree_leaf* pick_graphtree_leaf_rand(struct graphtree_idx* g);
#endif

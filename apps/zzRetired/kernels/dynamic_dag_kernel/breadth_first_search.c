#include <stdio.h>
#include <stdint.h>

#include "dynamic_set.h"
#include "breadth_first_search.h"
/*#include "grow_dag.h"*/


/* two sets of gray_nodes because we cannot perform inplace delete
 */


/*--------------------
  while(1) bfs_step()

  bfs_step()
  {
    2. visit all gray nodes;mark them as black (bfs_visit)
    3. populate discovered_nodes
    4. flip_switch gray_nodes <- discovered_nodes; clear(discovered_nodes)
    }
---------------------*/

void bfs_discover(void* x, uint32_t nodeid, struct graphtree_leaf* adjacency_tree)
{

  struct bfs_search_env* env = (struct bfs_search_env*)x;
  /*if not already gray,  mark as gray */
  if(nodeid ==env->desID){
    env->done = 1;
    env->res = 1;
  }
  struct vnode* vn = search_vnode(env->gray_nodes, nodeid, 0);
  if(!vn)
    {

      insert_value(env->discovered_nodes, nodeid, adjacency_tree,0);
    }
}

/* visit all gray nodes */
void bfs_visit(void* x, uint32_t nodeid, void* location)
{
  struct bfs_search_env* env = (struct bfs_search_env*) x;
  struct graphtree_leaf* currNode = (struct graphtree_leaf*) location;

  //  printf("gray-->black: %p %d \n", currNode, currNode->nodeid);
  /* mark as black node i.e.     never visit again
     remove from gray_nodes set
  */
  /* if nodeid is already black */
  struct vnode* vn = search_vnode(env->black_nodes,nodeid, 0);
  if(!vn)
    {
      insert_value(env->black_nodes, nodeid, location, 0);
      adjtree_inorder_traversal((void*)env, currNode->adjs, bfs_discover);
    }
}

void bfs_step(struct bfs_search_env* env)
{
  inorder_traversal(env, env->gray_nodes, bfs_visit); /*call bfs_visit(env, x) -- for each x in set env->gray_nodes */
  destruct_tree(env->gray_nodes); /* this nodes are now in black set */

  if(is_empty(env->discovered_nodes))
    {
      env->done = 1;
      return;
    }
  struct inode* gray_nodes_del = env->gray_nodes;

  env->gray_nodes = env->discovered_nodes;
  free(gray_nodes_del);
  env->discovered_nodes = construct_inode(); /* get ready for new step */
}


struct bfs_search_env* construct_env(struct graphtree_leaf* src, uint32_t desid)
{
  struct bfs_search_env* env = (struct bfs_search_env*) malloc(sizeof(struct bfs_search_env));
  env->gray_nodes= construct_inode();
  env->black_nodes = construct_inode();
  env->discovered_nodes = construct_inode();
  insert_value(env->gray_nodes, src->nodeid, src,0);
  env->desID= desid;
  env->done  = 0;
  env->res = 0;
}

void destruct_bfs_env(struct bfs_search_env* env)
{
  destruct_tree(env->gray_nodes);
  destruct_tree(env->discovered_nodes);
  destruct_tree(env->black_nodes);
  free(env->gray_nodes);
  free(env->black_nodes);
  free(env->discovered_nodes);

  free(env);
}

int reachabilitySearch(struct bfs_search_env* env)
{
  while(env->done == 0)
    {
      bfs_step(env);
    }
  destruct_tree(env->black_nodes);
  return env->res;
}



/* int main() */
/* { */

/*   /\* */
/*     1-->3 --> 4 --> 5 */
/*     1-->2--> 6-->7 */
/*    *\/ */
/*   struct graphtree_idx* G = construct_graphtree_idx(); */

/*   add_node(G, 1); */
/*   add_node(G, 2); */
/*   add_node(G, 3); */
/*   add_node(G, 4); */
/*   add_node(G, 5); */
/*   add_node(G, 6); */
/*   add_node(G, 7); */
/*   uint8_t l; */
/*   l= add_edge(G, 1, 2); */
/*   l = add_edge(G, 1,3); */
/*   l = add_edge(G, 1, 4); */
/*   l = add_edge(G, 3, 4); */
/*   l = add_edge(G, 4, 5); */
/*   l = add_edge(G, 2, 6); */
/*   l = add_edge(G, 6, 7); */


/*   /\* ---- init datastructre for bfs --- *\/ */



/*   /\* insert a source vertex *\/ */

/*   struct graphtree_leaf* srcNode = find_graphtree_leaf(G, 3, 0);  */
/*   struct bfs_search_env* env = construct_env(srcNode, 7); */
/*   int a = reachabilitySearch(env); */
/*   printf("%d \n ", a); */
/*   destruct_graphtree(G); */
/*   destruct_bfs_env(env); */
/*   /\* 0 --> level *\/ */
/*   /\*  insert_value(env->gray_nodes, srcNode->nodeid, srcNode,0); */
/*   printf("%d\n", srcNode->nodeid); */

/*   bfs_step(); */
/*   printf("step= %d \n", 2); */
/*   bfs_step(); */
/*   printf("step= %d \n", 3); */
/*   bfs_step(); */
/*   printf("step= %d \n", 4); */
/*   bfs_step();*\/ */
/* } */



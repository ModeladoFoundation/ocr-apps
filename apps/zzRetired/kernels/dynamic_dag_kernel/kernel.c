#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>

/* all routines to grow dag */
/*#include "dynamic_graph_impl.h"*/
#include "grow_dag.h"
#include "breadth_first_search.h"

int genOp()
{

  double y = drand48();
  if(y < .7)
    {
      return 1;       //grow-dag by one node;

    }
  if(y < .999)
    {
      return 2; //query-graph
    }

  return 3; //output-stats (1 in 1000)

}




/*---------------kernel variables--------------------*/
int num_steps = 0;
int total_steps = 50000;
struct graphtree_idx* dag;
int num_nodes = 0;
int num_queries = 0;
int num_connected = 0;

/*--------------------------------------------------*/


/*---------------kernel operators-------------------*/

void insert_node()
{
  addNewNode(dag);
  ++num_nodes;
}



void query_graph()
{
  struct graphtree_leaf* srcNode = pick_graphtree_leaf_rand(dag);
  struct graphtree_leaf* desNode = pick_graphtree_leaf_rand(dag);
  struct bfs_search_env* env = construct_env(srcNode, desNode->nodeid);
  reachabilitySearch(env);
  if(env->res == 1) ++num_connected;
  destruct_bfs_env(env);
  ++num_queries;
}


void print_stats()
{
  printf("--------#step = %d/%d----------\n", num_steps,total_steps);
  printf("#nodes = %d \n",num_nodes);
  printf("#queries = %d\n",num_queries);
  printf("#connected = %d\n\n\n", num_connected);

  /*TODO: Time elapsed */

}






/*----------------------------------------------------*/



void run()
{

  int i =0;
  /* boot-strap the kernel by
     initialize the graph with few nodes
  */
  for(; i < 1000; ++i)
    {
      ++num_steps;
      boot_strap_new_node(dag);
    }
  while(1)
    {
      if(num_steps == total_steps) return;
      int op = genOp();
      if(op == 1) insert_node();
      if(op == 2) query_graph();
      if(op == 3) print_stats();
      ++num_steps;
    }
}


int main(int argc, char** argv)
{
  int maxDeg = 8; /*  the max degree for any node in dag */
  grow_dag_init(maxDeg);
  dag = construct_graphtree_idx();
  run();
  destruct_graphtree(dag);
  free_dag_generator();
}

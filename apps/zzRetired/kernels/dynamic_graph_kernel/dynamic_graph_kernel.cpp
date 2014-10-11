
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
using namespace boost;
typedef adjacency_list<setS, setS, directedS, property<vertex_index_t, long int> , no_property, setS> Graph;
typedef graph_traits<Graph>::vertex_descriptor Vertex;
typedef graph_traits<Graph>::edge_descriptor Edge;

//include order is important
#include "reachabilitySearch.h"

#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include "Tree.h"
#include <utility>


int genOp()
{

  double y = drand48();


  if(y < .002)
    {
      return 0; //delete-node (1 in 1000)
    }
  if(y < .02)
    {
      return 1; //delete-edge; (20 in 1000)
    }

  if(y < .05)
    {
      return 2; //insert-node (50 in 1000)
    }
  if(y < .60)
    {
      return 3; //insert-edge (600 in 1000)

    }
  if(y < .99999)
    {
      return 4; //query-graph (350 in 1000)
    }

  return 5; //output-stats (1 in 1000)

}


int genRand(long int M)
{
  double r = drand48();
  double x = M * r;
  int y = (int)x;
  if(y < M)
    return y;
  else
    return 0;

}




namespace dynamic_rmat_benchmark
{

  long int numsteps = 10000000; //default: 10 million
  inode* rmat_gen_tree; //dynamic social network generator
  Graph dyn_graph;
  long int step = 0; //step id in each simulation
  std::map<long int, Vertex> key_2_graphID; //map node_key --> graph ID

  //create a Pool of edges;
  //randomly picked;
  //for delete_edge choose one from pool
  int pool_limit = 10000;
  typedef std::pair< std::pair<long int, long int>, Edge> pool_rec;
  std::vector< pool_rec > edge_pool;

  int num_node_ins =0;
  int num_node_del = 0;
  int num_edge_ins = 0;
  int num_edge_del = 0;
  int num_queries = 0;
  int num_reachable = 0;
  int num_nopath = 0;
  void delete_node()
  {
    //std::cout<<"Inside Delete Node "<<std::endl;
    vnode* leaf = choose_vnode(rmat_gen_tree, rmat_gen_tree);
    int k = leaf->key;
    Vertex v = key_2_graphID[k];
    clear_vertex(v, dyn_graph);   ///clear all edges
    remove_vertex(v, dyn_graph);  ///clear the node
    key_2_graphID.erase(key_2_graphID.find(k));
    delete_vnode(rmat_gen_tree, leaf);
    //std::cout<<"Deleting "<<k<<std::endl;
    ++num_node_del;
  }

  void delete_edge()
  {
    if(edge_pool.size() == 0)
      {
	//nothing in the pool to delete
	//would have to wait
	return;
      }
    //std::cout<<"Inside Delete Edge "<<std::endl;
    int pos;
    pool_rec picked;
    while(edge_pool.size() > 0)
      {
	pos = genRand(edge_pool.size());
	picked = edge_pool[pos];
	//could be an invalid edge
	//vertex might not be availabe
	if(key_2_graphID.find(picked.first.first) == key_2_graphID.end()  || key_2_graphID.find(picked.first.second) == key_2_graphID.end())
	  {
	    //src is invalid;
	        edge_pool[pos] = edge_pool.back();
		edge_pool.pop_back();

	  }
	else
	  {
	    break;
	  }
      }

    if(edge_pool.size() == 0) return;
    remove_edge(picked.second, dyn_graph);
    edge_pool[pos] = edge_pool.back();
    edge_pool.pop_back();


    ++num_edge_del;
  }

  void insert_node()
  {
    //step is the id of the node
    Vertex v = add_vertex(step, dyn_graph);
    vnode* leaf = insert_value(rmat_gen_tree, step);
    key_2_graphID[step] = v;
    ++num_node_ins;

  }
  void edge_inserter(int srcID, int desID)
  {
    if(srcID != desID)
      {
	Vertex vsrc = key_2_graphID[srcID];
	Vertex vdes = key_2_graphID[desID];
	bool res; Edge e;
	tie(e, res) = add_edge(vsrc, vdes, dyn_graph);
	///add to delete pool
	double p  = drand48();
	if( p < .1)
	  {
	    if(edge_pool.size() < pool_limit)
	      edge_pool.push_back(std::make_pair(std::make_pair(srcID, desID), e));
	    else
	      {
		int pos = genRand(edge_pool.size());
		edge_pool[pos] =
		  std::make_pair(std::make_pair(srcID,desID), e);
	      }

	  }
      }

  }

  void insert_edge()
  {
    rmat_jointWalk(rmat_gen_tree, edge_inserter); //invoke rmat-generator
    ++num_edge_ins;
  }

  void query_graph()
  {
    vnode* srcnode = choose_vnode(rmat_gen_tree, rmat_gen_tree);
    vnode* desnode = choose_vnode(rmat_gen_tree, rmat_gen_tree);
    Vertex vsrc = key_2_graphID[srcnode->key];
    Vertex vdes = key_2_graphID[desnode->key];
    bool res = isReachable(dyn_graph, vsrc, vdes);
    if(res) ++num_reachable;
    else ++num_nopath;
    //std::cout<<"Query "<<srcnode->key<<" "<<desnode->key<<" "<<res<<" "<<std::endl;
    ++num_queries;
    //perform bfs on

  }
 void stats()
  {
    std::cout<<"Step "<<step<<std::endl;
    std::cout<<"Node Insertions "<<num_node_ins<<std::endl;
    std::cout<<"Node Deletions "<<num_node_del<<std::endl;
    std::cout<<"Edge Insertions "<<num_edge_ins<<std::endl;
    std::cout<<"Edge Deletions "<<num_edge_del<<std::endl;
    std::cout<<"Num Queries "<<num_queries<<" reachable "<<num_reachable<<" "<<num_nopath<<std::endl;

    std::cout<<"DynGraph: num_vertex = "<<num_vertices(dyn_graph)<<" num_edges= "<<num_edges(dyn_graph)<<std::endl;
  }

  void run()
  {
    //initialize tree with some nodes
    //else rmat gen will be stuck in infinite loop
    for(int i = 0 ; i < 1000; ++i)
      {
	++step;
	insert_node();
      }

    while(1)
      {
	int op = genOp();
	if(op == 0) delete_node();
	if(op == 1) delete_edge();
	if(op == 2) insert_node();
	if(op == 3) insert_edge();
	if(op == 4) query_graph();
	if(op == 5) stats();
	++step;
      }
  }
}



int main()
{
  time_t t1;
  (void) time(&t1);
  srand48((long) t1);
  dynamic_rmat_benchmark::rmat_gen_tree = init_tree();
  dynamic_rmat_benchmark::run();

  return 0;
}

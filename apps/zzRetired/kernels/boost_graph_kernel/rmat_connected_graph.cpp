#include <math.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/property_map/property_map.hpp>

#include <boost/graph/connected_components.hpp>
#include <boost/graph/rmat_graph_generator.hpp>
#include <boost/random/linear_congruential.hpp>

using namespace boost;
using std::cout;
using std::ofstream;

typedef adjacency_list<vecS, vecS, undirectedS, property<vertex_index_t, int>, property<edge_index_t, int> > ugraph;
typedef graph_traits<ugraph>::vertex_descriptor Vertex;
typedef graph_traits<ugraph>::edge_descriptor Edge;
typedef graph_traits<ugraph >::vertex_iterator vertex_iter;
typedef graph_traits<ugraph >::edge_iterator edge_iter;

void get_giant_component()
{
  // std::vector<int> component(num_vertices(tg));
  // int num = connected_components(tg, &component[0]);
}

int main(int argc, char** argv)
{
  typedef rmat_iterator<boost::minstd_rand, ugraph> rmat_iterator_t;
  boost::minstd_rand gen;
  ofstream ofs_h("out_graph_h.txt");
  ofstream ofs("out_graph.txt");
  int k = atoi(argv[1]);
  int n = pow(2.0,k);
  int m = 8 * n;
  double a = .6;
  double b = .15;
  double c = .15;
  double d = .1;
  ugraph g(rmat_iterator_t(gen,n,m,a,b,c,d), rmat_iterator_t(), n);

  //  cout<<num_vertices(g)<<" "<<num_edges(g)<<std::endl;
  std::vector<int> vcomp(num_vertices(g));

  int num = connected_components(g, &vcomp[0]);
  //  cout<<"#comp = num"<<num<<std::endl;
  std::vector<int> compsz(num,0);
  for(int i = 0; i < num_vertices(g); ++i)
    {
      compsz[vcomp[i]]++;
    }
  int max_comp = 0;
  for(int i = 0; i < num; ++i)
    {
      if(compsz[i] > compsz[max_comp]) max_comp = i;
    }



  // for(int i = 0; i < num; ++i)
  //   {
  //           cout<<compsz[i]<<std::endl;
  //   }
  /* assign the nodes in largest component new identifier in range [0:compsz[max_comp]-1] */
  std::vector<int> new_id(num_vertices(g), 0);
  int new_id_counter = 0;
  for(int i = 0; i < num_vertices(g); ++i)
    {
      if(vcomp[i] == max_comp)
	{
	  new_id[i] = new_id_counter;
	  ++new_id_counter ;
	}

    }

  /*output the induced graph */
  edge_iter ei,ei_end;
  tie(ei,ei_end) = edges(g);
  int num_edges = 0;
  for(; ei !=ei_end; ++ei)
    {
      int srcid = source(*ei, g);
      int desid = target(*ei, g);
      if(vcomp[srcid] == max_comp && vcomp[desid] == max_comp)
	{
	  cout<<new_id[srcid]<<" "<<new_id[desid]<<std::endl;
	  ofs<<new_id[srcid]<<" "<<new_id[desid]<<std::endl;
	  ++num_edges;
	}
    }
  ofs_h<<compsz[max_comp]<<" "<<num_edges<<std::endl;
  ofs.close();
  return 0;
}

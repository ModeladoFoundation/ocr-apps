#include <boost/graph/use_mpi.hpp>
#include <boost/config.hpp>
#include <boost/throw_exception.hpp>
#include <boost/graph/distributed/adjacency_list.hpp>
#include <boost/graph/distributed/mpi_process_group.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/rmat_graph_generator.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/property_map/parallel/global_index_map.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/random.hpp>

#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <fstream>
#include <string>
#include <utility>
using std::string;
using std::ifstream;
using std::ofstream;
#define int_t long int
/* dummy edge list */
enum { u, v, x, y, z, N };
char name[] = { 'u', 'v', 'x', 'y', 'z' };
typedef double time_type;
typedef std::pair<int,int> E;
E edgearr[] = { E(u,y), E(u,x), E(u,v),
	      E(v,u),
	      E(x,y), E(x,v),
	      E(y,v), E(y,z),
	      E(z,u), E(z,x) };


using namespace boost;
using boost::graph::distributed::mpi_process_group;


typedef adjacency_list<listS, distributedS<mpi_process_group, vecS>, undirectedS, property<vertex_distance_t, std::size_t> > Graph;
typedef Graph::process_group_type pg_type;
typedef graph_traits<Graph>::edge_iterator edge_iter;
typedef Graph::process_group_type pg_type;
typedef property_map<Graph, vertex_index_t>::const_type
VertexIndexMap;
typedef property_map<Graph, vertex_global_t>::const_type
VertexGlobalMap;

inline time_type get_time()
{
  return MPI_Wtime();
}


void doBFS(Graph&lg, int n,  boost::variate_generator<boost::mt19937&, boost::uniform_int<> >&die)
{
  property_map<Graph, vertex_distance_t>::type distance =
    get(vertex_distance, lg);
  graph_traits<Graph>::vertex_iterator vi, vi_end;
  tie(vi,vi_end) = vertices(lg);
  int max = 100 * n;
  for(; vi != vi_end; ++vi)
    {
      put(distance, *vi,  max); /* initialize the distance/level property */
    }


  int x = die();                      // simulate rolling a die
  graph_traits<Graph>::vertex_descriptor start = vertex(x, lg);
  put(distance, start, 0);
  synchronize(distance);

   breadth_first_search
	(lg, start,
	 visitor(make_bfs_visitor(record_distances(distance, on_tree_edge()))));
}

int main(int argc, char** argv)
{
  boost::mpi::environment env(argc,argv);

  mpi_process_group pg;
  std::size_t id = process_id(pg);
  typedef parallel::variant_distribution<mpi_process_group> Distribution;
  int_t n = 5;

  string name_pre(argv[1]);
  std::cerr<<"running "<<name_pre<<std::endl;
  string nfn = name_pre + "_num_nodes.gr"; /* store num_nodes in the induced graph */
  ifstream nfs(nfn.c_str());
  nfs >> n;
  nfs.close();
  Distribution distrib = parallel::block(pg, n);
  std::vector<E> all_edges;

  string fn = name_pre +  boost::lexical_cast<string>(id) + string(".gr"); /* store the induced graphs, mapped edges */
  ifstream ifs(fn.c_str());
  int_t ne;
  int_t srcid,desid;
  ifs>>ne;
  ifs>>srcid>>desid;
  while(ifs)
    {
      all_edges.push_back(std::make_pair(srcid,desid));
      ifs>>srcid>>desid;
    }

  //  Graph lg(edgearr,  edgearr+sizeof(edgearr)/size(E), n, pg, distrib);
  Graph lg(all_edges.begin(),  all_edges.end(), n, pg, distrib);
  synchronize(lg);
  all_edges.clear();
  string ofn = name_pre +  boost::lexical_cast<string>(id) + string(".gr_test");
  ofstream ofs(ofn.c_str());
  edge_iter ei, ei_end;

  parallel::global_index_map<VertexIndexMap, VertexGlobalMap>
    global_index(pg, num_vertices(lg), get(vertex_index, lg),
		 get(vertex_global, lg));

  tie(ei,ei_end) = edges(lg);
  for(; ei!=ei_end; ++ei)
   {
     int_t source_idx = get(global_index, source(*ei, lg));
     int_t target_idx = get(global_index, target(*ei, lg));
     ofs<<source_idx<<" "<<target_idx<<std::endl;
   }
  synchronize(lg);

  //-------------------------------------------------------------------------------------//
  /* setup for the benchmark */
  boost::mt19937 rng;                 // produces randomness out of thin air
  // see pseudo-random number generators
  boost::uniform_int<> six(0,n-1);      // distribution that maps to 1..6
  // see random number distributions
  boost::variate_generator<boost::mt19937&, boost::uniform_int<> >
    die(rng, six);             // glues randomness with mapping

  //-------------------------------------------------------------------------------------//
  /* run the benchmark by executing 100 runs of bfs */
  time_type gen_start = get_time();
  int ctr=0;
  while(ctr < 50)
    {
      doBFS(lg,n, die);
    }
  time_type gen_end = get_time();
  time_type duration = gen_end - gen_start;
  if(id == 0)
    {
      int_t np  = num_processes(pg);
      int_t total_edges = 0;
      int_t ne= 0;
      for(int_t i = 0; i < np; i++)
	{
	  string efn = name_pre +  boost::lexical_cast<string>(id) + string(".gr");
	  ifstream ifs(fn.c_str());
	  ifs>>ne;
	  total_edges += ne;
	  ifs.close();
	}
      std::cout<<total_edges<<" "<<duration<<" "<<(double)(50*total_edges)/duration<<std::endl;
    }
  return 0;


}

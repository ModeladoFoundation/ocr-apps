#include <boost/graph/use_mpi.hpp>
#include <boost/config.hpp>
#include <boost/throw_exception.hpp>
#include <boost/graph/distributed/adjacency_list.hpp>
#include <boost/graph/distributed/mpi_process_group.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/rmat_graph_generator.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/random.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_map/parallel/global_index_map.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/distributed/connected_components_parallel_search.hpp>


#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <fstream>
#include <string>
#include <iostream>
#include <limits>
#include <math.h>

#define int_t long int
using std::ofstream;
using std::cout;
using namespace boost;
using std::string;
using boost::graph::distributed::mpi_process_group;
typedef double time_type;
typedef adjacency_list<listS, distributedS<mpi_process_group, vecS>, undirectedS, property<vertex_distance_t, std::size_t, property<vertex_index1_t, int> > > Graph;

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

std::string print_time(time_type t)
{
  std::ostringstream out;
  out << std::setiosflags(std::ios::fixed) << std::setprecision(2) << t;
  return out.str();
}





int main(int argc, char** argv)
{
  boost::mpi::environment env(argc,argv);

  mpi_process_group pg;
  std::size_t id = process_id(pg);

  int_t n = pow(2,atoi(argv[1])); int_t m =8 * n;
  typedef parallel::variant_distribution<mpi_process_group> Distribution;
  Distribution distrib = parallel::block(pg, n);
  typedef scalable_rmat_iterator<mpi_process_group, Distribution, rand48, Graph>
    RMATIter;



  rand48 gen;
  double a =0.56;
  double b= 0.17;
  double c = 0.17;
  double d = 1.0 -a -b -c;

 assert(a + b +c +d ==1.0);
 Graph lg(RMATIter(pg, distrib, gen, n, m, a, b, c, d, true),
	  RMATIter(), n, pg, distrib);

 synchronize(lg);

 parallel::global_index_map<VertexIndexMap, VertexGlobalMap>
   global_index(pg, num_vertices(lg), get(vertex_index, lg),
  		get(vertex_global, lg));
 string name_pre(argv[2]);


 /*---------------------------- begin connected component operation----------------------------------- */
 std::vector<int> local_components_vec(num_vertices(lg));
 typedef iterator_property_map<std::vector<int>::iterator, property_map<Graph, vertex_index_t>::type> ComponentMap;
 ComponentMap component(local_components_vec.begin(), get(vertex_index, lg));

 int num_components = 0;


 num_components = connected_components(lg, component);

 if ( process_id(lg.process_group()) == 0 )
   {
     component.set_max_ghost_cells(0);
     for (int_t i = 0; i < n; ++i)
       get(component, vertex(i, lg)); /* the comp-id of all vertices on node-0 */
     synchronize(component);
   }
 else
   {
     synchronize(component);
   }

 /*---------------------------------------------------------------------------------------------------------------*/

 /*-------------------extract the induced graph------------------------------------------------------------------*/
 if ( process_id(lg.process_group()) == 0 )
   {
     int max_comp = 0;

     std::vector<int_t> compsz(num_components,0);

     for (int_t i = 0; i < n; ++i)
       {
 	 int_t cid =  get(component, vertex(i, lg));
 	 ++compsz[cid];
       }
     for(int_t i = 0; i < num_components; ++i)
       {
	 if(compsz[i] > compsz[max_comp]) max_comp = i;
       }
     // }
     //- remapping the giant component ---//
     typedef property_map<Graph, vertex_index1_t>::type
       VertexIndex1Map;
     VertexIndex1Map idx1map = get(vertex_index1, lg);
     int_t serial_id = 0;

     for (int_t i = 0; i < n; ++i)
       {
	 int cid =  get(component, vertex(i, lg));
	 if(cid == max_comp)
	   {
	     put(idx1map, vertex(i,lg), serial_id);
	     ++serial_id;
	   }
	 else
	   {
	     put(idx1map, vertex(i,lg), std::numeric_limits<int>::max());
	   }
       }

     string nfn = name_pre + "_num_nodes.gr"; /* store num_nodes in the induced graph */
     ofstream nfs(nfn.c_str());
     nfs<<serial_id<<std::endl;
     synchronize(idx1map);
   }
 else
   {
     typedef property_map<Graph, vertex_index1_t>::type
       VertexIndex1Map;
     VertexIndex1Map idx1map = get(vertex_index1, lg);
     synchronize(idx1map);

   }
 /*------------------------------------------------------------------------------------------------------------------*/

 /* iterate over local edges */
 string fn = name_pre +  boost::lexical_cast<string>(id) + string(".gr"); /* store the induced graphs, mapped edges */
 ofstream ifs(fn.c_str());
 ifs<<num_edges(lg)<<std::endl;
 edge_iter ei, ei_end;

 tie(ei,ei_end) = edges(lg);
 typedef property_map<Graph, vertex_index1_t>::type
   VertexIndex1Map;
 VertexIndex1Map idx1map = get(vertex_index1, lg);
 for(; ei!=ei_end; ++ei)
   {
     int_t source_idx = get(global_index, source(*ei, lg));
     int_t target_idx = get(global_index, target(*ei, lg));
     int_t source_mid = get(idx1map, source(*ei,lg)); /* mid <--- mapped id */
     int_t target_mid = get(idx1map, target(*ei,lg));
     if(source_mid != std::numeric_limits<int_t>::max() && target_mid != std::numeric_limits<int_t>::max())
       {
	 ifs<<source_mid<<" "<<target_mid<<std::endl;
       }
   }
 ifs.close();
 synchronize(lg);

 return 0;

}

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>


#include "PathStore_bgl.h"
#include "DecoderDS.h"

using namespace boost;
typedef adjacency_list<vecS, vecS, directedS> cpg_t; //compressed graph storage
typedef graph_traits<cpg_t>::edge_descriptor edge_t;

int g_maxID;


namespace NaivePatternQuery_Impl
{

  DecoderDS::BaseGraph* bg;


  std::vector<NodeSet> ridx_node_label_map; //reverse index for node label map

  //------------------------------------------------------//
  //pqmash is mashup of partition and query (pid, qid)
  //(pid, qid) --> nid . This is reverse map
  //----------------------------------------------------//
  std::vector<std::vector<NodeSet> > ridx_node_pqmash_map;



  cpg_t cpg; //compressed pattern graph
  std::map<nref, int> nref_cpgnode_map; //nref--> node_of_cpg
  std::map<int, nref> cpgnode_nref_map;

  int num_qnodes; //number of query nodes
  int num_partitions;
  int max_labels; //labels are integer [0, maxlabel]

  int pgc_edge_counter;
  std::vector<int> aquery;

  void clear()
    {
      cpgnode_nref_map.clear();
      nref_cpgnode_map.clear();
      ridx_node_pqmash_map.clear();

    }
  void  BFS(int src,  PathStore<edge_t>& result)
  {
    int src_out_deg = out_degree(src, cpg);
    if(src_out_deg == 0)
      {
	list<list<edge_t> > paths;
	PathStore<edge_t> tmp(paths);
	result.unionPaths(tmp);
	return ;
      }
    BOOST_FOREACH(edge_t e, out_edges(src, cpg))
      {
	int des = target(e, cpg);
	PathStore<edge_t> ps;
	BFS(des,  ps);
	ps.prependEdge(e);
	result.unionPaths(ps);
      }
	  return;
  }

  void AllPathsOptimized(ostream& output)
  {
    set<int> roots;

    int root_color = aquery[0];
    for(int pid = 0; pid < num_partitions; ++pid)
      {
	foreach(nref nid, ridx_node_pqmash_map[pid][root_color])
	  {
	    roots.insert(nref_cpgnode_map[nid]);
	  }
      }
    nref rid;
    int counter = 0;

    BOOST_FOREACH(rid, roots)
      {
	PathStore<edge_t> res;
	BFS(rid, res);
	res.printPaths(cpg, num_qnodes, output);
	int psz = res.size();
      }
  }

  int add_edge(int srcid, int desid)
  {
    //    std::cout<<"Now adding "<<srcid<<" "<<desid<<std::endl;
    return bg->add_edge(srcid, desid);
  }

  void add_depth(int nid, int depth)
  {
    //    std::cout<<"Depth "<<nid<<" "<<depth<<std::endl;
    bg->set_depth(nid, depth);
  }

  void build_dag(int maxID)
  {
    bg = new DecoderDS::BaseGraph(maxID); //initialize the dag
    g_maxID  = maxID;//set global maxID
    grow_dag_init(6); //set 6 is maxDegree for grow_dag routine
    for(int i = 0 ; i <=maxID; ++i)
      {
	addNewNode(add_edge, add_depth);
      }
  }


  void init_pq_run(int qnodes, int ml)
  {
    num_qnodes = qnodes;
    num_partitions = 1;
    max_labels = ml;
    pgc_edge_counter = 0;
  }

  void build_ridx_node_pqmash_map()
  {
    ridx_node_pqmash_map.clear();
    ridx_node_pqmash_map.resize(num_partitions);
    for(int i = 0; i < num_partitions; ++i)
      {
	ridx_node_pqmash_map[i].clear();
	ridx_node_pqmash_map[i].resize(max_labels + 1);
      }

    for(int labelid =0; labelid <=max_labels; ++labelid)
      {
	foreach(nref nid, ridx_node_label_map[labelid]) //nodes of labelid
	  {
	    int pid = 0;
	    ridx_node_pqmash_map[pid][labelid].insert(nid);
	  }
      }
    /* for(int pid = 0; pid < num_partitions; ++pid) */
    /*   { */
    /* 	for(int qidx =0; qidx < num_qnodes; ++qidx) */
    /* 	  { */
    /* 	    int counter = 0; */
    /* 	    std::cout<<"Color "<<pid<<","<<aquery[qidx]<<" : "; */
    /* 	    foreach(nref nid, ridx_node_pqmash_map[pid][aquery[qidx]]) */
    /* 	      { */
    /* 		++counter; */
    /* 		std::cout<<nid<<" "; */
    /* 	      } */
    /* 	    std::cout<<" : "<<counter<<std::endl; */
    /* 	  } */
    /*   } */
  }

  //add nodes to cpggraph
  //and
  //maintina nid --> cpg_node  mapping
  void build_nref_cpgnode_map()
  {
    for(int labelid =0; labelid <=max_labels; ++labelid)
      {
	foreach(nref nid, ridx_node_label_map[labelid]) //nodes of labelid
	  {
	    int cpg_nid = add_vertex(cpg);
	    nref_cpgnode_map[nid] = cpg_nid;
	    cpgnode_nref_map[cpg_nid] = nid;
	  }
      }
  }


  void add_edge_cpg(nref nid, nref anc_nid)
  {

    bool res = DecoderDS::IS_ANCESTOR(*bg, anc_nid, nid);
    assert(res);

    int cpg_snid = nref_cpgnode_map[anc_nid];
    int cpg_dnid = nref_cpgnode_map[nid];
    //    std::cout<<"add_edge "<<cpg_snid<<" "<<cpg_dnid<<" "<<res<<std::endl;
    add_edge(cpg_snid, cpg_dnid, cpg);
  }

  void build_cpg() //cpg <-- compressed pattern_graph
  {
    for(int k = 0; k< num_qnodes -1; k++)
      {
	for(int i = num_partitions - 1; i >=  0; --i)
	  {
	    NodeSet& blocks1 = ridx_node_pqmash_map[i][aquery[k]];
	    for(int j = num_partitions -1; j >= i; --j)
	      {

		NodeSet& blocks2 = ridx_node_pqmash_map[j][aquery[k+1]];
		//std::cout<<"BuildPatternEdge "<<j<<","<<aquery[k]<<"("<<blocks1.size()<<")"<< "<--- "<<i<<","<<aquery[k+1]<<"("<<blocks2.size()<<")"<<std::endl;
		DecoderDS::ANCESTORS<DecoderDS::BaseGraph>(*bg, blocks1, blocks2, add_edge_cpg);
	      }
	  }
      }
  }

  std::ofstream ofs;
  void write_node_label(std::ostream& out, const int& v)
  {

  }

  void draw_cpg()
  {
    ofs.open("cpg_graph.dot");
    write_graphviz(ofs, cpg, write_node_label);
  }
}

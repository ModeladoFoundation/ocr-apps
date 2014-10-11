#include <boost/property_map/property_map.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <map>
//using namespace boost;


class myvisitor
{


public:

  myvisitor()
  {

  }
  template<class Edge, class Graph>
  void tree_edge(Edge e, Graph& g)
  {
  }
  template <class Edge, class Graph>
  void gray_target(Edge e, Graph& g)
  {
  }

  template <class Vertex, class Graph>
  void initialize_vertex(Vertex u, Graph& g)
  {
  }

  template <class Edge, class Graph>
  void non_tree_edge(Edge, Graph&) { }
  template <class Vertex, class Graph>
  void discover_vertex(Vertex u, Graph& g) {

  }
  template <class Vertex, class Graph>
  void examine_vertex(Vertex u, Graph& g)
  {


  }
  template <class Edge, class Graph>
  void examine_edge(Edge e, Graph& g)
  {

  }
  template <class Edge, class Graph>
  void black_target(Edge, Graph&)
  {
  }
  template <class Vertex, class Graph>
  void finish_vertex(Vertex u, Graph& g)
  {

  }

};


  template <class IncidenceGraph, class Buffer, class BFSVisitor,
            class ColorMap>
  bool breadth_first_reachability
    (const IncidenceGraph& g,
     typename graph_traits<IncidenceGraph>::vertex_descriptor s, typename graph_traits<IncidenceGraph>::vertex_descriptor d,
     Buffer& Q, BFSVisitor vis, ColorMap color)
  {
    function_requires< IncidenceGraphConcept<IncidenceGraph> >();
    typedef graph_traits<IncidenceGraph> GTraits;
    typedef typename GTraits::vertex_descriptor Vertex;
    typedef typename GTraits::edge_descriptor Edge;
    function_requires< BFSVisitorConcept<BFSVisitor, IncidenceGraph> >();
    function_requires< ReadWritePropertyMapConcept<ColorMap, Vertex> >();
    typedef typename property_traits<ColorMap>::value_type ColorValue;
    typedef color_traits<ColorValue> Color;
    typename GTraits::out_edge_iterator ei, ei_end;

    put(color, s, Color::gray());             vis.discover_vertex(s, g);
    Q.push(s);
    while (! Q.empty()) {
      Vertex u = Q.top(); Q.pop();            vis.examine_vertex(u, g);
      for (tie(ei, ei_end) = out_edges(u, g); ei != ei_end; ++ei) {

        Vertex v = target(*ei, g);            vis.examine_edge(*ei, g);
	if(v == d) return true;
        ColorValue v_color = get(color, v);
        if (v_color == Color::white()) {      vis.tree_edge(*ei, g);
          put(color, v, Color::gray());       vis.discover_vertex(v, g);
          Q.push(v);
        } else {                              vis.non_tree_edge(*ei, g);
          if (v_color == Color::gray())       vis.gray_target(*ei, g);
          else                                vis.black_target(*ei, g);
        }
      } // end for
      put(color, u, Color::black());          vis.finish_vertex(u, g);
    } // end while
    return false;
  } // breadth_first_visit



bool isReachable(Graph& g,  Vertex src, Vertex des)
{
  boost::queue<Vertex> Q;
  myvisitor vis;
  std::map<Vertex, default_color_type> c_store;
  associative_property_map<std::map<Vertex, default_color_type>  > cpmap(c_store);
  bool res = breadth_first_reachability(g, src, des, Q, vis, cpmap);
  return res;
}


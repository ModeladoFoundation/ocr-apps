//Provides a PartitionList suitable for Decoding
#include <vector>
#include <queue>
#include <set>
#include <cassert>
#include <map>
#include <boost/foreach.hpp>
#include "h_array_impl.h"

/* This DecoderDS is copied and modified from
   ReachabilityFinalPush/src/DagPartitioning/
   We would remove the serialize/unserialize and
   tie it with grow_dag routine
*/



#ifndef __DECODERDS_H
#define __DECODERDS_H

using std::vector;
using std::set;


using namespace boost;
namespace DecoderDS
{
  class BaseGraph
  {

    //unordered_map<int, NodeSet> childs;
    std::vector<NodeSet> m_childs;
    vector<NodeSet> m_parents;
    std::vector<int> m_topologicalRank;
    std::vector<int> m_topologicalOrder;
    std::vector<int> m_depth;
    NodeSet m_allNodes;
    int m_maxID;


  public:
  BaseGraph(int maxID):m_maxID(maxID)
      {
	m_childs.resize(maxID +1);
	m_parents.resize(maxID +1);
	m_topologicalOrder.resize(maxID +1);
	m_topologicalRank.resize(maxID+1);
	m_depth.resize(maxID + 1, 0);
	for(int i = 0; i <= maxID; ++i)
	  {
	    m_topologicalRank[i] = i;
	    m_topologicalOrder[i] = i;
	  }
      }


    int add_edge(int src, int des)
    {
      assert(src <= m_maxID);
      m_childs[src].insert(des);
      m_parents[des].insert(src);
      m_allNodes.insert(src);
      m_allNodes.insert(des);
      return m_depth[src] + 1;

    }
    void set_depth(int nid, int depth)
    {
      m_depth[nid] = depth;
    }
    const NodeSet& childrens(int nid) const
    {
      assert(nid <m_childs.size());
      return m_childs[nid];
    }
    int topologicalRank(int nid) const
    {
      return m_topologicalRank[nid];
    }

    int atTopologicalOrder(int pos) const
    {
      return m_topologicalOrder[pos];
    }
    const NodeSet& parents(int nid) const
    {
      return m_parents[nid];
    }
    int getMaxID() const
    {return m_maxID;}

    const NodeSet& allNodes()
    {
      return m_allNodes;
    }

    ~BaseGraph()
      {
	m_childs.clear();
	m_parents.clear();
	m_topologicalOrder.clear();
	m_topologicalRank.clear();
	m_depth.clear();
	m_topologicalRank.clear();
	m_topologicalOrder.clear();

      }


  };

  template<class DAG_t>
  bool IS_ANCESTOR(const DAG_t& dag, nref src, nref des)
    {
      NodeSet visited;
      std::queue<int> Q;
      Q.push(des);
      while(!Q.empty())
	{
	  nref top = Q.front(); Q.pop();
	  if(top == src){
	    return true;
	  }
	  const NodeSet& top_parents = dag.parents(top);
	  BOOST_FOREACH(nref pid, top_parents)
	    {
	      if(visited.find(pid) == visited.end())
		{
		  Q.push(pid);
		  visited.insert(pid);
		}
	    }

	}
      return false;
    }

  template<class DAG_t>
    void ANCESTORS(const DAG_t& dag, const NodeSet& restricted, const NodeSet& preserve, void (*callback)(nref, nref) )
    {
    int maxID = dag.getMaxID();
    h_array<nref, int> marker;
    std::map<nref, NodeSet> result;
    NodeSet empty;
    for(int i = 0; i <= maxID; ++i)
      {
	nref nid = dag.atTopologicalOrder(i);

	if(nid == -1) continue;
	//	std::cout<<"Rank = "<<iCom<<" Nid = "<<nid<<std::endl;

	nref pid;
	const NodeSet& nidparents = dag.parents(nid);
	const NodeSet& nidchilds = dag.childrens(nid);
	marker[nid] = nidchilds.size();
	if(nidparents.size() == 0)
	  {
	    result[nid] = empty;
	  }
	else
	  {
	    NodeSet myancestors;

	    foreach(pid, nidparents)
	      {
		NodeSet& panc = result[pid];

		foreach(int xid, panc) //ancestors of my parents are my ancestors
		  {
		    myancestors.insert(xid);
		  }
		if( preserve.find(pid) == preserve.end())
		  {
		    marker[pid] = marker[pid] - 1;
		    assert(marker[pid] >=0);
		    if(marker[pid] == 0)
		      {
			result.erase(result.find(pid));
			//result.removeItemValue(pid);
		      }
		  }
		if(restricted.find(pid) !=restricted.end())
		  myancestors.insert(pid);
	      }
	    result[nid] = myancestors;
	    if(preserve.find(nid) != preserve.end())
	      {
	    foreach(nref aid, myancestors)
	      {
		callback(nid, aid); //callback with (nid, nid_anc)
	      }
	      }
	  }
	//if(i%500000 == 0) std::cout<< "In computing Ancestors "<<i<<std::endl;
      }
    return;
  }

}
#endif

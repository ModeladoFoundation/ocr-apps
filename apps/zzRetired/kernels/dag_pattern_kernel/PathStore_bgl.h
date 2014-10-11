#include <list>
#include <fstream>
#include <boost/foreach.hpp>

using std::ostream;
using std::list;

#ifndef __PATHSTORE_H
#define __PATHSTORE_H

template <typename edgeType>
class PathStore
{
	typedef list<edgeType> path;
	private:
	int m_status;
	list< path> m_paths;
	public:
	PathStore()
	{
		m_status = -1;
	}
	PathStore(list<path>& paths):m_paths(paths)
	{
		m_status = 0;
	}

	void prependEdge(edgeType e)
	{
	  if(m_status == -1) return;

	  if(m_paths.size() == 0)
	    {
	      path p;
	      p.push_back(e);
	      m_paths.push_back(p);
	      return ;
	    }
	  BOOST_FOREACH(list<edgeType>& l, m_paths)
	    {
	      l.push_front(e);
	    }
	}

	const list<path>& allPaths() const
	{
		return m_paths;
	}
	void unionPaths(const PathStore& ps)
	{
		if(ps.m_status != -1)
		{

		  const list<path>& psPaths = ps.allPaths();
		  BOOST_FOREACH(const list<edgeType>& l, psPaths)
		    {
		      m_paths.push_back(l);
		    }

		  m_status = 0;
		}
	}
	PathStore& operator=(const PathStore& x)
	{
		if(this != &x)
		{
			m_status = x.m_status;
			m_paths = x.m_paths;
		}
		return *this;
	}
	void printPath(const list<edgeType>& l, ostream& os)
	{
		edgeType e;
		BOOST_FOREACH(e, l)
		{
		  //os<<e<<" ";
		}

	}
	template<typename G>
	void printPath(const G& rg, const list<edgeType>& l, ostream& os)
	{

	  edgeType e = l.front();
	  int src = source(e, rg);
	  //os<<src<<" ";

	  BOOST_FOREACH(e, l)
	    {
	      int des = target(e, rg);
	      // os<<des<<" ";
	    }
	  //os<<std::endl;

	}

	template<typename G>
	  void printPaths(G& rg, int qsz, ostream& os)
	  {
	    BOOST_FOREACH(const list<edgeType>& l, m_paths)
	      {
		if(l.size() == qsz - 1)
		  {
		    printPath(rg,l, os);
		  }
	      }
	  }

	/* void printPaths(ostream& os) */
	/* { */
	/* 	int sz = m_paths.size(); */
	/* 	for(int i = 0; i < sz; i++) */
	/* 	{ */
	/* 		const list<edgeType>& l = m_paths[i]; */
	/* 		if(l.size() > 0) */
	/* 		{ */
	/* 		  printPath(l, os); */
	/* 		  os<<std::endl; */
	/* 		  //std::cout<<" XXXXXXXXXXXXXXXXXXXXXXXX"<<endl; */
	/* 		} */
	/* 	} */
	/* } */
	int size() const
	{
		return m_paths.size();
	}
	const path& getPath(int i) const
	{
		return m_paths[i];
	}

};
#endif



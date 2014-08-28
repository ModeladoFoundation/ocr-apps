#include <boost/unordered_map.hpp>

#ifndef __H_ARRAY_
#define __H_ARRAY_
template <class X, class Y>
  class h_array : public boost::unordered_map<X,Y>
  {
  public:
    bool defined(X x) const
    {
      if(boost::unordered_map<X,Y>::find(x) !=boost::unordered_map<X,Y>::end()) return true;
      return false;
    }
    void undefine(X x)
    {
      erase(find(x));
    }
  };


#endif

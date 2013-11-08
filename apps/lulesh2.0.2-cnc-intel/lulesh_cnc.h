
#include "cnc/cnc.h"
#include <vector>

using namespace std;
using namespace CnC;

class lulesh_context;
class Data;


struct countedStruct {
  static int64_t count;
  countedStruct() {
    // int64_t tmp = __sync_add_and_fetch(&count, 1);
    // if (tmp % 10000 == 0) 
    //   cout << "CREATED " << tmp << " CNC steps" << endl;
  } 
};

class ElemTag : countedStruct{
  vector<int64_t> m_vec;
  
public:
  ElemTag() {}
  ElemTag(int64_t i) {      
    m_vec.push_back(i);
  }
  ElemTag(const ElemTag &tt) : m_vec(tt.m_vec) {}  

  bool operator==(const ElemTag &tt) const {
    return m_vec[0] == tt[0];
  }

  operator size_t() const {
    return m_vec.size() * sizeof (int64_t);
  }

  int64_t operator[](int64_t i) const {
    return m_vec[i];
  }

  ostream& operator<< (ostream& stream) {  
    for (int64_t i=0; i<1; i++)
      stream << (*this)[i] << " ";
    
    return stream;
  }

#ifdef _DIST_
  // marshalling for distCnC
  void serialize( serializer & ser ) {
    ser & chunk<int64_t, no_alloc>(vec.begin(), m_vec.size());
  }
#endif
};

template <>
bool cnc_tag_hash_compare< ElemTag >::equal(const ElemTag& a, const ElemTag& b) const
{
  return a == b;
}

std::ostream & cnc_format( std::ostream& os, const ElemTag & t ) {
  os << "(" << t[0] << ")";
  return os;
}

ostream& operator<< (ostream& stream, const ElemTag &tt) {
  for (int64_t i=0; i<1; i++)
    stream << tt[i] << " ";
  return stream;
}

// our stencil core
class initStressTermsForElems_step
{
  // Step local variables (also has persistence because of current implementation)
public:
  int64_t execute( const ElemTag & t, lulesh_context & c ) const;
};


// our stencil core
class initStressTermsForElemsPutBack_step
{
  // Step local variables (also has persistence because of current implementation)
public:
  int64_t execute( const ElemTag & t, lulesh_context & c ) const;
};


class lulesh_context : public context< lulesh_context > {
  Domain *globalDomainPointer;
  Real_t *sigxx, *sigyy, *sigzz;

public:
  Domain *getGlobalDomainPointer() {return globalDomainPointer;}
  void setGlobalDomainPointer(Domain *d) {globalDomainPointer = d;}

  void getGlobalSigPointer(Real_t * &x, Real_t * &y, Real_t * &z) {
    x = sigxx;
    y = sigyy;
    z = sigzz;
  }
  void setGlobalSigPointer(Real_t * &x, Real_t * &y, Real_t * &z) {
    sigxx = x; sigyy = y; sigzz = z;
  }

  item_collection< ElemTag, Data> itemInitStressTermsForElems;
  step_collection<initStressTermsForElems_step> stepInitStressTermsForElems;
  tag_collection< ElemTag > tagInitStressTermsForElems;

  step_collection<initStressTermsForElemsPutBack_step> stepInitStressTermsForElemsPutBack;
  tag_collection< ElemTag > tagInitStressTermsForElemsPutBack;

  lulesh_context() : 
    itemInitStressTermsForElems(*this, "itemInitStressTermsForElems"), 
    stepInitStressTermsForElems(*this), 
    tagInitStressTermsForElems(*this),
    stepInitStressTermsForElemsPutBack(*this), 
    tagInitStressTermsForElemsPutBack(*this) 
  { 
    // prescription
    tagInitStressTermsForElems.prescribes( stepInitStressTermsForElems, *this );
    tagInitStressTermsForElemsPutBack.prescribes( stepInitStressTermsForElemsPutBack, *this );

    // producer/consumer
    stepInitStressTermsForElems.produces( itemInitStressTermsForElems );
    stepInitStressTermsForElemsPutBack.consumes( itemInitStressTermsForElems );
  }
};

struct Data {
  Real_t m_xx, m_yy, m_zz;
  Data(Real_t &x, Real_t &y, Real_t &z) : m_xx(x), m_yy(y), m_zz(z) {}

  Data() {};
  Data(const Data &d) : m_xx(d.m_xx), m_yy(d.m_yy), m_zz(d.m_zz) {};
  Data &operator=(const Data &d) {
    if (&d == this) return *this;
    m_xx = d.m_xx;
    m_yy = d.m_yy;
    m_zz = d.m_zz;
    return *this;
  };

  Real_t &xx() { return m_xx;}
  Real_t &yy() { return m_yy;}
  Real_t &zz() { return m_zz;}
};

template <>
size_t cnc_tag_hash_compare< ElemTag >::hash(const ElemTag& tt) const
{
  return tt[0] * 19;
}

int64_t initStressTermsForElems_step::execute
(const ElemTag & t, lulesh_context & c ) const {
  Domain &domain = *(c.getGlobalDomainPointer());

  int64_t i = t[0];
  Real_t data = - domain.p(i) - domain.q(i);
  Data d(data, data, data);
  c.itemInitStressTermsForElems.put(t, d);
  return CNC_Success;
}

int64_t initStressTermsForElemsPutBack_step::execute
(const ElemTag & t, lulesh_context & c ) const {
  Real_t *sigxx, *sigyy, *sigzz;
  c.getGlobalSigPointer(sigxx, sigyy, sigzz);
  Data d;
  c.itemInitStressTermsForElems.get(t, d);

  int64_t i = t[0];
  sigxx[i] = d.xx();
  sigyy[i] = d.yy();
  sigzz[i] = d.zz();

  return CNC_Success;
}

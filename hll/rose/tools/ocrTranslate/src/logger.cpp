#include "sage3basic.h"
#include "logger.h"
#include <typeinfo>
#include <boost/xpressive/xpressive_static.hpp>
#include <boost/xpressive/regex_actions.hpp>

using namespace std;
using namespace boost::xpressive;

namespace Logger {

  Logger glg(DEBUG);

  Logger::Logger() : m_level(INFO), m_scope("") { }

  Logger::Logger(SeverityLevel level) : m_level(level), m_scope("") { }

  Logger::Logger(string scope) : m_level(INFO), m_scope(scope) { }

  Logger::Logger(string scope, SeverityLevel level) : m_level(level), m_scope(scope) {
    if(m_level <= DEBUG && m_scope.length() > 0) m_oss << "--- " << m_scope << " ---\n";
  }

  SeverityLevel Logger::getSeverityLevel() const {
    return m_level;
  }

  ostream& Logger::getOSS() {
    return m_oss;
  }

  ostream& Logger::getNullStream() {
    return m_nulls;
  }

  Logger::~Logger() {
    if(m_level <= DEBUG && m_scope.length() > 0) m_oss << "--- " << m_scope << " ---\n";
    std::cout << m_oss.str();
  }

  ostream& debug(Logger& lg) {
    if(lg.getSeverityLevel() <= DEBUG) return lg.getOSS();
    else return lg.getNullStream();
  }

  ostream& info(Logger& lg) {
    if(lg.getSeverityLevel() <= INFO) return lg.getOSS();
    else return lg.getNullStream();
  }

  ostream& error(Logger& lg) {
    return std::cerr;
  }

};

namespace AstDebug {
  string astToString(SgNode* sgn, string tabspace) {
    if(!sgn) return "(null)";
    ostringstream ss;
    string indent;
    ss << tabspace << "(" << astTypeName(sgn);
    if(sgn->get_file_info()) {
      ss << "_l" << sgn->get_file_info()->get_line();
    }
    int nchild = sgn->get_numberOfTraversalSuccessors();
    if(nchild > 0) indent = tabspace + "  ";
    for(int i = 0; i < nchild; ++i) {
      ss << "\n" << indent;
      SgNode* sgchild_ = sgn->get_traversalSuccessorByIndex(i);
      ss << astToString(sgchild_, indent);
    }
    ss << ")";
    return ss.str();
  };

  string astTypeName(SgNode* sgn) {
    if(!sgn) return "null";
    stringstream ss;
    string typename_s = typeid(*sgn).name();
    return format(typename_s);
  }

  string format(string in_s) {
    mark_tag char_(1);
    // skip the leading digits and keep only the string that comes after
    sregex rx = *_d >> (char_=*_w);
    string out_s = regex_replace(in_s, rx, "" + char_);
    return out_s;
  }
};

namespace StrUtil {
  string stmtlist2str(list<SgNode*> stmtlist, string indent) {
    ostringstream oss;
    list<SgNode*>::iterator s = stmtlist.begin();
    for( ; s != stmtlist.end(); ++s) {
      oss << AstDebug::astToString(*s, indent) << endl;
    }
    return oss.str();
  }

  string sgnlist2str(list<SgNode*> sgnlist){
    std::ostringstream oss;
    oss << "[";
    std::list<SgNode*>::iterator i = sgnlist.begin();
    while(i != sgnlist.end()) {
      if(SgVarRefExp* sgv = isSgVarRefExp(*i)) {
	oss << sgv->get_symbol()->get_name().getString();
      }
      else {
	oss << AstDebug::astToString(*i);
      }
      ++i;
      if(i != sgnlist.end()) oss << ", ";
    }
    oss << "]";
    return oss.str();
  }
};

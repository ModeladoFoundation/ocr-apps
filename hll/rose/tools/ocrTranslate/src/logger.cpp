#include "sage3basic.h"
#include "logger.h"
#include <typeinfo>
#include <boost/xpressive/xpressive_static.hpp>
#include <boost/xpressive/regex_actions.hpp>

using namespace std;
using namespace boost::xpressive;

namespace Logger {

  Logger glg(DEBUG);

  Logger::Logger() : m_level(INFO), m_scope("Global") { }

  Logger::Logger(SeverityLevel level) : m_level(level), m_scope("Global") { }

  Logger::Logger(string scope, SeverityLevel level) : m_scope(scope), m_level(level) { }

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
    ss << "(" << astTypeName(sgn);
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

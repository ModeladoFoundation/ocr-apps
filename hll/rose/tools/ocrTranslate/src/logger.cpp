#include "sage3basic.h"
#include "logger.h"
#include <typeinfo>
#include <boost/xpressive/xpressive_static.hpp>
#include <boost/xpressive/regex_actions.hpp>

using namespace std;
using namespace boost::xpressive;

namespace Logger {

  Logger::Logger() : m_level(IGNORE), m_scope("") { }

  Logger::Logger(SeverityLevel level) : m_level(level), m_scope("") { }

  Logger::Logger(string scope) : m_level(IGNORE), m_scope(scope) { }

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
    if(!sgn) return tabspace + "(null)";
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

namespace SgNodeUtil {
  SgVarRefExp* identifier2sgn(std::string identifier_, SgNode* anchor) {
    Logger::Logger lg("OcrTaskPragmaParser::identifier2sgn");
    SgVarRefExp* idsgn;
    AstFromString::c_char = identifier_.c_str();
    AstFromString::c_sgnode = anchor->get_parent();
    if(AstFromString::afs_match_identifier()) {
      idsgn = isSgVarRefExp(AstFromString::c_parsed_node);
    }
    else {
      Logger::error(lg) << "Cannot create SgNode from identifer_=" << identifier_ << endl;
      assert(false);
    }
    assert(idsgn);
    return idsgn;
  }

  list<SgVarRefExp*> identifiers2sgnlist(list<string> identifiersList, SgNode* anchor) {
    Logger::Logger lg("OcrTaskPragmaParser::identifiers2sgnlist");
    list<SgVarRefExp*> idsgnList;
    list<string>::iterator i = identifiersList.begin();
    for( ; i != identifiersList.end(); ++i) {
      SgVarRefExp* idsgn = identifier2sgn(*i, anchor);
      Logger::debug(lg) << "idsgn=" << AstDebug::astToString(idsgn) << endl;
      assert(idsgn);
      idsgnList.push_back(idsgn);
    }
    return idsgnList;
  }
};

namespace StrUtil {
  string SgVarRefExp2Str(SgVarRefExp* sgvref) {
    return sgvref->get_symbol()->get_name().getString();
  }

  string SgVarRefExpList2Str(const list<SgVarRefExp*>& varRefExpList) {
    ostringstream oss;
    oss << "[";
    list<SgVarRefExp*>::const_iterator i = varRefExpList.begin();
    while(i != varRefExpList.end()) {
      oss << SgVarRefExp2Str(*i);
      ++i;
      if(i != varRefExpList.end()) oss << ", ";
    }
    oss << "]";
    return oss.str();
  }

  string SgInitializedName2Str(SgInitializedName* sgn) {
    return sgn->get_name().getString();
  }

  string SgInitializedNamePtrList2Str(list<SgInitializedName*>& sginitnamelist) {
    ostringstream oss;
    oss << "[";
    list<SgInitializedName*>::iterator i = sginitnamelist.begin();
    while(i != sginitnamelist.end()) {
      oss << (*i)->get_name().getString();
      ++i;
      if(i != sginitnamelist.end()) oss << ", ";
    }
    oss << "]";
    return oss.str();
  }

  string strlist2str(list<string>& strList) {
    ostringstream oss;
    oss << "[";
    list<string>::iterator l;
    for(l = strList.begin(); l != strList.end(); ) {
      oss << *l++;
      if(l != strList.end()) oss << ", ";
    }
    oss << "]";
    return oss.str();
  }

  string strlist2str(const list<string>& strList) {
    ostringstream oss;
    oss << "[";
    list<string>::const_iterator l;
    for(l = strList.begin(); l != strList.end(); ) {
      oss << *l++;
      if(l != strList.end()) oss << ", ";
    }
    oss << "]";
    return oss.str();
  }

  string stmtlist2str(const list<SgNode*>& stmtlist, string indent) {
    ostringstream oss;
    list<SgNode*>::const_iterator s = stmtlist.begin();
    for( ; s != stmtlist.end(); ++s) {
      oss << AstDebug::astToString(*s, indent) << endl;
    }
    return oss.str();
  }


  string stmtlist2str(const list<SgStatement*>& stmtlist, string indent) {
    ostringstream oss;
    list<SgStatement*>::const_iterator s = stmtlist.begin();
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

  // Given a raw filename with path and suffix return only the filename without path and suffix
  string GetFileNameString(string rawfilename) {
    string filenameNoPath = Rose::StringUtility::stripPathFromFileName(rawfilename);
    return Rose::StringUtility::stripFileSuffixFromFileName(filenameNoPath);
  }
};

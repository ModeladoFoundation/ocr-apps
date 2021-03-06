/*****************************************
 * Author: Sriram Aananthakrishnan, 2016 *
 *****************************************/
#pragma once
/*
 * A very simple logging facility to turn on/off messages
 */
#include <ostream>
#include <iostream>
#include <sstream>

namespace Logger {

  class NullBuffer : public std::streambuf {
  public:
    int overflow(int c) { return c; }
  };

  class NullStream : public std::ostream {
    NullBuffer m_nb;
  public:
  NullStream() : std::ostream(&m_nb) { }
  };

  enum SeverityLevel {
    DEBUG,
    INFO,
    IGNORE,
    ERROR
  };

  class Logger {
    NullStream m_nulls;
    std::ostringstream m_oss;
    SeverityLevel m_level;
    std::string m_scope;
  public:
    Logger();
    Logger(SeverityLevel level);
    Logger(std::string scope, SeverityLevel level);
    Logger(std::string scope);
    SeverityLevel getSeverityLevel() const;
    std::ostream& getOSS();
    std::ostream& getNullStream();
    ~Logger();
  };

  extern Logger glg;

  std::ostream& debug(Logger& lg);
  std::ostream& info(Logger& lg);
  std::ostream& error(Logger& lg);
}; // end namespace

// TODO: Move this to a separate file
namespace AstDebug {
  //! Print the sub-tree as a string
  //! Warning: Do not use it on a large sub tree
  //! Re-implementing this function based on Markus Schordan's AstTerm
  std::string astToString(SgNode* sgn, std::string indent="");
  //! Return the typename as string
  std::string astTypeName(SgNode* sgn);
  std::string format(std::string in);
};

namespace SgNodeUtil {
  SgVarRefExp* identifier2sgn(std::string identifier_, SgNode* anchor);
  std::list<SgVarRefExp*> identifiers2sgnlist(std::list<std::string> identifiersList, SgNode* anchor);
}

namespace StrUtil {
  // Assumption: Object of type AnyType implements str() method
  template <typename AnyType>
    std::string list2str(const std::list<boost::shared_ptr<AnyType> >& listContainer) {
    std::ostringstream oss;
    oss << "[";
    typename std::list<boost::shared_ptr<AnyType> >::const_iterator lbegin = listContainer.begin();
    typename std::list<boost::shared_ptr<AnyType> >::const_iterator lend = listContainer.end();
    while(lbegin != lend) {
      boost::shared_ptr<AnyType> item = *lbegin;
      oss << item->str();
      ++lbegin;
      if(lbegin != lend) oss << ", ";
    }
    oss << "]";
    return oss.str();
  }

  std::string SgInitializedName2Str(SgInitializedName* sgn);
  std::string SgInitializedNamePtrList2Str(std::list<SgInitializedName*>& sginitnamelist);
  std::string SgVarRefExp2Str(SgVarRefExp* sgvref);
  std::string SgVarRefExpList2Str(const std::list<SgVarRefExp*>& varRefExpList);
  std::string strlist2str(std::list<std::string>& strList);
  std::string strlist2str(const std::list<std::string>& strList);
  std::string stmtlist2str(const std::list<SgStatement*>& stmtlist, std::string indent="");
  std::string stmtlist2str(const std::list<SgNode*>& stmtlist, std::string indent="");
  std::string sgnlist2str(std::list<SgNode*> sgnlist);

  // Given a raw filename with path and suffix return only the filename without path and suffix
  std::string GetFileNameString(std::string rawfilename);
};

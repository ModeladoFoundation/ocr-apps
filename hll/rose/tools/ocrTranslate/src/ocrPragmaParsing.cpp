/*
 * Author: Sriram Aananthakrishnan, 2016 *
 */
#include "sage3basic.h"
#include "ocrPragmaParsing.h"
#include "AstFromString.h"
#include "RoseAst.h"
#include <string>
#include "logger.h"

using namespace std;
using namespace boost::xpressive;

/******************
 * MatchException *
 ******************/
MatchException::MatchException(std::string what_s) : m_what(what_s) { }

const char* MatchException::what() const throw() {
  return m_what.c_str();
}

MatchException::~MatchException() throw() {
}

/***********************
 * OcrTaskPragmaParser *
 ***********************/
OcrTaskPragmaParser::OcrTaskPragmaParser(const char* pragmaStr) : m_pragmaStr(pragmaStr) {
  //! Here we specify the grammar for our task pragma
  //! identifier is the basic building block similiar to a C++ identifier
  //! attr is the attribute attached to variable either in or out
  //! param is either just a identifer or an identifier with attributes
  //! paramlist is a list of param separated by ','
  //! taskName is TASK(identifier)
  //! depEvts is DEP_EVTs(paramlist)
  //! depDBKs is DEP_DBKs(paramlist)
  //! depElems is DEP_ELEMs(paramlist)
  //! taskBeginPragma is the sequence taskName depEvts depDBKs depElems
  identifier = +(alpha|as_xpr('_')) >> *_w;
  attr = icase("in:") | icase("out:");
  param = *_s >> *by_ref(attr) >> *_s >> identifier;
  // paramseq is the tail seq of a parameter list
  sregex paramseq = *_s >> as_xpr(',') >> *_s >> param;
  paramlist = as_xpr('(') >> *_s >> param >> *by_ref(paramseq) >> *_s >> as_xpr(')');
  taskName = *_s >> icase("TASK") >> *_s >> by_ref(paramlist);
  depEvts = *_s >> icase("DEP_EVTS") >> *_s >> by_ref(paramlist);
  depDbks = *_s >> icase("DEP_DBKs") >> *_s >> by_ref(paramlist);
  depElems = *_s >> icase("DEP_ELEMs") >> *_s >> by_ref(paramlist);
  outEvts = *_s >> icase("OEVENT") >> *_s >> by_ref(paramlist);
  taskBeginPragma = taskName >> *_s >> depEvts >> *_s >> depDbks >> *_s >> depElems >> *_s;
  taskEndPragma = *_s >> outEvts >> *_s;
}

//! A parameter is an identifier with attribute
//! matchIdentifier separates the identifier from the attribute
bool OcrTaskPragmaParser::matchIdentifier(string input_s, string& identifier_s) {
  Logger::Logger lg;
  Logger::debug(lg) << "matchIdentifier(input_s=" << input_s << ")\n";
  sregex attrkey = icase("in") | icase("out");
  sregex attrformat = attrkey >> as_xpr(':');
  sregex onlyid = *by_ref(attrformat) >> (s1=identifier);
  smatch matchResults;
  if(regex_search(input_s, matchResults, onlyid)) {
    identifier_s = matchResults[1];
    if(identifier_s.length() == 0) return false;
    else if(identifier_s.compare("NONE") == 0 ||
	    identifier_s.compare("none") == 0) return false;
    else return true;
  }
  else throw MatchException("MatchException from matchIdentifier\n");
  return false;
}

bool OcrTaskPragmaParser::matchParam(string input_s, list<string>& paramList) {
  Logger::Logger lg;
  Logger::debug(lg) << "matchParam(input_s=" << input_s << ")\n";
  smatch matchResults;
  if(!regex_match(input_s, matchResults, paramlist)) {
    throw MatchException("MatchException thrown by matchParam");
  }
  sregex_token_iterator cur(input_s.begin(), input_s.end(), param), end;
  for( ; cur != end; ++cur) {
    string identifier_s;
    if(matchIdentifier(*cur, identifier_s)) {
      paramList.push_back(identifier_s);
    }
  }
  return true;
}

// Given a string match a list of parameters "(param1,param2,..,paramN)"
bool OcrTaskPragmaParser::matchParamList(string input_s, list<string>& paramList) {
  smatch matchResults;
  if(regex_search(input_s, matchResults, paramlist)) {
    string paramlist_s = matchResults[0];
    return matchParam(paramlist_s, paramList);
  }
  else throw MatchException("matchParamList");
}

bool OcrTaskPragmaParser::matchTaskName(string input_s, string& taskName_s) {
  smatch matchResults;
  sregex taskNameOnly = *_s >> icase("TASK") >> *_s >> '(' >> *_s >> (s1=identifier) >> *_s >> ')';
  if(regex_search(input_s, matchResults, taskNameOnly)) {
    taskName_s = matchResults[1];
    return true;
  }
  else {
    throw MatchException("matchTaskName");
  }
  return false;
}

bool OcrTaskPragmaParser::matchDepEvts(string input_s, list<string>& depEvtsList) {
  smatch matchResults;
  if(regex_search(input_s, matchResults, depEvts)) {
    string depEvtsMatch = matchResults[0];
    matchParamList(depEvtsMatch, depEvtsList);
  }
  else throw MatchException("matchDepEvts Exception\n");
}

bool OcrTaskPragmaParser::matchDepDbks(string input_s, list<string>& depDbksList) {
  smatch matchResults;
  if(regex_search(input_s, matchResults, depDbks)) {
    string depDbksMatch = matchResults[0];
    matchParamList(depDbksMatch, depDbksList);
  }
  else throw MatchException("matchDepDbks Exception\n");
}

bool OcrTaskPragmaParser::matchDepElems(string input_s, list<string>& depElemsList) {
  smatch matchResults;
  if(regex_search(input_s, matchResults, depElems)) {
    string depElemsMatch = matchResults[0];
    matchParamList(depElemsMatch, depElemsList);
  }
  else throw MatchException("matchDepElems Exception\n");
}

bool OcrTaskPragmaParser::match() {
  Logger::Logger lg("OcrTaskPragmaParser::match()", Logger::INFO) ;
  Logger::info(lg) << "OcrTaskPragmaParser::match(" << m_pragmaStr << ")\n";
  try {
    smatch matchResults;
    // Check if the pragma matches the task annotation format
    // If not throw an exception and exit this function
    if(regex_match(m_pragmaStr, matchResults, taskBeginPragma)) {
      // Search on the matched result
      string taskbegin_s = matchResults[0];
      string taskName_s;
      // Extract the task's name
      matchTaskName(taskbegin_s, taskName_s);
      list<string> depEvtsList, depDbksList, depElemsList;
      // Extract the task's dependent Events
      matchDepEvts(taskbegin_s, depEvtsList);
      // Extract the task's dependent Datablocks
      matchDepDbks(taskbegin_s, depDbksList);
      // Extract the task's dependent elements (paramenters)
      matchDepElems(taskbegin_s, depElemsList);
      Logger::info(lg) << "taskName: " << taskName_s << endl;
      Logger::info(lg) << strlist2str(depEvtsList) << endl;
      Logger::info(lg) << strlist2str(depDbksList) << endl;
      Logger::info(lg) << strlist2str(depElemsList) << endl;
    }
    else if(regex_match(m_pragmaStr, matchResults, taskEndPragma)) {
    }
    else {
      ostringstream oss;
      oss << "ERROR: Exception parsing ocr task pragma \"" << m_pragmaStr << "\"" << endl;
      MatchException mexception(oss.str());
      throw mexception;
    }
  }
  catch(std::exception& e) {
    Logger::error(lg) << e.what();
    return false;
  }
}

string OcrTaskPragmaParser::strlist2str(list<string>& strList) const {
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

/*******************
 * OcrPragmaParser *
 *******************/
OcrPragmaParser::OcrPragmaParser() { }

void OcrPragmaParser::visit(SgNode* sgn) {
  if(SgPragmaDeclaration* sgpdecl = isSgPragmaDeclaration(sgn)) {
    SgPragma* sgp = sgpdecl->get_pragma();
    string pragmaStr = sgp->get_pragma();
    // Identify type of the pragma
    // Parse the pragma based on the type
    OcrPragmaType ptype = identifyPragmaType(pragmaStr);

    if(ptype == e_TaskBegin) {
      AstFromString::afs_skip_whitespace();
      OcrTaskPragmaParser taskPragmaParser(AstFromString::c_char);
      taskPragmaParser.match();
      astIterate(sgpdecl);
    }
    else if(ptype == e_DbkBegin) {
      astIterate(sgpdecl);
    }
    else { } // do nothing
  }
}

OcrPragmaParser::OcrPragmaType
OcrPragmaParser::identifyPragmaType(std::string pragmaStr) {
  AstFromString::c_char = pragmaStr.c_str();
  if(AstFromString::afs_match_substr("ocr task begin")) return e_TaskBegin;
  else if(AstFromString::afs_match_substr("ocr task end")) return e_TaskEnd;
  else if(AstFromString::afs_match_substr("ocr datablock begin")) return e_DbkBegin;
  else if(AstFromString::afs_match_substr("ocr datablock end")) return e_DbkEnd;
  else return e_NotOcr;
}

void OcrPragmaParser::astIterate(SgPragmaDeclaration* sgdecl) {
  Logger::Logger lg(Logger::DEBUG);
  // Find the parent of SgPragmaDeclration
  // Get my index and the total number of children
  // Iterate from my index + 1 until the end to find a matching task end
  SgNode* parent = sgdecl->get_parent();
  unsigned int cindex = parent->get_childIndex(sgdecl);
  unsigned int nchild = parent->get_numberOfTraversalSuccessors();
  Logger::debug(lg) << "-----------begin---------\n";
  for(int it = cindex+1; it < nchild; ++it) {
    SgNode* sgchild_ = parent->get_traversalSuccessorByIndex(it);
    if(isMatchingPragma(sgchild_)) break;
    Logger::debug(lg) << "child=" << it << endl;
    Logger::debug(lg) << AstDebug::astToString(sgchild_) << endl;
  }
  Logger::debug(lg) << "-------------end---------\n";
}

bool OcrPragmaParser::isMatchingPragma(SgNode* sgn) {
  if(SgPragmaDeclaration* sgpdecl = isSgPragmaDeclaration(sgn)) {
    string pstr = sgpdecl->get_pragma()->get_pragma();
    AstFromString::c_char = pstr.c_str();
    if(AstFromString::afs_match_substr("ocr task end") ||
       AstFromString::afs_match_substr("ocr datablock end")) return true;
  }
  return false;
}

void OcrPragmaParser::atTraversalEnd() {
}

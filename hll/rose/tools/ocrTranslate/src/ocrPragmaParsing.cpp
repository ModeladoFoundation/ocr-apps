/*
 * Author: Sriram Aananthakrishnan, 2016 *
 */
#include "sage3basic.h"
#include "ocrPragmaParsing.h"
#include "AstFromString.h"
#include "AstMatching.h"
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
OcrTaskPragmaParser::OcrTaskPragmaParser(const char* pragmaStr, OcrObjectManager& ocrObjectManager,
					 SgPragmaDeclaration* sgpdecl, unsigned int taskOrder)
  : m_pragmaStr(pragmaStr),
    m_ocrObjectManager(ocrObjectManager),
    m_sgpdecl(sgpdecl),
    m_taskOrder(taskOrder) {
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
  paramlist = as_xpr('(') >> *_s >> *by_ref(param) >> *by_ref(paramseq) >> *_s >> as_xpr(')');
  taskName = *_s >> icase("TASK") >> *_s >> by_ref(paramlist);
  depEvts = *_s >> icase("DEP_EVTS") >> *_s >> by_ref(paramlist);
  depDbks = *_s >> icase("DEP_DBKs") >> *_s >> by_ref(paramlist);
  depElems = *_s >> icase("DEP_ELEMs") >> *_s >> by_ref(paramlist);
  outEvts = *_s >> icase("OEVENT") >> *_s >> by_ref(paramlist);
  destroyDbks = *_s >> icase("DESTROY_DBKs") >> *_s >> by_ref(paramlist);
  destroyEvts = *_s >> icase("DESTROY_EVTs") >> *_s >> by_ref(paramlist);
  taskBeginPragma = taskName >> *_s >> depEvts >> *_s >> depDbks >> *_s >> depElems >> *_s;
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

bool OcrTaskPragmaParser::matchOutputEvt(string input, string& outputEvt) {
  sregex outEvtR;
  smatch matchresult;
  outEvtR = *_s >> icase("OEVENT") >> *_s >> '(' >> *_s >> (s1=identifier) >> *_s >> ')';
  if(regex_search(input, matchresult, outEvtR)) {
    outputEvt = matchresult[1];
  }
  else throw MatchException("Matching Output Event\n");
}

bool OcrTaskPragmaParser::matchOutputEvtList(string input, list<string>& evtsNameToSatisfy) {
  smatch matchResults;
  if(regex_search(input, matchResults, outEvts)) {
    string outEvtsMatch = matchResults[0];
    matchParamList(outEvtsMatch, evtsNameToSatisfy);
  }
  else throw MatchException("matchEvtsToSatisfy Exception\n");
}

bool OcrTaskPragmaParser::matchDestroyDbks(string input, list<string>& dbkNamesToDestroy) {
  smatch match_results;
  if(regex_search(input, match_results, destroyDbks)) {
    string destroyMatch = match_results[0];
    return matchParamList(destroyMatch, dbkNamesToDestroy);
  }
  return false;
}

bool OcrTaskPragmaParser::matchDestroyEvts(string input, list<string>& evtNamesToDestroy) {
  smatch match_results;
  if(regex_search(input, match_results, destroyEvts)) {
    string destroyMatch = match_results[0];
    return matchParamList(destroyMatch, evtNamesToDestroy);
  }
  return false;
}

bool OcrTaskPragmaParser::isMatchingPragma(SgNode* sgn) {
  if(SgPragmaDeclaration* sgpdecl = isSgPragmaDeclaration(sgn)) {
    string pstr = sgpdecl->get_pragma()->get_pragma();
    AstFromString::c_char = pstr.c_str();
    if(AstFromString::afs_match_substr("ocr task end")) return true;
  }
  return false;
}

// Beginning at root node m_sgpdecl which must be task begin
// Iterate over the parent's basic block until a matching task end is found
// We are assuming that task begin and task end pragma nodes are siblings
// Assumption is safe as every task begin must have a task end
// It is unlikely that the matching task end is a children of a sibling of task begin node
list<SgStatement*> OcrTaskPragmaParser::collectTaskStatements() {
  Logger::Logger lg("OcrTaskPragmaParser::collectTaskStatements");
  list<SgStatement*> taskStatementList;
  // Find the parent of SgPragmaDeclration
  // Get my index and the total number of children
  // Iterate from my index + 1 until the end to find a matching task end
  SgNode* parent = m_sgpdecl->get_parent();
  unsigned int cindex = parent->get_childIndex(m_sgpdecl);
  unsigned int nchild = parent->get_numberOfTraversalSuccessors();
  for(int it = cindex+1; it < nchild; ++it) {
    SgStatement* sgchild_ = isSgStatement(parent->get_traversalSuccessorByIndex(it));
    assert(sgchild_);
    taskStatementList.push_back(sgchild_);
    Logger::debug(lg) << AstDebug::astToString(sgchild_) << endl;
    if(isMatchingPragma(sgchild_)) break;
  }
  return taskStatementList;
}

SgVarRefExp* OcrTaskPragmaParser::identifier2sgn(std::string identifier_) {
  Logger::Logger lg("OcrTaskPragmaParser::identifier2sgn");
  SgVarRefExp* idsgn;
  AstFromString::c_char = identifier_.c_str();
  AstFromString::c_sgnode = m_sgpdecl->get_parent();
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

list<SgVarRefExp*> OcrTaskPragmaParser::identifiers2sgnlist(list<string> identifiersList) {
  Logger::Logger lg("OcrTaskPragmaParser::identifiers2sgnlist");
  list<SgVarRefExp*> idsgnList;
  list<string>::iterator i = identifiersList.begin();
  for( ; i != identifiersList.end(); ++i) {
    SgVarRefExp* idsgn = identifier2sgn(*i);
    Logger::debug(lg) << "idsgn=" << AstDebug::astToString(idsgn) << endl;
    assert(idsgn);
    idsgnList.push_back(idsgn);
  }
  return idsgnList;
}

bool OcrTaskPragmaParser::match() {
  Logger::Logger lg("OcrTaskPragmaParser::match()") ;
  try {
    smatch matchResults;
    // Check if the pragma matches the task annotation format
    // If not throw an exception and exit this function
    if(regex_match(m_pragmaStr, matchResults, taskBeginPragma)) {
      // Search on the matched result
      string taskbegin_s = matchResults[0];
      string taskName_s;
      // Extract the task's name in to taskName_s
      matchTaskName(taskbegin_s, taskName_s);
      list<string> depEvtsNameList, depDbksNameList, depElemsNameList;
      // Extract the task's dependent Events
      matchDepEvts(taskbegin_s, depEvtsNameList);
      list<OcrEvtContextPtr> depEvtsContextPtrList = m_ocrObjectManager.getOcrEvtContextList(depEvtsNameList);
      // Extract the task's dependent Datablocks
      matchDepDbks(taskbegin_s, depDbksNameList);
      list<OcrDbkContextPtr> depDbksContextPtrList = m_ocrObjectManager.getOcrDbkContextList(depDbksNameList);
      // Extract the task's dependent elements (paramenters)
      matchDepElems(taskbegin_s, depElemsNameList);
      list<SgVarRefExp*> depElemsSgnList = identifiers2sgnlist(depElemsNameList);
      // Find all statements between task begin and task end
      list<SgStatement*> taskStatementList = collectTaskStatements();
      // Last element of the list must be a SgPragmaDeclaration
      SgPragmaDeclaration* sgpdTaskEnd = isSgPragmaDeclaration(taskStatementList.back());
      taskStatementList.pop_back();
      // Safety checks
      if(!sgpdTaskEnd) {
	throw MatchException("Matching Task end annotation not found under same basic block\n");
      }
      // Collect all events that need to be satisfied by this task
      // Register the events with OcrObjectManager
      // list<string> evtsNameToSatisfy;
      // matchEvtsToSatisfy(sgpdTaskEnd->get_pragma()->get_pragma(),
      // 			 evtsNameToSatisfy);
      // list<OcrEvtContextPtr> outEvtsContextPtrList = m_ocrObjectManager.registerOcrEvts(evtsNameToSatisfy);
      string outputEvt;
      matchOutputEvt(sgpdTaskEnd->get_pragma()->get_pragma(), outputEvt);
      OcrEvtContextPtr outEvtContext = m_ocrObjectManager.registerOcrEvt(outputEvt);

      // Get List of OCR objects to destroy by this EDT
      list<string> dbkNamesToDestroy;
      matchDestroyDbks(sgpdTaskEnd->get_pragma()->get_pragma(), dbkNamesToDestroy);
      list<string> evtNamesToDestroy;
      matchDestroyEvts(sgpdTaskEnd->get_pragma()->get_pragma(), evtNamesToDestroy);

      // We have all the information we need for creating OcrEdtContext
      OcrEdtContextPtr edtcontext_sp = m_ocrObjectManager.registerOcrEdt(taskName_s, depEvtsContextPtrList,
									 depDbksContextPtrList, outEvtContext,
									 depElemsSgnList, taskStatementList,
									 dbkNamesToDestroy, evtNamesToDestroy,
									 m_sgpdecl, sgpdTaskEnd);
      // Register the in-order edt order for traversal
      m_ocrObjectManager.registerOcrEdtOrder(m_taskOrder, edtcontext_sp->get_name());
      Logger::debug(lg) << edtcontext_sp->str() << endl;
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

/********************
 * CollectAllocStmt *
 ********************/
CollectAllocStmt::CollectAllocStmt(SgNode* root) : m_root(root) { }

void CollectAllocStmt::visit(SgNode* sgn) {
  if(SgFunctionCallExp* fn = isSgFunctionCallExp(sgn)) {
    SgExpression* callexp = fn->get_function();
    // Note we can only detect mallocs if it is used directly
    // and not through any function pointers
    // Function pointer would require more analysis
    if(SgFunctionRefExp* fref = isSgFunctionRefExp(callexp)) {
      SgFunctionSymbol* fsymbol = fref->get_symbol();
      string fname = fsymbol->get_name().getString();
      if(fname.compare("malloc") == 0 ||
	 fname.compare("calloc") == 0) {
	SgStatement* allocStmt = SageInterface::getEnclosingStatement(fn);
	m_allocStmtList.push_back(allocStmt);
      }
    }
  }
}

void CollectAllocStmt::atTraversalEnd() { }

list<SgStatement*> CollectAllocStmt::getAllocStmt() const {
  return m_allocStmtList;
}

/**********************
 * OcrDbkPragmaParser *
 **********************/
OcrDbkPragmaParser::OcrDbkPragmaParser(SgPragmaDeclaration* sgpdecl, OcrObjectManager& ocrObjectManager)
  : m_sgpdecl(sgpdecl),
    m_ocrObjectManager(ocrObjectManager) {
  identifier = +(alpha|as_xpr('_')) >> *_w;
  param = *_s >> identifier;
  // paramseq is the tail seq of a parameter list
  sregex paramseq = *_s >> as_xpr(',') >> *_s >> param;
  paramlist = as_xpr('(') >> *_s >> param >> *by_ref(paramseq) >> *_s >> as_xpr(')');
  dbkNames = *_s >> icase("DATABLOCK") >> *_s >> by_ref(paramlist);
  dbkBegin = *_s >> as_xpr("ocr datablock begin") >> *dbkNames >> *_s;
}

bool OcrDbkPragmaParser::matchParams(string input, list<string>& paramList) {
  ostringstream oss;
  Logger::Logger lg("OcrDbkPragmaParser::matchParams");
  smatch matchResults;
  // Make sure we are working on a string that is paramlist
  if(!regex_match(input, matchResults, paramlist)) {
    throw MatchException("MatchException thrown by matchParamNames");
  }
  sregex_token_iterator cur(input.begin(), input.end(), identifier), end;
  for( ; cur != end; ++cur) {
    string identifier_s = *cur;
    paramList.push_back(identifier_s);
  }
  return true;
}

bool OcrDbkPragmaParser::matchParamList(string input, list<string>& paramList) {
  Logger::Logger lg("OcrDbkPragmaParser::matchParamList()");
  smatch matchresults;
  if(regex_search(input, matchresults, paramlist)) {
    string match  = matchresults[0];
    Logger::debug(lg) << "match=" << match << endl;
    matchParams(match, paramList);
  }
  else {
    throw MatchException("Failed to match paramlist in OcrDbkPragmaParser::matchParamList()");
  }
  return true;
}

bool OcrDbkPragmaParser::matchDbkNames(string input, list<string>& dbkNamesList) {
  Logger::Logger lg("OcrDbkPragmaParser::matchDbkNames");
  smatch matchresults;
  if(regex_search(input, matchresults, dbkNames)) {
    // On a successful match matchresults will be "DATABLOCK(...)"
    string match  = matchresults[0];
    matchParamList(match, dbkNamesList);
  }
  else {
    throw MatchException("OcrDbkPragmaParser::matchDbkNames()");
  }
  return true;
}

bool OcrDbkPragmaParser::isMatchingPragma(SgNode* sgn) {
  if(SgPragmaDeclaration* sgpdecl = isSgPragmaDeclaration(sgn)) {
    string pstr = sgpdecl->get_pragma()->get_pragma();
    AstFromString::c_char = pstr.c_str();
    if(AstFromString::afs_match_substr("ocr datablock end")) return true;
  }
  return false;
}

list<SgInitializedName*> OcrDbkPragmaParser::collectDbkVars() {
  list<SgInitializedName*> dbkVarList;
  SgNode* parent = m_sgpdecl->get_parent();
  unsigned int c_index = parent->get_childIndex(m_sgpdecl);
  unsigned int n_child = parent->get_numberOfTraversalSuccessors();
  for(int it = c_index+1; it < n_child; ++it) {
    // Collect all variable declarations between datablock begin and datablock end
    SgNode* sgchild_ = parent->get_traversalSuccessorByIndex(it);
    if(isMatchingPragma(sgchild_)) break;
    if(SgVariableDeclaration* vdecl = isSgVariableDeclaration(sgchild_)) {
      SgInitializedNamePtrList vlist = vdecl->get_variables();
      dbkVarList.insert(dbkVarList.end(), vlist.begin(), vlist.end());
    }
  }
  return dbkVarList;
}

list<SgStatement*> OcrDbkPragmaParser::collectAllocStmt(SgNode* root) {
  Logger::Logger lg("OcrDbkPragmaParser::collectAllocStmt");
  AllocStmtMap::iterator f = allocStmtMapCache.find(root);
  if(f != allocStmtMapCache.end()) {
    Logger::debug(lg) << "AllocStmtList: " << StrUtil::stmtlist2str(f->second) << endl;
    return f->second;
  }
  else {
    CollectAllocStmt cas(root);
    cas.traverse(root, preorder);
    list<SgStatement*> allocStmtList = cas.getAllocStmt();
    AllocStmtMapElem elem = std::make_pair(root, allocStmtList);
    allocStmtMapCache.insert(elem);
    Logger::debug(lg) << "AllocStmtList: " << StrUtil::stmtlist2str(allocStmtList) << endl;
    return allocStmtList;
  }
}

SgSymbol* OcrDbkPragmaParser::find_symbol(SgNode* sgn) {
  if(SgVarRefExp* vref = isSgVarRefExp(sgn)) {
    return vref->get_symbol();
  }
  else if(SgInitializedName* sgvdef = isSgInitializedName(sgn)) {
    return sgvdef->get_symbol_from_symbol_table();
  }
  else assert(false);
}

list<SgStatement*> OcrDbkPragmaParser::varFilterAllocStmt(list<SgStatement*>& allocStmtList, SgInitializedName* sgn) {
  list<SgStatement*> varAllocStmts;
  SgSymbol* vsymbol = sgn->get_symbol_from_symbol_table();
  // malloc can be in an SgAssignOp
  string query = "SgAssignOp($VAR=SgVarRefExp,_)";
  // malloc can be in an SgInitializer
  query += "|($VAR=SgInitializedName(SgAssignInitializer(_)))";
  list<SgStatement*>::iterator s = allocStmtList.begin();
  AstMatching matcher;
  for( ; s != allocStmtList.end(); ++s) {
    MatchResult match_results = matcher.performMatching(query, *s);
    MatchResult::iterator m = match_results.begin();
    for( ; m != match_results.end(); ++m) {
      SgSymbol* msymbol = find_symbol((*m)["$VAR"]);
      if(msymbol == vsymbol)
	varAllocStmts.push_back(*s);
    }
  }
  return varAllocStmts;
}

list<SgStatement*> OcrDbkPragmaParser::getAllocStmt(SgInitializedName* sgn) {
  SgScopeStatement* scope = SageInterface::getEnclosingScope(sgn);
  list<SgStatement*> allocStmtList = collectAllocStmt(scope);
  return varFilterAllocStmt(allocStmtList, sgn);
}

bool OcrDbkPragmaParser::match() {
  Logger::Logger lg("OcrDbkPragmaParser::match()");
  string pstr = m_sgpdecl->get_pragma()->get_pragma();
  try {
    smatch matchResults;
    if(regex_match(pstr, matchResults, dbkBegin)) {
      string mresults = matchResults[0];
      // Now search in this string for extracting the datablock names
      // Get the OCR names for the datablocks by parsing the DATABLOCK(...)
      list<string> dbkNameList;
      list<SgInitializedName*> dbkVarList;
      if(matchDbkNames(mresults, dbkNameList)) {
	dbkVarList = collectDbkVars();
	assert(dbkNameList.size() == dbkVarList.size());
	list<SgInitializedName*>::iterator vIt = dbkVarList.begin();
	list<string>::iterator nIt = dbkNameList.begin();
	for( ; vIt != dbkVarList.end() && nIt != dbkNameList.end();
	     ++vIt, ++nIt) {
	  SgType* vtype = (*vIt)->get_type();
	  switch(vtype->variantT()) {
	  case V_SgPointerType: {
	    list<SgStatement*> varAllocStmts = getAllocStmt(*vIt);
	    OcrDbkContextPtr dbkcontext_sp = m_ocrObjectManager.registerOcrDbk(*nIt, *vIt, varAllocStmts);
	    Logger::debug(lg) << dbkcontext_sp->str() << endl;
	    break;
	  }
	  case V_SgArrayType: {
	    throw MatchException("Unhandled Array Type in OcrDbkPragmaParser::match()\n");
	    break;
	  }
	  case V_SgNamedType: {
	    throw MatchException("Unhadled Named Type  in OcrDbkPragmaParser::match()\n");
	    break;
	  }
	  default:
	    throw MatchException("Unhandled Type in OcrDbkPragmaParser::match()\n");
	  }
	}
      }
      else {
	// What should we do when we have no names specified for datablocks
	// We could use the variable names
	// Using variable names could introduce ambiguity in the grammar
	throw MatchException("Dbk Match Failed: No names specified for datablocks");
      }
    }
    else {
      throw MatchException("Dbk Match Failed");
    }
  }
  catch(std::exception& e) {
    Logger::error(lg) << e.what();
    return false;
  }
}

/***************************
 * OcrShutdownPragmaParser *
 ***************************/
OcrShutdownPragmaParser::OcrShutdownPragmaParser(SgPragmaDeclaration* sgpdecl, std::string input, OcrObjectManager& ocrObjectManager)
  : m_spgdecl(sgpdecl), m_input(input), m_ocrObjectManager(ocrObjectManager) {
  sr_identifier = +(alpha|as_xpr('_')) >> *_w;
  sr_param = *_s >> *by_ref(sr_identifier);
  // paramseq is the tail seq of a parameter list
  sregex sr_paramseq = *_s >> as_xpr(',') >> *_s >> sr_param;
  sr_paramlist = as_xpr('(') >> *_s >> sr_param >> *by_ref(sr_paramseq) >> *_s >> as_xpr(')');
  sr_depevts = *_s >> icase("DEP_EVTS") >> *_s >> by_ref(sr_paramlist);
}

// Expected string input : (param1, param2,..,paramn)
// For extracting the parameters we just iterate using the identifier sregex token
bool OcrShutdownPragmaParser::matchParams(string input, list<string>& paramList) {
  smatch match_results;
  sregex_token_iterator cur(input.begin(), input.end(), sr_identifier), end;
  for( ; cur != end; ++cur) {
    string param = *cur;
    if(param.compare("NONE") != 0 &&
       param.compare("none") != 0) {
      paramList.push_back(param);
    }
  }
  return true;
}

bool OcrShutdownPragmaParser::match() {
  Logger::Logger lg("OcrShutdownPragmaParser::match()");
  string input = m_input;
  list<string> depEvtNames;
  try {
    if(regex_match(input, sr_depevts)) {
      smatch match_results;
      if(regex_search(input, match_results, sr_paramlist)) {
	string what = match_results[0];
	matchParams(what, depEvtNames);
      }
      else throw MatchException("Expecting a parameter list of dependent events");
    }
    else throw MatchException("Shutdown pragma parsing failed:" + input);
    // We have successfully matched the pragma and the list of events are in depEvtNames
    // Get context for each OcrEvt
    list<string>::iterator en = depEvtNames.begin();
    list<OcrEvtContextPtr> depEvts;
    for( ; en != depEvtNames.end(); ++en) {
      OcrEvtContextPtr evt = m_ocrObjectManager.registerOcrEvt(*en);
      depEvts.push_back(evt);
    }
    m_ocrObjectManager.registerOcrShutdownEdt(m_spgdecl, depEvts);
  }
  catch(std::exception& e) {
    Logger::error(lg) << e.what();
    return false;
  }
}

/*******************
 * OcrPragmaParser *
 *******************/
OcrPragmaParser::OcrPragmaParser() : m_taskOrderCounter(0) { }

const OcrObjectManager& OcrPragmaParser::getOcrObjectManager() const {
  return m_ocrObjectManager;
}

void OcrPragmaParser::visit(SgNode* sgn) {
  if(SgPragmaDeclaration* sgpdecl = isSgPragmaDeclaration(sgn)) {
    SgPragma* sgp = sgpdecl->get_pragma();
    string pragmaStr = sgp->get_pragma();
    // Identify type of the pragma
    // Parse the pragma based on the type
    OcrPragmaType ptype = identifyPragmaType(pragmaStr);

    if(ptype == e_TaskBegin) {
      AstFromString::afs_skip_whitespace();
      // Increase the traversal order counter
      m_taskOrderCounter = m_taskOrderCounter+1;
      OcrTaskPragmaParser taskPragmaParser(AstFromString::c_char, m_ocrObjectManager, sgpdecl, m_taskOrderCounter);
      taskPragmaParser.match();
    }
    else if(ptype == e_DbkBegin) {
      AstFromString::afs_skip_whitespace();
      OcrDbkPragmaParser dbkPragmaParser(sgpdecl, m_ocrObjectManager);
      dbkPragmaParser.match();
    }
    else if(ptype == e_shutdown) {
      AstFromString::afs_skip_whitespace();
      OcrShutdownPragmaParser shutdownPragmaParser(sgpdecl, AstFromString::c_char, m_ocrObjectManager);
      shutdownPragmaParser.match();
    }
    else { } // do nothing
  }
}

OcrPragmaParser::OcrPragmaType
OcrPragmaParser::identifyPragmaType(std::string pragmaStr) {
  AstFromString::c_char = pragmaStr.c_str();
  if(AstFromString::afs_match_substr("ocr task begin")) return e_TaskBegin;
  else if(AstFromString::afs_match_substr("ocr datablock begin")) return e_DbkBegin;
  else if(AstFromString::afs_match_substr("ocr shutdown")) return e_shutdown;
  else return e_NotOcr;
}

void OcrPragmaParser::astIterate(SgPragmaDeclaration* sgdecl) {
  Logger::Logger lg("OcrPragmaParser::astIterate", Logger::DEBUG);
  // Find the parent of SgPragmaDeclration
  // Get my index and the total number of children
  // Iterate from my index + 1 until the end to find a matching task end
  SgNode* parent = sgdecl->get_parent();
  unsigned int cindex = parent->get_childIndex(sgdecl);
  unsigned int nchild = parent->get_numberOfTraversalSuccessors();
  for(int it = cindex+1; it < nchild; ++it) {
    SgNode* sgchild_ = parent->get_traversalSuccessorByIndex(it);
    if(isMatchingPragma(sgchild_)) break;
    Logger::debug(lg) << AstDebug::astToString(sgchild_) << endl;
  }
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

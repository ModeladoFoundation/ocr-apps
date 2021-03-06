/*
 * Author: Sriram Aananthakrishnan, 2016 *
 */
#include "sage3basic.h"
#include "ocrPragmaParsing.h"
#include "AstFromString.h"
#include "AstMatching.h"
#include "RoseAst.h"
#include <string>
#include <numeric>
#include "logger.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

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

/*****************
 * OcrPragmaType *
 *****************/
OcrPragmaType::OcrPragmaType_t OcrPragmaType::identifyPragmaType(SgPragmaDeclaration* sgpdecl) {
  string pragmaStr = sgpdecl->get_pragma()->get_pragma();
  AstFromString::c_char = pragmaStr.c_str();
  if(AstFromString::afs_match_substr("ocr task")) return e_TaskEdt;
  else if(AstFromString::afs_match_substr("ocr datablock")) return e_Dbk;
  else if(AstFromString::afs_match_substr("ocr shutdown")) return e_ShutdownEdt;
  else if(AstFromString::afs_match_substr("ocr loop")) return e_LoopIterEdt;
  else if(AstFromString::afs_match_substr("ocr spmd region")) return e_SpmdRegionEdt;
  else if(AstFromString::afs_match_substr("ocr spmd send")) return e_SpmdSend;
  else if(AstFromString::afs_match_substr("ocr spmd recv")) return e_SpmdRecv;
  else if(AstFromString::afs_match_substr("ocr spmd reduce")) return e_SpmdReduce;
  else if(AstFromString::afs_match_substr("ocr spmd finalize")) return e_SpmdFinalizeEdt;
  else return e_NotOcr;
}



/******************************
 * OcrTaskBasicBlockTraversal *
 ******************************/
OcrTaskBasicBlockTraversal::OcrTaskBasicBlockTraversal(SgBasicBlock* basicblock)
  : m_basicblock(basicblock),
    m_finishEdt(false) { }

void OcrTaskBasicBlockTraversal::insertChildren(SgNode* sgn) {
  vector<SgNode*> successors = sgn->get_traversalSuccessorContainer();
  vector<SgNode*>::iterator n = successors.begin();
  for( ; n != successors.end(); ++n) {
    // If the SgNode is not null push it to the queue
    // Some AST can have null children - Example : SgAssignInitializer
    if(*n) {
      m_queue.push(*n);
    }
  }
}

void OcrTaskBasicBlockTraversal::bfs_search() {
  while(!m_queue.empty()) {
    SgNode* sgn = m_queue.front();
    // Remove this element from the queue
    m_queue.pop();
    // First time visit
    if(m_visited.find(sgn) == m_visited.end()) {
      // Mark the node as visited
      m_visited.insert(sgn);
      if(SgPragmaDeclaration* sgpdecl = isSgPragmaDeclaration(sgn)) {
	OcrPragmaType pragmaType;
	OcrPragmaType::OcrPragmaType_t ptype = pragmaType.identifyPragmaType(sgpdecl);
	if(ptype == OcrPragmaType::e_TaskEdt) {
	  // Get the next sibling
	  SgStatement* nextStmt = SageInterface::getNextStatement(sgpdecl);
	  assert(isSgBasicBlock(nextStmt) &&
		 m_queue.front() == nextStmt);
	  // Remove the basic block of the task annotation from traversal
	  m_queue.pop();
	  m_finishEdt = true;
	}
	// This is the for loop annotation
	else if(ptype == OcrPragmaType::e_LoopIterEdt) {
	  // Don't need to visit the children of this node
	  SgStatement* nextStmt = SageInterface::getNextStatement(sgpdecl);
	  assert(isSgForStatement(nextStmt) &&
		 nextStmt == m_queue.front());
	  // Don't traverse the SgForStatement
	  m_queue.pop();
	  m_finishEdt = true;
	}
	else if(ptype == OcrPragmaType::e_ShutdownEdt) {
	  // Don't need to visit the children of this node
	}
	else if(ptype == OcrPragmaType::e_Dbk) {
	  // Don't need to visit the child node of this pragma declaration
	}
	else {
	  // If this is a pragma that is not of ocr type
	  // continue the search
	  insertChildren(sgpdecl);
	}
      }
      else if(SgFunctionCallExp* sgfcallexp = isSgFunctionCallExp(sgn)) {
	// Currently nothing to do here
	// We need to detect if the call is going to create any EDTs
	// For now we will assume it is going to and mark the outerEdt as finish EDt
	m_finishEdt = true;
	// No need to visit the children
      }
      // If this is any other SgNode continue the search
      else {
	insertChildren(sgn);
      }
    }
    else {
      // No cycle in the AST
      // We should not be revisiting a node
      assert(false);
    }
  }
}

void OcrTaskBasicBlockTraversal::traverse() {
  // The search can be performed only on the basic block
  // bfs on anything that is not a basic block will result in incorrect results
  assert(m_basicblock);
  m_queue.push(m_basicblock);
  // Carry out the search on the basic block
  bfs_search();
}

list<OcrDbkContextPtr> OcrTaskBasicBlockTraversal::getDbksToCreate() const {
  return m_dbksToCreate;
}

bool OcrTaskBasicBlockTraversal::isFinishEdt() const {
  return m_finishEdt;
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
  taskRegEx = taskName >> depDbks >> depEvts >> depElems >> outEvts >> *_s;
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

// The pragma language requires that the task annotation be
// enclosed within a scope { }
// The statements of a task are wrapped into a basic block
// that immediately follows the task annotation
SgBasicBlock* OcrTaskPragmaParser::getTaskBasicBlock() {
  SgStatement* nextStmt = SageInterface::getNextStatement(m_sgpdecl);
  SgBasicBlock* basicblock = isSgBasicBlock(nextStmt);
  if(!basicblock) {
    throw MatchException("Parsing Failed: Expecting a basic block to follow the task annotation\n");
  }
  return basicblock;
}

bool OcrTaskPragmaParser::match() {
  Logger::Logger lg("OcrTaskPragmaParser::match()");
  try {
    smatch matchResults;
    // Check if the pragma matches the task annotation format
    // If not throw an exception and exit this function
    if(regex_match(m_pragmaStr, matchResults, taskRegEx)) {
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
      SgScopeStatement* scope = SageInterface::getEnclosingScope(m_sgpdecl);
      list<OcrDbkContextPtr> depDbksContextPtrList = m_ocrObjectManager.getOcrDbkContextList(depDbksNameList, scope);
      // Extract the task's dependent elements (paramenters)
      matchDepElems(taskbegin_s, depElemsNameList);
      list<SgVarRefExp*> depElemsSgnList = SgNodeUtil::identifiers2sgnlist(depElemsNameList, m_sgpdecl);
      string outputEvt;
      matchOutputEvt(taskbegin_s, outputEvt);
      OcrEvtContextPtr outEvtContext = m_ocrObjectManager.registerOcrEvt(outputEvt);
      Logger::debug(lg) << "outEvtContext=" << outEvtContext->str() << endl;

      // Collect the statements of the task annotation
      SgBasicBlock* basicblock = getTaskBasicBlock();
      OcrTaskBasicBlockTraversal taskBasicBlockTraversal(basicblock);
      taskBasicBlockTraversal.traverse();

      bool finishEdt = taskBasicBlockTraversal.isFinishEdt();
      // We have all the information we need for creating OcrEdtContext
      SgSourceFile* sourceFile = SageInterface::getEnclosingSourceFile(m_sgpdecl);
      OcrTaskContextPtr taskcontext_sp = m_ocrObjectManager.registerOcrEdt(taskName_s, m_taskOrder, m_sgpdecl, depDbksContextPtrList,
									   depEvtsContextPtrList, depElemsSgnList,
									   outEvtContext, basicblock, finishEdt);
      OcrEdtContextPtr edtcontext = boost::dynamic_pointer_cast<OcrEdtContext>(taskcontext_sp);
      Logger::debug(lg) << taskcontext_sp->str() << endl;
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
    assert(false);
    return false;
  }
  return true;
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
OcrDbkPragmaParser::OcrDbkPragmaParser(SgPragmaDeclaration* sgpdecl, OcrObjectManager& objectManager)
  : m_sgpdecl(sgpdecl),
    m_ocrObjectManager(objectManager) {
  identifier = +(alpha|as_xpr('_')) >> *_w;
  param = *_s >> identifier;
  // paramseq is the tail seq of a parameter list
  sregex paramseq = *_s >> as_xpr(',') >> *_s >> param;
  paramlist = as_xpr('(') >> *_s >> param >> *by_ref(paramseq) >> *_s >> as_xpr(')');
  dbkNames = *_s >> icase("DATABLOCK") >> *_s >> by_ref(paramlist);
  dbkBegin = *_s >> as_xpr("ocr datablock") >> *dbkNames >> *_s;
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

list<SgInitializedName*> OcrDbkPragmaParser::collectDbkVars(unsigned int ndbks) {
  list<SgInitializedName*> dbkVarList;
  SgNode* parent = m_sgpdecl->get_parent();
  unsigned int c_index = parent->get_childIndex(m_sgpdecl);
  unsigned int n_child = 0;
  for(int it = c_index+1; n_child < ndbks; ++it, ++n_child) {
    // Collect all variable declarations between datablock begin and datablock end
    SgNode* sgchild_ = parent->get_traversalSuccessorByIndex(it);
    SgVariableDeclaration* vdecl = isSgVariableDeclaration(sgchild_);
    assert(vdecl);
    SgInitializedNamePtrList vlist = vdecl->get_variables();
    dbkVarList.insert(dbkVarList.end(), vlist.begin(), vlist.end());
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
	unsigned int ndbks = dbkNameList.size();
	dbkVarList = collectDbkVars(ndbks);
	assert(dbkNameList.size() == dbkVarList.size());
	list<SgInitializedName*>::iterator vIt = dbkVarList.begin();
	list<string>::iterator nIt = dbkNameList.begin();
	for( ; vIt != dbkVarList.end() && nIt != dbkNameList.end();
	     ++vIt, ++nIt) {
	  SgType* vtype = (*vIt)->get_type();
	  switch(vtype->variantT()) {
	  case V_SgPointerType: {
	    list<SgStatement*> varAllocStmts = getAllocStmt(*vIt);
	    OcrDbkContextPtr dbkcontext_sp = boost::make_shared<OcrMemDbkContext>(*nIt, *vIt, varAllocStmts, m_sgpdecl);
	    Logger::debug(lg) << dbkcontext_sp->str() << endl;
	    m_dbkContextList.push_back(dbkcontext_sp);
	    break;
	  }
	  case V_SgArrayType: {
	    OcrDbkContextPtr arrDbkContext = boost::make_shared<OcrArrDbkContext>(*nIt, *vIt, m_sgpdecl);
	    Logger::debug(lg) << arrDbkContext->str() << endl;
	    m_dbkContextList.push_back(arrDbkContext);
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
  return true;
}

list<OcrDbkContextPtr> OcrDbkPragmaParser::getDbkContextList() const {
  return m_dbkContextList;
}

/***************************
 * OcrShutdownPragmaParser *
 ***************************/
OcrShutdownPragmaParser::OcrShutdownPragmaParser(SgPragmaDeclaration* sgpdecl, std::string input,
						 unsigned int traversalOrder, OcrObjectManager& ocrObjectManager)
  : m_sgpdecl(sgpdecl), m_input(input), m_traversalOrder(traversalOrder), m_ocrObjectManager(ocrObjectManager) {
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
    ostringstream oss;
    // A unique name to this annotation
    oss << "_" << m_traversalOrder << "ShutdownEdt";
    string shutdownEdtName = oss.str();
    m_ocrObjectManager.registerOcrShutdownEdt(shutdownEdtName, m_traversalOrder, m_sgpdecl, depEvts);
  }
  catch(std::exception& e) {
    Logger::error(lg) << e.what();
    return false;
  }
  return true;
}

/***************************
 * OcrLoopIterPragmaParser *
 ***************************/
OcrLoopIterPragmaParser::OcrLoopIterPragmaParser(SgPragmaDeclaration* sgpdecl, OcrObjectManager& objectManager,
						 unsigned int taskOrder)
  : m_sgpdecl(sgpdecl),
    m_ocrObjectManager(objectManager),
    m_taskOrder(taskOrder) {
  sr_identifier = +(alpha|as_xpr('_')) >> *_w;
  sr_param = *_s >> *by_ref(sr_identifier);
  // paramseq is the tail seq of a parameter list
  sregex sr_paramseq = *_s >> as_xpr(',') >> *_s >> sr_param;
  sr_paramlist = as_xpr('(') >> *_s >> *sr_param >> *by_ref(sr_paramseq) >> *_s >> as_xpr(')');
  sr_taskname = *_s >> icase("TASK") >> *_s >> as_xpr('(') >> *_s >> by_ref(sr_identifier) >> *_s >> as_xpr(')') >> *_s;
  sr_depdbks = *_s >> icase("DEP_DBKS") >> *_s >> by_ref(sr_paramlist);
  sr_depevts = *_s >> icase("DEP_EVTS") >> *_s >> by_ref(sr_paramlist);
  sr_depelems = *_s >> icase("DEP_ELEMS") >> *_s >> by_ref(sr_paramlist);
  sr_oevent = *_s >> icase("OEVENT") >> *_s >> as_xpr('(') >> *_s >> by_ref(sr_param) >> *_s >> as_xpr(')') >> *_s;
  sr_loop = *_s >> as_xpr("ocr loop") >> *_s >> sr_taskname >> sr_depdbks >> sr_depevts >> sr_depelems >> sr_oevent;
}

list<string> OcrLoopIterPragmaParser::matchParamNames(string input) {
  list<string> paramList;
  if(!regex_match(input, sr_paramlist)) {
    throw MatchException("MatchException thrown by matchParamNames");
  }
  sregex_token_iterator cur(input.begin(), input.end(), sr_identifier), end;
  for( ; cur != end; ++cur) {
    string identifier = *cur;
    if(identifier.compare("NONE") == 0 || identifier.compare("none") == 0) {
      continue;
    }
    paramList.push_back(identifier);
  }
  return paramList;
}

string OcrLoopIterPragmaParser::matchTaskName(string input) {
  sregex taskNameOnly = *_s >> icase("TASK") >> *_s >> '(' >> *_s >> (s1=sr_identifier) >> *_s >> ')';
  smatch matchResults;
  if(regex_search(input, matchResults, taskNameOnly)) {
    return matchResults[1];
  }
  else {
    throw MatchException("matchTaskName");
    return "";
  }
}

list<string> OcrLoopIterPragmaParser::matchDepDbks(string input) {
  sregex sr_dbknames = icase("DEP_DBKS") >> *_s >> (s1=sr_paramlist);
  smatch matchResults;
  list<string> dbkNames;
  if(regex_search(input, matchResults, sr_dbknames)) {
    string paramlist = matchResults[1];
    dbkNames = matchParamNames(paramlist);
  }
  else {
    throw MatchException("Matching failed in DEP_DBKs\n");
  }
  return dbkNames;
}

list<string> OcrLoopIterPragmaParser::matchDepEvts(string input) {
  sregex sr_dbknames = icase("DEP_EVTs") >> *_s >> (s1=sr_paramlist);
  smatch matchResults;
  list<string> dbkNames;
  if(regex_search(input, matchResults, sr_dbknames)) {
    string paramlist = matchResults[1];
    dbkNames = matchParamNames(paramlist);
  }
  else {
    throw MatchException("Matching failed in DEP_EVTs\n");
  }
  return dbkNames;
}

list<string> OcrLoopIterPragmaParser::matchDepElems(string input) {
  sregex sr_delems = icase("DEP_ELEMS") >> *_s >> (s1=sr_paramlist);
  list<string> depElemNames;
  smatch matchResults;
  if(regex_search(input, matchResults, sr_delems)) {
    string paramlist = matchResults[1];
    depElemNames = matchParamNames(paramlist);
  }
  else {
    throw MatchException("Matching failed in DEP_ELEMs\n");
  }
  return depElemNames;
}

string OcrLoopIterPragmaParser::matchOutEvt(string input) {
  sregex outEvtNameOnly = *_s >> icase("OEVENT") >> *_s >> '(' >> *_s >> (s1=sr_identifier) >> *_s >> ')';
  smatch matchResults;
  if(regex_search(input, matchResults, outEvtNameOnly)) {
    return matchResults[1];
  }
  else {
    throw MatchException("Matching Failed in OEVENT\n");
    return "";
  }
}

SgStatement* OcrLoopIterPragmaParser::getLoopStmt(SgPragmaDeclaration* sgpdecl) {
  // Next statement following the annotation must be a loop statement
  SgStatement* nextStmt = SageInterface::getNextStatement(sgpdecl);
  if(isSgForStatement(nextStmt)) {
    return nextStmt;
  }
  else if(isSgWhileStmt(nextStmt)) {
    throw MatchException("Unhandled Loop Stmt in OcrLoopIterPragmaParser\n");
  }
  else if(isSgDoWhileStmt(nextStmt)) {
    throw MatchException("Unhandled Loop Stmt in OcrLoopIterPragmaParser\n");
  }
  else {
    throw MatchException("Unhandled Stmt in OcrLoopIterPragmaParser\n");
  }
}

bool OcrLoopIterPragmaParser::match() {
  Logger::Logger lg("OcrLoopIterPragmaParser::match()");
  string input = m_sgpdecl->get_pragma()->get_pragma();
  Logger::debug(lg) << "input: " << input << endl;
  try {
    if(regex_match(input, sr_loop)) {
      string taskname = matchTaskName(input);
      list<string> depDbkNames = matchDepDbks(input);
      list<string> depEvtNames = matchDepEvts(input);
      list<string> depElemNames = matchDepElems(input);
      string outEvtName = matchOutEvt(input);
      SgScopeStatement* scope = SageInterface::getEnclosingScope(m_sgpdecl);
      list<OcrDbkContextPtr> depDbks = m_ocrObjectManager.getOcrDbkContextList(depDbkNames, scope);
      list<OcrEvtContextPtr> depEvts = m_ocrObjectManager.getOcrEvtContextList(depEvtNames);
      list<SgVarRefExp*> depElems = SgNodeUtil::identifiers2sgnlist(depElemNames, m_sgpdecl);
      // register the output event
      OcrEvtContextPtr outEvtContext = m_ocrObjectManager.registerOcrEvt(outEvtName);
      SgStatement* loopStmt = getLoopStmt(m_sgpdecl);
      // We have all the information we need for creating OcrLoopIterEdtContext
      OcrTaskContextPtr taskcontext_sp = m_ocrObjectManager.registerOcrLoopIterEdt(taskname, m_taskOrder, m_sgpdecl, depDbks,
										   depEvts, depElems,
										   outEvtContext, loopStmt);
    }
    else {
      throw MatchException("Error Matching LoopIterPragmaParser\n");
    }
  }
  catch(std::exception& e) {
    Logger::error(lg) << e.what();
    return false;
  }
  return true;
}

/*****************************
 * OcrSpmdRegionPragmaParser *
 *****************************/
OcrSpmdRegionPragmaParser::OcrSpmdRegionPragmaParser(SgPragmaDeclaration* sgpdecl, string input,
						     unsigned int traversalOrder, OcrObjectManager& ocrObjectManager)
  : m_sgpdecl(sgpdecl),
    m_traversalOrder(traversalOrder),
    m_ocrObjectManager(ocrObjectManager) {
  sr_identifier = +(alpha|as_xpr('_')) >> *_w;
  sr_param = *_s >> *by_ref(sr_identifier);
  // paramseq is the tail seq of a parameter list
  sregex sr_paramseq = *_s >> as_xpr(',') >> *_s >> sr_param;
  sr_paramlist = as_xpr('(') >> *_s >> *sr_param >> *by_ref(sr_paramseq) >> *_s >> as_xpr(')');
}

list<string> OcrSpmdRegionPragmaParser::matchParamNames(string input) {
  list<string> paramList;
  if(!regex_match(input, sr_paramlist)) {
    throw MatchException("MatchException thrown by matchParamNames");
  }
  sregex_token_iterator cur(input.begin(), input.end(), sr_identifier), end;
  for( ; cur != end; ++cur) {
    string identifier = *cur;
    if(identifier.compare("NONE") == 0 || identifier.compare("none") == 0) {
      continue;
    }
    paramList.push_back(identifier);
  }
  return paramList;
}

SgExpression* OcrSpmdRegionPragmaParser::matchNTasks(string input) {
  sregex sr_digits = +digit;
  sregex sr_ntasks = *_s >> icase("NTASKS") >> *_s >> '(' >> (s1=(sr_digits|sr_identifier)) >> ')';
  smatch match_results;
  if(regex_search(input, match_results, sr_ntasks)) {
    string ntasksStr = match_results[1];
    AstFromString::c_char = ntasksStr.c_str();
    AstFromString::c_sgnode = m_sgpdecl;
    if(AstFromString::afs_match_primary_expression()) {
      SgNode* expr = AstFromString::c_parsed_node;
      switch(expr->variantT()) {
      case V_SgVarRefExp:
      case V_SgIntVal: {
	return isSgExpression(expr);
      }
      default:
	// Any other Expr type is unexpected in pragma for ntasks
	throw MatchException("Unexpected Expression in TASKS\n");
      }
    }
    else {
      throw MatchException("AstFromString Matching failed in NTASKS\n");
    }
  }
  else {
      throw MatchException("Failed matching pragma for NTASKS\n");
  }
  return NULL;
}

list<string> OcrSpmdRegionPragmaParser::matchDepDbks(string input) {
  sregex sr_dbknames = icase("DEP_DBKS") >> *_s >> (s1=sr_paramlist);
  smatch matchResults;
  list<string> dbkNames;
  if(regex_search(input, matchResults, sr_dbknames)) {
    string paramlist = matchResults[1];
    dbkNames = matchParamNames(paramlist);
  }
  else {
    throw MatchException("Matching failed in DEP_DBKs\n");
  }
  return dbkNames;
}

list<string> OcrSpmdRegionPragmaParser::matchDepEvts(string input) {
  sregex sr_dbknames = icase("DEP_EVTs") >> *_s >> (s1=sr_paramlist);
  smatch matchResults;
  list<string> dbkNames;
  if(regex_search(input, matchResults, sr_dbknames)) {
    string paramlist = matchResults[1];
    dbkNames = matchParamNames(paramlist);
  }
  else {
    throw MatchException("Matching failed in DEP_EVTs\n");
  }
  return dbkNames;
}

list<string> OcrSpmdRegionPragmaParser::matchDepElems(string input) {
  sregex sr_delems = icase("DEP_ELEMS") >> *_s >> (s1=sr_paramlist);
  list<string> depElemNames;
  smatch matchResults;
  if(regex_search(input, matchResults, sr_delems)) {
    string paramlist = matchResults[1];
    depElemNames = matchParamNames(paramlist);
  }
  else {
    throw MatchException("Matching failed in DEP_ELEMs\n");
  }
  return depElemNames;
}

string OcrSpmdRegionPragmaParser::matchOutEvt(string input) {
  sregex outEvtNameOnly = *_s >> icase("OEVENT") >> *_s >> '(' >> *_s >> (s1=sr_identifier) >> *_s >> ')';
  smatch matchResults;
  if(regex_search(input, matchResults, outEvtNameOnly)) {
    return matchResults[1];
  }
  else {
    throw MatchException("Matching Failed in OEVENT\n");
    return "";
  }
}

SgBasicBlock* OcrSpmdRegionPragmaParser::getSpmdRegionBasicBlock() {
  SgStatement* nextStmt = SageInterface::getNextStatement(m_sgpdecl);
  SgBasicBlock* basicblock = isSgBasicBlock(nextStmt);
  if(!basicblock) {
    throw MatchException("BasicBlock for SPMD region NOT found\n");
  }
  return basicblock;
}

bool OcrSpmdRegionPragmaParser::match() {
  Logger::Logger lg("OcrSpmdRegionPragmaParser::match()", Logger::DEBUG);
  boost::xpressive::sregex sr_spmdregion = *_s >> as_xpr("ocr spmd region")
					       >> *_s >> icase("NTASKS") >> *_s >> '(' >> (sr_identifier |+digit) >> ')'
					       >> *_s >> icase("DEP_DBKS") >> *_s >> (sr_paramlist)
					       >> *_s >> icase("DEP_EVTS") >> *_s >> (sr_paramlist)
					       >> *_s >> icase("DEP_ELEMS") >> *_s >> (sr_paramlist)
					       >> *_s >> icase("OEVENT") >> *_s >> '(' >> *_s >> (sr_identifier) >> *_s >> ')'
					       >> *_s;
  string input = m_sgpdecl->get_pragma()->get_pragma();
  try {
    if(regex_match(input, sr_spmdregion)) {
      SgExpression* ntaskExpr = matchNTasks(input);
      list<string> depDbkNames = matchDepDbks(input);
      list<string> depEvtNames = matchDepEvts(input);
      list<string> depElemNames = matchDepElems(input);
      string outEvtName = matchOutEvt(input);
      SgScopeStatement* scope = SageInterface::getEnclosingScope(m_sgpdecl);
      list<OcrDbkContextPtr> depDbks = m_ocrObjectManager.getOcrDbkContextList(depDbkNames, scope);
      list<OcrEvtContextPtr> depEvts = m_ocrObjectManager.getOcrEvtContextList(depEvtNames);
      list<SgVarRefExp*> depElems = SgNodeUtil::identifiers2sgnlist(depElemNames, m_sgpdecl);
      // Register the output event
      OcrEvtContextPtr outEvtContext = m_ocrObjectManager.registerOcrEvt(outEvtName);
      string spmdEdtName = SageInterface::generateUniqueVariableName(scope, "spmdEdt");
      SgBasicBlock* basicblock = getSpmdRegionBasicBlock();
      OcrTaskContextPtr taskContext = m_ocrObjectManager.registerOcrSpmdRegionEdt(spmdEdtName, m_traversalOrder, m_sgpdecl,
										  depDbks, depEvts, depElems, outEvtContext, basicblock,
										  ntaskExpr);
      return true;
    }
    else {
      throw MatchException("Failed to Match SPMD Region Pragma\n");
    }
  }
  catch(std::exception& e) {
    Logger::error(lg) << e.what();
    return false;
  }
}

/*******************************
 * OcrSpmdFinalizePragmaParser *
 *******************************/
OcrSpmdFinalizePragmaParser::OcrSpmdFinalizePragmaParser(SgPragmaDeclaration* sgpdecl, string input,
							 unsigned int traversalOrder, OcrObjectManager& ocrObjectManager)
  : m_sgpdecl(sgpdecl),
    m_traversalOrder(traversalOrder),
    m_ocrObjectManager(ocrObjectManager) {
  sr_identifier = +(alpha|as_xpr('_')) >> *_w;
  sr_param = *_s >> *by_ref(sr_identifier);
  // paramseq is the tail seq of a parameter list
  sregex sr_paramseq = *_s >> as_xpr(',') >> *_s >> sr_param;
  sr_paramlist = as_xpr('(') >> *_s >> *sr_param >> *by_ref(sr_paramseq) >> *_s >> as_xpr(')');
}

list<string> OcrSpmdFinalizePragmaParser::matchParamNames(string input) {
  list<string> paramList;
  if(!regex_match(input, sr_paramlist)) {
    throw MatchException("MatchException thrown by matchParamNames");
  }
  sregex_token_iterator cur(input.begin(), input.end(), sr_identifier), end;
  for( ; cur != end; ++cur) {
    string identifier = *cur;
    if(identifier.compare("NONE") == 0 || identifier.compare("none") == 0) {
      continue;
    }
    paramList.push_back(identifier);
  }
  return paramList;
}

list<string> OcrSpmdFinalizePragmaParser::matchDepEvts(string input) {
  sregex sr_dbknames = icase("DEP_EVTs") >> *_s >> (s1=sr_paramlist);
  smatch matchResults;
  list<string> dbkNames;
  if(regex_search(input, matchResults, sr_dbknames)) {
    string paramlist = matchResults[1];
    dbkNames = matchParamNames(paramlist);
  }
  else {
    throw MatchException("Matching failed in OcrSpmdFinalizePragmaParser::DEP_EVTs\n");
  }
  return dbkNames;
}

bool OcrSpmdFinalizePragmaParser::match() {
  Logger::Logger lg("OcrSpmdFinalizePragmaParser::match()", Logger::DEBUG);
  boost::xpressive::sregex sr_spmdfinalize = *_s >> as_xpr("ocr spmd finalize")
					       >> *_s >> icase("DEP_EVTS") >> *_s >> (sr_paramlist)
					       >> *_s;
  string input = m_sgpdecl->get_pragma()->get_pragma();
  try {
    if(regex_match(input, sr_spmdfinalize)) {
      list<string> depEvtNames = matchDepEvts(input);
      SgScopeStatement* scope = SageInterface::getEnclosingScope(m_sgpdecl);
      list<OcrEvtContextPtr> depEvts = m_ocrObjectManager.getOcrEvtContextList(depEvtNames);
      string spmdFinalizeEdtName = SageInterface::generateUniqueVariableName(scope, "spmdFinalizeEdt");
      OcrTaskContextPtr taskContext = m_ocrObjectManager.registerOcrSpmdFinalizeEdt(spmdFinalizeEdtName, m_traversalOrder, m_sgpdecl,
										    depEvts);
      assert(taskContext);
      return true;
    }
    else {
      throw MatchException("Failed to Match SPMD Finalize Pragma\n");
    }
  }
  catch(std::exception& e) {
    Logger::error(lg) << e.what();
    return false;
  }
}

/***************************
 * OcrSpmdSendPragmaParser *
 ***************************/
OcrSpmdSendPragmaParser::OcrSpmdSendPragmaParser(SgPragmaDeclaration* sgpdecl,
						 unsigned int traversalOrder, OcrObjectManager& ocrObjectManager)
  : m_sgpdecl(sgpdecl),
    m_traversalOrder(traversalOrder),
    m_ocrObjectManager(ocrObjectManager) {
  sr_identifier = +(alpha|as_xpr('_')) >> *_w;
  sr_param = *_s >> *by_ref(sr_identifier);
  // paramseq is the tail seq of a parameter list
  sregex sr_paramseq = *_s >> as_xpr(',') >> *_s >> sr_param;
  sr_paramlist = as_xpr('(') >> *_s >> *sr_param >> *by_ref(sr_paramseq) >> *_s >> as_xpr(')');
}

list<string> OcrSpmdSendPragmaParser::matchParamNames(string input) {
  list<string> paramList;
  if(!regex_match(input, sr_paramlist)) {
    throw MatchException("MatchException thrown by matchParamNames");
  }
  sregex_token_iterator cur(input.begin(), input.end(), sr_identifier), end;
  for( ; cur != end; ++cur) {
    string identifier = *cur;
    if(identifier.compare("NONE") == 0 || identifier.compare("none") == 0) {
      continue;
    }
    paramList.push_back(identifier);
  }
  return paramList;
}

string OcrSpmdSendPragmaParser::matchSendDbk(string input) {
  sregex sr_sendDbk = *_s >> icase("SEND_DBK") >> *_s >> '(' >> *_s >> (s1=sr_identifier) >> *_s >> ')';
  smatch matchResults;
  if(regex_search(input, matchResults, sr_sendDbk)) {
    return matchResults[1];
  }
  else {
    throw MatchException("Matching Failed in OcrSpmdSendPragmaParser::matchOutEvt\n");
  }
}

list<string> OcrSpmdSendPragmaParser::matchDepEvts(string input) {
  sregex sr_dbknames = icase("DEP_EVTs") >> *_s >> (s1=sr_paramlist);
  smatch matchResults;
  list<string> dbkNames;
  if(regex_search(input, matchResults, sr_dbknames)) {
    string paramlist = matchResults[1];
    dbkNames = matchParamNames(paramlist);
  }
  else {
    throw MatchException("Matching failed in OcrSpmdFinalizePragmaParser::DEP_EVTs\n");
  }
  return dbkNames;
}

string OcrSpmdSendPragmaParser::matchOutEvt(string input) {
  sregex outEvtNameOnly = *_s >> icase("OEVENT") >> *_s >> '(' >> *_s >> (s1=sr_identifier) >> *_s >> ')';
  smatch matchResults;
  if(regex_search(input, matchResults, outEvtNameOnly)) {
    return matchResults[1];
  }
  else {
    throw MatchException("Matching Failed in OcrSpmdSendPragmaParser::matchOutEvt\n");
  }
}

bool OcrSpmdSendPragmaParser::match() {
  Logger::Logger lg("OcrSpmdSendPragmaParser::match()", Logger::DEBUG);
  boost::xpressive::sregex sr_spmdsend = *_s >> as_xpr("ocr spmd send")
					       >> *_s >> icase("SEND_DBK") >> *_s >> '(' >> *_s >> (sr_identifier) >> *_s >> ')'
					       >> *_s >> icase("DEP_EVTS") >> *_s >> (sr_paramlist)
					       >> *_s >> icase("OEVENT") >> *_s >> '(' >> *_s >> (sr_identifier) >> *_s >> ')'
					       >> *_s;
  string input = m_sgpdecl->get_pragma()->get_pragma();
  try {
    if(regex_match(input, sr_spmdsend)) {
      SgScopeStatement* scope = SageInterface::getEnclosingScope(m_sgpdecl);
      string dbkName = matchSendDbk(input);
      OcrDbkContextPtr sendDbkContext = m_ocrObjectManager.getOcrDbkContext(dbkName, scope);
      list<string> depEvtNames = matchDepEvts(input);
      list<OcrEvtContextPtr> depEvts = m_ocrObjectManager.getOcrEvtContextList(depEvtNames);
      string outEvtName = matchOutEvt(input);
      OcrEvtContextPtr outEvtContext = m_ocrObjectManager.registerOcrEvt(outEvtName);
      string sendContextName = SageInterface::generateUniqueVariableName(scope, "send");
      SgStatement* callExpStmt = SageInterface::getNextStatement(m_sgpdecl);
      SgFunctionCallExp* sendCallExp = NULL;
      if(sendCallExp = isSgFunctionCallExp(isSgExprStatement(callExpStmt)->get_expression())) {
	SgFunctionRefExp* functionExp = isSgFunctionRefExp(sendCallExp->get_function());
	// Ensure that the annotation follows MPI_Send
	if(functionExp) {
	  SgFunctionSymbol* fsymbol = functionExp->get_symbol();
	  string fname = fsymbol->get_name().getString();
	  assert(fname.compare("MPI_Send") == 0 ||
		 fname.compare("MPI_Isend") == 0);
	}
	else {
	  // Annotation is on a function call called through function pointer
	  throw MatchException("Cannot find MPI send call expression\n");
	}
      }
      else {
	// Annotation is not followed by MPI_Send call statement
	throw MatchException("Unexpected statement following Send annotation\n");
      }
      assert(sendCallExp);
      OcrTaskContextPtr sendContext = m_ocrObjectManager.registerOcrSpmdSendContext(sendContextName, m_traversalOrder, m_sgpdecl,
										    sendDbkContext, depEvts, outEvtContext, sendCallExp);
      return true;
    }
    else {
      throw MatchException("Failed to Match SPMD Send Pragma\n");
    }
  }
  catch(std::exception& e) {
    Logger::error(lg) << e.what();
    return false;
  }
}

/***************************
 * OcrSpmdRecvPragmaParser *
 ***************************/
OcrSpmdRecvPragmaParser::OcrSpmdRecvPragmaParser(SgPragmaDeclaration* sgpdecl,
						 unsigned int traversalOrder, OcrObjectManager& ocrObjectManager)
  : m_sgpdecl(sgpdecl),
    m_traversalOrder(traversalOrder),
    m_ocrObjectManager(ocrObjectManager) {
  sr_identifier = +(alpha|as_xpr('_')) >> *_w;
  sr_param = *_s >> *by_ref(sr_identifier);
  // paramseq is the tail seq of a parameter list
  sregex sr_paramseq = *_s >> as_xpr(',') >> *_s >> sr_param;
  sr_paramlist = as_xpr('(') >> *_s >> *sr_param >> *by_ref(sr_paramseq) >> *_s >> as_xpr(')');
}

list<string> OcrSpmdRecvPragmaParser::matchParamNames(string input) {
  list<string> paramList;
  if(!regex_match(input, sr_paramlist)) {
    throw MatchException("MatchException thrown by matchParamNames");
  }
  sregex_token_iterator cur(input.begin(), input.end(), sr_identifier), end;
  for( ; cur != end; ++cur) {
    string identifier = *cur;
    if(identifier.compare("NONE") == 0 || identifier.compare("none") == 0) {
      continue;
    }
    paramList.push_back(identifier);
  }
  return paramList;
}

string OcrSpmdRecvPragmaParser::matchRecvEvt(string input) {
  sregex sr_recvDbk = *_s >> icase("RECV_EVT") >> *_s >> '(' >> *_s >> (s1=sr_identifier) >> *_s >> ')';
  smatch matchResults;
  if(regex_search(input, matchResults, sr_recvDbk)) {
    return matchResults[1];
  }
  else {
    throw MatchException("Matching Failed in OcrSpmdRecvPragmaParser::matchRecvEvt\n");
  }
}

list<string> OcrSpmdRecvPragmaParser::matchDepEvts(string input) {
  sregex sr_dbknames = icase("DEP_EVTs") >> *_s >> (s1=sr_paramlist);
  smatch matchResults;
  list<string> dbkNames;
  if(regex_search(input, matchResults, sr_dbknames)) {
    string paramlist = matchResults[1];
    dbkNames = matchParamNames(paramlist);
  }
  else {
    throw MatchException("Matching failed in OcrSpmdFinalizePragmaParser::DEP_EVTs\n");
  }
  return dbkNames;
}

string OcrSpmdRecvPragmaParser::matchOutEvt(string input) {
  sregex outEvtNameOnly = *_s >> icase("OEVENT") >> *_s >> '(' >> *_s >> (s1=sr_identifier) >> *_s >> ')';
  smatch matchResults;
  if(regex_search(input, matchResults, outEvtNameOnly)) {
    return matchResults[1];
  }
  else {
    throw MatchException("Matching Failed in OcrSpmdRecvPragmaParser::matchOutEvt\n");
  }
}

SgVariableSymbol* OcrSpmdRecvPragmaParser::getRecvBuffVariableSymbol_rec(SgExpression* rbuffExp) {
  assert(rbuffExp);
  switch(rbuffExp->variantT()) {
  // MPI_Recv(buff,...)
  // base case
  case V_SgVarRefExp: {
    return isSgVarRefExp(rbuffExp)->get_symbol();
  }
  // MPI_Recv(&buff,..)
  case V_SgAddressOfOp: {
    SgExpression* operand = isSgAddressOfOp(rbuffExp)->get_operand();
    return getRecvBuffVariableSymbol_rec(operand);
  }
  // MPI_Recv((void*) &rbuff,..
  case V_SgCastExp: {
    SgExpression* operand = isSgCastExp(rbuffExp)->get_operand();
    return getRecvBuffVariableSymbol_rec(operand);
  }
  default:
    throw MatchException("Cannot find recv buff variable symbol from recvCallExp\n");
  }
}

SgVariableSymbol* OcrSpmdRecvPragmaParser::getRecvBuffVariableSymbol(SgFunctionCallExp* recvCallExp) {
  // recursively traverse the tree and extract the variable symbol
  // recv buff is the first argument in the argument list
  SgExpression* rbuffExp = recvCallExp->get_args()->get_expressions()[0];
  return getRecvBuffVariableSymbol_rec(rbuffExp);
}

bool OcrSpmdRecvPragmaParser::match() {
  Logger::Logger lg("OcrSpmdRecvPragmaParser::match()", Logger::DEBUG);
  boost::xpressive::sregex sr_spmdrecv = *_s >> as_xpr("ocr spmd recv")
					     >> *_s >> icase("RECV_EVT") >> *_s >> '(' >> *_s >> (sr_identifier) >> *_s >> ')'
					     >> *_s >> icase("DEP_EVTS") >> *_s >> (sr_paramlist)
					     >> *_s >> icase("OEVENT") >> *_s >> '(' >> *_s >> (sr_identifier) >> *_s >> ')'
					     >> *_s;
  string input = m_sgpdecl->get_pragma()->get_pragma();
  try {
    if(regex_match(input, sr_spmdrecv)) {
      SgScopeStatement* scope = SageInterface::getEnclosingScope(m_sgpdecl);
      SgStatement* callExpStmt = SageInterface::getNextStatement(m_sgpdecl);
      SgFunctionCallExp* recvCallExp = NULL;
      if(recvCallExp = isSgFunctionCallExp(isSgExprStatement(callExpStmt)->get_expression())) {
	SgFunctionRefExp* functionExp = isSgFunctionRefExp(recvCallExp->get_function());
	// Ensure that the annotation follows MPI_Recv
	if(functionExp) {
	  SgFunctionSymbol* fsymbol = functionExp->get_symbol();
	  string fname = fsymbol->get_name().getString();
	  assert(fname.compare("MPI_Recv") == 0 ||
		 fname.compare("MPI_Irecv") == 0);
	}
	else {
	  // Annotation is on a function call called through function pointer
	  throw MatchException("Cannot find MPI recv call expression\n");
	}
      }
      else {
	// Annotation is not followed by MPI_Recv call statement
	throw MatchException("Unexpected statement following Recv annotation\n");
      }
      assert(recvCallExp);
      SgExpression* recvBuffExpr= recvCallExp->get_args()->get_expressions()[0];
      string recvEvtName = matchRecvEvt(input);
      OcrEvtContextPtr recvEvtContext = m_ocrObjectManager.registerOcrRecvEvt(recvEvtName, recvBuffExpr);

      list<string> depEvtNames = matchDepEvts(input);
      list<OcrEvtContextPtr> depEvts = m_ocrObjectManager.getOcrEvtContextList(depEvtNames);
      string outEvtName = matchOutEvt(input);
      OcrEvtContextPtr outEvtContext = m_ocrObjectManager.registerOcrEvt(outEvtName);

      string recvContextName = SageInterface::generateUniqueVariableName(scope, "recv");
      OcrTaskContextPtr recvContext = m_ocrObjectManager.registerOcrSpmdRecvContext(recvContextName, m_traversalOrder, m_sgpdecl,
										    recvEvtContext, depEvts, outEvtContext, recvCallExp);
      return true;
    }
    else {
      throw MatchException("Failed to Match SPMD Recv Pragma\n");
    }
  }
  catch(std::exception& e) {
    Logger::error(lg) << e.what();
    return false;
  }
}

/***************************
 * OcrSpmdReducePragmaParser *
 ***************************/
OcrSpmdReducePragmaParser::OcrSpmdReducePragmaParser(SgPragmaDeclaration* sgpdecl,
						 unsigned int traversalOrder, OcrObjectManager& ocrObjectManager)
  : m_sgpdecl(sgpdecl),
    m_traversalOrder(traversalOrder),
    m_ocrObjectManager(ocrObjectManager) {
  sr_identifier = +(alpha|as_xpr('_')) >> *_w;
  sr_param = *_s >> *by_ref(sr_identifier);
  // paramseq is the tail seq of a parameter list
  sregex sr_paramseq = *_s >> as_xpr(',') >> *_s >> sr_param;
  sr_paramlist = as_xpr('(') >> *_s >> *sr_param >> *by_ref(sr_paramseq) >> *_s >> as_xpr(')');
}

list<string> OcrSpmdReducePragmaParser::matchParamNames(string input) {
  list<string> paramList;
  if(!regex_match(input, sr_paramlist)) {
    throw MatchException("MatchException thrown by matchParamNames");
  }
  sregex_token_iterator cur(input.begin(), input.end(), sr_identifier), end;
  for( ; cur != end; ++cur) {
    string identifier = *cur;
    if(identifier.compare("NONE") == 0 || identifier.compare("none") == 0) {
      continue;
    }
    paramList.push_back(identifier);
  }
  return paramList;
}

string OcrSpmdReducePragmaParser::matchReduceEvt(string input) {
  sregex sr_reduceDbk = *_s >> icase("REDUCE_EVT") >> *_s >> '(' >> *_s >> (s1=sr_identifier) >> *_s >> ')';
  smatch matchResults;
  if(regex_search(input, matchResults, sr_reduceDbk)) {
    return matchResults[1];
  }
  else {
    throw MatchException("Matching Failed in OcrSpmdReducePragmaParser::matchReduceEvt\n");
  }
}

list<string> OcrSpmdReducePragmaParser::matchDepEvts(string input) {
  sregex sr_dbknames = icase("DEP_EVTs") >> *_s >> (s1=sr_paramlist);
  smatch matchResults;
  list<string> dbkNames;
  if(regex_search(input, matchResults, sr_dbknames)) {
    string paramlist = matchResults[1];
    dbkNames = matchParamNames(paramlist);
  }
  else {
    throw MatchException("Matching failed in OcrSpmdFinalizePragmaParser::DEP_EVTs\n");
  }
  return dbkNames;
}

string OcrSpmdReducePragmaParser::matchOutEvt(string input) {
  sregex outEvtNameOnly = *_s >> icase("OEVENT") >> *_s >> '(' >> *_s >> (s1=sr_identifier) >> *_s >> ')';
  smatch matchResults;
  if(regex_search(input, matchResults, outEvtNameOnly)) {
    return matchResults[1];
  }
  else {
    throw MatchException("Matching Failed in OcrSpmdReducePragmaParser::matchOutEvt\n");
  }
}

bool OcrSpmdReducePragmaParser::match() {
  Logger::Logger lg("OcrSpmdReducePragmaParser::match()", Logger::DEBUG);
  boost::xpressive::sregex sr_spmdreduce = *_s >> as_xpr("ocr spmd reduce")
					     >> *_s >> icase("REDUCE_EVT") >> *_s >> '(' >> *_s >> (sr_identifier) >> *_s >> ')'
					     >> *_s >> icase("DEP_EVTS") >> *_s >> (sr_paramlist)
					     >> *_s >> icase("OEVENT") >> *_s >> '(' >> *_s >> (sr_identifier) >> *_s >> ')'
					     >> *_s;
  string input = m_sgpdecl->get_pragma()->get_pragma();
  try {
    if(regex_match(input, sr_spmdreduce)) {
      SgScopeStatement* scope = SageInterface::getEnclosingScope(m_sgpdecl);
      SgStatement* callExpStmt = SageInterface::getNextStatement(m_sgpdecl);
      SgFunctionCallExp* reduceCallExp = NULL;
      if(reduceCallExp = isSgFunctionCallExp(isSgExprStatement(callExpStmt)->get_expression())) {
	SgFunctionRefExp* functionExp = isSgFunctionRefExp(reduceCallExp->get_function());
	// Ensure that the annotation follows MPI_Reduce
	if(functionExp) {
	  SgFunctionSymbol* fsymbol = functionExp->get_symbol();
	  string fname = fsymbol->get_name().getString();
	  assert(fname.compare("MPI_Reduce") == 0);
	}
	else {
	  // Annotation is on a function call called through function pointer
	  throw MatchException("Cannot find MPI reduce call expression\n");
	}
      }
      else {
	// Annotation is not followed by MPI_Reduce call statement
	throw MatchException("Unexpected statement following Reduce annotation\n");
      }
      assert(reduceCallExp);

      string reduceEvtName = matchReduceEvt(input);
      // Get the reduce buffer variable symbol from the reduce call exp
      SgExprListExp* reduceCallArgs = reduceCallExp->get_args();
      // recvbuf is the second argument
      SgExpression* recvbuf = reduceCallArgs->get_expressions()[1];
      assert(recvbuf);
      OcrEvtContextPtr reduceEvtContext = m_ocrObjectManager.registerOcrReduceEvt(reduceEvtName, recvbuf);

      list<string> depEvtNames = matchDepEvts(input);
      list<OcrEvtContextPtr> depEvts = m_ocrObjectManager.getOcrEvtContextList(depEvtNames);
      string outEvtName = matchOutEvt(input);
      OcrEvtContextPtr outEvtContext = m_ocrObjectManager.registerOcrEvt(outEvtName);

      string reduceContextName = SageInterface::generateUniqueVariableName(scope, "reduce");
      OcrTaskContextPtr reduceContext = m_ocrObjectManager.registerOcrSpmdReduceContext(reduceContextName, m_traversalOrder, m_sgpdecl,
											reduceEvtContext, depEvts, outEvtContext, reduceCallExp);
      return true;
    }
    else {
      throw MatchException("Failed to Match SPMD Reduce Pragma\n");
    }
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

OcrObjectManager& OcrPragmaParser::getOcrObjectManager() {
  return m_ocrObjectManager;
}

void OcrPragmaParser::visit(SgNode* sgn) {
  Logger::Logger lg("OcrPragmaParser::visit()");
  if(SgPragmaDeclaration* sgpdecl = isSgPragmaDeclaration(sgn)) {
    // Identify type of the pragma
    OcrPragmaType::OcrPragmaType_t ptype = m_ocrPragmaType.identifyPragmaType(sgpdecl);

    if(ptype == OcrPragmaType::e_TaskEdt) {
      AstFromString::afs_skip_whitespace();
      // Increase the traversal order counter
      m_taskOrderCounter = m_taskOrderCounter+1;
      OcrTaskPragmaParser taskPragmaParser(AstFromString::c_char, m_ocrObjectManager, sgpdecl, m_taskOrderCounter);
      if(taskPragmaParser.match()) {
	Logger::info(lg) << "Task Pragma Parsed Successfully\n";
      }
      else {
	Logger::error(lg) << "Task Pragma Parsing Failed\n";
	std::terminate();
      }
    }
    else if(ptype == OcrPragmaType::e_LoopIterEdt) {
      AstFromString::afs_skip_whitespace();
      // Increase the traversal order counter
      m_taskOrderCounter = m_taskOrderCounter+1;
      OcrLoopIterPragmaParser loopIterPragmaParser(sgpdecl, m_ocrObjectManager, m_taskOrderCounter);
      if(loopIterPragmaParser.match()) {
	Logger::info(lg) << "Loop Pragma Parsed Successfully\n";
      }
      else {
	Logger::error(lg) << "Loop Pragma Parsing Failed\n";
	std::terminate();
      }
    }
    else if(ptype == OcrPragmaType::e_ShutdownEdt) {
      // Increase the traversal order counter
      m_taskOrderCounter = m_taskOrderCounter+1;
      AstFromString::afs_skip_whitespace();
      OcrShutdownPragmaParser shutdownPragmaParser(sgpdecl, AstFromString::c_char, m_taskOrderCounter, m_ocrObjectManager);
      if(shutdownPragmaParser.match()) {
	Logger::info(lg) << "Shutdown Pragma Parsed Successfully\n";
      }
      else {
	Logger::error(lg) << "Shutdown Pragma Parsing Failed\n";
	std::terminate();
      }
    }
    else if(ptype == OcrPragmaType::e_SpmdRegionEdt) {
      // Increase the traversal order counter
      m_taskOrderCounter = m_taskOrderCounter+1;
      AstFromString::afs_skip_whitespace();
      OcrSpmdRegionPragmaParser spmdRegionPragmaParser(sgpdecl, AstFromString::c_char, m_taskOrderCounter, m_ocrObjectManager);
      if(spmdRegionPragmaParser.match()) {
	Logger::info(lg) << "Spmd Region Pragma Parsed Successfully\n";
      }
      else {
	Logger::error(lg) << "Spmd Region Pragma Parsing Failed\n";
	std::terminate();
      }
    }
    else if(ptype == OcrPragmaType::e_SpmdFinalizeEdt) {
      // Increase the traversal order counter
      m_taskOrderCounter = m_taskOrderCounter+1;
      AstFromString::afs_skip_whitespace();
      OcrSpmdFinalizePragmaParser spmdFinalizePragmaParser(sgpdecl, AstFromString::c_char, m_taskOrderCounter, m_ocrObjectManager);
      if(spmdFinalizePragmaParser.match()) {
	Logger::info(lg) << "SpmdFinalize Pragma Parsed Successfully\n";
      }
      else {
	Logger::error(lg) << "SpmdFinalize Pragma Parsing Failed\n";
	std::terminate();
      }
    }
    else if(ptype == OcrPragmaType::e_SpmdSend) {
      // Increase the traversal order counter
      m_taskOrderCounter++;
      OcrSpmdSendPragmaParser spmdSendPragmaParser(sgpdecl, m_taskOrderCounter, m_ocrObjectManager);
      if(spmdSendPragmaParser.match()) {
	Logger::info(lg) << "SpmdSend Pragma Parsed Successfully\n";
      }
      else {
	Logger::error(lg) << "SpmdSend Pragma Parsing Failed\n";
	std::terminate();
      }
    }
    else if(ptype == OcrPragmaType::e_SpmdRecv) {
      // Increase the traversal order counter
      m_taskOrderCounter++;
      OcrSpmdRecvPragmaParser spmdRecvPragmaParser(sgpdecl, m_taskOrderCounter, m_ocrObjectManager);
      if(spmdRecvPragmaParser.match()) {
	Logger::info(lg) << "SpmdRecv Pragma Parsed Successfully\n";
      }
      else {
	Logger::error(lg) << "SpmdRecv Pragma Parsing Failed\n";
	std::terminate();
      }
    }
    else if(ptype == OcrPragmaType::e_SpmdReduce) {
      // Increase the traversal order counter
      m_taskOrderCounter++;
      OcrSpmdReducePragmaParser spmdReducePragmaParser(sgpdecl, m_taskOrderCounter, m_ocrObjectManager);
      if(spmdReducePragmaParser.match()) {
	Logger::info(lg) << "Spmd Reduce Pragma Parsed Successfully\n";
      }
      else {
	Logger::error(lg) << "SpmdReduce Pragma Parsing Failed\n";
	std::terminate();
      }
    }
    else if(ptype == OcrPragmaType::e_Dbk) {
      AstFromString::afs_skip_whitespace();
      OcrDbkPragmaParser dbkPragmaParser(sgpdecl, m_ocrObjectManager);
      if(dbkPragmaParser.match()) {
	SgScopeStatement* scope = SageInterface::getEnclosingScope(sgpdecl);
	list<OcrDbkContextPtr> dbkContextList = dbkPragmaParser.getDbkContextList();
	list<OcrDbkContextPtr>::iterator d = dbkContextList.begin();
	for( ; d != dbkContextList.end(); ++d) {
	  m_ocrObjectManager.registerOcrDbk((*d)->get_name(), *d, scope);
	}
	Logger::info(lg) << "Datablock Pragma Parsed Successfully\n";
      }
      else {
	Logger::error(lg) << "Datablock Pragma Parsing Failed\n";
	std::terminate();
      }
    }
    else if(SageInterface::isMain(sgn)) {
      m_taskOrderCounter = m_taskOrderCounter+1;
      SgFunctionDeclaration* mdecl = isSgFunctionDeclaration(sgn);
      SgFunctionDefinition* mdefn = mdecl->get_definition();
      SgBasicBlock* mbasicblock = mdefn->get_body();
      OcrTaskContextPtr mainEdtContext = m_ocrObjectManager.registerOcrMainEdt("mainEdt", m_taskOrderCounter, mbasicblock);
      Logger::debug(lg) << mainEdtContext->str() << endl;
    }
    else{ } // do nothing
  }
  // Collect all MPI call expressions
  else if(SgFunctionCallExp* callExp = isSgFunctionCallExp(sgn)) {
    SgFunctionRefExp* functionExp = isSgFunctionRefExp(callExp->get_function());
    SgStatement* callStmt = SageInterface::getEnclosingStatement(callExp);
    if(functionExp) {
      SgFunctionSymbol* fsymbol = functionExp->get_symbol();
      assert(fsymbol);
      string fname = fsymbol->get_name().getString();
      MpiOpContextPtr mpiOpContext;
      bool isMpiOp = false;
      if(isMpiOp = (fname.compare("MPI_Init") == 0)) {
	mpiOpContext = boost::make_shared<MpiOpContext>(MpiOpContext::OP_INIT, callStmt);
      }
      else if(isMpiOp = (fname.compare("MPI_Finalize") == 0)) {
	mpiOpContext = boost::make_shared<MpiOpContext>(MpiOpContext::OP_FINALIZE, callStmt);
      }
      else if(isMpiOp = (fname.compare("MPI_Comm_rank") == 0)) {
	mpiOpContext = boost::make_shared<MpiOpContext>(MpiOpContext::OP_COMM_RANK, callStmt);
      }
      else if(isMpiOp = (fname.compare("MPI_Comm_size") == 0)) {
	mpiOpContext = boost::make_shared<MpiOpContext>(MpiOpContext::OP_COMM_SIZE, callStmt);
      }
      else if(isMpiOp = (fname.compare("MPI_Send") == 0)) {
	mpiOpContext = boost::make_shared<MpiOpContext>(MpiOpContext::OP_SEND, callStmt);
      }
      else if(isMpiOp = (fname.compare("MPI_Isend") == 0)) {
	mpiOpContext = boost::make_shared<MpiOpContext>(MpiOpContext::OP_SEND, callStmt);
      }
      else if(isMpiOp = (fname.compare("MPI_Recv") == 0)) {
	mpiOpContext = boost::make_shared<MpiOpContext>(MpiOpContext::OP_RECV, callStmt);
      }
      else if(isMpiOp = (fname.compare("MPI_Irecv") == 0)) {
	mpiOpContext = boost::make_shared<MpiOpContext>(MpiOpContext::OP_RECV, callStmt);
      }
      else if(isMpiOp = (fname.compare("MPI_Reduce") == 0)) {
	mpiOpContext = boost::make_shared<MpiOpContext>(MpiOpContext::OP_REDUCE, callStmt);
      }
      else if(isMpiOp = (fname.compare("MPI_Wait") == 0)) {
	mpiOpContext = boost::make_shared<MpiOpContext>(MpiOpContext::OP_WAIT, callStmt);
      }
      else if(isMpiOp = (fname.compare("MPI_Wtime") == 0)) {
	mpiOpContext = boost::make_shared<MpiOpContext>(MpiOpContext::OP_WTIME, callStmt);
      }
      else if(isMpiOp = (fname.find("MPI_") == 0)){
	std::cerr << "Unhandled MPI function " << fname << endl;
	std::terminate();
      }
      else {
	// all other functions that are not MPI
      }
      if(isMpiOp) {
	assert(mpiOpContext);
	m_ocrObjectManager.registerMpiOpContext(mpiOpContext);
      }
    }
    // Function call expression is happening through pointers or class
    // TODO: How to identify MPI calls under such cases
    else {

    }
  }
}

void OcrPragmaParser::atTraversalEnd() {
}

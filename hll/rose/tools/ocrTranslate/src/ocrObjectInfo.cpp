/*
 * Author: Sriram Aananthakrishnan, 2016
 */

#include "sage3basic.h"
#include "ocrObjectInfo.h"
#include "boost/make_shared.hpp"
#include "logger.h"
#include <algorithm>

using namespace std;

/*****************
 * OcrDbkContext *
 *****************/
OcrDbkContext::OcrDbkContext(std::string name)
  : m_name(name) { }

OcrDbkContext::OcrDbkContext(std::string name, SgInitializedName* vdefn, list<SgStatement*> allocStmts, SgPragmaDeclaration* pragma)
  : m_name(name),
    m_vdefn(vdefn),
    m_allocStmts(allocStmts),
    m_pragma(pragma) { }

SgInitializedName* OcrDbkContext::getSgInitializedName() const {
  return m_vdefn;
}

SgPragmaDeclaration* OcrDbkContext::get_pragma() const {
  return m_pragma;
}

// Function that computes the pointer type for the datablock pointer
SgType* OcrDbkContext::getDbkPtrType() {
  SgType* vtype = m_vdefn->get_type();
  SgType* dbkPtrType = NULL;
  SgType* btype = NULL;
  switch(vtype->variantT()) {
  // If the declared variable is a base type we simply build a pointer to the base type
  case V_SgTypeBool:
  case V_SgTypeChar:
  case V_SgTypeDouble:
  case V_SgTypeFloat:
  case V_SgTypeInt:
  case V_SgTypeLong:
  case V_SgTypeLongDouble:
  case V_SgTypeLongLong:
  case V_SgTypeShort:
  case V_SgTypeVoid:
  case V_SgTypeUnsignedChar:
  case V_SgTypeUnsignedInt:
  case V_SgTypeUnsignedLong:
  case V_SgTypeUnsignedLongLong:
  case V_SgTypeUnsignedShort:
    dbkPtrType = SageBuilder::buildPointerType(vtype);
    return dbkPtrType;
  case V_SgArrayType:
  case V_SgPointerType:
    btype = vtype->findBaseType();
    assert(btype);
    dbkPtrType = SageBuilder::buildPointerType(btype);
    return dbkPtrType;
  // If this is a struct type
  case V_SgClassType:
    dbkPtrType = SageBuilder::buildPointerType(vtype);
    return dbkPtrType;
  case V_SgTypedefType:
    dbkPtrType = SageBuilder::buildPointerType(vtype);
    return dbkPtrType;
  default:
    cerr << "Unhandled Type " << AstDebug::astToString(vtype) << endl;
    assert(false);
  };
}

SgDeclarationStatement* OcrDbkContext::get_declaration() const {
  SgDeclarationStatement* stmt = m_vdefn->get_definition();
  assert(stmt);
  return stmt;
}

list<SgStatement*> OcrDbkContext::get_allocStmts() const {
  return m_allocStmts;
}

string OcrDbkContext::get_name() const {
  return m_name;
}

string OcrDbkContext::str() const {
  ostringstream oss;
  string indent = " ";
  oss << "[DBK: " << m_name << "\n";
  oss << indent << "vdefn:" << StrUtil::SgInitializedName2Str(m_vdefn) << endl;
  oss << indent << "allocStmts:[\n" << StrUtil::stmtlist2str(m_allocStmts, indent) << "]";
  oss << "]";
  return oss.str();
}

OcrDbkContext::~OcrDbkContext() {
  // No dynamic memory here to cleanup
}

/*****************
 * OcrEvtContext *
 *****************/
OcrEvtContext::OcrEvtContext(std::string name)
  : m_name(name) { }

string OcrEvtContext::get_name() const {
  return m_name;
}

string OcrEvtContext::str() const {
  ostringstream oss;
  oss << "[EVT: " << m_name << "]";
  return oss.str();
}

OcrEvtContext::~OcrEvtContext() {
  // No dynamic memory here to cleanup
}

/******************
 * OcrTaskContext *
 ******************/
OcrTaskContext::OcrTaskContext(OcrTaskType type, string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl)
  : m_type(type),
    m_taskName(name),
    m_traversalOrder(traversalOrder),
    m_sgpdecl(sgpdecl) {
}

OcrTaskContext::OcrTaskType OcrTaskContext::getTaskType() const {
  return m_type;
}

SgPragmaDeclaration* OcrTaskContext::getPragma() const {
  return m_sgpdecl;
}

unsigned int OcrTaskContext::getTraversalOrder() const {
  return m_traversalOrder;
}

string OcrTaskContext::getTaskName() const {
  return m_taskName;
}

OcrTaskContext::~OcrTaskContext() {
  // no cleanup required here
}

/*****************
 * OcrEdtContext *
 *****************/
OcrEdtContext::OcrEdtContext(string name,
			     unsigned int traversalOrder,
			     SgPragmaDeclaration* sgpdecl,
			     list<OcrDbkContextPtr> depDbks,
			     list<OcrEvtContextPtr> depEvts,
			     list<SgVarRefExp*> depElems,
			     OcrEvtContextPtr outputEvt,
			     SgBasicBlock* basicblock,
			     bool finishEdt)
  : OcrTaskContext(OcrTaskContext::e_TaskEdt, name, traversalOrder, sgpdecl),
    m_depDbks(depDbks),
    m_depEvts(depEvts),
    m_depElems(depElems),
    m_outputEvt(outputEvt),
    m_basicblock(basicblock),
    m_finishEdt(finishEdt) {
}

string OcrEdtContext::get_name() const {
  return m_taskName;
}

SgBasicBlock* OcrEdtContext::getTaskBasicBlock() const {
  return m_basicblock;
}

list<SgVarRefExp*> OcrEdtContext::getDepElems() const {
  return m_depElems;
}

list<OcrDbkContextPtr> OcrEdtContext::getDepDbks() const {
  return m_depDbks;
}

list<OcrEvtContextPtr> OcrEdtContext::getDepEvts() const {
  return m_depEvts;
}

OcrEvtContextPtr OcrEdtContext::getOutputEvt() const {
  return m_outputEvt;
}

SgSourceFile* OcrEdtContext::getSourceFile() {
  return SageInterface::getEnclosingSourceFile(m_sgpdecl);
}

SgPragmaDeclaration* OcrEdtContext::getTaskPragma() const {
  return m_sgpdecl;
}

unsigned int OcrEdtContext::getDepDbkSlotNumber(string dbkname) const {
  int slot = 0;
  list<OcrDbkContextPtr>::const_iterator dbk = m_depDbks.begin();
  for( ; dbk != m_depDbks.end(); ++dbk, ++slot) {
    if(dbkname.compare((*dbk)->get_name()) == 0) {
      return slot;
    }
  }
  if(dbk == m_depDbks.end()) {
    cerr << "ERROR: Cannot find " << dbkname << "in m_depDbks\n";
    assert(false);
  }
}

unsigned int OcrEdtContext::getDepEvtSlotNumber(string evtname) const {
  // starting slot for evts is after datablocks
  int slot = m_depDbks.size();
  list<OcrEvtContextPtr>::const_iterator evt = m_depEvts.begin();
  for( ; evt != m_depEvts.end(); ++evt, ++slot) {
    if(evtname.compare((*evt)->get_name()) == 0) {
      return slot;
    }
  }
  if(evt == m_depEvts.end()) {
    cerr << "ERROR: Cannot find " << evtname << "in m_depEvt\n";
    assert(false);
  }
}

list<string> OcrEdtContext::getDbksToDestroy() const {
  return m_dbksToDestroy;
}

list<string> OcrEdtContext::getEvtsToDestroy() const {
  return m_evtsToDestroy;
}

unsigned int OcrEdtContext::getNumDepElems() const {
  return m_depElems.size();
}

unsigned int OcrEdtContext::getNumDepDbks() const {
  return m_depDbks.size();
}

unsigned int OcrEdtContext::getNumDepEvts() const {
  return m_depEvts.size();
}

list<OcrDbkContextPtr> OcrEdtContext::getDbksToCreate() const {
  return m_dbksToCreate;
}

void OcrEdtContext::setDbksToDestroy(list<string> dbksToDestroy) {
  m_dbksToDestroy = dbksToDestroy;
}

void OcrEdtContext::setEvtsToDestroy(list<string> evtsToDestroy) {
  m_evtsToDestroy = evtsToDestroy;
}

void OcrEdtContext::setDbksToCreate(list<OcrDbkContextPtr> dbksToCreate) {
  m_dbksToCreate = dbksToCreate;
}

bool OcrEdtContext::isFinishEdt() const {
  return m_finishEdt;
}

string OcrEdtContext::str() const {
  ostringstream oss;
  string indent = " ";
  oss << "[EDT: " << m_taskName << "\n";
  oss << indent << "depEvts:" << StrUtil::list2str<OcrEvtContext>(m_depEvts) << endl;
  oss << indent << "depDbks: [";
  list<OcrDbkContextPtr>::const_iterator d = m_depDbks.begin();
  while(d != m_depDbks.end()) {
    oss << (*d)->get_name();
    ++d;
    if(d != m_depDbks.end()) oss << ", ";
  }
  oss << "]\n";
  oss << indent << "outEvt: " << m_outputEvt->str() << endl;
  oss << indent << "depElems: " << StrUtil::SgVarRefExpList2Str(m_depElems) << endl;
  oss << indent << "dbksToCreate: [";
  d = m_dbksToCreate.begin();
  while(d != m_dbksToCreate.end()) {
    oss << (*d)->get_name();
    ++d;
    if(d != m_depDbks.end()) oss << ", ";
  }
  oss << "]\n";
  // oss << indent << "taskStmts:[\n" << AstDebug::astToString(m_basicblock) << "]";
  oss << "]";
  return oss.str();
}

OcrEdtContext::~OcrEdtContext() {
  // no cleanup required
}

/*************************
 * OcrShutdownEdtContext *
 *************************/
OcrShutdownEdtContext::OcrShutdownEdtContext(std::string name, unsigned int traversalOrder,
					     SgPragmaDeclaration* shutdownPragma, std::list<OcrEvtContextPtr> depEvts)
  : OcrTaskContext(OcrTaskContext::e_TaskShutDown, name, traversalOrder, shutdownPragma),
    m_depEvts(depEvts) {
}

std::list<OcrEvtContextPtr> OcrShutdownEdtContext::getDepEvts() const {
  return m_depEvts;
}

unsigned int OcrShutdownEdtContext::getNumDepEvts() {
  return m_depEvts.size();
}

SgSourceFile* OcrShutdownEdtContext::getSourceFile() const {
  return SageInterface::getEnclosingSourceFile(m_sgpdecl);
}

string OcrShutdownEdtContext::getShutdownEdtFuncName() const {
  string filename = StrUtil::GetFileNameString(m_sgpdecl->get_file_info()->get_filenameString());
  return filename+"ShutdownEdt";
}

string OcrShutdownEdtContext::str() const {
  ostringstream oss;
  string indent = " ";
  oss << "[Shutdown EDT\n";
  oss << indent << "depEvts:" << StrUtil::list2str<OcrEvtContext>(m_depEvts);
  oss << "]";
  return oss.str();
}

/*********************
 * OcrMainEdtContext *
 *********************/
OcrMainEdtContext::OcrMainEdtContext(string name, unsigned int traversalOrder, SgBasicBlock* basicblock)
  : OcrTaskContext(OcrTaskContext::e_TaskMain, name, traversalOrder, NULL),
    m_basicblock(basicblock) {
}

string OcrMainEdtContext::getMainEdtFuncName() const {
  return getTaskName();
}

string OcrMainEdtContext::str() const {
  ostringstream oss;
  string indent = " ";
  oss << "[EDT: " << m_taskName;
  oss << "]\n";
  return oss.str();
}

/*************************
 * OcrLoopIterEdtContext *
 *************************/
OcrLoopIterEdtContext::OcrLoopIterEdtContext(std::string name,  unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
					     std::list<OcrDbkContextPtr> depDbks,
					     std::list<OcrEvtContextPtr> depEvts,
					     std::list<SgVarRefExp*> depElems,
					     OcrEvtContextPtr outputEvt,
					     SgStatement* loopStmt)
  : OcrTaskContext(OcrTaskContext::e_TaskLoopIter, name, traversalOrder, sgpdecl),
    m_depDbks(depDbks),
    m_depEvts(depEvts),
    m_depElems(depElems),
    m_outputEvt(outputEvt),
    m_loopStmt(loopStmt) {
}

list<OcrDbkContextPtr> OcrLoopIterEdtContext::getDepDbks() const {
  return m_depDbks;
}

list<OcrEvtContextPtr> OcrLoopIterEdtContext::getDepEvts() const {
  return m_depEvts;
}

list<SgVarRefExp*> OcrLoopIterEdtContext::getDepElems() const {
  return m_depElems;
}


SgStatement* OcrLoopIterEdtContext::getLoopStmt() const {
  return m_loopStmt;
}

string OcrLoopIterEdtContext::getLoopBodyEdtName() const {
  return m_taskName + "LoopBodyEdt";
}

SgSourceFile* OcrLoopIterEdtContext::getSourceFile() const {
  return SageInterface::getEnclosingSourceFile(m_sgpdecl);
}

string OcrLoopIterEdtContext::getLoopControlEdtName() const {
  return m_taskName + "LoopControlEdt";
}

string OcrLoopIterEdtContext::str() const {
  return "OcrLoopIterEdtContext";
}

OcrLoopIterEdtContext::~OcrLoopIterEdtContext() {
}

/********************
 * OcrObjectManager *
 ********************/
OcrObjectManager::OcrObjectManager() { }

list<OcrEvtContextPtr> OcrObjectManager::getOcrEvtContextList(list<string> evtNamesList) {
  Logger::Logger lg("OcrObjectManager::getOcrEvtContextList", Logger::INFO);
  list<OcrEvtContextPtr> ocrEvtContextList;
  list<string>::iterator l = evtNamesList.begin();
  for( ; l != evtNamesList.end(); ++l) {
    string evtName = *l;
    Logger::debug(lg) << "evtName: " << evtName << endl;
    OcrEvtObjectMap::iterator found = m_ocrEvtObjectMap.find(evtName);
    if(found != m_ocrEvtObjectMap.end()) {
      OcrEvtContextPtr evtcontext_sp = boost::dynamic_pointer_cast<OcrEvtContext>(found->second);
      ocrEvtContextList.push_back(evtcontext_sp);
    }
    // #SA:1/4/2017
    // OcrEvts are registered by the annotation OEVT(...)
    // When this function is used to get OcrEvtContexts for all DEP_EVTs(...),
    // the OcrEvts are expected to be registered by a prior OEVT
    // It is possible that DEP_EVTs can have OcrEvts that are not registered apriori
    // For now, I'm assuming all OcrEvts listed under DEP_EVTs are registered by
    // a prior OEVT annotation
    else {
      Logger::error(lg) << "evtName: " << evtName << " not found in m_ocrEvtObjectMap\n";
      assert(false);
    }
  }
  return ocrEvtContextList;
}

list<OcrDbkContextPtr> OcrObjectManager::getOcrDbkContextList(list<string> dbkNamesList, SgScopeStatement* scope) {
  Logger::Logger lg("OcrObjectManager::getOcrDbkContextList");
  list<OcrDbkContextPtr> dbkContextList;
  list<string>::iterator d = dbkNamesList.begin();
  for( ; d != dbkNamesList.end(); ++d) {
    string dbkName = *d;
    OcrDbkContextPtr dbkContext = m_dbkSymbolTable.getObjectPtr(dbkName, scope);
    assert(dbkContext);
    dbkContextList.push_back(dbkContext);
  }
  return dbkContextList;
}

list<OcrEvtContextPtr> OcrObjectManager::registerOcrEvts(list<string> evtsNameList) {
  list<OcrEvtContextPtr> evtContextsList;
  list<string>::iterator l = evtsNameList.begin();
  for( ; l != evtsNameList.end(); ++l) {
    OcrEvtContextPtr evtcontext_sp = registerOcrEvt(*l);
    evtContextsList.push_back(evtcontext_sp);
  }
  return evtContextsList;
}

//! Check to see if the OcrObject is already registered
//! If already registered return its context
//! If not registered create a new context and insert in m_ocrObjectMap
OcrEvtContextPtr OcrObjectManager::registerOcrEvt(string evtName) {
  OcrEvtObjectMap::iterator f = m_ocrEvtObjectMap.find(evtName);
  OcrEvtContextPtr evtcontext_sp;
  if(f != m_ocrEvtObjectMap.end()) {
    evtcontext_sp = boost::dynamic_pointer_cast<OcrEvtContext>(f->second);
  }
  else {
    evtcontext_sp = boost::make_shared<OcrEvtContext>(evtName);
    OcrEvtObjectMapElem elem(evtName, evtcontext_sp);
    m_ocrEvtObjectMap.insert(elem);
  }
  assert(evtcontext_sp);
  return evtcontext_sp;
}

void OcrObjectManager::registerOcrDbk(std::string dbkName, OcrDbkContextPtr dbkContext, SgScopeStatement* scope) {
  m_dbkSymbolTable.insertObjectPtr(dbkName, dbkContext, scope);
}

//! Check to see if the OcrObject is already registered
//! If already registered return its context
//! If not registered create a new context and insert in m_ocrObjectMap
OcrTaskContextPtr OcrObjectManager::registerOcrEdt(std::string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
						   std::list<OcrDbkContextPtr> depDbks,
						   std::list<OcrEvtContextPtr> depEvts,
						   std::list<SgVarRefExp*> depElems,
						   OcrEvtContextPtr outputEvt,
						   SgBasicBlock* basicblock,
						   bool finishEdt) {
  OcrTaskContextMap::iterator f = m_ocrTaskContextMap.find(name);
  if(f != m_ocrTaskContextMap.end()) {
    assert(f->second);
    cerr << "WARNING: TASK " << name << "is already registered\n";
    return f->second;
  }
  else {
    OcrTaskContextPtr taskcontext_sp(new OcrEdtContext(name, traversalOrder, sgpdecl, depDbks, depEvts, depElems, outputEvt,
						       basicblock, finishEdt));
    OcrTaskContextMapElem elem(name, taskcontext_sp);
    assert(taskcontext_sp);
    m_ocrTaskContextMap.insert(elem);
    return taskcontext_sp;
  }
}

OcrTaskContextPtr OcrObjectManager::registerOcrLoopIterEdt(std::string name,  unsigned int traversalOrder,
							  SgPragmaDeclaration* sgpdecl,
							  std::list<OcrDbkContextPtr> depDbks,
							  std::list<OcrEvtContextPtr> depEvts,
							  std::list<SgVarRefExp*> depElems,
							  OcrEvtContextPtr outputEvt,
							  SgStatement* loopStmt) {
  OcrTaskContextMap::iterator f = m_ocrTaskContextMap.find(name);
  if(f != m_ocrTaskContextMap.end()) {
    assert(f->second);
    cerr << "WARNING: TASK " << name << "is already registered\n";
    return f->second;
  }
  else {
    OcrLoopIterEdtContextPtr taskcontext_sp(new OcrLoopIterEdtContext(name, traversalOrder, sgpdecl, depDbks, depEvts,
								      depElems, outputEvt, loopStmt));
    OcrTaskContextMapElem elem(name, taskcontext_sp);
    assert(taskcontext_sp);
    m_ocrTaskContextMap.insert(elem);
    return taskcontext_sp;
  }
}

OcrTaskContextPtr OcrObjectManager::registerOcrShutdownEdt(std::string name, unsigned int traversalOrder, SgPragmaDeclaration* shutdownPragma,
					      std::list<OcrEvtContextPtr> depEvts) {
  OcrTaskContextMap::iterator f = m_ocrTaskContextMap.find(name);
  if(f != m_ocrTaskContextMap.end()) {
    assert(f->second);
    cerr << "WARNING: TASK " << name << "is already registered\n";
    return f->second;
  }
  else {
    OcrShutdownEdtContextPtr taskcontext_sp(new OcrShutdownEdtContext(name, traversalOrder, shutdownPragma, depEvts));
    OcrTaskContextMapElem elem(name, taskcontext_sp);
    assert(taskcontext_sp);
    m_ocrTaskContextMap.insert(elem);
    return taskcontext_sp;
  }
}

OcrTaskContextPtr OcrObjectManager::registerOcrMainEdt(string name, unsigned int traversalOrder, SgBasicBlock* basicblock) {
  OcrTaskContextMap::iterator f = m_ocrTaskContextMap.find(name);
  if(f != m_ocrTaskContextMap.end()) {
    assert(f->second);
    cerr << "WARNING: TASK " << name << "is already registered\n";
    return f->second;
  }
  else {
    OcrTaskContextPtr taskcontext_sp = boost::make_shared<OcrMainEdtContext>(name, traversalOrder, basicblock);
    OcrTaskContextMapElem elem(name, taskcontext_sp);
    assert(taskcontext_sp);
    m_ocrTaskContextMap.insert(elem);
    return taskcontext_sp;
  }
}

struct TaskContextCompare {
  bool operator() (const OcrTaskContextPtr& lptr, const OcrTaskContextPtr& rptr) const {
    return lptr->getTraversalOrder() < rptr->getTraversalOrder();
  }
};

list<string> OcrObjectManager::getEdtTraversalOrder() const {
  vector<OcrTaskContextPtr> taskContextList;
  list<string> edtTraversalOrder;
  OcrTaskContextMap::const_iterator t = m_ocrTaskContextMap.begin();
  for( ; t != m_ocrTaskContextMap.end(); ++t) {
    taskContextList.push_back(t->second);
  }
  TaskContextCompare compareTaskContext;
  std::sort(taskContextList.begin(), taskContextList.end(), compareTaskContext);

  vector<OcrTaskContextPtr>::const_iterator tc = taskContextList.begin();
  for( ; tc != taskContextList.end(); ++tc) {
    string taskName = (*tc)->getTaskName();
    edtTraversalOrder.push_back(taskName);
  }

  return edtTraversalOrder;
}

list<OcrDbkContextPtr> OcrObjectManager::getOcrDbkContextList() const {
  return m_dbkSymbolTable.flatten();
}

OcrTaskContextPtr OcrObjectManager::getOcrTaskContext(string edtname) const {
  OcrTaskContextMap::const_iterator f = m_ocrTaskContextMap.find(edtname);
  assert(f != m_ocrTaskContextMap.end());
  return f->second;
}

OcrEvtContextPtr OcrObjectManager::getOcrEvtContext(string evtname) {
  OcrEvtObjectMap::iterator f = m_ocrEvtObjectMap.find(evtname);
  assert(f != m_ocrEvtObjectMap.end());
  return f->second;
}

const OcrTaskContextMap& OcrObjectManager::getOcrTaskContextMap() const {
  return m_ocrTaskContextMap;
}

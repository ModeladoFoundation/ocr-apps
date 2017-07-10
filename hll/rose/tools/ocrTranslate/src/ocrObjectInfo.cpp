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
OcrDbkContext::OcrDbkContext(std::string name, OcrDbkType dbkType, SgPragmaDeclaration* pragma)
  : m_name(name),
    m_type(dbkType),
    m_pragma(pragma) { }

OcrDbkContext::OcrDbkType OcrDbkContext::getDbkType() const {
  return m_type;
}

SgPragmaDeclaration* OcrDbkContext::get_pragma() const {
  return m_pragma;
}

std::string OcrDbkContext::get_name() const {
  return m_name;
}

OcrDbkContext::~OcrDbkContext() { }

/********************
 * OcrMemDbkContext *
 ********************/
OcrMemDbkContext::OcrMemDbkContext(std::string name, SgInitializedName* vdefn, list<SgStatement*> allocStmts, SgPragmaDeclaration* pragma)
  : OcrDbkContext(name, OcrDbkContext::DBK_mem, pragma),
    m_vdefn(vdefn),
    m_allocStmts(allocStmts) { }

SgInitializedName* OcrMemDbkContext::getSgInitializedName() const {
  return m_vdefn;
}

SgPragmaDeclaration* OcrMemDbkContext::get_pragma() const {
  return m_pragma;
}

// Function that computes the pointer type for the datablock pointer
SgType* OcrMemDbkContext::getDbkPtrType() {
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

SgDeclarationStatement* OcrMemDbkContext::get_declaration() const {
  SgDeclarationStatement* stmt = m_vdefn->get_definition();
  assert(stmt);
  return stmt;
}

list<SgStatement*> OcrMemDbkContext::get_allocStmts() const {
  return m_allocStmts;
}

string OcrMemDbkContext::get_name() const {
  return m_name;
}

string OcrMemDbkContext::str() const {
  ostringstream oss;
  string indent = " ";
  oss << "[DBK: " << m_name << "\n";
  oss << indent << "vdefn:" << StrUtil::SgInitializedName2Str(m_vdefn) << endl;
  oss << indent << "allocStmts:[\n" << StrUtil::stmtlist2str(m_allocStmts, indent) << "]";
  oss << "]";
  return oss.str();
}

OcrMemDbkContext::~OcrMemDbkContext() {
  // No dynamic memory here to cleanup
}

/********************
 * OcrArrDbkContext *
 ********************/
OcrArrDbkContext::OcrArrDbkContext(std::string name, SgInitializedName* arrInitName, SgPragmaDeclaration* pragma)
  : OcrDbkContext(name, OcrDbkContext::DBK_arr, pragma), m_arrInitName(arrInitName) { }

string OcrArrDbkContext::str() const {
  ostringstream oss;
  oss << "[OcrArrDbkContext name: " << m_name << ", ";
  oss << "ndim=" << SageInterface::getDimensionCount(m_arrInitName->get_type()) << ", ";
  SgType* arrElemType = SageInterface::getArrayElementType(m_arrInitName->get_type());
  oss << "type=" << AstDebug::astTypeName(arrElemType);
  oss << "]";
  return oss.str();
}

SgInitializedName* OcrArrDbkContext::getArrInitializedName() const {
  return m_arrInitName;
}

OcrArrDbkContext::~OcrArrDbkContext() { }

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
OcrEdtContext::OcrEdtContext(OcrTaskType type, string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
			     list<OcrDbkContextPtr> depDbks,
			     list<OcrEvtContextPtr> depEvts,
			     list<SgVarRefExp*> depElems,
			     OcrEvtContextPtr outputEvt,
			     SgBasicBlock* basicblock,
			     bool finishEdt)
  : OcrTaskContext(type, name, traversalOrder, sgpdecl),
    m_depDbks(depDbks),
    m_depEvts(depEvts),
    m_depElems(depElems),
    m_outputEvt(outputEvt),
    m_basicblock(basicblock),
    m_finishEdt(finishEdt) {
}

OcrEdtContext::OcrEdtContext(OcrTaskType type, string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
			     list<OcrDbkContextPtr> depDbks,
			     list<OcrEvtContextPtr> depEvts,
			     list<SgVarRefExp*> depElems,
			     OcrEvtContextPtr outputEvt)
  : OcrTaskContext(type, name, traversalOrder, sgpdecl),
    m_depDbks(depDbks),
    m_depEvts(depEvts),
    m_depElems(depElems),
    m_outputEvt(outputEvt),
    m_basicblock(NULL),
    m_finishEdt(false) {
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

unsigned int OcrEdtContext::getNumDepElems() const {
  return m_depElems.size();
}

unsigned int OcrEdtContext::getNumDepDbks() const {
  return m_depDbks.size();
}

unsigned int OcrEdtContext::getNumDepEvts() const {
  return m_depEvts.size();
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
  : OcrEdtContext(OcrTaskContext::e_TaskLoopIter, name, traversalOrder, sgpdecl, depDbks, depEvts, depElems, outputEvt),
    m_loopStmt(loopStmt) {
}

SgBasicBlock* OcrLoopIterEdtContext::getTaskBasicBlock() const {
  assert(false);
}

SgStatement* OcrLoopIterEdtContext::getLoopStmt() const {
  return m_loopStmt;
}

vector<SgStatement*> OcrLoopIterEdtContext::getLoopInitStmts() {
  vector<SgStatement*> loopInitStmts;
  if(SgForStatement* forStmt = isSgForStatement(m_loopStmt)) {
    loopInitStmts = forStmt->get_init_stmt();
  }
  return loopInitStmts;
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

/************************
 * OcrSpmdRegionContext *
 ************************/
OcrSpmdRegionContext::OcrSpmdRegionContext(string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
					   list<OcrDbkContextPtr> depDbks, list<OcrEvtContextPtr> depEvts,
					   list<SgVarRefExp*> depElems,
					   OcrEvtContextPtr outEvt,
					   SgBasicBlock* basicblock,
					   unsigned int ntasks)
  : OcrEdtContext(OcrTaskContext::e_TaskSpmdRegion, name, traversalOrder, sgpdecl,
		  depDbks, depEvts, depElems, outEvt, basicblock, false),
    m_ntasks(ntasks) {
}

unsigned int OcrSpmdRegionContext::getNTasks() const {
  return m_ntasks;
}

string OcrSpmdRegionContext::str() const {
  return "[OcrSpmdRegionContext]";
}

/**************************
 * OcrSpmdFinalizeContext *
 **************************/
OcrSpmdFinalizeContext::OcrSpmdFinalizeContext(string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
					       list<OcrEvtContextPtr> depEvts)
  : OcrTaskContext(OcrTaskContext::e_TaskSpmdFinalize, name, traversalOrder, sgpdecl),
    m_depEvts(depEvts) { }

list<OcrEvtContextPtr> OcrSpmdFinalizeContext::getDepEvts() const {
  return m_depEvts;
}

string OcrSpmdFinalizeContext::str() const {
  return "[OcrSpmdFinalizeContext]";
}

/**********************
 * OcrSpmdSendContext *
 **********************/
OcrSpmdSendContext::OcrSpmdSendContext(string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
				       OcrDbkContextPtr dbkToSend, list<OcrEvtContextPtr> depEvts,
				       OcrEvtContextPtr outEvt, SgFunctionCallExp* sendCallExp)
  : OcrTaskContext(OcrTaskContext::e_TaskSpmdSend, name, traversalOrder, sgpdecl),
    m_dbkToSend(dbkToSend),
    m_depEvts(depEvts),
    m_outEvt(outEvt),
    m_sendCallExp(sendCallExp) { }

OcrDbkContextPtr OcrSpmdSendContext::getDbkToSend() const {
  return m_dbkToSend;
}

list<OcrEvtContextPtr> OcrSpmdSendContext::getDepEvts() const {
  return m_depEvts;
}

OcrEvtContextPtr OcrSpmdSendContext::getOutputEvt() const {
  return m_outEvt;
}

SgFunctionCallExp* OcrSpmdSendContext::getSendCallExp() const {
  return m_sendCallExp;
}

string OcrSpmdSendContext::str() const {
  ostringstream oss;
  oss << "[OcrSpmdSendContext: ";
  oss << "dbkToSend " << m_dbkToSend->get_name() << "]\n";
  return oss.str();
}

OcrSpmdSendContext::~OcrSpmdSendContext() {
  // Nothing to cleanup here
}

/****************
 * MpiOpContext *
 ****************/
MpiOpContext::MpiOpContext(MpiOpContext::MpiOpType type, SgStatement* stmt)
  : m_opType(type),
    m_stmt(stmt) { }

MpiOpContext::MpiOpType MpiOpContext::getMpiOpType() const {
  return m_opType;
}

SgStatement* MpiOpContext::getMpiCallStmt() const {
  return m_stmt;
}

string MpiOpContext::str() const {
  ostringstream oss;
  oss << "[MpiOpContext : ";
  switch(m_opType) {
  case OP_INIT: {
    oss << "MPI_Init";
    break;
  }
  case OP_FINALIZE: {
    oss << "MPI_Finalize";
    break;
  }
  case OP_COMM_RANK: {
    oss << "MPI_Comm_rank";
    break;
  }
  case OP_COMM_SIZE: {
    oss << "MPI_Comm_size";
    break;
  }
  case OP_SEND: {
    oss << "MPI_Send";
    break;
  }
  case OP_RECV: {
    oss << "MPI_Recv";
    break;
  }
  case OP_REDUCE: {
    oss << "MPI_Reduce";
    break;
  }
  default:
    oss << "Unknown";
  }
  oss << "]";
  return oss.str();
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

OcrDbkContextPtr OcrObjectManager::getOcrDbkContext(string dbkName, SgScopeStatement* scope) {
  OcrDbkContextPtr dbkContext = m_dbkSymbolTable.getObjectPtr(dbkName, scope);
  return dbkContext;
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
    OcrTaskContextPtr taskcontext_sp(new OcrEdtContext(OcrTaskContext::e_TaskEdt, name, traversalOrder, sgpdecl, depDbks, depEvts, depElems, outputEvt,
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

OcrTaskContextPtr OcrObjectManager::registerOcrSpmdRegionEdt(string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
							     list<OcrDbkContextPtr> depDbks,
							     list<OcrEvtContextPtr> depEvts,
							     list<SgVarRefExp*> depElems,
							     OcrEvtContextPtr outEvt,
							     SgBasicBlock* basicblock,
							     unsigned int ntasks) {
  OcrTaskContextMap::iterator f = m_ocrTaskContextMap.find(name);
  if(f != m_ocrTaskContextMap.end()) {
    assert(f->second);
    cerr << "WARNING: TASK " << name << "is already registered\n";
    return f->second;
  }
  else {
    OcrTaskContextPtr taskcontext_sp(new OcrSpmdRegionContext(name, traversalOrder, sgpdecl, depDbks, depEvts, depElems, outEvt,
							      basicblock, ntasks));
    OcrTaskContextMapElem elem(name, taskcontext_sp);
    assert(taskcontext_sp);
    m_ocrTaskContextMap.insert(elem);
    return taskcontext_sp;
  }
}

OcrTaskContextPtr OcrObjectManager::registerOcrSpmdFinalizeEdt(string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
							       std::list<OcrEvtContextPtr> depEvts) {
  OcrTaskContextMap::iterator f = m_ocrTaskContextMap.find(name);
  if(f != m_ocrTaskContextMap.end()) {
    assert(f->second);
    cerr << "WARNING: TASK " << name << "is already registered\n";
    return f->second;
  }
  else {
    OcrTaskContextPtr taskcontext_sp = boost::make_shared<OcrSpmdFinalizeContext>(name, traversalOrder, sgpdecl, depEvts);
    OcrTaskContextMapElem elem(name, taskcontext_sp);
    m_ocrTaskContextMap.insert(elem);
    return taskcontext_sp;
  }
}

OcrTaskContextPtr OcrObjectManager::registerOcrSpmdSendContext(string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
							       OcrDbkContextPtr dbkToSend, list<OcrEvtContextPtr> depEvts,
							       OcrEvtContextPtr outEvt, SgFunctionCallExp* sendCallExp) {
  OcrTaskContextMap::iterator f = m_ocrTaskContextMap.find(name);
  if(f != m_ocrTaskContextMap.end()) {
    assert(f->second);
    cerr << "WARNING: TASK " << name << "is already registered\n";
    return f->second;
  }
  else {
    OcrTaskContextPtr taskcontext_sp = boost::make_shared<OcrSpmdSendContext>(name, traversalOrder, sgpdecl, dbkToSend,
									      depEvts, outEvt, sendCallExp);
    OcrTaskContextMapElem elem(name, taskcontext_sp);
    m_ocrTaskContextMap.insert(elem);
    return taskcontext_sp;
  }
}

bool OcrObjectManager::registerMpiOpContext(MpiOpContextPtr mpiOpContext) {
  m_mpiOpContextList.push_back(mpiOpContext);
  return true;
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

list<MpiOpContextPtr> OcrObjectManager::getMpiOpContextList() const {
  return m_mpiOpContextList;
}

bool OcrObjectManager::hasMpiOp() const {
  return !m_mpiOpContextList.empty();
}

/*
 * Author: Sriram Aananthakrishnan, 2016
 */

#include "sage3basic.h"
#include "ocrObjectInfo.h"
#include "boost/make_shared.hpp"
#include "logger.h"

using namespace std;

/********************
 * OcrObjectContext *
 ********************/
OcrObjectContext::OcrObjectContext() { }
OcrObjectContext::~OcrObjectContext() { }

/*****************
 * OcrDbkContext *
 *****************/
OcrDbkContext::OcrDbkContext(std::string name)
  : OcrObjectContext(),
    m_name(name) { }

OcrDbkContext::OcrDbkContext(std::string name, SgInitializedName* vdefn, list<SgStatement*> allocStmts)
  : OcrObjectContext(),
    m_name(name),
    m_vdefn(vdefn),
    m_allocStmts(allocStmts) { }

SgSymbol* OcrDbkContext::getSgSymbol() {
  SgSymbol* symbol = m_vdefn->search_for_symbol_from_symbol_table();
  assert(symbol);
  return symbol;
}

SgInitializedName* OcrDbkContext::getSgInitializedName() const {
  return m_vdefn;
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
  : OcrObjectContext(),
    m_name(name) { }

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

/*****************
 * OcrEdtContext *
 *****************/
OcrEdtContext::OcrEdtContext(std::string name, std::list<OcrEvtContextPtr> depEvts,
			     std::list<OcrDbkContextPtr> depDbks, OcrEvtContextPtr outputEvt,
			     std::list<SgVarRefExp*> depElems, std::list<SgStatement*> taskStatements,
			     list<string> dbksToDestroy, list<string> evtsToDestroy,
			     SgPragmaDeclaration* taskBegin, SgPragmaDeclaration* taskEnd)
  : m_name(name),
    m_depEvts(depEvts),
    m_depDbks(depDbks),
    m_outputEvt(outputEvt),
    m_depElems(depElems),
    m_statements(taskStatements),
    m_dbksToDestroy(dbksToDestroy),
    m_evtsToDestroy(evtsToDestroy),
    m_taskBegin(taskBegin),
    m_taskEnd(taskEnd) { }

string OcrEdtContext::get_name() const {
  return m_name;
}

list<SgStatement*> OcrEdtContext::getStmtList() const {
  return m_statements;
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
  assert(m_statements.size() > 0);
  SgNode* sgn = *m_statements.begin();
  return SageInterface::getEnclosingSourceFile(sgn);
}

SgPragmaDeclaration* OcrEdtContext::getTaskBeginPragma() const {
  return m_taskBegin;
}

SgPragmaDeclaration* OcrEdtContext::getTaskEndPragma() const {
  return m_taskEnd;
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

string OcrEdtContext::str() const {
  ostringstream oss;
  string indent = " ";
  oss << "[EDT: " << m_name << "\n";
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
  oss << indent << "dbksToDestroy: " << StrUtil::strlist2str(m_dbksToDestroy) << endl;
  oss << indent << "evtsToDestroy: " << StrUtil::strlist2str(m_evtsToDestroy) << endl;
  oss << indent << "taskStmts:[\n" << StrUtil::stmtlist2str(m_statements, indent) << "]";
  oss << "]";
  return oss.str();
}

OcrEdtContext::~OcrEdtContext() {
  // no cleanup required
}

/*************************
 * OcrShutdownEdtContext *
 *************************/
OcrShutdownEdtContext::OcrShutdownEdtContext(SgPragmaDeclaration* shutdownPragma, std::list<OcrEvtContextPtr> depEvts)
  : m_shutdownPragma(shutdownPragma),
    m_depEvts(depEvts) { }

std::list<OcrEvtContextPtr> OcrShutdownEdtContext::getDepEvts() const {
  return m_depEvts;
}

unsigned int OcrShutdownEdtContext::getNumDepEvts() {
  return m_depEvts.size();
}

SgPragmaDeclaration* OcrShutdownEdtContext::getPragma() const {
  return m_shutdownPragma;
}

string OcrShutdownEdtContext::str() const {
  ostringstream oss;
  string indent = " ";
  oss << "[Shutdown EDT\n";
  oss << indent << "depEvts:" << StrUtil::list2str<OcrEvtContext>(m_depEvts);
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

list<OcrDbkContextPtr> OcrObjectManager::getOcrDbkContextList(list<string> dbkNamesList) {
  Logger::Logger lg("OcrObjectManager::getOcrDbkContextList");
  list<OcrDbkContextPtr> ocrDbkContextList;
  list<string>::iterator l = dbkNamesList.begin();
  for( ; l != dbkNamesList.end(); ++l) {
    string dbkName = *l;
    Logger::debug(lg) << "dbkName=" << dbkName << endl;
    OcrDbkObjectMap::iterator found = m_ocrDbkObjectMap.find(dbkName);
    if(found != m_ocrDbkObjectMap.end()) {
      OcrDbkContextPtr dbkcontext_sp = boost::dynamic_pointer_cast<OcrDbkContext>(found->second);
      ocrDbkContextList.push_back(dbkcontext_sp);
    }
    // #SA:1/3/2017
    // Normally we would throw an error
    // We do not expect OcrObjects to be created after annotations
    // This function should be used under the assumption that all OcrObjects are already created
    // Since I do not have all types of OcrObject creation in place
    // I will use the else block to register the OcrObjects using their name
    // When all OcrObject creation is in place replace the else to throw error
    else {
      Logger::error(lg) << "dbkName: " << dbkName << " not found in m_ocrDbkObjectMap\n";
      assert(false);
      // OcrDbkContextPtr dbkcontext_sp = registerOcrDbk(dbkName);
      // ocrDbkContextList.push_back(dbkcontext_sp);
    }
  }
  return ocrDbkContextList;
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

OcrDbkContextPtr OcrObjectManager::registerOcrDbk(string dbkName,
						  SgInitializedName* vdefn,
						  list<SgStatement*> allocStmts) {
  Logger::Logger lg("OcrObjectManager::registerOcrDbk");
  OcrDbkObjectMap::iterator f = m_ocrDbkObjectMap.find(dbkName);
  OcrDbkContextPtr dbkcontext_sp;
  if(f != m_ocrDbkObjectMap.end()) {
    Logger::debug(lg) << "f=true\n";
    dbkcontext_sp = boost::dynamic_pointer_cast<OcrDbkContext>(f->second);
    assert(dbkcontext_sp);
  }
  else {
    Logger::debug(lg) << "f=false\n";
    dbkcontext_sp = boost::make_shared<OcrDbkContext>(dbkName, vdefn, allocStmts);
    OcrDbkObjectMapElem elem(dbkName, dbkcontext_sp);
    m_ocrDbkObjectMap.insert(elem);
  }
  return dbkcontext_sp;
}

bool OcrObjectManager::registerOcrEdtOrder(int taskOrder, string edtname) {
  pair<EdtPragmaOrderMap::iterator, bool> ret;
  EdtPragmaOrderMapElem elem(taskOrder, edtname);
  ret = m_edtPragmaOrderMap.insert(elem);
  // We can insert into the map only once
  // We should not see any conflicts when we are inserting
  assert(ret.second);
  return ret.second;
}

//! Check to see if the OcrObject is already registered
//! If already registered return its context
//! If not registered create a new context and insert in m_ocrObjectMap
OcrEdtContextPtr OcrObjectManager::registerOcrEdt(string edtName, list<OcrEvtContextPtr> depEvts,
						  list<OcrDbkContextPtr> depDbks,
						  OcrEvtContextPtr outputEvt,
						  list<SgVarRefExp*> depElems,
						  list<SgStatement*> taskStatements,
						  list<string> dbksToDestroy,
						  list<string> evtsToDestroy,
						  SgPragmaDeclaration* taskBegin,
						  SgPragmaDeclaration* taskEnd) {
  OcrEdtObjectMap::iterator f = m_ocrEdtObjectMap.find(edtName);
  OcrEdtContextPtr edtcontext_sp;
  if(f != m_ocrEdtObjectMap.end()) {
    edtcontext_sp = boost::dynamic_pointer_cast<OcrEdtContext>(f->second);
    assert(edtcontext_sp);
    return edtcontext_sp;
  }
  else {
    OcrEdtContextPtr edtcontext_sp(new OcrEdtContext(edtName, depEvts,
						     depDbks, outputEvt,
						     depElems, taskStatements,
						     dbksToDestroy, evtsToDestroy,
						     taskBegin, taskEnd));
    OcrEdtObjectMapElem elem(edtName, edtcontext_sp);
    m_ocrEdtObjectMap.insert(elem);
    assert(edtcontext_sp);
    return edtcontext_sp;
  }
}

bool OcrObjectManager::registerOcrShutdownEdt(SgPragmaDeclaration* shutdownPragma, list<OcrEvtContextPtr> depEvts) {
  OcrShutdownEdtContextPtr shutdownEdt = boost::make_shared<OcrShutdownEdtContext>(shutdownPragma, depEvts);
  m_ocrShutdownEdtList.push_back(shutdownEdt);
}

list<string> OcrObjectManager::getEdtTraversalOrder() const {
  list<string> edtTraversalOrder;
  EdtPragmaOrderMap::const_iterator e = m_edtPragmaOrderMap.begin();
  for( ; e != m_edtPragmaOrderMap.end(); ++e) {
    string edtname = e->second;
    edtTraversalOrder.push_back(edtname);
  }
  return edtTraversalOrder;
}

OcrEdtContextPtr OcrObjectManager::getOcrEdtContext(string edtname) const {
  OcrEdtObjectMap::const_iterator f = m_ocrEdtObjectMap.find(edtname);
  assert(f != m_ocrEdtObjectMap.end());
  return f->second;
}

OcrDbkContextPtr OcrObjectManager::getOcrDbkContext(string dbkname) {
  OcrDbkObjectMap::iterator f = m_ocrDbkObjectMap.find(dbkname);
  assert(f != m_ocrDbkObjectMap.end());
  return f->second;
}

OcrEvtContextPtr OcrObjectManager::getOcrEvtContext(string evtname) {
  OcrEvtObjectMap::iterator f = m_ocrEvtObjectMap.find(evtname);
  assert(f != m_ocrEvtObjectMap.end());
  return f->second;
}

const OcrEdtObjectMap& OcrObjectManager::getOcrEdtObjectMap() const {
  return m_ocrEdtObjectMap;
}

const OcrDbkObjectMap& OcrObjectManager::getOcrDbkObjectMap() const {
  return m_ocrDbkObjectMap;
}

const OcrShutdownEdtList& OcrObjectManager::getOcrShutdownEdtList() const {
  return m_ocrShutdownEdtList;
}

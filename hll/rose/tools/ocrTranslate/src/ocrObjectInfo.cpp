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

OcrDbkContext::OcrDbkContext(std::string name, SgInitializedName* vdefn, list<SgNode*> allocStmts)
  : OcrObjectContext(),
    m_name(name),
    m_vdefn(vdefn),
    m_allocStmts(allocStmts) { }

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
			     std::list<OcrDbkContextPtr> depDbks, std::list<OcrEvtContextPtr> evtsToSatisfy,
			     std::list<SgNode*> depElems, std::list<SgNode*> taskStatements)
  : m_name(name),
    m_depEvts(depEvts),
    m_depDbks(depDbks),
    m_evtsToSatisfy(evtsToSatisfy),
    m_depElems(depElems),
    m_statements(taskStatements) { }

string OcrEdtContext::get_name() const {
  return m_name;
}

SgSourceFile* OcrEdtContext::getSourceFile() {
  assert(m_statements.size() > 0);
  SgNode* sgn = *m_statements.begin();
  return SageInterface::getEnclosingSourceFile(sgn);
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
  oss << indent << "outEvts: " << StrUtil::list2str<OcrEvtContext>(m_evtsToSatisfy) << endl;
  oss << indent << "depElems: " << StrUtil::sgnlist2str(m_depElems) << endl;
  oss << indent << "taskStmts:[\n" << StrUtil::stmtlist2str(m_statements, indent) << "]";
  oss << "]";
  return oss.str();
}

OcrEdtContext::~OcrEdtContext() {
  // no cleanup required
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
						  list<SgNode*> allocStmts) {
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

//! Check to see if the OcrObject is already registered
//! If already registered return its context
//! If not registered create a new context and insert in m_ocrObjectMap
OcrEdtContextPtr OcrObjectManager::registerOcrEdt(string edtName, list<OcrEvtContextPtr> depEvts,
						  list<OcrDbkContextPtr> depDbks,
						  list<OcrEvtContextPtr> evtsToSatisfy,
						  list<SgNode*> depElems,
						  list<SgNode*> taskStatements) {
  OcrEdtObjectMap::iterator f = m_ocrEdtObjectMap.find(edtName);
  OcrEdtContextPtr edtcontext_sp;
  if(f != m_ocrEdtObjectMap.end()) {
    edtcontext_sp = boost::dynamic_pointer_cast<OcrEdtContext>(f->second);
  }
  else {
    edtcontext_sp = boost::make_shared<OcrEdtContext>(edtName, depEvts,
						      depDbks, evtsToSatisfy,
						      depElems, taskStatements);

    OcrEdtObjectMapElem elem(edtName, edtcontext_sp);
    m_ocrEdtObjectMap.insert(elem);
  }
  assert(edtcontext_sp);
  return edtcontext_sp;
}

const OcrEdtObjectMap& OcrObjectManager::getOcrEdtObjectMap() const {
  return m_ocrEdtObjectMap;
}

/*****************************************
 * Author: Sriram Aananthakrishnan, 2017 *
 *****************************************/

#include "sage3basic.h"
#include "ocrAstInfo.h"
#include "boost/make_shared.hpp"
#include "logger.h"
#include <string>

using namespace std;


/*********************
 * Utility Functions *
 *********************/
SgVariableSymbol* GetVariableSymbol(std::string name, SgScopeStatement* currentScope) {
  SgName vname(name);
  SgVariableSymbol* vsymbol = SageInterface::lookupVariableSymbolInParentScopes(vname, currentScope);
  assert(vsymbol);
  return vsymbol;
}

SgVariableSymbol* GetVariableSymbol(SgVariableDeclaration* vdecl, string vname) {
  SgVariableSymbol* v_sym = NULL;
  SgName name(vname);
  SgInitializedName* vsgn = vdecl->get_decl_item(name);
  assert(vsgn);
  SgScopeStatement* scope = vsgn->get_scope();
  if(scope->symbol_exists(name)) {
    v_sym = scope->lookup_variable_symbol(name);
  }
  else {
    // Else look for the symbol in declarations scope
    scope = vdecl->get_scope();
    v_sym = scope->lookup_variable_symbol(name);
  }
  if(!v_sym) cerr << "SgVariableSymbol lookup failed\n";
  assert(v_sym);
  return v_sym;
}

SgVariableSymbol* GetVariableSymbol(SgInitializedName* vsgn) {
  SgVariableSymbol* v_sym = NULL;
  SgScopeStatement* scope = vsgn->get_scope();
  SgName name = vsgn->get_name();
  if(scope->symbol_exists(name)) {
    v_sym = scope->lookup_variable_symbol(name);
  }
  else {
    // Else look for the symbol in declarations scope
    SgDeclarationStatement* vdecl = vsgn->get_declaration();
    scope = vdecl->get_scope();
    v_sym = scope->lookup_variable_symbol(name);
  }
  if(!v_sym) cerr << "SgVariableSymbol lookup failed\n";
  assert(v_sym);
  return v_sym;

}

/**************
 * DbkAstInfo *
 **************/
DbkAstInfo::DbkAstInfo(string dbkname, string guidName, string ptrName)
  : m_dbkname(dbkname),
    m_dbkGuidName(guidName),
    m_dbkPtrName(ptrName) {
}

string DbkAstInfo::getDbkGuidName() const {
  return m_dbkGuidName;
}

string DbkAstInfo::getDbkPtrName() const {
  return m_dbkPtrName;
}

string DbkAstInfo::str() const {
  ostringstream oss;
  oss << "[ocrGuid:" << m_dbkGuidName << ", ";
  oss << "ptr: " << m_dbkPtrName << "]";
  return oss.str();
}

/*****************
 * ArrDbkAstInfo *
 *****************/
ArrDbkAstInfo::ArrDbkAstInfo(std::string dbkname, std::string guidName, std::string ptrName)
  : DbkAstInfo(dbkname, guidName, ptrName) { }
ArrDbkAstInfo::ArrDbkAstInfo(std::string dbkname, std::string guidName, std::string ptrName,
			     SgClassDeclaration* dbkStructDecl, SgType* dbkStructType, std::string arrPtrName)
  : DbkAstInfo(dbkname, guidName, ptrName),
    m_dbkStructDecl(dbkStructDecl),
    m_dbkStructType(dbkStructType),
    m_arrPtrName(arrPtrName) { }
// Get functions
SgClassDeclaration* ArrDbkAstInfo::getDbkStructDecl() const {
  return m_dbkStructDecl;
}

SgType* ArrDbkAstInfo::getDbkStructType() const {
  return m_dbkStructType;
}

std::string ArrDbkAstInfo::getArrPtrName() const {
  return m_arrPtrName;
}

// Set functions
void ArrDbkAstInfo::setDbkStructDecl(SgClassDeclaration* dbkStructDecl) {
  m_dbkStructDecl = dbkStructDecl;
}

void ArrDbkAstInfo::setDbkStructType(SgType* dbkStructType) {
  m_dbkStructType = dbkStructType;
}

void ArrDbkAstInfo::setDbkArrPtrName(std::string arrPtrName) {
  m_arrPtrName = arrPtrName;
}

std::string ArrDbkAstInfo::str() const {
  ostringstream oss;
  oss << "[ArrDbkAstInfo: " << m_dbkname << ", " << m_dbkGuidName << ", " << m_dbkPtrName << ", ";
  oss << AstDebug::astTypeName(m_dbkStructType) << ", " << m_arrPtrName << "]";
  return oss.str();
}

ArrDbkAstInfo::~ArrDbkAstInfo() { }

/**************
 * EvtAstInfo *
 **************/
EvtAstInfo::EvtAstInfo(string evtname, string evtGuidName)
  : m_evtname(evtname), m_evtGuidName(evtGuidName) {
}

string EvtAstInfo::getEvtGuidName() const {
  return m_evtGuidName;
}

string EvtAstInfo::str() const {
  ostringstream oss;
  oss << "[" << m_evtname << ", " << m_evtGuidName << "]";
  return oss.str();
}

/************************
 * TaskOutliningAstInfo *
 ************************/
TaskOutliningAstInfo::TaskOutliningAstInfo(string name) :
  m_taskName(name),
  m_depElemStructDecl(NULL),
  m_depElemTypedefDecl(NULL),
  m_depElemBaseType(NULL),
  m_depElemTypedefType(NULL),
  m_edtDecl(NULL),
  m_hasDepElems(false) { }

void TaskOutliningAstInfo::setDepElemStructDecl(SgClassDeclaration* depElemStructDecl) {
  m_hasDepElems = true;
  m_depElemStructDecl = depElemStructDecl;
}

void TaskOutliningAstInfo::setDepElemTypedefDecl(SgTypedefDeclaration* depElemTypedefDecl) {
  m_depElemTypedefDecl = depElemTypedefDecl;
}

void TaskOutliningAstInfo::setDepElemBaseType(SgType* depElemBaseType) {
  m_depElemBaseType = depElemBaseType;
}

void TaskOutliningAstInfo::setDepElemTypedefType(SgType* depElemTypedefType) {
  m_depElemTypedefType = depElemTypedefType;
}

void TaskOutliningAstInfo::setTaskFuncDecl(SgFunctionDeclaration* edtDecl) {
  m_edtDecl = edtDecl;
}

SgClassDeclaration* TaskOutliningAstInfo::getDepElemStructDecl() const {
  return m_depElemStructDecl;
}

SgTypedefDeclaration* TaskOutliningAstInfo::getDepElemTypedefDecl() const {
  return m_depElemTypedefDecl;
}

SgClassDefinition* TaskOutliningAstInfo::getDepElemStructDefn() const {
  SgClassDefinition* structDefn = m_depElemStructDecl->get_definition();
  assert(structDefn);
  return structDefn;
}

SgType* TaskOutliningAstInfo::getDepElemBaseType() const {
  return m_depElemBaseType;
}

SgType* TaskOutliningAstInfo::getDepElemTypedefType() const {
  return m_depElemTypedefType;
}

SgFunctionDeclaration* TaskOutliningAstInfo::getTaskFuncDecl() const {
  return m_edtDecl;
}

bool TaskOutliningAstInfo::hasDepElems() const {
  return m_hasDepElems;
}

string TaskOutliningAstInfo::str() const {
  ostringstream oss;
  oss << "[TaskOutliningAstInfo : " << m_taskName << "\n";
  oss << "  " << AstDebug::astToString(m_edtDecl) << "\n]\n";
  return oss.str();
}

TaskOutliningAstInfo::~TaskOutliningAstInfo() {
  // Do not delete the SgNode pointers
  // Nothing to cleanup here otherwise
}

/***************
 * TaskAstInfo *
 ***************/
TaskAstInfo::TaskAstInfo(string name) : m_taskName(name) {
  m_outliningInfo = boost::make_shared<TaskOutliningAstInfo>(m_taskName);
}

TaskAstInfo::TaskType TaskAstInfo::getTaskType() const {
  return m_taskType;
}

TaskOutliningAstInfoPtr TaskAstInfo::getTaskOutliningAstInfo() const {
  return m_outliningInfo;
}

// Get Functions for OutliningInfo
SgClassDeclaration* TaskAstInfo::getDepElemStructDecl() const {
  return m_outliningInfo->getDepElemStructDecl();
}

SgTypedefDeclaration* TaskAstInfo::getDepElemTypedefDecl() const {
  return m_outliningInfo->getDepElemTypedefDecl();
}

SgClassDefinition* TaskAstInfo::getDepElemStructDefn() const {
  return m_outliningInfo->getDepElemStructDefn();
}

SgType* TaskAstInfo::getDepElemBaseType() const {
  return m_outliningInfo->getDepElemBaseType();
}

SgType* TaskAstInfo::getDepElemTypedefType() const {
  return m_outliningInfo->getDepElemTypedefType();
}

SgFunctionDeclaration* TaskAstInfo::getTaskFuncDecl() const {
  return m_outliningInfo->getTaskFuncDecl();
}

bool TaskAstInfo::hasDepElems() const {
  return m_outliningInfo->hasDepElems();
}

// Set Functions for Outlining Info
void TaskAstInfo::setDepElemStructDecl(SgClassDeclaration* depElemStructDecl) {
  m_outliningInfo->setDepElemStructDecl(depElemStructDecl);
}

void TaskAstInfo::setDepElemTypedefDecl(SgTypedefDeclaration* depElemTypedefDecl) {
  m_outliningInfo->setDepElemTypedefDecl(depElemTypedefDecl);
}

void TaskAstInfo::setDepElemBaseType(SgType* depElemBaseType) {
  m_outliningInfo->setDepElemBaseType(depElemBaseType);
}

void TaskAstInfo::setDepElemTypedefType(SgType* depElemTypedefType) {
  m_outliningInfo->setDepElemTypedefType(depElemTypedefType);
}

void TaskAstInfo::setTaskFuncDecl(SgFunctionDeclaration* edtDecl) {
  m_outliningInfo->setTaskFuncDecl(edtDecl);
}

TaskAstInfo::~TaskAstInfo() {
  // Nothing to destroy here
}

/**************
 * EdtAstInfo *
 **************/
EdtAstInfo::EdtAstInfo(string edtname) : TaskAstInfo(edtname), m_ndeps(0) { }

string EdtAstInfo::getEdtTemplateGuidName() const {
  return m_edtTemplGuidName;
}

string EdtAstInfo::getDepElemStructName() const {
  return m_depElemStructName;
}

string EdtAstInfo::getEdtGuidName() const {
  return m_edtGuidName;
}

string EdtAstInfo::getDepElemSizeVarName() const {
  return m_depElemSizeVarName;
}

int EdtAstInfo::getNDeps() const {
  return m_ndeps;
}

void EdtAstInfo::setTemplGuidName(string edtTemplGuidName) {
  m_edtTemplGuidName = edtTemplGuidName;
}
void EdtAstInfo::setDepElemStructName(string depElemStructName) {
  m_depElemStructName = depElemStructName;
}

void EdtAstInfo::setEdtGuidName(string edtGuidName) {
  m_edtGuidName = edtGuidName;
}

void EdtAstInfo::setDepElemSizeVarName(string depElemSizeVarName) {
  m_depElemSizeVarName = depElemSizeVarName;
}

void EdtAstInfo::setNDeps(int ndeps) {
  m_ndeps = ndeps;
}


string EdtAstInfo::str() const {
  ostringstream oss;
  oss << "[EdtAstInfo: " << m_edtTemplGuidName << ", "
      << m_depElemStructName << ", " << m_edtGuidName << "]";
  return oss.str();
}

EdtAstInfo::~EdtAstInfo() { }

/*****************************
 * LoopControlEdtAstInfo *
 *****************************/
LoopControlEdtAstInfo::LoopControlEdtAstInfo(string name) : EdtAstInfo(name) { }

SgBasicBlock* LoopControlEdtAstInfo::getBasicBlock() const {
  return m_basicblock;
}

string LoopControlEdtAstInfo::getCompEvtGuidName() const {
  return m_compEvtGuidName;
}

string LoopControlEdtAstInfo::getDepElemCompEvtGuidName() const {
  return m_depElemCompEvtGuidName;
}

void LoopControlEdtAstInfo::setBasicBlock(SgBasicBlock* basicblock) {
  m_basicblock = basicblock;
}

void LoopControlEdtAstInfo::setCompEvtGuidName(string compEvtGuidName) {
  m_compEvtGuidName = compEvtGuidName;
}

void LoopControlEdtAstInfo::setDepElemCompEvtGuidName(string depElemCompEvtGuidName) {
  m_depElemCompEvtGuidName = depElemCompEvtGuidName;
}

void LoopControlEdtAstInfo::setLoopControlIfBasicBlock(SgBasicBlock* ifBasicBlock) {
  m_ifBasicBlock = ifBasicBlock;
}

void LoopControlEdtAstInfo::setLoopControlElseBasicBlock(SgBasicBlock* elseBasicBlock) {
  m_elseBasicBlock = elseBasicBlock;
}

void LoopControlEdtAstInfo::setLoopControlIfStmt(SgIfStmt* loopControlIfStmt) {
  m_ifStmt = loopControlIfStmt;
}

std::string LoopControlEdtAstInfo::str() const {
  ostringstream oss;
  oss << "[LoopControlEdtAstInfo : " << m_taskName;
  oss << "  " << AstDebug::astToString(m_basicblock) << "\n";
  oss << m_compEvtGuidName << "]";
  return oss.str();
}

LoopControlEdtAstInfo::~LoopControlEdtAstInfo() {
  // Nothing to cleanup here
}

/************************
 * SpmdRegionEdtAstInfo *
 ************************/
SpmdRegionEdtAstInfo::SpmdRegionEdtAstInfo(string name) : EdtAstInfo(name) { }

string SpmdRegionEdtAstInfo::getDbkAccessTypeArrName() const {
  return m_dbkAccessTypeArrName;
}

string SpmdRegionEdtAstInfo::getCountVarName() const {
  return m_countVarName;
}

void SpmdRegionEdtAstInfo::setDbkAccessTypeArrName(string dbkAccessTypeArrName) {
  m_dbkAccessTypeArrName = dbkAccessTypeArrName;
}

void SpmdRegionEdtAstInfo::setCountVarName(string countVarName) {
  m_countVarName = countVarName;
}

string SpmdRegionEdtAstInfo::str() const {
  return "[SpmdRegionEdtAstInfo]";
}

SpmdRegionEdtAstInfo::~SpmdRegionEdtAstInfo() {
  // no cleanup required here
}

/**************************
 * SpmdFinalizeEdtAstInfo *
 **************************/
SpmdFinalizeEdtAstInfo::SpmdFinalizeEdtAstInfo(string name) : EdtAstInfo(name) { }

string SpmdFinalizeEdtAstInfo::getDepEvtGuidName() const {
  return m_depEvtGuidName;
}

void SpmdFinalizeEdtAstInfo::setDepEvtGuidName(string depEvtGuidName) {
  m_depEvtGuidName = depEvtGuidName;
}

string SpmdFinalizeEdtAstInfo::str() const {
  return "[SpmdFinalizeEdtAstInfo]";
}

SpmdFinalizeEdtAstInfo::~SpmdFinalizeEdtAstInfo() {
  // no cleanup here
}

/******************
 * AstInfoManager *
 ******************/
AstInfoManager::AstInfoManager() { }

void AstInfoManager::regDbkAstInfo(string dbkName, DbkAstInfoPtr dbkAstInfo, SgScopeStatement* scope) {
  m_dbkAstInfoMap.insertObjectPtr(dbkName, dbkAstInfo, scope);
}

bool AstInfoManager::findEdtAstInfo(string edtName) const {
  TaskAstInfoMap::const_iterator f = m_taskAstInfoMap.find(edtName);
  return f != m_taskAstInfoMap.end();
}

EdtAstInfoPtr AstInfoManager::regEdtAstInfo(string edtName) {
  EdtAstInfoPtr edtAstInfoPtr = boost::make_shared<EdtAstInfo>(edtName);
  TaskAstInfoMapElem elem(edtName, edtAstInfoPtr);
  m_taskAstInfoMap.insert(elem);
  return edtAstInfoPtr;
}

EvtAstInfoPtr AstInfoManager::regEvtAstInfo(string evtname, string evtGuidName, SgScopeStatement* scope) {
  EvtAstInfoPtr evtAstInfo = boost::make_shared<EvtAstInfo>(evtname, evtGuidName);
  m_evtAstInfoMap.insertObjectPtr(evtname, evtAstInfo, scope);
  return evtAstInfo;
}

LoopControlEdtAstInfoPtr AstInfoManager::regLoopControlEdtAstInfo(std::string edtName) {
  LoopControlEdtAstInfoPtr loopControlEdtAstInfo = boost::make_shared<LoopControlEdtAstInfo>(edtName);
  TaskAstInfoMapElem elem(edtName, loopControlEdtAstInfo);
  m_taskAstInfoMap.insert(elem);
  return loopControlEdtAstInfo;
}

SpmdRegionEdtAstInfoPtr AstInfoManager::regSpmdRegionEdtAstInfo(std::string edtName) {
  SpmdRegionEdtAstInfoPtr spmdRegionEdtAstInfo = boost::make_shared<SpmdRegionEdtAstInfo>(edtName);
  TaskAstInfoMapElem elem(edtName, spmdRegionEdtAstInfo);
  m_taskAstInfoMap.insert(elem);
  return spmdRegionEdtAstInfo;
}

DbkAstInfoPtr AstInfoManager::getDbkAstInfo(string dbkname, SgScopeStatement* scope) {
  DbkAstInfoPtr dbkAstInfo = m_dbkAstInfoMap.getObjectPtr(dbkname, scope);
  assert(dbkAstInfo);
  return dbkAstInfo;
}

EdtAstInfoPtr AstInfoManager::getEdtAstInfo(string edtname) {
  TaskAstInfoMap::iterator f = m_taskAstInfoMap.find(edtname);
  assert(f != m_taskAstInfoMap.end());
  EdtAstInfoPtr edtAstInfo = boost::dynamic_pointer_cast<EdtAstInfo>(f->second);
  assert(edtAstInfo);
  return edtAstInfo;
}

SpmdRegionEdtAstInfoPtr AstInfoManager::getSpmdRegionEdtAstInfo(string edtname) {
  TaskAstInfoMap::iterator f = m_taskAstInfoMap.find(edtname);
  assert(f != m_taskAstInfoMap.end());
  SpmdRegionEdtAstInfoPtr spmdRegionEdtAstInfo = boost::dynamic_pointer_cast<SpmdRegionEdtAstInfo>(f->second);
  assert(spmdRegionEdtAstInfo);
  return spmdRegionEdtAstInfo;
}

/*!
 * \brief Get the Event's AST info from the symbolTable map
 */
EvtAstInfoPtr AstInfoManager::getEvtAstInfo(string evtname, SgScopeStatement* scope) {
  // getObjectPtr terminates when not found in the map
  EvtAstInfoPtr evtAstInfo = m_evtAstInfoMap.getObjectPtr(evtname, scope);
  return evtAstInfo;
}

string AstInfoManager::EvtAstInfoMap2Str() const {
  return m_evtAstInfoMap.str();
}

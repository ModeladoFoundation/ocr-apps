/*
 * Author: Sriram Aananthakrishnan, 2017 *
 */

#include "sage3basic.h"
#include "ocrTranslateEngine.h"
#include "ocrAstBuilder.h"
#include "boost/make_shared.hpp"
#include "logger.h"

using namespace std;

/**********************
 * TranslateException *
 **********************/
TranslateException::TranslateException(std::string what) : m_what(what) { }

const char* TranslateException::what() const throw() {
  return m_what.c_str();
}

TranslateException::~TranslateException() throw() {
}

/*****************
 * OcrDbkAstInfo *
 *****************/
OcrDbkAstInfo::OcrDbkAstInfo(string dbkname, SgVariableSymbol* ocrGuidSymbol,
			     SgVariableSymbol* ptrSymbol)
  : m_dbkname(dbkname),
    m_ocrGuidSymbol(ocrGuidSymbol),
    m_ptrSymbol(ptrSymbol) { }

SgVariableSymbol* OcrDbkAstInfo::getOcrGuidSymbol() const {
  return m_ocrGuidSymbol;
}

SgVariableSymbol* OcrDbkAstInfo::getPtrSymbol() const {
  return m_ptrSymbol;
}

string OcrDbkAstInfo::str() const {
  ostringstream oss;
  oss << "[ocrGuid:" << m_ocrGuidSymbol->get_name().getString() << ", ";
  oss << "ptr: " << m_ptrSymbol->get_name().getString() << "]";
  return oss.str();
}

/*****************
 * OcrEdtAstInfo *
 *****************/
OcrEdtAstInfo::OcrEdtAstInfo(string edtname, SgFunctionDeclaration* edtDecl)
  : m_edtname(edtname), m_edtDecl(edtDecl) { }

SgFunctionDeclaration* OcrEdtAstInfo::getEdtFunctionDeclaration() const {
  return m_edtDecl;
}

SgClassDeclaration* OcrEdtAstInfo::getDepElemStructDecl() const {
  return m_depElemStructDecl;
}

SgType* OcrEdtAstInfo::getDepElemTypedefType() const {
  return m_depElemTypedefType;
}

SgType* OcrEdtAstInfo::getDepElemBaseType() const {
  return m_depElemBaseType;
}

SgVariableSymbol* OcrEdtAstInfo::getEdtTemplateGuid() const {
  return m_edtTemplateGuid;
}

SgVariableSymbol* OcrEdtAstInfo::getDepElemStructSymbol() const {
  return m_depElemStructSymbol;
}

SgVariableSymbol* OcrEdtAstInfo::getEdtGuid() const {
  return m_edtGuid;
}

void OcrEdtAstInfo::setDepElemTypedefType(SgType* depElemType) {
  m_depElemTypedefType = depElemType;
}

void OcrEdtAstInfo::setDepElemBaseType(SgType* depElemBaseType) {
  m_depElemBaseType = depElemBaseType;
}

void OcrEdtAstInfo::setEdtTemplateGuid(SgVariableSymbol* edtTemplateGuid) {
  m_edtTemplateGuid = edtTemplateGuid;
}

void OcrEdtAstInfo::setDepElemStructSymbol(SgVariableSymbol* depElemStructSymbol) {
  m_depElemStructSymbol = depElemStructSymbol;
}

void OcrEdtAstInfo::setDepElemStructDecl(SgClassDeclaration* depElemStructDecl) {
  m_depElemStructDecl = depElemStructDecl;
}

void OcrEdtAstInfo::setEdtGuid(SgVariableSymbol* edtGuid) {
  m_edtGuid = edtGuid;
}

string OcrEdtAstInfo::str() const {
  return "OcrEdtAstInfo";
}

/*****************
 * OcrEvtAstInfo *
 *****************/
OcrEvtAstInfo::OcrEvtAstInfo(string evtname, SgVariableSymbol* evtGuid)
  : m_evtname(evtname),
    m_evtGuid(evtGuid) {
}

SgVariableSymbol* OcrEvtAstInfo::getEvtGuid() const {
  return m_evtGuid;
}

/**********************
 * OcrGuidSymbolTable *
 **********************/
OcrGuidSymbolTable::OcrGuidSymbolTable() { }

bool OcrGuidSymbolTable::insert(string ocrObjectName, SgVariableSymbol* varSymbol) {
  OcrGuidSymbolMap::iterator f = m_ocrGuidSymbolMap.find(ocrObjectName);
  assert(f == m_ocrGuidSymbolMap.end());
  OcrGuidSymbolMapElem elem(ocrObjectName, varSymbol);
  m_ocrGuidSymbolMap.insert(elem);
  return true;
}

SgVariableSymbol* OcrGuidSymbolTable::getGuidSymbol(string ocrObjectName) {
  OcrGuidSymbolMap::iterator f = m_ocrGuidSymbolMap.find(ocrObjectName);
  assert(f != m_ocrGuidSymbolMap.end());
  return f->second;
}

string OcrGuidSymbolTable::str() const {
  ostringstream oss;
  OcrGuidSymbolMap::const_iterator melem = m_ocrGuidSymbolMap.begin();
  string indent = " ";
  for( ; melem != m_ocrGuidSymbolMap.end(); ++melem) {
    oss << indent << "[" << melem->first << ", " << melem->second->get_name() << "]\n";
  }
  return oss.str();
}

/*********************
 * OcrAstInfoManager *
 *********************/
OcrAstInfoManager::OcrAstInfoManager() { }

bool OcrAstInfoManager::regOcrDbkAstInfo(string dbkname, SgVariableSymbol* ocrGuidSymbol, SgVariableSymbol* ptrSymbol) {
  pair<OcrDbkAstInfoMap::iterator, bool> ret;
  OcrDbkAstInfoPtr dbkAstInfoPtr = boost::make_shared<OcrDbkAstInfo>(dbkname, ocrGuidSymbol, ptrSymbol);
  OcrDbkAstInfoMapElem elem(dbkname, dbkAstInfoPtr);
  ret = m_ocrDbkAstInfoMap.insert(elem);
  return ret.second;
}

bool OcrAstInfoManager::regOcrEdtAstInfo(string edtName, SgFunctionDeclaration* edtDecl) {
  pair<OcrEdtAstInfoMap::iterator, bool> ret;
  OcrEdtAstInfoPtr edtAstInfoPtr = boost::make_shared<OcrEdtAstInfo>(edtName, edtDecl);
  OcrEdtAstInfoMapElem elem(edtName, edtAstInfoPtr);
  ret = m_ocrEdtAstInfoMap.insert(elem);
  return ret.second;
}

bool OcrAstInfoManager::regOcrEvtAstInfo(string evtname, SgVariableSymbol* evtGuid) {
  pair<OcrEvtAstInfoMap::iterator, bool> ret;
  OcrEvtAstInfoPtr evtAstInfoPtr = boost::make_shared<OcrEvtAstInfo>(evtname, evtGuid);
  OcrEvtAstInfoMapElem elem(evtname, evtAstInfoPtr);
  ret = m_ocrEvtAstInfoMap.insert(elem);
  return ret.second;
}

OcrDbkAstInfoPtr OcrAstInfoManager::getOcrDbkAstInfo(string dbkname) {
  OcrDbkAstInfoMap::iterator f = m_ocrDbkAstInfoMap.find(dbkname);
  assert(f != m_ocrDbkAstInfoMap.end());
  return f->second;
}

OcrEdtAstInfoPtr OcrAstInfoManager::getOcrEdtAstInfo(string edtname) {
  OcrEdtAstInfoMap::iterator f = m_ocrEdtAstInfoMap.find(edtname);
  assert(f != m_ocrEdtAstInfoMap.end());
  return f->second;
}

OcrEvtAstInfoPtr OcrAstInfoManager::getOcrEvtAstInfo(string evtname) {
  OcrEvtAstInfoMap::iterator f = m_ocrEvtAstInfoMap.find(evtname);
  if(f == m_ocrEvtAstInfoMap.end()) {
    cerr << "Cannot find OCR Event " << evtname << endl;
    cerr << ocrEvtAstInfoMap2Str() << endl;
    assert(f != m_ocrEvtAstInfoMap.end());
  }
  return f->second;
}

// We do not know when or where the symbol table for a function will be created
// We will register on-the-fly as and when needed
// Look for the mapped value and if not found, create and return the entry
OcrGuidSymbolTablePtr OcrAstInfoManager::getOcrGuidSymbolTable(SgScopeStatement* scope) {
  ScopeGuidSymbolMap::iterator f = m_scopeGuidSymbolMap.find(scope);
  if(f == m_scopeGuidSymbolMap.end()) {
    OcrGuidSymbolTablePtr symbolTable = boost::make_shared<OcrGuidSymbolTable>();
    ScopeGuidSymbolMapElem elem(scope, symbolTable);
    m_scopeGuidSymbolMap.insert(elem);
    return elem.second;
  }
  return f->second;
}

string OcrAstInfoManager::ocrEvtAstInfoMap2Str() const {
  ostringstream oss;
  OcrEvtAstInfoMap::const_iterator i = m_ocrEvtAstInfoMap.begin();
  OcrEvtAstInfoMap::const_iterator e = m_ocrEvtAstInfoMap.end();
  oss << "[";
  while(i != e) {
    oss << i->first;
    if(i != e) {
      oss << ", ";
    }
    ++i;
  }
  oss << "]";
  return oss.str();
}
/************************
 * DepElemVarRefExpPass *
 ************************/
DepElemVarRefExpPass::DepElemVarRefExpPass(SgScopeStatement* scope, SgName depElemStructName, list<SgVarRefExp*> depElems)
  : m_scope(scope),
    m_depElemStructName(depElemStructName) {
  list<SgVarRefExp*>::iterator l = depElems.begin();
  for( ; l != depElems.end(); ++l) {
    SgVarRefExp* item = *l;
    m_varSymbolSet.insert(item->get_symbol());
  }
}

void DepElemVarRefExpPass::visit(SgNode* sgn) {
  if(SgVarRefExp* vref = isSgVarRefExp(sgn)) {
    SgVariableSymbol* symbol = vref->get_symbol();
    if(m_varSymbolSet.find(symbol) != m_varSymbolSet.end()) {
      SgName vname = symbol->get_name();
      SgVarRefExp* rexp = SageBuilder::buildVarRefExp(vname, m_scope);
      SgVarRefExp* lexp = SageBuilder::buildVarRefExp(m_depElemStructName, m_scope);
      SgArrowExp* arrowExp = SageBuilder::buildArrowExp(lexp, rexp);
      // When depElem is declared in basicblock, a symbol
      // for depElem is inserted in to the basic block
      // When building the arrow exp, we build the rhs using the vname
      // The variable vname may not have a symbol in the basic block
      // This later shows up as a Warning in AST consistency checks
      // To avoid this, the symbol for var is inserted into the symbol table.
      // This seems to fix the AST consistency check
      // However, I need to check if this is the way for building dot and arrow
      // expressions
      // m_scope->get_symbol_table()->insert(vname, rexp->get_symbol());
      if(!m_scope->get_symbol_table()->exists(symbol->get_name())) {
	m_scope->get_symbol_table()->insert(symbol->get_name(), rexp->get_symbol());
      }
      // Now replace the SgVarRefExp with SgArrowExp
      SageInterface::replaceExpression(vref, arrowExp, false);
    }
  }
}

void DepElemVarRefExpPass::atTraversalEnd() {
}

/*********************
 * Utility Functions *
 *********************/

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

/*****************
 * OcrTranslator *
 *****************/
OcrTranslator::OcrTranslator(SgProject* project, const OcrObjectManager& ocrObjectManager)
  : m_project(project),
    m_ocrObjectManager(ocrObjectManager) { }

void OcrTranslator::insertOcrHeaderFiles() {
  SgFilePtrList files = m_project->get_files();
  SgFilePtrList::iterator f = files.begin();
  for( ; f != files.end(); ++f) {
    if(SgSourceFile* source = isSgSourceFile(*f)) {
      SgGlobal* global = source->get_globalScope();
      SageInterface::insertHeader(source, "ocr.h", false, true);
    }
  }
}

void OcrTranslator::translateDbk(string dbkName, OcrDbkContextPtr dbkContext) {
  Logger::Logger lg("OcrTranslator::translateDbk");
  // Mark the statements to be removed at the end of this function
  set<SgStatement*> stmtsToRemove;
  SgInitializedName* varInitializedName = dbkContext->getSgInitializedName();
  SgScopeStatement* scope = SageInterface::getEnclosingScope(dbkContext->get_pragma());
  SgName varName = varInitializedName->get_name();
  SgName ocrGuidName(dbkName);
  SgType* varType = varInitializedName->get_type();
  SgType* varDbkType = AstBuilder::buildOcrDbkType(varType, scope);
  SgVariableDeclaration* varDbkDecl = AstBuilder::buildOcrDbkVarDecl(varName, varDbkType, scope);
  SgVariableDeclaration* varDbkGuid = AstBuilder::buildOcrDbkGuid(ocrGuidName.getString(), scope);
  // AST Modifications
  // stmt is the declaration of the datablock variable
  // We don't need the declaration anymore
  // Remove the stmt and insert datablock declaration instead
  SgStatement* stmt = SageInterface::getEnclosingStatement(varInitializedName);
  // First get the anchor point where the stmt will be inserted
  SageInterface::insertStatementBefore(stmt, varDbkDecl, true);
  SageInterface::insertStatementBefore(stmt, varDbkGuid, true);
  stmtsToRemove.insert(stmt);
  // Replace each alloc stmt with ocrDbCreate
  list<SgStatement*> allocStmts = dbkContext->get_allocStmts();
  list<SgStatement*>::iterator s = allocStmts.begin();
  for( ; s != allocStmts.end(); ++s) {
    SgExprStatement* dbCreateStmt = AstBuilder::buildOcrDbCreateFuncCallExp(ocrGuidName, varName, scope, *s);
    SgStatement* stmtAfterAllocStmt = SageInterface::getNextStatement(*s);
    SageInterface::insertStatementBefore(stmtAfterAllocStmt, dbCreateStmt, true);
    // Mark the statement for removal
    stmtsToRemove.insert(*s);
  }
  // Now remove all the statements marked for removal
  set<SgStatement*>::iterator st = stmtsToRemove.begin();
  for( ; st != stmtsToRemove.end(); ++st) {
    SageInterface::removeStatement(*st);
  }
  // Register the symbols with the Translation manager
  SgVariableSymbol* ocrGuidSymbol = GetVariableSymbol(varDbkGuid, ocrGuidName);
  SgVariableSymbol* dbkPtrSymbol = GetVariableSymbol(varDbkDecl, varName);
  assert(ocrGuidSymbol);
  assert(dbkPtrSymbol);
  // Bookkeeping
  m_ocrAstInfoManager.regOcrDbkAstInfo(dbkName, ocrGuidSymbol, dbkPtrSymbol);
  OcrGuidSymbolTablePtr guidSymbolTable = m_ocrAstInfoManager.getOcrGuidSymbolTable(scope);
  Logger::debug(lg) << "dbkname:" << dbkName << endl;
  Logger::debug(lg) << "scope: " << scope << ", " << AstDebug::astToString(scope) << endl;
  guidSymbolTable->insert(dbkName, ocrGuidSymbol);
  Logger::debug(lg) << "SymbolTable:\n" << guidSymbolTable->str() << endl;
}

//! Outline the Edt to a function
void OcrTranslator::outlineEdt(string edtName, OcrEdtContextPtr edtContext) {
  Logger::Logger lg("OcrTranslator::outlineEdt");
  SgSourceFile* sourcefile = edtContext->getSourceFile();
  // scope where the edt function will be created
  SgGlobal* global = sourcefile->get_globalScope();
  // Build an empty defining declaration for the EDT
  SgFunctionDeclaration* edt_decl = AstBuilder::buildOcrEdtFuncDecl(edtName, global);
  // Get the scope under which the statements of EDT will be outlined
  SgBasicBlock* basicblock = edt_decl->get_definition()->get_body();
  // parameters that make up the edt
  vector<SgInitializedName*> edt_params = AstBuilder::buildOcrEdtSignature(basicblock);
  // Insert the parameters to the function declaration
  vector<SgInitializedName*>::iterator p = edt_params.begin();
  SgFunctionParameterList* edt_paramlist = edt_decl->get_parameterList();
  for( ; p != edt_params.end(); ++p) {
    SageInterface::appendArg(edt_paramlist, *p);
  }
  // struct declaration for the dependent elements
  // SgClassDeclaration* depElemStructType = AstBuilder::buildOcrEdtDepElemStruct(edtContext, edt_decl);
  // // corresponding typedef for the dependent elements
  // SgTypedefDeclaration* depElemTypedefType = AstBuilder::buildTypeDefDecl(edtName, depElemStructType->get_type(), global);
  // // Build a declaration for the depElem struct inside basicblock
  // SgName depElemStructName("depElem");
  // SgVariableDeclaration* depElemStructVar = AstBuilder::buildOcrEdtDepElemStructDecl(depElemTypedefType->get_type(), depElemStructName, basicblock);
  // // Build decleration for the datablock pointers
  // vector<SgStatement*> depDbksDecl = AstBuilder::buildOcrDbksDecl(edtContext, basicblock, edt_decl);
  // EDT statements retrieved from the annotation
  // Insert the statements that make up the EDT
  // 1. depElem decl
  // 2. datablock pointer declaration
  // 3. EDT statements
  // SageInterface::appendStatement(depElemStructVar, basicblock);
  // SageInterface::appendStatementList(depDbksDecl, basicblock);
  SgBasicBlock* taskBasicBlock = edtContext->getTaskBasicBlock();
  AstBuilder::buildEdtStmts(taskBasicBlock, basicblock);

  //  SageInterface::appendStatementList(edt_stmts, basicblock);
  // 4. If we have any objects to destroy call their destroy methods
  vector<SgStatement*> cleanupStmts;
  list<string> dbksToDestroy = edtContext->getDbksToDestroy();
  list<string>::iterator db = dbksToDestroy.begin();
  SgVariableSymbol* depvSymbol = GetVariableSymbol(edt_params.back());
  for( ; db != dbksToDestroy.end(); ++db) {
    string dbkname = *db;
    int slot = edtContext->getDepDbkSlotNumber(dbkname);
    SgStatement* dbkDestroyCallExp = AstBuilder::buildOcrDbDestroyCallExp(slot, depvSymbol, basicblock);
    cleanupStmts.push_back(dbkDestroyCallExp);
  }
  // cleanup any events
  // list<string> evtsToDestroy = edtContext->getEvtsToDestroy();
  // list<string>::iterator evt = evtsToDestroy.begin();
  // for( ; evt != evtsToDestroy.end(); ++evt) {
  //   string evtname = *evt;
  //   int slot = edtContext->getDepEvtSlotNumber(evtname);
  //   SgStatement* evtDestroyCallExp = AstBuilder::buildEvtDestroyCallExp(slot, depvSymbol, basicblock);
  //   cleanupStmts.push_back(evtDestroyCallExp);
  // }
  SageInterface::appendStatementList(cleanupStmts, basicblock);

  // Replace depElem variable references
  // DepElemVarRefExpPass replaceDepElemVarRefExp(basicblock, depElemStructName, edtContext->getDepElems());
  // replaceDepElemVarRefExp.traverse(basicblock, postorder);

  // Add a return NULL_GUID statement to the EDT
  SgIntVal* zero = SageBuilder::buildIntVal(0);
  string returnExp = "NULL_GUID";
  SageInterface::addTextForUnparser(zero, returnExp, AstUnparseAttribute::e_replace);
  SgStatement* returnStmt = SageBuilder::buildReturnStmt(zero);
  SageInterface::appendStatement(returnStmt, basicblock);

  // EDT synthesis is complete
  // Insert them just before the main function
  // The correct thing to do however, is to insert the EDT function
  // just before the enclosing function of the pragma
  SgFunctionDeclaration* anchor = SageInterface::findMain(global);
  SageInterface::insertStatementBefore(anchor, edt_decl, true);
  // SageInterface::insertStatementBefore(edt_decl, depElemTypedefType, true);
  // SageInterface::insertStatementBefore(depElemTypedefType, depElemStructType, true);
  // Build the EDT AST information
  m_ocrAstInfoManager.regOcrEdtAstInfo(edtName, edt_decl);
}

void OcrTranslator::insertDepDbkDecl(string edtname, OcrEdtContextPtr edtContext) {
  Logger::Logger lg(" OcrTranslator::insertDepDbkDecl");
  OcrEdtAstInfoPtr edtAstInfoPtr = m_ocrAstInfoManager.getOcrEdtAstInfo(edtname);
  SgFunctionDeclaration* edtDecl = edtAstInfoPtr->getEdtFunctionDeclaration();
  SgBasicBlock* basicblock = edtDecl->get_definition()->get_body();
  OcrGuidSymbolTablePtr guidSymbolTable = m_ocrAstInfoManager.getOcrGuidSymbolTable(basicblock);
  // Get the list of datablock dependences
  // Build ASTs for the pointer declaration
  // Build ASTs for the guid declaration
  list<OcrDbkContextPtr> depDbks = edtContext->getDepDbks();
  // For each dependences create the pointer and the guid
  list<OcrDbkContextPtr>::iterator dbk = depDbks.begin();
  // For each datablock set up the declaration from depv
  SgInitializedNamePtrList& args = edtDecl->get_args();
  // depv is the last element in the EDT argument list
  SgInitializedName* depv = args.back(); assert(depv);
  vector<SgStatement*> depDbksDeclStmts;
  for(unsigned int slot=0 ; dbk != depDbks.end(); ++dbk, ++slot) {
    // Build the pointer
    SgName vname = (*dbk)->getSgInitializedName()->get_name();
    SgType* dbkPtrType = (*dbk)->getDbkPtrType();
    SgVariableDeclaration* vdecl = AstBuilder::buildDbkPtrDecl(vname, dbkPtrType, slot, depv, basicblock);
    depDbksDeclStmts.push_back(vdecl);
    // Build the Guid
    string guidName(vname+"Guid");
    SgVariableDeclaration* guidDecl = AstBuilder::buildDbkGuidDecl(guidName, slot, depv, basicblock);
    Logger::debug(lg) << AstDebug::astToString(guidDecl) << endl;
    SgVariableSymbol* dbkGuidSymbol = GetVariableSymbol(guidDecl, guidName);
    assert(dbkGuidSymbol);
    guidSymbolTable->insert((*dbk)->get_name(), dbkGuidSymbol);
    depDbksDeclStmts.push_back(guidDecl);
  }

  SageInterface::prependStatementList(depDbksDeclStmts, basicblock);
}

void OcrTranslator::insertDepElemDecl(string edtname, OcrEdtContextPtr edtContext) {
  // struct declaration for the dependent elements
  OcrEdtAstInfoPtr edtAstInfoPtr = m_ocrAstInfoManager.getOcrEdtAstInfo(edtname);
  SgFunctionDeclaration* edtDecl = edtAstInfoPtr->getEdtFunctionDeclaration();
  SgBasicBlock* basicblock = edtDecl->get_definition()->get_body();

  SgScopeStatement* edtDeclScope = SageInterface::getEnclosingScope(edtDecl);
  SgClassDeclaration* depElemStructDecl = AstBuilder::buildOcrEdtDepElemStruct(edtContext, edtDecl);

  // corresponding typedef for the dependent elements
  SgTypedefDeclaration* depElemTypedefType = AstBuilder::buildTypeDefDecl(edtname, depElemStructDecl->get_type(), edtDeclScope);

  // Build a declaration for the depElem struct inside basicblock
  SgName depElemStructName("depElem");
  SgVariableDeclaration* depElemStructVar = AstBuilder::buildOcrEdtDepElemStructDecl(depElemTypedefType->get_type(), depElemStructName, basicblock);
  SageInterface::prependStatement(depElemStructVar, basicblock);
  SageInterface::insertStatementBefore(edtDecl, depElemStructDecl, true);
  SageInterface::insertStatementAfter(depElemStructDecl, depElemTypedefType, true);
  // Bookkeeping
  edtAstInfoPtr->setDepElemTypedefType(depElemTypedefType->get_type());
  edtAstInfoPtr->setDepElemBaseType(depElemStructDecl->get_type());
  edtAstInfoPtr->setDepElemStructDecl(depElemStructDecl);
  // Store the guid symbols in their map
}

void OcrTranslator::outlineEdts() {
  list<string> orderedEdts = m_ocrObjectManager.getEdtTraversalOrder();
  list<string>::reverse_iterator edt = orderedEdts.rbegin();
  for( ; edt != orderedEdts.rend(); ++edt) {
    string edtname = *edt;
    OcrEdtContextPtr edtContext = m_ocrObjectManager.getOcrEdtContext(edtname);
    outlineEdt(edtname, edtContext);
    insertDepDbkDecl(edtname, edtContext);
    insertDepElemDecl(edtname, edtContext);
  }
}

void OcrTranslator::setupEdtEvtCreate(std::string edtname, OcrEdtContextPtr edtContext) {
  SgPragmaDeclaration* taskPragma = edtContext->getTaskPragma();
  SgScopeStatement* scope = SageInterface::getScope(taskPragma);
  vector<SgStatement*> evtCreateStmts;
  // Create guid and events that this EDT has to satisfy
  OcrEvtContextPtr outEvt = edtContext->getOutputEvt();
  SgType* ocrGuidType = AstBuilder::buildOcrGuidType(scope);

  // For each event create ocrGuid and the event using ocrEvtCreate
  string evtGuidName = outEvt->get_name() + "EvtGuid";
  SgVariableDeclaration* evtGuidDecl = SageBuilder::buildVariableDeclaration(evtGuidName, ocrGuidType, NULL, scope);
  SgVariableSymbol* evtGuidSymbol = GetVariableSymbol(evtGuidDecl, evtGuidName);
  evtCreateStmts.push_back(evtGuidDecl);
  SgExprStatement* evtCreateCallExp = AstBuilder::buildEvtCreateCallExp(evtGuidSymbol, scope);
  evtCreateStmts.push_back(evtCreateCallExp);
  // Now some bookkeeping
  m_ocrAstInfoManager.regOcrEvtAstInfo(outEvt->get_name(), evtGuidSymbol);
  // Add the statements to the AST
  SageInterface::insertStatementListBefore(taskPragma, evtCreateStmts);
}

void OcrTranslator::setupEdtTemplate(string edtname, OcrEdtContextPtr edtContext) {
  SgPragmaDeclaration* taskPragma = edtContext->getTaskPragma();
  SgScopeStatement* scope = SageInterface::getScope(taskPragma);
  // Build OcrGuid variable declaration for EDT template
  string edtTemplateGuidName = edtname + "TemplGuid";
  SgVariableDeclaration* edtTemplateGuidDecl = AstBuilder::buildOcrGuidEdtTemplateVarDecl(edtTemplateGuidName, scope);
  // Setting up the edt template creation function call
  OcrEdtAstInfoPtr edtAstInfoPtr = m_ocrAstInfoManager.getOcrEdtAstInfo(edtname);
  SgFunctionDeclaration* edtDecl = edtAstInfoPtr->getEdtFunctionDeclaration();
  unsigned int ndelems = 1; // all parameters are wrapped as a struct
  unsigned int ndbks = edtContext->getNumDepDbks() + edtContext->getNumDepEvts();
  SgExprStatement* edtTemplateCallExp = AstBuilder::buildOcrEdtTemplateCallExp(edtTemplateGuidDecl, edtDecl, ndelems, ndbks, scope);
  SageInterface::insertStatementBefore(taskPragma, edtTemplateGuidDecl, true);
  SageInterface::insertStatementBefore(taskPragma, edtTemplateCallExp, true);
  // Now some bookkeeping
  // Add the variable symbol of the template guid
  SgVariableSymbol* edtTemplateGuid = GetVariableSymbol(edtTemplateGuidDecl, edtTemplateGuidName);
  edtAstInfoPtr->setEdtTemplateGuid(edtTemplateGuid);
}

void OcrTranslator::setupEdtDepElems(string edtname, OcrEdtContextPtr edtContext) {
  Logger::Logger lg("OcrTranslator::setupEdtDepElems");
  SgPragmaDeclaration* taskPragma = edtContext->getTaskPragma();
  SgScopeStatement* scope = SageInterface::getScope(taskPragma);
  OcrEdtAstInfoPtr edtAstInfoPtr = m_ocrAstInfoManager.getOcrEdtAstInfo(edtname);
  // Next set up the depenedent elements
  SgType* depElemStructType = edtAstInfoPtr->getDepElemTypedefType();
  string depElemVarName = edtname+"DepElem";
  SgVariableDeclaration* depElemStructVar = SageBuilder::buildVariableDeclaration(depElemVarName, depElemStructType, NULL, scope);
  list<SgVarRefExp*> depElemVarList = edtContext->getDepElems();
  SgClassDeclaration* depElemStructDecl = edtAstInfoPtr->getDepElemStructDecl();
  vector<SgStatement*> depElemSetupStmts = AstBuilder::buildEdtDepElemSetupStmts(depElemStructVar, depElemStructDecl, depElemVarList);
  SageInterface::insertStatementBefore(taskPragma, depElemStructVar, true);
  SageInterface::insertStatementListBefore(taskPragma, depElemSetupStmts);
  // Now some bookkeeping
  SgVariableSymbol* depElemStructSymbol = GetVariableSymbol(depElemStructVar, depElemVarName);
  edtAstInfoPtr->setDepElemStructSymbol(depElemStructSymbol);
}

//! Method to setup the ocrEdtCreate function call expression
void OcrTranslator::setupEdtCreate(string edtname, OcrEdtContextPtr edtContext) {
  SgPragmaDeclaration* taskPragma = edtContext->getTaskPragma();
  SgScopeStatement* scope = SageInterface::getScope(taskPragma);
  OcrEdtAstInfoPtr edtAstInfoPtr = m_ocrAstInfoManager.getOcrEdtAstInfo(edtname);
  string edtGuidName = edtname+"EdtGuid";
  SgType* ocrGuidType = AstBuilder::buildOcrGuidType(scope);
  SgVariableDeclaration* edtGuidDecl = SageBuilder::buildVariableDeclaration(edtGuidName, ocrGuidType, NULL, scope);
  SgVariableSymbol* edtGuidSymbol = GetVariableSymbol(edtGuidDecl, edtGuidName);
  // We need the edtTemplateGuid
  SgVariableSymbol* edtTemplateGuidSymbol = edtAstInfoPtr->getEdtTemplateGuid();
  // We also need the depElemStructSymbol
  SgVariableSymbol* depElemStructSymbol = edtAstInfoPtr->getDepElemStructSymbol();
  OcrEvtContextPtr outEvtContext = edtContext->getOutputEvt();
  OcrEvtAstInfoPtr outEvtAstInfo = m_ocrAstInfoManager.getOcrEvtAstInfo(outEvtContext->get_name());
  SgVariableSymbol* outEvtGuidSymbol = outEvtAstInfo->getEvtGuid();
  bool finishEdt = edtContext->isFinishEdt();
  SgExprStatement* ocrEdtCreateCallExp = AstBuilder::buildOcrEdtCreateCallExp(edtGuidSymbol, edtTemplateGuidSymbol,
									      depElemStructSymbol, outEvtGuidSymbol, finishEdt, scope);
  SageInterface::insertStatementBefore(taskPragma, edtGuidDecl, true);
  SageInterface::insertStatementBefore(taskPragma, ocrEdtCreateCallExp, true);
  // Now some bookkeeping
  edtAstInfoPtr->setEdtGuid(edtGuidSymbol);
}

void OcrTranslator::setupEdtDepDbks(string edtname, OcrEdtContextPtr edtContext) {
  Logger::Logger lg("OcrTranslator::setupEdtDepDbks");
  SgPragmaDeclaration* taskPragma = edtContext->getTaskPragma();
  SgScopeStatement* scope = SageInterface::getScope(taskPragma);
  OcrEdtAstInfoPtr edtAstInfoPtr = m_ocrAstInfoManager.getOcrEdtAstInfo(edtname);
  SgVariableSymbol* edtGuidSymbol = edtAstInfoPtr->getEdtGuid();
  // for each datablock add ocrAddDependence
  list<OcrDbkContextPtr> dbkList = edtContext->getDepDbks();
  list<OcrDbkContextPtr>::iterator l = dbkList.begin();
  vector<SgStatement*> dbkSetupStmts;
  for(int slot = 0 ; l != dbkList.end(); ++l, ++slot) {
    OcrDbkContextPtr dbkContext = *l;
    string dbkname = dbkContext->get_name();
    OcrDbkAstInfoPtr dbkAstInfoPtr = m_ocrAstInfoManager.getOcrDbkAstInfo(dbkname);
    // Get the GuidSymbolTable for the current scope
    OcrGuidSymbolTablePtr guidSymbolTable = m_ocrAstInfoManager.getOcrGuidSymbolTable(scope);
    SgVariableSymbol* dbkGuidSymbol = guidSymbolTable->getGuidSymbol(dbkname);
    SgExprStatement* ocrAddDependenceCallExp = AstBuilder::buildOcrAddDependenceCallExp(dbkGuidSymbol, edtGuidSymbol, slot,
											AstBuilder::DbkMode::DB_DEFAULT_MODE, scope);
    dbkSetupStmts.push_back(static_cast<SgStatement*>(ocrAddDependenceCallExp));
  }
  SageInterface::insertStatementListBefore(taskPragma, dbkSetupStmts);
}

void OcrTranslator::setupEdtDepEvts(string edtname, OcrEdtContextPtr edtContext) {
  SgPragmaDeclaration* taskPragma = edtContext->getTaskPragma();
  SgScopeStatement* scope = SageInterface::getScope(taskPragma);
  OcrEdtAstInfoPtr edtAstInfoPtr = m_ocrAstInfoManager.getOcrEdtAstInfo(edtname);
  SgVariableSymbol* edtGuidSymbol = edtAstInfoPtr->getEdtGuid();
  // For each event add ocrAddDependence
  list<OcrEvtContextPtr> evtList = edtContext->getDepEvts();
  list<OcrEvtContextPtr>::iterator e = evtList.begin();
  // The starting slot dependent events is after the datablocks
  unsigned int slotIndex = edtContext->getNumDepDbks();
  vector<SgStatement*> evtDepSetupStmts;
  for( ; e != evtList.end(); ++e, ++slotIndex) {
    OcrEvtContextPtr evt = *e;
    string evtname = evt->get_name();
    OcrEvtAstInfoPtr evtAstInfo = m_ocrAstInfoManager.getOcrEvtAstInfo(evtname);
    SgVariableSymbol* evtGuid = evtAstInfo->getEvtGuid();
    SgExprStatement* ocrAddDependenceCallExp = AstBuilder::buildOcrAddDependenceCallExp(evtGuid, edtGuidSymbol, slotIndex,
											AstBuilder::DbkMode::DB_MODE_NULL, scope);
    evtDepSetupStmts.push_back(static_cast<SgStatement*>(ocrAddDependenceCallExp));
  }
  SageInterface::insertStatementListBefore(taskPragma, evtDepSetupStmts);
}

void OcrTranslator::removeOcrTaskPragma(string edtname, OcrEdtContextPtr edtContext) {
  SgPragmaDeclaration* taskPragma = edtContext->getTaskPragma();
  SgBasicBlock* basicblock = edtContext->getTaskBasicBlock();
  // Assert the basic block is empty
  SgStatementPtrList& statements = basicblock->get_statements();
  assert(statements.size() == 0);

  SageInterface::removeStatement(taskPragma);
  SageInterface::removeStatement(basicblock);
}

void OcrTranslator::outlineShutdownEdt(string shutdownEdtName, SgSourceFile* sourcefile) {
  // scope where the edt function will be created
  SgGlobal* global = sourcefile->get_globalScope();
  // Build an empty defining declaration for the EDT
  SgFunctionDeclaration* edt_decl = AstBuilder::buildOcrEdtFuncDecl(shutdownEdtName, global);
  // Get the scope under which the statements of EDT will be outlined
  SgBasicBlock* basicblock = edt_decl->get_definition()->get_body();
  // parameters that make up the edt
  vector<SgInitializedName*> edt_params = AstBuilder::buildOcrEdtSignature(basicblock);
  // Insert the parameters to the function declaration
  vector<SgInitializedName*>::iterator p = edt_params.begin();
  SgFunctionParameterList* edt_paramlist = edt_decl->get_parameterList();
  for( ; p != edt_params.end(); ++p) {
    SageInterface::appendArg(edt_paramlist, *p);
  }
  SgExprStatement* shutdownCallExp = AstBuilder::buildOcrShutdownCallExp(basicblock);

  // Add a return NULL_GUID statement to the EDT
  SgIntVal* zero = SageBuilder::buildIntVal(0);
  string returnExp = "NULL_GUID";
  SageInterface::addTextForUnparser(zero, returnExp, AstUnparseAttribute::e_replace);
  SgStatement* returnStmt = SageBuilder::buildReturnStmt(zero);
  // Add both statements to the basic block
  SageInterface::appendStatement(shutdownCallExp, basicblock);
  SageInterface::appendStatement(returnStmt, basicblock);
  // Insert the function just above the pragma
  SgStatement* anchor = SageInterface::getFirstStatement(global);
  SageInterface::insertStatementBefore(anchor, edt_decl, true);
  // Now bookkeeping
  // Build the EDT AST information
  m_ocrAstInfoManager.regOcrEdtAstInfo(shutdownEdtName, edt_decl);
}

void OcrTranslator::setupShutdownEdt(string shutdownEdtSuffix, OcrShutdownEdtContextPtr shutdownEdtContext, int count) {
  SgPragmaDeclaration* shutdownPragma = shutdownEdtContext->getPragma();
  SgSourceFile* sourcefile = SageInterface::getEnclosingSourceFile(shutdownPragma);
  string filename = StrUtil::GetFileNameString(sourcefile->get_file_info()->get_filenameString());
  string shutdownEdtName = filename+shutdownEdtSuffix;
  SgScopeStatement* scope = SageInterface::getScope(shutdownPragma);
  // Build OcrGuid variable declaration for EDT template
  // We may have multiple locations within same file
  // Use count to distinguish the different locations
  stringstream ss;
  ss << shutdownEdtName << "TemplGuid" << count << "_";
  string edtTemplateGuidName(ss.str());
  SgVariableDeclaration* edtTemplateGuidDecl = AstBuilder::buildOcrGuidEdtTemplateVarDecl(edtTemplateGuidName, scope);
  // Setting up the edt template creation function call
  OcrEdtAstInfoPtr edtAstInfoPtr = m_ocrAstInfoManager.getOcrEdtAstInfo(shutdownEdtName);
  SgFunctionDeclaration* edtDecl = edtAstInfoPtr->getEdtFunctionDeclaration();
  unsigned int ndbks = shutdownEdtContext->getNumDepEvts();
  SgExprStatement* edtTemplateCallExp = AstBuilder::buildOcrEdtTemplateCallExp(edtTemplateGuidDecl, edtDecl, 0, ndbks, scope);
  SageInterface::insertStatementBefore(shutdownPragma, edtTemplateGuidDecl, true);
  SageInterface::insertStatementBefore(shutdownPragma, edtTemplateCallExp, true);
  // Now some bookkeeping
  // Add the variable symbol of the template guid
  SgVariableSymbol* edtTemplateGuidSymbol = GetVariableSymbol(edtTemplateGuidDecl, edtTemplateGuidName);
  edtAstInfoPtr->setEdtTemplateGuid(edtTemplateGuidSymbol);

  // Setup the EDT creation
  // Build the variable name
  ss.str("");
  ss << shutdownEdtName << "EdtGuid" << count << "_";
  string edtGuidName(ss.str());
  SgType* ocrGuidType = AstBuilder::buildOcrGuidType(scope);
  SgVariableDeclaration* edtGuidDecl = SageBuilder::buildVariableDeclaration(edtGuidName, ocrGuidType, NULL, scope);
  SgVariableSymbol* edtGuidSymbol = GetVariableSymbol(edtGuidDecl, edtGuidName);
  // We need the edtTemplateGuid
  SgExprStatement* ocrEdtCreateCallExp = AstBuilder::buildOcrEdtCreateCallExp(edtGuidSymbol, edtTemplateGuidSymbol, NULL, NULL, false, scope);
  SageInterface::insertStatementBefore(shutdownPragma, edtGuidDecl, true);
  SageInterface::insertStatementBefore(shutdownPragma, ocrEdtCreateCallExp, true);
  // Now some bookkeeping
  edtAstInfoPtr->setEdtGuid(edtGuidSymbol);

  // Finally add the dependent events
  list<OcrEvtContextPtr> evtList = shutdownEdtContext->getDepEvts();
  list<OcrEvtContextPtr>::iterator e = evtList.begin();
  // The starting slot dependent events is after the datablocks
  unsigned int slotIndex = 0;
  vector<SgStatement*> evtDepSetupStmts;
  for( ; e != evtList.end(); ++e, ++slotIndex) {
    OcrEvtContextPtr evt = *e;
    string evtname = evt->get_name();
    OcrEvtAstInfoPtr evtAstInfo = m_ocrAstInfoManager.getOcrEvtAstInfo(evtname);
    SgVariableSymbol* evtGuid = evtAstInfo->getEvtGuid();
    SgExprStatement* ocrAddDependenceCallExp = AstBuilder::buildOcrAddDependenceCallExp(evtGuid, edtGuidSymbol, slotIndex,
											AstBuilder::DbkMode::DB_MODE_NULL, scope);
    evtDepSetupStmts.push_back(static_cast<SgStatement*>(ocrAddDependenceCallExp));
  }
  SageInterface::insertStatementListBefore(shutdownPragma, evtDepSetupStmts);

  // Finally remove the pragma from the AST
  SageInterface::removeStatement(shutdownPragma);
}

void OcrTranslator::replaceDepElemVars(string edtname, OcrEdtContextPtr edtContext) {
  OcrEdtAstInfoPtr edtAstInfoPtr = m_ocrAstInfoManager.getOcrEdtAstInfo(edtname);
  SgFunctionDeclaration* edtDecl = edtAstInfoPtr->getEdtFunctionDeclaration();
  SgBasicBlock* basicblock = edtDecl->get_definition()->get_body();
  SgName depElemStructName("depElem");
  DepElemVarRefExpPass replaceDepElemVarRefExp(basicblock, depElemStructName, edtContext->getDepElems());
  replaceDepElemVarRefExp.traverse(basicblock, postorder);
}

void OcrTranslator::setupEdts() {
  list<string> orderedEdts = m_ocrObjectManager.getEdtTraversalOrder();
  // Traverse them in the reverse order
  list<string>::iterator edt = orderedEdts.begin();
  for( ; edt != orderedEdts.end(); ++edt) {
    string edtname = *edt;
    OcrEdtContextPtr edtContext = m_ocrObjectManager.getOcrEdtContext(edtname);
    setupEdtEvtCreate(edtname, edtContext);
    setupEdtTemplate(edtname, edtContext);
    setupEdtDepElems(edtname, edtContext);
    setupEdtCreate(edtname, edtContext);
    setupEdtDepDbks(edtname, edtContext);
    setupEdtDepEvts(edtname, edtContext);
    // Finally remove the pragma for each edt
    removeOcrTaskPragma(edtname, edtContext);
  }
}

void OcrTranslator::replaceDepElemPass() {
  list<string> orderedEdts = m_ocrObjectManager.getEdtTraversalOrder();
  // Traverse them in the reverse order
  list<string>::iterator edt = orderedEdts.begin();
  for( ; edt != orderedEdts.end(); ++edt) {
    string edtname = *edt;
    OcrEdtContextPtr edtContext = m_ocrObjectManager.getOcrEdtContext(edtname);
    // Finally do the pass to replace depElem
    replaceDepElemVars(edtname, edtContext);
  }
}

void OcrTranslator::translateDbks() {
  set<SgPragmaDeclaration*> dbkPragmaSet;
  const OcrDbkObjectMap& dbkMap = m_ocrObjectManager.getOcrDbkObjectMap();
  OcrDbkObjectMap::const_iterator d = dbkMap.begin();
  for( ; d != dbkMap.end(); ++d) {
    translateDbk(d->first, d->second);
    // After translating the datablock
    // Collect its pragma and mark the annotation for deletion
    SgPragmaDeclaration* dbkPragma = d->second->get_pragma();
    dbkPragmaSet.insert(dbkPragma);
  }

  // We can have multiple datablocks annotated by the same pragma
  // We collect all the pragmas into a set delete them later
  set<SgPragmaDeclaration*>::iterator p = dbkPragmaSet.begin();
  for( ; p != dbkPragmaSet.end(); ++p) {
    SageInterface::removeStatement(*p);
  }
}

void OcrTranslator::setupShutdownEdts() {
  string shutdownEdtSuffix = "Shutdown";
  // We may need to allow the user to call shutdown from multiple points in the program
  // The difficulty is we need to synthesize the shutdown EDT and place its definition
  // before the pragma with the lowest depth from the root
  // The simplest choice is to place them at the beginning of a source file
  // We may however have multiple source files
  // We can generate one copy of the function for each source file
  // Alternatively,
  list<OcrShutdownEdtContextPtr> shutdownEdts = m_ocrObjectManager.getOcrShutdownEdtList();
  set<SgSourceFile*> shutdownEdtSourceFiles = getSourceFilesOfShutdownEdts(shutdownEdts);
  set<SgSourceFile*>::iterator sf = shutdownEdtSourceFiles.begin();
  // For each sourcefile generate a shutdownEdt
  for( ; sf != shutdownEdtSourceFiles.end(); ++sf) {
    string filename = StrUtil::GetFileNameString((*sf)->get_file_info()->get_filenameString());
    outlineShutdownEdt(filename + shutdownEdtSuffix, *sf);
  }
  // Setup the shutdown edt for each shutdown annotation
  list<OcrShutdownEdtContextPtr>::iterator sedt = shutdownEdts.begin();
  for(int count = 0; sedt != shutdownEdts.end(); ++sedt, ++count) {
    setupShutdownEdt(shutdownEdtSuffix, *sedt, count);
  }
}

void OcrTranslator::outlineMainEdt() {
  SgFunctionDeclaration* mainFunction = SageInterface::findMain(m_project);
  // scope where the edt function will be created
  SgSourceFile* sourcefile = SageInterface::getEnclosingSourceFile(mainFunction);
  SgGlobal* global = sourcefile->get_globalScope();
  // Build an empty defining declaration for the EDT
  string mainEdtName = "mainEdt";
  SgFunctionDeclaration* edt_decl = AstBuilder::buildOcrEdtFuncDecl(mainEdtName, global);
  // Get the scope under which the statements of EDT will be outlined
  SgBasicBlock* basicblock = edt_decl->get_definition()->get_body();
  // parameters that make up the edt
  vector<SgInitializedName*> edt_params = AstBuilder::buildOcrEdtSignature(basicblock);
  // Insert the parameters to the function declaration
  vector<SgInitializedName*>::iterator p = edt_params.begin();
  SgFunctionParameterList* edt_paramlist = edt_decl->get_parameterList();
  for( ; p != edt_params.end(); ++p) {
    SageInterface::appendArg(edt_paramlist, *p);
  }
  // Synthesize arugments for argc and argv
  // And then move the basicblock from main to mainEdt
  vector<SgInitializedName*> main_args = mainFunction->get_args();
  if(main_args.size() > 0) {
    // main must have exactly two arguments
    assert(main_args.size() == 2);
    SgInitializedName* mainArgc = main_args[0];
    SgInitializedName* mainArgv = main_args[1];
    // First fetch the datablock from argument
    SgName mainEdtDbkName("mainEdtDbk");
    SgInitializedName* mainEdtArgv = edt_paramlist->get_args().back();
    SgType* u64PtrType = AstBuilder::buildu64PtrType(basicblock);
    SgVariableDeclaration* mainEdtDbkDecl = AstBuilder::buildDbkPtrDecl(mainEdtDbkName, u64PtrType, 0, mainEdtArgv, basicblock);
    SgVariableSymbol* mainEdtDbkSymbol = GetVariableSymbol(mainEdtDbkDecl, mainEdtDbkName);
    SgVariableDeclaration* mainEdtArgcDecl = AstBuilder::buildMainEdtArgcDecl(mainArgc, mainEdtDbkSymbol, basicblock);
    SgVariableDeclaration* mainEdtArgvDecl = AstBuilder::buildMainEdtArgvDecl(mainArgv, mainArgc, basicblock);
    SageInterface::appendStatement(mainEdtDbkDecl, basicblock);
    SageInterface::appendStatement(mainEdtArgcDecl, basicblock);
    SageInterface::appendStatement(mainEdtArgvDecl, basicblock);

    // Now setup the function to initialize the argv
    SgName mainEdtInitName("mainEdtInit");
    SgFunctionDeclaration* mainEdtInitFuncDecl = AstBuilder::buildMainEdtInitFuncDecl(mainEdtInitName, mainArgc, mainArgv, mainEdtDbkSymbol, global);
    SgVariableSymbol* mainEdtArgcSymbol = GetVariableSymbol(mainEdtArgcDecl, mainArgc->get_name());
    SgVariableSymbol* mainEdtArgvSymbol = GetVariableSymbol(mainEdtArgvDecl, mainArgv->get_name());
    SgExprStatement* mainEdtInitCallExp = AstBuilder::buildMainEdtInitCallExp(mainEdtInitFuncDecl->get_name(), mainEdtArgcSymbol, mainEdtArgvSymbol, mainEdtDbkSymbol, basicblock);
    SageInterface::appendStatement(mainEdtInitCallExp, basicblock);

    // Insert the mainEdt_init function declaration before main
    SageInterface::insertStatementBefore(mainFunction, mainEdtInitFuncDecl, true);
  }
  SgBasicBlock* mainBasicBlock = mainFunction->get_definition()->get_body();

  // Replace all the return statements
  AstBuilder::ReplaceReturnStmt replaceRetStmts;
  replaceRetStmts.traverse(mainBasicBlock, preorder);
  // Add the statements from main to mainEdt
  SageInterface::appendStatementList(mainBasicBlock->get_statements(), basicblock);
  // Finally replace the main with mainEdt
  SageInterface::replaceStatement(mainFunction, edt_decl, true);
}

void OcrTranslator::translate() {
  Logger::Logger lg("OcrTranslator::translate");
  try {
    // insert the header files
    insertOcrHeaderFiles();
    outlineEdts();
    translateDbks();
    setupEdts();
    replaceDepElemPass();
    setupShutdownEdts();
    outlineMainEdt();
  }
  catch(TranslateException& ewhat) {
    Logger::error(lg) << ewhat.what() << endl;
  }
}

set<SgSourceFile*> OcrTranslator::getSourceFilesOfShutdownEdts(list<OcrShutdownEdtContextPtr>& shutdownEdts) {
  set<SgSourceFile*> shutdownEdtSourceFiles;
  list<OcrShutdownEdtContextPtr>::iterator s = shutdownEdts.begin();
  for( ; s != shutdownEdts.end(); ++s) {
    SgSourceFile* sourcefile = SageInterface::getEnclosingSourceFile((*s)->getPragma());
    shutdownEdtSourceFiles.insert(sourcefile);
  }
  return shutdownEdtSourceFiles;
}

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
OcrEdtAstInfo::OcrEdtAstInfo() { }

string OcrEdtAstInfo::str() const {
  return "OcrEdtAstInfo";
}

/*********************
 * OcrAstInfoManager *
 *********************/
OcrAstInfoManager::OcrAstInfoManager() { }

bool OcrAstInfoManager::regOcrDbkAstInfo(string dbkname, SgVariableSymbol* ocrGuidSymbol, SgVariableSymbol* ptrSymbol) {
  OcrDbkAstInfoPtr dbkAstInfoPtr = boost::make_shared<OcrDbkAstInfo>(dbkname, ocrGuidSymbol, ptrSymbol);
  OcrDbkAstInfoMapElem elem(dbkname, dbkAstInfoPtr);
  m_ocrDbkAstInfoMap.insert(elem);
}

OcrDbkAstInfoPtr OcrAstInfoManager::getOcrDbkAstInfo(string dbkname) {
  OcrDbkAstInfoMap::iterator f = m_ocrDbkAstInfoMap.find(dbkname);
  assert(f != m_ocrDbkAstInfoMap.end());
  return f->second;
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
  Logger::Logger lg("OcrTranslator::translateDbk", Logger::DEBUG);
  // Mark the statements to be removed at the end of this function
  set<SgStatement*> stmtsToRemove;
  SgInitializedName* varInitializedName = dbkContext->getSgInitializedName();
  SgSymbol* varSymbol = dbkContext->getSgSymbol();
  SgScopeStatement* scope = varSymbol->get_scope();
  SgName varName = varSymbol->get_name();
  SgName ocrGuidName(dbkName);
  SgType* varType = varSymbol->get_type();
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
  SgVariableSymbol* ocrGuidSymbol = isSgVariableSymbol(varDbkGuid->get_decl_item(ocrGuidName)->
						       search_for_symbol_from_symbol_table());
  SgVariableSymbol* dbkPtrSymbol = isSgVariableSymbol(varDbkDecl->get_decl_item(varName)->
						      search_for_symbol_from_symbol_table());
  assert(ocrGuidSymbol);
  assert(dbkPtrSymbol);
  m_ocrAstInfoManager.regOcrDbkAstInfo(dbkName, ocrGuidSymbol, dbkPtrSymbol);
  // Make sure it is registered
  // OcrDbkAstInfoPtr regDbk = m_ocrAstInfoManager.getOcrDbkAstInfo(dbkName);
  // Logger::debug(lg) << "DBKASTINFO : " << regDbk->str() << "\n";
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
  SgClassDeclaration* depElemStructType = AstBuilder::buildOcrEdtDepElemStruct(edtContext, edt_decl);
  // corresponding typedef for the dependent elements
  SgTypedefDeclaration* depElemTypedefType = AstBuilder::buildTypeDefDecl(edtName, depElemStructType->get_type(), global);
  // Build a declaration for the depElem struct inside basicblock
  SgName depElemStructName("depElem");
  SgVariableDeclaration* depElemStructVar = AstBuilder::buildOcrEdtDepElemStructDecl(depElemTypedefType->get_type(), depElemStructName, basicblock);
  // Build decleration for the datablock pointers
  vector<SgStatement*> depDbksDecl = AstBuilder::buildOcrDbksDecl(edtContext, basicblock, edt_decl);
  // EDT statements retrieved from the annotation
  vector<SgStatement*> edt_stmts = AstBuilder::buildOcrEdtStmts(edtContext);
  // Insert the statements that make up the EDT
  // 1. depElem decl
  // 2. datablock pointer declaration
  // 3. EDT statements
  SageInterface::appendStatement(depElemStructVar, basicblock);
  SageInterface::appendStatementList(depDbksDecl, basicblock);
  SageInterface::appendStatementList(edt_stmts, basicblock);
  // TODO: replace depElem variable references
  DepElemVarRefExpPass replaceDepElemVarRefExp(basicblock, depElemStructName, edtContext->getDepElems());
  replaceDepElemVarRefExp.traverse(basicblock, postorder);
  // EDT synthesis is complete
  // Insert them just before the main function
  // The correct thing to do however, is to insert the EDT function
  // just before the enclosing function of the pragma
  SgFunctionDeclaration* anchor = SageInterface::findMain(global);
  SageInterface::insertStatementBefore(anchor, edt_decl, true);
  SageInterface::insertStatementBefore(edt_decl, depElemTypedefType, true);
  SageInterface::insertStatementBefore(depElemTypedefType, depElemStructType, true);
  //AstBuilder::buildOcrEdt(edtName, edtContext);
}

void OcrTranslator::outlineEdts() {
  const OcrEdtObjectMap& edtMap = m_ocrObjectManager.getOcrEdtObjectMap();
  OcrEdtObjectMap::const_iterator e = edtMap.begin();
  for( ; e != edtMap.end(); ++e) {
    outlineEdt(e->first, e->second);
  }
}

void OcrTranslator::translateDbks() {
  const OcrDbkObjectMap& dbkMap = m_ocrObjectManager.getOcrDbkObjectMap();
  OcrDbkObjectMap::const_iterator d = dbkMap.begin();
  for( ; d != dbkMap.end(); ++d) {
    translateDbk(d->first, d->second);
  }
}

void OcrTranslator::translate() {
  Logger::Logger lg("OcrTranslator::translate");
  try {
    // insert the header files
    insertOcrHeaderFiles();
    translateDbks();
    outlineEdts();
  }
  catch(TranslateException& ewhat) {
    Logger::error(lg) << ewhat.what() << endl;
  }
}

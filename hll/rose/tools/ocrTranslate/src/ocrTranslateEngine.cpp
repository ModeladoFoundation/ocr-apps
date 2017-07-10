/*
 * Author: Sriram Aananthakrishnan, 2017 *
 */

#include "sage3basic.h"
#include "ocrTranslateEngine.h"
#include "ocrAstBuilder.h"
#include "boost/make_shared.hpp"
#include "logger.h"
#include <algorithm>

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
 * OcrTranslator *
 *****************/
OcrTranslator::OcrTranslator(SgProject* project, OcrObjectManager& ocrObjectManager)
  : m_project(project),
    m_ocrObjectManager(ocrObjectManager) { }

void OcrTranslator::insertOcrHeaderFiles() {
  SgFilePtrList files = m_project->get_files();
  SgFilePtrList::iterator f = files.begin();
  for( ; f != files.end(); ++f) {
    if(SgSourceFile* source = isSgSourceFile(*f)) {
      SgGlobal* global = source->get_globalScope();
      SageInterface::insertHeader(source, "ocr.h", false, true);
      if(m_ocrObjectManager.hasMpiOp()) {
	SageInterface::insertHeader(source, "spmd.h", false, true);
      }
    }
  }
}

void OcrTranslator::translateDbk(string dbkName, OcrDbkContextPtr dbkContext) {
  Logger::Logger lg("OcrTranslator::translateDbk");
  // Mark the statements to be removed at the end of this function
  set<SgStatement*> stmtsToRemove;
  SgScopeStatement* scope = SageInterface::getEnclosingScope(dbkContext->get_pragma());
  if(dbkContext->getDbkType() == OcrDbkContext::DBK_mem) {
    OcrMemDbkContextPtr memDbkContext = boost::dynamic_pointer_cast<OcrMemDbkContext>(dbkContext);
    SgInitializedName* varInitializedName = memDbkContext->getSgInitializedName();
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
    list<SgStatement*> allocStmts = memDbkContext->get_allocStmts();
    list<SgStatement*>::iterator s = allocStmts.begin();
    for( ; s != allocStmts.end(); ++s) {
      SgExprStatement* dbCreateStmt = AstBuilder::buildOcrDbCreateFuncCallExp(ocrGuidName, varName, scope, *s);
      SgStatement* stmtAfterAllocStmt = SageInterface::getNextStatement(*s);
      SageInterface::insertStatementBefore(stmtAfterAllocStmt, dbCreateStmt, true);
      // Mark the statement for removal
      stmtsToRemove.insert(*s);
    }
    // Bookkeeping
    // Register the variable names we assigned to dbkGuid and dbkPtr
    DbkAstInfoPtr dbkAstInfo = boost::make_shared<DbkAstInfo>(dbkName, ocrGuidName.getString(), varName.getString());
    m_astInfoManager.regDbkAstInfo(dbkName, dbkAstInfo, scope);
  }
  else if(dbkContext->getDbkType() == OcrDbkContext::DBK_arr) {
    OcrArrDbkContextPtr arrDbkContext = boost::dynamic_pointer_cast<OcrArrDbkContext>(dbkContext);
    SgInitializedName* arrInitializedName = arrDbkContext->getArrInitializedName();
    // We need insert the dbk struct declarations in the global scope before all the EDTs
    SgGlobal* globalScope = SageInterface::getGlobalScope(scope);
    string dbkStructName = SageInterface::generateUniqueVariableName(globalScope, arrInitializedName->get_name().getString()+"Dbk");
    SgClassDeclaration* dbkStructDecl = AstBuilder::buildArrDbkStructDecl(dbkStructName, arrInitializedName, globalScope);
    SgTypedefDeclaration* dbkStructTypeDecl = AstBuilder::buildArrDbkStructTypedefType(dbkStructDecl, globalScope);
    SgStatement* fstmt = SageInterface::getFirstStatement(globalScope);
    SageInterface::insertStatementBefore(fstmt, dbkStructDecl);
    SageInterface::insertStatementBefore(fstmt, dbkStructTypeDecl);

    // Build a guid and pointer to the datablock
    string dbkGuidName = arrInitializedName->get_name().getString() +"DbkGuid";
    SgVariableDeclaration* dbkGuidDecl = AstBuilder::buildOcrDbkGuid(dbkGuidName, scope);
    SgPointerType* dbkPtrType = SageBuilder::buildPointerType(dbkStructTypeDecl->get_type());

    string dbkPtrName = arrInitializedName->get_name().getString()+"DbkPtr";
    SgVariableDeclaration* dbkPtrDecl = AstBuilder::buildOcrDbkVarDecl(dbkPtrName, dbkPtrType, scope);
    SgVariableSymbol* dbkPtrSymbol = GetVariableSymbol(dbkPtrDecl, dbkPtrName);

    SgExprStatement* dbCreateStmt = AstBuilder::buildOcrDbCreateFuncCallExp(dbkGuidName, dbkPtrName, scope, dbkStructTypeDecl->get_type());
    SgVariableDeclaration* arrPtrDecl = AstBuilder::buildArrPtrDecl(arrInitializedName, dbkPtrSymbol, dbkStructDecl, scope);

    SgStatement* enclosingStmt = SageInterface::getEnclosingStatement(arrInitializedName);

    SageInterface::insertStatementBefore(enclosingStmt, dbkGuidDecl);
    SageInterface::insertStatementBefore(enclosingStmt, dbkPtrDecl);
    SageInterface::insertStatementBefore(enclosingStmt, dbCreateStmt);
    SageInterface::insertStatementBefore(enclosingStmt, arrPtrDecl);
    stmtsToRemove.insert(enclosingStmt);
    // Bookkeeping
    DbkAstInfoPtr arrDbkAstInfo = boost::make_shared<ArrDbkAstInfo>(dbkName, dbkGuidName, dbkPtrName, dbkStructDecl, dbkStructTypeDecl->get_type(),
								    arrInitializedName->get_name().getString());
    assert(arrDbkAstInfo);
    // Register the datablock in the scope
    Logger::debug(lg) << arrDbkAstInfo->str() << endl;
    Logger::debug(lg) << "registering dbk at scope: " << scope << endl;
    m_astInfoManager.regDbkAstInfo(dbkName, arrDbkAstInfo, scope);
  }
  else assert(false);
  // Now remove all the statements marked for removal
  set<SgStatement*>::iterator st = stmtsToRemove.begin();
  for( ; st != stmtsToRemove.end(); ++st) {
    SageInterface::removeStatement(*st);
  }
}

void OcrTranslator::translateDbks() {
  set<SgPragmaDeclaration*> dbkPragmaSet;
  list<OcrDbkContextPtr> dbkContextPtrList = m_ocrObjectManager.getOcrDbkContextList();
  list<OcrDbkContextPtr>::iterator d = dbkContextPtrList.begin();
  for( ; d != dbkContextPtrList.end(); ++d) {
    translateDbk((*d)->get_name(), *d);
    // After translating the datablock
    // Collect its pragma and mark the annotation for deletion
    SgPragmaDeclaration* dbkPragma = (*d)->get_pragma();
    dbkPragmaSet.insert(dbkPragma);
  }

  // We can have multiple datablocks annotated by the same pragma
  // We collect all the pragmas into a set delete them later
  set<SgPragmaDeclaration*>::iterator p = dbkPragmaSet.begin();
  for( ; p != dbkPragmaSet.end(); ++p) {
    SageInterface::removeStatement(*p);
  }
}

// void OcrTranslator::insertDbkDestroyStmts(string edtName, list<string>& dbksToDestroy, unsigned int slotbegin) {
//   // 4. If we have any objects to destroy call their destroy methods
//   OcrEdtAstInfoPtr edtAstInfoPtr = m_astInfoManager.getEdtAstInfo(edtName);
//   assert(edtAstInfoPtr);
//   SgFunctionDeclaration* edtDecl = edtAstInfoPtr->getTaskFuncDecl();
//   SgBasicBlock* basicblock = edtDecl->get_definition()->get_body();
//   // EDT signature consists of 4 arguments
//   // Unless the signuare changes there is no need to verify this assertion
//   SgInitializedNamePtrList& edt_params = edtDecl->get_parameterList()->get_args();
//   assert(basicblock);
//   // Dbk cleanup stmt list
//   vector<SgStatement*> cleanupStmts;
//   list<string>::iterator db = dbksToDestroy.begin();
//   SgVariableSymbol* depvSymbol = GetVariableSymbol(edt_params.back());
//   int slot = slotbegin;
//   for( ; db != dbksToDestroy.end(); ++db, ++slot) {
//     string dbkname = *db;
//     SgStatement* dbkDestroyCallExp = AstBuilder::buildOcrDbDestroyCallExp(slot, depvSymbol, basicblock);
//     cleanupStmts.push_back(dbkDestroyCallExp);
//   }
//   SageInterface::appendStatementList(cleanupStmts, basicblock);
// }

// void OcrTranslator::insertEvtDestroyStmts(string edtName, list<string>& evtsToDestroy, unsigned int slotbegin) {
//   OcrEdtAstInfoPtr edtAstInfoPtr = m_ocrAstInfoManager.getOcrEdtAstInfo(edtName);
//   assert(edtAstInfoPtr);
//   SgFunctionDeclaration* edtDecl = edtAstInfoPtr->getEdtFunctionDeclaration();
//   // EDT signature consists of 4 arguments
//   // Unless the signuare changes there is no need to verify this assertion
//   SgInitializedNamePtrList& edt_params = edtDecl->get_parameterList()->get_args();
//   SgBasicBlock* basicblock = edtDecl->get_definition()->get_body();
//   assert(basicblock);
//   // Evt cleanup stmt list
//   list<string>::iterator evt = evtsToDestroy.begin();
//   int slot = slotbegin;
//   SgVariableSymbol* depvSymbol = GetVariableSymbol(edt_params.back());
//   vector<SgStatement*> cleanupStmts;
//   for( ; evt != evtsToDestroy.end(); ++evt) {
//     string evtname = *evt;
//     SgStatement* evtDestroyCallExp = AstBuilder::buildEvtDestroyCallExp(slot, depvSymbol, basicblock);
//     cleanupStmts.push_back(evtDestroyCallExp);
//   }
//   SageInterface::appendStatementList(cleanupStmts, basicblock);
// }

void OcrTranslator::insertDepElemDecl(string edtName, TaskAstInfoPtr taskAstInfo) {
  SgClassDeclaration* depElemStructDecl = taskAstInfo->getDepElemStructDecl();
  SgTypedefDeclaration* depElemTypedefDecl = taskAstInfo->getDepElemTypedefDecl();
  SgFunctionDeclaration* edtDecl = taskAstInfo->getTaskFuncDecl();

  // If we did not create a struct decl nothing to insert here
  if(!taskAstInfo->hasDepElems()) {
    return;
  }
  SgDeclarationStatementPtrList memberList = depElemStructDecl->get_definition()->get_members();
  assert(!memberList.empty());
  // Insert the depElem struct above the EDT
  SageInterface::insertStatementBefore(edtDecl, depElemTypedefDecl, true);
  SageInterface::insertStatementBefore(depElemTypedefDecl, depElemStructDecl, true);
  // Insert depElem inside the EDT
  SgBasicBlock* basicblock = edtDecl->get_definition()->get_body();
  string depElemName = SageInterface::generateUniqueVariableName(basicblock, "depElem");
  SgName depElemStructName(depElemName);
  SgVariableDeclaration* depElemStructVar = AstBuilder::buildOcrEdtDepElemStructDecl(depElemTypedefDecl->get_type(), depElemStructName, basicblock);
  SageInterface::prependStatement(depElemStructVar, basicblock);

  SgVariableSymbol* depElemStructSymbol = GetVariableSymbol(depElemStructVar, depElemStructName);
  // For each member of the struct, build the variable declaration with initializer
  vector<SgStatement*> depElemVarDeclStmts = AstBuilder::buildEdtDepElemVarsDecl(depElemStructDecl, depElemStructSymbol, basicblock);
  SageInterface::insertStatementListAfter(depElemStructVar, depElemVarDeclStmts);
}

/*!
 * \brief Function to build a struct for local variables needed by the EDT and
 *  insert the local variables inside the EDT declaration
 * \in edtName Name of the EDT for which the struct is built
 * \in list<SgVarRefExp*> list of variable symbols to be added to the struct
 * \in TaskAstInfoPtr Associating the newly built AST fragments with the EDT
 *
 * Function builds a struct for the list of variables in depElems, typedef declaration
 * for the struct and inserts the struct and typedef declaration just before the edt declaration.
 */
void OcrTranslator::buildDepElemAST(string edtName, list<SgVarRefExp*> depElems, TaskAstInfoPtr taskAstInfo) {
  // When the list is empty there is no need to build a struct
  SgFunctionDeclaration* edtDecl = taskAstInfo->getTaskFuncDecl();
  assert(edtDecl);
  SgScopeStatement* edtScope = SageInterface::getEnclosingScope(edtDecl);
  if(depElems.empty()) {
    assert(!taskAstInfo->hasDepElems());
    return;
  }

  SgClassDeclaration* depElemStructDecl = AstBuilder::buildOcrEdtDepElemStruct(edtName, depElems, edtScope);
  // Build a typedef for it
  SgTypedefDeclaration* depElemTypedefDecl = AstBuilder::buildTypeDefDecl(edtName, depElemStructDecl->get_type(), edtScope);

  // Bookkeeping
  taskAstInfo->setDepElemStructDecl(depElemStructDecl);
  taskAstInfo->setDepElemTypedefDecl(depElemTypedefDecl);
  taskAstInfo->setDepElemTypedefType(depElemTypedefDecl->get_type());
  taskAstInfo->setDepElemBaseType(depElemStructDecl->get_type());
}

void OcrTranslator::buildLoopDepElemAST(string edtName, list<SgVarRefExp*> depElems,
					TaskAstInfoPtr taskAstInfo, string loopCompEvtName) {
  SgFunctionDeclaration* edtDecl = taskAstInfo->getTaskFuncDecl();
  SgScopeStatement* edtScope = SageInterface::getEnclosingScope(edtDecl);
  SgClassDeclaration* depElemStructDecl = AstBuilder::buildOcrEdtDepElemStruct(edtName, depElems, edtScope);
  // Build a typedef for it
  SgTypedefDeclaration* depElemTypedefDecl = AstBuilder::buildTypeDefDecl(edtName, depElemStructDecl->get_type(), edtScope);

  // Add the guid for the loop completion event
  string loopCompEvtGuidName = loopCompEvtName + "Guid";
  SgClassDefinition* depElemStructDefn = depElemStructDecl->get_definition();
  SgVariableDeclaration* loopCompEvtGuidDecl = AstBuilder::buildGuidVarDecl(loopCompEvtGuidName, depElemStructDefn);
  SgVariableSymbol* loopCompEvtGuidSymbol = GetVariableSymbol(loopCompEvtGuidDecl, loopCompEvtGuidName);
  depElemStructDefn->append_member(loopCompEvtGuidDecl);

  // Bookkeeping
  // Register the event inside the struct scope
  taskAstInfo->setDepElemStructDecl(depElemStructDecl);
  taskAstInfo->setDepElemTypedefDecl(depElemTypedefDecl);
  taskAstInfo->setDepElemTypedefType(depElemTypedefDecl->get_type());
  taskAstInfo->setDepElemBaseType(depElemStructDecl->get_type());
  m_astInfoManager.regEvtAstInfo(loopCompEvtName, loopCompEvtGuidName, depElemStructDefn);
}

void OcrTranslator::outlineEdt(string edtName, SgBasicBlock* taskBasicBlock, SgSourceFile* sourcefile,
			       TaskAstInfoPtr taskAstInfo) {
  Logger::Logger lg("OcrTranslator::outlineEdt");
  // scope where the edt function will be created
  SgGlobal* global = sourcefile->get_globalScope();
  // Build an empty defining declaration for the EDT
  SgFunctionDeclaration* edtDecl = AstBuilder::buildOcrEdtFuncDecl(edtName, global);
  // Get the scope under which the statements of EDT will be outlined
  SgBasicBlock* basicblock = edtDecl->get_definition()->get_body();
  // parameters that make up the edt
  vector<SgInitializedName*> edt_params = AstBuilder::buildOcrEdtSignature(basicblock);
  // Insert the parameters to the function declaration
  vector<SgInitializedName*>::iterator p = edt_params.begin();
  SgFunctionParameterList* edt_paramlist = edtDecl->get_parameterList();
  for( ; p != edt_params.end(); ++p) {
    SageInterface::appendArg(edt_paramlist, *p);
  }
  AstBuilder::buildEdtStmts(taskBasicBlock, basicblock);
  // Insert Return Stmt
  // Add a return NULL_GUID statement to the EDT
  SgIntVal* zero = SageBuilder::buildIntVal(0);
  string returnExp = "NULL_GUID";
  SageInterface::addTextForUnparser(zero, returnExp, AstUnparseAttribute::e_replace);
  SgStatement* returnStmt = SageBuilder::buildReturnStmt(zero);
  SageInterface::appendStatement(returnStmt, basicblock);

  SgStatement* firstStmt = SageInterface::getFirstStatement(global);
  SgDeclarationStatement* nonDefiningDeclaration = edtDecl->get_firstNondefiningDeclaration();
  SageInterface::insertStatementBefore(firstStmt, nonDefiningDeclaration, global);

  SgStatement* lastStmt = SageInterface::getLastStatement(global);
  SageInterface::insertStatementBefore(lastStmt, edtDecl, global);

  // Bookkeeping: Store the function declaration in the outlining info
  taskAstInfo->setTaskFuncDecl(edtDecl);
}

void OcrTranslator::outlineShutdownEdt(OcrShutdownEdtContextPtr shutdownEdtContext) {
  SgSourceFile* sourcefile = shutdownEdtContext->getSourceFile();
  // scope where the edt function will be created
  SgGlobal* global = sourcefile->get_globalScope();
  // Build an empty defining declaration for the EDT
  // We will define one EDT for a source file
  // Get the name that we will call this EDT from its context
  string shutdownEdtName = shutdownEdtContext->getShutdownEdtFuncName();
  // Check if we have already created the EDT for the shutdown annotation
  bool found = m_astInfoManager.findEdtAstInfo(shutdownEdtName);

  if(!found) {
    SgFunctionDeclaration* edtDecl = AstBuilder::buildOcrEdtFuncDecl(shutdownEdtName, global);
    // Get the scope under which the statements of EDT will be outlined
    SgBasicBlock* basicblock = edtDecl->get_definition()->get_body();
    // parameters that make up the edt
    vector<SgInitializedName*> edt_params = AstBuilder::buildOcrEdtSignature(basicblock);
    // Insert the parameters to the function declaration
    vector<SgInitializedName*>::iterator p = edt_params.begin();
    SgFunctionParameterList* edt_paramlist = edtDecl->get_parameterList();
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
    SageInterface::insertStatementBefore(anchor, edtDecl, true);
    // Now bookkeeping
    // Build the EDT AST information
    EdtAstInfoPtr edtAstInfo = m_astInfoManager.regEdtAstInfo(shutdownEdtName);
    edtAstInfo->setTaskFuncDecl(edtDecl);
  }
  // else nothing to be done in outlining
}

void OcrTranslator::outlineForLoopBodyEdt(OcrLoopIterEdtContextPtr loopIterEdtContext, SgForStatement* forStmt) {
  SgBasicBlock* loopBodyBasicBlock = SageInterface::ensureBasicBlockAsBodyOfFor(forStmt);
  string loopBodyEdtName = loopIterEdtContext->getLoopBodyEdtName();
  // Bookkeeping: Register the EDT and get its outlining AST info
  EdtAstInfoPtr loopBodyEdtAstInfo = m_astInfoManager.regEdtAstInfo(loopBodyEdtName);

  SgSourceFile* sourcefile = loopIterEdtContext->getSourceFile();

  outlineEdt(loopBodyEdtName, loopBodyBasicBlock, sourcefile, loopBodyEdtAstInfo);

  // Insert declaration for depElem stack variables inside the EDT
  list<SgVarRefExp*> depElems = loopIterEdtContext->getDepElems();
  string loopCompEvtName = loopIterEdtContext->getOutputEvt()->get_name();
  buildLoopDepElemAST(loopBodyEdtName, depElems, loopBodyEdtAstInfo, loopCompEvtName);
  insertDepElemDecl(loopBodyEdtName, loopBodyEdtAstInfo);

  SgScopeStatement* depElemStructScope = loopBodyEdtAstInfo->getDepElemStructDecl()->get_definition();
  assert(depElemStructScope);
  EvtAstInfoPtr loopCompEvtAstInfo = m_astInfoManager.getEvtAstInfo(loopCompEvtName, depElemStructScope);
  assert(loopCompEvtAstInfo);
  // We must now register the evt guid that will be inserted by insertDepElemDecl in the loop's basic block context
  m_astInfoManager.regEvtAstInfo(loopCompEvtName, loopCompEvtAstInfo->getEvtGuidName(), loopBodyBasicBlock);
}

void OcrTranslator::outlineForLoopControlEdt(OcrLoopIterEdtContextPtr loopIterEdtContext, SgForStatement* forStmt) {
  // Now generate the EDT for loop control
  string loopControlEdtName = loopIterEdtContext->getLoopControlEdtName();
  // Register EdtAstInfo with AstInfoManager
  LoopControlEdtAstInfoPtr loopControlEdtAstInfo = m_astInfoManager.regLoopControlEdtAstInfo(loopControlEdtName);

  // We will create a basic block with just the loop condition transformed as if stmt
  SgBasicBlock* ifBasicBlock = AstBuilder::buildLoopControlIfBody(forStmt);
  SgBasicBlock* elseBasicBlock = AstBuilder::buildLoopControlElseBody();
  SgIfStmt* loopControlIfStmt = AstBuilder::buildLoopControlIfStmt(ifBasicBlock, elseBasicBlock, forStmt);
  SgBasicBlock* loopControlBasicBlock = SageBuilder::buildBasicBlock();
  SageInterface::appendStatement(loopControlIfStmt, loopControlBasicBlock);

  SgSourceFile* sourcefile = loopIterEdtContext->getSourceFile();

  outlineEdt(loopControlEdtName, loopControlBasicBlock, sourcefile, loopControlEdtAstInfo);

  // Insert declaration for depElem stack variables inside the EDT
  buildLoopDepElemAST(loopControlEdtName, loopIterEdtContext->getDepElems(), loopControlEdtAstInfo,
		      loopIterEdtContext->getOutputEvt()->get_name());
  insertDepElemDecl(loopControlEdtName, loopControlEdtAstInfo);

  // Book Keeping
  loopControlEdtAstInfo->setLoopControlIfBasicBlock(ifBasicBlock);
  loopControlEdtAstInfo->setLoopControlElseBasicBlock(elseBasicBlock);
  loopControlEdtAstInfo->setLoopControlIfStmt(loopControlIfStmt);
  loopControlEdtAstInfo->setBasicBlock(loopControlBasicBlock);
}

// First step in creating the EDTs from pragmas
// Register the AST Info for the EDT based on the type
void OcrTranslator::outlineEdts() {
  Logger::Logger lg("OcrTranslator::outlineEdts", Logger::DEBUG);
  list<string> orderedEdts = m_ocrObjectManager.getEdtTraversalOrder();
  list<string>::reverse_iterator edt = orderedEdts.rbegin();
  for( ; edt != orderedEdts.rend(); ++edt) {
    string edtname = *edt;
    Logger::debug(lg) << "Outlining " << edtname << "\n";
    OcrTaskContextPtr taskContext = m_ocrObjectManager.getOcrTaskContext(edtname);
    if(taskContext->getTaskType() == OcrTaskContext::e_TaskEdt) {
      OcrEdtContextPtr edtContext = boost::dynamic_pointer_cast<OcrEdtContext>(taskContext);
      assert(edtContext);
      // Register the AST Info object for the EDT
      EdtAstInfoPtr edtAstInfo = m_astInfoManager.regEdtAstInfo(edtname);
      // Arguments for EDT outlining
      SgBasicBlock* basicblock = edtContext->getTaskBasicBlock();
      SgSourceFile* sourcefile = edtContext->getSourceFile();
      // Outlining Info will be updated by the outlineEdt function
      outlineEdt(edtname, basicblock, sourcefile, edtAstInfo);
      // Insert declaration for depElem stack variables inside the EDT
      list<SgVarRefExp*> depElems = edtContext->getDepElems();
      buildDepElemAST(edtname, depElems, edtAstInfo);
      insertDepElemDecl(edtname, edtAstInfo);
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskLoopIter) {
      OcrLoopIterEdtContextPtr loopIterEdtContext = boost::dynamic_pointer_cast<OcrLoopIterEdtContext>(taskContext);
      assert(loopIterEdtContext);
      string loopEdtName = loopIterEdtContext->getTaskName();
      SgStatement* loopStmt = loopIterEdtContext->getLoopStmt();
      if(SgForStatement* forStmt = isSgForStatement(loopStmt)) {
	outlineForLoopBodyEdt(loopIterEdtContext, forStmt);
	// 1. First generate an empty body with empty if-else basic blocks
	// 2. Insert dependent datablocks at the beginning of the EDT
	// 3. Insert depElem struct at the beginning of the EDT
	// 4. Finally generate body of the loop control EDT.
	// NOTE - It is important that step 4 follows step 2 as step 2 generates
	// variable symbols that will be used in step 4
	outlineForLoopControlEdt(loopIterEdtContext, forStmt);
      }
      else if(isSgWhileStmt(loopStmt)) {
	throw TranslateException("While Stmt Outlining Unhandled\n");
      }
      else if(isSgDoWhileStmt(loopStmt)) {
	throw TranslateException("Do While Stmt Outlining Unhandled\n");
      }
      else {
	throw TranslateException("loopStmt is not for/while/do-while\n");
      }
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskShutDown) {
      OcrShutdownEdtContextPtr shutdownEdtContext = boost::dynamic_pointer_cast<OcrShutdownEdtContext>(taskContext);
      assert(shutdownEdtContext);
      outlineShutdownEdt(shutdownEdtContext);
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskMain) {
      OcrMainEdtContextPtr mainEdtContext = boost::dynamic_pointer_cast<OcrMainEdtContext>(taskContext);
      assert(mainEdtContext);
      assert(false);
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskSpmdRegion) {
      OcrSpmdRegionContextPtr spmdRegionContext = boost::dynamic_pointer_cast<OcrSpmdRegionContext>(taskContext);
      assert(spmdRegionContext);
      string spmdRegionEdtName = spmdRegionContext->get_name();
      // Register the AST Info object for the EDT
      SpmdRegionEdtAstInfoPtr spmdRegionEdtAstInfo = m_astInfoManager.regSpmdRegionEdtAstInfo(spmdRegionEdtName);
      // Arguments for EDT outlining
      SgBasicBlock* basicblock = spmdRegionContext->getTaskBasicBlock();
      SgSourceFile* sourcefile = spmdRegionContext->getSourceFile();
      // Outlining Info will be updated by the outlineEdt function
      outlineEdt(spmdRegionEdtName, basicblock, sourcefile, spmdRegionEdtAstInfo);
      // Insert declaration for depElem stack variables inside the EDT
      list<SgVarRefExp*> depElems = spmdRegionContext->getDepElems();
      buildDepElemAST(spmdRegionEdtName, depElems, spmdRegionEdtAstInfo);
      insertDepElemDecl(spmdRegionEdtName, spmdRegionEdtAstInfo);
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskSpmdSend) {
      // No outlining required for spmdSend
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskSpmdFinalize) {
      // No outlining necessary for spmdRankFinalize
    }
    else {
      cerr << "Unhandled Task Context in OcrTranslator::outlineEdts()\n";
      cerr << taskContext->str() << endl;
      std::terminate();
    }
  }
}

void OcrTranslator::insertDepDbkDecl(string edtName, list<OcrDbkContextPtr>& depDbks,
				     unsigned int slotbegin, TaskAstInfoPtr taskAstInfo) {
  Logger::Logger lg(" OcrTranslator::insertDepDbkDecl");
  SgFunctionDeclaration* edtDecl = taskAstInfo->getTaskFuncDecl();
  SgBasicBlock* basicblock = edtDecl->get_definition()->get_body();

  // For each item in list of datablock dependences
  // Build ASTs for the pointer declaration
  // Build ASTs for the guid declaration
  // For each dependences create the pointer and the guid
  list<OcrDbkContextPtr>::iterator dbk = depDbks.begin();
  // For each datablock set up the declaration from depv
  SgInitializedNamePtrList& args = edtDecl->get_args();
  // depv is the last element in the EDT argument list
  SgInitializedName* depv = args.back(); assert(depv);
  vector<SgStatement*> depDbksDeclStmts;
  unsigned int slot = slotbegin;
  for( ; dbk != depDbks.end(); ++dbk, ++slot) {
    OcrDbkContextPtr dbkContext = *dbk;
    string dbkname = (*dbk)->get_name();
    string dbkPtrName, guidName;
    DbkAstInfoPtr dbkAstInfo;
    if(dbkContext->getDbkType() == OcrDbkContext::DBK_mem) {
      OcrMemDbkContextPtr mdbkContext = boost::dynamic_pointer_cast<OcrMemDbkContext>(dbkContext);
      // Build the pointer
      dbkPtrName = mdbkContext->getSgInitializedName()->get_name();
      SgType* dbkPtrType = mdbkContext->getDbkPtrType();
      SgVariableDeclaration* vdecl = AstBuilder::buildDepDbkPtrDecl(dbkPtrName, dbkPtrType, slot, depv, basicblock);
      depDbksDeclStmts.push_back(vdecl);
      // Build the Guid
      string guidName(dbkPtrName+"Guid");
      SgVariableDeclaration* guidDecl = AstBuilder::buildDepDbkGuidDecl(guidName, slot, depv, basicblock);
      depDbksDeclStmts.push_back(guidDecl);
      dbkAstInfo = boost::make_shared<DbkAstInfo>(dbkname, guidName, dbkPtrName);
      // Bookkeeping
      m_astInfoManager.regDbkAstInfo(dbkname, dbkAstInfo, basicblock);
    }
    else if(dbkContext->getDbkType() == OcrDbkContext::DBK_arr) {
      OcrArrDbkContextPtr arrDbkContext = boost::dynamic_pointer_cast<OcrArrDbkContext>(dbkContext);
      SgInitializedName* arrInitializedName = arrDbkContext->getArrInitializedName();
      string arrInitName = arrInitializedName->get_name().getString();
      SgScopeStatement* arrInitNameScope = SageInterface::getEnclosingScope(arrInitializedName);
      assert(arrInitNameScope);
      // Get the AstInfo from where the datablock was created
      // We need the AstInfo to know the struct type we created for this datablock
      DbkAstInfoPtr dbkAstInfo = m_astInfoManager.getDbkAstInfo(dbkname, arrInitNameScope);
      ArrDbkAstInfoPtr arrDbkAstInfo = boost::dynamic_pointer_cast<ArrDbkAstInfo>(dbkAstInfo);
      assert(arrDbkAstInfo);
      SgType* dbkType = arrDbkAstInfo->getDbkStructType();
      // Build a pointer to the struct type
      SgPointerType* dbkPtrType = SageBuilder::buildPointerType(dbkType);
      // Build the pointer variable
      string dbkPtrName = arrInitName+"DbkPtr";
      SgVariableDeclaration* vdecl = AstBuilder::buildDepDbkPtrDecl(dbkPtrName, dbkPtrType, slot, depv, basicblock);
      depDbksDeclStmts.push_back(vdecl);
      // Build the guid
      SgVariableSymbol* dbkPtrSymbol = GetVariableSymbol(vdecl, dbkPtrName);
      string guidName(dbkPtrName+"Guid");
      SgVariableDeclaration* guidDecl = AstBuilder::buildDepDbkGuidDecl(guidName, slot, depv, basicblock);
      depDbksDeclStmts.push_back(guidDecl);
      // Build the array pointer
      SgClassDeclaration* dbkStructDecl = arrDbkAstInfo->getDbkStructDecl();
      SgVariableDeclaration* arrPtrDecl = AstBuilder::buildArrPtrDecl(arrInitializedName, dbkPtrSymbol, dbkStructDecl, basicblock);
      depDbksDeclStmts.push_back(arrPtrDecl);
      // Store the dbkAstInfo
      DbkAstInfoPtr currDbkAstInfo = boost::make_shared<ArrDbkAstInfo>(dbkname, guidName, dbkPtrName, dbkStructDecl, dbkType,
								       arrInitializedName->get_name().getString());
      m_astInfoManager.regDbkAstInfo(dbkname, currDbkAstInfo, basicblock);
    }

  } // end for
  SageInterface::prependStatementList(depDbksDeclStmts, basicblock);
}

void OcrTranslator::insertEdtsDepDbksDecl() {
  Logger::Logger lg("OcrTranslator::insertEdtsDepDbksDecl", Logger::DEBUG);
  list<string> orderedEdts = m_ocrObjectManager.getEdtTraversalOrder();
  list<string>::iterator edt = orderedEdts.begin();
  for( ; edt != orderedEdts.end(); ++edt) {
    string edtname = *edt;
    OcrTaskContextPtr taskContext = m_ocrObjectManager.getOcrTaskContext(edtname);
    if(taskContext->getTaskType() == OcrTaskContext::e_TaskEdt) {
      OcrEdtContextPtr edtContext = boost::dynamic_pointer_cast<OcrEdtContext>(taskContext);
      assert(edtContext);
      // Register the AST Info object for the EDT
      EdtAstInfoPtr edtAstInfo = m_astInfoManager.getEdtAstInfo(edtname);
      // Insert the ptr, guid for depDbks inside the EDT
      list<OcrDbkContextPtr> depDbks = edtContext->getDepDbks();
      insertDepDbkDecl(edtname, depDbks, 0, edtAstInfo);
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskLoopIter) {
      OcrLoopIterEdtContextPtr loopIterEdtContext = boost::dynamic_pointer_cast<OcrLoopIterEdtContext>(taskContext);
      string loopBodyEdtName = loopIterEdtContext->getLoopBodyEdtName();
      // Loop Body's Info is stored as a regular EdtAstInfo
      EdtAstInfoPtr loopBodyEdtAstInfo = m_astInfoManager.getEdtAstInfo(loopBodyEdtName);

      // We need to insert dep dbks in the loop control and loop body EDT
      list<OcrDbkContextPtr> depDbks = loopIterEdtContext->getDepDbks();
      insertDepDbkDecl(loopBodyEdtName, depDbks, 0, loopBodyEdtAstInfo);
      string loopControlEdtName = loopIterEdtContext->getLoopControlEdtName();
      LoopControlEdtAstInfoPtr loopControlEdtAstInfo = m_astInfoManager.getLoopControlEdtAstInfo(loopControlEdtName);
      insertDepDbkDecl(loopControlEdtName, depDbks, 0, loopControlEdtAstInfo);
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskShutDown) {
      // Nothing to be done here
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskSpmdRegion) {
      OcrSpmdRegionContextPtr spmdRegionContext = boost::dynamic_pointer_cast<OcrSpmdRegionContext>(taskContext);
      string spmdRegionEdtName = spmdRegionContext->get_name();
      SpmdRegionEdtAstInfoPtr spmdRegionEdtAstInfo = m_astInfoManager.getSpmdRegionEdtAstInfo(spmdRegionEdtName);
      assert(spmdRegionEdtAstInfo);
      list<OcrDbkContextPtr> depDbks = spmdRegionContext->getDepDbks();
      insertDepDbkDecl(spmdRegionEdtName, depDbks, 0, spmdRegionEdtAstInfo);
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskSpmdFinalize) {
      // Nothing to be done here
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskSpmdSend) {
      // Nothing to be done here
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskSpmdRecv) {
      // Nothing to be done here
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskMain) {
      // Nothing to be done here
      // This is handled separately in outlineMainEdt
    }
    else {
      throw TranslateException("Unhandled Task Context in OcrTranslator::outlineEdts()\n");
    }
  }
}

vector<SgStatement*> OcrTranslator::setupEdtOutEvt(std::string edtname, string outEvtName, SgScopeStatement* scope) {
  vector<SgStatement*> evtCreateStmts;
  // Create guid and events that this EDT has to satisfy
  // For each event create ocrGuid and the event using ocrEvtCreate
  SgType* ocrGuidType = AstBuilder::buildOcrGuidType(scope);
  string evtGuidName = outEvtName + "EvtGuid";
  SgVariableDeclaration* evtGuidDecl = SageBuilder::buildVariableDeclaration(evtGuidName, ocrGuidType, NULL, scope);
  evtCreateStmts.push_back(evtGuidDecl);
  SgVariableSymbol* evtGuidSymbol = GetVariableSymbol(evtGuidName, scope);
  SgExprStatement* evtCreateCallExp = AstBuilder::buildEvtCreateCallExp(evtGuidSymbol, scope);
  evtCreateStmts.push_back(evtCreateCallExp);
  // Now some bookkeeping
  m_astInfoManager.regEvtAstInfo(outEvtName, evtGuidName, scope);
  // Add the statements to the AST
  return evtCreateStmts;
}

vector<SgStatement*> OcrTranslator::setupEdtTemplate(string edtname, unsigned int ndeps, SgScopeStatement* scope) {
  vector<SgStatement*> edtTemplSetupStmts;
  // Build OcrGuid variable declaration for EDT template
  string edtTemplateGuidName = edtname + "TemplGuid";
  SgVariableDeclaration* edtTemplateGuidDecl = AstBuilder::buildOcrGuidEdtTemplateVarDecl(edtTemplateGuidName, scope);
  edtTemplSetupStmts.push_back(edtTemplateGuidDecl);

  // Setting up the edt template creation function call
  EdtAstInfoPtr edtAstInfo = m_astInfoManager.getEdtAstInfo(edtname);
  SgFunctionDeclaration* edtDecl = edtAstInfo->getTaskFuncDecl();
  SgType* depElemType = edtAstInfo->getDepElemTypedefType();
  SgExpression* depElemSizeVarRef;
  // Generate paramc variable expression only if we created a type for it
  if(edtAstInfo->hasDepElems()) {
    string depElemSizeName = SageInterface::generateUniqueVariableName(scope, "paramc");
    SgVariableDeclaration* depElemSizeVarDecl = AstBuilder::buildDepElemSizeVarDecl(depElemSizeName, depElemType, scope);
    edtTemplSetupStmts.push_back(depElemSizeVarDecl);
    // Additional bookkeeping
    edtAstInfo->setDepElemSizeVarName(depElemSizeName);

    SgVariableSymbol* depElemSizeVarSymbol = GetVariableSymbol(depElemSizeVarDecl, depElemSizeName);
    depElemSizeVarRef = SageBuilder::buildVarRefExp(depElemSizeVarSymbol);
  }
  else {
    SgIntVal* zeroval = SageBuilder::buildIntVal(0);
    depElemSizeVarRef = zeroval;
  }
  SgExprStatement* edtTemplateCallExp = AstBuilder::buildOcrEdtTemplateCallExp(edtTemplateGuidDecl, edtDecl, depElemSizeVarRef, ndeps, scope);
  edtTemplSetupStmts.push_back(edtTemplateCallExp);

  // Now some bookkeeping
  // Add the variable symbol of the template guid
  edtAstInfo->setTemplGuidName(edtTemplateGuidName);
  edtAstInfo->setNDeps(ndeps);

  return edtTemplSetupStmts;
}

/*!
 *\brief Function for setting up the dependent elements for an EDT
 *
 * 1. First we generate a variable declaration for the depElem struct type
 * 2. For each variable in the struct we generate an assignment statement
 * which copies the current value of the variable in the stack into the struct member variable
 * e.g., depElem.mvar = mvar;
 */
vector<SgStatement*> OcrTranslator::setupEdtDepElems(string edtName, list<SgVarRefExp*>& depElemVarList, SgScopeStatement* scope) {
  Logger::Logger lg("OcrTranslator::setupEdtDepElems");
  vector<SgStatement*> depElemSetupStmts;
  EdtAstInfoPtr edtAstInfoPtr = m_astInfoManager.getEdtAstInfo(edtName);
  // Next set up the depenedent elements
  SgType* depElemStructType = edtAstInfoPtr->getDepElemTypedefType();
  // First exit condition
  // When there are no dependent elements to be copied return empty list
  if(!edtAstInfoPtr->hasDepElems()) return depElemSetupStmts;

  string depElemVarName = SageInterface::generateUniqueVariableName(scope, "depElem");
  SgVariableDeclaration* depElemStructVar = SageBuilder::buildVariableDeclaration(depElemVarName, depElemStructType, NULL, scope);
  depElemSetupStmts.push_back(depElemStructVar);

  SgClassDeclaration* depElemStructDecl = edtAstInfoPtr->getDepElemStructDecl();
  // Build assignment statement for each member of the struct
  vector<SgStatement*> depElemVarSetupStmts = AstBuilder::buildEdtDepElemSetupStmts(depElemStructVar, depElemStructDecl, depElemVarList);
  // Copy the assignment statement list back in the setup stmts
  depElemSetupStmts.insert(depElemSetupStmts.end(), depElemVarSetupStmts.begin(), depElemVarSetupStmts.end());
  // Bookkeeping
  edtAstInfoPtr->setDepElemStructName(depElemVarName);

  return depElemSetupStmts;
}

vector<SgStatement*> OcrTranslator::setupLoopEdtDepElems(string edtName, list<SgVarRefExp*>& depElemVarList, string loopCompEvtName, SgScopeStatement* scope) {
  Logger::Logger lg("OcrTranslator::setupLoopEdtDepElems");
  vector<SgStatement*> depElemSetupStmts;
  EdtAstInfoPtr edtAstInfoPtr = m_astInfoManager.getEdtAstInfo(edtName);
  // Next set up the depenedent elements
  SgType* depElemStructType = edtAstInfoPtr->getDepElemTypedefType();

  string depElemVarName = SageInterface::generateUniqueVariableName(scope, "depElem");
  SgVariableDeclaration* depElemStructVar = SageBuilder::buildVariableDeclaration(depElemVarName, depElemStructType, NULL, scope);
  depElemSetupStmts.push_back(depElemStructVar);

  SgClassDeclaration* depElemStructDecl = edtAstInfoPtr->getDepElemStructDecl();
  if(!depElemVarList.empty()) {
    // Build assignment statement for each member of the struct
    vector<SgStatement*> depElemVarSetupStmts = AstBuilder::buildEdtDepElemSetupStmts(depElemStructVar, depElemStructDecl, depElemVarList);
    // Copy the assignment statement list back in the setup stmts
    depElemSetupStmts.insert(depElemSetupStmts.end(), depElemVarSetupStmts.begin(), depElemVarSetupStmts.end());
  }
  // Now generate the guid copy for the loop's completion event
  // First get the guid name we gave for the Loop's completion event inside the struct
  SgClassDefinition* depElemStructDefn = depElemStructDecl->get_definition();
  EvtAstInfoPtr depElemOutEvtAstInfo = m_astInfoManager.getEvtAstInfo(loopCompEvtName, depElemStructDefn);
  assert(depElemOutEvtAstInfo);
  string depElemOutEvtGuidName = depElemOutEvtAstInfo->getEvtGuidName();
  SgVariableSymbol* depElemMemberVarSymbol = GetVariableSymbol(depElemOutEvtGuidName, depElemStructDefn);
  assert(depElemMemberVarSymbol);

  // Now get the guid name we gave for the loop completetion event in an earlier scope
  EvtAstInfoPtr outEvtAstInfo = m_astInfoManager.getEvtAstInfo(loopCompEvtName, scope);
  assert(outEvtAstInfo);
  string outEvtGuidName = outEvtAstInfo->getEvtGuidName();
  SgVariableSymbol* depElemVarSymbol = GetVariableSymbol(outEvtGuidName, scope);
  SgVariableSymbol* depElemStructVarSymbol = GetVariableSymbol(depElemStructVar, depElemVarName);
  SgExprStatement* outEvtSetupStmt = AstBuilder::buildEdtDepElemSetupStmt(depElemStructVarSymbol, depElemMemberVarSymbol, depElemVarSymbol);
  depElemSetupStmts.push_back(outEvtSetupStmt);
  // Bookkeeping
  edtAstInfoPtr->setDepElemStructName(depElemVarName);

  return depElemSetupStmts;
}

vector<SgStatement*> OcrTranslator::setupEdtDepElems(string edtname, OcrEdtContextPtr edtContext,
						     list<SgVarRefExp*>& depElemVarList, SgScopeStatement* scope) {
  Logger::Logger lg("OcrTranslator::setupEdtDepElems");
  vector<SgStatement*> depElemSetupStmts;
  EdtAstInfoPtr edtAstInfoPtr = m_astInfoManager.getEdtAstInfo(edtname);
  // Next set up the depenedent elements
  SgType* depElemStructType = edtAstInfoPtr->getDepElemTypedefType();
  string depElemVarName = edtname+"DepElem";
  SgVariableDeclaration* depElemStructVar = SageBuilder::buildVariableDeclaration(depElemVarName, depElemStructType, NULL, scope);
  depElemSetupStmts.push_back(depElemStructVar);

  SgClassDeclaration* depElemStructDecl = edtAstInfoPtr->getDepElemStructDecl();
  vector<SgStatement*> depElemVarSetupStmts = AstBuilder::buildEdtDepElemSetupStmts(depElemStructVar, depElemStructDecl, depElemVarList);
  depElemSetupStmts.insert(depElemSetupStmts.end(), depElemVarSetupStmts.begin(), depElemVarSetupStmts.end());

  // If this is a depElem of loop iteration EDT add a setup for the loop completion event
  if(boost::static_pointer_cast<OcrTaskContext>(edtContext)->getTaskType() == OcrTaskContext::e_TaskLoopIter) {
    OcrLoopIterEdtContextPtr loopIterEdtContext = boost::dynamic_pointer_cast<OcrLoopIterEdtContext>(edtContext);
    string loopCompEvtName = loopIterEdtContext->getOutputEvt()->get_name();
    // OutEvt created at the struct scope
    SgClassDefinition* depElemStructDefn = depElemStructDecl->get_definition();
    EvtAstInfoPtr depElemOutEvtAstInfo = m_astInfoManager.getEvtAstInfo(loopCompEvtName, depElemStructDefn);
    assert(depElemOutEvtAstInfo);
    string depElemOutEvtGuidName = depElemOutEvtAstInfo->getEvtGuidName();
    SgVariableSymbol* depElemMemberVarSymbol = GetVariableSymbol(depElemOutEvtGuidName, depElemStructDefn);
    assert(depElemMemberVarSymbol);

    // OutEvt created at the local scope
    EvtAstInfoPtr outEvtAstInfo = m_astInfoManager.getEvtAstInfo(loopCompEvtName, scope);
    assert(outEvtAstInfo);
    string outEvtGuidName = outEvtAstInfo->getEvtGuidName();
    SgVariableSymbol* depElemVarSymbol = GetVariableSymbol(outEvtGuidName, scope);
    SgVariableSymbol* depElemStructVarSymbol = GetVariableSymbol(depElemStructVar, depElemVarName);
    SgExprStatement* outEvtSetupStmt = AstBuilder::buildEdtDepElemSetupStmt(depElemStructVarSymbol, depElemMemberVarSymbol, depElemVarSymbol);
    depElemSetupStmts.push_back(outEvtSetupStmt);
  }
  edtAstInfoPtr->setDepElemStructName(depElemVarName);
  return depElemSetupStmts;
}

// //! Method to setup the ocrEdtCreate function call expression
vector<SgStatement*> OcrTranslator::setupEdtCreate(string edtname, string outEvtName, bool isFinishEdt, SgScopeStatement* scope) {
  vector<SgStatement*> edtCreateSetupStmts;
  EdtAstInfoPtr edtAstInfoPtr = m_astInfoManager.getEdtAstInfo(edtname);
  string edtGuidName = edtname+"Guid";
  SgType* ocrGuidType = AstBuilder::buildOcrGuidType(scope);
  SgVariableDeclaration* edtGuidDecl = SageBuilder::buildVariableDeclaration(edtGuidName, ocrGuidType, NULL, scope);
  edtCreateSetupStmts.push_back(edtGuidDecl);

  SgVariableSymbol* edtGuidSymbol = GetVariableSymbol(edtGuidDecl, edtGuidName);
  // We need the edtTemplateGuid
  string edtTemplGuidName = edtAstInfoPtr->getEdtTemplateGuidName();
  SgVariableSymbol* edtTemplateGuidSymbol = GetVariableSymbol(edtTemplGuidName, scope);
  // We need the paramc symbol
  string paramcName = edtAstInfoPtr->getDepElemSizeVarName();
  SgVariableSymbol* paramcSymbol = GetVariableSymbol(paramcName, scope);
  // We also need the depElemStructSymbol
  string depElemStructName = edtAstInfoPtr->getDepElemStructName();
  SgVariableSymbol* depElemStructSymbol = GetVariableSymbol(depElemStructName, scope);
  // Get depc
  int ndeps = edtAstInfoPtr->getNDeps();
  SgVariableSymbol* outEvtGuidSymbol = NULL;
  if(outEvtName.length() != 0) {
    EvtAstInfoPtr outEvtAstInfo = m_astInfoManager.getEvtAstInfo(outEvtName, scope);
    string outEvtGuidName  = outEvtAstInfo->getEvtGuidName();
    outEvtGuidSymbol = GetVariableSymbol(outEvtGuidName, scope);
  }
  SgExprStatement* edtCreateCallExp = AstBuilder::buildOcrEdtCreateCallExp(edtGuidSymbol, edtTemplateGuidSymbol, paramcSymbol,
									   depElemStructSymbol, ndeps, outEvtGuidSymbol, isFinishEdt, scope);
  edtCreateSetupStmts.push_back(edtCreateCallExp);
  // Now some bookkeeping
  edtAstInfoPtr->setEdtGuidName(edtGuidName);
  return edtCreateSetupStmts;
}

vector<SgStatement*> OcrTranslator::setupEdtDepDbks(string edtname, list<OcrDbkContextPtr>& dbkList, unsigned int slotBegin, SgScopeStatement* scope) {
  Logger::Logger lg("OcrTranslator::setupEdtDepDbks");
  EdtAstInfoPtr edtAstInfoPtr = m_astInfoManager.getEdtAstInfo(edtname);
  string edtGuidName = edtAstInfoPtr->getEdtGuidName();
  SgVariableSymbol* edtGuidSymbol = GetVariableSymbol(edtGuidName, scope);
  // for each datablock add ocrAddDependence
  list<OcrDbkContextPtr>::iterator l = dbkList.begin();
  vector<SgStatement*> dbkSetupStmts;
  for(int slot = slotBegin ; l != dbkList.end(); ++l, ++slot) {
    OcrDbkContextPtr dbkContext = *l;
    string dbkname = dbkContext->get_name();
    DbkAstInfoPtr dbkAstInfoPtr = m_astInfoManager.getDbkAstInfo(dbkname, scope);
    // Get the GuidSymbolTable for the current scope
    string dbkGuidName = dbkAstInfoPtr->getDbkGuidName();
    SgVariableSymbol* dbkGuidSymbol = GetVariableSymbol(dbkGuidName, scope);
    SgExprStatement* ocrAddDependenceCallExp = AstBuilder::buildOcrAddDependenceCallExp(dbkGuidSymbol, edtGuidSymbol, slot,
											AstBuilder::DbkMode::DB_DEFAULT_MODE, scope);
    dbkSetupStmts.push_back(static_cast<SgStatement*>(ocrAddDependenceCallExp));
  }
  return dbkSetupStmts;
}

vector<SgStatement*> OcrTranslator::setupEdtDepEvts(string edtname, list<OcrEvtContextPtr>& evtList, unsigned int slotBegin, SgScopeStatement* scope) {
  EdtAstInfoPtr edtAstInfoPtr = m_astInfoManager.getEdtAstInfo(edtname);
  string edtGuidName = edtAstInfoPtr->getEdtGuidName();
  SgVariableSymbol* edtGuidSymbol = GetVariableSymbol(edtGuidName, scope);
  // For each event add ocrAddDependence
  list<OcrEvtContextPtr>::iterator e = evtList.begin();
  // The starting slot dependent events is after the datablocks
  unsigned int slotIndex = slotBegin;
  vector<SgStatement*> evtDepSetupStmts;
  for( ; e != evtList.end(); ++e, ++slotIndex) {
    OcrEvtContextPtr evt = *e;
    string evtname = evt->get_name();
    EvtAstInfoPtr evtAstInfo = m_astInfoManager.getEvtAstInfo(evtname, scope);
    string evtGuidName = evtAstInfo->getEvtGuidName();
    SgVariableSymbol* evtGuid = GetVariableSymbol(evtGuidName, scope);
    SgExprStatement* ocrAddDependenceCallExp = AstBuilder::buildOcrAddDependenceCallExp(evtGuid, edtGuidSymbol, slotIndex,
											AstBuilder::DbkMode::DB_MODE_NULL, scope);
    evtDepSetupStmts.push_back(static_cast<SgStatement*>(ocrAddDependenceCallExp));
  }
  return evtDepSetupStmts;
}

// Satisfies the event with NULL_GUID
SgExprStatement* OcrTranslator::setupEvtSatisfy(string evtName, SgScopeStatement* scope) {
  EvtAstInfoPtr evtAstInfo = m_astInfoManager.getEvtAstInfo(evtName, scope);
  string evtGuidName = evtAstInfo->getEvtGuidName();
  SgVariableSymbol* evtGuidSymbol = GetVariableSymbol(evtGuidName, scope);
  SgExprStatement* evtSatisfyCallExp = AstBuilder::buildEvtSatisfyCallExp(evtGuidSymbol, scope);
  return evtSatisfyCallExp;
}

void OcrTranslator::removeOcrTaskPragma(string edtname, SgBasicBlock* taskBasicBlock, SgPragmaDeclaration* taskPragma) {
  // Assert the basic block is empty
  SgStatementPtrList& statements = taskBasicBlock->get_statements();
  assert(statements.size() == 0);

  SageInterface::removeStatement(taskPragma);
  SageInterface::removeStatement(taskBasicBlock);
}

vector<SgStatement*> OcrTranslator::setupForLoopCompEvt(string loopCompEvtName, SgScopeStatement* scope) {
  vector<SgStatement*> setupStmts;
  // Create guid and events that this EDT has to satisfy
  // For each event create ocrGuid and the event using ocrEvtCreate
  SgType* ocrGuidType = AstBuilder::buildOcrGuidType(scope);
  string loopCompEvtGuidName = loopCompEvtName + "Guid";
  SgVariableDeclaration* loopCompEvtGuidDecl = SageBuilder::buildVariableDeclaration(loopCompEvtGuidName, ocrGuidType, NULL, scope);
  setupStmts.push_back(loopCompEvtGuidDecl);
  SgVariableSymbol* loopCompEvtGuidSymbol = GetVariableSymbol(loopCompEvtGuidName, scope);
  SgExprStatement* loopCompEvtCreateCallExp = AstBuilder::buildEvtCreateCallExp(loopCompEvtGuidSymbol, scope);
  setupStmts.push_back(loopCompEvtCreateCallExp);
  // Now some bookkeeping
  m_astInfoManager.regEvtAstInfo(loopCompEvtName, loopCompEvtGuidName, scope);
  return setupStmts;
}

/*!
 * \brief setup an OCR EDT
 *
 * EDT Setup consists of following steps
 * 1. Setup the output event for the EDT
 * 2. Setup the template creation
 * 3. Setup the EDT creation
 * 4. Add dependences to EDT from datablocks
 * 5. Add dependences to EDT from events
 */
void OcrTranslator::setupEdt(string edtName, OcrEdtContextPtr edtContext) {
  SgPragmaDeclaration* taskPragma = edtContext->getTaskPragma();
  SgScopeStatement* scope = SageInterface::getEnclosingScope(taskPragma);

  // 1. Setup the output event for the EDT
  string outEvtName = edtContext->getOutputEvt()->get_name();
  vector<SgStatement*> outEvtSetupStmts = setupEdtOutEvt(edtName, outEvtName, scope);
  SageInterface::insertStatementListBefore(taskPragma, outEvtSetupStmts);
  outEvtSetupStmts.clear();

  // 2. Setup the template creation for the EDT
  unsigned int ndeps = edtContext->getNumDepDbks() + edtContext->getNumDepEvts();
  vector<SgStatement*> edtTemplSetupStmts = setupEdtTemplate(edtName, ndeps, scope);
  SageInterface::insertStatementListBefore(taskPragma, edtTemplSetupStmts);
  edtTemplSetupStmts.clear();

  // 3. Setup the depElems
  list<SgVarRefExp*> depElems = edtContext->getDepElems();
  vector<SgStatement*> depElemSetupStmts = setupEdtDepElems(edtName, depElems, scope);
  SageInterface::insertStatementListBefore(taskPragma, depElemSetupStmts);
  depElemSetupStmts.clear();

  OcrEvtContextPtr outEvtContext = edtContext->getOutputEvt();
  bool isFinishEdt = edtContext->isFinishEdt();
  vector<SgStatement*> edtCreateSetupStmts = setupEdtCreate(edtName, outEvtContext->get_name(), isFinishEdt, scope);
  SageInterface::insertStatementListBefore(taskPragma, edtCreateSetupStmts);
  edtCreateSetupStmts.clear();

  list<OcrDbkContextPtr> depDbks = edtContext->getDepDbks();
  unsigned int slotBegin = 0;
  vector<SgStatement*> depDbkSetupStmts = setupEdtDepDbks(edtName, depDbks, slotBegin, scope);
  SageInterface::insertStatementListBefore(taskPragma, depDbkSetupStmts);
  depDbkSetupStmts.clear();

  list<OcrEvtContextPtr> depEvts = edtContext->getDepEvts();
  slotBegin = depDbks.size();
  vector<SgStatement*> depEvtSetupStmts = setupEdtDepEvts(edtName, depEvts, slotBegin, scope);
  SageInterface::insertStatementListBefore(taskPragma, depEvtSetupStmts);
  depEvtSetupStmts.clear();

  // Finally remove the pragma for each edt
  SgBasicBlock* taskBasicBlock = edtContext->getTaskBasicBlock();
  removeOcrTaskPragma(edtName, taskBasicBlock, taskPragma);
}

void OcrTranslator::setupLoopControlEdtBody(OcrLoopIterEdtContextPtr loopIterEdtContext) {
  string loopControlEdtName = loopIterEdtContext->getLoopControlEdtName();
  LoopControlEdtAstInfoPtr loopControlEdtAstInfo = m_astInfoManager.getLoopControlEdtAstInfo(loopControlEdtName);
  SgBasicBlock* loopControlBasicBlock = loopControlEdtAstInfo->getTaskFuncDecl()->get_definition()->get_body();
  SgBasicBlock* ifBasicBlock = loopControlEdtAstInfo->getLoopControlIfBasicBlock();
  SgBasicBlock* elseBasicBlock = loopControlEdtAstInfo->getLoopControlElseBasicBlock();

  // Get the registered guid name with the depElem struct declaration
  SgScopeStatement* depElemStructScope = loopControlEdtAstInfo->getDepElemStructDecl()->get_definition();
  assert(depElemStructScope);
  EvtAstInfoPtr loopCompEvtAstInfo = m_astInfoManager.getEvtAstInfo(loopIterEdtContext->getOutputEvt()->get_name(), depElemStructScope);
  string loopCompEvtGuidName = loopCompEvtAstInfo->getEvtGuidName();
  // We must now register the output evts AST info with the loop's basic block
  m_astInfoManager.regEvtAstInfo(loopIterEdtContext->getOutputEvt()->get_name(), loopCompEvtGuidName, loopControlBasicBlock);

  // Outlining control EDT has the following steps
  // 2. Setup the output event for the body EDT
  // 3. Setup dependent element for the body EDT
  // 4. Setup the EDT template for the body EDT
  // 5. Setup the EDT creation for the body EDT
  // 6. Setup the EDT datablocks for the body EDT
  // 7. Setup the depElem for next iteration of the control EDT
  // 8. Setup EDT template for next iteration of control EDT
  // 9. Setup EDT creation for next iteration control EDT
  // 10. Setup depDbks for next iteration control EDT
  // 11. Setup iteration complete event as a dependency to the next iteration control EDT
  string loopBodyEdtName = loopIterEdtContext->getLoopBodyEdtName();

  vector<SgStatement*> stmtsToInsert;
  vector<SgStatement*> setupStmts;

  // 2. Setup the output event for the body EDT
  string loopBodyOutEvtName = loopBodyEdtName + "Out";
  m_ocrObjectManager.registerOcrEvt(loopBodyOutEvtName);
  setupStmts = setupEdtOutEvt(loopBodyEdtName, loopBodyOutEvtName, ifBasicBlock);
  stmtsToInsert.insert(stmtsToInsert.end(), setupStmts.begin(), setupStmts.end());
  setupStmts.clear();

  // 3. Setup dependent element for the body EDT
  list<SgVarRefExp*> depElems = loopIterEdtContext->getDepElems();
  setupStmts = setupLoopEdtDepElems(loopBodyEdtName, depElems, loopIterEdtContext->getOutputEvt()->get_name(), ifBasicBlock);
  stmtsToInsert.insert(stmtsToInsert.end(), setupStmts.begin(), setupStmts.end());
  setupStmts.clear();

  // 4. Setup the EDT template for the body EDT
  list<OcrDbkContextPtr> depDbks = loopIterEdtContext->getDepDbks();
  setupStmts = setupEdtTemplate(loopBodyEdtName, depDbks.size(), ifBasicBlock);
  stmtsToInsert.insert(stmtsToInsert.end(), setupStmts.begin(), setupStmts.end());
  setupStmts.clear();

  // 5. Setup the EDT creation for the body EDT
  // Setup the body EDT as finish EDT
  setupStmts = setupEdtCreate(loopBodyEdtName, loopBodyOutEvtName, true, ifBasicBlock);
  stmtsToInsert.insert(stmtsToInsert.end(), setupStmts.begin(), setupStmts.end());
  setupStmts.clear();

  // 6. Setup the EDT datablocks for the body EDT
  // Setup dependent datablocks for the body EDT
  setupStmts = setupEdtDepDbks(loopBodyEdtName, depDbks, 0, ifBasicBlock);
  stmtsToInsert.insert(stmtsToInsert.end(), setupStmts.begin(), setupStmts.end());
  setupStmts.clear();

  // Add the statements to the AST
  SageInterface::prependStatementList(stmtsToInsert, ifBasicBlock);
  stmtsToInsert.clear();

  // 7. Setup the depElem for next iteration of the control EDT
  // Now we can to setup the next iteration control EDT
  setupStmts = setupLoopEdtDepElems(loopControlEdtName, depElems, loopIterEdtContext->getOutputEvt()->get_name(), ifBasicBlock);
  SageInterface::appendStatementList(setupStmts, ifBasicBlock);
  setupStmts.clear();

  // 8. Setup EDT template for next iteration of control EDT
  // Number of dependences for the next iteration is depDbks.size() + 1 for the output of bodyEDT outEvt
  setupStmts = setupEdtTemplate(loopControlEdtName, depDbks.size() + 1, ifBasicBlock);
  SageInterface::appendStatementList(setupStmts, ifBasicBlock);
  setupStmts.clear();

  // 9. Setup EDT creation for next iteration control EDT
  setupStmts = setupEdtCreate(loopControlEdtName, "", false, ifBasicBlock);
  SageInterface::appendStatementList(setupStmts, ifBasicBlock);
  setupStmts.clear();

  // 10. Setup depDbks for next iteration control EDT
  // Setup dependent datablocks for the body EDT
  setupStmts = setupEdtDepDbks(loopControlEdtName, depDbks, 0, ifBasicBlock);
  SageInterface::appendStatementList(setupStmts, ifBasicBlock);
  setupStmts.clear();

  // 11. Setup iteration complete event as a dependency to t
  OcrEvtContextPtr loopBodyOutEvt = m_ocrObjectManager.getOcrEvtContext(loopBodyOutEvtName);
  list<OcrEvtContextPtr> loopControlEdtDepEvts;
  loopControlEdtDepEvts.push_back(loopBodyOutEvt);
  unsigned int slotBegin = depDbks.size();
  setupStmts = setupEdtDepEvts(loopControlEdtName, loopControlEdtDepEvts, slotBegin, ifBasicBlock);
  SageInterface::appendStatementList(setupStmts, ifBasicBlock);
  setupStmts.clear();

  // Finally setup the ovrEventSatisfy in the elseBasicBlock
  string loopCompEvtName = loopIterEdtContext->getOutputEvt()->get_name();
  SgExprStatement* evtSatisfyCallExp = setupEvtSatisfy(loopCompEvtName, elseBasicBlock);
  SageInterface::appendStatement(evtSatisfyCallExp, elseBasicBlock);
}

void OcrTranslator::setupLoopControlEdt(OcrLoopIterEdtContextPtr loopIterEdtContext) {
  SgPragmaDeclaration* taskPragma = loopIterEdtContext->getPragma();
  SgScopeStatement* scope = SageInterface::getEnclosingScope(taskPragma);
  string loopControlEdtName = loopIterEdtContext->getLoopControlEdtName();

  vector<SgStatement*> loopInitStmts = loopIterEdtContext->getLoopInitStmts();
  if(loopInitStmts.size() > 0) {
    SageInterface::insertStatementListBefore(taskPragma, loopInitStmts);
  }
  loopInitStmts.clear();

  vector<SgStatement*> setupStmts;
  // 1. Setup the output event for the EDT
  string outEvtName = loopIterEdtContext->getOutputEvt()->get_name();
  setupStmts = setupEdtOutEvt(loopControlEdtName, outEvtName, scope);
  SageInterface::insertStatementListBefore(taskPragma, setupStmts);
  setupStmts.clear();

  list<SgVarRefExp*> depElems = loopIterEdtContext->getDepElems();
  setupStmts = setupLoopEdtDepElems(loopControlEdtName, depElems, outEvtName, scope);
  SageInterface::insertStatementListBefore(taskPragma, setupStmts);
  setupStmts.clear();

  // 2. Setup the template creation for the EDT
  unsigned int ndeps = loopIterEdtContext->getNumDepDbks() + loopIterEdtContext->getNumDepEvts();
  setupStmts = setupEdtTemplate(loopControlEdtName, ndeps, scope);
  SageInterface::insertStatementListBefore(taskPragma, setupStmts);
  setupStmts.clear();

  bool isFinishEdt = loopIterEdtContext->isFinishEdt();
  setupStmts = setupEdtCreate(loopControlEdtName, "", isFinishEdt, scope);
  SageInterface::insertStatementListBefore(taskPragma, setupStmts);
  setupStmts.clear();

  list<OcrDbkContextPtr> depDbks = loopIterEdtContext->getDepDbks();
  unsigned int slotBegin = 0;
  setupStmts = setupEdtDepDbks(loopControlEdtName, depDbks, slotBegin, scope);
  SageInterface::insertStatementListBefore(taskPragma, setupStmts);
  setupStmts.clear();

  list<OcrEvtContextPtr> depEvts = loopIterEdtContext->getDepEvts();
  slotBegin = depDbks.size();
  setupStmts = setupEdtDepEvts(loopControlEdtName, depEvts, slotBegin, scope);
  SageInterface::insertStatementListBefore(taskPragma, setupStmts);
  setupStmts.clear();
  // assert(false);

  // Remove the pragma and the basic block
  SgStatement* loopStmt = loopIterEdtContext->getLoopStmt();
  SageInterface::removeStatement(taskPragma);
  SageInterface::removeStatement(loopStmt);
}

/*!
 * \brief setup spmdEdtSpawn call
 *
 * 1. Setup template guid for spmdRegionEdt
 * 2. Setup paramc for spmdRegionEdt
 * 3. Setup paramv for spmdRegionEdt
 * 4. Setup depc for spmdRegionEdt
 * 5. Setup depv[] for spmdRegionEdt
 * 6. Setup ocrDbAccessMode_t[] for spmdRegionEdt
 * 7. Setup output event
 * 8. Call spmdEdtSpawn with above arguments
 */
void OcrTranslator::setupSpmdRegionEdt(string spmdRegionEdtName, OcrSpmdRegionContextPtr spmdRegionContext) {
  SgPragmaDeclaration* pragmaStmt = spmdRegionContext->getTaskPragma();
  SgScopeStatement* scope = SageInterface::getScope(pragmaStmt);

  // 7. Setup output event
  vector<SgStatement*> setupStmts;
  OcrEvtContextPtr outEvt = spmdRegionContext->getOutputEvt();
  setupStmts = setupEdtOutEvt(spmdRegionEdtName, outEvt->get_name(), scope);
  SageInterface::insertStatementListBefore(pragmaStmt, setupStmts);
  setupStmts.clear();

  // 1. Setup template guid for spmdRegionEdt
  // 2. Setup paramc for spmdRegionEdt
  unsigned int ndeps = spmdRegionContext->getNumDepDbks() + spmdRegionContext->getNumDepEvts();
  setupStmts = setupEdtTemplate(spmdRegionEdtName, ndeps, scope);
  SageInterface::insertStatementListBefore(pragmaStmt, setupStmts);

  // 3. Setup paramv for spmdRegionEdt
  list<SgVarRefExp*> depElems = spmdRegionContext->getDepElems();
  setupStmts = setupEdtDepElems(spmdRegionEdtName, depElems, scope);
  SageInterface::insertStatementListBefore(pragmaStmt, setupStmts);
  setupStmts.clear();

  // 5. Setup depv[] for spmdRegionEdt
  // Create an array variable of ocrGuid_t[]
  string depvGuidArrName = SageInterface::generateUniqueVariableName(scope, "depvGuids");
  SgVariableDeclaration* depvGuidArrVarDecl = AstBuilder::buildGuidArrVarDecl(depvGuidArrName, ndeps, scope);
  SageInterface::insertStatementBefore(pragmaStmt, depvGuidArrVarDecl);

  SgVariableSymbol* depvGuidArrSymbol = GetVariableSymbol(depvGuidArrVarDecl, depvGuidArrName);

  // Copy the guids of dependent datablocks and dependent events in to the array
  list<OcrDbkContextPtr> depDbks = spmdRegionContext->getDepDbks();
  list<OcrDbkContextPtr>::iterator dbk = depDbks.begin();
  int index;
  for(int index=0; dbk != depDbks.end(); ++dbk, index++) {
    string dbkName = (*dbk)->get_name();
    string dbkGuidName = m_astInfoManager.getDbkAstInfo(dbkName, scope)->getDbkGuidName();
    SgVariableSymbol* dbkGuidSymbol = GetVariableSymbol(dbkGuidName, scope);
    SgStatement* guidCopyStmt = AstBuilder::buildGuidArrCopyStmt(dbkGuidSymbol, depvGuidArrSymbol, index);
    SageInterface::insertStatementBefore(pragmaStmt, guidCopyStmt);
  }

  list<OcrEvtContextPtr> depEvts = spmdRegionContext->getDepEvts();
  list<OcrEvtContextPtr>::iterator evt = depEvts.begin();
  for( ; evt != depEvts.end(); ++evt, index++) {
    string evtName = (*evt)->get_name();
    string evtGuidName = m_astInfoManager.getEvtAstInfo(evtName, scope)->getEvtGuidName();
    SgVariableSymbol* evtGuidSymbol = GetVariableSymbol(evtGuidName, scope);
    SgStatement* guidCopyStmt = AstBuilder::buildGuidArrCopyStmt(evtGuidSymbol, depvGuidArrSymbol, index);
    SageInterface::insertStatementBefore(pragmaStmt, guidCopyStmt);
  }

  // 6. Setup ocrDbAccessMode_t[] for spmdRegionEdt
  string dbAccessModeArrName = SageInterface::generateUniqueVariableName(scope, "dbAcessModes");
  SgVariableDeclaration* dbAccessModeArrVarDecl = AstBuilder::buildDbAccessModeArrVarDecl(dbAccessModeArrName, ndeps, scope);
  SageInterface::insertStatementBefore(pragmaStmt, dbAccessModeArrVarDecl);
  SgVariableSymbol* dbAccessModeArrSymbol = GetVariableSymbol(dbAccessModeArrVarDecl, dbAccessModeArrName);

  // Add datablocks in the default mode now
  // This will change depending on the annotation
  dbk = depDbks.begin();
  for(index=0; dbk != depDbks.end(); ++dbk, index++) {
    SgStatement* accessModeAssignStmt = AstBuilder::buildDbAccessModeAssignStmt(dbAccessModeArrSymbol, AstBuilder::DbkMode::DB_DEFAULT_MODE, index);
    SageInterface::insertStatementBefore(pragmaStmt, accessModeAssignStmt);
  }
  // Events are added with DB_MODE_NULL
  // Events may carry datablocks and this could be changed
  evt = depEvts.begin();
  for( ; evt != depEvts.end(); ++evt, index++) {
    SgStatement* accessModeAssignStmt = AstBuilder::buildDbAccessModeAssignStmt(dbAccessModeArrSymbol, AstBuilder::DbkMode::DB_MODE_NULL, index);
    SageInterface::insertStatementBefore(pragmaStmt, accessModeAssignStmt);
  }

  // 8. Call spmdEdtSpawn with above arguments
  SpmdRegionEdtAstInfoPtr spmdRegionEdtAstInfo = m_astInfoManager.getSpmdRegionEdtAstInfo(spmdRegionEdtName);
  string edtTemplGuidName = spmdRegionEdtAstInfo->getEdtTemplateGuidName();
  // first argument
  SgVariableSymbol* edtTemplGuidSymbol = GetVariableSymbol(edtTemplGuidName, scope);
  // second argument
  int ntasks = spmdRegionContext->getNTasks();
  // third argument
  SgVariableSymbol* depElemStructSymbol = NULL;
  // fourth argument
  SgVariableSymbol* depElemSizeVarSymbol = NULL;
  if(spmdRegionEdtAstInfo->hasDepElems()) {
    string depElemStructName = spmdRegionEdtAstInfo->getDepElemStructName();
    string depElemSizeVarName = spmdRegionEdtAstInfo->getDepElemSizeVarName();
    depElemStructSymbol = GetVariableSymbol(depElemStructName, scope);
    depElemSizeVarSymbol = GetVariableSymbol(depElemSizeVarName, scope);
  }
  // fifth argument is ndeps
  // sixth argument is depvGuidArrSymbol
  // seventh argument is dbAccessModeArrSymbol
  // eighth argument is ranksPerAffinity
  int ranksPerAffinity = 0;
  EvtAstInfoPtr outEvtAstInfo = m_astInfoManager.getEvtAstInfo(outEvt->get_name(), scope);
  string outEvtGuidName = outEvtAstInfo->getEvtGuidName();
  // ninth argument is outEvtGuid symbol
  SgVariableSymbol* outEvtGuidSymbol = GetVariableSymbol(outEvtGuidName, scope);
  SgStatement* spmdEdtSpawnStmt = AstBuilder::buildEdtSpawnCallExp(edtTemplGuidSymbol, ntasks, depElemStructSymbol,
								   depElemSizeVarSymbol, ndeps, depvGuidArrSymbol, dbAccessModeArrSymbol,
								   ranksPerAffinity, outEvtGuidSymbol, scope);
  SageInterface::insertStatementBefore(pragmaStmt, spmdEdtSpawnStmt);
  // Remove the pragma
  SageInterface::removeStatement(pragmaStmt);
  SageInterface::removeStatement(spmdRegionContext->getTaskBasicBlock());
}

void OcrTranslator::setupSpmdFinalize(string spmdFinalizeName, OcrSpmdFinalizeContextPtr spmdFinalizeContext) {
  Logger::Logger lg("OcrTranslator::setupSpmdFinalize", Logger::DEBUG);
  list<OcrEvtContextPtr> depEvts = spmdFinalizeContext->getDepEvts();
  SgPragmaDeclaration* pragmaStmt = spmdFinalizeContext->getPragma();
  SgScopeStatement* scope = SageInterface::getScope(pragmaStmt);

  SgVariableSymbol* triggerEvtGuidSymbol = NULL;
  if(depEvts.empty()) {
    // Generate spmdRankFinalize with NULL_GUID
    SgExprStatement* spmdRankFinalizeCallExp = AstBuilder::buildSpmdRankFinalizeCallExp(triggerEvtGuidSymbol, scope);
    SageInterface::insertStatementBefore(pragmaStmt, spmdRankFinalizeCallExp, true);
  }
  else {
    // Create a trigger event
    // Add all dependent events as dependencies to the trigger event using ocrAddDependence
    string triggerEvtName = SageInterface::generateUniqueVariableName(scope, "trigger");
    SgVariableDeclaration* triggerEvtGuidDecl = AstBuilder::buildGuidVarDecl(triggerEvtName, scope);
    SageInterface::insertStatementBefore(pragmaStmt, triggerEvtGuidDecl, true);
    // Create the event
    triggerEvtGuidSymbol = GetVariableSymbol(triggerEvtGuidDecl, triggerEvtName);
    SgExprStatement* triggerEvtCreateStmt = AstBuilder::buildEvtCreateCallExp(triggerEvtGuidSymbol, scope);
    SageInterface::insertStatementBefore(pragmaStmt, triggerEvtCreateStmt, true);
    // Add dependences from depEvts to triggerEvt
    list<OcrEvtContextPtr>::iterator evt = depEvts.begin();
    for(int slot = 0; evt != depEvts.end(); ++evt, ++slot) {
      EvtAstInfoPtr depEvtAstInfo = m_astInfoManager.getEvtAstInfo((*evt)->get_name(), scope);
      string depEvtGuidName = depEvtAstInfo->getEvtGuidName();
      SgVariableSymbol* depEvtGuidSymbol = GetVariableSymbol(depEvtGuidName, scope);
      assert(depEvtGuidSymbol);
      SgExprStatement* addDependenceCallExp = AstBuilder::buildOcrAddDependenceCallExp(depEvtGuidSymbol, triggerEvtGuidSymbol,
										       slot, AstBuilder::DbkMode::DB_MODE_NULL, scope);
      SageInterface::insertStatementBefore(pragmaStmt, addDependenceCallExp, true);
    }
    // Finally now add the spmdRankFinalizeCall
    SgExprStatement* spmdRankFinalizeCallExp = AstBuilder::buildSpmdRankFinalizeCallExp(triggerEvtGuidSymbol, scope);
    SageInterface::insertStatementBefore(pragmaStmt, spmdRankFinalizeCallExp, true);
  }
  SageInterface::removeStatement(pragmaStmt);
}

void OcrTranslator::setupShutdownEdt(string shutdownEdtName, OcrShutdownEdtContextPtr shutdownEdtContext) {
  SgSourceFile* sourcefile = shutdownEdtContext->getSourceFile();
  string edtFuncName = shutdownEdtContext->getShutdownEdtFuncName();
  SgPragmaDeclaration* shutdownPragma = shutdownEdtContext->getPragma();
  SgScopeStatement* scope = SageInterface::getScope(shutdownPragma);
  // Build OcrGuid variable declaration for EDT template
  // We may have multiple locations within same file
  // Use count to distinguish the different locations
  string edtTemplateGuidName = shutdownEdtName + "TemplGuid";
  SgVariableDeclaration* edtTemplateGuidDecl = AstBuilder::buildOcrGuidEdtTemplateVarDecl(edtTemplateGuidName, scope);
  // Setting up the edt template creation function call
  EdtAstInfoPtr edtAstInfoPtr = m_astInfoManager.getEdtAstInfo(edtFuncName);
  SgFunctionDeclaration* edtDecl = edtAstInfoPtr->getTaskFuncDecl();
  unsigned int nevts = shutdownEdtContext->getNumDepEvts();
  SgExprStatement* edtTemplateCallExp = AstBuilder::buildOcrEdtTemplateCallExp(edtTemplateGuidDecl, edtDecl, SageBuilder::buildIntVal(0), nevts, scope);
  SageInterface::insertStatementBefore(shutdownPragma, edtTemplateGuidDecl, true);
  SageInterface::insertStatementBefore(shutdownPragma, edtTemplateCallExp, true);
  // Now some bookkeeping
  // Add the variable symbol of the template guid
  SgVariableSymbol* edtTemplateGuidSymbol = GetVariableSymbol(edtTemplateGuidDecl, edtTemplateGuidName);
  edtAstInfoPtr->setTemplGuidName(edtTemplateGuidName);
  string edtGuidName = shutdownEdtName+"EdtGuid";
  SgType* ocrGuidType = AstBuilder::buildOcrGuidType(scope);
  SgVariableDeclaration* edtGuidDecl = SageBuilder::buildVariableDeclaration(edtGuidName, ocrGuidType, NULL, scope);
  SgVariableSymbol* edtGuidSymbol = GetVariableSymbol(edtGuidDecl, edtGuidName);
  // We need the edtTemplateGuid
  SgExprStatement* ocrEdtCreateCallExp = AstBuilder::buildOcrEdtCreateCallExp(edtGuidSymbol, edtTemplateGuidSymbol, NULL,
									      NULL, nevts, NULL, false, scope);
  SageInterface::insertStatementBefore(shutdownPragma, edtGuidDecl, true);
  SageInterface::insertStatementBefore(shutdownPragma, ocrEdtCreateCallExp, true);
  // Now some bookkeeping
  edtAstInfoPtr->setEdtGuidName(edtGuidName);

  // Finally add the dependent events
  list<OcrEvtContextPtr> evtList = shutdownEdtContext->getDepEvts();
  list<OcrEvtContextPtr>::iterator e = evtList.begin();
  // No datablocks for shutdown EDT
  // Starting slot is 0
  unsigned int slotIndex = 0;
  vector<SgStatement*> evtDepSetupStmts;
  for( ; e != evtList.end(); ++e, ++slotIndex) {
    OcrEvtContextPtr evt = *e;
    string evtname = evt->get_name();
    EvtAstInfoPtr evtAstInfo = m_astInfoManager.getEvtAstInfo(evtname, scope);
    string evtGuidName = evtAstInfo->getEvtGuidName();
    SgVariableSymbol* evtGuid = GetVariableSymbol(evtGuidName, scope);
    SgExprStatement* ocrAddDependenceCallExp = AstBuilder::buildOcrAddDependenceCallExp(evtGuid, edtGuidSymbol, slotIndex,
											AstBuilder::DbkMode::DB_MODE_NULL, scope);
    evtDepSetupStmts.push_back(static_cast<SgStatement*>(ocrAddDependenceCallExp));
  }
  SageInterface::insertStatementListBefore(shutdownPragma, evtDepSetupStmts);

  // Finally remove the pragma from the AST
  SageInterface::removeStatement(shutdownPragma);
}

void OcrTranslator::setupEdts() {
  Logger::Logger lg(" OcrTranslator::setupEdts", Logger::DEBUG);
  list<string> orderedEdts = m_ocrObjectManager.getEdtTraversalOrder();
  // Traverse them in the order
  list<string>::iterator edt = orderedEdts.begin();
  for( ; edt != orderedEdts.end(); ++edt) {
    string edtname = *edt;
    Logger::debug(lg) << "Setting " << edtname << endl;
    OcrTaskContextPtr taskContext = m_ocrObjectManager.getOcrTaskContext(edtname);
    // EDT Setup consists of following steps
    // 1. Setup the output event for the EDT
    // 2. Setup the template creation
    // 3. Setup the EDT creation
    // 4. Add dependences to EDT from datablocks
    // 5. Add dependences to EDT from events
    if(taskContext->getTaskType() == OcrTaskContext::e_TaskEdt) {
      OcrEdtContextPtr edtContext = boost::dynamic_pointer_cast<OcrEdtContext>(taskContext);
      setupEdt(edtname, edtContext);
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskLoopIter) {
      OcrLoopIterEdtContextPtr loopIterEdtContext = boost::dynamic_pointer_cast<OcrLoopIterEdtContext>(taskContext);
      setupLoopControlEdtBody(loopIterEdtContext);
      setupLoopControlEdt(loopIterEdtContext);
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskShutDown) {
      OcrShutdownEdtContextPtr shutdownEdtContext = boost::dynamic_pointer_cast<OcrShutdownEdtContext>(taskContext);
      setupShutdownEdt(edtname, shutdownEdtContext);
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskSpmdRegion) {
      OcrSpmdRegionContextPtr spmdRegionContext = boost::dynamic_pointer_cast<OcrSpmdRegionContext>(taskContext);
      setupSpmdRegionEdt(edtname, spmdRegionContext);
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskSpmdFinalize) {
      OcrSpmdFinalizeContextPtr spmdFinalizeContext = boost::dynamic_pointer_cast<OcrSpmdFinalizeContext>(taskContext);
      setupSpmdFinalize(edtname, spmdFinalizeContext);
    }
    else {
      cerr << "Unhandled Task Context in OcrTranslator::setupEdts\n";
      std::terminate();
    }
  }
}

SgStatement* OcrTranslator::replaceCommRank(SgStatement* callStmt) {
  // MPI_Comm_rank has the following AST - (SgExprStatement (SgFunctionCallExp (SgFunctionRefExp) (SgExprListExp)))
  // When MPI_Comm_rank is called through a function pointer it will have a different AST
  SgFunctionCallExp* callExp = isSgFunctionCallExp(isSgExprStatement(callStmt)->get_expression());
  if(!callExp) {
    ostringstream oss;
    oss << "Cannot replace " << callStmt->unparseToString() << " with spmdMyRank()\n";
    throw new TranslateException(oss.str());
  }
  SgExprListExp* args = callExp->get_args();
  SgExpressionPtrList& argsList = args->get_expressions();
  // We need to find the second argument of MPI_Comm_rank
  SgExpression* second = NULL;
  if(SgCastExp* castExp = isSgCastExp(argsList[1])) {
    second = castExp->get_operand();
  }
  else {
    second = argsList[1];
  }
  SgVarRefExp* rankVarRefExp = NULL;
  if(SgAddressOfOp* addrOfOp = isSgAddressOfOp(second)) {
    rankVarRefExp = isSgVarRefExp(addrOfOp->get_operand());
  }
  if(!rankVarRefExp) {
    throw new TranslateException("Cannot find rank SgVarRefExp in MPI_Comm_rank\n");
  }
  SgScopeStatement* scope = SageInterface::getEnclosingScope(callStmt);
  SgStatement* spmdMyRankCallStmt = AstBuilder::buildSpmdMyRankCallExp(rankVarRefExp, scope);
  return spmdMyRankCallStmt;
}

SgStatement* OcrTranslator::replaceCommSize(SgStatement* callStmt) {
  // MPI_Comm_size has the following AST - (SgExprStatement (SgFunctionCallExp (SgFunctionRefExp) (SgExprListExp)))
  // When MPI_Comm_size is called through a function pointer it will have a different AST
  SgFunctionCallExp* callExp = isSgFunctionCallExp(isSgExprStatement(callStmt)->get_expression());
  if(!callExp) {
    ostringstream oss;
    oss << "Cannot replace " << callStmt->unparseToString() << " with spmdMySize()\n";
    throw new TranslateException(oss.str());
  }
  SgExprListExp* args = callExp->get_args();
  SgExpressionPtrList& argsList = args->get_expressions();
  // We need to find the second argument of MPI_Comm_size
  SgExpression* second = NULL;
  if(SgCastExp* castExp = isSgCastExp(argsList[1])) {
    second = castExp->get_operand();
  }
  else {
    second = argsList[1];
  }
  SgVarRefExp* sizeVarRefExp = NULL;
  if(SgAddressOfOp* addrOfOp = isSgAddressOfOp(second)) {
    sizeVarRefExp = isSgVarRefExp(addrOfOp->get_operand());
  }
  if(!sizeVarRefExp) {
    throw new TranslateException("Cannot find size SgVarRefExp in MPI_Comm_size\n");
  }
  SgScopeStatement* scope = SageInterface::getEnclosingScope(callStmt);
  SgStatement* spmdSizeCallStmt = AstBuilder::buildSpmdSizeCallExp(sizeVarRefExp, scope);
  return spmdSizeCallStmt;
}

void OcrTranslator::translateMpi() {
  Logger::Logger lg("OcrTranslator::translateMpi", Logger::DEBUG);
  list<MpiOpContextPtr> mpiOpContextList = m_ocrObjectManager.getMpiOpContextList();
  list<MpiOpContextPtr>::iterator o = mpiOpContextList.begin();
  for( ; o != mpiOpContextList.end(); ++o) {
    MpiOpContextPtr mpiOpContext = *o;
    SgStatement* callStmt = mpiOpContext->getMpiCallStmt();
    switch(mpiOpContext->getMpiOpType()) {
    case MpiOpContext::OP_INIT:
    case MpiOpContext::OP_FINALIZE: {
      SageInterface::removeStatement(callStmt);
      break;
    }
    case MpiOpContext::OP_COMM_RANK: {
      SgStatement* stmt = replaceCommRank(callStmt);
      SageInterface::insertStatementBefore(callStmt, stmt, true);
      SageInterface::removeStatement(callStmt);
      break;
    }
    case MpiOpContext::OP_COMM_SIZE: {
      SgStatement* stmt = replaceCommSize(callStmt);
      SageInterface::insertStatementBefore(callStmt, stmt, true);
      SageInterface::removeStatement(callStmt);
      break;
    }
    default:
      cerr << "Unhandled MPI Operation in OcrTranslator::translateMpi()\n";
    }
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
    SgVariableDeclaration* mainEdtDbkDecl = AstBuilder::buildDepDbkPtrDecl(mainEdtDbkName, u64PtrType, 0, mainEdtArgv, basicblock);
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

// Main driver for translation
void OcrTranslator::translate() {
  Logger::Logger lg("OcrTranslator::translate");
  try {
    /* The steps are as follows
    * Do not change the order of the functions
    * 1. The ocr header files are first inserted in all sourcefiles
    * 2. The annoated task regions are outlined in to a function
    * 3. The datablock annotations are then translated to datablock declarations.
    *    This must be done after the outlining as the DbkAstInfo for each
    *    datablock is associaed with scope and outlining changes the scope of
    *    where the datablock declarations reside.
    * 4. For each EDT insert the dependent datablock declarations
    * 5. Setup the EDT creation which includes - ocrEdtTemplateCreate, ocrEdtCreate
    *    and ocrAddDependence
    * 6. Replace MPI calls with SPMD lib calls
    * 7. Generate and outline the main to mainEdt
    */
    insertOcrHeaderFiles();
    outlineEdts();
    translateDbks();
    insertEdtsDepDbksDecl();
    setupEdts();
    translateMpi();
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

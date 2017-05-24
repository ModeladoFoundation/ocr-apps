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
  // Bookkeeping
  // Register the variable names we assigned to dbkGuid and dbkPtr
  m_astInfoManager.regDbkAstInfo(dbkName, ocrGuidName.getString(), varName.getString(), scope);
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
    string dbkname = (*dbk)->get_name();
    // Build the pointer
    string dbkPtrName = (*dbk)->getSgInitializedName()->get_name();
    SgType* dbkPtrType = (*dbk)->getDbkPtrType();
    SgVariableDeclaration* vdecl = AstBuilder::buildDepDbkPtrDecl(dbkPtrName, dbkPtrType, slot, depv, basicblock);
    depDbksDeclStmts.push_back(vdecl);
    // Build the Guid
    string guidName(dbkPtrName+"Guid");
    SgVariableDeclaration* guidDecl = AstBuilder::buildDepDbkGuidDecl(guidName, slot, depv, basicblock);
    depDbksDeclStmts.push_back(guidDecl);
    // Bookkeeping
    m_astInfoManager.regDbkAstInfo(dbkname, guidName, dbkPtrName, basicblock);
  }
  SageInterface::prependStatementList(depDbksDeclStmts, basicblock);
}

void OcrTranslator::insertDepElemDecl(string edtName, OcrEdtContextPtr edtContext, TaskAstInfoPtr taskAstInfo) {
  SgFunctionDeclaration* edtDecl = taskAstInfo->getTaskFuncDecl();
  assert(edtDecl);
  SgScopeStatement* edtScope = SageInterface::getEnclosingScope(edtDecl);

  // Build a depElem struct with annotated list of variables
  list<SgVarRefExp*> depElems = edtContext->getDepElems();
  SgClassDeclaration* depElemStructDecl = AstBuilder::buildOcrEdtDepElemStruct(edtName, depElems, edtScope);

  // Build a typedef for it
  SgTypedefDeclaration* depElemTypedefType = AstBuilder::buildTypeDefDecl(edtName, depElemStructDecl->get_type(), edtScope);

  // Insert the depElem struct above the EDT
  SageInterface::insertStatementBefore(edtDecl, depElemTypedefType, true);
  SageInterface::insertStatementBefore(depElemTypedefType, depElemStructDecl, true);

  // Bookkeeping
  taskAstInfo->setDepElemStructDecl(depElemStructDecl);
  taskAstInfo->setDepElemTypedefType(depElemTypedefType->get_type());
  taskAstInfo->setDepElemBaseType(depElemStructDecl->get_type());

  SgBasicBlock* basicblock = edtDecl->get_definition()->get_body();

  // Insert the struct inside the edtDecl
  SgName depElemStructName("depElem");
  SgVariableDeclaration* depElemStructVar = AstBuilder::buildOcrEdtDepElemStructDecl(depElemTypedefType->get_type(), depElemStructName, basicblock);
  SageInterface::prependStatement(depElemStructVar, basicblock);

  SgVariableSymbol* depElemStructSymbol = GetVariableSymbol(depElemStructVar, depElemStructName);
  // Build the stack of local variables for the EDT from the struct
  vector<SgStatement*> depElemVarDeclStmts = AstBuilder::buildEdtDepElemVarsDecl(depElemStructDecl, depElemStructSymbol, basicblock);
  SageInterface::insertStatementListAfter(depElemStructVar, depElemVarDeclStmts);
  //
  if(boost::static_pointer_cast<OcrTaskContext>(edtContext)->getTaskType() == OcrTaskContext::e_TaskLoopIter) {
    // anchor
    SgStatement* lastStmt = depElemVarDeclStmts.back();
    OcrLoopIterEdtContextPtr loopIterEdtContext = boost::dynamic_pointer_cast<OcrLoopIterEdtContext>(edtContext);
    string loopCompEvtName = loopIterEdtContext->getOutputEvt()->get_name();
    string loopCompEvtGuidName = loopCompEvtName + "Guid";
    SgClassDefinition* depElemStructDefn = depElemStructDecl->get_definition();
    // Register the event inside the struct scope
    m_astInfoManager.regEvtAstInfo(loopCompEvtName, loopCompEvtGuidName, depElemStructDefn);

    SgVariableDeclaration* loopCompEvtGuidDecl = AstBuilder::buildGuidVarDecl(loopCompEvtGuidName, depElemStructDefn);
    SgVariableSymbol* loopCompEvtGuidSymbol = GetVariableSymbol(loopCompEvtGuidDecl, loopCompEvtGuidName);
    depElemStructDefn->append_member(loopCompEvtGuidDecl);
    // Now create a declration inside the EDT
    SgVariableDeclaration* edtLoopCompEvtGuidDecl = AstBuilder::buildEdtDepElemVarDecl(depElemStructSymbol, loopCompEvtGuidSymbol, basicblock);
    SageInterface::insertStatementAfter(lastStmt, edtLoopCompEvtGuidDecl, true);
    // Bookkeeping
    m_astInfoManager.regEvtAstInfo(loopCompEvtName, loopCompEvtGuidName, basicblock);
  }
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
  // Insert the ptr and guid for depDbks inside the EDT
  list<OcrDbkContextPtr> depDbks = loopIterEdtContext->getDepDbks();
  insertDepDbkDecl(loopBodyEdtName, depDbks, 0, loopBodyEdtAstInfo);
  // Insert declaration for depElem stack variables inside the EDT
  list<SgVarRefExp*> depElems = loopIterEdtContext->getDepElems();
  insertDepElemDecl(loopBodyEdtName, loopIterEdtContext, loopBodyEdtAstInfo);
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

  // Insert the ptr and guid for depDbks inside the EDT
  list<OcrDbkContextPtr> depDbks = loopIterEdtContext->getDepDbks();
  insertDepDbkDecl(loopControlEdtName, depDbks, 0, loopControlEdtAstInfo);
   // Insert declaration for depElem stack variables inside the EDT
  insertDepElemDecl(loopControlEdtName,  loopIterEdtContext, loopControlEdtAstInfo);


  // book keeping
  loopControlEdtAstInfo->setLoopControlIfBasicBlock(ifBasicBlock);
  loopControlEdtAstInfo->setLoopControlElseBasicBlock(elseBasicBlock);
  loopControlEdtAstInfo->setLoopControlIfStmt(loopControlIfStmt);

  // Outlining control EDT has the following steps
  // 1. Create an iteration complete event for the body EDT (ignoring this step for now)
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

  string loopBodyOutEvtName = loopBodyEdtName + "Out";
  m_ocrObjectManager.registerOcrEvt(loopBodyOutEvtName);
  setupStmts = setupEdtOutEvt(loopBodyEdtName, loopBodyOutEvtName, ifBasicBlock);
  stmtsToInsert.insert(stmtsToInsert.end(), setupStmts.begin(), setupStmts.end());
  setupStmts.clear();

  list<SgVarRefExp*> depElems = loopIterEdtContext->getDepElems();
  setupStmts = setupEdtDepElems(loopBodyEdtName, loopIterEdtContext, depElems, ifBasicBlock);
  stmtsToInsert.insert(stmtsToInsert.end(), setupStmts.begin(), setupStmts.end());
  setupStmts.clear();

  setupStmts = setupEdtTemplate(loopBodyEdtName, depDbks.size(), ifBasicBlock);
  stmtsToInsert.insert(stmtsToInsert.end(), setupStmts.begin(), setupStmts.end());
  setupStmts.clear();

  // Setup the body EDT as finish EDT
  setupStmts = setupEdtCreate(loopBodyEdtName, loopBodyOutEvtName, true, ifBasicBlock);
  stmtsToInsert.insert(stmtsToInsert.end(), setupStmts.begin(), setupStmts.end());
  setupStmts.clear();

  // Setup dependent datablocks for the body EDT
  setupStmts = setupEdtDepDbks(loopBodyEdtName, depDbks, 0, ifBasicBlock);
  stmtsToInsert.insert(stmtsToInsert.end(), setupStmts.begin(), setupStmts.end());
  setupStmts.clear();

  // Add the statements to the AST
  SageInterface::prependStatementList(stmtsToInsert, ifBasicBlock);
  stmtsToInsert.clear();

  // Now we can to setup the next iteration control EDT
  setupStmts = setupEdtDepElems(loopControlEdtName, loopIterEdtContext, depElems, ifBasicBlock);
  SageInterface::appendStatementList(setupStmts, ifBasicBlock);
  setupStmts.clear();

  // Number of dependences for the next iteration is depDbks.size() + 1 for the output of bodyEDT outEvt
  setupStmts = setupEdtTemplate(loopControlEdtName, depDbks.size() + 1, ifBasicBlock);
  SageInterface::appendStatementList(setupStmts, ifBasicBlock);
  setupStmts.clear();

  setupStmts = setupEdtCreate(loopControlEdtName, "", false, ifBasicBlock);
  SageInterface::appendStatementList(setupStmts, ifBasicBlock);
  setupStmts.clear();

  // Setup dependent datablocks for the body EDT
  setupStmts = setupEdtDepDbks(loopControlEdtName, depDbks, 0, ifBasicBlock);
  SageInterface::appendStatementList(setupStmts, ifBasicBlock);
  setupStmts.clear();


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

// First step in creating the EDTs from pragmas
// Register the AST Info for the EDT based on the type
void OcrTranslator::outlineEdts() {
  Logger::Logger lg("OcrTranslator::outlineEdts", Logger::DEBUG);
  list<string> orderedEdts = m_ocrObjectManager.getEdtTraversalOrder();
  list<string>::reverse_iterator edt = orderedEdts.rbegin();
  for( ; edt != orderedEdts.rend(); ++edt) {
    string edtname = *edt;
    Logger::debug(lg) << "Outling " << edtname << "\n";
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
      // Insert the ptr, guid for depDbks inside the EDT
      list<OcrDbkContextPtr> depDbks = edtContext->getDepDbks();
      insertDepDbkDecl(edtname, depDbks, 0, edtAstInfo);
      // Insert declaration for depElem stack variables inside the EDT
      insertDepElemDecl(edtname, edtContext, edtAstInfo);
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskLoopIter) {
      OcrLoopIterEdtContextPtr loopIterEdtContext = boost::dynamic_pointer_cast<OcrLoopIterEdtContext>(taskContext);
      assert(loopIterEdtContext);
      string loopEdtName = loopIterEdtContext->getTaskName();
      SgStatement* loopStmt = loopIterEdtContext->getLoopStmt();
      if(SgForStatement* forStmt = isSgForStatement(loopStmt)) {
	outlineForLoopBodyEdt(loopIterEdtContext, forStmt);
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

    else {
      cerr << "Unhandled Task Context in OcrTranslator::outlineEdts()\n";
      std::terminate();
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
  string depElemSizeName = "_paramc"+edtname;
  SgType* depElemType = edtAstInfo->getDepElemTypedefType();
  SgVariableDeclaration* depElemSizeVarDecl = AstBuilder::buildDepElemSizeVarDecl(depElemSizeName, depElemType, scope);
  edtTemplSetupStmts.push_back(depElemSizeVarDecl);

  SgVariableSymbol* depElemSizeVarSymbol = GetVariableSymbol(depElemSizeVarDecl, depElemSizeName);
  SgVarRefExp* depElemSizeVarRef = SageBuilder::buildVarRefExp(depElemSizeVarSymbol);
  SgExprStatement* edtTemplateCallExp = AstBuilder::buildOcrEdtTemplateCallExp(edtTemplateGuidDecl, edtDecl, depElemSizeVarRef, ndeps, scope);
  edtTemplSetupStmts.push_back(edtTemplateCallExp);

  // Now some bookkeeping
  // Add the variable symbol of the template guid
  edtAstInfo->setTemplGuidName(edtTemplateGuidName);

  return edtTemplSetupStmts;
}

vector<SgStatement*> OcrTranslator::setupEdtDepElems(string edtname, OcrEdtContextPtr edtContext, list<SgVarRefExp*>& depElemVarList, SgScopeStatement* scope) {
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
  // We also need the depElemStructSymbol
  string depElemStructName = edtAstInfoPtr->getDepElemStructName();
  SgVariableSymbol* depElemStructSymbol = GetVariableSymbol(depElemStructName, scope);

  SgVariableSymbol* outEvtGuidSymbol = NULL;
  if(outEvtName.length() != 0) {
    EvtAstInfoPtr outEvtAstInfo = m_astInfoManager.getEvtAstInfo(outEvtName, scope);
    string outEvtGuidName  = outEvtAstInfo->getEvtGuidName();
    outEvtGuidSymbol = GetVariableSymbol(outEvtGuidName, scope);
  }
  SgExprStatement* edtCreateCallExp = AstBuilder::buildOcrEdtCreateCallExp(edtGuidSymbol, edtTemplateGuidSymbol,
									   depElemStructSymbol, outEvtGuidSymbol, isFinishEdt, scope);
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
  SgExprStatement* ocrEdtCreateCallExp = AstBuilder::buildOcrEdtCreateCallExp(edtGuidSymbol, edtTemplateGuidSymbol, NULL, NULL, false, scope);
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
  list<string> orderedEdts = m_ocrObjectManager.getEdtTraversalOrder();
  // Traverse them in the order
  list<string>::iterator edt = orderedEdts.begin();
  for( ; edt != orderedEdts.end(); ++edt) {
    string edtname = *edt;
    OcrTaskContextPtr taskContext = m_ocrObjectManager.getOcrTaskContext(edtname);
    // EDT Setup consists of following steps
    // 1. Setup the output event for the EDT
    // 2. Setup the template creation
    // 3. Setup the EDT creation
    // 4. Add dependences to EDT from datablocks
    // 5. Add dependences to EDT from events
    if(taskContext->getTaskType() == OcrTaskContext::e_TaskEdt) {
      OcrEdtContextPtr edtContext = boost::dynamic_pointer_cast<OcrEdtContext>(taskContext);
      assert(edtContext);
      SgPragmaDeclaration* taskPragma = edtContext->getTaskPragma();
      SgScopeStatement* scope = SageInterface::getEnclosingScope(taskPragma);

      // 1. Setup the output event for the EDT
      string outEvtName = edtContext->getOutputEvt()->get_name();
      vector<SgStatement*> outEvtSetupStmts = setupEdtOutEvt(edtname, outEvtName, scope);
      SageInterface::insertStatementListBefore(taskPragma, outEvtSetupStmts);
      outEvtSetupStmts.clear();

      // 2. Setup the template creation for the EDT
      unsigned int ndeps = edtContext->getNumDepDbks() + edtContext->getNumDepEvts();
      vector<SgStatement*> edtTemplSetupStmts = setupEdtTemplate(edtname, ndeps, scope);
      SageInterface::insertStatementListBefore(taskPragma, edtTemplSetupStmts);
      edtTemplSetupStmts.clear();

      // 3. Setup the depElems
      list<SgVarRefExp*> depElems = edtContext->getDepElems();
      vector<SgStatement*> depElemSetupStmts = setupEdtDepElems(edtname, edtContext, depElems, scope);
      SageInterface::insertStatementListBefore(taskPragma, depElemSetupStmts);
      depElemSetupStmts.clear();

      OcrEvtContextPtr outEvtContext = edtContext->getOutputEvt();
      bool isFinishEdt = edtContext->isFinishEdt();
      vector<SgStatement*> edtCreateSetupStmts = setupEdtCreate(edtname, outEvtContext->get_name(), isFinishEdt, scope);
      SageInterface::insertStatementListBefore(taskPragma, edtCreateSetupStmts);
      edtCreateSetupStmts.clear();

      list<OcrDbkContextPtr> depDbks = edtContext->getDepDbks();
      unsigned int slotBegin = 0;
      vector<SgStatement*> depDbkSetupStmts = setupEdtDepDbks(edtname, depDbks, slotBegin, scope);
      SageInterface::insertStatementListBefore(taskPragma, depDbkSetupStmts);
      depDbkSetupStmts.clear();

      list<OcrEvtContextPtr> depEvts = edtContext->getDepEvts();
      slotBegin = depDbks.size();
      vector<SgStatement*> depEvtSetupStmts = setupEdtDepEvts(edtname, depEvts, slotBegin, scope);
      SageInterface::insertStatementListBefore(taskPragma, depEvtSetupStmts);
      depEvtSetupStmts.clear();

      // Finally remove the pragma for each edt
      SgBasicBlock* taskBasicBlock = edtContext->getTaskBasicBlock();
      removeOcrTaskPragma(edtname, taskBasicBlock, taskPragma);
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskLoopIter) {
      OcrLoopIterEdtContextPtr loopIterEdtContext = boost::dynamic_pointer_cast<OcrLoopIterEdtContext>(taskContext);
      assert(loopIterEdtContext);

      SgPragmaDeclaration* taskPragma = loopIterEdtContext->getPragma();
      SgScopeStatement* scope = SageInterface::getEnclosingScope(taskPragma);
      string loopControlEdtName = loopIterEdtContext->getLoopControlEdtName();

      // 1. Setup the output event for the EDT
      string outEvtName = loopIterEdtContext->getOutputEvt()->get_name();
      vector<SgStatement*> outEvtSetupStmts = setupEdtOutEvt(loopControlEdtName, outEvtName, scope);
      SageInterface::insertStatementListBefore(taskPragma, outEvtSetupStmts);
      outEvtSetupStmts.clear();

      vector<SgStatement*> loopInitStmts = loopIterEdtContext->getLoopInitStmts();
      if(loopInitStmts.size() > 0) {
	SageInterface::insertStatementListBefore(taskPragma, loopInitStmts);
      }
      loopInitStmts.clear();

      list<SgVarRefExp*> depElems = loopIterEdtContext->getDepElems();
      vector<SgStatement*> depElemSetupStmts = setupEdtDepElems(loopControlEdtName, loopIterEdtContext, depElems, scope);
      SageInterface::insertStatementListBefore(taskPragma, depElemSetupStmts);
      depElemSetupStmts.clear();

      // 2. Setup the template creation for the EDT
      unsigned int ndeps = loopIterEdtContext->getNumDepDbks() + loopIterEdtContext->getNumDepEvts();
      vector<SgStatement*> edtTemplSetupStmts = setupEdtTemplate(loopControlEdtName, ndeps, scope);
      SageInterface::insertStatementListBefore(taskPragma, edtTemplSetupStmts);
      edtTemplSetupStmts.clear();

      bool isFinishEdt = loopIterEdtContext->isFinishEdt();
      vector<SgStatement*> edtCreateSetupStmts = setupEdtCreate(loopControlEdtName, "", isFinishEdt, scope);
      SageInterface::insertStatementListBefore(taskPragma, edtCreateSetupStmts);
      edtCreateSetupStmts.clear();

      list<OcrDbkContextPtr> depDbks = loopIterEdtContext->getDepDbks();
      unsigned int slotBegin = 0;
      vector<SgStatement*> depDbkSetupStmts = setupEdtDepDbks(loopControlEdtName, depDbks, slotBegin, scope);
      SageInterface::insertStatementListBefore(taskPragma, depDbkSetupStmts);
      depDbkSetupStmts.clear();

      list<OcrEvtContextPtr> depEvts = loopIterEdtContext->getDepEvts();
      slotBegin = depDbks.size();
      vector<SgStatement*> depEvtSetupStmts = setupEdtDepEvts(loopControlEdtName, depEvts, slotBegin, scope);
      SageInterface::insertStatementListBefore(taskPragma, depEvtSetupStmts);
      depEvtSetupStmts.clear();
      // assert(false);

      // Remove the pragma and the basic block
      SgStatement* loopStmt = loopIterEdtContext->getLoopStmt();
      SageInterface::removeStatement(taskPragma);
      SageInterface::removeStatement(loopStmt);
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskShutDown) {
      OcrShutdownEdtContextPtr shutdownEdtContext = boost::dynamic_pointer_cast<OcrShutdownEdtContext>(taskContext);
      setupShutdownEdt(edtname, shutdownEdtContext);
    }
    else {
      cerr << "Unhandled Task Context in OcrTranslator::setupEdts\n";
      std::terminate();
    }
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
    // insert the header files
    insertOcrHeaderFiles();
    outlineEdts();
    translateDbks();
    setupEdts();
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

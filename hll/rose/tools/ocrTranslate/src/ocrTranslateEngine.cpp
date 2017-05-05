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

void OcrTranslator::insertDepElemDecl(string edtName, list<SgVarRefExp*>& depElems,
				      TaskAstInfoPtr taskAstInfo) {
  // struct declaration for the dependent elements
  SgFunctionDeclaration* edtDecl = taskAstInfo->getTaskFuncDecl();
  SgBasicBlock* basicblock = edtDecl->get_definition()->get_body();

  SgScopeStatement* edtDeclScope = SageInterface::getEnclosingScope(edtDecl);
  SgClassDeclaration* depElemStructDecl = AstBuilder::buildOcrEdtDepElemStruct(edtName, depElems, edtDecl);

  // corresponding typedef for the dependent elements
  SgTypedefDeclaration* depElemTypedefType = AstBuilder::buildTypeDefDecl(edtName, depElemStructDecl->get_type(), edtDeclScope);

  // Build a declaration for the depElem struct inside the EDT
  SgName depElemStructName("depElem");
  SgVariableDeclaration* depElemStructVar = AstBuilder::buildOcrEdtDepElemStructDecl(depElemTypedefType->get_type(), depElemStructName, basicblock);
  SageInterface::prependStatement(depElemStructVar, basicblock);
  SageInterface::insertStatementBefore(edtDecl, depElemStructDecl, true);
  SageInterface::insertStatementAfter(depElemStructDecl, depElemTypedefType, true);
  SgVariableSymbol* depElemStructSymbol = GetVariableSymbol(depElemStructVar, depElemStructName);
  // Build the stack of local variables for the EDT from the struct
  vector<SgStatement*> depElemVarDeclStmts = AstBuilder::buildEdtDepElemVarsDecl(depElemStructDecl, depElemStructSymbol, basicblock);
  SageInterface::insertStatementListAfter(depElemStructVar, depElemVarDeclStmts);
  // Bookkeeping
  taskAstInfo->setDepElemTypedefType(depElemTypedefType->get_type());
  taskAstInfo->setDepElemBaseType(depElemStructDecl->get_type());
  taskAstInfo->setDepElemStructDecl(depElemStructDecl);
}

void OcrTranslator::outlineEdt(string edtName, list<OcrDbkContextPtr> depDbks, list<SgVarRefExp*>& depElems,
			       SgBasicBlock* taskBasicBlock, SgSourceFile* sourcefile,
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

  // Insert the dep dbks and depElems for the task
  insertDepDbkDecl(edtName, depDbks, 0, taskAstInfo);
  insertDepElemDecl(edtName, depElems, taskAstInfo);
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

void OcrTranslator::outlineForLoopEdt(OcrLoopIterEdtContextPtr loopIterEdtContext, SgForStatement* forStmt) {
  SgBasicBlock* loopBodyBasicBlock = SageInterface::ensureBasicBlockAsBodyOfFor(forStmt);
  string loopBodyEdtName = loopIterEdtContext->getLoopBodyEdtName();
  // Bookkeeping: Register the EDT and get its outlining AST info
  EdtAstInfoPtr loopBodyEdtAstInfo = m_astInfoManager.regEdtAstInfo(loopBodyEdtName);

  SgSourceFile* sourcefile = loopIterEdtContext->getSourceFile();
  list<OcrDbkContextPtr> depDbks = loopIterEdtContext->getDepDbks();
  list<SgVarRefExp*> depElems = loopIterEdtContext->getDepElems();
  outlineEdt(loopBodyEdtName, depDbks, depElems, loopBodyBasicBlock, sourcefile, loopBodyEdtAstInfo);

  // Now generate the EDT for loop control
  string loopControlEdtName = loopIterEdtContext->getLoopControlEdtName();
  // We will create a basic block with just the loop condition transformed as if stmt
  SgBasicBlock* loopControlBasicBlock = AstBuilder::buildLoopControlEdtBasicBlock(forStmt);
  LoopControlEdtAstInfoPtr loopControlEdtAstInfo = boost::make_shared<LoopControlEdtAstInfo>(loopControlEdtName);
  outlineEdt(loopControlEdtName, depDbks, depElems, loopControlBasicBlock, sourcefile, loopControlEdtAstInfo);
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
      list<OcrDbkContextPtr> depDbks = edtContext->getDepDbks();
      list<SgVarRefExp*> depElems = edtContext->getDepElems();
      // Outlining Info will be updated by the outlineEdt function
      outlineEdt(edtname, depDbks, depElems, basicblock, sourcefile, edtAstInfo);
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskLoopIter) {
      OcrLoopIterEdtContextPtr loopIterEdtContext = boost::dynamic_pointer_cast<OcrLoopIterEdtContext>(taskContext);
      assert(loopIterEdtContext);
      string loopEdtName = loopIterEdtContext->getTaskName();
      SgStatement* loopStmt = loopIterEdtContext->getLoopStmt();
      if(SgForStatement* forStmt = isSgForStatement(loopStmt)) {
	outlineForLoopEdt(loopIterEdtContext, forStmt);
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

void OcrTranslator::setupEdtEvtCreate(std::string edtname, OcrEdtContextPtr edtContext) {
  SgPragmaDeclaration* taskPragma = edtContext->getTaskPragma();
  SgScopeStatement* scope = SageInterface::getEnclosingScope(taskPragma);
  vector<SgStatement*> evtCreateStmts;
  // Create guid and events that this EDT has to satisfy
  OcrEvtContextPtr outEvt = edtContext->getOutputEvt();
  SgType* ocrGuidType = AstBuilder::buildOcrGuidType(scope);

  // For each event create ocrGuid and the event using ocrEvtCreate
  string evtGuidName = outEvt->get_name() + "EvtGuid";
  SgVariableDeclaration* evtGuidDecl = SageBuilder::buildVariableDeclaration(evtGuidName, ocrGuidType, NULL, scope);
  evtCreateStmts.push_back(evtGuidDecl);
  SgVariableSymbol* evtGuidSymbol = GetVariableSymbol(evtGuidName, scope);
  SgExprStatement* evtCreateCallExp = AstBuilder::buildEvtCreateCallExp(evtGuidSymbol, scope);
  evtCreateStmts.push_back(evtCreateCallExp);
  // Now some bookkeeping
  m_astInfoManager.regEvtAstInfo(outEvt->get_name(), evtGuidName, scope);
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
  EdtAstInfoPtr edtAstInfo = m_astInfoManager.getEdtAstInfo(edtname);
  SgFunctionDeclaration* edtDecl = edtAstInfo->getTaskFuncDecl();
  // TODO: Fix this - ndelemes is not 1
  unsigned int ndelems = 1; // all parameters are wrapped as a struct
  unsigned int ndbks = edtContext->getNumDepDbks() + edtContext->getNumDepEvts();
  SgExprStatement* edtTemplateCallExp = AstBuilder::buildOcrEdtTemplateCallExp(edtTemplateGuidDecl, edtDecl, ndelems, ndbks, scope);
  SageInterface::insertStatementBefore(taskPragma, edtTemplateGuidDecl, true);
  SageInterface::insertStatementBefore(taskPragma, edtTemplateCallExp, true);
  // Now some bookkeeping
  // Add the variable symbol of the template guid
  edtAstInfo->setTemplGuidName(edtTemplateGuidName);
}

void OcrTranslator::setupEdtDepElems(string edtname, OcrEdtContextPtr edtContext) {
  Logger::Logger lg("OcrTranslator::setupEdtDepElems");
  SgPragmaDeclaration* taskPragma = edtContext->getTaskPragma();
  SgScopeStatement* scope = SageInterface::getScope(taskPragma);
  EdtAstInfoPtr edtAstInfoPtr = m_astInfoManager.getEdtAstInfo(edtname);
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
  edtAstInfoPtr->setDepElemStructName(depElemVarName);
}

// //! Method to setup the ocrEdtCreate function call expression
void OcrTranslator::setupEdtCreate(string edtname, OcrEdtContextPtr edtContext) {
  SgPragmaDeclaration* taskPragma = edtContext->getTaskPragma();
  SgScopeStatement* scope = SageInterface::getScope(taskPragma);
  EdtAstInfoPtr edtAstInfoPtr = m_astInfoManager.getEdtAstInfo(edtname);
  string edtGuidName = edtname+"EdtGuid";
  SgType* ocrGuidType = AstBuilder::buildOcrGuidType(scope);
  SgVariableDeclaration* edtGuidDecl = SageBuilder::buildVariableDeclaration(edtGuidName, ocrGuidType, NULL, scope);
  SgVariableSymbol* edtGuidSymbol = GetVariableSymbol(edtGuidDecl, edtGuidName);
  // We need the edtTemplateGuid
  string edtTemplGuidName = edtAstInfoPtr->getEdtTemplateGuidName();
  SgVariableSymbol* edtTemplateGuidSymbol = GetVariableSymbol(edtTemplGuidName, scope);
  // We also need the depElemStructSymbol
  string depElemStructName = edtAstInfoPtr->getDepElemStructName();
  SgVariableSymbol* depElemStructSymbol = GetVariableSymbol(depElemStructName, scope);
  OcrEvtContextPtr outEvtContext = edtContext->getOutputEvt();
  EvtAstInfoPtr outEvtAstInfo = m_astInfoManager.getEvtAstInfo(outEvtContext->get_name(), scope);
  string outEvtGuidName  = outEvtAstInfo->getEvtGuidName();
  SgVariableSymbol* outEvtGuidSymbol = GetVariableSymbol(outEvtGuidName, scope);
  bool finishEdt = edtContext->isFinishEdt();
  SgExprStatement* ocrEdtCreateCallExp = AstBuilder::buildOcrEdtCreateCallExp(edtGuidSymbol, edtTemplateGuidSymbol,
									      depElemStructSymbol, outEvtGuidSymbol, finishEdt, scope);
  SageInterface::insertStatementBefore(taskPragma, edtGuidDecl, true);
  SageInterface::insertStatementBefore(taskPragma, ocrEdtCreateCallExp, true);
  // Now some bookkeeping
  edtAstInfoPtr->setEdtGuidName(edtGuidName);
}

void OcrTranslator::setupEdtDepDbks(string edtname, OcrEdtContextPtr edtContext) {
  Logger::Logger lg("OcrTranslator::setupEdtDepDbks");
  SgPragmaDeclaration* taskPragma = edtContext->getTaskPragma();
  SgScopeStatement* scope = SageInterface::getScope(taskPragma);
  EdtAstInfoPtr edtAstInfoPtr = m_astInfoManager.getEdtAstInfo(edtname);
  string edtGuidName = edtAstInfoPtr->getEdtGuidName();
  SgVariableSymbol* edtGuidSymbol = GetVariableSymbol(edtGuidName, scope);
  // for each datablock add ocrAddDependence
  list<OcrDbkContextPtr> dbkList = edtContext->getDepDbks();
  list<OcrDbkContextPtr>::iterator l = dbkList.begin();
  vector<SgStatement*> dbkSetupStmts;
  for(int slot = 0 ; l != dbkList.end(); ++l, ++slot) {
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
  SageInterface::insertStatementListBefore(taskPragma, dbkSetupStmts);
}

void OcrTranslator::setupEdtDepEvts(string edtname, OcrEdtContextPtr edtContext) {
  SgPragmaDeclaration* taskPragma = edtContext->getTaskPragma();
  SgScopeStatement* scope = SageInterface::getEnclosingScope(taskPragma);
  EdtAstInfoPtr edtAstInfoPtr = m_astInfoManager.getEdtAstInfo(edtname);
  string edtGuidName = edtAstInfoPtr->getEdtGuidName();
  SgVariableSymbol* edtGuidSymbol = GetVariableSymbol(edtGuidName, scope);
  // For each event add ocrAddDependence
  list<OcrEvtContextPtr> evtList = edtContext->getDepEvts();
  list<OcrEvtContextPtr>::iterator e = evtList.begin();
  // The starting slot dependent events is after the datablocks
  unsigned int slotIndex = edtContext->getNumDepDbks();
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
  SgExprStatement* edtTemplateCallExp = AstBuilder::buildOcrEdtTemplateCallExp(edtTemplateGuidDecl, edtDecl, 0, nevts, scope);
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
    if(taskContext->getTaskType() == OcrTaskContext::e_TaskEdt) {
      OcrEdtContextPtr edtContext = boost::dynamic_pointer_cast<OcrEdtContext>(taskContext);
      assert(edtContext);
      setupEdtEvtCreate(edtname, edtContext);
      setupEdtTemplate(edtname, edtContext);
      setupEdtDepElems(edtname, edtContext);
      setupEdtCreate(edtname, edtContext);
      setupEdtDepDbks(edtname, edtContext);
      setupEdtDepEvts(edtname, edtContext);
      // Finally remove the pragma for each edt
      removeOcrTaskPragma(edtname, edtContext);
    }
    else if(taskContext->getTaskType() == OcrTaskContext::e_TaskLoopIter) {
      assert(false);
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

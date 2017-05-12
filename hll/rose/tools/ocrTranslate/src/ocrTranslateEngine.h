#ifndef _OCRTRANSLATEENGINE_H
#define _OCRTRANSLATEENGINE_H

/*
 * Author: Sriram Aananthakrishnan, 2017 *
 */

#include "ocrObjectInfo.h"
#include "ocrAstInfo.h"
#include <string>

/**********************
 * TranslateException *
 **********************/
class TranslateException : public std::exception {
  std::string m_what;
 public:
  TranslateException(std::string what);
  virtual const char* what() const throw();
  ~TranslateException() throw();
};

/************************
 * DepElemVarRefExpPass *
 ************************/
//! Top-down AST traversal which replaces the sub-tree (SgVarRefExp)
//! of any depElem with the AST (SgArrowExp(SgVarRefExp, SgVarRefExp))
//! example: x => depElem->x
//! Traversal done in post-order to avoid visiting the transformed AST
class DepElemVarRefExpPass : public AstSimpleProcessing {
  SgScopeStatement* m_scope;
  SgName m_depElemStructName;
  // SgVariableSymbols picked from the task annotation
  // The scope of these symbols is the same as the scope of task annotation
  // Since we just cut-paste the AST from the task annotation to the EDT function
  // these variable symbols are the same as gathered from the task annotation
  // We can directly use the pointers for comparison when replacing the SgVarRefExp
  // of the dependent elements
  std::set<SgVariableSymbol*> m_varSymbolSet;
 public:
  DepElemVarRefExpPass(SgScopeStatement* root, SgName depElemStructName, std::list<SgVarRefExp*> depElems);
  void visit(SgNode* sgn);
  void atTraversalEnd();
};

/*****************
 * OcrTranslator *
 *****************/
//! Driver for OCR Translation
class OcrTranslator {
  SgProject* m_project;
  OcrObjectManager& m_ocrObjectManager;
  AstInfoManager m_astInfoManager;
 private:
  void insertOcrHeaderFiles();
  // Routines for building the EDT Function Definition
  void outlineEdt(std::string edtName, std::list<OcrDbkContextPtr> depDbks, std::list<SgVarRefExp*>& depElems,
		  SgBasicBlock* taskBasicBlock, SgSourceFile* sourcefile,
		  TaskAstInfoPtr taskAstInfo);
  void outlineForLoopBodyEdt(OcrLoopIterEdtContextPtr loopIterEdtContext, SgForStatement* forStmt);
  void outlineForLoopControlEdt(OcrLoopIterEdtContextPtr loopIterEdtContext, SgForStatement* forStmt);
  void insertDepDbkDecl(std::string edtName, std::list<OcrDbkContextPtr>& depDbks,
			unsigned int slotbegin, TaskAstInfoPtr taskAstInfo);
  void insertDepElemDecl(std::string edtName, std::list<SgVarRefExp*>& depElems, TaskAstInfoPtr taskAstInfo);
  void insertEdtReturnStmt(std::string edtName);
  /* void insertDbkDestroyStmts(std::string edtName, std::list<std::string>& dbksToDestroy, unsigned int slotbegin); */
  /* void insertEvtDestroyStmts(std::string edtName, std::list<std::string>& evtsToDestroy, unsigned int slotbegin); */
  // Routine for building the shutdown EDT
  void outlineShutdownEdt(OcrShutdownEdtContextPtr shutdownEdt);
  //! datablock translation involves building the following AST fragments
  //! 1. AST for variable declaration of ocrGuid for the datablock
  //! 2. AST for variable declaration of the pointer (u64*) for the datablock
  //! 3. Replace malloc calls with ocrDbCreate API call
  //! 4. Bookkeeping SgSymbols associated with the datablock
  void translateDbk(std::string name, OcrDbkContextPtr dbkContext);

  // EDT Translation
  std::vector<SgStatement*> setupEdtOutEvt(std::string edtname, std::string outEvt, SgScopeStatement* scope);
  std::vector<SgStatement*> setupEdtTemplate(std::string edtname, unsigned int ndeps, SgScopeStatement* scope);
  std::vector<SgStatement*> setupEdtDepElems(std::string edtname, std::list<SgVarRefExp*>& depElemVarList, SgScopeStatement* scope);
  std::vector<SgStatement*> setupEdtCreate(std::string edtname, std::string outEvtName, bool isFinishEdt, SgScopeStatement* scope);
  std::vector<SgStatement*> setupEdtDepDbks(std::string edtname, std::list<OcrDbkContextPtr>& dbkList, unsigned int slotBegin, SgScopeStatement* scope);
  std::vector<SgStatement*> setupEdtDepEvts(std::string edtname, std::list<OcrEvtContextPtr>& depEvts, unsigned int slotBegin, SgScopeStatement* scope);
  void setupEdtEvtsSatisfy(std::string edtname, OcrEdtContextPtr edtContext);
  void removeOcrTaskPragma(std::string edtname, SgBasicBlock* taskBasicBlock, SgPragmaDeclaration* taskPragma);

  // Loop EDT Translation
  std::vector<SgStatement*> setupForLoopIterCompEvt(std::string iterCompEvtName, SgScopeStatement* scope);

  // Shutdown EDTs
  void setupShutdownEdt(std::string shutdownEdtName, OcrShutdownEdtContextPtr shutdownEdtContext);
  // Miscellaneous utility functions
  std::set<SgSourceFile*> getSourceFilesOfShutdownEdts(std::list<OcrShutdownEdtContextPtr>& shutdownEdts);
  void replaceDepElemVars(std::string edtname, OcrEdtContextPtr edtContext);
 public:
  OcrTranslator(SgProject* project, OcrObjectManager& ocrObjectManager);
  void outlineEdts();
  void setupEdts();
  void translateDbks();
  void outlineMainEdt();
  // main driver function
  void translate();
};

#endif

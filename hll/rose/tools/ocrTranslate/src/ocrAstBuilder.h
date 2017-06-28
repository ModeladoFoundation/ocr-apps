#ifndef _OCRASTBUILDER_H
#define _OCRASTBUILDER_H

/*
 * Author: Sriram Aananthakrishnan, 2017 *
 */

#include <vector>
#include <list>

// High-level API for building fragments of AST for OCR
/**************
 * AstBuilder *
 **************/

namespace AstBuilder {
  class DbkMode {
  public:
    enum DbkModeType {
      DB_DEFAULT_MODE,
      DB_MODE_NULL
    };
  private:
    DbkModeType m_mode;
  public:
    DbkMode(DbkModeType mode)
      : m_mode(mode) { }
    DbkModeType getDbkMode() const {
      return m_mode;
    }
  };

  SgType* buildu32Type(SgScopeStatement* scope);
  SgType* buildu64Type(SgScopeStatement* scope);
  SgType* buildu64PtrType(SgScopeStatement* scope);
  SgType* buildOcrEdtDepType(SgScopeStatement* scope);
  SgType* buildOcrEdtDepArrType(SgScopeStatement* scope);
  SgType* buildOcrGuidType(SgScopeStatement* scope);
  SgType* buildVoidPtrType();
  SgType* buildArgvType();
  SgType* buildOcrGuidArrType(SgScopeStatement* scope, SgExpression* dimExpr);

  // Utility Functions
  std::vector<SgInitializedName*> getDepElemStructMembers(SgClassDeclaration* depElemStructDecl);
  SgVariableDeclaration* buildGuidVarDecl(std::string name, SgScopeStatement* scope);

  /**************************
   * EDT Outlining Builders *
   **************************/
  SgFunctionDeclaration* buildOcrEdtFuncDecl(std::string name, SgScopeStatement* scope);
  std::vector<SgInitializedName*> buildOcrEdtSignature(SgScopeStatement* scope);
  SgTypedefDeclaration* buildTypeDefDecl(std::string edtName, SgType* baseType, SgScopeStatement* scope);
  SgClassDeclaration* buildOcrEdtDepElemStruct(std::string edtName, std::list<SgVarRefExp*>& depElems, SgScopeStatement* scope);
  SgVariableDeclaration* buildOcrEdtDepElemStructDecl(SgType* type, SgName name, SgScopeStatement* scope);

  SgVariableDeclaration* buildEdtDepElemVarDecl(SgVariableSymbol* depElemStructSymbol, SgVariableSymbol* memberVarSymbol, SgScopeStatement* scope);
  std::vector<SgStatement*> buildEdtDepElemVarsDecl(SgClassDeclaration* depElemStructDecl, SgVariableSymbol* depElemStructSymbol, SgScopeStatement* scope);

  SgVariableDeclaration* buildDepDbkPtrDecl(std::string name, SgType* dbkPtrType, unsigned int slot, SgInitializedName* depv,SgScopeStatement* scope);
  SgVariableDeclaration* buildDepDbkGuidDecl(std::string guidName, unsigned int slot, SgInitializedName* depv, SgScopeStatement* scope);
  void buildEdtStmts(SgBasicBlock* from, SgBasicBlock* to);
  SgStatement* buildOcrDbDestroyCallExp(unsigned int slot, SgVariableSymbol* depvSymbol, SgScopeStatement* scope);
  SgStatement* buildEvtDestroyCallExp(unsigned int slot, SgVariableSymbol* depvSymbol, SgScopeStatement* scope);
  SgExprStatement* buildEvtSatisfyCallExp(SgVariableSymbol* evtGuidSymbol, SgScopeStatement* scope);


  /**********************
   * Loop EDT Outlining *
   **********************/
  SgBasicBlock* buildLoopControlIfBody(SgForStatement* forStmt);
  SgBasicBlock* buildLoopControlElseBody();
  SgIfStmt* buildLoopControlIfStmt(SgBasicBlock* ifBasicBlock, SgBasicBlock* elseBasicBlock, SgForStatement* forStmt);

  // AST builders for OCR EDT template setup
  SgExprStatement* buildEdtDepElemSetupStmt(SgVariableSymbol* depElemStructVarSymbol, SgVariableSymbol* depElemMemberVarSymbol, SgVariableSymbol* depElemVarSymbol);
  std::vector<SgStatement*> buildEdtDepElemSetupStmts(SgVariableDeclaration* depElemStructVar, SgClassDeclaration* depElemStructDecl,
						      std::list<SgVarRefExp*> depElemVarList);

  SgVariableDeclaration* buildOcrGuidEdtTemplateVarDecl(std::string edtname, SgScopeStatement* scope);
  SgExprStatement* buildOcrEdtTemplateCallExp(SgVariableDeclaration* edtTemplateGuid, SgFunctionDeclaration* edtFuncDecl,
					      SgExpression* nparamc, unsigned int ndbks, SgScopeStatement* scope);


  SgExprStatement* buildOcrEdtCreateCallExp(SgVariableSymbol* edtGuidSymbol, SgVariableSymbol* edtTemplateGuidSymbol,
					    SgVariableSymbol* depElemStructSymbol,
					    SgVariableSymbol* outEvtGuidSymbol,
					    bool finishEdt,
					    SgScopeStatement* scope);
  SgExprStatement* buildOcrAddDependenceCallExp(SgVariableSymbol* from, SgVariableSymbol* to,
						int slot, DbkMode dbkmode, SgScopeStatement*  scope);
  SgExpression* buildDepElemSizeOfExpr(SgType* depElemType, SgScopeStatement* scope);
  SgVariableDeclaration* buildDepElemSizeVarDecl(std::string varName, SgType* depElemType, SgScopeStatement* scope);

  SgStatement* buildEdtReturnStmt();

  // AST builders for OCR Event management
  SgExprStatement* buildEvtCreateCallExp(SgVariableSymbol* evtGuidSymbol, SgScopeStatement* scope);

  // AST builder for shutdown EDT
  SgExprStatement* buildOcrShutdownCallExp(SgScopeStatement* scope);

  // AST builders for mainEdt
  SgVariableDeclaration* buildMainEdtArgcDecl(SgInitializedName* mainArgc, SgVariableSymbol* mainEdtDbkSymbol, SgScopeStatement* basicblock);
  SgVariableDeclaration* buildMainEdtArgvDecl(SgInitializedName* mainArgv, SgInitializedName* mainArgc, SgScopeStatement* scope);
  SgExpression* buildGetArgcCallExp(SgVariableSymbol* mainDbkSymbol, SgScopeStatement* scope);
  SgExpression* buildGetArgvCallExp(SgVariableSymbol* mainDbkSymbol, SgScopeStatement* scope);

  // Build a function declaration for setting up mainEdtArgs
  SgFunctionDeclaration* buildMainEdtInitFuncDecl(SgName fname, SgInitializedName* mainArgc, SgInitializedName* mainArgv,
						  SgVariableSymbol* mainEdtDbkSymbol, SgGlobal* global);
  SgExprStatement* buildMainEdtInitCallExp(SgName mainEdtInitName, SgVariableSymbol* mainEdtArgcSymbol,
					   SgVariableSymbol* mainEdtArgvSymbol, SgVariableSymbol* mainEdtDbkSymbol, SgScopeStatement* scope);

  /**********************
   * SpmdRegionEdtSetup *
   **********************/
  SgVariableDeclaration* buildGuidArrVarDecl(std::string guidArrVarName, int dim, SgScopeStatement* scope);
  /*!
   * \brief Generate an assignment statement that copies a guid into array of guids at specified index
   */
  SgStatement* buildGuidArrCopyStmt(SgVariableSymbol* guidSymbol, SgVariableSymbol* guidArrSymbol, int index);

  SgVariableDeclaration* buildDbAccessModeArrVarDecl(std::string dbAccessModeArrName, int dim, SgScopeStatement* scope);

  SgStatement* buildDbAccessModeAssignStmt(SgVariableSymbol* dbAccessModeArrSymbol, DbkMode mode, int index);

  SgStatement* buildEdtSpawnCallExp(SgVariableSymbol* edtTemplGuidSymbol, int ntasks, SgVariableSymbol* depElemStructSymbol,
				    SgVariableSymbol* depElemSizeVarSymbol, int ndeps, SgVariableSymbol* depvGuidArrSymbol,
				    SgVariableSymbol* dbAccessModeArrSymbol, int ranksPerAffinity, SgVariableSymbol* outEvtGuidSymbol,
				    SgScopeStatement* scope);
  SgExprStatement* buildSpmdRankFinalizeCallExp(SgVariableSymbol* triggerEvtGuidSymbol, SgScopeStatement* scope);

  SgStatement* buildSpmdMyRankCallExp(SgVarRefExp* rankVarRefExp, SgScopeStatement* scope);
  SgStatement* buildSpmdSizeCallExp(SgVarRefExp* sizeVarRefExp, SgScopeStatement* scope);

  /*********************
   * ReplaceReturnStmt *
   *********************/
  class ReplaceReturnStmt : public AstSimpleProcessing {
  public:
    ReplaceReturnStmt();
    void visit(SgNode* sgn);
  };


  // AST builders for OCR Datablock
  void translateOcrDbk(std::string name, OcrDbkContextPtr dbkContext);
  SgType* buildOcrDbkType(SgType* varType, SgScopeStatement* scope);
  SgVariableDeclaration* buildOcrDbkVarDecl(SgName name, SgType* varDbkType, SgScopeStatement* scope);
  SgExprStatement* buildOcrDbCreateFuncCallExp(SgName dbkGuidName, SgName dbkPtrName, SgScopeStatement* scope, SgNode* sizeOfRoot);
  SgVariableDeclaration* buildOcrDbkGuid(std::string dbkName, SgScopeStatement* scope);
  /*!
   * \brief Build an array variable declaration and wrap it inside a struct declaration
   * @param[in] dbkStructName struct name for the arr datablock
   * @param[in] arrInitializedName must be an SgInitializedName of an array declaration with type SgArrayType
   * @param[in] scope scope where the struct declaration will be created
   * \return Returns a SgClassDeclaration AST node
   */
  SgClassDeclaration* buildArrDbkStructDecl(std::string dbkStructName, SgInitializedName* arrInitializedName, SgScopeStatement* scope);
  /*!
   * \brief Build a typedef type for the struct declaration
   * \return Returns the SgTypedefDeclaration AST node
   */
  SgTypedefDeclaration* buildArrDbkStructTypedefType(SgClassDeclaration* dbkStructDecl, SgScopeStatement* scope);
  SgVariableDeclaration* buildArrPtrDecl(SgInitializedName* arrInitializedName, SgVariableSymbol* dbkPtrSymbol,
					 SgClassDeclaration* dbkStructDecl, SgScopeStatement* scope);
};

#endif

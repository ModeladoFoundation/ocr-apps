#ifndef _OCRASTBUILDER_H
#define _OCRASTBUILDER_H

/*
 * Author: Sriram Aananthakrishnan, 2017 *
 */

#include <vector>
#include <list>

/**************
 * AstBuilder *
 **************/
// High-level API for building fragments of AST for OCR
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

  // EDT Outlining AST builders
  SgFunctionDeclaration* buildOcrEdtFuncDecl(std::string name, SgScopeStatement* scope);
  std::vector<SgInitializedName*> buildOcrEdtSignature(SgScopeStatement* scope);
  SgClassDeclaration* buildOcrEdtDepElemStruct(OcrEdtContextPtr edtContext, SgFunctionDeclaration* decl);
  SgTypedefDeclaration* buildTypeDefDecl(std::string edtName, SgType* baseType, SgScopeStatement* scope);
  SgVariableDeclaration* buildOcrEdtDepElemStructDecl(SgType* type, SgName name, SgScopeStatement* scope);
  std::vector<SgStatement*> buildOcrDbksDecl(OcrEdtContextPtr edtContext, SgScopeStatement* scope, SgFunctionDeclaration* edtDecl);
  std::vector<SgStatement*> buildOcrEdtStmts(OcrEdtContextPtr edtContext);

  // AST builders for OCR EDT template setup
  SgVariableDeclaration* buildOcrGuidEdtTemplateVarDecl(std::string edtname, SgScopeStatement* scope);
  SgExprStatement* buildOcrEdtTemplateCallExp(SgVariableDeclaration* edtTemplateGuid, SgFunctionDeclaration* edtFuncDecl,
					      unsigned int ndelems, unsigned int ndbks, SgScopeStatement* scope);
  std::vector<SgStatement*> buildEdtDepElemSetupStmts(SgVariableDeclaration* depElemStructVar, OcrEdtContextPtr edtContext);
  SgExprStatement* buildOcrEdtCreateCallExp(SgVariableSymbol* edtGuidSymbol, SgVariableSymbol* edtTemplateGuidSymbol,
					    SgVariableSymbol* depElemStructSymbol,
					    SgVariableSymbol* outEvtGuidSymbol, SgScopeStatement* scope);
  SgExprStatement* buildOcrAddDependenceCallExp(SgVariableSymbol* dbkGuidSymbol, SgVariableSymbol* edtGuidSymbol,
						int slot, DbkMode dbkmode, SgScopeStatement*  scope);

  // AST builders for OCR Event management
  SgExprStatement* buildEvtCreateCallExp(SgVariableSymbol* evtGuidSymbol, SgScopeStatement* scope);

  // AST builders for OCR Datablock
  void translateOcrDbk(std::string name, OcrDbkContextPtr dbkContext);
  SgType* buildOcrDbkType(SgType* varType, SgScopeStatement* scope);
  SgVariableDeclaration* buildOcrDbkVarDecl(SgName name, SgType* varDbkType, SgScopeStatement* scope);
  SgExprStatement* buildOcrDbCreateFuncCallExp(SgName dbkGuidName, SgName dbkPtrName, SgScopeStatement* scope, SgStatement* allocStmt);
  SgVariableDeclaration* buildOcrDbkGuid(std::string dbkName, SgScopeStatement* scope);
};

#endif

#ifndef _OCRASTBUILDER_H
#define _OCRASTBUILDER_H

/*
 * Author: Sriram Aananthakrishnan, 2017 *
 */

/**************
 * AstBuilder *
 **************/
// High-level API for building fragments of AST for OCR
namespace AstBuilder {
  SgType* buildu32Type(SgScopeStatement* scope);
  SgType* buildu64Type(SgScopeStatement* scope);
  SgType* buildu64PtrType(SgScopeStatement* scope);
  SgType* buildOcrEdtDepType(SgScopeStatement* scope);
  SgType* buildOcrEdtDepArrType(SgScopeStatement* scope);
  SgFunctionDeclaration* buildOcrEdtFuncDecl(std::string name, SgScopeStatement* scope);
  SgVariableDeclaration* buildOcrEdtDepElem(SgVarRefExp* vref, SgScopeStatement* scope, SgFunctionDeclaration* decl);
  SgClassDeclaration* buildOcrEdtDepElems(OcrEdtContextPtr edtContext, SgFunctionDeclaration* decl);
  void insertOcrEdtDepElemDecl(SgClassDeclaration* sdecl, SgScopeStatement* scope);
  void buildOcrEdtParams(SgFunctionDeclaration* edtdecl, SgScopeStatement* scope);
  void buildOcrEdtStmts(OcrEdtContextPtr edtContext, SgScopeStatement* scopes);
  SgFunctionDeclaration* buildOcrEdt(std::string name, OcrEdtContextPtr edtContext);

  typedef std::map<std::string, SgClassDeclaration*> EdtDepElemSgTypeMap;
};

#endif

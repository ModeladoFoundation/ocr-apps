#ifndef _OCRTRANSLATEENGINE_H
#define _OCRTRANSLATEENGINE_H

/*
 * Author: Sriram Aananthakrishnan, 2017 *
 */

#include "ocrObjectInfo.h"
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

/*****************
 * OcrTranslator *
 *****************/
class OcrTranslator {
  SgProject* m_project;
  const OcrObjectManager& m_ocrObjectManager;
 private:
  void insertOcrHeaderFiles();
  void outlineEdt(std::string, OcrEdtContextPtr edt);
 public:
  OcrTranslator(SgProject* project, const OcrObjectManager& ocrObjectManager);
  void outlineEdts();
  void translate();
};

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
  SgFunctionParameterList* buildOcrEdtParams(SgFunctionDeclaration* edtdecl);
  SgFunctionDeclaration* buildOcrEdt(std::string name, OcrEdtContextPtr edtContext);
};


#endif

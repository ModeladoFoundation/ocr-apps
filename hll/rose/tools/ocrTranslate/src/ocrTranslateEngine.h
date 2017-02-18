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

// We will do some book keeping for the generated AST fragments
// during the translation phase.
// Useful in accessing them at various points of translation
// Motivation : We will need the GUIDs of datablock, edts and events
// at various steps of generating the AST for OCR objects
/*****************
 * OcrDbkAstInfo *
 *****************/
class OcrDbkAstInfo {
  std::string m_dbkname;
  // SgSymbol represents the concept of a name in ROSE
  // With a symbol we can access a variable's declaration, scope and build SgVarRefExp
  //! SgSymbol associated with the ocrGuid of the datablock
  SgVariableSymbol* m_ocrGuidSymbol;
  //! SgSymbol associated with the ptr of the datablock
  SgVariableSymbol* m_ptrSymbol;
 public:
  OcrDbkAstInfo(std::string dbkname, SgVariableSymbol* ocrGuidSymbol, SgVariableSymbol* ptrSymbol);
  SgVariableSymbol* getOcrGuidSymbol() const;
  SgVariableSymbol* getPtrSymbol() const;
  std::string str() const;
};
typedef boost::shared_ptr<OcrDbkAstInfo> OcrDbkAstInfoPtr;

/*****************
 * OcrEdtAstInfo *
 *****************/
class OcrEdtAstInfo {
  SgType* depElemType;
  SgFunctionSymbol* edtFuncSymbol;
 public:
  OcrEdtAstInfo();
  std::string str() const;
};
typedef boost::shared_ptr<OcrEdtAstInfo> OcrEdtAstInfoPtr;

/*********************
 * OcrAstInfoManager *
 *********************/
typedef std::map<std::string, OcrDbkAstInfoPtr> OcrDbkAstInfoMap;
typedef std::pair<std::string, OcrDbkAstInfoPtr> OcrDbkAstInfoMapElem;
class OcrAstInfoManager {
  OcrDbkAstInfoMap m_ocrDbkAstInfoMap;
 public:
  OcrAstInfoManager();
  bool regOcrDbkAstInfo(std::string dbkname, SgVariableSymbol* ocrGuidSymbol, SgVariableSymbol* ptrSymbol);
  OcrDbkAstInfoPtr getOcrDbkAstInfo(std::string dbkname);
};

/*****************
 * OcrTranslator *
 *****************/
//! Driver for OCR Translation
class OcrTranslator {
  SgProject* m_project;
  const OcrObjectManager& m_ocrObjectManager;
  OcrAstInfoManager m_ocrAstInfoManager;
 private:
  void insertOcrHeaderFiles();
  void outlineEdt(std::string, OcrEdtContextPtr edt);
  //! datablock translation involves building the following AST fragments
  //! 1. AST for variable declaration of ocrGuid for the datablock
  //! 2. AST for variable declaration of the pointer (u64*) for the datablock
  //! 3. Replace malloc calls with ocrDbCreate API call
  //! 4. Bookkeeping SgSymbols associated with the datablock
  void translateDbk(std::string name, OcrDbkContextPtr dbkContext);
 public:
  OcrTranslator(SgProject* project, const OcrObjectManager& ocrObjectManager);
  void outlineEdts();
  //
  void translateDbks();
  // main driver function
  void translate();
};

#endif

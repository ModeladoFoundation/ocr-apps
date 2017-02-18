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
  return "OcrDbkAstInfo";
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

//! Outline the Edt to a function
void OcrTranslator::outlineEdt(string edtName, OcrEdtContextPtr edtContext) {
  Logger::Logger lg("OcrTranslator::outlineEdt");
  AstBuilder::buildOcrEdt(edtName, edtContext);
}

void OcrTranslator::translateDbk(string dbkName, OcrDbkContextPtr dbkContext) {
  AstBuilder::translateOcrDbk(dbkName, dbkContext);
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
    outlineEdts();
    translateDbks();
  }
  catch(TranslateException& ewhat) {
    Logger::error(lg) << ewhat.what() << endl;
  }
}

/*
 * Author: Sriram Aananthakrishnan, 2017 *
 */

#include "sage3basic.h"
#include "ocrTranslateEngine.h"
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

/**************
 * AstBuilder *
 **************/
namespace AstBuilder {
  using namespace SageBuilder;
  // Each push and pop are operations on stl list
  // Under a common use case all the type building functions
  // will be used under the same scope.
  // Is is profitable to assume a toplevel scope stack and
  // not do push and pop for every type building function?
  // Both push and pop are O(1) operation.
  // Using a scope as a parameter however forces to build the
  // types under the desired scope and ensures modularity and correctness.
  // If the number of push/pops are too many then consider assuming a
  // toplevel scope and remove the redundant push/pop
  SgType* buildu32Type(SgScopeStatement* scope) {
    SgType* u32_t = SageBuilder::buildOpaqueType("u32", scope);
    return u32_t;
  }

  SgType* buildu64Type(SgScopeStatement* scope) {
    SgType* u64_t = SageBuilder::buildOpaqueType("u64", scope);
    return u64_t;
  }

  SgType* buildu64PtrType(SgScopeStatement* scope) {
    assert(scope);
    SgType* btype = buildu64Type(scope);
    SgType* u64_p = SageBuilder::buildPointerType(btype);
    return u64_p;
  }

  SgType* buildOcrEdtDepType(SgScopeStatement* scope) {
    assert(scope);
    SageBuilder::pushScopeStack(scope);
    SgType* edtDep_t = SageBuilder::buildOpaqueType("ocrEdtDep_t", scope);
    SageBuilder::popScopeStack();
    return edtDep_t;
  }

  SgType* buildOcrEdtDepArrType(SgScopeStatement* scope) {
    assert(scope);
    SgType* btype = buildOcrEdtDepType(scope);
    SageBuilder::pushScopeStack(scope);
    SgType* edtDepArr_t = SageBuilder::buildArrayType(btype);
    SageBuilder::popScopeStack();
    return edtDepArr_t;
  }

  SgFunctionDeclaration* buildOcrEdtFuncDecl(string name, SgScopeStatement* scope) {
    // return type of the EDT
    SgType* void_t = SageBuilder::buildVoidType();
    SageBuilder::pushScopeStack(scope);
    SgFunctionParameterList* paramList = SageBuilder::buildFunctionParameterList();
    SgFunctionDeclaration* edtdecl = SageBuilder::buildDefiningFunctionDeclaration(name, void_t, paramList, scope);
    SageBuilder::popScopeStack();
    return edtdecl;
  }

  SgFunctionParameterList* buildOcrEdtParams(SgFunctionDeclaration* edtdecl) {
    assert(edtdecl);
    // The scope of the parameter is the scope of the function body
    SgScopeStatement* scope = isSgScopeStatement(edtdecl->get_definition()->get_body());
    assert(scope);
    SgFunctionParameterList* paramList = edtdecl->get_parameterList();
    // Build the parameters
    SgType* u32_t = buildu32Type(scope);
    SgInitializedName* paramc = SageBuilder::buildInitializedName("paramc", u32_t);
    SageInterface::appendArg(paramList, paramc);

    SgType* u64p_t = buildu64PtrType(scope);
    SgInitializedName* paramv = SageBuilder::buildInitializedName("paramv", u64p_t);
    SageInterface::appendArg(paramList, paramv);

    SgInitializedName* depc = SageBuilder::buildInitializedName("depc", u32_t);
    SageInterface::appendArg(paramList, depc);

    SgType* edtDep_t = buildOcrEdtDepArrType(scope);
    SgInitializedName* depv = SageBuilder::buildInitializedName("depv", edtDep_t);
    SageInterface::appendArg(paramList, depv);

    return paramList;
  }

  SgFunctionDeclaration* buildOcrEdt(string name, OcrEdtContextPtr edtContext) {
    Logger::Logger lg("AstBuilder::buildOcrEdt");
    SgSourceFile* sourcefile = edtContext->getSourceFile();
    // scope where the edt function will be created
    SgGlobal* global = sourcefile->get_globalScope();
    // position in the AST where the EDT function will be inserted
    SgStatement* first = SageInterface::getFirstStatement(global);
    // Function created is a defining declaration which means
    // the declaration has an empty body
    SgFunctionDeclaration* edtdecl = buildOcrEdtFuncDecl(name, global);
    Logger::debug(lg) << "edtdecl:" << AstDebug::astToString(edtdecl) << endl;
    // Set up the parameters for the EDT
    buildOcrEdtParams(edtdecl);
    SageInterface::insertStatement(first, edtdecl, true, true);
    return edtdecl;
  }
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

void OcrTranslator::outlineEdts() {
  const OcrEdtObjectMap& edtMap = m_ocrObjectManager.getOcrEdtObjectMap();
  OcrEdtObjectMap::const_iterator e = edtMap.begin();
  for( ; e != edtMap.end(); ++e) {
    outlineEdt(e->first, e->second);
  }
}

void OcrTranslator::translate() {
  Logger::Logger lg("OcrTranslator::translate");
  try {
    // insert the header files
    insertOcrHeaderFiles();
    outlineEdts();
  }
  catch(TranslateException& ewhat) {
    Logger::error(lg) << ewhat.what() << endl;
  }
}

/*
 * Author: Sriram Aananthakrishnan, 2017 *
 */

#include "sage3basic.h"
#include "ocrTranslateEngine.h"
#include "logger.h"

/**************
 * AstBuilder *
 **************/
namespace AstBuilder {
  using namespace SageBuilder;
  using namespace std;
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

  void buildOcrEdtParams(SgFunctionParameterList* paramList, SgScopeStatement* scope) {
    assert(scope);
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
  }

  SgVariableDeclaration* buildOcrEdtDepElem(SgVarRefExp* vref, SgScopeStatement* scope) {
    SgVariableDeclaration* vdecl = SageBuilder::buildVariableDeclaration(vref->get_symbol()->get_name(),
									 vref->get_type(),
									 NULL,
									 scope);
    assert(vdecl);
    return vdecl;
  }

  SgClassDeclaration* buildOcrEdtDepElems(OcrEdtContextPtr edtContext, SgFunctionDeclaration* decl) {
    // Build a struct for all the parameters
    string depElemStructName = edtContext->get_name() + "_depElem_t";
    SgClassDeclaration* depElemStructDecl = SageBuilder::buildStructDeclaration(depElemStructName, decl->get_scope());
    list<SgVarRefExp*> depElems = edtContext->getDepElems();
    list<SgVarRefExp*>::iterator v = depElems.begin();
    SgClassDefinition* sdefn = depElemStructDecl->get_definition();
    assert(sdefn);
    for( ; v != depElems.end(); ++v) {
      SgVariableDeclaration* vdecl = buildOcrEdtDepElem(*v, sdefn);
      sdefn->append_member(vdecl);
    }
    return depElemStructDecl;
  }

  void buildOcrEdtStmts(OcrEdtContextPtr edtContext, SgFunctionDeclaration* edtDecl) {
    SgScopeStatement* basicblock = isSgScopeStatement(edtDecl->get_definition()->get_body());
    assert(basicblock);
    // Set up the parameters for the EDT
    buildOcrEdtParams(edtDecl->get_parameterList(), basicblock);
    // Outline the Task's statements to the EDT function
    list<SgStatement*> stmtList = edtContext->getStmtList();
    list<SgStatement*>::iterator s = stmtList.begin();
    for( ; s != stmtList.end(); ++s) {
      // Detach the statement from its original scope
      SageInterface::removeStatement(*s);
      // Add the statement into the current scope
      SageInterface::appendStatement(*s, basicblock);
    }
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
    SgClassDeclaration* depElemStruct = buildOcrEdtDepElems(edtContext, edtdecl);
    SageInterface::insertStatement(first, edtdecl, true, true);
    SageInterface::insertStatementBefore(edtdecl, depElemStruct, true);
    buildOcrEdtStmts(edtContext, edtdecl);
    Logger::debug(lg) << "edtdecl:" << AstDebug::astToString(edtdecl) << endl;
    return edtdecl;
  }
}

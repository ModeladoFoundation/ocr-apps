/*
 * Author: Sriram Aananthakrishnan, 2017 *
 */

#include "sage3basic.h"
#include "ocrTranslateEngine.h"
#include "RoseAst.h"
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
    string depElemStructName = edtContext->get_name() + "DepElems";
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

  SgTypedefDeclaration* buildTypeDefDecl(string edtName, SgType* baseType, SgScopeStatement* scope) {
    string typedefName = edtName + "DepElem_t";
    return SageBuilder::buildTypedefDeclaration(typedefName, baseType, scope, true);
  }

  void buildOcrEdtStmts(OcrEdtContextPtr edtContext, SgScopeStatement* scope) {
    assert(scope);
    // Outline the Task's statements to the EDT function
    list<SgStatement*> stmtList = edtContext->getStmtList();
    list<SgStatement*>::iterator s = stmtList.begin();
    for( ; s != stmtList.end(); ++s) {
      // Detach the statement from its original scope
      SageInterface::removeStatement(*s);
      // Add the statement into the current scope
      SageInterface::appendStatement(*s, scope);
    }
  }

  SgSymbol* insertOcrEdtDepElemDecl(SgType* type, SgScopeStatement* scope) {
    assert(scope);
    SgPointerType* ptype = SageBuilder::buildPointerType(type);
    SgName vname("depElem");
    SgVariableDeclaration* vdecl = SageBuilder::buildVariableDeclaration(vname, ptype, NULL, scope);
    SageInterface::appendStatement(vdecl, scope);
    return vdecl->get_decl_item(vname)->get_symbol_from_symbol_table();
  }

  void varRefExp2ArrowExpInStmt(SgVarRefExp* oexp, SgArrowExp* nexp, SgStatement* stmt) {
    RoseAst ast(stmt);
    RoseAst::iterator it = ast.begin();
    for( ; it != ast.end(); ++it) {
      if(SgVarRefExp* vref = isSgVarRefExp(*it)) {
	if(vref->get_symbol() == oexp->get_symbol()) {
	  SageInterface::replaceExpression(vref, nexp, false);
	}
      }
    }
  }

  void varRefExp2ArrowExp(SgVarRefExp* oexp, SgArrowExp* nexp, SgStatementPtrList& statements) {
    SgStatementPtrList::iterator s = statements.begin();
    for( ; s != statements.end(); ++s) {
      varRefExp2ArrowExpInStmt(oexp, nexp, *s);
    }
  }

  void replaceDepElemVars(SgSymbol* depElemSymbol, SgScopeStatement* scope,
			  SgStatementPtrList& statements, OcrEdtContextPtr edtContext) {
    list<SgVarRefExp*> depElemList = edtContext->getDepElems();
    list<SgVarRefExp*>::iterator e = depElemList.begin();
    for( ; e != depElemList.end(); ++e) {
      SgVariableSymbol* dsymbol = (*e)->get_symbol();
      SgVarRefExp* rexp = SageBuilder::buildVarRefExp(dsymbol->get_name(), scope);
      SgVarRefExp* lexp = SageBuilder::buildVarRefExp(depElemSymbol->get_name(), scope);
      SgArrowExp* arrowExp = SageBuilder::buildArrowExp(lexp, rexp);
      varRefExp2ArrowExp(*e, arrowExp, statements);
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
    SgTypedefDeclaration* depElemTypedefType = buildTypeDefDecl(name, depElemStruct->get_type(), global);
    // Set up the parameters for the EDT
    // Get the basic block for the EDT function for inserting parameters, statements etc.
    SgBasicBlock* basicblock = edtdecl->get_definition()->get_body();
    buildOcrEdtParams(edtdecl->get_parameterList(), basicblock);
    SgSymbol* depElemSymbol = insertOcrEdtDepElemDecl(depElemTypedefType->get_type(), basicblock);
    SageInterface::insertStatement(first, edtdecl, true, true);
    SageInterface::insertStatementBefore(edtdecl, depElemStruct, true);
    SageInterface::insertStatementAfter(depElemStruct, depElemTypedefType, true);
    buildOcrEdtStmts(edtContext, basicblock);
    replaceDepElemVars(depElemSymbol, basicblock, basicblock->get_statements(), edtContext);
    Logger::debug(lg) << "edtdecl:" << AstDebug::astToString(edtdecl) << endl;
    return edtdecl;
  }
}

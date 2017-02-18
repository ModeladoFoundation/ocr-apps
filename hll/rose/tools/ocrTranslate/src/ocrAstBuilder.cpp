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

  /*****************
   * Type Builders *
   *****************/
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

  SgType* buildVoidPtrPtrType(SgScopeStatement* scope) {
    SgType* voidType = SageBuilder::buildVoidType();
    SgPointerType* voidPtrType = SageBuilder::buildPointerType(voidType);
    SgPointerType* voidPtrPtrType = SageBuilder::buildPointerType(voidPtrType);
    return voidPtrPtrType;
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

  SgType* buildOcrGuidType(SgScopeStatement* scope) {
    assert(scope);
    SgType* ocrGuidType = SageBuilder::buildOpaqueType("ocrGuid_t", scope);
    return ocrGuidType;
  }

  SgTypedefDeclaration* buildTypeDefDecl(string edtName, SgType* baseType, SgScopeStatement* scope) {
    string typedefName = edtName + "DepElem_t";
    return SageBuilder::buildTypedefDeclaration(typedefName, baseType, scope, true);
  }
  /************************
   * Builders for OCR EDT *
   ************************/
  SgFunctionDeclaration* buildOcrEdtFuncDecl(string name, SgScopeStatement* scope) {
    // return type of the EDT
    SgType* void_t = SageBuilder::buildVoidType();
    SageBuilder::pushScopeStack(scope);
    SgFunctionParameterList* paramList = SageBuilder::buildFunctionParameterList();
    SgFunctionDeclaration* edtdecl = SageBuilder::buildDefiningFunctionDeclaration(name, void_t, paramList, scope);
    SageBuilder::popScopeStack();
    return edtdecl;
  }

  void buildOcrEdtSignature(SgFunctionParameterList* paramList, SgScopeStatement* scope) {
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
    SgVarRefExp* paramvVarRef = SageBuilder::buildVarRefExp("paramv", scope);
    SgIntVal* zero = SageBuilder::buildIntVal(0);
    SgPntrArrRefExp* arrRefExp = SageBuilder::buildPntrArrRefExp(paramvVarRef, zero);
    SgAssignInitializer* initializer = SageBuilder::buildAssignInitializer(arrRefExp, ptype);
    SgVariableDeclaration* vdecl = SageBuilder::buildVariableDeclaration(vname, ptype, initializer, scope);
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

  SgVariableDeclaration* buildOcrDbkDecl(OcrDbkContextPtr dbkContext, unsigned int index,
		       SgInitializedName* depv,SgScopeStatement* scope) {
    SgType* u64PtrType = buildu64PtrType(scope);
    SgVariableSymbol* vsymbol = isSgVariableSymbol(depv->get_symbol_from_symbol_table());
    assert(vsymbol);
    SgVarRefExp* depvVarRefExp = SageBuilder::buildVarRefExp(vsymbol);
    SgIntVal* sgIndex = SageBuilder::buildIntVal(index);
    SgPntrArrRefExp* arrRefExp = SageBuilder::buildPntrArrRefExp(depvVarRefExp, sgIndex);
    SgVarRefExp* ptrVarRefExp = SageBuilder::buildVarRefExp("ptr", scope);
    SgVariableSymbol* ptrVarSymbol = ptrVarRefExp->get_symbol();
    // Fix for avoiding ROSE Warnings
    scope->get_symbol_table()->insert(ptrVarSymbol->get_name(), ptrVarSymbol);
    SgDotExp* dotExp = SageBuilder::buildDotExp(arrRefExp, ptrVarRefExp);
    SgAssignInitializer* initializer = SageBuilder::buildAssignInitializer(dotExp, u64PtrType);
    SgName vname = dbkContext->getSgSymbol()->get_name();
    SgVariableDeclaration* vdecl = SageBuilder::buildVariableDeclaration(vname, u64PtrType, initializer, scope);
    return vdecl;
  }

  vector<SgStatement*> buildOcrDbksDecl(OcrEdtContextPtr edtContext, SgScopeStatement* scope, SgFunctionDeclaration* edtDecl) {
    list<OcrDbkContextPtr> depDbks = edtContext->getDepDbks();
    vector<SgStatement*> depDbksDecl;
    // for each datablock set up the declaration from depv
    SgInitializedNamePtrList& args = edtDecl->get_args();
    // depv is the last element in the EDT argument list
    SgInitializedName* depv = args.back();
    assert(depv);
    list<OcrDbkContextPtr>::iterator d = depDbks.begin();
    for(unsigned int index=0 ; d != depDbks.end(); ++d, ++index) {
      SgVariableDeclaration* vdecl = buildOcrDbkDecl(*d, index, depv, scope);
      SgStatement* declStmt = isSgStatement(vdecl);
      assert(declStmt);
      depDbksDecl.push_back(declStmt);
    }
    return depDbksDecl;
  }

  void replaceDepElemVars(SgSymbol* depElemSymbol, SgScopeStatement* scope,
			  SgStatementPtrList& statements, OcrEdtContextPtr edtContext) {
    list<SgVarRefExp*> depElemList = edtContext->getDepElems();
    list<SgVarRefExp*>::iterator e = depElemList.begin();
    for( ; e != depElemList.end(); ++e) {
      SgVariableSymbol* dsymbol = (*e)->get_symbol();
      SgName dname = dsymbol->get_name();
      SgVarRefExp* rexp = SageBuilder::buildVarRefExp(dname, scope);
      SgVarRefExp* lexp = SageBuilder::buildVarRefExp(depElemSymbol->get_name(), scope);
      SgArrowExp* arrowExp = SageBuilder::buildArrowExp(lexp, rexp);
      // rexp is rhs of arrow exp (e.g. depElem->var)
      // When depElem is declared in basicblock, a symbol
      // for depElem is inserted in to the basic block
      // When building the arrow exp, we build the rhs using the name
      // The variable may not have a symbol in the basic block
      // This later shows up as a Warning in AST consistency checks
      // To avoid this, the symbol for var is inserted into the symbol table.
      // This seems to fix the AST consistency check
      // However, I need to check if this is the way for building dot and arrow
      // expressions
      scope->get_symbol_table()->insert(dname, rexp->get_symbol());
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
    buildOcrEdtSignature(edtdecl->get_parameterList(), basicblock);
    SgSymbol* depElemSymbol = insertOcrEdtDepElemDecl(depElemTypedefType->get_type(), basicblock);
    SageInterface::insertStatement(first, edtdecl, true, true);
    SageInterface::insertStatementBefore(edtdecl, depElemStruct, true);
    SageInterface::insertStatementAfter(depElemStruct, depElemTypedefType, true);
    vector<SgStatement*> depDbksDecl = buildOcrDbksDecl(edtContext, basicblock, edtdecl);
    SageInterface::insertStatementAfterLastDeclaration(depDbksDecl, basicblock);
    buildOcrEdtStmts(edtContext, basicblock);
    replaceDepElemVars(depElemSymbol, basicblock, basicblock->get_statements(), edtContext);
    Logger::debug(lg) << "edtdecl:" << AstDebug::astToString(edtdecl) << endl;
    return edtdecl;
  }

  /**************************
   * Ocr Datablock Builders *
   **************************/
  SgType* buildOcrDbkType(SgType* varType, SgScopeStatement* scope) {
    switch(varType->variantT()) {
    case V_SgPointerType:
      return buildu64PtrType(scope);
      break;
    case V_SgArrayType:
      assert(false);
      break;
    default:
      assert(false);
    }
  }

  SgVariableDeclaration* buildOcrDbkVarDecl(SgName name, SgType* varDbkType, SgScopeStatement* scope) {
    return SageBuilder::buildVariableDeclaration(name, varDbkType, NULL, scope);
  }

  SgVariableDeclaration* buildOcrDbkGuid(string dbkName, SgScopeStatement* scope) {
    SgType* ocrGuidType = buildOcrGuidType(scope);
    return SageBuilder::buildVariableDeclaration(dbkName, ocrGuidType, NULL, scope);
  }

  // Given an allocStmt extract the len in bytes
  SgExpression* extractSizeInBytes(SgStatement* allocStmt) {
    class ExtractMallocArgument : public AstSimpleProcessing {
      SgExpression* m_arg;
    public:
      ExtractMallocArgument() { }
      void visit(SgNode* sgn) {
	if(SgFunctionCallExp* mallocCallExp = isSgFunctionCallExp(sgn)) {
	  string name = mallocCallExp->getAssociatedFunctionSymbol()->get_name().getString();
	  if(name.compare("malloc") == 0 ||
	     name.compare("calloc") == 0) {
	    SgExprListExp* callExprListExp = mallocCallExp->get_args();
	    SgExpressionPtrList& exprList = callExprListExp->get_expressions();
	    assert(exprList.size() == 1);
	    m_arg = exprList[0];
	  }
	}
      }

      void atTraversalEnd() {
	assert(m_arg);
      }

      SgExpression* getMallocArg() const {
	return m_arg;
      }
    };
    ExtractMallocArgument extractMallocArg;
    extractMallocArg.traverse(allocStmt, preorder);
    SgExpression* arg = extractMallocArg.getMallocArg();
    assert(arg);
    return arg;
  }

  // Signature for ocrDbCreate
  // u8 ocrDbCreate( ocrGuid_t ∗ db, void ∗∗ addr, u64 len, u16
  // 		  flags, ocrHint_t ∗ hint, ocrInDbAllocator_t allocator )
  SgExprStatement* buildOcrDbCreateFuncCallExp(SgName dbkGuidName, SgName dbkPtrName, SgScopeStatement* scope, SgStatement* allocStmt) {
    SgType* voidType = SageBuilder::buildVoidType();
    // Arguments for ocrDbCreate
    vector<SgExpression*> args;
    SgVarRefExp* ocrGuidVarRefExp = SageBuilder::buildVarRefExp(dbkGuidName, scope);
    // Build the first argument
    SgExpression* first = SageBuilder::buildAddressOfOp(ocrGuidVarRefExp);
    args.push_back(first);
    // Build the second argument
    SgVarRefExp* varDbkVarRefExp = SageBuilder::buildVarRefExp(dbkPtrName, scope);
    SgType* castType = buildVoidPtrPtrType(scope);
    SgCastExp* castExp = SageBuilder::buildCastExp(varDbkVarRefExp, castType);
    args.push_back(castExp);
    // third argument is len
    // this information is either in a malloc call or in the declaration
    SgExpression* third = extractSizeInBytes(allocStmt);
    args.push_back(third);
    // Build the fourth argument
    // fourth argument is flags
    // default is DB_PROP_NONE
    string pflag = "DB_PROP_NONE";
    SgIntVal* fourth = SageBuilder::buildIntVal();
    SageInterface::addTextForUnparser(fourth, pflag, AstUnparseAttribute::e_replace);
    args.push_back(fourth);
    // Build the fifth argument
    // Fifth argument is hint
    // default value: NULL_HINT
    SgIntVal* fifth = SageBuilder::buildIntVal();
    string nullhint = "NULL_HINT";
    SageInterface::addTextForUnparser(fifth, nullhint, AstUnparseAttribute::e_replace);
    args.push_back(fifth);
    // Build the sixth argument
    // Sixth argument is allocator
    // default : NO_ALLOC
    SgIntVal* sixth = SageBuilder::buildIntVal();
    string noalloc_ = "NO_ALLOC";
    SageInterface::addTextForUnparser(sixth, noalloc_, AstUnparseAttribute::e_replace);
    args.push_back(sixth);
    // Build the argument list
    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    // Build the
    SgExprStatement* stmt = SageBuilder::buildFunctionCallStmt("ocrDbCreate", voidType, exprList, scope);
    return stmt;
  }

  void translateOcrDbk(string dbkName, OcrDbkContextPtr dbkContext) {
    Logger::Logger lg("AstBuilder::translateOcrDbk", Logger::DEBUG);
    set<SgStatement*> stmtsToRemove;
    SgInitializedName* varInitializedName = dbkContext->getSgInitializedName();
    SgSymbol* varSymbol = dbkContext->getSgSymbol();
    SgScopeStatement* scope = varSymbol->get_scope();
    SgName varName = varSymbol->get_name();
    SgName ocrGuidName(dbkName);
    SgType* varType = varSymbol->get_type();
    SgType* varDbkType = buildOcrDbkType(varType, scope);
    SgVariableDeclaration* varDbkDecl = buildOcrDbkVarDecl(varName, varDbkType, scope);
    SgVariableDeclaration* varDbkGuid = buildOcrDbkGuid(ocrGuidName.getString(), scope);
    // AST Modifications
    // stmt is the declaration of the datablock variable
    // We don't need the declaration anymore
    // Remove the stmt and insert datablock declaration instead
    SgStatement* stmt = SageInterface::getEnclosingStatement(varInitializedName);
    // First get the anchor point where the stmt will be inserted
    SageInterface::insertStatementBefore(stmt, varDbkDecl, true);
    SageInterface::insertStatementBefore(stmt, varDbkGuid, true);
    stmtsToRemove.insert(stmt);
    // Replace each alloc stmt with ocrDbCreate
    list<SgStatement*> allocStmts = dbkContext->get_allocStmts();
    list<SgStatement*>::iterator s = allocStmts.begin();
    for( ; s != allocStmts.end(); ++s) {
      SgExprStatement* dbCreateStmt = buildOcrDbCreateFuncCallExp(ocrGuidName, varName, scope, *s);
      SgStatement* stmtAfterAllocStmt = SageInterface::getNextStatement(*s);
      SageInterface::insertStatementBefore(stmtAfterAllocStmt, dbCreateStmt, true);
      // Mark the statement for removal
      stmtsToRemove.insert(*s);
    }
    // Now remove all the statements marked for removal
    set<SgStatement*>::iterator st = stmtsToRemove.begin();
    for( ; st != stmtsToRemove.end(); ++st) {
      SageInterface::removeStatement(*st);
    }
  }
}

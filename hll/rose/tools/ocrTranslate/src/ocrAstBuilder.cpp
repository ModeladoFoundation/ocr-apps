/*
 * Author: Sriram Aananthakrishnan, 2017 *
 */

#include "sage3basic.h"
#include "ocrTranslateEngine.h"
#include "RoseAst.h"
#include "ocrAstBuilder.h"
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
    SgType* returnType = buildOcrGuidType(scope);
    SgFunctionParameterList* paramList = SageBuilder::buildFunctionParameterList();
    SgFunctionDeclaration* edtdecl = SageBuilder::buildDefiningFunctionDeclaration(name, returnType, paramList, scope);
    return edtdecl;
  }

  vector<SgInitializedName*> buildOcrEdtSignature(SgScopeStatement* scope) {
    assert(scope);
    vector<SgInitializedName*> edt_params;
    // Build the parameters
    SgType* u32_t = buildu32Type(scope);
    SgInitializedName* paramc = SageBuilder::buildInitializedName("paramc", u32_t);
    edt_params.push_back(paramc);

    SgType* u64p_t = buildu64PtrType(scope);
    SgInitializedName* paramv = SageBuilder::buildInitializedName("paramv", u64p_t);
    edt_params.push_back(paramv);

    SgInitializedName* depc = SageBuilder::buildInitializedName("depc", u32_t);
    edt_params.push_back(depc);

    SgType* edtDep_t = buildOcrEdtDepArrType(scope);
    SgInitializedName* depv = SageBuilder::buildInitializedName("depv", edtDep_t);
    edt_params.push_back(depv);
    return edt_params;
  }

  SgVariableDeclaration* buildOcrEdtDepElemVarDecl(SgVarRefExp* vref, SgScopeStatement* scope) {
    SgVariableDeclaration* vdecl = SageBuilder::buildVariableDeclaration(vref->get_symbol()->get_name(),
									 vref->get_type(),
									 NULL,
									 scope);
    assert(vdecl);
    return vdecl;
  }

  SgClassDeclaration* buildOcrEdtDepElemStruct(OcrEdtContextPtr edtContext, SgFunctionDeclaration* decl) {
    // Build a struct for all the parameters
    string depElemStructName = edtContext->get_name() + "DepElems";
    SgClassDeclaration* depElemStructDecl = SageBuilder::buildStructDeclaration(depElemStructName, decl->get_scope());
    list<SgVarRefExp*> depElems = edtContext->getDepElems();
    list<SgVarRefExp*>::iterator v = depElems.begin();
    SgClassDefinition* sdefn = depElemStructDecl->get_definition();
    assert(sdefn);
    for( ; v != depElems.end(); ++v) {
      SgVariableDeclaration* vdecl = buildOcrEdtDepElemVarDecl(*v, sdefn);
      sdefn->append_member(vdecl);
    }
    return depElemStructDecl;
  }

  SgVariableDeclaration* buildOcrEdtDepElemStructDecl(SgType* type, SgName name, SgScopeStatement* scope) {
    assert(scope);
    SgPointerType* ptype = SageBuilder::buildPointerType(type);
    SgVarRefExp* paramv = SageBuilder::buildVarRefExp("paramv", scope);
    SgCastExp* castExp = SageBuilder::buildCastExp(paramv, ptype);
    SgAssignInitializer* initializer = SageBuilder::buildAssignInitializer(castExp, ptype);
    SgVariableDeclaration* depElemStructDecl = SageBuilder::buildVariableDeclaration(name, ptype, initializer, scope);
    return depElemStructDecl;
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

  vector<SgStatement*> buildOcrEdtStmts(OcrEdtContextPtr edtContext) {
    vector<SgStatement*> edt_stmts;
    // Outline the Task's statements to the EDT function
    list<SgStatement*> stmtList = edtContext->getStmtList();
    list<SgStatement*>::iterator s = stmtList.begin();
    for( ; s != stmtList.end(); ++s) {
      // Detach the statement from its original scope
      SageInterface::removeStatement(*s);
      // Add the statement into the current scope
      edt_stmts.push_back(*s);
    }
    // Add a return NULL_GUID statement to the EDT
    SgIntVal* zero = SageBuilder::buildIntVal(0);
    string returnExp = "NULL_GUID";
    SageInterface::addTextForUnparser(zero, returnExp, AstUnparseAttribute::e_replace);
    SgStatement* returnStmt = SageBuilder::buildReturnStmt(zero);
    edt_stmts.push_back(returnStmt);
    return edt_stmts;
  }

  /****************************************
   * Builders for Setting up EDT Template *
   ****************************************/
  SgVariableDeclaration* buildOcrGuidEdtTemplateVarDecl(string name, SgScopeStatement* scope) {
    assert(scope);
    SgType* ocrGuidType = buildOcrGuidType(scope);
    SgVariableDeclaration* decl = SageBuilder::buildVariableDeclaration(name, ocrGuidType, NULL, scope);
    return decl;
  }

  SgExprStatement* buildOcrEdtTemplateCallExp(SgVariableDeclaration* edtTemplateGuid, SgFunctionDeclaration* edtFuncDecl,
					      unsigned int ndelems, unsigned int ndbks, SgScopeStatement* scope) {
    SgType* voidType = SageBuilder::buildVoidType();
    // Arguments for ocrEdtTemplateCreate
    vector<SgExpression*> args;
    SgVarRefExp* ocrGuidVarRefExp = SageBuilder::buildVarRefExp(edtTemplateGuid);
    // Build the first argument
    SgExpression* first = SageBuilder::buildAddressOfOp(ocrGuidVarRefExp);
    args.push_back(first);
    // Build the second argument
    SgExpression* second = SageBuilder::buildFunctionRefExp(edtFuncDecl);
    args.push_back(second);
    SgUnsignedIntVal* third = SageBuilder::buildUnsignedIntVal(ndelems);
    args.push_back(third);
    SgUnsignedIntVal* fourth = SageBuilder::buildUnsignedIntVal(ndbks);
    args.push_back(fourth);
    // Build the argument list
    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    // Build the statement
    SgExprStatement* stmt = SageBuilder::buildFunctionCallStmt("ocrEdtTemplateCreate", voidType, exprList, scope);
    return stmt;
  }

  SgExprStatement* buildEdtDepElemSetupStmt(SgVariableDeclaration* depElemStructVar, SgVarRefExp* depElemVarRef) {
    SgVarRefExp* alhs = SageBuilder::buildVarRefExp(depElemStructVar);
    SgVarRefExp* arhs = SageBuilder::buildVarRefExp(depElemVarRef->get_symbol());
    SgDotExp* dotExp = SageBuilder::buildDotExp(alhs, arhs);
    SgVarRefExp* rhs = SageBuilder::buildVarRefExp(depElemVarRef->get_symbol());
    SgExprStatement* assign = SageBuilder::buildAssignStatement(dotExp, rhs);
    return assign;
  }

  vector<SgStatement*> buildEdtDepElemSetupStmts(SgVariableDeclaration* depElemStructVar, OcrEdtContextPtr edtContext) {
    vector<SgStatement*> depElemSetupStmts;
    list<SgVarRefExp*> depElems = edtContext->getDepElems();
    list<SgVarRefExp*>::iterator l = depElems.begin();
    for( ; l != depElems.end(); ++l) {
      SgExprStatement* assignStmt = buildEdtDepElemSetupStmt(depElemStructVar, *l);
      depElemSetupStmts.push_back(assignStmt);
    }
    return depElemSetupStmts;
  }

  // u8 ocrEdtCreate( ocrGuid_t ∗ guid, ocrGuid_t templateGuid,
  // 		   u32 paramc, u64 ∗ paramv,
  // 		   u32 depc, ocrGuid_t ∗ depv,
  // 		   u16 flags, ocrHint_t ∗ hint, ocrGuid_t ∗ outputEvent )
  SgExprStatement* buildOcrEdtCreateCallExp(SgVariableSymbol* edtGuidSymbol, SgVariableSymbol* edtTemplateGuidSymbol,
					    SgVariableSymbol* depElemStructSymbol,
					    SgVariableSymbol* outEvtGuidSymbol, SgScopeStatement* scope) {
    SgType* voidType = SageBuilder::buildVoidType();
    // Arguments for ocrEdtTemplateCreate
    vector<SgExpression*> args;
    SgVarRefExp* ocrGuidVarRefExp = SageBuilder::buildVarRefExp(edtGuidSymbol);
    // Build the first argument
    SgExpression* first = SageBuilder::buildAddressOfOp(ocrGuidVarRefExp);
    args.push_back(first);
    // Second argument is edt template guid
    SgExpression* second = SageBuilder::buildVarRefExp(edtTemplateGuidSymbol);
    args.push_back(second);
    // this is paramc argument
    // We will generate EDT_PARAM_DEF instead
    // First we need a dummy place holder expression
    SgIntVal* third = SageBuilder::buildIntVal(1);
    string paramc = "EDT_PARAM_DEF";
    SageInterface::addTextForUnparser(third, paramc, AstUnparseAttribute::e_replace);
    args.push_back(third);
    // Fourth argument is address of depElem struct
    SgVarRefExp* depElemVarRef = SageBuilder::buildVarRefExp(depElemStructSymbol);
    SgType* u64_t = AstBuilder::buildu64PtrType(scope);
    SgExpression* addressOfExp = SageBuilder::buildAddressOfOp(depElemVarRef);
    SgExpression* fourth = SageBuilder::buildCastExp(addressOfExp, u64_t);
    args.push_back(fourth);
    // fifth which is depv
    SgIntVal* fifth = SageBuilder::buildIntVal(0);
    string depc = "EDT_PARAM_DEF";
    SageInterface::addTextForUnparser(fifth, depc, AstUnparseAttribute::e_replace);
    args.push_back(fifth);
    // sixth is paramv
    // we will setup this one as NULL
    // datablock dependences will be explicitly added later
    SgIntVal* sixth = SageBuilder::buildIntVal(0);
    string depv = "NULL";
    SageInterface::addTextForUnparser(sixth, depv, AstUnparseAttribute::e_replace);
    args.push_back(sixth);
    // seventh argument is flags
    SgIntVal* seventh = SageBuilder::buildIntVal(0);
    string flags = "EDT_PROP_NONE";
    SageInterface::addTextForUnparser(seventh, flags, AstUnparseAttribute::e_replace);
    args.push_back(seventh);
    // eighth argument is hints
    SgIntVal* eighth = SageBuilder::buildIntVal(0);
    string hints = "NULL_HINT";
    SageInterface::addTextForUnparser(eighth, hints, AstUnparseAttribute::e_replace);
    args.push_back(eighth);
    // ninth argument is output event
    SgVarRefExp* outputEvtVarRef = SageBuilder::buildVarRefExp(outEvtGuidSymbol);
    SgExpression* ninth = SageBuilder::buildAddressOfOp(outputEvtVarRef);
    args.push_back(ninth);
    // Build the argument list
    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    // Build the function call exp
    SgExprStatement* stmt = SageBuilder::buildFunctionCallStmt("ocrEdtCreate", voidType, exprList, scope);
    return stmt;
  }

  // u8 ocrAddDependence(ocrGuid_t source, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode)
  SgExprStatement* buildOcrAddDependenceCallExp(SgVariableSymbol* dbkGuidSymbol, SgVariableSymbol* edtGuidSymbol, int slot, DbkMode dbkmode, SgScopeStatement*  scope) {
    SgType* voidType = SageBuilder::buildVoidType();
    // Arguments for ocrEdtTemplateCreate
    vector<SgExpression*> args;
    SgVarRefExp* source = SageBuilder::buildVarRefExp(dbkGuidSymbol);
    args.push_back(source);
    SgVarRefExp* destination = SageBuilder::buildVarRefExp(edtGuidSymbol);
    args.push_back(destination);
    SgIntVal* slotn_ = SageBuilder::buildIntVal(slot);
    args.push_back(slotn_);
    SgIntVal* modeExp = SageBuilder::buildIntVal(0);
    string modeText;
    switch(dbkmode.getDbkMode()) {
    case DbkMode::DB_DEFAULT_MODE:
      modeText = "DB_DEFAULT_MODE";
      break;
    case DbkMode::DB_MODE_NULL:
      modeText = "DB_MODE_NULL";
      break;
    default:
      assert(false);
    }
    SageInterface::addTextForUnparser(modeExp, modeText, AstUnparseAttribute::e_replace);
    args.push_back(modeExp);
    // Build the argument list
    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    // Build the function call exp
    SgExprStatement* stmt = SageBuilder::buildFunctionCallStmt("ocrAddDependence", voidType, exprList, scope);
    return stmt;
  }

  // u8 ocrEventCreate(ocrGuid_t ∗guid, ocrEventTypes_t eventType, u16 flags)
  // We will assument OCR_EVENT_ONCE_T as a default for now
  // flags is EVT_PROP_NONE which is the default behavior
  SgExprStatement* buildEvtCreateCallExp(SgVariableSymbol* evtGuidSymbol, SgScopeStatement* scope) {
    SgType* voidType = SageBuilder::buildVoidType();
    // Arguments for ocrEventCreate
    vector<SgExpression*> args;
    SgVarRefExp* evtGuidVarRefExp = SageBuilder::buildVarRefExp(evtGuidSymbol);
    SgExpression* addressOfExp = SageBuilder::buildAddressOfOp(evtGuidVarRefExp);
    args.push_back(addressOfExp);
    // placeholder expression
    SgIntVal* eventType = SageBuilder::buildIntVal();
    string eventTypeStr = "OCR_EVENT_ONCE_T";
    SageInterface::addTextForUnparser(eventType, eventTypeStr, AstUnparseAttribute::e_replace);
    args.push_back(eventType);
    SgIntVal* flags = SageBuilder::buildIntVal();
    string flagsStr = "EVT_PROP_NONE";
    SageInterface::addTextForUnparser(flags, flagsStr, AstUnparseAttribute::e_replace);
    args.push_back(flags);
    // Build the argument list
    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    // Build the function call exp
    SgExprStatement* stmt = SageBuilder::buildFunctionCallStmt("ocrEventCreate", voidType, exprList, scope);
    return stmt;
  }


  // void varRefExp2ArrowExpInStmt(SgVarRefExp* oexp, SgArrowExp* nexp, SgStatement* stmt) {
  //   RoseAst ast(stmt);
  //   RoseAst::iterator it = ast.begin();
  //   for( ; it != ast.end(); ++it) {
  //     if(SgVarRefExp* vref = isSgVarRefExp(*it)) {
  // 	if(vref->get_symbol() == oexp->get_symbol()) {
  // 	  SageInterface::replaceExpression(vref, nexp, false);
  // 	}
  //     }
  //   }
  // }

  // void varRefExp2ArrowExp(SgVarRefExp* oexp, SgArrowExp* nexp, SgStatementPtrList& statements) {
  //   SgStatementPtrList::iterator s = statements.begin();
  //   for( ; s != statements.end(); ++s) {
  //     varRefExp2ArrowExpInStmt(oexp, nexp, *s);
  //   }
  // }

  // void replaceDepElemVars(SgSymbol* depElemSymbol, SgScopeStatement* scope,
  // 			  SgStatementPtrList& statements, OcrEdtContextPtr edtContext) {
  //   list<SgVarRefExp*> depElemList = edtContext->getDepElems();
  //   list<SgVarRefExp*>::iterator e = depElemList.begin();
  //   for( ; e != depElemList.end(); ++e) {
  //     SgVariableSymbol* dsymbol = (*e)->get_symbol();
  //     SgName dname = dsymbol->get_name();
  //     SgVarRefExp* rexp = SageBuilder::buildVarRefExp(dname, scope);
  //     SgVarRefExp* lexp = SageBuilder::buildVarRefExp(depElemSymbol->get_name(), scope);
  //     SgArrowExp* arrowExp = SageBuilder::buildArrowExp(lexp, rexp);
  //     // rexp is rhs of arrow exp (e.g. depElem->var)
  //     // When depElem is declared in basicblock, a symbol
  //     // for depElem is inserted in to the basic block
  //     // When building the arrow exp, we build the rhs using the name
  //     // The variable may not have a symbol in the basic block
  //     // This later shows up as a Warning in AST consistency checks
  //     // To avoid this, the symbol for var is inserted into the symbol table.
  //     // This seems to fix the AST consistency check
  //     // However, I need to check if this is the way for building dot and arrow
  //     // expressions
  //     scope->get_symbol_table()->insert(dname, rexp->get_symbol());
  //     varRefExp2ArrowExp(*e, arrowExp, statements);
  //   }
  // }

  // SgFunctionDeclaration* buildOcrEdt(string name, OcrEdtContextPtr edtContext) {
  //   Logger::Logger lg("AstBuilder::buildOcrEdt");
  //   SgSourceFile* sourcefile = edtContext->getSourceFile();
  //   // scope where the edt function will be created
  //   SgGlobal* global = sourcefile->get_globalScope();
  //   // position in the AST where the EDT function will be inserted
  //   SgStatement* first = SageInterface::getFirstStatement(global);
  //   // Function created is a defining declaration which means
  //   // the declaration has an empty body
  //   SgFunctionDeclaration* edtdecl = buildOcrEdtFuncDecl(name, global);
  //   SgClassDeclaration* depElemStruct = buildOcrEdtDepElems(edtContext, edtdecl);
  //   SgTypedefDeclaration* depElemTypedefType = buildTypeDefDecl(name, depElemStruct->get_type(), global);
  //   // Set up the parameters for the EDT
  //   // Get the basic block for the EDT function for inserting parameters, statements etc.
  //   SgBasicBlock* basicblock = edtdecl->get_definition()->get_body();
  //   buildOcrEdtSignature(edtdecl->get_parameterList(), basicblock);
  //   SgSymbol* depElemSymbol = insertOcrEdtDepElemDecl(depElemTypedefType->get_type(), basicblock);
  //   SageInterface::insertStatement(first, edtdecl, true, true);
  //   SageInterface::insertStatementBefore(edtdecl, depElemStruct, true);
  //   SageInterface::insertStatementAfter(depElemStruct, depElemTypedefType, true);
  //   vector<SgStatement*> depDbksDecl = buildOcrDbksDecl(edtContext, basicblock, edtdecl);
  //   SageInterface::insertStatementAfterLastDeclaration(depDbksDecl, basicblock);
  //   buildOcrEdtStmts(edtContext, basicblock);
  //   replaceDepElemVars(depElemSymbol, basicblock, basicblock->get_statements(), edtContext);
  //   Logger::debug(lg) << "edtdecl:" << AstDebug::astToString(edtdecl) << endl;
  //   return edtdecl;
  // }

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
}

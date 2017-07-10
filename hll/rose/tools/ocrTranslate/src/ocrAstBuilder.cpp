/*
 * Author: Sriram Aananthakrishnan, 2017 *
 */

#include "sage3basic.h"
#include "ocrObjectInfo.h"
#include "ocrAstInfo.h"
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

  SgType* buildOcrGuidArrType(SgScopeStatement* scope, SgExpression* dimExpr) {
    assert(scope);
    SgType* guidType = buildOcrGuidType(scope);
    SgType* guidArrType = SageBuilder::buildArrayType(guidType, dimExpr);
    return guidArrType;
  }

  SgTypedefDeclaration* buildTypeDefDecl(string edtName, SgType* baseType, SgScopeStatement* scope) {
    string typedefName = edtName + "DepElem_t";
    return SageBuilder::buildTypedefDeclaration(typedefName, baseType, scope, true);
  }

  SgType* buildVoidPtrType() {
    SgType* vtype = SageBuilder::buildVoidType();
    return SageBuilder::buildPointerType(vtype);
  }

  SgType* buildArgvType() {
    SgTypeChar* ctype = SageBuilder::buildCharType();
    SgPointerType* ptype = SageBuilder::buildPointerType(ctype);
    SgArrayType* arrType = SageBuilder::buildArrayType(ptype);
    return arrType;
  }

  SgVariableDeclaration* buildGuidVarDecl(string name, SgScopeStatement* scope) {
    SgType* guidType = buildOcrGuidType(scope);
    SgVariableDeclaration* guidDecl = SageBuilder::buildVariableDeclaration(name, guidType, NULL, scope);
    return guidDecl;
  }

  /**************************
   * EDT Outlining Builders *
   **************************/
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

  SgClassDeclaration* buildOcrEdtDepElemStruct(string edtName, list<SgVarRefExp*>& depElems, SgScopeStatement* scope) {
    // Build a struct for all the parameters
    string depElemStructName = edtName + "DepElems";
    SgClassDeclaration* depElemStructDecl = SageBuilder::buildStructDeclaration(depElemStructName, scope);
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

  // Builds the datablock pointer from depv and slot number
  SgVariableDeclaration* buildDepDbkPtrDecl(string name, SgType* dbkPtrType, unsigned int slot,
					 SgInitializedName* depv,SgScopeStatement* scope) {
    SgVariableSymbol* vsymbol = isSgVariableSymbol(depv->get_symbol_from_symbol_table());
    assert(vsymbol);
    SgVarRefExp* depvVarRefExp = SageBuilder::buildVarRefExp(vsymbol);
    SgIntVal* sgIndex = SageBuilder::buildIntVal(slot);
    SgPntrArrRefExp* arrRefExp = SageBuilder::buildPntrArrRefExp(depvVarRefExp, sgIndex);
    SgName ptrMemName("ptr");
    SgVarRefExp* ptrVarRefExp = SageBuilder::buildVarRefExp(ptrMemName, scope);
    SgVariableSymbol* ptrVarSymbol = ptrVarRefExp->get_symbol();
    SgDotExp* dotExp = SageBuilder::buildDotExp(arrRefExp, ptrVarRefExp);
    SgCastExp* castExp = SageBuilder::buildCastExp(dotExp, dbkPtrType);
    SgAssignInitializer* initializer = SageBuilder::buildAssignInitializer(castExp, dbkPtrType);
    SgVariableDeclaration* vdecl = SageBuilder::buildVariableDeclaration(name, dbkPtrType, initializer, scope);

    // Fix for avoiding ROSE Warnings
    // Forcefully inserting the symbol into the symbol table
    if(!scope->symbol_exists(ptrVarRefExp->get_symbol())) {
      scope->insert_symbol(ptrMemName, ptrVarRefExp->get_symbol());
    }
    // return the declaration
    return vdecl;
  }

  // Build the datablock guid declaration from depv and slot number
  SgVariableDeclaration* buildDepDbkGuidDecl(string guidName, unsigned int slot, SgInitializedName* depv, SgScopeStatement* scope) {
    SgVariableSymbol* vsymbol = isSgVariableSymbol(depv->get_symbol_from_symbol_table());
    assert(vsymbol);
    SgType* ocrGuidType = buildOcrGuidType(scope);
    SgVarRefExp* depvVarRefExp = SageBuilder::buildVarRefExp(vsymbol);
    SgIntVal* slotN = SageBuilder::buildIntVal(slot);
    SgPntrArrRefExp* arrRefExp = SageBuilder::buildPntrArrRefExp(depvVarRefExp, slotN);
    SgName guidMemName("guid");
    SgVarRefExp* guidVarRefExp = SageBuilder::buildVarRefExp("guid", scope);
    SgDotExp* dotExp = SageBuilder::buildDotExp(arrRefExp, guidVarRefExp);
    SgAssignInitializer* initializer = SageBuilder::buildAssignInitializer(dotExp, ocrGuidType);
    SgVariableDeclaration* vdecl = SageBuilder::buildVariableDeclaration(guidName, ocrGuidType, initializer, scope);

    // Fix for avoiding ROSE Warnings
    // Forcefully inserting the symbol into the symbol table
    if(!scope->symbol_exists(guidVarRefExp->get_symbol())) {
      scope->insert_symbol(guidMemName, guidVarRefExp->get_symbol());
    }
    // return the variable declaration
    return vdecl;
  }

  // Build an individual declaration for a depElem inside the EDT
  SgVariableDeclaration* buildEdtDepElemVarDecl(SgVariableSymbol* depElemStructSymbol, SgVariableSymbol* memberVarSymbol, SgScopeStatement* scope) {
    SgType* varType = memberVarSymbol->get_type();
    SgVarRefExp* memberVarRefExp = SageBuilder::buildVarRefExp(memberVarSymbol);
    SgVarRefExp* depElemStructVarRefExp = SageBuilder::buildVarRefExp(depElemStructSymbol);
    SgArrowExp* memberRefExparrowExp = SageBuilder::buildArrowExp(depElemStructVarRefExp, memberVarRefExp);
    SgAssignInitializer* assignInitializer = SageBuilder::buildAssignInitializer(memberRefExparrowExp, varType);
    SgVariableDeclaration* depElemVarDecl = SageBuilder::buildVariableDeclaration(memberVarSymbol->get_name(), varType, assignInitializer, scope);
    return depElemVarDecl;
  }

  vector<SgStatement*> buildEdtDepElemVarsDecl(SgClassDeclaration* depElemStructDecl, SgVariableSymbol* depElemStructSymbol, SgScopeStatement* scope) {
    vector<SgStatement*> depElemVarsDeclStmts;
    vector<SgInitializedName*> structMembers = getDepElemStructMembers(depElemStructDecl);
    vector<SgInitializedName*>::iterator in = structMembers.begin();
    for( ; in != structMembers.end(); ++in) {
      SgVariableSymbol* vsymbol = GetVariableSymbol(*in);
      SgType* vtype = vsymbol->get_type();
      assert(vtype);
      SgVarRefExp* depElemStructVarRefExp = SageBuilder::buildVarRefExp(depElemStructSymbol);
      SgVarRefExp* member = SageBuilder::buildVarRefExp(vsymbol);
      SgArrowExp* memberRefExp = SageBuilder::buildArrowExp(depElemStructVarRefExp, member);
      SgAssignInitializer* assignInitializer = SageBuilder::buildAssignInitializer(memberRefExp, vtype);
      SgVariableDeclaration* depElemVarDecl = SageBuilder::buildVariableDeclaration(vsymbol->get_name(), vtype, assignInitializer, scope);
      depElemVarsDeclStmts.push_back(depElemVarDecl);
    }
    return depElemVarsDeclStmts;
  }

  // Easiest thing to do is to call SageInteface::moveStatementsBetweenBlocks
  // It however fails when trying to move a pragma declaration
  // First, each statement must be added to the target basic block
  // If there is any statement which is a variable declaration,
  // adjust the scope accordingly
  // For reference - See SageInterface.C:18011-18100
  // TODO: Add the fix in SageInterfac::moveStatementsBetweenBlocks
  void buildEdtStmts(SgBasicBlock* sourceBlock, SgBasicBlock* targetBlock) {
    vector<SgStatement*> edt_stmts = sourceBlock->get_statements();
    vector<SgStatement*>::iterator st = edt_stmts.begin();
    for( ; st != edt_stmts.end(); ++st) {
      SageInterface::removeStatement(*st, true);
      SageInterface::appendStatement(*st, targetBlock);
      // Set the scope appropriately for declaration statements
      if(SgDeclarationStatement* declStmt = isSgDeclarationStatement(*st)) {
	switch(declStmt->variantT()) {
	case V_SgVariableDeclaration: {
	  SgVariableDeclaration* varDecl = isSgVariableDeclaration(declStmt);
	  SgInitializedNamePtrList & l = varDecl->get_variables();
	  for (SgInitializedNamePtrList::iterator i = l.begin(); i != l.end(); i++) {
	    // reset the scope, but make sure it was set targetBlock sourceBlock targetBlock make sure.
	    // This might be an issue for extern variable declaration that have a scope
	    // in a separate namespace of a static class member defined external targetBlock
	    // its class, etc. I don't want targetBlock worry about those cases right now.
	    assert((*i)->get_scope() == sourceBlock);
	    (*i)->set_scope(targetBlock);
	  }
	  break;
	}
	case V_SgPragmaDeclaration: {
	  assert(declStmt->get_scope() == targetBlock);
	  break;
	}
	case V_SgTypedefDeclaration:
	case V_SgFunctionDeclaration: {
	  break;
	}
	default:
	  cerr << "Moving " << declStmt->class_name() << " not supported in AstBuilder::buildEdtStmts\n";
	  assert(false);
	}
      }
    }

    // Move the symbol table
    assert(sourceBlock->get_symbol_table() != NULL);
    targetBlock->set_symbol_table(sourceBlock->get_symbol_table());

    assert(sourceBlock != NULL);
    assert(targetBlock != NULL);
    assert(targetBlock->get_symbol_table() != NULL);
    assert(sourceBlock->get_symbol_table() != NULL);
    targetBlock->get_symbol_table()->set_parent(targetBlock);

    assert(sourceBlock->get_symbol_table() != NULL);
    sourceBlock->set_symbol_table(NULL);

    // DQ (9/23/2011): Reset with a valid symbol table.
    sourceBlock->set_symbol_table(new SgSymbolTable());
    sourceBlock->get_symbol_table()->set_parent(sourceBlock);
  }

  SgStatement* buildOcrDbDestroyCallExp(unsigned int slot, SgVariableSymbol* depvSymbol, SgScopeStatement* scope) {
    SgIntVal* slotExp = SageBuilder::buildIntVal(slot);
    SgVarRefExp* depvVarRefExp = SageBuilder::buildVarRefExp(depvSymbol);
    SgPntrArrRefExp* depvArrRefExp = SageBuilder::buildPntrArrRefExp(depvVarRefExp, slotExp);
    SgVarRefExp* guidVarRefExp = SageBuilder::buildVarRefExp("guid", scope);
    SgDotExp* argument = SageBuilder::buildDotExp(depvArrRefExp, guidVarRefExp);
    // Fix for ROSE Warnings
    SgVariableSymbol* guidSymbol = guidVarRefExp->get_symbol();
    assert(guidSymbol);
    if(!scope->symbol_exists(guidSymbol->get_name())) {
      scope->insert_symbol(guidSymbol->get_name(), guidSymbol);
    }
    vector<SgExpression*> args;
    args.push_back(argument);
    // Build the argument list
    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    // Build the statement
    SgExprStatement* stmt = SageBuilder::buildFunctionCallStmt("ocrDbDestroy", SageBuilder::buildVoidType(), exprList, scope);
    return stmt;
  }

  SgStatement* buildEvtDestroyCallExp(unsigned int slot, SgVariableSymbol* depvSymbol, SgScopeStatement* scope) {
    SgIntVal* slotExp = SageBuilder::buildIntVal(slot);
    SgVarRefExp* depvVarRefExp = SageBuilder::buildVarRefExp(depvSymbol);
    SgPntrArrRefExp* depvArrRefExp = SageBuilder::buildPntrArrRefExp(depvVarRefExp, slotExp);
    SgVarRefExp* guidVarRefExp = SageBuilder::buildVarRefExp("guid", scope);
    SgDotExp* argument = SageBuilder::buildDotExp(depvArrRefExp, guidVarRefExp);
    vector<SgExpression*> args;
    args.push_back(argument);
    // Build the argument list
    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    // Build the statement
    SgExprStatement* stmt = SageBuilder::buildFunctionCallStmt("ocrEventDestroy", SageBuilder::buildVoidType(), exprList, scope);
    return stmt;
  }

  /**********************
   * Loop EDT Outlining *
   **********************/
  SgBasicBlock* buildLoopControlIfBody(SgForStatement* forStmt) {
    // Get the loop increment expression
    // NOTE: This will fail if the for loop's increment expression is inside the body
    SgExpression* incrementExpr = forStmt->get_increment();
    assert(incrementExpr);

    SgBasicBlock* ifBasicBlock = SageBuilder::buildBasicBlock();
    SgExprStatement* incrementExprStmt = SageBuilder::buildExprStatement(incrementExpr);
    SageInterface::appendStatement(incrementExprStmt, ifBasicBlock);
    return ifBasicBlock;
  }

  SgBasicBlock* buildLoopControlElseBody() {
    SgBasicBlock* elseBasicBlock = SageBuilder::buildBasicBlock();
    return elseBasicBlock;
  }

  SgIfStmt* buildLoopControlIfStmt(SgBasicBlock* ifBasicBlock, SgBasicBlock* elseBasicBlock, SgForStatement* forStmt) {
    SgStatement* loopConditionStmt = SageInterface::getLoopCondition(forStmt);
    assert(loopConditionStmt);
    SgIfStmt* loopControlIfStmt = SageBuilder::buildIfStmt(loopConditionStmt, ifBasicBlock, elseBasicBlock);
    return loopControlIfStmt;
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
					      SgExpression* nparamc, unsigned int ndeps, SgScopeStatement* scope) {
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

    SgExpression* third = nparamc;
    args.push_back(third);

    SgUnsignedIntVal* fourth = SageBuilder::buildUnsignedIntVal(ndeps);
    args.push_back(fourth);
    // Build the argument list
    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    // Build the statement
    SgExprStatement* stmt = SageBuilder::buildFunctionCallStmt("ocrEdtTemplateCreate", voidType, exprList, scope);
    return stmt;
  }

  vector<SgInitializedName*> getDepElemStructMembers(SgClassDeclaration* depElemStructDecl) {
    vector<SgInitializedName*> members;
    SgClassDefinition* depElemStructDefn = depElemStructDecl->get_definition();
    assert(depElemStructDefn);
    vector<SgDeclarationStatement*> depElemStructMemDecl = depElemStructDefn->get_members();
    vector<SgDeclarationStatement*>::iterator decl = depElemStructMemDecl.begin();
    for( ; decl != depElemStructMemDecl.end(); ++decl) {
      SgVariableDeclaration* vdecl = isSgVariableDeclaration(*decl);
      if(vdecl) {
	vector<SgInitializedName*> vars = vdecl->get_variables();
	members.insert(members.end(), vars.begin(), vars.end());
      }
    }
    return members;
  }

  SgInitializedName* getMatchingDepElemMemberVar(vector<SgInitializedName*>& depElemMemberVars, string varName) {
    vector<SgInitializedName*>::iterator d = depElemMemberVars.begin();
    for( ; d != depElemMemberVars.end(); ++d) {
      string memberName = (*d)->get_name().getString();
      if(memberName.compare(varName) == 0) {
	return *d;
      }
    }
    assert(false);
  }

  SgExprStatement* buildEdtDepElemSetupStmt(SgVariableSymbol* depElemStructVarSymbol, SgVariableSymbol* depElemMemberVarSymbol, SgVariableSymbol* depElemVarSymbol) {
    SgVarRefExp* alhs = SageBuilder::buildVarRefExp(depElemStructVarSymbol);
    SgVarRefExp* arhs = SageBuilder::buildVarRefExp(depElemMemberVarSymbol);
    SgDotExp* dotExp = SageBuilder::buildDotExp(alhs, arhs);
    SgVarRefExp* rhs = SageBuilder::buildVarRefExp(depElemVarSymbol);
    SgExprStatement* assign = SageBuilder::buildAssignStatement(dotExp, rhs);
    return assign;
  }


  SgExprStatement* buildEdtDepElemSetupStmt(SgVariableDeclaration* depElemStructVar, SgInitializedName* memberVar,
					    SgVarRefExp* depElemVarRef) {
    SgVarRefExp* alhs = SageBuilder::buildVarRefExp(depElemStructVar);
    SgVariableSymbol* memberVarSymbol = GetVariableSymbol(memberVar);
    SgVarRefExp* arhs = SageBuilder::buildVarRefExp(memberVarSymbol);
    SgDotExp* dotExp = SageBuilder::buildDotExp(alhs, arhs);
    SgVarRefExp* rhs = SageBuilder::buildVarRefExp(depElemVarRef->get_symbol());
    SgExprStatement* assign = SageBuilder::buildAssignStatement(dotExp, rhs);
    return assign;
  }

  vector<SgStatement*> buildEdtDepElemSetupStmts(SgVariableDeclaration* depElemStructVar, SgClassDeclaration* depElemStructDecl,
						 list<SgVarRefExp*> depElemVarList) {
    Logger::Logger lg("buildEdtDepElemSetupStmts");
    vector<SgStatement*> depElemSetupStmts;
    list<SgVarRefExp*>::iterator v = depElemVarList.begin();
    vector<SgInitializedName*> depElemMemberVars = getDepElemStructMembers(depElemStructDecl);
    for( ; v != depElemVarList.end(); ++v) {
      SgInitializedName* depElemMemberVar = getMatchingDepElemMemberVar(depElemMemberVars, (*v)->get_symbol()->get_name().getString());
      SgExprStatement* setupStmt = buildEdtDepElemSetupStmt(depElemStructVar, depElemMemberVar, *v);
      depElemSetupStmts.push_back(setupStmt);
    }
    return depElemSetupStmts;
  }

  SgExpression*  buildDepElemSizeOfExpr(SgType* depElemType, SgScopeStatement* scope) {
    SgType* u64Type = buildu64Type(scope);
    SgSizeOfOp* sizeofu64Num = SageBuilder::buildSizeOfOp(u64Type);
    SgSubtractOp* sizeofu64MinusOne = SageBuilder::buildSubtractOp(sizeofu64Num, SageBuilder::buildIntVal(1));
    SgSizeOfOp* sizeofDepElem = SageBuilder::buildSizeOfOp(depElemType);
    SgAddOp* sizeofDepElemPlus = SageBuilder::buildAddOp(sizeofDepElem, sizeofu64MinusOne);
    SgSizeOfOp* sizeofu64Den = SageBuilder::buildSizeOfOp(u64Type);
    SgDivideOp* depElemSizeExpr = SageBuilder::buildDivideOp(sizeofDepElemPlus, sizeofu64Den);
    return depElemSizeExpr;
  }

  SgVariableDeclaration* buildDepElemSizeVarDecl(string varName, SgType* depElemType, SgScopeStatement* scope) {
    SgType* u32Type = buildu32Type(scope);
    SgExpression* depElemSizeExpr = buildDepElemSizeOfExpr(depElemType, scope);
    SgInitializer* varInitializer = SageBuilder::buildAssignInitializer(depElemSizeExpr, u32Type);
    SgVariableDeclaration* depElemSizeVarDecl = SageBuilder::buildVariableDeclaration(varName, u32Type, varInitializer, scope);
    return depElemSizeVarDecl;
  }

  // vector<SgStatement*> buildEdtDepElemSetupStmts(SgVariableDeclaration* depElemStructVar, OcrEdtContextPtr edtContext) {
  //   vector<SgStatement*> depElemSetupStmts;
  //   list<SgVarRefExp*> depElems = edtContext->getDepElems();
  //   list<SgVarRefExp*>::iterator l = depElems.begin();
  //   for( ; l != depElems.end(); ++l) {
  //     SgExprStatement* assignStmt = buildEdtDepElemSetupStmt(depElemStructVar, *l);
  //     depElemSetupStmts.push_back(assignStmt);
  //   }
  //   return depElemSetupStmts;
  // }

  // u8 ocrEdtCreate( ocrGuid_t ∗ guid, ocrGuid_t templateGuid,
  // 		   u32 paramc, u64 ∗ paramv,
  // 		   u32 depc, ocrGuid_t ∗ depv,
  // 		   u16 flags, ocrHint_t ∗ hint, ocrGuid_t ∗ outputEvent )
  SgExprStatement* buildOcrEdtCreateCallExp(SgVariableSymbol* edtGuidSymbol, SgVariableSymbol* edtTemplateGuidSymbol,
					    SgVariableSymbol* paramcSymbol,
					    SgVariableSymbol* depElemStructSymbol,
					    int ndeps,
					    SgVariableSymbol* outEvtGuidSymbol,
					    bool finishEdt,
					    SgScopeStatement* scope) {
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
    SgExpression* third = NULL;
    if(paramcSymbol) {
      third = SageBuilder::buildVarRefExp(paramcSymbol);
    }
    else {
      third = SageBuilder::buildIntVal(0);
    }
    args.push_back(third);
    // Fourth argument is address of depElem struct
    SgExpression* fourth;
    // If the symbol is not NULL
    if(depElemStructSymbol) {
      SgVarRefExp* depElemVarRef = SageBuilder::buildVarRefExp(depElemStructSymbol);
      SgType* u64_t = AstBuilder::buildu64PtrType(scope);
      SgExpression* addressOfExp = SageBuilder::buildAddressOfOp(depElemVarRef);
      fourth = SageBuilder::buildCastExp(addressOfExp, u64_t);
    }
    // When there are no depElems
    else {
      fourth = SageBuilder::buildIntVal(0);
      SageInterface::addTextForUnparser(fourth, "NULL", AstUnparseAttribute::e_replace);
    }
    args.push_back(fourth);
    // fifth which is depc
    SgIntVal* fifth = SageBuilder::buildIntVal(ndeps);
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
    string flags;
    // If we have both finish and valid output event
    if(finishEdt && outEvtGuidSymbol) {
      flags = "EDT_PROP_FINISH | EDT_PROP_OEVT_VALID";
    }
    // If we have a valid output event symbol
    else if(outEvtGuidSymbol) {
      flags = "EDT_PROP_OEVT_VALID";
    }
    else if(finishEdt) {
      flags = "EDT_PROP_FINISH";
    }
    else {
      flags = "EDT_PROP_NONE";
    }
    SageInterface::addTextForUnparser(seventh, flags, AstUnparseAttribute::e_replace);
    args.push_back(seventh);
    // eighth argument is hints
    SgIntVal* eighth = SageBuilder::buildIntVal(0);
    string hints = "NULL_HINT";
    SageInterface::addTextForUnparser(eighth, hints, AstUnparseAttribute::e_replace);
    args.push_back(eighth);
    // ninth argument is output event
    SgExpression* ninth;
    // If it is not NULL create the address of expression
    if(outEvtGuidSymbol) {
      SgVarRefExp* outputEvtVarRef = SageBuilder::buildVarRefExp(outEvtGuidSymbol);
      ninth = SageBuilder::buildAddressOfOp(outputEvtVarRef);
    }
    // If it is NULL
    else {
      ninth = SageBuilder::buildIntVal(0);
      SageInterface::addTextForUnparser(ninth, "NULL", AstUnparseAttribute::e_replace);
    }
    args.push_back(ninth);
    // Build the argument list
    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    // Build the function call exp
    SgExprStatement* stmt = SageBuilder::buildFunctionCallStmt("ocrEdtCreate", voidType, exprList, scope);
    return stmt;
  }

  // u8 ocrAddDependence(ocrGuid_t source, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode)
  SgExprStatement* buildOcrAddDependenceCallExp(SgVariableSymbol* from, SgVariableSymbol* to, int slot, DbkMode dbkmode, SgScopeStatement*  scope) {
    SgType* voidType = SageBuilder::buildVoidType();
    // Arguments for ocrEdtTemplateCreate
    vector<SgExpression*> args;
    SgVarRefExp* source = SageBuilder::buildVarRefExp(from);
    args.push_back(source);
    SgVarRefExp* destination = SageBuilder::buildVarRefExp(to);
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
    // For now we will create idempotent events
    string eventTypeStr = "OCR_EVENT_STICKY_T";
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

  SgExprStatement* buildEvtSatisfyCallExp(SgVariableSymbol* evtGuidSymbol, SgScopeStatement* scope) {
    SgType* voidType = SageBuilder::buildVoidType();
    // Arguments for ocrEventCreate
    vector<SgExpression*> args;
    SgVarRefExp* evtGuidVarRefExp = SageBuilder::buildVarRefExp(evtGuidSymbol);
    args.push_back(evtGuidVarRefExp);
    // placeholder expression
    SgIntVal* nullGuid = SageBuilder::buildIntVal();
    // For now we will create idempotent events
    string nullGuidStr = "NULL_GUID";
    SageInterface::addTextForUnparser(nullGuid, nullGuidStr, AstUnparseAttribute::e_replace);
    args.push_back(nullGuid);
    // Build the argument list
    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    // Build the function call exp
    SgExprStatement* stmt = SageBuilder::buildFunctionCallStmt("ocrEventSatisfy", voidType, exprList, scope);
    return stmt;
  }

  // ocrShutdown();
  SgExprStatement* buildOcrShutdownCallExp(SgScopeStatement* scope) {
    SgType* voidType = SageBuilder::buildVoidType();
    // Arguments for ocrEdtTemplateCreate
    vector<SgExpression*> args;
    // Build the argument list
    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    // Build the function call exp
    SgExprStatement* stmt = SageBuilder::buildFunctionCallStmt("ocrShutdown", voidType, exprList, scope);
    return stmt;
  }

  SgExpression* buildGetArgcCallExp(SgVariableSymbol* mainEdtDbkSymbol, SgScopeStatement* scope) {
    // Arguments for getArg
    vector<SgExpression*> args;
    SgVarRefExp* mainDbkVarRef = SageBuilder::buildVarRefExp(mainEdtDbkSymbol);
    SgType* castType = buildVoidPtrType();
    SgCastExp* first = SageBuilder::buildCastExp(mainDbkVarRef, castType);
    args.push_back(first);
    SgType* getArgcRetType = buildu64Type(scope);
    // Build the argument list
    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    // Build the function call exp
    SgExpression* callExp = SageBuilder::buildFunctionCallExp("getArgc", getArgcRetType, exprList, scope);
    return callExp;
  }

  // getArgv(dbk, index)
  SgExpression* buildGetArgvCallExp(SgVariableSymbol* dbkSymbol, SgVariableSymbol* indexSymbol, SgScopeStatement* scope) {
    vector<SgExpression*> args;
    SgVarRefExp* first = SageBuilder::buildVarRefExp(dbkSymbol);
    args.push_back(first);
    SgVarRefExp* second = SageBuilder::buildVarRefExp(indexSymbol);
    args.push_back(second);

    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    // Build the function call exp
    return SageBuilder::buildFunctionCallExp("getArgv", SageBuilder::buildVoidType(), exprList, scope);
  }

  SgStatement* buildEdtReturnStmt() {
    SgIntVal* zero = SageBuilder::buildIntVal();
    SageInterface::addTextForUnparser(zero, "NULL_GUID", AstUnparseAttribute::e_replace);
    return SageBuilder::buildReturnStmt(zero);
  }

  SgVariableDeclaration* buildMainEdtArgvDecl(SgInitializedName* mainArgv, SgInitializedName* mainArgc, SgScopeStatement* scope) {
    SgArrayType* arrayType = isSgArrayType(mainArgv->get_type());
    assert(arrayType);
    arrayType->set_index(SageBuilder::buildVarRefExp(mainArgc, scope));
    SgName vname = mainArgv->get_name();
    SgVariableDeclaration* decl = SageBuilder::buildVariableDeclaration(vname, arrayType, NULL, scope);
    return decl;
  }

  SgVariableDeclaration* buildMainEdtArgcDecl(SgInitializedName* mainArgc, SgVariableSymbol* mainEdtDbkSymbol, SgScopeStatement* scope) {
    SgType* vtype = mainArgc->get_type();
    SgName vname = mainArgc->get_name();
    SgExpression* getArgcCallExp = buildGetArgcCallExp(mainEdtDbkSymbol, scope);
    SgAssignInitializer* initializer = SageBuilder::buildAssignInitializer(getArgcCallExp, vtype);
    SgVariableDeclaration* decl = SageBuilder::buildVariableDeclaration(vname, vtype, initializer, scope);
    return decl;
  }

  SgExprStatement* buildMainEdtInitCallExp(SgName mainEdtInitName, SgVariableSymbol* mainEdtArgcSymbol,
					   SgVariableSymbol* mainEdtArgvSymbol, SgVariableSymbol* mainEdtDbkSymbol, SgScopeStatement* scope) {
    vector<SgExpression*> args;
    SgExpression* first = SageBuilder::buildVarRefExp(mainEdtArgcSymbol);
    args.push_back(first);

    SgExpression* second = SageBuilder::buildVarRefExp(mainEdtArgvSymbol);
    args.push_back(second);

    SgExpression* third = SageBuilder::buildVarRefExp(mainEdtDbkSymbol);
    args.push_back(third);

    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    // Build the function call exp
    SgFunctionCallExp* callExp = SageBuilder::buildFunctionCallExp(mainEdtInitName, SageBuilder::buildVoidType(), exprList, scope);
    return SageBuilder::buildExprStatement(callExp);
  }

  SgFunctionParameterList* buildMainEdtInitFuncParams(SgInitializedName* mainArgc, SgInitializedName* mainArgv, SgVariableSymbol* mainEdtDbkSymbol) {
    SgFunctionParameterList* paramList = SageBuilder::buildFunctionParameterList();
    // Now setup the parameters
    SgInitializedName* argc  = SageBuilder::buildInitializedName(mainArgc->get_name(), mainArgc->get_type(), NULL);
    SageInterface::appendArg(paramList, argc);

    SgType* argvType = buildArgvType();
    SgInitializedName* argv = SageBuilder::buildInitializedName(mainArgv->get_name(), argvType, NULL);
    SageInterface::appendArg(paramList, argv);

    SgType* dbkType = buildVoidPtrType();
    SgInitializedName* dbk = SageBuilder::buildInitializedName(mainEdtDbkSymbol->get_name(), dbkType, NULL);
    SageInterface::appendArg(paramList, dbk);
    return paramList;
  }

  SgForStatement* buildMainEdtInitForStmt(SgVariableSymbol* indexSymbol, SgVariableSymbol* argcSymbol) {
    SgVarRefExp* testExprLhs = SageBuilder::buildVarRefExp(indexSymbol);
    SgVarRefExp* testExprRhs = SageBuilder::buildVarRefExp(argcSymbol);
    SgExpression* lessThanOp = SageBuilder::buildLessThanOp(testExprLhs, testExprRhs);
    SgExprStatement* testStmt = SageBuilder::buildExprStatement(lessThanOp);

    SgVarRefExp* incVarRhs = SageBuilder::buildVarRefExp(indexSymbol);
    SgIntVal* one = SageBuilder::buildIntVal(1);
    SgAddOp* addop = SageBuilder::buildAddOp(incVarRhs, one);
    SgVarRefExp* incVarLhs = SageBuilder::buildVarRefExp(indexSymbol);
    SgAssignOp* incrementExp = SageBuilder::buildAssignOp(incVarLhs, addop);

    SgBasicBlock* forBasicBlock = SageBuilder::buildBasicBlock();
    SgForStatement* forStmt = SageBuilder::buildForStatement(NULL, testStmt, incrementExp, forBasicBlock, NULL);
    return forStmt;
  }

  vector<SgStatement*> buildMainEdtInitForBody(SgVariableSymbol* indexSymbol,SgVariableSymbol* argvSymbol,
					       SgVariableSymbol* dbkSymbol, SgBasicBlock* basicblock) {
  vector<SgStatement*> forStmtBodyList;
  // We are building the statement argv[i] = getArgv(dbk, i);
  SgVarRefExp* lvar = SageBuilder::buildVarRefExp(argvSymbol);
  SgVarRefExp* indexVarRef = SageBuilder::buildVarRefExp(indexSymbol);
  SgPntrArrRefExp* lhsAssignOp = SageBuilder::buildPntrArrRefExp(lvar, indexVarRef);

  // NOTE: basicblock must be that of a function's and not for loop's
  SgExpression* rhsAssignOp = buildGetArgvCallExp(dbkSymbol, indexSymbol, basicblock);
  SgAssignOp* assignOp = SageBuilder::buildAssignOp(lhsAssignOp, rhsAssignOp);
  SgExprStatement* assignStmt = SageBuilder::buildExprStatement(assignOp);
  forStmtBodyList.push_back(assignStmt);
  return forStmtBodyList;
}

  // Function building the mainEdtInit declaration
  SgFunctionDeclaration* buildMainEdtInitFuncDecl(SgName fname, SgInitializedName* mainArgc, SgInitializedName* mainArgv,
						  SgVariableSymbol* mainEdtDbkSymbol, SgGlobal* global) {
    SgType* returnType = SageBuilder::buildVoidType();
    SgFunctionParameterList* paramList = buildMainEdtInitFuncParams(mainArgc, mainArgv, mainEdtDbkSymbol);
    SgFunctionDeclaration* mainEdtInitFuncDecl = SageBuilder::buildDefiningFunctionDeclaration(fname, returnType, paramList, global);


    SgBasicBlock* mainEdtInitBasicBlock = mainEdtInitFuncDecl->get_definition()->get_body();
    vector<SgStatement*> mainEdtInitBody;

    SgAssignInitializer* initializer = SageBuilder::buildAssignInitializer(SageBuilder::buildIntVal(0));
    SgName indexVarName("i");
    SgVariableDeclaration* indexDecl = SageBuilder::buildVariableDeclaration(indexVarName, SageBuilder::buildIntType(), initializer, mainEdtInitBasicBlock);
    mainEdtInitBody.push_back(indexDecl);

    // Get all the variable symbols for which we will be creating SgVarRefExp
    SgVariableSymbol* indexSymbol = GetVariableSymbol(indexDecl, indexVarName.getString());
    SgInitializedNamePtrList& mainEdtInitArgs = paramList->get_args();
    SgVariableSymbol* argcSymbol = GetVariableSymbol(mainEdtInitArgs[0]);
    SgVariableSymbol* argvSymbol = GetVariableSymbol(mainEdtInitArgs[1]);
    SgVariableSymbol* dbkSymbol = GetVariableSymbol(mainEdtInitArgs[2]);

    SgForStatement* mainEdtInitForStmt = buildMainEdtInitForStmt(indexSymbol, argcSymbol);
    SgBasicBlock* forBasicBlock = isSgBasicBlock(mainEdtInitForStmt->get_loop_body());
    // NOTE: We are passing mainEdtInitBasicBlock for getArgv call expression
    vector<SgStatement*> mainEdtForBody = buildMainEdtInitForBody(indexSymbol, argvSymbol, dbkSymbol, mainEdtInitBasicBlock);
    SageInterface::appendStatementList(mainEdtForBody, forBasicBlock);

    // Insert the for stmt into the body
    mainEdtInitBody.push_back(mainEdtInitForStmt);

    SageInterface::appendStatementList(mainEdtInitBody, mainEdtInitBasicBlock);
    return mainEdtInitFuncDecl;
  }

  /**********************
   * SpmdRegionEdtSetup *
   **********************/
  SgVariableDeclaration* buildGuidArrVarDecl(string guidArrVarName, int dim, SgScopeStatement* scope) {
    SgIntVal* dimExpr = SageBuilder::buildIntVal(dim);
    SgType* guidArrType = buildOcrGuidArrType(scope, dimExpr);
    SgVariableDeclaration* guidArrVarDecl = SageBuilder::buildVariableDeclaration(guidArrVarName, guidArrType, NULL, scope);
    return guidArrVarDecl;
  }

  SgStatement* buildGuidArrCopyStmt(SgVariableSymbol* guidSymbol, SgVariableSymbol* guidArrSymbol, int index) {
    SgVarRefExp* guidVarRefExp = SageBuilder::buildVarRefExp(guidSymbol);
    SgIntVal* indexExp = SageBuilder::buildIntVal(index);
    SgVarRefExp* arrVarRefExp = SageBuilder::buildVarRefExp(guidArrSymbol);
    SgPntrArrRefExp* arrRefExp = SageBuilder::buildPntrArrRefExp(arrVarRefExp, indexExp);
    SgExprStatement* assignStmt = SageBuilder::buildAssignStatement(arrRefExp, guidVarRefExp);
    return assignStmt;
  }

  SgVariableDeclaration* buildDbAccessModeArrVarDecl(string dbAccessModeArrName, int dim, SgScopeStatement* scope) {
    SgIntVal* dimExpr = SageBuilder::buildIntVal(dim);
    SgType* dbAccessModeType = SageBuilder::buildOpaqueType("ocrDbAccessMode_t", scope);
    SgType* dbArrAccessModeArrType = SageBuilder::buildArrayType(dbAccessModeType, dimExpr);
    SgVariableDeclaration* dbAccessModeArrVarDecl = SageBuilder::buildVariableDeclaration(dbAccessModeArrName, dbArrAccessModeArrType, NULL, scope);
    return dbAccessModeArrVarDecl;
  }

  SgStatement* buildDbAccessModeAssignStmt(SgVariableSymbol* dbAccessModeArrSymbol, DbkMode mode, int index) {
    SgIntVal* indexExp = SageBuilder::buildIntVal(index);
    SgVarRefExp* arrVarRefExp = SageBuilder::buildVarRefExp(dbAccessModeArrSymbol);
    SgPntrArrRefExp* pntrArrRefExp = SageBuilder::buildPntrArrRefExp(arrVarRefExp, indexExp);
    SgIntVal* modeExp = SageBuilder::buildIntVal(0);
    string modeText;
    switch(mode.getDbkMode()) {
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
    SgExprStatement* assignStmt = SageBuilder::buildAssignStatement(pntrArrRefExp, modeExp);
    return assignStmt;
  }

  SgStatement* buildEdtSpawnCallExp(SgVariableSymbol* edtTemplGuidSymbol, int ntasks, SgVariableSymbol* depElemStructSymbol,
				    SgVariableSymbol* depElemSizeVarSymbol, int ndeps, SgVariableSymbol* depvGuidArrSymbol,
				    SgVariableSymbol* dbAccessModeArrSymbol, int ranksPerAffinity, SgVariableSymbol* outEvtGuidSymbol,
				    SgScopeStatement* scope) {
    vector<SgExpression*> args;
    SgVarRefExp* first = SageBuilder::buildVarRefExp(edtTemplGuidSymbol);
    args.push_back(first);

    SgIntVal* second = SageBuilder::buildIntVal(ntasks);
    args.push_back(second);

    // third and fourth can be NULL
    SgExpression *third, *fourth;
    if(depElemStructSymbol && depElemSizeVarSymbol) {
      third = SageBuilder::buildVarRefExp(depElemSizeVarSymbol);
      SgType* u64_t = AstBuilder::buildu64PtrType(scope);
      SgVarRefExp* depElemVarRefExp = SageBuilder::buildVarRefExp(depElemStructSymbol);
      SgExpression* addressOfExp = SageBuilder::buildAddressOfOp(depElemVarRefExp);
      fourth = SageBuilder::buildCastExp(addressOfExp, u64_t);
    }
    else {
      third = SageBuilder::buildIntVal(0);
      fourth = SageBuilder::buildIntVal(0);
    }
    args.push_back(third);
    args.push_back(fourth);

    SgExpression* fifth = SageBuilder::buildIntVal(ndeps);
    args.push_back(fifth);

    SgExpression* sixth;
    if(depvGuidArrSymbol) {
      sixth = SageBuilder::buildVarRefExp(depvGuidArrSymbol);
    }
    else {
      sixth = SageBuilder::buildIntVal(0);
    }
    args.push_back(sixth);

    SgExpression* seventh;
    if(dbAccessModeArrSymbol) {
      seventh = SageBuilder::buildVarRefExp(dbAccessModeArrSymbol);
    }
    else {
      seventh = SageBuilder::buildIntVal(0);
    }
    args.push_back(seventh);

    SgExpression* eighth = SageBuilder::buildIntVal(ranksPerAffinity);
    args.push_back(eighth);

    SgExpression* ninth = SageBuilder::buildVarRefExp(outEvtGuidSymbol);
    args.push_back(ninth);

    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    SgFunctionCallExp* callExp = SageBuilder::buildFunctionCallExp("spmdEdtSpawn", SageBuilder::buildVoidType(), exprList, scope);
    return SageBuilder::buildExprStatement(callExp);
  }

  SgExprStatement* buildSpmdRankFinalizeCallExp(SgVariableSymbol* triggerEvtGuidSymbol, SgScopeStatement* scope) {
    vector<SgExpression*> args;
    SgExpression* first = NULL;
    if(triggerEvtGuidSymbol) {
      first = SageBuilder::buildVarRefExp(triggerEvtGuidSymbol);
    }
    else {
      first = SageBuilder::buildIntVal();
      string nullGuidStr = "NULL_GUID";
      SageInterface::addTextForUnparser(first, nullGuidStr, AstUnparseAttribute::e_replace);
    }
    args.push_back(first);
    SgExpression* second = SageBuilder::buildBoolValExp(0);
    args.push_back(second);
    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    SgFunctionCallExp* callExp = SageBuilder::buildFunctionCallExp("spmdRankFinalize", SageBuilder::buildVoidType(), exprList, scope);
    return SageBuilder::buildExprStatement(callExp);
  }

  SgStatement* buildSpmdMyRankCallExp(SgVarRefExp* rankVarRefExp, SgScopeStatement* scope) {
    vector<SgExpression*> args;
    assert(rankVarRefExp->get_symbol());
    SgVarRefExp* lvalue = SageBuilder::buildVarRefExp(rankVarRefExp->get_symbol());
    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    SgFunctionCallExp* callExp = SageBuilder::buildFunctionCallExp("spmdMyRank", SageBuilder::buildIntType(), exprList, scope);
    SgExprStatement* rankStmt = SageBuilder::buildAssignStatement(lvalue, callExp);
    return rankStmt;
  }

  SgStatement* buildSpmdSizeCallExp(SgVarRefExp* sizeVarRefExp, SgScopeStatement* scope) {
    vector<SgExpression*> args;
    assert(sizeVarRefExp->get_symbol());
    SgVarRefExp* lvalue = SageBuilder::buildVarRefExp(sizeVarRefExp->get_symbol());
    SgExprListExp* exprList = SageBuilder::buildExprListExp(args);
    SgFunctionCallExp* callExp = SageBuilder::buildFunctionCallExp("spmdSize", SageBuilder::buildIntType(), exprList, scope);
    SgExprStatement* sizeStmt = SageBuilder::buildAssignStatement(lvalue, callExp);
    return sizeStmt;
  }

  /*********************
   * ReplaceReturnStmt *
   *********************/
  ReplaceReturnStmt::ReplaceReturnStmt() { }

  void ReplaceReturnStmt::visit(SgNode* sgn) {
    if(SgReturnStmt* returnStmt = isSgReturnStmt(sgn)) {
      SgStatement* newStmt = buildEdtReturnStmt();
      SageInterface::replaceStatement(returnStmt, newStmt, true);
    }
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
      return varType;
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
  SgExpression* extractSizeInBytes(SgNode* root) {
    class GetDbkSizeTraversal : public AstSimpleProcessing {
      SgExpression* m_arg;
    public:
      GetDbkSizeTraversal() { }
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
	else if(SgTypedefType* dbkType = isSgTypedefType(sgn)) {
	  m_arg = SageBuilder::buildSizeOfOp(dbkType);
	}
      }

      void atTraversalEnd() {
	assert(m_arg);
      }

      SgExpression* getSizeOfExpr() const {
	return m_arg;
      }
    };
    GetDbkSizeTraversal getDbkSizeTraversal;
    getDbkSizeTraversal.traverse(root, preorder);
    SgExpression* arg = getDbkSizeTraversal.getSizeOfExpr();
    assert(arg);
    return arg;
  }

  // Signature for ocrDbCreate
  // u8 ocrDbCreate( ocrGuid_t ∗ db, void ∗∗ addr, u64 len, u16
  // 		  flags, ocrHint_t ∗ hint, ocrInDbAllocator_t allocator )
  SgExprStatement* buildOcrDbCreateFuncCallExp(SgName dbkGuidName, SgName dbkPtrName, SgScopeStatement* scope, SgNode* dbkSizeRoot) {
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
    SgCastExp* castExp = SageBuilder::buildCastExp(SageBuilder::buildAddressOfOp(varDbkVarRefExp), castType);
    args.push_back(castExp);
    // third argument is len
    // this information is either in a malloc call or in the declaration
    SgExpression* third = extractSizeInBytes(dbkSizeRoot);
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

  SgClassDeclaration* buildArrDbkStructDecl(std::string dbkStructName, SgInitializedName* arrInitializedName, SgScopeStatement* scope) {
    SgClassDeclaration* dbkStructDecl = SageBuilder::buildStructDeclaration(dbkStructName, scope);
    SgVariableDeclaration* arrDecl = SageBuilder::buildVariableDeclaration(arrInitializedName->get_name(), arrInitializedName->get_type(), NULL, scope);
    SgClassDefinition* dbkStructDefn = dbkStructDecl->get_definition();
    dbkStructDefn->append_member(arrDecl);
    return dbkStructDecl;
  }

  SgTypedefDeclaration* buildArrDbkStructTypedefType(SgClassDeclaration* dbkStructDecl, SgScopeStatement* scope) {
    std::string dbkStructName = dbkStructDecl->get_name().getString();
    std::string dbkTypedefName = dbkStructName + "_t";
    SgType* baseType = dbkStructDecl->get_type();
    SgTypedefDeclaration* typedefDecl = SageBuilder::buildTypedefDeclaration(dbkTypedefName, baseType, scope, true);
    return typedefDecl;
  }

  SgVariableDeclaration* buildArrPtrDecl(SgInitializedName* arrInitializedName, SgVariableSymbol* dbkPtrSymbol,
					 SgClassDeclaration* dbkStructDecl, SgScopeStatement* scope) {
    vector<SgInitializedName*> depElemMemberVars = getDepElemStructMembers(dbkStructDecl);
    string arrPtrName = arrInitializedName->get_name().getString();
    SgInitializedName* arrPtrMemberVar = getMatchingDepElemMemberVar(depElemMemberVars, arrPtrName);
    SgArrayType* arrType = isSgArrayType(arrInitializedName->get_type());
    // We need to skip the first dimension and create a pointer for array type from second dimension
    SgPointerType* arrPtrType = SageBuilder::buildPointerType(arrType->get_base_type());
    SgVarRefExp* dbkPtrVarRefExp = SageBuilder::buildVarRefExp(dbkPtrSymbol);
    SgVariableSymbol* dbkMemberVarSymbol = GetVariableSymbol(arrPtrMemberVar);
    SgVarRefExp* dbkMemberVarRefExp = SageBuilder::buildVarRefExp(dbkMemberVarSymbol);
    SgArrowExp* rhsAssignOp = SageBuilder::buildArrowExp(dbkPtrVarRefExp, dbkMemberVarRefExp);
    SgAssignInitializer* assignInitializer = SageBuilder::buildAssignInitializer(rhsAssignOp, arrPtrType);
    SgVariableDeclaration* arrPtrDecl = SageBuilder::buildVariableDeclaration(arrPtrName, arrPtrType, assignInitializer, scope);
    return arrPtrDecl;
  }
}

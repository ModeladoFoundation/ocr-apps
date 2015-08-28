/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */


// The ROSE tool globals2db finds all globals and
// file statics, and
//    1. Place them in a structure
//    2. Creates an OCR Data Block.
//    3. Initialize the globals and statics which were originally
//       initialized in the declaration.
//    4. Update the references to the global and file statics to
//       reflect their new location.
//    5. Places a pointer to the OCR Data Block in ELS.
//
// The ROSE tool globals2struct finds all globals and
// file statics, and
//    1. Place them in a structure
//    2. Initialize the globals and statics which were originally
//       initialized in the declaration.
//    3. Update the references to the global and file statics to
//       reflect their new location.
//    4. Places a pointer to the structure in TLS.
//
// Note that the original declarations remain, because they are needed
// in the initialization routines.


#include "rose.h"
#include "arrIndexLabeler.h"
#include <stdio.h>
#include <string.h>
#include "dbElement.h"
#include "mpi2mpilite.h"

using namespace std;
using namespace SageBuilder;
using namespace SageInterface;


class visitorTraversal : public AstSimpleProcessing
{
  public:
    visitorTraversal(SgProject* project);
    virtual void atTraversalStart();
    virtual void visit(SgNode* n);
    virtual void atTraversalEnd();
    void writeHeaderFile(SgProject* project);
    void writeInitFunction(SgProject * project);
    void insertHeaders(SgProject * project);
    void errorCheck();
    DbElement* onGlobalDeclList(SgVariableSymbol* sym);
    FileElement* onLocalFileList(string filename);
    SourceFile * onFileList(string pathname);
    FunctionElement * onFunctionList(SgFunctionDefinition * func);
    TgvElement* onTgvList(string varName);
    IncElement* onIncList(string nodeStr);
    SgBasicBlock* getLocalInitFunction(DbElement* dbElement);
    SgExprStatement* createInitStmt(DbElement* dbElement, SgVarRefExp* structPtrExp);
    bool isMagicalVariable(SgInitializedName * iname);
    SgVariableSymbol* find_symbol_in_symbol_table(SgInitializedName* variableName);



    enum Status
    {
        E_NONE,
        E_NO_MPI_FOUND,
        E_MPI_FOUND
    };
    void setStatus(visitorTraversal::Status status) {_status=status;}
    visitorTraversal::Status getStatus(){return _status;}


    private:
    Rose_STL_Container<DbElement *> _globalDeclList;
    Rose_STL_Container<FileElement *> _localFileList;
    Rose_STL_Container<SourceFile *> _fileList;
    Rose_STL_Container<FunctionElement *> _functionList;
    Rose_STL_Container<TgvElement *> _tgvList;
    Rose_STL_Container<IncElement *>_incList;
    SgScopeStatement* _globalScope;
    string _structName;
    SgClassDeclaration* _structDecl;
    SgClassDefinition* _structDef;
    SgType* _structType;
    string _ptrName;
    bool _isCreated;
    string _currentPathname;
    SgScopeStatement* _fileScope;
    string _mainPathname;
    SgScopeStatement* _mainScope;
    SgStatement * _here;
    Status _status;
};


visitorTraversal::visitorTraversal(SgProject* project)
{
    _globalScope=getFirstGlobalScope(project);
    _structName = strFFWDS;
    _structDecl = buildStructDeclaration(_structName, _globalScope);
    _structDef = buildClassDefinition(_structDecl);
    _structType = _structDecl->get_type();
    _ptrName = strFFWDP;
    _isCreated=false;
    _mainPathname.clear();
    _mainScope=NULL;
    _here=NULL;
    _status=E_NO_MPI_FOUND;

    // Push the include files that are needed by translated code onto the include list.
    // #include <stdlib.h>  (for malloc())
    IncElement * incElement = new IncElement(string("#include <stdlib.h>\n"));
    _incList.push_back(incElement);
    // #include <string.h>  (for memcpy())
    incElement = new IncElement(string("#include <string.h>\n"));
    _incList.push_back(incElement);
    // #include <stdio.h>  (for debugging)
    incElement = new IncElement(string("#include <stdio.h>\n"));
    _incList.push_back(incElement);
#if defined (__FFWD_DB_)
    // #include <mpi.h>     (for ocrCheckStatus())
    incElement = new IncElement(string("#include <mpi.h>\n"));
    _incList.push_back(incElement);
    // #include <ocr.h>     (for u8, u64)
    incElement = new IncElement(string("#include <ocr.h>\n"));
    _incList.push_back(incElement);
#endif
}


void visitorTraversal::writeHeaderFile(SgProject* project)
{
    FILE *fp=NULL;
    string fname = strROSE_FFWDH;


    if ((fp = fopen(fname.c_str(), "w")) == 0)
    {
        printf("Error: Could not create header file.  aborting\n") ;
        exit(-1) ;
    }

    fprintf(fp, "#ifndef _FFWD_H_\n");
    fprintf(fp, "#define _FFWD_H_\n\n");



    // Add include files.  At te moment, I am adding all of the includes files,
    // because i have not found a reliable way to determine which include files
    // are needed, and which ones are not.
    for (Rose_STL_Container<IncElement *>::iterator iter = _incList.begin();
         iter != _incList.end(); iter++)
    {
        IncElement * incElement = (*iter);
        fprintf(fp, "%s", incElement->get_string().c_str());
    }
    fprintf(fp, "\n");


    // struct __ffwd2_s {...};
    fprintf(fp, "struct %s\n{\n", _structName.c_str());


    for (Rose_STL_Container<DbElement *>::iterator iter = _globalDeclList.begin();
         iter != _globalDeclList.end(); iter++)
    {
        DbElement* dbElement=(*iter);
        string typeString = dbElement->get_type_str();
        fprintf(fp, "    %s %s", typeString.c_str(), dbElement->get_new_name().c_str());

        Rose_STL_Container<SgExpression*>dimExprPtrList = dbElement->get_dim_expr_ptr_list();
        if (dimExprPtrList.size() > 0)
        {
            Rose_STL_Container<SgExpression*>::iterator it = dimExprPtrList.begin();
            SgExpression* indexExp = isSgExpression(*it);
            ROSE_ASSERT(indexExp != NULL);
            for ( ; it < dimExprPtrList.end(); it++)
            {
                indexExp = isSgExpression(*it);
                ROSE_ASSERT(indexExp != NULL);
                fprintf(fp, "[%s]", indexExp->unparseToString().c_str());
            }
            fprintf(fp, ";\n");
        }
        else
            fprintf(fp, ";\n");

    }
    for (Rose_STL_Container<TgvElement *>::iterator iter = _tgvList.begin();
         iter != _tgvList.end(); iter++)
    {
        TgvElement* tgvElement=(*iter);
        fprintf(fp, "    int %s;\n", tgvElement->get_name().c_str());
    }
    fprintf(fp, "};\n\n");

    // typedef struct __ffwd_s __ffwd_t;
    fprintf(fp, "typedef struct %s %s;\n\n", _structName.c_str(), strFFWDT.c_str());

#ifdef __FFWD_DB_
    // these function are defined in mpi_ocr.c.
    fprintf(fp, "extern u64 * __getGlobalDBAddr();\n");
    fprintf(fp, "extern void __setGlobalDBContext(ocrGuid_t * ffwd_db_guid, void ** ffwd_addr_ptr);\n");
    fprintf(fp, "extern void __ocrCheckStatus(u8 status, char * functionName);\n\n");
#endif

    fprintf(fp, "#endif\n");

    fclose(fp);
}


#if defined (__FFWD_DB_)
// write a global initialization routine
//
// ocrGuid_t  __ffwd_init(void** ffwd_addr_p){...}
    // ocrGuid_t ffwd_db_guid=0;
    // __ffwd_t * ffwd_addr_ptr=NULL;
    // u8 ffwd_status;
    // ffwd_status = ocrDbCreate( &ffwd_db_guid, (void **)&ffwd_addr_ptr, sizeof(__ffwd_t),
    //                            0, NULL_GUID, NO_ALLOC);
    // __setGlobalDBContext(&ffwd_db_guid, (void *)&ffwd_addr_ptr);
    // __ocrCheckStatus(ffwd_status, "ocrDbCreate");
    // ffwd2_<filename>_init();
    // *ffwd_addr_p = (void *)ffwd_addr_ptr;
    // return ffwd_db_guid;

void visitorTraversal::writeInitFunction(SgProject * project)
{
    // ocrGuid_t is defined as a intptr_t.  ROSE does not have this type built-in.
    string ocrGuidTypedefName("ocrGuid_t");
    SgTypedefDeclaration * ocrGuidTypedefDecl = buildTypedefDeclaration(ocrGuidTypedefName, buildLongType(),
                                                                        _globalScope);
    SgTypedefType * ocrGuidType = ocrGuidTypedefDecl->get_type();

    // ocrGuid_t  __ffwd_init(void** ffwd_addr_p){...}
    const SgName* globalInitName = new SgName(strFFWDInit);
    SgFunctionParameterList * paraList = buildFunctionParameterList();
    SgInitializedName * arg1 = buildInitializedName("ffwd_addr_p",
                                                    buildPointerType(buildPointerType(buildVoidType())));
    appendArg(paraList, arg1);
    SgFunctionDeclaration * globalInit = buildDefiningFunctionDeclaration
                                         (*globalInitName,(SgType*)ocrGuidType,
                                          paraList,_globalScope);
    appendStatement(globalInit, _globalScope);

    // create the body:
    SgBasicBlock * globalInitBB = globalInit->get_definition()->get_body();
    SgScopeStatement * globalInitScope = globalInit->get_scope();

    // ocrGuid_t ffwd_db_guid=0;
    string dbName("ffwd_db_guid");
    SgVarRefExp* dbNameExp = buildVarRefExp(SgName(dbName), globalInitBB);
    SgVariableDeclaration* ffwdDBGuid = buildVariableDeclaration(SgName(dbName), ocrGuidType,
                                                                   NULL, globalInitBB);
    appendStatement(ffwdDBGuid, globalInitBB);
    SgExprStatement * assignOp1 = buildAssignStatement(dbNameExp, buildLongIntVal(0));
    appendStatement(assignOp1, globalInitBB);

    // __ffwd_t * ffwd_addr_ptr=NULL;
    SgTypedefDeclaration * ffwdTypedefDecl = buildTypedefDeclaration(strFFWDT,_structType,
                                                                    globalInitBB);
    SgTypedefType * ffwdType = ffwdTypedefDecl->get_type();
    string ptrName("ffwd_addr_ptr");
    SgVarRefExp * ffwdAddrExp = buildVarRefExp(SgName(ptrName), globalInitBB);
    SgPointerType * ptrType = buildPointerType(ffwdType);
    SgVariableDeclaration* ffwdAddrPtr = buildVariableDeclaration(SgName(ptrName), ptrType,
                                                                   NULL, globalInitBB);
    appendStatement(ffwdAddrPtr, globalInitBB);
    SgExprStatement * assignOp2 = buildAssignStatement(ffwdAddrExp, buildLongIntVal(0));
    appendStatement(assignOp2, globalInitBB);


    // u8 ffwd_status;
    SgName statusName = SgName("ffwd_status");
    SgVariableDeclaration * ffwdStatus = buildVariableDeclaration(statusName,
                                                                 (SgType *)buildShortType() ,
                                                                 NULL, globalInitBB);
    appendStatement(ffwdStatus, globalInitBB);


    // ffwd_status = ocrDbCreate( &ffwd_db_guid, (void **)&ffwd_addr_ptr, sizeof(__ffwd_t),
    //                            0, NULL_GUID, NO_ALLOC);
    SgExpression * addrOfDBGuid = buildAddressOfOp(buildVarRefExp(SgName(dbName),globalInitBB));
    SgPointerType * ptrPtrType = buildPointerType(buildPointerType(buildVoidType()));
    SgExpression * castExp = buildCastExp(buildAddressOfOp(buildVarRefExp(SgName(ptrName), globalInitBB)),
                                         ptrPtrType, SgCastExp::e_C_style_cast);
    SgExprListExp * sizeofParam = buildExprListExp(buildVarRefExp(strFFWDT, globalInitBB));
    SgFunctionCallExp* sizeofExp = buildFunctionCallExp(SgName("sizeof"), buildIntType(),
                                                        sizeofParam, globalInitBB);
    SgExprListExp * arg_list = buildExprListExp();
    appendExpression(arg_list,addrOfDBGuid);
    appendExpression(arg_list,castExp);
    appendExpression(arg_list,sizeofExp);
    appendExpression(arg_list,buildIntVal(0));
    appendExpression(arg_list,buildIntVal(0));
    appendExpression(arg_list,buildIntVal(0));

    SgFunctionCallExp * dbCreateCall = buildFunctionCallExp("ocrDbCreate", (SgType *)buildShortType(),
                                                       arg_list, globalInitBB);
    SgVarRefExp * ffwdStatusExp = buildVarRefExp(statusName, globalInitBB);
    SgExprStatement * assignOp3 = buildAssignStatement(ffwdStatusExp,dbCreateCall);
    appendStatement(assignOp3, globalInitBB);


#if 0
    // DEBUGGING!!!!
    // printf("writeInitFunction: ffwd_db_guid=%p, ffwd_addr_ptr=%p\n", ffwd_db_guid, ffwd_addr_ptr);
    string message = "writeInitFunction: ffwd_db_guid=%p, ffwd_addr_ptr=%p\\n";
    SgVarRefExp * ffwdGuidExp1 = buildVarRefExp(SgName("ffwd_db_guid"), globalInitBB);
    SgVarRefExp * ffwdAddrExp1 = buildVarRefExp(SgName("ffwd_addr_ptr"), globalInitBB);

    SgExprListExp* params = buildExprListExp(buildStringVal(message),
                                             ffwdGuidExp1, ffwdAddrExp1);
    SgExprStatement* printCall = buildFunctionCallStmt("printf", buildVoidType(),
                                                       params, globalInitBB);
    appendStatement(printCall, globalInitBB);
#endif


    // __ocrCheckStatus will exit if there is a failure.
    // __ocrCheckStatus(ffwd_status, "ocrDbCreate");
    SgVarRefExp * statusExp = buildVarRefExp(SgName("ffwd_status"), globalInitBB);
    SgStringVal * stringVal = buildStringVal("ocrDbCreate");
    SgExprListExp * arg1_list = buildExprListExp();
    appendExpression(arg1_list,statusExp);
    appendExpression(arg1_list,stringVal);
    SgExprStatement * checkStatusCall = buildFunctionCallStmt("__ocrCheckStatus", (SgType *)buildVoidType(),
                                                              arg1_list, globalInitBB);
    appendStatement(checkStatusCall, globalInitBB);


     // call ffwd_filename_init() functions here
    if ( ! _localFileList.empty() )
    {
        // call the initialization functions in the other files...
        if (_here == NULL)
            _here = getFirstStatement(_globalScope);

        for (Rose_STL_Container<FileElement*>::iterator iter=_localFileList.begin();
             iter!=_localFileList.end(); iter++)
        {
            // function call...
            // ffwd_<filename>_init(ffwd_addr_ptr);
            FileElement* element = (*iter);
            SgName* name = element->get_init_name();
            SgVarRefExp * ptrRef = buildVarRefExp(SgName(ptrName), globalInitBB);
            SgExprListExp * paraList= buildExprListExp();
            appendExpression(paraList, ptrRef);
            SgExprStatement * funcCall = buildFunctionCallStmt(*name, (SgType *)buildVoidType(),
                                                               paraList, _globalScope);
            appendStatement(funcCall, globalInitBB);

            // external declarations...
            // extern void ffwd_<filename>_init(__ffwd_t * __ffwd_p);
            SgTypedefDeclaration * ffwdTypedefDecl = buildTypedefDeclaration(strFFWDT,
                                                                             _structType,
                                                                             globalInitBB);
            SgTypedefType * ffwdType = ffwdTypedefDecl->get_type();
            SgPointerType * ptrType1 = buildPointerType(ffwdType);
            SgFunctionParameterList * parameterList = buildFunctionParameterList();
            SgInitializedName * arg1 = buildInitializedName(strFFWDP, ptrType1);
            appendArg(parameterList, arg1);
            SgFunctionDeclaration * decl = buildNondefiningFunctionDeclaration
                                           (*name,(SgType*)buildVoidType(),parameterList,
                                            _globalScope);

            string pathname = element->get_file_info()->get_filename();
            if(strcmp(_mainPathname.c_str(), pathname.c_str()) != 0)
                decl->get_declarationModifier().get_storageModifier().setExtern();
            insertStatementBefore(_here, decl);
            _here = decl;
        }
    }  // not localFileList empty

    // *ffwd_addr_p = (void *)ffwd_addr_ptr;
    SgExpression * addrDeref = buildPointerDerefExp(buildVarRefExp(SgName("ffwd_addr_p"), globalInitBB));
    SgExpression * castPtrExp = buildCastExp(buildVarRefExp(SgName("ffwd_addr_ptr"), globalInitBB),
                                             buildPointerType(buildVoidType()), SgCastExp::e_C_style_cast);
    SgExprStatement * assignOp4 = buildAssignStatement(addrDeref, castPtrExp);
    appendStatement(assignOp4, globalInitBB);


    // return ffwd_db_guid;
    SgVarRefExp* dbGuid = buildVarRefExp(SgName("ffwd_db_guid"), globalInitBB);
    SgReturnStmt * returnStmt = buildReturnStmt(dbGuid);
    appendStatement(returnStmt, globalInitBB);
}


void visitorTraversal::insertHeaders(SgProject* project)
{
    for (Rose_STL_Container<SourceFile*>::iterator iter=_fileList.begin();
         iter!=_fileList.end(); iter++)
    {
        SourceFile* file = (*iter);

        // at top, insert include files.  mpi.h should already be present.
        SgScopeStatement* fileScope=file->get_file_scope();
        insertHeader(strROSE_FFWDH, PreprocessingInfo::after, false, fileScope);
        insertHeader("ocr.h", PreprocessingInfo::after, false, fileScope);
    }
}



// mpilite redefines the "main" function to be called "__mpiOcrMain".
SgFunctionDeclaration* findMpiOcrMain( SgProject* project)
{
    Rose_STL_Container<SgNode*> funcList = NodeQuery::querySubTree(project, V_SgFunctionDeclaration);
    for(Rose_STL_Container<SgNode*>::iterator i = funcList.begin(); i != funcList.end(); i++)
    {
        SgFunctionDeclaration* funcNode = isSgFunctionDeclaration(*i);

        if (isSgGlobal(isSgStatement(funcNode)->get_scope()) &&
            isSgFunctionDeclaration(funcNode)->get_name() == "__mpiOcrMain")
            return funcNode;
    }
    return NULL;
}



#elif defined(__FFWD_STRUCT_)

// write a global initialization routine
void visitorTraversal::writeInitFunction(SgProject * project)
{
    // void ffwd_init(){...}
    string globalInitName = strFFWDInit;
    SgFunctionParameterList * paraList = buildFunctionParameterList();
    SgFunctionDeclaration * globalInit = buildDefiningFunctionDeclaration
                                         (globalInitName,(SgType*)buildVoidType(),
                                          paraList,_globalScope);
    appendStatement(globalInit, _globalScope);

    // create the body:
    SgBasicBlock* globalInitBB = globalInit->get_definition()->get_body();

    // allocate some memory
    // ffwd_p = ((__ffwd_t *)(malloc((sizeof(__ffwd_t)))));
    string typedefName = strFFWDT;
    SgTypedefDeclaration* typedefDecl = buildTypedefDeclaration(typedefName,_structType, _globalScope);
    SgTypedefType * typedefType = typedefDecl->get_type();
    // this is inserted as text in the ffwd include file.

    SgVarRefExp * castVar = buildVarRefExp(typedefName, _globalScope);
    SgExprListExp* sizeofParam = buildExprListExp(castVar);
    SgFunctionCallExp* sizeofExp = buildFunctionCallExp(SgName("sizeof"), buildIntType(),
                                                        sizeofParam, _globalScope);
    SgExprListExp* mallocParam = buildExprListExp(sizeofExp);
    SgExpression * mallocExp = buildFunctionCallExp(SgName("malloc"),  buildVoidType(), mallocParam, _globalScope);
    SgPointerType* ptrType = buildPointerType(typedefType);
    SgExpression* castExp = buildCastExp(mallocExp, ptrType, SgCastExp::e_C_style_cast);
    SgExprStatement * assignOp = buildAssignStatement(buildVarRefExp(_ptrName, _globalScope), castExp);
    appendStatement(assignOp, globalInitBB);

    // memset(ffwd_p, 0, sizeof(__ffwd_t));
    SgExprListExp* memsetParam = buildExprListExp(buildVarRefExp(_ptrName, _globalScope), buildIntVal(0), deepCopy(sizeofExp));
    SgExpression * memsetExp = buildFunctionCallExp(SgName("memset"),  buildVoidType(), memsetParam, _globalScope);
    SgExprStatement * memsetStmt = buildExprStatement(memsetExp);
    appendStatement(memsetStmt, globalInitBB);

    if ( ! _localFileList.empty() )
    {
        // call the initialization functions in the other files...
        if (_here == NULL)
            _here = getFirstStatement(_globalScope);

        for (Rose_STL_Container<FileElement*>::iterator iter=_localFileList.begin();
             iter!=_localFileList.end(); iter++)
        {
            // function call...
            // ffwd_<filename>_init(__ffwd_p);
            FileElement* element = (*iter);
            SgName* name = element->get_init_name();
            SgVarRefExp * ptrRef = buildVarRefExp(SgName(_ptrName), globalInitBB);
            SgExprListExp * paraList= buildExprListExp();
            appendExpression(paraList, ptrRef);
            SgExprStatement * funcCall = buildFunctionCallStmt(*name, (SgType *)buildVoidType(),
                                                               paraList, _globalScope);
            appendStatement(funcCall, globalInitBB);

            // external declarations...
            // extern void ffwd_<filename>_init(__ffwd_t * __ffwd_p);
            SgTypedefDeclaration * ffwdTypedefDecl = buildTypedefDeclaration(strFFWDT,
                                                                             _structType,
                                                                             globalInitBB);
            SgTypedefType * ffwdType = ffwdTypedefDecl->get_type();
            SgPointerType * ptrType1 = buildPointerType(ffwdType);
            SgFunctionParameterList * parameterList = buildFunctionParameterList();
            SgInitializedName * arg1 = buildInitializedName(strFFWDP, ptrType1);
            appendArg(parameterList, arg1);
            SgFunctionDeclaration * decl = buildNondefiningFunctionDeclaration
                                           (*name,(SgType*)buildVoidType(),parameterList,
                                            _globalScope);

            string pathname = element->get_file_info()->get_filename();

            if(strcmp(_mainPathname.c_str(), pathname.c_str()) != 0)
                decl->get_declarationModifier().get_storageModifier().setExtern();
            insertStatementBefore(_here, decl);
            _here = decl;
        }
    }  // not localFileList empty
}


void visitorTraversal::insertHeaders(SgProject* project)
{
    for (Rose_STL_Container<SourceFile*>::iterator iter=_fileList.begin();
         iter!=_fileList.end(); iter++)
    {
        SourceFile* file = (*iter);
        SgScopeStatement* scope;

        // at top, insert #include "rose_ffwd.h"
        SgScopeStatement* fileScope=file->get_file_scope();
        if (fileScope != 0)
            scope = fileScope;
        else
            scope = _globalScope;

        insertHeader(strROSE_FFWDH, PreprocessingInfo::after,false, scope);

        // Create a pointer to the new data structure
        // __ffwd_t *ffwd_p;
        string typedefName = strFFWDT;
        SgTypedefDeclaration* typedefDecl = buildTypedefDeclaration(typedefName, _structType, scope);
        SgTypedefType * typedefType = typedefDecl->get_type();
        // this is inserted as text in the ffwd include file.

        string ptrName = strFFWDP;
        SgVarRefExp* ptrNameExp = buildVarRefExp(SgName(ptrName), scope);
        SgPointerType* ptrType = buildPointerType(typedefType);
        SgVariableDeclaration* structVarPtr = buildVariableDeclaration(SgName(ptrName), ptrType,
                                                                       NULL, scope);
        // set TLS to add __thread
        structVarPtr->get_declarationModifier().get_storageModifier().set_thread_local_storage(true);

        string pathname = file->get_path_name();
        if(strcmp(_mainPathname.c_str(), pathname.c_str()) != 0)
            structVarPtr->get_declarationModifier().get_storageModifier().setExtern();
        insertStatementBefore(getFirstStatement(scope), structVarPtr);
    }
}

#else
#error  *******  ERROR: Neither __FFWD_DB_ nor __FFWD_STRUCT_ is defined. *******
#endif



void visitorTraversal::errorCheck()
{
    bool error_reported=false;

#ifdef __FFWD_DB_
    if (_status == E_NO_MPI_FOUND)
    {
        printf("Error: MPI_Init() not found.  This tool is specifically for applications with MPI support.  Stopping.\n");
        error_reported=true;
    }
#endif
    if (_globalDeclList.empty())
    {
        printf("Error: No global or function static variables were found.  This tool is specifically for applications with global or file static variables.  Stopping.\n");
        error_reported = true;
    }

    if (error_reported)
        exit(1);
    else
        //printf("errorCheck: no error reported\n");
        ;
}

DbElement* visitorTraversal::onGlobalDeclList(SgVariableSymbol* sym)
{
    SgInitializedName * iName = sym->get_declaration();
    string sName = iName->get_name().getString();
    SgDeclarationStatement * decl = iName->get_declaration();
    SgScopeStatement * scope = iName->get_scope();
    string nName = create_new_name(iName, scope);


    ROSE_ASSERT(isSgGlobal(scope) || isStatic(decl));

    if(_globalDeclList.empty())
        return NULL;

    for (Rose_STL_Container<DbElement*>::iterator iter=_globalDeclList.begin();
         iter!=_globalDeclList.end(); iter++)
    {
        DbElement* element = *iter;
        string name = element->get_name()->get_name().getString();
        string newName = element->get_new_name();

        if (name == sName && newName == nName)
            return element;
    }
    return NULL;
}


FileElement* visitorTraversal::onLocalFileList(string filename)
{
    if(_localFileList.empty())
        return NULL;

    for (Rose_STL_Container<FileElement*>::iterator iter=_localFileList.begin();
         iter!=_localFileList.end(); iter++)
    {
        FileElement* element = (*iter);
        string name = element->get_file_name();
        if (strcmp(name.c_str(), filename.c_str()) == 0) {
            return element;
        }
    }
    return NULL;
}


SourceFile * visitorTraversal::onFileList(string pathname)
{
    if(_fileList.empty())
        return NULL;

    for (Rose_STL_Container<SourceFile *>::iterator iter=_fileList.begin();
         iter!=_fileList.end(); iter++)
    {
        SourceFile * element = (*iter);
        string path = element->get_path_name();
        if (strcmp(path.c_str(), pathname.c_str()) == 0) {
            return element;
        }
    }
    return NULL;
}


FunctionElement* visitorTraversal::onFunctionList(SgFunctionDefinition * func)
{
    if(_functionList.empty())
        return NULL;

    for (Rose_STL_Container<FunctionElement*>::iterator iter=_functionList.begin();
         iter!=_functionList.end(); iter++)
    {
        FunctionElement* element = (*iter);

        if (element->get_func_def() == func) {
            return element;
        }
    }
    return NULL;
}


TgvElement* visitorTraversal::onTgvList(string varName)
{
    if(_tgvList.empty())
        return NULL;

    for (Rose_STL_Container<TgvElement*>::iterator iter=_tgvList.begin();
         iter!=_tgvList.end(); iter++)
    {
        TgvElement* element = (*iter);
        string name = element->get_name();
        if (strcmp(name.c_str(), varName.c_str()) == 0) {
            return element;
        }
    }
    return NULL;
}


IncElement* visitorTraversal::onIncList(string nodeStr)
{
    if(_incList.empty())
        return NULL;

    for (Rose_STL_Container<IncElement*>::iterator iter=_incList.begin();
         iter!=_incList.end(); iter++)
    {
        IncElement* element = (*iter);
        string str = element->get_string();
        if (strcmp(str.c_str(), nodeStr.c_str()) == 0) {
            return element;
        }
    }
    return NULL;
}


// Create a file-specific init function.
SgBasicBlock* visitorTraversal::getLocalInitFunction(DbElement* dbElement)
{
    // find the file that this static belongs to.
    Sg_File_Info* fileInfo = dbElement->get_file_info();
    string filename = fileInfo->get_filename();
    SgScopeStatement* fileScope = dbElement->get_scope();
    FileElement* element = NULL;
    SgBasicBlock* initBody = NULL;

    if ((element = onLocalFileList(filename)) == NULL)
    {
        // void ffwd_<filename>_init( __ffwd_t * __ffwd_p){...}
        string file_no_path = StringUtility::stripPathFromFileName(filename);
        std::size_t dot = file_no_path.find(".");
        std::string fname = file_no_path.substr(0,dot);
        SgName* initName = new SgName(strFFWD+strUnderscore+fname+
                                      strUnderscore+strINIT);
        SgTypedefDeclaration * ffwdTypedefDecl = buildTypedefDeclaration(strFFWDT,
                                                                         _structType,
                                                                         fileScope);

        SgTypedefType * ffwdType = ffwdTypedefDecl->get_type();
        SgPointerType * ptrType = buildPointerType(ffwdType);
        SgFunctionParameterList * paraList = buildFunctionParameterList();
        SgInitializedName * arg1 = buildInitializedName(strFFWDP, ptrType);
        appendArg(paraList, arg1);
        SgFunctionDeclaration * init = buildDefiningFunctionDeclaration
                                       (*initName,(SgType*)buildVoidType(),paraList,
                                        fileScope);
        appendStatement(init,fileScope);

        initBody = init->get_definition()->get_body();
        FileElement* element = new FileElement(fileInfo, initBody, initName, fileScope);
        _localFileList.push_back(element);
    }
    else {
        initBody = element->get_init_func();
    }
    return initBody;
}


// Create a statement which initializes the new global with its old value.
SgExprStatement* visitorTraversal::createInitStmt(DbElement* dbElement,
                                                  SgVarRefExp* structPtrExp)
{
    string newName = dbElement->get_new_name();
    SgVarRefExp* newNameExp = buildVarRefExp(newName, _globalScope);
    SgExpression* arrowExp = buildArrowExp(structPtrExp, newNameExp);
    SgScopeStatement* scope = dbElement->get_scope();

    SgExprStatement* initStmt= NULL;
    if (dbElement->get_num_dimensions() == 0)
    {
        if (dbElement->is_const())
        {
            // initialize the field with the original value un-const.
            SgExpression * castExp;
            SgVarRefExp * orig = buildVarRefExp(dbElement->get_name(), scope);
            SgType * type = dbElement->get_base_type();
            string typeStr = dbElement->get_type_str();

            if (typeStr.find("**") != std::string::npos)
            {
                SgPointerType * ptrType = buildPointerType(buildPointerType(type));
                castExp = buildCastExp(orig, ptrType, SgCastExp::e_C_style_cast);
            }
            else if (typeStr.find("*") != std::string::npos)
            {
                SgPointerType * ptrType = buildPointerType(type);
                castExp = buildCastExp(orig, ptrType, SgCastExp::e_C_style_cast);
            }
            else
                castExp = buildCastExp(orig, type, SgCastExp::e_C_style_cast);

            initStmt = buildAssignStatement(arrowExp,castExp);
        }
        else
        {
            // initialize the field with the original value
            initStmt = buildAssignStatement(arrowExp,
                                            buildVarRefExp(dbElement->get_name(), scope));
        }
    }
    else
    {
        // memcpy(__ffwd_p->_orig, orig, numElements*sizeof(type));
        SgExprListExp * sizeofParam = buildExprListExp(buildVarRefExp(dbElement->get_type_str(),
                                                                      scope));
        SgFunctionCallExp* sizeofExp = buildFunctionCallExp(SgName("sizeof"), buildIntType(),
                                                            sizeofParam, scope);
        SgExpression * mulOp = buildMultiplyOp(buildIntVal(dbElement->get_num_elements()),
                                                           sizeofExp);
        SgVarRefExp * nameExp = buildVarRefExp(dbElement->get_name(), scope);
        SgExprListExp * memcpyParam = buildExprListExp(arrowExp, nameExp, mulOp);
        SgFunctionCallExp* memcpyExp = buildFunctionCallExp(SgName("memcpy"), buildIntType(),
                                                            memcpyParam, scope);
        initStmt = buildExprStatement(memcpyExp);
    }
    return initStmt;
}


bool visitorTraversal::isMagicalVariable(SgInitializedName * iname)
{
    const char * name = iname->get_name().getString().c_str();

    // sometimes __PRETTY_FUNCTION__ has a symbol, and sometimes it does not.
    // i do not know why. we do not want magical symbols in the DB.

    if (strcmp(name, "__PRETTY_FUNCTION__") == 0)
        return true;
    if (strcmp(name, "__func__") == 0)
        return true;
    if (strcmp(name, "__FILE__") == 0)
        return true;
    if (strcmp(name, "__LINE__") == 0)
        return true;
    return false;
}


SgVariableSymbol* visitorTraversal::find_symbol_in_symbol_table(SgInitializedName* variableName)
{
    SgVariableSymbol* variableSymbol = isSgVariableSymbol(variableName->get_symbol_from_symbol_table());
    if (variableSymbol != NULL)
        return variableSymbol;


    SgScopeStatement * scope = variableName->get_scope();
    SgSymbolTable * symTable = scope->get_symbol_table();
    std::set<SgNode*> symbols = symTable->get_symbols();
    for (std::set<SgNode*>::iterator it=symbols.begin(); it!=symbols.end(); it++)
    {
        SgVariableSymbol * sym = isSgVariableSymbol(*it);
        if (sym != NULL)
        {
            string symStr = sym->get_name().getString();
            if (strcmp(symStr.c_str(), variableName->get_name().getString().c_str()) == 0)
            {
                variableSymbol = sym;
                return variableSymbol;
            }
        }
    }


    if ( ! isMagicalVariable(variableName))
        printf("Warning: find_symbol_in_symbol_table: Should %s be magical?\n",
               variableName->get_name().getString().c_str());

    return variableSymbol;
}


void visitorTraversal::atTraversalStart()
{
    _currentPathname.clear();
    _fileScope=NULL;
}



// Visit each node in the AST.
void visitorTraversal::visit(SgNode* node)
{
    if (_currentPathname.empty())
    {
        // save current file name - this would be better done in atTraversalStart().
        _currentPathname = node->get_file_info()->get_filenameString();
        //printf("visit: processing %s\n", _currentPathname.c_str());
    }

    if (isSgLocatedNode(node))    // preprocessing directives...
    {
        SgLocatedNode * locatedNode = isSgLocatedNode(node);
        ROSE_ASSERT(locatedNode != NULL);

        AttachedPreprocessingInfoType* info = locatedNode->getAttachedPreprocessingInfo();
        if(info != NULL)
        {
            for(AttachedPreprocessingInfoType::iterator iter=info->begin();
                iter != info->end(); iter++)
            {
                if ((*iter)->getTypeOfDirective() == PreprocessingInfo::CpreprocessorIncludeDeclaration)
                {
                    if ( ! onIncList((*iter)->getString())) {
                        IncElement * incElement = new IncElement((*iter)->getString());
                        _incList.push_back(incElement);
                    }
                }
            }
        }
    }

    if (isSgVariableDeclaration(node) != NULL)
    {
        SgVariableDeclaration * variableDeclaration = isSgVariableDeclaration(node);
        SgInitializedNamePtrList::const_iterator i = variableDeclaration->get_variables().begin();
        ROSE_ASSERT(i != variableDeclaration->get_variables().end());

        do {
            SgInitializedName* variableName = isSgInitializedName(*i);
            if ((isSgGlobal(variableName->get_scope()) || isStatic(variableDeclaration)) &&
                //variableDeclaration->get_declarationModifier().get_storageModifier().isExtern() == false &&
                isMagicalVariable(variableName) == false)
            {
                DbElement * dbElement = NULL;
                SgType* variableType = variableName->get_type();
                SgVariableSymbol* variableSymbol = isSgVariableSymbol(find_symbol_in_symbol_table(variableName));

                if (variableSymbol == NULL)
                {
                    i++;
                    continue;
                }

                if ( ! onGlobalDeclList(variableSymbol))
                {
                    // create a new DB entry if user's file
                    if (isNodeDefinedInUserLocation(variableSymbol) == true) {
                        DbElement * dbElement = new DbElement(variableSymbol, isSgNode(*i));
                        _globalDeclList.push_back(dbElement);
                    }
                }
                if ( ! isSgGlobal(variableName->get_scope())  && dbElement!=NULL)
                {
                    // function static - replace the expression
                    string newName = dbElement->get_new_name();
                    SgScopeStatement * scope = dbElement->get_scope();
                    SgVarRefExp* newNameExp = buildVarRefExp(newName, scope);
                    SgVarRefExp* ptrNameExp = buildVarRefExp(_ptrName, scope);
                    SgExpression* arrowExp = buildArrowExp(ptrNameExp, newNameExp);
                    replaceExpression(isSgExpression(*i), arrowExp, true);


                    // create a tool generated variable (TGV) that will indicate
                    // if the function static has been initialized.
                    string tgvName = newName + strUnderscore + strINIT;
                    if ( ! onTgvList(tgvName))
                    {
                        TgvElement * tgvElement = new TgvElement(tgvName, scope);
                        dbElement->set_tgv(tgvElement);
                        _tgvList.push_back(tgvElement);
                    }
                }

                if ( ! onFileList(_currentPathname))
                {
                    if (_fileScope == NULL)
                        _fileScope = variableName->get_scope();
                    SourceFile * source = new SourceFile(_currentPathname, _fileScope);
                    _fileList.push_back(source);
                }
            }
            i++;
        }
        while (i != variableDeclaration->get_variables().end());

    }  // isSgVariableDeclaration

    if (isSgVarRefExp(node) != NULL)
    {
        // replace the global reference with a reference to the structure
        SgVarRefExp * varRefExp = isSgVarRefExp(node);
        SgVariableSymbol* varSymbol = varRefExp->get_symbol();
        SgInitializedName * varName = varSymbol->get_declaration();
        SgScopeStatement* varScope = varName->get_scope();
        SgDeclarationStatement * varDecl = varName->get_declaration();


        if ( isSgGlobal(varScope)  || isStatic(varDecl))
        {
            // global reference
            DbElement* element = onGlobalDeclList(varSymbol);
            if (element == NULL)
            {
                // TODO: Is there a way to avoid creating the dbElement for externals,
                // and wait until the actual declaration is seen?

                // declaration must be in a different file
                // create a new DB entry if user's file
                if (isNodeDefinedInUserLocation(varSymbol) == true &&
                    //varDecl->get_declarationModifier().get_storageModifier().isExtern() == false  &&
                    isMagicalVariable(varName) == false )
                {
                    element = new DbElement(varSymbol, node);
                    _globalDeclList.push_back(element);
                }
            }


            if (element != NULL)
            {
                // each function that references the variable will need to have
                // the pointer to the datablock set up.
                SgFunctionDefinition * enclosingFunc = getEnclosingFunctionDefinition(node);
                ROSE_ASSERT(enclosingFunc!=NULL);


                if ( ! onFunctionList(enclosingFunc))
                {
                    FunctionElement * func = new FunctionElement(enclosingFunc, node);
                    _functionList.push_back(func);
                }

                if ( ! onFileList(_currentPathname))
                {
                    SourceFile * source = new SourceFile(_currentPathname, varScope);
                    _fileList.push_back(source);
                }

                // replace the global reference with a pointer
                string newName = element->get_new_name();
                SgVarRefExp* newNameExp = buildVarRefExp(newName, varScope);
                SgVarRefExp* ptrNameExp = buildVarRefExp(_ptrName, element->get_scope());
                SgExpression* arrowExp = buildArrowExp(ptrNameExp, newNameExp);
                replaceExpression(isSgExpression(node), arrowExp, true);


                if ( ! isSgGlobal(varScope))
                {
                    // create a tool generated variable (TGV) that will indicate
                    // if the function static has been initialized.
                    string tgvName = newName + strUnderscore + strINIT;
                    if ( ! onTgvList(tgvName))
                    {
                        TgvElement * tgvElement = new TgvElement(tgvName, element->get_scope());
                        fflush(stdout);
                        element->set_tgv(tgvElement);
                        _tgvList.push_back(tgvElement);
                    }
                }
            }
        }
    } // isSgVarRefExp

#ifdef __FFWD_DB_
    if (isSgFunctionCallExp(node) != NULL)
    {
        SgFunctionCallExp * callExp = isSgFunctionCallExp(node);
        SgFunctionSymbol * funcSym = callExp->getAssociatedFunctionSymbol();
        if (funcSym != NULL)
        {
            SgName funcName = funcSym->get_name();
            SgScopeStatement * scope = funcSym->get_declaration()->get_scope();
            if (strncmp(funcName.getString().c_str(), "MPI_", 4) == 0)
            {
                if (strcmp(funcName.getString().c_str(), "MPI_Init") == 0)
                    _status=E_MPI_FOUND;

                // MPI-lite uses different definitions of MPI_Comm, MPI_Op and
                // MPI_Datatype, so if the app is using the real MPI definitions,
                // they need to be replaced.
                //
                SgExprListExp * funcExprListExp = callExp->get_args();
                if (funcExprListExp != NULL)
                {
                    SgExpressionPtrList & exprPtrList = funcExprListExp->get_expressions();
                    for (SgExpressionPtrList::iterator iter = exprPtrList.begin();
                         iter != exprPtrList.end(); iter++)
                    {
                        SgExpression * arg = *iter;
                        string str = arg->unparseToString();

                        if (strncmp(str.c_str(), "((MPI_Datatype )", 16) == 0)
                        {
                            unsigned long datatype=get_mpi_datatype(str);
                            SgUnsignedLongVal * newArg = buildUnsignedLongVal(datatype);
                            string dataName("MPI_Datatype");
                            SgTypedefDeclaration* typedefData =
                                buildTypedefDeclaration(dataName, buildUnsignedLongType(), scope);
                            SgTypedefType * datatypeType = typedefData->get_type();
                            SgExpression * castData = buildCastExp(newArg, datatypeType,
                                                                   SgCastExp::e_C_style_cast);
                            replaceExpression(arg, castData);
                        }
                        else if (strncmp(str.c_str(), "((MPI_Comm )", 12) == 0)
                        {
                            unsigned long comm=get_mpi_comm(str);
                            SgUnsignedLongVal * newArg = buildUnsignedLongVal(comm);
                            string commName("MPI_Comm");
                            SgTypedefDeclaration* typedefComm =
                                buildTypedefDeclaration(commName, buildUnsignedLongType(), scope);
                            SgTypedefType * commType = typedefComm->get_type();
                            SgExpression * castComm = buildCastExp(newArg, commType,
                                                                   SgCastExp::e_C_style_cast);
                            replaceExpression(arg, castComm);
                        }
                        else if (strncmp(str.c_str(), "((MPI_Op )", 10) == 0)
                        {
                            unsigned long op=get_mpi_op(str);
                            SgUnsignedLongVal * newArg = buildUnsignedLongVal(op);
                            string opName("MPI_Op");
                            SgTypedefDeclaration* typedefOp =
                                buildTypedefDeclaration(opName, buildUnsignedLongType(), scope);
                            SgTypedefType * opType = typedefOp->get_type();
                            SgExpression * opComm = buildCastExp(newArg, opType,
                                                                 SgCastExp::e_C_style_cast);
                            replaceExpression(arg, opComm);
                        }
                        else ;
                    }
                }
            }
        }
    } // isSgFunctionCallExp
#endif
}



// atTraversalEnd() is called at the end of every source file that is processed.
void visitorTraversal::atTraversalEnd()
{
//    if ( _globalDeclList.empty())
//        return;


#ifdef __FFWD_DB_
    if ( _status == E_NO_MPI_FOUND )
        return;

    // set up the pointer to the structure in each function that references
    // a global or file static variable.
    if ( ! _functionList.empty())
    {
        for (Rose_STL_Container<FunctionElement *>::iterator iter = _functionList.begin();
             iter != _functionList.end(); iter++)
        {
            FunctionElement* element = *iter;
            SgFunctionDefinition * funcDef = element->get_func_def();
            SgStatement * firstStmt = getFirstStatement(funcDef);
            SgBasicBlock * funcBB =funcDef->get_body();

            // __ffwd_t *  __ffwd_p;
            SgTypedefDeclaration * ffwdTypedefDecl = buildTypedefDeclaration(strFFWDT,
                                                                             _structType,
                                                                             funcBB);
            SgTypedefType * ffwdType = ffwdTypedefDecl->get_type();
            SgPointerType * ptrType = buildPointerType(ffwdType);
            SgVariableDeclaration* ffwdAddrPtr = buildVariableDeclaration(SgName(strFFWDP),
                                                                          ptrType, NULL,
                                                                          funcBB);
            insertStatementBefore(firstStmt, ffwdAddrPtr);


            // __ffwd_p = (__ffwd_t *)__getGlobalDBAddr();
            SgExprListExp * parameters = buildExprListExp();
            SgFunctionCallExp * funcCall = buildFunctionCallExp(SgName("__getGlobalDBAddr"),
                                                               buildUnsignedLongLongType(),
                                                               parameters, _globalScope);
            SgExpression * castPtrExp = buildCastExp(funcCall, ptrType,
                                                     SgCastExp::e_C_style_cast);
            SgVarRefExp * ffwdAddrExp = buildVarRefExp(SgName(strFFWDP), funcBB);
            SgExprStatement * assignOp = buildAssignStatement(ffwdAddrExp, castPtrExp);
            insertStatementBefore(firstStmt, assignOp);


#if 0
            // DEBUGGING!!!!
            // printf("funcName: __ffwd_p=%p\n", __ffwd_p);
            SgName funcName = funcDef->get_declaration()->get_name();

            string message = funcName.getString() + ": __ffwd_p=%p\\n";
            SgVarRefExp * ffwdAddrExp1 = buildVarRefExp(SgName(strFFWDP), funcBB);

            SgExprListExp* params = buildExprListExp(buildStringVal(message),
                                                     ffwdAddrExp1);
            SgExprStatement* printCall = buildFunctionCallStmt("printf", buildVoidType(),
                                                               params, funcBB);
            insertStatementBefore(firstStmt, printCall);
#endif
        }

        _functionList.clear();
    }
#endif


    if (_mainScope == NULL)
    {
        SgFunctionDeclaration* mainDecl = findMain(getFirstGlobalScope(getProject()));
#ifdef __FFWD_DB_
        if (mainDecl==0) {
            // mpi-lite redefines main to __mpiOcrMain.
            mainDecl = findMpiOcrMain(getProject());
        }
#endif
        ROSE_ASSERT(mainDecl != 0);

        SgFunctionDefinition* mainDef = mainDecl->get_definition();
        ROSE_ASSERT(mainDef != NULL);
        _mainScope = mainDef->get_body();
        _mainPathname = mainDecl->get_file_info()->get_filename();

        if ( ! onFileList(_mainPathname))
        {
            SourceFile * source = new SourceFile(_mainPathname, _globalScope);
            _fileList.push_back(source);
        }
    }


    // initialize the global elements which were initialized
    // in their declaration in a file specific init file

    for (Rose_STL_Container<DbElement *>::iterator iter = _globalDeclList.begin();
         iter != _globalDeclList.end(); iter++)
    {
        DbElement* dbElement = *iter;
        if ( isSgGlobal(dbElement->get_scope()))
        {   // global or file static
            if (dbElement->is_initialized() && ! dbElement->init_created())
            {
                SgBasicBlock* initBB = getLocalInitFunction(dbElement);
                ROSE_ASSERT(initBB != NULL);
                SgVarRefExp* structPtrExp = buildVarRefExp(_ptrName, _globalScope);
                SgExprStatement* stmt = createInitStmt(dbElement, structPtrExp);
                ROSE_ASSERT(stmt != NULL);
                appendStatement(stmt, initBB);
                dbElement->set_init_created(true);
            }
        }
        else
        {   //function static
            // if (__ffwd_p->tgv == 0) {
            //     __ffwd_p->fs = fs;
            //     __ffwd_p->tgv = 1;  }

            TgvElement* tgv = dbElement->get_tgv();
            if (tgv != NULL)
            {
                SgScopeStatement * scope = dbElement->get_scope();
                SgBasicBlock * trueBB = buildBasicBlock();

                //     __ffwd_p->fs = fs;
                SgVarRefExp* structPtrExp = buildVarRefExp(_ptrName, scope);
                SgExprStatement* initStmt = createInitStmt(dbElement, structPtrExp);
                dbElement->set_init_created(true);
                appendStatement(initStmt, trueBB);

                //     __ffwd_p->tgv = 1;
                string tgvName = tgv->get_name();
                SgVarRefExp* newNameExp = buildVarRefExp(tgvName, scope);
                SgVarRefExp* ptrNameExp = buildVarRefExp(_ptrName, scope);
                SgExpression* arrowExp = buildArrowExp(ptrNameExp, newNameExp);
                SgExprStatement * assignOp = buildAssignStatement(arrowExp,
                                                                  buildIntVal(1));
                appendStatement(assignOp, trueBB);

                // if (__ffwd_p->tgv == 0)
                SgExpression * tgvEqualZero = buildEqualityOp(deepCopy(arrowExp),
                                                              buildIntVal(0));
                SgIfStmt * ifStmt = buildIfStmt( tgvEqualZero, trueBB, NULL);
                insertStatementAfterLastDeclaration(ifStmt, scope);
            }
        }
    }  // globalDeclList

#ifdef __FFWD_STRUCT_
    if (strcmp(_currentPathname.c_str(), _mainPathname.c_str())==0)
    {
        // forward declarations...
        // void __ffwd_init();
        string globalInitName = strFFWDInit;
        SgFunctionParameterList * parameterList = buildFunctionParameterList();
        SgFunctionDeclaration * forwardDecl = buildNondefiningFunctionDeclaration
                                              (globalInitName,(SgType*)buildVoidType(),
                                               parameterList,_globalScope);
        insertStatementBefore(getFirstStatement(_globalScope), forwardDecl);
        _here=forwardDecl;

        // main calls the initialization function
        // __ffwd_init();
        SgExprListExp * parameters = buildExprListExp();
        SgExprStatement * globalInitCall = buildFunctionCallStmt(globalInitName,(SgType *)buildVoidType(),
                                                                 parameters, _mainScope);
        insertStatementBefore(getFirstStatement(_mainScope), globalInitCall);
    }  // is main file
#endif
}



int main ( int argc, char* argv[] )
{
    SgProject* project = frontend(argc,argv);
    ROSE_ASSERT (project != NULL);

    // add the annotations that identify the index expressions of SgPntrArrRefExps
    arrIndexLabeler::addArrayIndexAnnotations(project);


    // for debugging!
    //generateDOT(*project);

    // Build the traversal object
    visitorTraversal exampleTraversal(project);

    // Call the traversal function (member function of AstSimpleProcessing)
    // starting at the project node of the AST, using a preorder traversal.
    //printf("Traversing the input files\n");
    exampleTraversal.traverseInputFiles(project,preorder);

    // Check for issues.
    exampleTraversal.errorCheck();

    // Write the header file.
    //printf("Writing the header file and init function\n");
    exampleTraversal.insertHeaders(project);
    exampleTraversal.writeHeaderFile(project);
    exampleTraversal.writeInitFunction(project);


    // run all tests
    //printf("Running all tests\n\n");
    AstTests::runAllTests(project);

    // Generate source code from AST and call the vendor's compiler
    //printf("\nCalling backend\n");
    int res = backend(project);

    //printf("Done!\n");
    return (res);
}



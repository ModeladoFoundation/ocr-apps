/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */


// The ROSE tool globals2db supports MPI-lite.  It finds all globals and
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
// file and function statics, and
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
    void insertHeaders(SourceFile * file);
    void errorCheck();
    DbElement* onGlobalDeclList(SgVariableSymbol* sym);
    FileElement* onLocalFileList(string filename);
    HeaderFileElement* onHeaderFileList(string filename);
    SourceFile * onFileList(string pathname);
    FunctionElement * onFunctionList(SgFunctionDefinition * func);
    TgvElement* onTgvList(string varName);
    IncElement* onIncList(string nodeStr);
    SgBasicBlock* getLocalInitFunction(DbElement* dbElement);
    SgExprStatement* createInitStmt(DbElement* dbElement, SgVarRefExp* structPtrExp);
    bool isMagicalVariable(SgInitializedName * iname);
    SgVariableSymbol* find_symbol_in_symbol_table(SgInitializedName* variableName);
    void findMainFn();

    // the beginning of error checking.
    enum Status
    {
        E_NONE,
        E_NO_MPI_FOUND,
        E_MPI_FOUND
    };
    void setStatus(visitorTraversal::Status status) {_status=status;}
    visitorTraversal::Status getStatus(){return _status;}


    private:
    // the list of global and static variables.
    Rose_STL_Container<DbElement *> _globalDeclList;

    // the list of files that contain file static variables.
    Rose_STL_Container<FileElement *> _localFileList;

    // the list of include files that contain file static variables.
    Rose_STL_Container<HeaderFileElement *> _headerFileList;

    // list of files that contain global or static variables.
    Rose_STL_Container<SourceFile *> _fileList;

    // the list of functions that contain references to globals or statics.
    Rose_STL_Container<FunctionElement *> _functionList;

    // the list of tool generated variables (tgv).
    Rose_STL_Container<TgvElement *> _tgvList;

    // the list of include files used by the application.
    Rose_STL_Container<IncElement *>_incList;

    SgScopeStatement* _globalScope;
    string _structName;
    SgClassDeclaration* _structDecl;
    SgClassDefinition* _structDef;
    SgType* _structType;
    string _ptrName;
    bool _isCreated;
    string _currentPathname;
    Sg_File_Info* _currentFileInfo;
    SgScopeStatement* _currentFileScope;
    string _mainPathname;
    SgScopeStatement* _mainScope;
    Sg_File_Info* _mainFileInfo;
    SgSourceFile* _sourceFileNode;
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
    _mainFileInfo=NULL;
    _sourceFileNode=NULL;
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


// There is no way to tell ROSE that there is a new application file.
// Just use normal print statement to create this file.
//
void visitorTraversal::writeHeaderFile(SgProject* project)
{
    FILE *fp=NULL;
    string fname = strROSE_FFWDH;


    if ((fp = fopen(fname.c_str(), "w")) == 0)
    {
        printf("Error: Could not create header file.  aborting\n") ;
        exit(-1) ;
    }

    fprintf(fp, "#ifndef _ROSE_FFWD_H_\n");
    fprintf(fp, "#define _ROSE_FFWD_H_\n\n");



    // Add include files.  At the moment, I am adding all of the includes files,
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

    // add the list of globals, file statics and function statics.
    for (Rose_STL_Container<DbElement *>::iterator iter = _globalDeclList.begin();
         iter != _globalDeclList.end(); iter++)
    {
        DbElement* dbElement=(*iter);
        string new_name = dbElement->get_new_name();

        if (strcmp(new_name.c_str(), "___ffwd_p_") != 0)
        {
            string typeString = dbElement->get_type_str();
            fprintf(fp, "    %s %s", typeString.c_str(), new_name.c_str());

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
    }

    // Add the tool generated variables, which are used for initialization
    // of function static variables.
    for (Rose_STL_Container<TgvElement *>::iterator iter = _tgvList.begin();
         iter != _tgvList.end(); iter++)
    {
        TgvElement* tgvElement=(*iter);
        fprintf(fp, "    int %s;\n", tgvElement->get_name().c_str());
    }
    fprintf(fp, "};\n\n");

    // typedef struct __ffwd_s __ffwd_t;
    fprintf(fp, "typedef struct %s %s;\n\n", _structName.c_str(), strFFWDT.c_str());

    // if this is a C++ file, we need to specify C-linkage.
    fprintf(fp, "#ifdef __cplusplus\n");
    fprintf(fp, "extern \"C\" {\n");
    fprintf(fp, "#endif\n");

#ifdef __FFWD_DB_
    fprintf(fp, "int __mpiOcrMain(int argc,char **argv);\n");
    fprintf(fp, "ocrGuid_t __ffwd_init(void **ffwd_addr_p);\n");
    // these function are defined in mpilite/mpi_ocr.c.
    fprintf(fp, "extern u64 * __getGlobalDBAddr();\n");
    fprintf(fp, "extern void __ocrCheckStatus(u8 status, char * functionName);\n");
#else
    fprintf(fp, "void ffwd_init()\n");
#endif
    fprintf(fp, "#ifdef __cplusplus\n");
    fprintf(fp, "}\n");
    fprintf(fp, "#endif\n\n");
    fprintf(fp, "#endif\n");

    fclose(fp);
}


#if defined (__FFWD_DB_)
// write a global initialization routine
//
// ocrGuid_t  __ffwd_init(void** ffwd_addr_p){...}
    // ocrGuid_t ffwd_db_guid;
    // __ffwd_t * ffwd_addr_ptr=NULL;
    // u8 ffwd_status;
    // ffwd_status = ocrDbCreate( &ffwd_db_guid, (void **)&ffwd_addr_ptr, sizeof(__ffwd_t),
    //                            0, 0, (ocrInDbAllocator_t)0);
    // __ocrCheckStatus(ffwd_status, (char *)"ocrDbCreate");
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

    // ocrGuid_t ffwd_db_guid;
    string dbName("ffwd_db_guid");
    SgVariableDeclaration* ffwdDBGuid = buildVariableDeclaration(SgName(dbName), ocrGuidType,
                                                                   NULL, globalInitBB);
    appendStatement(ffwdDBGuid, globalInitBB);

    // __ffwd_t * ffwd_addr_ptr=NULL;
    SgTypedefDeclaration * ffwdTypedefDecl = buildTypedefDeclaration(strFFWDT,_structType,
                                                                    globalInitBB);
    SgTypedefType * ffwdType = ffwdTypedefDecl->get_type();
    string ptrName("ffwd_addr_ptr");
    SgPointerType * ptrType = buildPointerType(ffwdType);
    SgInitializer* ptrInitializer = buildAssignInitializer(buildLongIntVal(0),ptrType);
    SgVariableDeclaration* ffwdAddrPtr = buildVariableDeclaration(SgName(ptrName), ptrType,
                                                                  ptrInitializer, globalInitBB);
    appendStatement(ffwdAddrPtr, globalInitBB);


    // u8 ffwd_status;
    SgName statusName = SgName("ffwd_status");
    SgVariableDeclaration * ffwdStatus = buildVariableDeclaration(statusName,
                                                                 (SgType *)buildShortType() ,
                                                                 NULL, globalInitBB);
    appendStatement(ffwdStatus, globalInitBB);

    // ffwd_status = ocrDbCreate( &ffwd_db_guid, (void **)&ffwd_addr_ptr, sizeof(__ffwd_t),
    //                            0, 0, (ocrInDbAllocator_t)0);
    //
    SgExpression * addrOfDBGuid = buildAddressOfOp(buildVarRefExp(SgName(dbName),globalInitBB));
    SgPointerType * ptrPtrType = buildPointerType(buildPointerType(buildVoidType()));
    SgExpression * castExp = buildCastExp(buildAddressOfOp(buildVarRefExp(SgName(ptrName), globalInitBB)),
                                         ptrPtrType, SgCastExp::e_C_style_cast);

    SgSizeOfOp* sizeofExp = buildSizeOfOp(ffwdType);

    string declName("ocrInDbAllocator");
    SgClassDeclaration* decl = buildStructDeclaration(declName, _globalScope);
    //SgClassDefinition* def = buildClassDefinition(decl);
    SgType* declType = decl->get_type();
    string typedefName("ocrInDbAllocator_t");
    SgTypedefDeclaration* typedefDecl = buildTypedefDeclaration(typedefName, declType, _globalScope);
    SgTypedefType * typedefType = typedefDecl->get_type();
    SgExpression * castExp2 = buildCastExp(buildIntVal(0), typedefType, SgCastExp::e_C_style_cast);


    SgExprListExp * arg_list = buildExprListExp();
    appendExpression(arg_list,addrOfDBGuid);
    appendExpression(arg_list,castExp);
    appendExpression(arg_list,sizeofExp);
    appendExpression(arg_list,buildIntVal(0));
    appendExpression(arg_list,buildIntVal(0));
    appendExpression(arg_list,castExp2);

    SgFunctionCallExp * dbCreateCall = buildFunctionCallExp("ocrDbCreate", (SgType *)buildShortType(),
                                                       arg_list, globalInitBB);
    SgVarRefExp * ffwdStatusExp = buildVarRefExp(statusName, globalInitBB);
    SgExprStatement * assignOp3 = buildAssignStatement(ffwdStatusExp,dbCreateCall);
    appendStatement(assignOp3, globalInitBB);


#if 0
    // DEBUGGING!!!!
    // printf("__ffwd_init: ffwd_db_guid=%p, ffwd_addr_ptr=%p\n", ffwd_db_guid, ffwd_addr_ptr);
    string message = "__ffwd_init: ffwd_db_guid=%p, ffwd_addr_ptr=%p\\n";
    SgVarRefExp * ffwdGuidExp1 = buildVarRefExp(SgName("ffwd_db_guid"), globalInitBB);
    SgVarRefExp * ffwdAddrExp1 = buildVarRefExp(SgName("ffwd_addr_ptr"), globalInitBB);

    SgExprListExp* params = buildExprListExp(buildStringVal(message),
                                             ffwdGuidExp1, ffwdAddrExp1);
    SgExprStatement* printCall = buildFunctionCallStmt("printf", buildVoidType(),
                                                       params, globalInitBB);
    appendStatement(printCall, globalInitBB);
#endif


    // __ocrCheckStatus will exit if there is a failure.
    // __ocrCheckStatus(ffwd_status, (char *)"ocrDbCreate");
    SgVarRefExp * statusExp = buildVarRefExp(SgName("ffwd_status"), globalInitBB);
    SgStringVal * stringVal = buildStringVal("ocrDbCreate");
    SgPointerType * charPtrType = buildPointerType(buildCharType());
    SgExpression * castExp3 = buildCastExp(stringVal, charPtrType, SgCastExp::e_C_style_cast);
    SgExprListExp * arg1_list = buildExprListExp();
    appendExpression(arg1_list,statusExp);
    appendExpression(arg1_list,castExp3);
    SgExprStatement * checkStatusCall = buildFunctionCallStmt("__ocrCheckStatus", (SgType *)buildVoidType(),
                                                              arg1_list, globalInitBB);
    appendStatement(checkStatusCall, globalInitBB);

    // If there a function static variables, initialize the TGV which guards their initialization.
    for (Rose_STL_Container<TgvElement *>::iterator iter = _tgvList.begin();
         iter != _tgvList.end(); iter++)
    {
        //     ffwd_addr_ptr->tgv = 0;
        TgvElement* tgv=(*iter);
        string tgvName = tgv->get_name();
        SgVarRefExp* newNameExp = buildVarRefExp(tgvName, globalInitBB);
        SgVarRefExp* ptrNameExp = buildVarRefExp(ptrName, globalInitBB);
        SgExpression* arrowExp = buildArrowExp(ptrNameExp, newNameExp);
        SgExprStatement * assignOp = buildAssignStatement(arrowExp,
                                                          buildIntVal(0));
        appendStatement(assignOp, globalInitBB);
    }


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

    // call ffwd_filename_h_init() functions here
    if ( ! _headerFileList.empty() )
    {
        // call the initialization functions in the other files...
        if (_here == NULL)
            _here = getFirstStatement(_globalScope);

        for (Rose_STL_Container<HeaderFileElement*>::iterator iter=_headerFileList.begin();
             iter!=_headerFileList.end(); iter++)
        {
            // function call...
            // ffwd_<filename>_h_init(ffwd_addr_ptr);
            HeaderFileElement* element = (*iter);
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

            string pathname = element->get_header_file_name();
            if(strcmp(_mainPathname.c_str(), pathname.c_str()) != 0)
                decl->get_declarationModifier().get_storageModifier().setExtern();
            insertStatementBefore(_here, decl);
            _here = decl;
        }
    }  // not headerFileList empty

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


void visitorTraversal::insertHeaders(SourceFile* file)
{
    // at top, insert include file.  mpi.h should already be present.
    insertHeader(file->get_source_file_node(), strROSE_FFWDH, false, PreprocessingInfo::before);
}



// mpilite redefines the "main" function to be called "__mpiOcrMain".
SgFunctionDeclaration* findMpiOcrMain( SgProject* project)
{
    Rose_STL_Container<SgNode*> funcList = NodeQuery::querySubTree(project, V_SgFunctionDeclaration);
    for(Rose_STL_Container<SgNode*>::iterator i = funcList.begin(); i != funcList.end(); i++)
    {
        SgFunctionDeclaration* funcNode = isSgFunctionDeclaration(*i);

        if ( funcNode == funcNode->get_definingDeclaration())
        {
            if (isSgGlobal(isSgStatement(funcNode)->get_scope()) &&
                isSgFunctionDeclaration(funcNode)->get_name() == "__mpiOcrMain")
                return funcNode;
        }
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

    SgSizeOfOp* sizeofExp = buildSizeOfOp(typedefType);
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


void visitorTraversal::insertHeaders(SourceFile* file)
{
    SgScopeStatement* scope;

    // at top, insert #include "rose_ffwd.h"
    SgScopeStatement* fileScope=file->get_file_scope();
    if (fileScope != 0)
        scope = fileScope;
    else
        scope = _globalScope;

    insertHeader(file->get_source_file_node(), strROSE_FFWDH, false, PreprocessingInfo::before);

    // Create a pointer to the new data structure
    // __ffwd_t *ffwd_p;
    string typedefName = strFFWDT;
    SgTypedefDeclaration* typedefDecl = buildTypedefDeclaration(typedefName, _structType, scope);
    SgTypedefType * typedefType = typedefDecl->get_type();
    // this is inserted as text in the ffwd include file.

    string ptrName = strFFWDP;
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

// the list of global and static variables.
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


// the list of files that contain file static variables.
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

// the list of include files that contain file static variables.
HeaderFileElement* visitorTraversal::onHeaderFileList(string filename)
{
    if(_headerFileList.empty())
        return NULL;

    for (Rose_STL_Container<HeaderFileElement*>::iterator iter=_headerFileList.begin();
         iter!=_headerFileList.end(); iter++)
    {
        HeaderFileElement* element = (*iter);
        string name = element->get_header_file_name();
        if (strcmp(name.c_str(), filename.c_str()) == 0) {
            return element;
        }
    }
    return NULL;
}


// list of files that contain global or static variables.
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


// the list of functions that contains references to global or static variables.
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


// the list of tool generated variables (tgv).
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


// the list of include files used by the application.
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
    FileElement* element = NULL;
    SgBasicBlock* initBody = NULL;

    // find the file that this static belongs to.
    Sg_File_Info* fileInfo = dbElement->get_file_info();
    string filename = fileInfo->get_filename();
    SgScopeStatement* fileScope = dbElement->get_scope();

    string file_no_path = StringUtility::stripPathFromFileName(filename);
    size_t dot = file_no_path.find(".");
    string fname = file_no_path.substr(0,dot);
    string suffix = file_no_path.substr(dot);

    if ((strcmp(suffix.c_str(), ".h") == 0) ||
        (strcmp(suffix.c_str(), ".hpp") == 0) )
    {
        HeaderFileElement* hfElement=NULL;
        if ((hfElement = onHeaderFileList(filename)) == NULL)
        {
            // void ffwd_<filename>_h_init( __ffwd_t * __ffwd_p){...}
            SgTypedefDeclaration * ffwdTypedefDecl = buildTypedefDeclaration(strFFWDT,
                                                                             _structType,
                                                                             fileScope);

            SgTypedefType * ffwdType = ffwdTypedefDecl->get_type();
            SgPointerType * ptrType = buildPointerType(ffwdType);
            SgFunctionParameterList * paraList = buildFunctionParameterList();
            SgInitializedName * arg1 = buildInitializedName(strFFWDP, ptrType);
            appendArg(paraList, arg1);
            fname += "_h";
            SgName* initName = new SgName(strFFWD+strUnderscore+fname+
                                          strUnderscore+strINIT);
            SgFunctionDeclaration * init = buildDefiningFunctionDeclaration
                                           (*initName,(SgType*)buildVoidType(),paraList,
                                            fileScope);
            appendStatement(init,fileScope);

            initBody = init->get_definition()->get_body();
            HeaderFileElement* element = new HeaderFileElement(fileInfo, fileScope,
                                                               _currentFileInfo,_globalScope,
                                                               initBody, initName);
            _headerFileList.push_back(element);
        }
        else
        {
            initBody = hfElement->get_init_func();
        }
    }

    else if ((element = onLocalFileList(filename)) == NULL )
    {
        // void ffwd_<filename>_init( __ffwd_t * __ffwd_p){...}
        SgTypedefDeclaration * ffwdTypedefDecl = buildTypedefDeclaration(strFFWDT,
                                                                         _structType,
                                                                         fileScope);

        SgTypedefType * ffwdType = ffwdTypedefDecl->get_type();
        SgPointerType * ptrType = buildPointerType(ffwdType);
        SgFunctionParameterList * paraList = buildFunctionParameterList();
        SgInitializedName * arg1 = buildInitializedName(strFFWDP, ptrType);
        appendArg(paraList, arg1);
        SgName* initName = new SgName(strFFWD+strUnderscore+fname+
                                      strUnderscore+strINIT);
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
        if (dbElement->is_initialized()) {
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
            else {
                // initialize the field with the original value
                initStmt = buildAssignStatement(arrowExp,
                                                buildVarRefExp(dbElement->get_name(), scope));
            }
        }
        else {
            // initialize the field to zero
            if ( isKnownType(dbElement->get_base_type()))
            {
                if (dbElement->is_pointer()) {
                    initStmt = buildAssignStatement(arrowExp, buildUnsignedLongVal(0));
                }
                else {
                    initStmt = buildAssignStatement(arrowExp, buildInitVal(dbElement));
                }
            }
            else
            {
                // use buildOpaqueType() when type is unknown
                SgSizeOfOp* sizeofExp = buildSizeOfOp(buildOpaqueType(dbElement->get_type_str(),scope));
                SgPointerType * voidPtr = buildPointerType(buildVoidType());
                SgExpression * castExp = NULL;

                if (isSgPointerType(dbElement->get_node())){
                    castExp = buildCastExp(arrowExp, voidPtr, SgCastExp::e_C_style_cast);
                }
                else {
                    castExp = buildCastExp( buildAddressOfOp(arrowExp), voidPtr, SgCastExp::e_C_style_cast);
                }

                SgExprListExp * memsetParam  = buildExprListExp (castExp, buildIntVal(0), sizeofExp);
                SgFunctionCallExp* memsetExp = buildFunctionCallExp(SgName("memset"), buildIntType(),
                                                                    memsetParam, scope);
                initStmt = buildExprStatement(memsetExp);
            }
        }
    }
    else  // this is an array
    {
        // use buildOpaqueType() when type is unknown
        SgSizeOfOp* sizeofExp = buildSizeOfOp(buildOpaqueType(dbElement->get_type_str(),scope));
        SgExpression * mulOp = buildMultiplyOp(buildIntVal(dbElement->get_num_elements()),
                                               sizeofExp);
        SgExprListExp * param = NULL;

        if (dbElement->is_initialized())
        {
            // memcpy(__ffwd_p->_orig, orig, numElements*sizeof(type));
            SgVarRefExp * nameExp = buildVarRefExp(dbElement->get_name(), scope);
            param = buildExprListExp(arrowExp, nameExp, mulOp);
            SgFunctionCallExp* memcpyExp = buildFunctionCallExp(SgName("memcpy"), buildIntType(),
                                                                param, scope);
            initStmt = buildExprStatement(memcpyExp);
        }
        else
        {
            // memcpy(__ffwd_p->_orig, (void *)0, numElements*sizeof(type));
            SgPointerType * voidPtr = buildPointerType(buildVoidType());
            SgExpression * castExp = buildCastExp(arrowExp, voidPtr, SgCastExp::e_C_style_cast);
            param = buildExprListExp(castExp, buildIntVal(0), mulOp);
            SgFunctionCallExp* memsetExp = buildFunctionCallExp(SgName("memset"), buildIntType(),
                                                                param, scope);
            initStmt = buildExprStatement(memsetExp);
        }
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
    SgVariableSymbol* variableSymbol = isSgVariableSymbol(variableName->search_for_symbol_from_symbol_table());
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


void visitorTraversal::findMainFn()
{
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
        _mainFileInfo = mainDecl->get_file_info();
        _mainPathname = _mainFileInfo->get_filename();
        _sourceFileNode = getEnclosingSourceFile(mainDecl);
    }
}



void visitorTraversal::atTraversalStart()
{
    _currentPathname.clear();
    _currentFileScope=NULL;
}



// Visit each node in the AST.
void visitorTraversal::visit(SgNode* node)
{
    if (_currentPathname.empty())
    {
        // save current file name - this would be better done in atTraversalStart().
        _currentFileInfo = node->get_file_info();
        _currentPathname = _currentFileInfo->get_filenameString();
        //printf("visit: processing %s\n", _currentPathname.c_str());
    }

    // find the file that contains main()
    if (_mainScope == NULL)
    {
        findMainFn();
    }

    // find the included files.
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
                    string str = (*iter)->getString();
                    if ( ! onIncList(str)) {
                        if (str.find("rose_ffwd.h") == string::npos)
                        {
                            IncElement * incElement = new IncElement(str);
                            _incList.push_back(incElement);
                        }
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
                    _currentFileScope = variableName->get_scope();
                    SourceFile * source = new SourceFile(_currentPathname,
                                                         getEnclosingSourceFile(node),
                                                         _currentFileScope);
                     _fileList.push_back(source);
                    insertHeaders(source);
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
                // declaration must be in a different file
                // create a new DB entry if user's file
                if (isNodeDefinedInUserLocation(varSymbol) == true &&
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
                    SourceFile * source = new SourceFile(_currentPathname,
                                                         getEnclosingSourceFile(node),
                                                         varScope);
                    _fileList.push_back(source);
                    insertHeaders(source);
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
                        element->set_tgv(tgvElement);
                        _tgvList.push_back(tgvElement);
                    }
                }
            }
        }
    } // isSgVarRefExp

#ifdef __FFWD_DB_
    // check that this application actually uses MPI.  It is not clear that this
    // error checking is helpful.
    //
    if (isSgFunctionCallExp(node) != NULL)
    {
        SgFunctionCallExp * callExp = isSgFunctionCallExp(node);
        SgFunctionSymbol * funcSym = callExp->getAssociatedFunctionSymbol();
        if (funcSym != NULL)
        {
            SgName funcName = funcSym->get_name();
            SgScopeStatement * scope = funcSym->get_declaration()->get_scope();
            if (strncmp(funcName.getString().c_str(), "MPI", 3) == 0)
            {
                if ((strcmp(funcName.getString().c_str(), "MPI_Init") == 0) ||
                    (strcmp(funcName.getString().c_str(), "MPIlite_Init") == 0))
                    _status=E_MPI_FOUND;

            }
        }
    } // isSgFunctionCallExp
#endif
}



// atTraversalEnd() is called at the end of every source file that is processed.
void visitorTraversal::atTraversalEnd()
{
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

            // __ffwd_t * const __ffwd_p = (__ffwd_t * const )(__getGlobalDBAddr());
            SgTypedefDeclaration * ffwdTypedefDecl = buildTypedefDeclaration(strFFWDT,
                                                                             _structType,
                                                                             funcBB);
            SgTypedefType * ffwdType = ffwdTypedefDecl->get_type();
            SgModifierType * modType = buildModifierType(buildPointerType(ffwdType));
            modType->get_typeModifier().get_constVolatileModifier().setConst();

            SgFunctionCallExp * funcCall = buildFunctionCallExp(SgName("__getGlobalDBAddr"),
                                                               buildUnsignedLongLongType(),
                                                               buildExprListExp(), funcBB);
            SgExpression * castPtrExp = buildCastExp(funcCall, modType,
                                                     SgCastExp::e_C_style_cast);
            SgAssignInitializer * initCastPtrExp = buildAssignInitializer(castPtrExp, modType);
            SgVariableDeclaration* ffwdAddrPtr = buildVariableDeclaration(SgName(strFFWDP),
                                                                          modType, initCastPtrExp,
                                                                          funcBB);
            insertStatementBefore(firstStmt, ffwdAddrPtr);



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


    // even if no globals were detected in the file that contains main,
    // we still need to insert the rose_ffwd.h file, because ffwd_init()
    // will be inserted in the main file.
    //
    if ( ! onFileList(_mainPathname))
    {
        SourceFile * source = new SourceFile(_mainPathname,_sourceFileNode, _globalScope);
        _fileList.push_back(source);
        insertHeaders(source);
    }


    // initialize the global elements which were initialized
    // in their declaration in a file specific init file.
    // initialize to zero the global elements which were not
    // initialized in their original declaration.

    for (Rose_STL_Container<DbElement *>::iterator iter = _globalDeclList.begin();
         iter != _globalDeclList.end(); iter++)
    {
        DbElement* dbElement = *iter;
        if ( isSgGlobal(dbElement->get_scope()))
        {   // global or file static
            if ( ! dbElement->init_created())
            {
                SgBasicBlock* initBB = getLocalInitFunction(dbElement);
                if (initBB != NULL)
                {
                    SgVarRefExp* structPtrExp = buildVarRefExp(_ptrName, _globalScope);
                    SgExprStatement* stmt = createInitStmt(dbElement, structPtrExp);
                    if (stmt != NULL) {
                        appendStatement(stmt, initBB);
                        dbElement->set_init_created(true);
                    }
                }
            }
        }
        else
        {   //function static
            // if (__ffwd_p->tgv == 0) {
            //     __ffwd_p->fs = fs;
            //     __ffwd_p->tgv = 1;  }
            if ( ! dbElement->init_created())
            {
                TgvElement* tgv = dbElement->get_tgv();
                if (tgv != NULL)
                {
                    SgScopeStatement * scope = dbElement->get_scope();
                    SgBasicBlock * trueBB = buildBasicBlock();

                    //     __ffwd_p->fs = fs;
                    SgVarRefExp* structPtrExp = buildVarRefExp(_ptrName, scope);
                    SgExprStatement* initStmt = createInitStmt(dbElement, structPtrExp);
                    if (initStmt != NULL)
                    {
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
                        insertStatementAfter(dbElement->get_name()->get_declaration(), ifStmt, true);
                    }
                }
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

    // for debugging!
    //generateDOT(*project);

    // Build the traversal object
    visitorTraversal exampleTraversal(project);

    // Call the traversal function (member function of AstSimpleProcessing)
    // starting at the project node of the AST, using a preorder traversal.
    exampleTraversal.traverseInputFiles(project,preorder);

    // Check for issues.
    exampleTraversal.errorCheck();

    // Write the header file.
    exampleTraversal.writeHeaderFile(project);
    exampleTraversal.writeInitFunction(project);


    // run all tests
    AstTests::runAllTests(project);

    // Generate source code from AST and call the vendor's compiler
    int res = backend(project);

    return (res);
}



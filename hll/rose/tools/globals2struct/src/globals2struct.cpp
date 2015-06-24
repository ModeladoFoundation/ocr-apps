/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */


// The ROSE tool globals2struct finds all globals and
// file statics, and
//    1. Place them in a structure
//    2. Initialize the globals and statics which were originally
//       initialized in the declaration.
//    3. Update the references to the global and file statics to
//       reflect their new location.
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
    void insertHeaders(SgProject * project);
    void errorCheck();
    DbElement* onGlobalDeclList(SgVariableSymbol* sym);
    FileElement* onLocalFileList(string filename);
    SgBasicBlock* createLocalInitFunction(DbElement* dbElement);
    SgExprStatement* createInitStmt(DbElement* dbElement, SgBasicBlock* initBB,
                                    SgVarRefExp* structPtrExp);


    private:
    Rose_STL_Container<DbElement *> _globalDeclList;
    Rose_STL_Container<FileElement *> _localFileList;
    Rose_STL_Container<SourceFile *> _fileList;
    SgScopeStatement* _globalScope;
    string _structName;
    SgClassDeclaration* _structDecl;
    SgClassDefinition* _structDef;
    SgType* _structType;
    string _ptrName;
    bool _isCreated;
    string _currentPathname;
    SgScopeStatement* _currentScope;
    string _mainPathname;
    SgScopeStatement* _mainScope;
    SgStatement * _here;
};


visitorTraversal::visitorTraversal(SgProject* project)
{
    _globalScope=getFirstGlobalScope(project);
    _structName = strUnderscore + strUnderscore + strFFWD2 + strS;
    _structDecl = buildStructDeclaration(_structName, _globalScope);
    _structDef = buildClassDefinition(_structDecl);
    _structType = _structDecl->get_type();
    _ptrName = strFFWD2 + strP;
    _isCreated=false;
    _mainPathname.clear();
    _mainScope=NULL;
    _here=NULL;
}


void visitorTraversal::writeHeaderFile(SgProject* project)
{
    FILE *fp=NULL;
    string fname = strFFWD + ".h";


    if ((fp = fopen(fname.c_str(), "w")) == 0)
    {
        printf("Error: Could not create header file.  aborting\n") ;
        exit(-1) ;
    }

    fprintf(fp, "#ifndef _FFWD_H_\n");
    fprintf(fp, "#define _FFWD_H_\n\n");


    // #include <string.h>  (for memcpy())
    fprintf(fp, "#include <string.h>\n");

    // struct __ffwd2_s {...};
    fprintf(fp, "struct %s\n{\n", _structName.c_str());


    for (Rose_STL_Container<DbElement *>::iterator iter = _globalDeclList.begin();
         iter != _globalDeclList.end(); iter++)
    {
        DbElement* dbElement=(*iter);
        string typeString = get_type_string(dbElement->get_base_type());
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
    fprintf(fp, "};\n\n");

    // typedef struct __ffwd2_s __ffwd2_t;
    string typedefName = strUnderscore + strUnderscore + strFFWD2 + strT;
    fprintf(fp, "typedef struct %s %s;\n\n", _structName.c_str(), typedefName.c_str());

    fprintf(fp, "#endif\n");

    fclose(fp);
}


// write a global initialization routine
void visitorTraversal::writeInitFunction(SgProject * project)
{
    // void ffwd2_init(){...}
    const SgName* globalInitName = new SgName(strFFWD2+strUnderscore+strINIT);
    SgFunctionParameterList * paraList = buildFunctionParameterList();
    SgFunctionDeclaration * globalInit = buildDefiningFunctionDeclaration
                                         (*globalInitName,(SgType*)buildVoidType(),
                                          paraList,_globalScope);
    appendStatement(globalInit, _globalScope);

    // create the body:
    SgBasicBlock* globalInitBB = globalInit->get_definition()->get_body();

    // allocate some memory
    // ffwd2_p = ((__ffwd2_t *)(malloc((sizeof(__ffwd2_t)))));
    string typedefName = strUnderscore + strUnderscore + strFFWD2 + strT;
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

    // memset(ffwd2_p, 0, sizeof(__ffwd2_t));
    SgExprListExp* memsetParam = buildExprListExp(buildVarRefExp(_ptrName, _globalScope), buildIntVal(0), deepCopy(sizeofExp));
    SgExpression * memsetExp = buildFunctionCallExp(SgName("memset"),  buildVoidType(), memsetParam, _globalScope);
    SgExprStatement * memsetStmt = buildExprStatement(memsetExp);
    appendStatement(memsetStmt, globalInitBB);


    if ( ! _localFileList.empty() )
    {
        // call the initialization functions in the other files...
        //SgStatement * prevDecl = getFirstStatement(_globalScope);
        ROSE_ASSERT(_here != NULL);

        for (Rose_STL_Container<FileElement*>::iterator iter=_localFileList.begin();
             iter!=_localFileList.end(); iter++)
        {
            // ffwd2_<filename>_init();
            FileElement* element = (*iter);
            SgName* name = element->get_init_name();
            SgExprListExp * parameters = buildExprListExp();
            SgExprStatement * funcCall = buildFunctionCallStmt(*name, (SgType *)buildVoidType(),
                                                               parameters, _globalScope);
            appendStatement(funcCall, globalInitBB);

            // external declarations...
            // extern void ffwd2_<filename>_init();
            SgFunctionParameterList * parameterList = buildFunctionParameterList();
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


void visitorTraversal::errorCheck()
{
    if (_globalDeclList.empty())
    {
        printf("Error: No global or function static variables were found.  This tool is specifically for applications with global or file static variables.  Stopping.\n");
        exit(1);
    }
}



void visitorTraversal::insertHeaders(SgProject* project)
{
    for (Rose_STL_Container<SourceFile*>::iterator iter=_fileList.begin();
         iter!=_fileList.end(); iter++)
    {
        SourceFile* file = (*iter);

        // at top, insert #include "ffwd.h"
        SgScopeStatement* fileScope=file->get_file_scope();
        insertHeader("ffwd.h",PreprocessingInfo::after,false,fileScope);

        // Create a pointer to the new data structure
        // __ffwd2_t *ffwd2_p;
        string typedefName = strUnderscore + strUnderscore + strFFWD2 + strT;
        SgTypedefDeclaration* typedefDecl = buildTypedefDeclaration(typedefName, _structType, fileScope);
        SgTypedefType * typedefType = typedefDecl->get_type();
        // this is inserted as text in the ffwd include file.

        string ptrName = strFFWD2 + strP;
        SgVarRefExp* ptrNameExp = buildVarRefExp(SgName(ptrName), fileScope);
        SgPointerType* ptrType = buildPointerType(typedefType);
        SgVariableDeclaration* structVarPtr = buildVariableDeclaration(SgName(ptrName), ptrType, NULL, fileScope);
        // set TLS to add __thread
        structVarPtr->get_declarationModifier().get_storageModifier().set_thread_local_storage(true);

        string pathname = file->get_path_name();
        if(strcmp(_mainPathname.c_str(), pathname.c_str()) != 0)
            structVarPtr->get_declarationModifier().get_storageModifier().setExtern();
        insertStatementAfter(getFirstStatement(fileScope), structVarPtr);
    }
}



DbElement* visitorTraversal::onGlobalDeclList(SgVariableSymbol* sym)
{
    ROSE_ASSERT(isSgGlobal(sym->get_declaration()->get_scope()));

    if(_globalDeclList.empty())
        return NULL;

    for (Rose_STL_Container<DbElement*>::iterator iter=_globalDeclList.begin();
         iter!=_globalDeclList.end(); iter++)
    {
        DbElement* element = *iter;
        SgVariableSymbol* symbol = isSgVariableSymbol(element->get_symbol());
        //printf("Debug: onGlobalDeclList: sym = %p, symbol = %p\n", sym, symbol);
        if (sym == symbol)
            return element;
    }

    // look for externals
    SgInitializedName * iName = sym->get_declaration();
    string sName = iName->get_name().getString();

    for (Rose_STL_Container<DbElement*>::iterator iter=_globalDeclList.begin();
         iter!=_globalDeclList.end(); iter++)
    {
        DbElement* element = *iter;
        string str = element->get_name()->get_name().getString();

        if (element->get_scope() == _globalScope)
            if (strcmp(sName.c_str(), str.c_str()) == 0) {
                //printf("Debug: onGlobalDeclList: found symbol %s\n", sName.c_str());
                return element;
            }
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
            //printf("found %s on localFileList\n", name.c_str());
            return element;
        }
    }
    return NULL;
}


// Create a file-specific init function.
SgBasicBlock* visitorTraversal::createLocalInitFunction(DbElement* dbElement)
{
    // find the file that this static belongs to.
    Sg_File_Info* fileInfo = dbElement->get_file_info();
    string filename = fileInfo->get_filename();
    SgScopeStatement* fileScope = dbElement->get_scope();
    FileElement* element = NULL;
    SgBasicBlock* initBody = NULL;

    if ((element = onLocalFileList(filename)) == NULL)
    {
        // void ffwd2_<filename>_init(){...}
        string file_no_path = StringUtility::stripPathFromFileName(filename);
        std::size_t dot = file_no_path.find(".");
        std::string fname = file_no_path.substr(0,dot);
        SgName* initName = new SgName(strFFWD2+strUnderscore+fname+
                                      strUnderscore+strINIT);
        SgFunctionParameterList * paraList = buildFunctionParameterList();
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
SgExprStatement* visitorTraversal::createInitStmt(DbElement* dbElement, SgBasicBlock* initBB,
                                                  SgVarRefExp* structPtrExp)
{
    string newName = dbElement->get_new_name();
    SgVarRefExp* newNameExp = buildVarRefExp(newName, _globalScope);
    SgExpression* arrowExp = buildArrowExp(structPtrExp, newNameExp);

    SgExprStatement* initStmt= NULL;
    if (dbElement->get_num_dimensions() == 0)
    {
        // initialize the field with the original value
        initStmt = buildAssignStatement(arrowExp,
                                        buildVarRefExp(dbElement->get_name(), initBB));
    }
    else {
        // Call memcpy().
        SgVarRefExp* nameExp = buildVarRefExp(dbElement->get_name(), _globalScope);
        SgExprListExp* memcpyParam = buildExprListExp(arrowExp, nameExp,
                                                      buildIntVal(dbElement->get_total_size()));
        SgFunctionCallExp* memcpyExp = buildFunctionCallExp(SgName("memcpy"), buildIntType(),
                                                            memcpyParam, _globalScope);
        initStmt = buildExprStatement(memcpyExp);
    }
    return initStmt;
}


void visitorTraversal::atTraversalStart()
{
    _currentPathname.clear();
    _currentScope=NULL;
}



// Visit each node in the AST.
void visitorTraversal::visit(SgNode* node)
{
    if (_currentPathname.empty())
    {
        // save current file name - this would be better done in atTraversalStart().
        _currentPathname = node->get_file_info()->get_filenameString();
    }

    if (isSgVariableDeclaration(node) != NULL)
    {
        SgVariableDeclaration * variableDeclaration = isSgVariableDeclaration(node);
        SgInitializedNamePtrList::const_iterator i = variableDeclaration->get_variables().begin();
        ROSE_ASSERT(i != variableDeclaration->get_variables().end());

        do {
            SgInitializedName* variableName = isSgInitializedName(*i);
            if (isSgGlobal(variableName->get_scope()))
            {
                //printNode(node);
                SgType* variableType = variableName->get_type();
                //printf("Found Declaration: %s\n",variableName->get_name().str());
                SgVariableSymbol* variableSymbol = isSgVariableSymbol(variableName->get_symbol_from_symbol_table ());
                ROSE_ASSERT (variableSymbol != NULL);
                if ( ! onGlobalDeclList(variableSymbol))
                {
                    // create a new DB entry
                    DbElement * dbElement = new DbElement(variableSymbol, isSgNode(*i));
                    _globalDeclList.push_back(dbElement);

                 }
                if (_currentScope == NULL)
                {
                    _currentScope = isSgGlobal(variableName->get_scope());
                    SourceFile * source = new SourceFile(_currentPathname, _currentScope);
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
        if ( isSgGlobal(varScope))
        {
            // global reference
            DbElement* element = onGlobalDeclList(varSymbol);
            if (element == NULL)
            {
                // declaration must be in a different file
                // create a new DB entry if user's file
                if ( isNodeInUserLocation(node) == true) {
                    element = new DbElement(varSymbol, node);
                    _globalDeclList.push_back(element);
                }
            }

            if (element != NULL)
            {
                string newName = element->get_new_name();
                SgVarRefExp* newNameExp = buildVarRefExp(newName, varScope);
                SgVarRefExp* ptrNameExp = buildVarRefExp(_ptrName, element->get_scope());
                SgExpression* arrowExp = buildArrowExp(ptrNameExp, newNameExp);

                SgNode * parent = varRefExp->get_parent();
                if (isSgPntrArrRefExp(parent) != NULL)
                {
                    SgPntrArrRefExp * pntrArrRefExp = isSgPntrArrRefExp(parent);
                    pntrArrRefExp->set_lhs_operand(arrowExp);
                    arrowExp->set_parent(pntrArrRefExp);
                }
                else
                {
                    SgBinaryOp * binaryOp = isSgBinaryOp(parent);
                    binaryOp->set_lhs_operand(arrowExp);
                    arrowExp->set_parent(binaryOp);
                }
            }
        }
    }
}


// atTraversalEnd() is called at the end of every source file that is processed.
void visitorTraversal::atTraversalEnd()
{
    if ( _globalDeclList.empty())
        return;

    if (_mainScope == NULL) {
        SgFunctionDeclaration* mainDecl = findMain(getFirstGlobalScope(getProject()));
        SgFunctionDefinition* mainDef = mainDecl->get_definition();
        ROSE_ASSERT(mainDef != NULL);
        _mainScope = mainDef->get_body();
        _mainPathname = mainDecl->get_file_info()->get_filename();
    }
    ROSE_ASSERT(_globalScope != NULL);

    // initialize the global elements which were initialized
    // in their declaration in a file specific init file

    string ptrName = strFFWD2 + strP;
    SgVarRefExp* structPtrExp = buildVarRefExp(_ptrName, _globalScope);
    for (Rose_STL_Container<DbElement *>::iterator iter = _globalDeclList.begin();
         iter != _globalDeclList.end(); iter++)
    {
        DbElement* dbElement = *iter;
        if (dbElement->is_initialized() && ! dbElement->init_created())
        {
            SgBasicBlock* initBB = createLocalInitFunction(dbElement);
            ROSE_ASSERT(initBB != NULL);
            SgExprStatement* stmt = createInitStmt(dbElement, initBB, structPtrExp);
            ROSE_ASSERT(stmt != NULL);
            appendStatement(stmt, initBB);
            dbElement->set_init_created(true);
        }
    }  // globalist


    //printf("atTraversalEnd(): currentPathname=%s, mainPathname=%s\n",
    //       _currentPathname.c_str(), _mainPathname.c_str());

    if (strcmp(_currentPathname.c_str(), _mainPathname.c_str())==0)
    {

        // forward declarations...
        // void ffwd2_init();
        SgName* globalInitName = new SgName(strFFWD2+strUnderscore+strINIT);
        SgFunctionParameterList * parameterList = buildFunctionParameterList();
        SgFunctionDeclaration * forwardDecl = buildNondefiningFunctionDeclaration
                                              (*globalInitName,(SgType*)buildVoidType(),
                                               parameterList,_globalScope);
        insertStatementBefore(getFirstStatement(_globalScope), forwardDecl);
        _here=forwardDecl;

        // main calls the initialization function
        // ffwd2_init();
        SgExprListExp * parameters = buildExprListExp();
        SgExprStatement * globalInitCall = buildFunctionCallStmt(*globalInitName,(SgType *)buildVoidType(),
                                                                 parameters, _mainScope);
        insertStatementBefore(getFirstStatement(_mainScope), globalInitCall);
    }  // is main file
    //printf ("Traversal ends here. \n");
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
    printf("Traversing the input files\n");
    exampleTraversal.traverseInputFiles(project,preorder);

    // Check for issues.
    exampleTraversal.errorCheck();

    // Write the header file.
    printf("Writing the header file and init function\n");
    exampleTraversal.insertHeaders(project);
    exampleTraversal.writeHeaderFile(project);
    exampleTraversal.writeInitFunction(project);


    // run all tests
    printf("Running all tests\n");
    printf("Emitting a ton of warnings during AST Testing is one of ROSE's charms.\n\n");
    AstTests::runAllTests(project);

    // Generate source code from AST and call the vendor's compiler
    printf("\nCalling the backend\n");
    int res = backend(project);

    printf("Done!\n");
    return (res);
}



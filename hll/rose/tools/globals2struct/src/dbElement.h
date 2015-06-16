#ifndef _DB_ELEMENT_H
#define _DB_ELEMENT_H

#include "rose.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;
using namespace SageBuilder;
using namespace SageInterface;

const string strFFWD2("ffwd2");
const string strFFWD("ffwd");
const string strDB("db");
const string strINIT("init");
const string strT("_t");
const string strP("_p");
const string strS("_s");
const string strUnderscore("_");



// forward declarations
size_t sizeOfType(const SgType *type);
static void printNode(SgNode* node);
string create_new_name(SgInitializedName * name);




// This class stores information about all source files as they are processed.
class SourceFile
{
  public:
    SourceFile(string pathname, SgScopeStatement* fileScope)
    {
        _pathname = pathname;
        _fileScope = fileScope;
    }
    ~SourceFile() {};
    string get_path_name() { return _pathname; }
    SgScopeStatement * get_file_scope() { return _fileScope; }

  private:
    string _pathname;
    SgScopeStatement * _fileScope;
};


// This class stores information about the files that have initialized
// global or file static variables.
class FileElement
{
  public:
    FileElement(Sg_File_Info* fileInfo, SgBasicBlock* initFunc,SgName* initName,
                         SgScopeStatement* fileScope );
    ~FileElement() {};
    Sg_File_Info* get_file_info() { return _fileInfo; }
    string get_file_name() { return _fileInfo->get_filename(); }
    SgBasicBlock* get_init_func() { return _initFunc; }
    SgName* get_init_name() { return _initName; }
    SgScopeStatement* get_file_scope() { return _fileScope; }

  private:
    Sg_File_Info* _fileInfo;
    SgBasicBlock* _initFunc;
    SgName* _initName;
    SgScopeStatement* _fileScope;
};


// constructor
FileElement::FileElement(Sg_File_Info* fileInfo, SgBasicBlock* initFunc, SgName* initName,
                         SgScopeStatement* fileScope)
{
    _fileInfo = fileInfo;
    _initFunc = initFunc;
    _initName = initName;
    _fileScope = fileScope;
}


// This class stores information about global and file static variables.
class DbElement
{
  public:
    DbElement(SgVariableSymbol* sym, SgNode* node);
    ~DbElement(){};
    SgNode* get_node() { return _node; }
    SgVariableSymbol* get_symbol() { return _sym; }
    SgType* get_base_type() { return _baseType; }
    SgInitializedName* get_name() { return _name; }
    string get_new_name() { return _newName; }
    SgScopeStatement* get_scope() { return _scope; }
    SgDeclarationStatement* get_declaration() { return _decl; }
    bool is_initialized() { return _isInitialized; }
    bool is_static() { return _isStatic; }
    bool entry_created() { return _entryCreated; }
    void set_entry_created(bool b) { _entryCreated = b; }
    bool init_created() { return _initCreated; }
    void set_init_created(bool b) { _initCreated = b; }
    Sg_File_Info* get_file_info() { return _fileInfo; }
    int get_num_dimensions() { return _numDimensions; }
    int get_num_elements() { return _numElements; }
    int get_total_size() { return _totalSize; }
    Rose_STL_Container<SgExpression*> get_dim_expr_ptr_list() { return _dimExprPtrList; }
    void get_declarators(SgArrayType* arrayType );
    //int get_first_array_dimension(SgArrayType arrayType);
    //void get_empty_declarators(SgArrayType* arrayType);


  private:
    SgNode* _node;
    SgVariableSymbol* _sym;
    Sg_File_Info* _fileInfo;
    SgType* _baseType;
    SgInitializedName* _name;
    string _newName;
    SgScopeStatement* _scope;
    SgDeclarationStatement* _decl;
    bool _isInitialized;
    bool _isStatic;
    bool _entryCreated;
    bool _initCreated;
    int _totalSize;
    int _numElements;
    int _numDimensions;
    Rose_STL_Container<SgExpression*> _dimExprPtrList;
};

// constructor
DbElement::DbElement(SgVariableSymbol* sym, SgNode* node)
{
    _node = node;
    _sym = sym;
    _name = sym->get_declaration();
    ROSE_ASSERT (_name != NULL);
    _newName = create_new_name(_name);
    _scope = _name->get_scope();

    ROSE_ASSERT (_scope != NULL);
    SgDeclarationStatement* _decl = sym->get_declaration()->get_declaration();
    ROSE_ASSERT(_decl);

    _fileInfo=isSgNode(_decl)->get_file_info();
    _baseType= _name->get_type()->findBaseType();

    _numDimensions=0;
    _numElements=0;

    SgInitializer* initNode = _name->get_initializer();
    if (initNode != NULL)
        _isInitialized=true;
    else
        _isInitialized=false;

    _isStatic = isStatic(_decl);

    _entryCreated=false;
    _initCreated=false;

    if (isSgArrayType(_name->get_type()))
    {
        SgArrayType* arrayType = isSgArrayType(_name->get_type());
        get_declarators(arrayType);
        _numDimensions = _dimExprPtrList.size();
    }
    else
        _numElements=1;


    _totalSize = _numElements * sizeOfType(_baseType);
}


// Create a new variable name for the global and file static variables.
// Globals are prepending with an underscore.  File statics are prepended
// with an underscore and the main part of the file name.
//
string create_new_name( SgInitializedName * name)
{
    static int num=0;
    char buf[256];
    string newName;

    if (isStatic(name->get_declaration()))
    {
        string filename =  name->get_file_info()->get_filename();
        string file_no_path = StringUtility::stripPathFromFileName(filename);
        std::size_t dot = file_no_path.find(".");
        std::string fname = file_no_path.substr(0,dot);
        std::string tstr = strUnderscore + fname + strUnderscore +
                           name->get_name().str() + strUnderscore;

        sprintf(buf, "%s%d", tstr.c_str(),num);
        newName =  string(buf);
    }
    else
    {
        std::string tstr = strUnderscore + name->get_name().str() + strUnderscore;
        sprintf(buf, "%s%d", tstr.c_str(),num);
        newName = string(buf);
    }

    num++;
    printf("create_new_name(): newname=%s\n", newName.c_str());

    return newName;
}






// get_dim_info() does not work for C++.  Use get_index() to get the indices of an array.
void DbElement::get_declarators(SgArrayType* arrayType )
{
      SgExpression* indexExp =  arrayType->get_index();
      if(indexExp)
      {   // this is the dimension for the array
          // int a[] = {0, 1, 2};  get_index() returns SgNullExpression
          // int b[3];             get_index() returns 3 in form of an SgValueExp.
          // int c[i];             allowed in C; get_index() returns an expression

          if (isSgNullExpression(indexExp)) {
              printf("Error: [] is not supported.\n");
              exit(1);
#if 0
              printf("get_declarators:: [] WIP\n");
              int index = get_first_array_dimension(arrayType);
              printf("get_declarators:: index=%d\n", index);
              if (_numElements == 0)
                  _numElements = index;
              else
                  _numElements *= index;

              // This should be a SgValueExp, but the function to build this
              // is TBD.  Try to use an SgExpression.  If that does not work,
              // try to write the buildSgValueExp() function.
              //SgValueExp* valueExp = buildSgValueExp(buildIntVal(index));
              //ROSE_ASSERT(valueExp != NULL);
              //SgExpression* indexExp = isSgExpression(valueExp);
              SgExpression* indexExp = isSgExpression(buildIntVal(index));
              ROSE_ASSERT(indexExp != NULL);
              _dimExprPtrList.push_back(indexExp);
#endif
          }

          else if (isSgValueExp(indexExp)) {
              SgExpression* copyExp = deepCopy(indexExp);
              _dimExprPtrList.push_back(copyExp);
              SgValueExp* valueExp = isSgValueExp(indexExp);
              unsigned long long index = getIntegerConstantValue(valueExp);
              if (_numElements == 0)
                  _numElements = index;
              else
                  _numElements *= index;
          }
          else
          {
              printf("Error: unsupported array dimension type.\n");
              exit(1);
          }
      }
      SgArrayType* arraybase = isSgArrayType(arrayType->get_base_type());
      if (arraybase)
         get_declarators(arraybase);
}

#if 0
// The SgAggregateInitializer support does not seem to be working correctly.
// When I look at the AST for an array with an empty declaration, it clearly
// shows that the SgInitializedName initptr points to the SgAggregateInitializer.
// However, when I execute the code below, the aggregate initializer == NULL.
// I am abandoning this for now.
// if the first array dimension is non-static, compute it.
int DbElement::get_first_array_dimension(SgArrayType arrayType)
{
    printf("get_first_array_dimension: entering\n");
    int first;

    if (_isInitialized)
    {
        //SgAggregateInitializer* aggInit = isSgAggregateInitializer(_name->get_initializer());
        SgAggregateInitializer* aggInit = isSgAggregateInitializer(_name->get_initptr());
        ROSE_ASSERT(aggInit != NULL);
        SgExprListExp* initExprList = aggInit->get_initializers();
        ROSE_ASSERT(initExprList != NULL);
        SgExpressionPtrList exprPtrList = initExprList->get_expressions();
        size_t size = exprPtrList.size();
        printf("get_first_array_dimension: size = %d\n", size);

        int count = 0;
        size_t sz;
        for(int i=0; i<size; i++)
        {
            SgExpression* expr = isSgExpression(exprPtrList[i]);
            SgType * type = expr->get_type();
            sz=sizeOfType(type);
            printf("get_first_array_dimension: sizeOfType = %d\n", sizeOfType(type));
            count++;
        }
        first = size/sz;
        printf("get_first_array_dimension: count = %d\n", count);

    }
    printf("get_first_array_dimension: returning first element = %d\n", first);
    return first;
}
#endif

#if 0
    // use std::vector<SgExpression*> SageInterface::get_C_array_dimensions(const SgArrayType& arrtype)
    // to handle implicit first dimension for array initializers
    // for something like
    //      int p[][2][3] = {{{ 1, 2, 3 }, { 4, 5, 6 }}}
    //  we can calculate the first dimension as
    //      sizeof( p ) / ( sizeof( int ) * 2 * 3 )

void DbElement::get_empty_declarator(SgArrayType* arrayType)
{
    Rose_STL_Container<SgExpression*> indices = get_C_array_dimensions(arrayType);
    for (Rose_STL_Container<SgExpression*>::iterator it = indices.begin(); it != indices.end(); it++)
    {
        if (isSgNullExpression(*it))
        {
            int value = get_first_array_dimension


        if (isSgValueExp(*it)) {
            SgExpression* indexExp = isSgValueExp(*it);
            SgExpression* copyExp = deepCopy(indexExp);
            _dimExprPtrList.push_back(copyExp);
            SgValueExp* valueExp = isSgValueExp(indexExp);
            unsigned long long index = getIntegerConstantValue(valueExp);
            _numElements *= index;
            printf("get_empty_declaration:index=%ld\n", index);
        }
    }
}

#endif


// Utilities




    string get_type_string(const SgType * type)
    {
        string strBool="bool";
        string strChar="char";
        string strDouble="double";
        string strLongDouble="long double";
        string strFloat="float";
        string strInt="int";
        string strLong="long";
        string strLongLong="long long";
        string strShort="short";
        string strUChar="unsigned char";
        string strUInt="unsigned int";
        string strULong="unsigned long";
        string strUShort="unsigned short";
        string strWChar="wchar";
        string strVoidStar="void *";
        string strNULL="";


        const SgType *t = type->stripTypedefsAndModifiers();
        if      (isSgTypeBool(t)) return strBool;
        else if (isSgTypeChar(t)) return strChar;
        else if (isSgTypeDouble(t)) return strDouble;
        else if (isSgTypeLongDouble(t)) return strLongDouble;
        else if (isSgTypeFloat(t)) return strFloat;
        else if (isSgTypeInt(t)) return strInt;
        else if (isSgTypeLong(t)) return strLong;
        else if (isSgTypeLongLong(t)) return strLongLong;
        else if (isSgTypeShort(t)) return strShort;
        else if (isSgTypeSignedChar(t)) return strChar;
        else if (isSgTypeSignedInt(t)) return strInt;
        else if (isSgTypeSignedLong(t)) return strLong;
        else if (isSgTypeSignedShort(t)) return strShort;
        else if (isSgTypeUnsignedChar(t)) return strUChar;
        else if (isSgTypeUnsignedInt(t)) return strUInt;
        else if (isSgTypeUnsignedLong(t)) return strULong;
        else if (isSgTypeUnsignedShort(t)) return strUShort;
        else if (isSgTypeWchar(t)) return strWChar;
        else if (isSgPointerType(t)) return strVoidStar;
        else {
            printf("Warning: get_type_string(): unrecognized type\n");
            return strNULL;
        }
    }



    size_t sizeOfType(const SgType *type) {
        const SgType *t = type->stripTypedefsAndModifiers();
        if (isSgTypeBool(t)) return sizeof(bool);
        else if (isSgTypeChar(t)) return sizeof(char);
        else if (isSgTypeDouble(t)) return sizeof(double);
        else if (isSgTypeLongDouble(t)) return sizeof(long double);
        else if (isSgTypeFloat(t)) return sizeof(float);
        else if (isSgTypeInt(t)) return sizeof(int);
        else if (isSgTypeLong(t)) return sizeof(long);
        else if (isSgTypeLongDouble(t)) return sizeof(long double);
        else if (isSgTypeLongLong(t)) return sizeof(long long);
        else if (isSgTypeShort(t)) return sizeof(short);
        else if (isSgTypeSignedChar(t)) return sizeof(signed char);
        else if (isSgTypeSignedInt(t)) return sizeof(signed int);
        else if (isSgTypeSignedLong(t)) return sizeof(signed long);
        else if (isSgTypeSignedShort(t)) return sizeof(signed short);
        else if (isSgTypeUnsignedChar(t)) return sizeof(unsigned char);
        else if (isSgTypeUnsignedInt(t)) return sizeof(unsigned int);
        else if (isSgTypeUnsignedLong(t)) return sizeof(unsigned long);
        else if (isSgTypeUnsignedShort(t)) return sizeof(unsigned short);
        else if (isSgTypeWchar(t)) return sizeof(wchar_t);
        else if (isSgPointerType(t)) return sizeof(void *);
        else return 0;
}

// from ShiftCalculus::safeInterface_modified.C
// return the value buried in a value expression.
unsigned long long getIntegerConstantValue(SgValueExp* expr) {
    switch (expr->variantT()) {
      case V_SgCharVal: return (long long)(isSgCharVal(expr)->get_value());
      case V_SgUnsignedCharVal: return isSgUnsignedCharVal(expr)->get_value();
      case V_SgShortVal: return (long long)(isSgShortVal(expr)->get_value());
      case V_SgUnsignedShortVal: return isSgUnsignedShortVal(expr)->get_value();
      case V_SgIntVal: return (long long)(isSgIntVal(expr)->get_value());
      case V_SgUnsignedIntVal: return isSgUnsignedIntVal(expr)->get_value();
      case V_SgLongIntVal: return (long long)(isSgLongIntVal(expr)->get_value());
      case V_SgUnsignedLongVal: return isSgUnsignedLongVal(expr)->get_value();
      case V_SgLongLongIntVal: return isSgLongLongIntVal(expr)->get_value();
      case V_SgUnsignedLongLongIntVal: return isSgUnsignedLongLongIntVal(expr)->get_value();
      default: ROSE_ASSERT (!"Bad kind in getIntegerConstantValue");
    }
    ROSE_ASSERT (!"Bad kind return in getIntegerConstantValue");
    return 0;
}


//  from UpcTranslation::upc_translation.C
//  Rename main() to extern  user_main()
void renameMainToUserMain(SgFunctionDeclaration* sg_func)
{
    ROSE_ASSERT(isMain(sg_func));

    // grab symbol before any modifications.
    SgGlobal* global_scope= isSgGlobal(sg_func->get_scope());
    ROSE_ASSERT(global_scope);
    SgFunctionSymbol * symbol = global_scope->lookup_function_symbol
                                (SgName("main"),sg_func->get_type());
    ROSE_ASSERT(symbol == sg_func->get_firstNondefiningDeclaration()->get_symbol_from_symbol_table());
    global_scope->remove_symbol(symbol);
    delete (symbol); // avoid dangling symbol!!

    // rename it
    SgName new_name = SgName("user_main");
    sg_func->set_name(new_name);
    sg_func->get_declarationModifier().get_storageModifier().setExtern();

    //handle function symbol:remove the original one, insert a new one
    symbol = new SgFunctionSymbol(sg_func);
    global_scope->insert_symbol(new_name,symbol);
}


static void printNode(SgNode* node)
{
    printf("node: %s = %s\n", node->sage_class_name(), node->unparseToString().c_str());
}

static void printLocation(SgNode* node)
{
    Sg_File_Info* file = node->get_file_info();
    printf("File: (%s, %d, %d) %s = %s\n", file->get_filenameString().c_str(),
           file->get_line(),
           file->get_col(),
           isSgNode(node)->sage_class_name(),
           isSgNode(node)->unparseToString().c_str());
}

static bool isNodeInUserLocation(SgNode* node)
{
    Sg_File_Info* file = node->get_file_info();
    const char* filename = file->get_filenameString().c_str();
    if (strncmp(filename, "/usr", 4) == 0)
        return false;
    else if (strncmp(filename, "/opt", 4) == 0)
        return false;
    else
        return true;
}


#endif // _DB_ELEMENT_H

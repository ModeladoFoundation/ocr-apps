/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

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


const string strFFWD("ffwd");
const string strFFWDInit("__ffwd_init");
const string strFFWDT("__ffwd_t");
const string strFFWDP("__ffwd_p");
const string strFFWDS("__ffwd_s");
const string strROSE_FFWDH("rose_ffwd.h");
const string strINIT("init");
const string strUnderscore("_");



// forward declarations
size_t sizeOfType(const SgType *type);
static void printNode(SgNode* node);
string create_new_name(SgInitializedName * name,  SgScopeStatement * scope);
string get_type_string(SgType * type);



// This class stores information about tool generated variables (TGV).
// These variables will be placed in the global structure with the
// DbElement variables, but they lack all of the context that the
// applications variables contain.
class TgvElement
{
  public:
    TgvElement(string name, SgScopeStatement* scope)
    {
        _name = name;
        _scope = scope;
    }
    ~TgvElement(){};
    string get_name() { return _name; }
    SgScopeStatement* get_scope() { return _scope; }

  private:
    string _name;
    SgScopeStatement* _scope;
};


// This class stores the include file name that
// is needed by the translated code.
class IncElement
{
  public:
    IncElement(string nodeStr)
    {
        _nodeStr = nodeStr;
    }
    ~IncElement(){};
    string get_string() { return _nodeStr; }

  private:
    string _nodeStr;
};




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


// This class stores information about the functions which contain
// references to global and file static variables.
class FunctionElement
{
  public:
    FunctionElement(SgFunctionDefinition * func, SgNode * node);
    ~FunctionElement(){};
    SgFunctionDefinition* get_func_def() { return _func; }
    SgNode* get_node() { return _node; }

  private:
    SgFunctionDefinition * _func;
    SgNode * _node;
};


// constructor
FunctionElement::FunctionElement(SgFunctionDefinition * func, SgNode * node)
{
    _func = func;
    _node = node;
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
    bool is_const() {return _isConst; }
    SgInitializedName* get_name() { return _name; }
    string get_new_name() { return _newName; }
    SgScopeStatement* get_scope() { return _scope; }
    SgDeclarationStatement* get_declaration() { return _decl; }
    string get_type_from_decl();
    string get_type_str();
    TgvElement * get_tgv() { return _tgv; }
    void set_tgv(TgvElement * tgv) { _tgv = tgv; }
    bool is_initialized() { return _isInitialized; }
    bool is_static() { return _isStatic; }
    bool is_const_type();
    bool entry_created() { return _entryCreated; }
    void set_entry_created(bool b) { _entryCreated = b; }
    bool init_created() { return _initCreated; }
    void set_init_created(bool b) { _initCreated = b; }
    Sg_File_Info* get_file_info() { return _fileInfo; }
    int get_num_dimensions() { return _numDimensions; }
    int get_num_elements() { return _numElements; }
    Rose_STL_Container<SgExpression*> get_dim_expr_ptr_list() { return _dimExprPtrList; }
    void get_declarators(SgArrayType* arrayType );


  private:
    SgNode* _node;
    SgVariableSymbol* _sym;
    Sg_File_Info* _fileInfo;
    SgType* _baseType;
    bool _isConst;
    SgInitializedName* _name;
    string _newName;
    SgScopeStatement* _scope;
    SgDeclarationStatement* _decl;
    string _orig;
    TgvElement * _tgv;
    bool _isInitialized;
    bool _isStatic;
    bool _entryCreated;
    bool _initCreated;
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

    _scope = _name->get_scope();
    ROSE_ASSERT (_scope != NULL);

    _newName = create_new_name(_name, _scope);

    SgDeclarationStatement* _decl = isSgDeclarationStatement(_name->get_declaration());
    ROSE_ASSERT(_decl != NULL);

    switch(_decl->variantT())
    {
      case V_SgVariableDeclaration:
        _orig = isSgVariableDeclaration(_decl)->unparseToString();
        break;
      case V_SgTypedefDeclaration:
        _orig = isSgTypedefDeclaration(_decl)->unparseToString();
        break;
      case V_SgClassDeclaration:
        _orig = isSgClassDeclaration(_decl)->unparseToString();
        break;
      default:
        break;
    }
    ROSE_ASSERT(_orig.length() > 0);


    _tgv = NULL;

    _fileInfo=isSgNode(_decl)->get_file_info();

    _baseType= _name->get_type()->findBaseType();

    _isConst = is_const_type();

    _numDimensions=0;
    _numElements=0;

    if ( _name->get_initializer() != NULL)
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
}



// String manipulation routine.  sage function isConstType() is
// const char *.
bool DbElement::is_const_type()
{
    string orig = _orig;

    if (strncmp(orig.c_str(), "static ", 7) == 0)
        orig = orig.substr(7);
    if (strncmp(orig.c_str(), "extern ", 7) == 0)
        orig = orig.substr(7);
    if (strncmp(orig.c_str(), "const ", 6) == 0)
        return true;
    return false;
}


// String manipulation routine.  I cannot figure out how ROSE is storing
// system-defined and user-defined types.  Instead, parse the original declaration.
string DbElement::get_type_from_decl()
{
    string orig = _orig;

    if (strncmp(orig.c_str(), "static ", 7) == 0)
        orig = orig.substr(7);
    if (strncmp(orig.c_str(), "extern ", 7) == 0)
        orig = orig.substr(7);
    if (strncmp(orig.c_str(), "const ", 6) == 0)
        orig = orig.substr(6);
    if (strncmp(orig.c_str(), "__thread ", 9) == 0)
        orig = orig.substr(9);
    size_t pos = orig.find(_name->get_name().getString());
    string typeStr = orig.substr(0,pos);
    while(isspace(*typeStr.begin()))
        typeStr.erase(typeStr.begin());
    while(isspace(*typeStr.rbegin()))
        typeStr.erase(typeStr.length()-1);
    return typeStr;
}


string DbElement::get_type_str()
{
    // isPointerType() is missing a lot pointers;
    // use the original declaration instead.
    //
    string tstr = this->get_type_from_decl();
    return tstr;
}



// Create a new variable name for the global and file/function static variables.
// Globals are prepended with an underscore, and appended with underscore.
// File statics are prepended with an underscore and appended with the main part of
// the file name.
// Function statics are prepended with an underscore and appended with the main part
// of the file name, the function name.
// Remove the trailing number, because it is preventing externals from resolving properly.
//
string create_new_name( SgInitializedName * name, SgScopeStatement * scope)
{
    char buf[256];
    string newName;

    if (isStatic(name->get_declaration()))
    {
        string filename =  name->get_file_info()->get_filename();
        string file_no_path = StringUtility::stripPathFromFileName(filename);
        size_t dot = file_no_path.find(".");
        string fname = file_no_path.substr(0,dot);

        if (isSgGlobal(scope))
        {
            // file static
            newName = strUnderscore + fname + strUnderscore +
                      name->get_name().str() + strUnderscore;

        }
        else
        {
            // function static
            SgFunctionDeclaration * funcDecl = getEnclosingFunctionDeclaration(scope);
            if (funcDecl == 0)
            {
                printf("Warning: create_new_name: Enclosing function not found for %s\n",
                       name->get_name().getString().c_str());
                return "";
            }
            string funcName = funcDecl->get_name().getString();
            newName = strUnderscore + fname + strUnderscore +
                      funcName + strUnderscore +
                      name->get_name().str() + strUnderscore;
        }
    }

    else // global
    {
        newName = strUnderscore + name->get_name().str() + strUnderscore;
    }

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
              unsigned long long index=0;
              SgExpression* copyExp = deepCopy(indexExp);
              _dimExprPtrList.push_back(copyExp);
              if (isSgEnumVal(indexExp))
              {
                   index = isSgEnumVal(indexExp)->get_value();
              }
              else
              {
                  index = getIntegerConstantValue(isSgValueExp(indexExp));
                  //SgValueExp* valueExp = isSgValueExp(indexExp);
                  //index = getIntegerConstantValue(valueExp);
              }

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


// Utilities



string get_type_string(SgType * t)
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
    string result=strNULL;

    if      (isSgTypeBool(t)) result = strBool;
    else if (isSgTypeChar(t)) result = strChar;
    else if (isSgTypeDouble(t)) result = strDouble;
    else if (isSgTypeLongDouble(t)) result = strLongDouble;
    else if (isSgTypeFloat(t)) result = strFloat;
    else if (isSgTypeInt(t)) result = strInt;
    else if (isSgTypeLong(t)) result = strLong;
    else if (isSgTypeLongLong(t)) result = strLongLong;
    else if (isSgTypeShort(t)) result = strShort;
    else if (isSgTypeSignedChar(t)) result = strChar;
    else if (isSgTypeSignedInt(t)) result = strInt;
    else if (isSgTypeSignedLong(t)) result = strLong;
    else if (isSgTypeSignedShort(t)) result = strShort;
    else if (isSgTypeUnsignedChar(t)) result = strUChar;
    else if (isSgTypeUnsignedInt(t)) result = strUInt;
    else if (isSgTypeUnsignedLong(t)) result = strULong;
    else if (isSgTypeUnsignedShort(t)) result = strUShort;
    else if (isSgTypeWchar(t)) result = strWChar;
    else result = strNULL;

    return result;
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
      default: return 0;
    }
    return 0;
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

static bool isNodeDefinedInUserLocation(SgVariableSymbol * sym)
{
    SgDeclarationStatement* decl = sym->get_declaration()->get_declaration();
    ROSE_ASSERT(decl);
    Sg_File_Info* file=isSgNode(decl)->get_file_info();

    const char* filename = file->get_filenameString().c_str();

    if (strncmp(filename, "/usr", 4) == 0)
        return false;
    else if (strncmp(filename, "/opt", 4) == 0)
        return false;
    else
        return true;
}


#endif // _DB_ELEMENT_H

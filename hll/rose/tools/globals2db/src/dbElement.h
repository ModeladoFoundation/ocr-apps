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
const string strINIT("init");
const string strUnderscore("_");



// forward declarations
size_t sizeOfType(const SgType *type);
static void printNode(SgNode* node);
string create_new_name(SgInitializedName * name,  SgScopeStatement * scope);


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
    SgInitializedName* get_name() { return _name; }
    string get_new_name() { return _newName; }
    SgScopeStatement* get_scope() { return _scope; }
    SgDeclarationStatement* get_declaration() { return _decl; }
    string get_type_from_decl();
    TgvElement * get_tgv() { return _tgv; }
    void set_tgv(TgvElement * tgv) { _tgv = tgv; }
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


  private:
    SgNode* _node;
    SgVariableSymbol* _sym;
    Sg_File_Info* _fileInfo;
    SgType* _baseType;
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


// String manipulation routine.  I cannot figure out how ROSE is storing
// system-defined and user-defined types.  Instead, parse the original declation.
string DbElement::get_type_from_decl()
{
    string orig = _orig;

    if (strncmp(orig.c_str(), "static ", 7) == 0)
        orig = orig.substr(7);
    size_t pos = orig.find(this->get_name()->get_name().getString());
    string typeStr = orig.substr(0,pos);
    while(isspace(*typeStr.begin()))
        typeStr.erase(typeStr.begin());
    while(isspace(*typeStr.rbegin()))
        typeStr.erase(typeStr.length()-1);
    return typeStr;
}



// Create a new variable name for the global and file/function static variables.
// Globals are prepended with an underscore, and appended with underscore and number.
// File statics are prepended with an underscore and appended with the main part of
// the file name and a number.
// Function statics are prepended with an underscore and appended with the main part
// of the file name, the function name and a number.
//
string create_new_name( SgInitializedName * name, SgScopeStatement * scope)
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
        if (isSgGlobal(scope))
        {
            // file static
            std::string tstr = strUnderscore + fname + strUnderscore +
                               name->get_name().str() + strUnderscore;

            sprintf(buf, "%s%d", tstr.c_str(),num);
            newName =  string(buf);
        }
        else
        {
            // function static
            SgFunctionDeclaration * funcDecl = getEnclosingFunctionDeclaration(scope);
            if (funcDecl == 0)
            {
                printf("Warning: create_new_name: Enclosing fnction not found for %s\n",
                       name->get_name().getString().c_str());
                return "";
            }
            string funcName = funcDecl->get_name().getString();
            string tstr = strUnderscore + fname + strUnderscore +
                          funcName + strUnderscore +
                          name->get_name().str() + strUnderscore;
            sprintf(buf, "%s%d", tstr.c_str(),num);
            newName =  string(buf);
        }
    }

    else // global
    {
        std::string tstr = strUnderscore + name->get_name().str() + strUnderscore;
        sprintf(buf, "%s%d", tstr.c_str(),num);
        newName = string(buf);
    }

    num++;
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
                   printf("index=%ld\n", index);
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


// these values are copied from mpilite/mpi.h.
// Since MPIlite uses different values than MPI,
// this tool must replace the MPI value with the
// MPIlite value.
#define MPILITE_COMM_WORLD   ( 0)
#define MPILITE_COMM_SELF    ( 2)

#define MPILITE_CHAR                 1
#define MPILITE_SIGNED_CHAR          2
#define MPILITE_SHORT                3
#define MPILITE_INT                  4
#define MPILITE_LONG                 5
#define MPILITE_LONG_LONG            6
#define MPILITE_LONG_LONG_INT        7
#define MPILITE_BYTE                 8
#define MPILITE_UNSIGNED_CHAR        9
#define MPILITE_UINT8_T             MPILITE_UNSIGNED_CHAR
#define MPILITE_UNSIGNED_SHORT      10
#define MPILITE_UINT16_T            MPILITE_UNSIGNED_SHORT
#define MPILITE_UNSIGNED            11
#define MPILITE_UINT32_T            MPILITE_UNSIGNED
#define MPILITE_UNSIGNED_LONG       12
#define MPILITE_UINT64_T            MPILITE_UNSIGNED_LONG
#define MPILITE_UNSIGNED_LONG_LONG  13
#define MPILITE_FLOAT               14
#define MPILITE_DOUBLE              15
#define MPILITE_DOUBLE_INT          16
#define _MPILITE_LAST_DATATYPE      16

#define MPILITE_SUM           1
#define MPILITE_MIN           2
#define MPILITE_MAX           3
#define MPILITE_MINLOC        4
#define MPILITE_MAXLOC        5
#define MPILITE_PROD          6
#define MPILITE_LAND          7
#define MPILITE_BAND          8
#define MPILITE_LOR           9
#define MPILITE_BOR          10
#define MPILITE_LXOR         11
#define MPILITE_BXOR         12


// these values are defined in the official mpi.h
typedef int MPI_Comm;
#define MPI_COMM_WORLD ((MPI_Comm)0x44000000)
#define MPI_COMM_SELF  ((MPI_Comm)0x44000001)

typedef int MPI_Datatype;
#define MPI_CHAR           ((MPI_Datatype)0x4c000101)
#define MPI_SIGNED_CHAR    ((MPI_Datatype)0x4c000118)
#define MPI_UNSIGNED_CHAR  ((MPI_Datatype)0x4c000102)
#define MPI_BYTE           ((MPI_Datatype)0x4c00010d)
#define MPI_SHORT          ((MPI_Datatype)0x4c000203)
#define MPI_UNSIGNED_SHORT ((MPI_Datatype)0x4c000204)
#define MPI_INT            ((MPI_Datatype)0x4c000405)
#define MPI_UNSIGNED       ((MPI_Datatype)0x4c000406)
#define MPI_LONG           ((MPI_Datatype)0x4c000807)
#define MPI_UNSIGNED_LONG  ((MPI_Datatype)0x4c000808)
#define MPI_FLOAT          ((MPI_Datatype)0x4c00040a)
#define MPI_DOUBLE         ((MPI_Datatype)0x4c00080b)
#define MPI_LONG_LONG_INT  ((MPI_Datatype)0x4c000809)
#define MPI_UNSIGNED_LONG_LONG ((MPI_Datatype)0x4c000819)
#define MPI_LONG_LONG      MPI_LONG_LONG_INT
#define MPI_DOUBLE_INT        ((MPI_Datatype)0x8c000001)
// There are more, but MPI-lite does not support them now.

/* Collective operations */
typedef int MPI_Op;
#define MPI_MAX     (MPI_Op)(0x58000001)
#define MPI_MIN     (MPI_Op)(0x58000002)
#define MPI_SUM     (MPI_Op)(0x58000003)
#define MPI_PROD    (MPI_Op)(0x58000004)
#define MPI_LAND    (MPI_Op)(0x58000005)
#define MPI_BAND    (MPI_Op)(0x58000006)
#define MPI_LOR     (MPI_Op)(0x58000007)
#define MPI_BOR     (MPI_Op)(0x58000008)
#define MPI_LXOR    (MPI_Op)(0x58000009)
#define MPI_BXOR    (MPI_Op)(0x5800000a)
#define MPI_MINLOC  (MPI_Op)(0x5800000b)
#define MPI_MAXLOC  (MPI_Op)(0x5800000c)
// There are more, but MPI-lite does not support them now.


unsigned long get_mpi_datatype(string str)
{
    unsigned long type;
    if (strncmp(str.c_str(), "((MPI_Datatype )", 16) == 0)
    {
        string substr = str.substr(16, 10);
        unsigned long orig = strtoul(substr.c_str(), NULL, 0);

        switch (orig)
        {
          case MPI_CHAR:
            type=MPILITE_CHAR;
            break;
          case MPI_SIGNED_CHAR:
            type=MPILITE_SIGNED_CHAR;
            break;
          case MPI_UNSIGNED_CHAR:
            type=MPILITE_UNSIGNED_CHAR;
            break;
          case MPI_BYTE:
            type=MPILITE_CHAR;
            break;
          case MPI_SHORT:
            type=MPILITE_SHORT;
            break;
          case MPI_UNSIGNED_SHORT:
            type=MPILITE_UNSIGNED_SHORT;
            break;
          case MPI_INT:
            type=MPILITE_INT;
            break;
          case MPI_UNSIGNED:
            type=MPILITE_UNSIGNED;
            break;
          case MPI_LONG:
            type=MPILITE_LONG;
            break;
          case MPI_UNSIGNED_LONG:
            type=MPILITE_UNSIGNED_LONG;
            break;
          case MPI_FLOAT:
            type=MPILITE_FLOAT;
            break;
          case MPI_DOUBLE:
            type=MPILITE_DOUBLE;
            break;
          case MPI_LONG_LONG_INT:
            type=MPILITE_LONG_LONG_INT;
            break;
          case MPI_UNSIGNED_LONG_LONG:
            type=MPILITE_UNSIGNED_LONG_LONG;
            break;
          case MPI_DOUBLE_INT:
            type=MPILITE_DOUBLE_INT;
            break;
          default:
            printf("ERROR: get_mpi_datatype(): Unsupported datatype.\n");
            exit(-1);
        }
        return type;
    }
    else
    {
        printf("ERROR: get_mpi_datatype(): Internal error.\n");
        exit(-1);
    }
}


unsigned long get_mpi_comm(string str)
{
    unsigned long comm=0;
    if (strncmp(str.c_str(), "((MPI_Comm )", 12) == 0)
    {
        string substr = str.substr(12, 10);
        unsigned long orig=strtoul(substr.c_str(), NULL, 0);

        switch (orig)
        {
          case MPI_COMM_WORLD:
            comm=MPILITE_COMM_WORLD;
            break;
          case MPI_COMM_SELF:
            comm=MPILITE_COMM_SELF;
            break;
          default:
            printf("ERROR: get_mpi_comm(): Unsupported COMM.\n");
            exit(-1);
        }
        return comm;
    }
    else
    {
        printf("ERROR: get_mpi_comm(): Internal error.\n");
        exit(-1);
    }
}


unsigned long get_mpi_op(string str)
{
    unsigned long op=0;
    if (strncmp(str.c_str(), "((MPI_Op )", 10) == 0)
    {
        string substr = str.substr(10, 10);
        unsigned long orig=strtoul(substr.c_str(), NULL, 0);

        switch (orig)
        {
          case MPI_MAX:
            op=MPILITE_MAX;
            break;
          case MPI_MIN:
            op=MPILITE_MIN;
            break;
          case MPI_SUM:
            op=MPILITE_SUM;
            break;
          case MPI_PROD:
            op=MPILITE_PROD;
            break;
          case MPI_LAND:
            op=MPILITE_LAND;
            break;
          case MPI_BAND:
            op=MPILITE_BAND;
            break;
          case MPI_LOR:
            op=MPILITE_LOR;
            break;
          case MPI_BOR:
            op=MPILITE_BOR;
            break;
          case MPI_LXOR:
            op=MPILITE_LXOR;
            break;
          case MPI_BXOR:
            op=MPILITE_BXOR;
            break;
          case MPI_MINLOC:
            op=MPILITE_MINLOC;
            break;
          case MPI_MAXLOC:
            op=MPILITE_MAXLOC;
            break;
          default:
            printf("ERROR: get_mpi_op(): Unsupported MPI_Op.\n");
            exit(-1);
        }
        return op;
    }
    else
    {
        printf("ERROR: get_mpi_op(): Internal error.\n");
        exit(-1);
    }
}


string get_type_string(DbElement * dbElement)
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

    const SgType * type = dbElement->get_base_type();
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
    else return dbElement->get_type_from_decl();
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

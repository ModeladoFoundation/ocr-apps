/*****************************************
 * Author: Sriram Aananthakrishnan, 2017 *
 *****************************************/
#ifndef _OCRASTINFO_H
#define _OCRASTINFO_H

#include <string>
#include <boost/shared_ptr.hpp>
#include "ocrSymbolTable.h"

// We will do some book keeping for the generated AST fragments
// during the translation phase.
// Useful in accessing them at various points during translation
// Motivation : We will need the GUIDs of datablock, edts and events
// at various steps of generating the AST for objects

/*********************
 * Utility Functions *
 *********************/
SgVariableSymbol* GetVariableSymbol(std::string name, SgScopeStatement* currentScope);
SgVariableSymbol* GetVariableSymbol(SgVariableDeclaration* vdecl, std::string vname);
SgVariableSymbol* GetVariableSymbol(SgInitializedName* vsgn);

/**************
 * DbkAstInfo *
 **************/
/*!
 * \brief DbkAstInfo stores the variable names for guid
 * and ptr created by the translation.
 *
 * The symbol is looked up using the SageInterface::lookupSymbolInParentScopes(varname)
 */
class DbkAstInfo {
  std::string m_dbkname;
  std::string m_dbkGuidName;
  std::string m_dbkPtrName;
 public:
  DbkAstInfo(std::string dbkname, std::string guidName, std::string ptrName);
  std::string getDbkName() const;
  std::string getDbkGuidName() const;
  std::string getDbkPtrName() const;
  std::string str() const;
};
typedef boost::shared_ptr<DbkAstInfo> DbkAstInfoPtr;

/**************
 * EvtAstInfo *
 **************/
class EvtAstInfo {
  std::string m_evtname;
  std::string m_evtGuidName;
 public:
  EvtAstInfo(std::string evtname, std::string evtGuidName);
  std::string getEvtGuidName() const;
};
typedef boost::shared_ptr<EvtAstInfo> EvtAstInfoPtr;

/************************
 * TaskOutliningAstInfo *
 ************************/
/*!
 * \brief TaskOutliningAstInfo holds the outlined AST components of an EDT
 */
class TaskOutliningAstInfo {
  std::string m_taskName;
  SgClassDeclaration* m_depElemStructDecl;
  SgType* m_depElemBaseType;
  SgType* m_depElemTypedefType;
  SgFunctionDeclaration* m_edtDecl;
 public:
  TaskOutliningAstInfo(std::string name);
  // set functions
  void setDepElemStructDecl(SgClassDeclaration* depElemStructDecl);
  void setDepElemBaseType(SgType* depElemBaseType);
  void setDepElemTypedefType(SgType* depElemTypedefType);
  void setTaskFuncDecl(SgFunctionDeclaration* edtDecl);
  // get functions
  SgClassDeclaration* getDepElemStructDecl() const;
  SgClassDefinition* getDepElemStructDefn() const;
  SgType* getDepElemBaseType() const;
  SgType* getDepElemTypedefType() const;
  SgFunctionDeclaration* getTaskFuncDecl() const;
  std::string str() const;
  ~TaskOutliningAstInfo();
};
typedef boost::shared_ptr<TaskOutliningAstInfo> TaskOutliningAstInfoPtr;

/***************
 * TaskAstInfo *
 ***************/
/*!
 *\brief Base class for storing AST builder information
 *
 * TaskAstInfo holds a shared_ptr to TaskOutliningAstInfo
 * All Edt AstInfo class must inherit TaskAstInfo to access its outlined EDT's AST components
 */
class TaskAstInfo {
 public:
  typedef enum {
    e_TaskEdt,
    e_TaskLoopIterControl,
    e_TaskShutDown
  } TaskType;
 protected:
  TaskType m_taskType;
  std::string m_taskName;
  TaskOutliningAstInfoPtr m_outliningInfo;
 public:
  TaskAstInfo(std::string name);
  TaskType getTaskType() const;
  TaskOutliningAstInfoPtr getTaskOutliningAstInfo() const;
  // Get Functions for OutliningInfo
  SgClassDeclaration* getDepElemStructDecl() const;
  SgClassDefinition* getDepElemStructDefn() const;
  SgType* getDepElemBaseType() const;
  SgType* getDepElemTypedefType() const;
  SgFunctionDeclaration* getTaskFuncDecl() const;
  // Set Functions for Outlining Info
  void setDepElemStructDecl(SgClassDeclaration* depElemStructDecl);
  void setDepElemBaseType(SgType* depElemBaseType);
  void setDepElemTypedefType(SgType* depElemTypedefType);
  void setTaskFuncDecl(SgFunctionDeclaration* edtDecl);
  virtual std::string str() const=0;
  ~TaskAstInfo();
};
typedef boost::shared_ptr<TaskAstInfo> TaskAstInfoPtr;

/**************
 * EdtAstInfo *
 **************/
/*!
 * \brief EdtAstInfo holds the variable names of parameters needed in setting up the EDT
 *
 * Store the variable names and use the SgSymbolTable for looking up the SgVariableSymbol.
 * Working with the assumption that all variable declaration statements are inserted
 * either in the current scope of EDT setup or in the parent scope.
 * SageInterface::loopVariableSymbolInParentScopes() can then be used to get the variable symbols.
 */
class EdtAstInfo : public TaskAstInfo {
  std::string m_edtTemplGuidName;
  std::string m_depElemStructName;
  std::string m_edtGuidName;
 public:
  EdtAstInfo(std::string edtname);
  std::string getEdtTemplateGuidName() const;
  std::string getDepElemStructName() const;
  std::string getEdtGuidName() const;
  void setTemplGuidName(std::string edtTemplGuidName);
  void setDepElemStructName(std::string depElemStructName);
  void setEdtGuidName(std::string edtGuidName);
  std::string str() const;
  ~EdtAstInfo();
};
typedef boost::shared_ptr<EdtAstInfo> EdtAstInfoPtr;

/*****************************
 * LoopControlEdtAstInfo *
 *****************************/
class LoopControlEdtAstInfo : public EdtAstInfo {
  // We generate this basic block and it is used to insert statements later
  SgBasicBlock* m_basicblock;
  // Basic block for true body of the loop control EDT
  SgBasicBlock* m_ifBasicBlock;
  // Basic block for the false body of the loop control EDT
  SgBasicBlock* m_elseBasicBlock;
  // If Stmt for loop control structure
  SgIfStmt* m_ifStmt;
  // Name we assign inside the depElem struct
  std::string m_depElemCompEvtGuidName;
  // Name we assign when creating the event
  std::string m_compEvtGuidName;
 public:
  LoopControlEdtAstInfo(std::string name);
  SgBasicBlock* getBasicBlock() const;
  std::string getCompEvtGuidName() const;
  std::string getDepElemCompEvtGuidName() const;

  void setBasicBlock(SgBasicBlock* basicblock);
  void setCompEvtGuidName(std::string compEvtGuidName);
  void setDepElemCompEvtGuidName(std::string depElemCompEvtGuidName);
  void setLoopControlIfBasicBlock(SgBasicBlock* ifBasicBlock);
  void setLoopControlElseBasicBlock(SgBasicBlock* elseBasicBlock);
  void setLoopControlIfStmt(SgIfStmt* loopControlIfStmt);
  std::string str() const;
  ~LoopControlEdtAstInfo();
};
typedef boost::shared_ptr<LoopControlEdtAstInfo> LoopControlEdtAstInfoPtr;

/******************
 * AstInfoManager *
 ******************/
typedef std::map<std::string, TaskAstInfoPtr> TaskAstInfoMap;
typedef std::pair<std::string, TaskAstInfoPtr> TaskAstInfoMapElem;
class AstInfoManager {
  OcrObjectSymbolTable<DbkAstInfo> m_dbkAstInfoMap;
  OcrObjectSymbolTable<EvtAstInfo> m_evtAstInfoMap;
  TaskAstInfoMap m_taskAstInfoMap;
 public:
  AstInfoManager();
  bool findEdtAstInfo(std::string edtName) const;
  DbkAstInfoPtr regDbkAstInfo(std::string dbkname, std::string dbkGuidName, std::string dbkPtrName, SgScopeStatement* scope);
  EvtAstInfoPtr regEvtAstInfo(std::string evtName, std::string evtGuidName, SgScopeStatement* scope);
  EdtAstInfoPtr regEdtAstInfo(std::string edtName);
  LoopControlEdtAstInfoPtr regLoopControlEdtAstInfo(std::string edtName);

  DbkAstInfoPtr getDbkAstInfo(std::string dbkname, SgScopeStatement* scope);
  EvtAstInfoPtr getEvtAstInfo(std::string evtname, SgScopeStatement* scope);
  EdtAstInfoPtr getEdtAstInfo(std::string edtname);
  LoopControlEdtAstInfo getLoopControlEdtAstInfo(std::string edtName);
  std::string EvtAstInfoMap2Str() const;
};

#endif

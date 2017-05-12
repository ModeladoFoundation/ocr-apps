/*
 * Author: Sriram Aananthakrishnan, 2016 *
 */

#ifndef OCROBJECTINFO_H
#define OCROBJECTINFO_H

#include <string>
#include <list>
#include <boost/shared_ptr.hpp>
#include "ocrSymbolTable.h"


/*****************
 * OcrDbkContext *
 *****************/
class OcrDbkContext {
  std::string m_name;
  SgInitializedName* m_vdefn;
  std::list<SgStatement*> m_allocStmts;
  SgPragmaDeclaration* m_pragma;
public:
  OcrDbkContext(std::string name);
  OcrDbkContext(std::string name, SgInitializedName* vdefn, std::list<SgStatement*> allocStmt, SgPragmaDeclaration* pragma);
  SgDeclarationStatement* get_declaration() const;
  SgInitializedName* getSgInitializedName() const;
  SgType* getDbkPtrType();
  std::list<SgStatement*> get_allocStmts() const;
  std::string get_name() const;
  SgPragmaDeclaration* get_pragma() const;
  std::string str() const;
  ~OcrDbkContext();
};

typedef boost::shared_ptr<OcrDbkContext> OcrDbkContextPtr;

/*****************
 * OcrEvtContext *
 *****************/
class OcrEvtContext {
  std::string m_name;
public:
  OcrEvtContext(std::string name);
  std::string get_name() const;
  std::string str() const;
  ~OcrEvtContext();
};

typedef boost::shared_ptr<OcrEvtContext> OcrEvtContextPtr;

/******************
 * OcrTaskContext *
 ******************/
/*! \class OcrTaskContext
 *  \brief Base class for the OCR task context.
 * A context carries all the necessary information to synthesize OCR code.
 * OcrTaskContext is the base class for different tasks - EDT, LoopIterEdt, ShutdownEdt etc.,
 * The context class is used by the translator class to generate the corresponding OCR code.
 * We will use the base class shared_ptr for storing all task contexts in a map
 */
class OcrTaskContext {
 public:
  typedef enum {
    e_TaskEdt,
    e_TaskLoopIter,
    e_TaskShutDown,
    e_TaskMain
  } OcrTaskType;
 protected:
  OcrTaskType m_type;
  std::string m_taskName;
  unsigned int m_traversalOrder; //!< In-Order Traversal Order of the Pragma Nodes
  SgPragmaDeclaration* m_sgpdecl;
 public:
  OcrTaskContext(OcrTaskType type, std::string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl);
  OcrTaskType getTaskType() const;
  SgPragmaDeclaration* getPragma() const;
  unsigned int getTraversalOrder() const;
  std::string getTaskName() const;
  virtual std::string str() const = 0;
  virtual ~OcrTaskContext();
};
typedef boost::shared_ptr<OcrTaskContext> OcrTaskContextPtr;

/*****************
 * OcrEdtContext *
 *****************/
//! Class to store information required to synthesize an OCR EDT
//! List of dependent data blocks
//! List of dependent events
//! List of parameters that should be passed as an input
//! List of statements collected by an AST traversal
class OcrEdtContext : public OcrTaskContext {
  SgBasicBlock* m_basicblock;
  std::list<OcrDbkContextPtr> m_depDbks;
  std::list<OcrEvtContextPtr> m_depEvts;
  std::list<OcrDbkContextPtr> m_dbksToCreate;
  std::list<SgVarRefExp*> m_depElems;
  OcrEvtContextPtr m_outputEvt;
  std::list<std::string> m_dbksToDestroy;
  std::list<std::string> m_evtsToDestroy;
  bool m_finishEdt;
public:
  OcrEdtContext(std::string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
		std::list<OcrDbkContextPtr> depDbks,
		std::list<OcrEvtContextPtr> depEvts,
		std::list<SgVarRefExp*> depElems,
		OcrEvtContextPtr outputEvt,
		SgBasicBlock* basicblock,
		bool finishEdt);
  // Constructor to be used by its children
  OcrEdtContext(OcrTaskType type, std::string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
		std::list<OcrDbkContextPtr> depDbks,
		std::list<OcrEvtContextPtr> depEvts,
		std::list<SgVarRefExp*> depElems,
		OcrEvtContextPtr outputEvt);
  // Set Functions
  void setDbksToDestroy(std::list<std::string> dbksToDestroy);
  void setEvtsToDestroy(std::list<std::string> evtsToDestroy);
  void setDbksToCreate(std::list<OcrDbkContextPtr> dbksToCreate);

  // Get Functions
  std::string get_name() const;
  SgSourceFile* getSourceFile();
  std::list<SgVarRefExp*> getDepElems() const;
  std::list<OcrDbkContextPtr> getDepDbks() const;
  std::list<OcrEvtContextPtr> getDepEvts() const;
  OcrEvtContextPtr getOutputEvt() const;
  SgPragmaDeclaration* getTaskPragma() const;
  unsigned int getNumDepElems() const;
  unsigned int getNumDepDbks() const;
  unsigned int getNumDepEvts() const;
  unsigned int getDepDbkSlotNumber(std::string dbkname) const;
  unsigned int getDepEvtSlotNumber(std::string evtname) const;
  bool isFinishEdt() const;
  virtual SgBasicBlock* getTaskBasicBlock() const;
  std::list<std::string> getDbksToDestroy() const;
  std::list<std::string> getEvtsToDestroy() const;
  virtual std::string str() const;
  ~OcrEdtContext();
};

typedef boost::shared_ptr<OcrEdtContext> OcrEdtContextPtr;

/*************************
 * OcrLoopIterEdtContext *
 *************************/
class OcrLoopIterEdtContext : public OcrEdtContext {
  // While/do-while/for
  SgStatement* m_loopStmt;
 public:
  OcrLoopIterEdtContext(std::string name,  unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
			std::list<OcrDbkContextPtr> depDbks,
			std::list<OcrEvtContextPtr> depEvts,
			std::list<SgVarRefExp*> depElems,
			OcrEvtContextPtr outputEvt,
			SgStatement* loopStmt);
  std::string getLoopBodyEdtName() const;
  std::string getLoopControlEdtName() const;
  SgSourceFile* getSourceFile() const;
  SgStatement* getLoopStmt() const;
  std::vector<SgStatement*> getLoopInitStmts();
  virtual SgBasicBlock* getTaskBasicBlock() const;
  std::string str() const;
  ~OcrLoopIterEdtContext();
};
typedef boost::shared_ptr<OcrLoopIterEdtContext> OcrLoopIterEdtContextPtr;

/*********************
 * OcrMainEdtContext *
 *********************/
class OcrMainEdtContext : public OcrTaskContext {
  SgBasicBlock* m_basicblock;
 public:
  OcrMainEdtContext(std::string name, unsigned int traversalOrder, SgBasicBlock* basicblock);
  std::string getMainEdtFuncName() const;
  std::string str() const;
};
typedef boost::shared_ptr<OcrMainEdtContext> OcrMainEdtContextPtr;

/*************************
 * OcrShutdownEdtContext *
 *************************/
// ocrShutdown() is wrapped inside a shutdown EDT
// Wrapping ocrShutdown inside an EDT allows us add dependent events
// and control the runtime shutting down
// OCR shutdown is specified using the following pragma:
// #pragma ocr shutdown DEP_EVTs(...)
// For each shutdown pragma we will associate a OcrShutdownEdtContext
class OcrShutdownEdtContext : public OcrTaskContext {
  std::list<OcrEvtContextPtr> m_depEvts;
 public:
  OcrShutdownEdtContext(std::string name, unsigned int traversalOrder, SgPragmaDeclaration* shutdownPragma,
			std::list<OcrEvtContextPtr> depEvts);
  std::list<OcrEvtContextPtr> getDepEvts() const;
  SgSourceFile* getSourceFile() const;
  unsigned int getNumDepEvts();
  std::string getShutdownEdtFuncName() const;
  std::string str() const;
};

typedef boost::shared_ptr<OcrShutdownEdtContext> OcrShutdownEdtContextPtr;

/********************
 * OcrObjectManager *
 ********************/
/*!
 *! \brief OcrObjectManager should be used with a traversal class
 *! that will populate information based on OCR annotations.
 */
//! OcrObjects are either OCR Datablocks, EDTs or Events
//! OcrContext object containts the necessary information
//! gathered from annotation for each OCR object.
//! As we traverse the AST, we build OcrObjects based on the
//! annotations. The traversal builds the context for each
//! OCR object and uses the OcrObjectManager for managing
//! the contexts for each OCR object.
typedef std::map<std::string, OcrTaskContextPtr> OcrTaskContextMap;
typedef std::pair<std::string, OcrTaskContextPtr> OcrTaskContextMapElem;
typedef std::map<std::string, OcrDbkContextPtr> OcrDbkObjectMap;
typedef std::map<std::string, OcrEvtContextPtr> OcrEvtObjectMap;
typedef std::pair<std::string, OcrEvtContextPtr> OcrEvtObjectMapElem;
typedef std::map<int, std::string> EdtPragmaOrderMap;
typedef std::pair<int, std::string> EdtPragmaOrderMapElem;

class OcrObjectManager {
  //! Associates an OcrContext for each OcrObject
  //! Key: OcrObject name (string)
  //! Value: OcrContext
  OcrTaskContextMap m_ocrTaskContextMap;
  OcrEvtObjectMap m_ocrEvtObjectMap;
  EdtPragmaOrderMap m_edtPragmaOrderMap;
  OcrObjectSymbolTable<OcrDbkContext> m_dbkSymbolTable;
 public:
  OcrObjectManager();
  // Lookup functions for OcrContext using their names
  std::list<OcrEvtContextPtr> getOcrEvtContextList(std::list<std::string> evtList);
  std::list<OcrDbkContextPtr> getOcrDbkContextList(std::list<std::string> dbksList, SgScopeStatement* scope);

  // Functions to create shared_ptr for OcrContext
  std::list<OcrEvtContextPtr> registerOcrEvts(std::list<std::string> evtsNameList);
  OcrEvtContextPtr registerOcrEvt(std::string evtName);
  void registerOcrDbk(std::string dbkName, OcrDbkContextPtr dbkContext, SgScopeStatement* scope);
  OcrTaskContextPtr registerOcrEdt(std::string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
				   std::list<OcrDbkContextPtr> depDbks,
				   std::list<OcrEvtContextPtr> depEvts,
				   std::list<SgVarRefExp*> depElems,
				   OcrEvtContextPtr outputEvt,
				   SgBasicBlock* basicblock,
				   bool finishEdt);
  OcrTaskContextPtr registerOcrMainEdt(std::string name, unsigned int traversalOrder, SgBasicBlock* basicblock);
  OcrTaskContextPtr registerOcrLoopIterEdt(std::string name,  unsigned int traversalOrder,
					   SgPragmaDeclaration* sgpdecl,
					   std::list<OcrDbkContextPtr> depDbks,
					   std::list<OcrEvtContextPtr> depEvts,
					   std::list<SgVarRefExp*> depElems,
					   OcrEvtContextPtr outputEvt,
					   SgStatement* loopStmt);
  OcrTaskContextPtr registerOcrShutdownEdt(std::string name, unsigned int traversalOrder, SgPragmaDeclaration* shutdownPragma,
					   std::list<OcrEvtContextPtr> depEvts);
  // return a list of edtnames in the same order they were encountered in the AST
  std::list<std::string> getEdtTraversalOrder() const;

  // Access functions
  const OcrTaskContextMap& getOcrTaskContextMap() const;
  OcrTaskContextPtr getOcrTaskContext(std::string edtname) const;
  OcrEvtContextPtr getOcrEvtContext(std::string evtname);
  std::list<OcrDbkContextPtr> getOcrDbkContextList() const;
};

#endif

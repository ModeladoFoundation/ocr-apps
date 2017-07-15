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
 public:
  typedef enum {
    DBK_mem,
    DBK_arr
  } OcrDbkType;
 protected:
  OcrDbkType m_type;
  std::string m_name;
  SgPragmaDeclaration* m_pragma;
 public:
  OcrDbkContext(std::string name, OcrDbkType dbkType, SgPragmaDeclaration* pragma);
  OcrDbkType getDbkType() const;
  SgPragmaDeclaration* get_pragma() const;
  std::string get_name() const;
  virtual std::string str() const=0;
  ~OcrDbkContext();
};
typedef boost::shared_ptr<OcrDbkContext> OcrDbkContextPtr;

/********************
 * OcrMemDbkContext *
 ********************/
class OcrMemDbkContext : public OcrDbkContext {
  SgInitializedName* m_vdefn;
  std::list<SgStatement*> m_allocStmts;
public:
  OcrMemDbkContext(std::string name, SgInitializedName* vdefn, std::list<SgStatement*> allocStmt, SgPragmaDeclaration* pragma);
  SgDeclarationStatement* get_declaration() const;
  SgInitializedName* getSgInitializedName() const;
  SgType* getDbkPtrType();
  std::list<SgStatement*> get_allocStmts() const;
  std::string get_name() const;
  SgPragmaDeclaration* get_pragma() const;
  std::string str() const;
  ~OcrMemDbkContext();
};
typedef boost::shared_ptr<OcrMemDbkContext> OcrMemDbkContextPtr;

/********************
 * OcrArrDbkContext *
 ********************/
class OcrArrDbkContext : public OcrDbkContext {
  SgInitializedName* m_arrInitName;
 public:
  OcrArrDbkContext(std::string name, SgInitializedName* arrDefn, SgPragmaDeclaration* pragma);
  SgInitializedName* getArrInitializedName() const;
  std::string str() const;
  ~OcrArrDbkContext();
};
typedef boost::shared_ptr<OcrArrDbkContext> OcrArrDbkContextPtr;

/*****************
 * OcrEvtContext *
 *****************/
class OcrEvtContext {
 public:
  typedef enum {
    EVT_NODBK,
    EVT_DBK
  } EvtDbkAttrType;
 protected:
  std::string m_name;
  EvtDbkAttrType m_dbkAttrType;
public:
  OcrEvtContext(std::string name, EvtDbkAttrType dbkAttrType);
  std::string get_name() const;
  EvtDbkAttrType getEvtDbkAttrType() const;
  virtual bool isEvtTakesDbkArg() const;
  virtual std::string str() const;
  ~OcrEvtContext();
};

typedef boost::shared_ptr<OcrEvtContext> OcrEvtContextPtr;

/********************
 * OcrEvtDbkContext *
 ********************/
class OcrEvtDbkContext : public OcrEvtContext {
  SgVariableSymbol* m_dbkSymbol;
 public:
  OcrEvtDbkContext(std::string name, EvtDbkAttrType dbkAttrType, SgVariableSymbol* dbkSymbol);
  virtual bool isEvtTakesDbkArg() const;
  SgVariableSymbol* getDbkSymbol() const;
  std::string str() const;
  ~OcrEvtDbkContext();
};
typedef boost::shared_ptr<OcrEvtDbkContext> OcrEvtDbkContextPtr;

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
    e_TaskSpmdRegion,
    e_TaskSpmdFinalize,
    e_TaskSpmdSend,
    e_TaskSpmdRecv,
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

  std::list<SgVarRefExp*> m_depElems;
  OcrEvtContextPtr m_outputEvt;

  bool m_finishEdt;
public:
  OcrEdtContext(OcrTaskType type, std::string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
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

/************************
 * OcrSpmdRegionContext *
 ************************/
/*!
 * \brief Store information required for spawning spmd EDTs
 */
class OcrSpmdRegionContext : public OcrEdtContext {
  unsigned int m_ntasks;
 public:
  OcrSpmdRegionContext(std::string name, unsigned int traversalOrder, SgPragmaDeclaration* spgdecl,
		       std::list<OcrDbkContextPtr> depDbks, std::list<OcrEvtContextPtr> depEvts,
		       std::list<SgVarRefExp*> depElems,
		       OcrEvtContextPtr outEvt,
		       SgBasicBlock* basicblock,
		       unsigned int ntasks);
  unsigned int getNTasks() const;
  std::string str() const;
};

typedef boost::shared_ptr<OcrSpmdRegionContext> OcrSpmdRegionContextPtr;

/**************************
 * OcrSpmdFinalizeContext *
 **************************/
/*!
 * \brief Store information required to call spmdRankFinalize()
 */
class OcrSpmdFinalizeContext : public OcrTaskContext {
  std::list<OcrEvtContextPtr> m_depEvts;
 public:
  OcrSpmdFinalizeContext(std::string name, unsigned int traversalOrder, SgPragmaDeclaration* spgdecl,
			 std::list<OcrEvtContextPtr> depEvts);
  std::list<OcrEvtContextPtr> getDepEvts() const;
  std::string str() const;
};
typedef boost::shared_ptr<OcrSpmdFinalizeContext> OcrSpmdFinalizeContextPtr;

/**********************
 * OcrSpmdSendContext *
 **********************/
class OcrSpmdSendContext : public OcrTaskContext {
  OcrDbkContextPtr m_dbkToSend;
  std::list<OcrEvtContextPtr> m_depEvts;
  OcrEvtContextPtr m_outEvt;
  SgFunctionCallExp* m_sendCallExp;
 public:
  OcrSpmdSendContext(std::string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
		     OcrDbkContextPtr dbkToSend, std::list<OcrEvtContextPtr> depEvts,
		     OcrEvtContextPtr outEvt, SgFunctionCallExp* sendCallExp);
  OcrDbkContextPtr getDbkToSend() const;
  std::list<OcrEvtContextPtr> getDepEvts() const;
  OcrEvtContextPtr getOutputEvt() const;
  SgFunctionCallExp* getSendCallExp() const;
  std::string str() const;
  ~OcrSpmdSendContext();
};
typedef boost::shared_ptr<OcrSpmdSendContext> OcrSpmdSendContextPtr;

/**********************
 * OcrSpmdRecvContext *
 **********************/
class OcrSpmdRecvContext : public OcrTaskContext {
  OcrEvtContextPtr m_recvEvt;
  std::list<OcrEvtContextPtr> m_depEvts;
  OcrEvtContextPtr m_outEvt;
  SgFunctionCallExp* m_recvCallExp;
 public:
  OcrSpmdRecvContext(std::string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
		     OcrEvtContextPtr recvEvt, std::list<OcrEvtContextPtr> depEvts,
		     OcrEvtContextPtr outEvt, SgFunctionCallExp* recvCallExp);
  OcrEvtContextPtr getRecvEvt() const;
  OcrEvtContextPtr getOutputEvt() const;
  std::list<OcrEvtContextPtr> getDepEvts() const;
  SgFunctionCallExp* getRecvCallExp() const;
  std::string str() const;
  ~OcrSpmdRecvContext();
};
typedef boost::shared_ptr<OcrSpmdRecvContext> OcrSpmdRecvContextPtr;

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

/****************
 * MpiOpContext *
 ****************/
/*!
 *\brief Context information surrounding MPI operation
 */
class MpiOpContext {
 public:
  typedef enum {
    OP_INIT,
    OP_FINALIZE,
    OP_COMM_RANK,
    OP_COMM_SIZE,
    OP_SEND,
    OP_RECV,
    OP_REDUCE
  } MpiOpType;
 private:
  MpiOpType m_opType;
  SgStatement* m_stmt;
 public:
  MpiOpContext(MpiOpType type, SgStatement* stmt);
  MpiOpType getMpiOpType() const;
  SgStatement* getMpiCallStmt() const;
  std::string str() const;
};
typedef boost::shared_ptr<MpiOpContext> MpiOpContextPtr;

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
typedef std::list<MpiOpContextPtr> MpiOpContextList;

class OcrObjectManager {
  //! Associates an OcrContext for each OcrObject
  //! Key: OcrObject name (string)
  //! Value: OcrContext
  OcrTaskContextMap m_ocrTaskContextMap;
  OcrEvtObjectMap m_ocrEvtObjectMap;
  EdtPragmaOrderMap m_edtPragmaOrderMap;
  MpiOpContextList m_mpiOpContextList;
  OcrObjectSymbolTable<OcrDbkContext> m_dbkSymbolTable;
 public:
  OcrObjectManager();

  // Functions to create shared_ptr for OcrContext
  std::list<OcrEvtContextPtr> registerOcrEvts(std::list<std::string> evtsNameList);
  OcrEvtContextPtr registerOcrEvt(std::string evtName);
  OcrEvtContextPtr registerOcrEvtDbk(std::string evtName, SgVariableSymbol* dbkSymbol);
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
  OcrTaskContextPtr registerOcrSpmdRegionEdt(std::string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
					     std::list<OcrDbkContextPtr> depDbks,
					     std::list<OcrEvtContextPtr> depEvts,
					     std::list<SgVarRefExp*> depElems,
					     OcrEvtContextPtr outEvt,
					     SgBasicBlock* basicblock,
					     unsigned int ntasks);
  OcrTaskContextPtr registerOcrSpmdFinalizeEdt(std::string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
					       std::list<OcrEvtContextPtr> depEvts);
  OcrTaskContextPtr registerOcrSpmdSendContext(std::string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
					       OcrDbkContextPtr dbkToSend, std::list<OcrEvtContextPtr> depEvts,
					       OcrEvtContextPtr outEvt, SgFunctionCallExp* sendCallExp);
  OcrTaskContextPtr registerOcrSpmdRecvContext(std::string name, unsigned int traversalOrder, SgPragmaDeclaration* sgpdecl,
					       OcrEvtContextPtr recvEvt, std::list<OcrEvtContextPtr> depEvts,
					       OcrEvtContextPtr outEvt, SgFunctionCallExp* recvCallExp);

  bool registerMpiOpContext(MpiOpContextPtr mpiOpContext);
  // return a list of edtnames in the same order they were encountered in the AST
  std::list<std::string> getEdtTraversalOrder() const;


  // Lookup functions for OcrContext using their names
  std::list<OcrEvtContextPtr> getOcrEvtContextList(std::list<std::string> evtList);
  std::list<OcrDbkContextPtr> getOcrDbkContextList(std::list<std::string> dbksList, SgScopeStatement* scope);
  OcrTaskContextPtr getOcrTaskContext(std::string edtname) const;
  OcrEvtContextPtr getOcrEvtContext(std::string evtname);
  OcrDbkContextPtr getOcrDbkContext(std::string dbkname, SgScopeStatement* scope);
  std::list<MpiOpContextPtr> getMpiOpContextList() const;
  const OcrTaskContextMap& getOcrTaskContextMap() const;
  std::list<OcrDbkContextPtr> getOcrDbkContextList() const;

  /*!
   *\brief Returns true if the program has MPI operations
   */
  bool hasMpiOp() const;
};
#endif

/*
 * Author: Sriram Aananthakrishnan, 2016 *
 */

#ifndef OCROBJECTINFO_H
#define OCROBJECTINFO_H

#include <string>
#include <list>
#include <boost/shared_ptr.hpp>

/***************
 * OcrObjectId *
 ***************/
/*! \brief Generate id for OcrObjects
 *
 * Not a thread-safe implementation of id generation
 * Using a static variable instead of a singleton pattern
 */
class GenOcrObjectId {
  static unsigned int m_id;
 public:
  GenOcrObjectId();
  //! returns a new id
  unsigned int get_new_id();
};

/*************
 * OcrObject *
 *************/
class OcrObject {
  unsigned int m_id;
 public:
  OcrObject(unsigned int id_);
  virtual std::string str() const;
  unsigned int get_id() const;
  bool operator<(const OcrObject& that) const;
};

/********************
 * OcrObjectContext *
 ********************/
class OcrObjectContext {
 public:
  OcrObjectContext();
  virtual std::string str() const=0;
  virtual ~OcrObjectContext();
};

typedef boost::shared_ptr<OcrObjectContext> OcrObjectContextPtr;

/*****************
 * OcrDbkContext *
 *****************/
class OcrDbkContext : public OcrObjectContext {
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
class OcrEvtContext : public OcrObjectContext {
  std::string m_name;
public:
  OcrEvtContext(std::string name);
  std::string get_name() const;
  std::string str() const;
  ~OcrEvtContext();
};

typedef boost::shared_ptr<OcrEvtContext> OcrEvtContextPtr;

/*****************
 * OcrEdtContext *
 *****************/
//! Class to store information required to synthesize an OCR EDT
//! List of dependent data blocks
//! List of dependent events
//! List of parameters that should be passed as an input
//! List of statements collected by an AST traversal
class OcrEdtContext : public OcrObjectContext {
  std::string m_name;
  std::list<OcrEvtContextPtr> m_depEvts;
  std::list<OcrDbkContextPtr> m_depDbks;
  std::list<SgVarRefExp*> m_depElems;
  OcrEvtContextPtr m_outputEvt;
  SgBasicBlock* m_basicblock;
  std::list<std::string> m_dbksToDestroy;
  std::list<std::string> m_evtsToDestroy;
  SgPragmaDeclaration* m_sgpdecl;
  bool m_finishEdt;
public:
  OcrEdtContext(std::string name, std::list<OcrDbkContextPtr> depDbks,
		std::list<OcrEvtContextPtr> depEvts, std::list<SgVarRefExp*> depElems,
		OcrEvtContextPtr outputEvt, SgBasicBlock* basicblock,
		std::list<std::string> dbksToDestroy, std::list<std::string> evtsToDestroy,
		SgPragmaDeclaration* spgdecl, bool finishEdt);
  std::string get_name() const;
  std::string str() const;
  SgSourceFile* getSourceFile();
  SgBasicBlock* getTaskBasicBlock() const;
  std::list<SgVarRefExp*> getDepElems() const;
  std::list<OcrDbkContextPtr> getDepDbks() const;
  std::list<OcrEvtContextPtr> getDepEvts() const;
  std::list<std::string> getDbksToDestroy() const;
  std::list<std::string> getEvtsToDestroy() const;
  OcrEvtContextPtr getOutputEvt() const;
  SgPragmaDeclaration* getTaskPragma() const;
  unsigned int getNumDepElems() const;
  unsigned int getNumDepDbks() const;
  unsigned int getNumDepEvts() const;
  unsigned int getDepDbkSlotNumber(std::string dbkname) const;
  unsigned int getDepEvtSlotNumber(std::string evtname) const;
  bool isFinishEdt() const;
  ~OcrEdtContext();
};

typedef boost::shared_ptr<OcrEdtContext> OcrEdtContextPtr;

/*************************
 * OcrShutdownEdtContext *
 *************************/
// ocrShutdown() is wrapped inside a shutdown EDT
// Wrapping ocrShutdown inside an EDT allows us add dependent events
// and control the runtime shutting down
// OCR shutdown is specified using the following pragma:
// #pragma ocr shutdown DEP_EVTs(...)
// For each shutdown pragma we will associate a OcrShutdownEdtContext
class OcrShutdownEdtContext : public OcrObjectContext {
  SgPragmaDeclaration* m_shutdownPragma;
  std::list<OcrEvtContextPtr> m_depEvts;
 public:
  OcrShutdownEdtContext(SgPragmaDeclaration* shutdownPragma, std::list<OcrEvtContextPtr> depEvts);
  std::list<OcrEvtContextPtr> getDepEvts() const;
  unsigned int getNumDepEvts();
  std::string str() const;
  SgPragmaDeclaration* getPragma() const;
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
typedef std::map<std::string, OcrEdtContextPtr> OcrEdtObjectMap;
typedef std::pair<std::string, OcrEdtContextPtr> OcrEdtObjectMapElem;
typedef std::map<std::string, OcrDbkContextPtr> OcrDbkObjectMap;
typedef std::pair<std::string, OcrDbkContextPtr> OcrDbkObjectMapElem;
typedef std::map<std::string, OcrEvtContextPtr> OcrEvtObjectMap;
typedef std::pair<std::string, OcrEvtContextPtr> OcrEvtObjectMapElem;
typedef std::map<int, std::string> EdtPragmaOrderMap;
typedef std::pair<int, std::string> EdtPragmaOrderMapElem;
typedef std::list<OcrShutdownEdtContextPtr> OcrShutdownEdtList;
class OcrObjectManager {
  //! Associates an OcrContext for each OcrObject
  //! Key: OcrObject name (string)
  //! Value: OcrContext
  OcrEdtObjectMap m_ocrEdtObjectMap;
  OcrDbkObjectMap m_ocrDbkObjectMap;
  OcrEvtObjectMap m_ocrEvtObjectMap;
  EdtPragmaOrderMap m_edtPragmaOrderMap;
  OcrShutdownEdtList m_ocrShutdownEdtList;
 public:
  OcrObjectManager();
  // Lookup functions for OcrContext using their names
  std::list<OcrEvtContextPtr> getOcrEvtContextList(std::list<std::string> evtList);
  std::list<OcrDbkContextPtr> getOcrDbkContextList(std::list<std::string> dbksList);

  // Functions to create shared_ptr for OcrContext
  std::list<OcrEvtContextPtr> registerOcrEvts(std::list<std::string> evtsNameList);
  OcrEvtContextPtr registerOcrEvt(std::string evtName);
  OcrDbkContextPtr registerOcrDbk(std::string dbkName, SgInitializedName* vdefn, std::list<SgStatement*> allocStmts, SgPragmaDeclaration* pragma);
  OcrEdtContextPtr registerOcrEdt(std::string name, std::list<OcrDbkContextPtr> depDbks,
				  std::list<OcrEvtContextPtr> depEvts, std::list<SgVarRefExp*> depElems,
				  OcrEvtContextPtr outputEvt, SgBasicBlock* basicblock,
				  std::list<std::string> dbksToDestroy, std::list<std::string> evtsToDestroy,
				  SgPragmaDeclaration* spgdecl, bool finishEdt);
  bool registerOcrEdtOrder(int order, std::string edtname);
  bool registerOcrShutdownEdt(SgPragmaDeclaration* shutdownPragma, std::list<OcrEvtContextPtr> depEvts);
  // return a list of edtnames in the same order they were encountered in the AST
  std::list<std::string> getEdtTraversalOrder() const;

  // Access functions
  const OcrEdtObjectMap& getOcrEdtObjectMap() const;
  const OcrDbkObjectMap& getOcrDbkObjectMap() const;
  OcrEdtContextPtr getOcrEdtContext(std::string edtname) const;
  OcrDbkContextPtr getOcrDbkContext(std::string dbkname);
  OcrEvtContextPtr getOcrEvtContext(std::string evtname);
  const OcrShutdownEdtList& getOcrShutdownEdtList() const;
};

#endif

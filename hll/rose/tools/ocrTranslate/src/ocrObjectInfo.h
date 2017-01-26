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
  std::list<SgNode*> m_allocStmts;
public:
  OcrDbkContext(std::string name);
  OcrDbkContext(std::string name, SgInitializedName* vdefn, std::list<SgNode*> allocStmt);
  SgSymbol* getSgSymbol();
  std::string get_name() const;
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
  std::list<OcrEvtContextPtr> m_evtsToSatisfy;
  std::list<SgVarRefExp*> m_depElems;
  std::list<SgStatement*> m_statements;
public:
  OcrEdtContext(std::string name, std::list<OcrEvtContextPtr> depEvts,
		std::list<OcrDbkContextPtr> depDbks, std::list<OcrEvtContextPtr> evtsToSatisfy,
		std::list<SgVarRefExp*> depElems, std::list<SgStatement*> taskStatements);
  std::string get_name() const;
  std::string str() const;
  SgSourceFile* getSourceFile();
  std::list<SgStatement*> getStmtList() const;
  std::list<SgVarRefExp*> getDepElems() const;
  ~OcrEdtContext();
};

typedef boost::shared_ptr<OcrEdtContext> OcrEdtContextPtr;

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
class OcrObjectManager {
  //! Associates an OcrContext for each OcrObject
  //! Key: OcrObject name (string)
  //! Value: OcrContext
  OcrEdtObjectMap m_ocrEdtObjectMap;
  OcrDbkObjectMap m_ocrDbkObjectMap;
  OcrEvtObjectMap m_ocrEvtObjectMap;
 public:
  OcrObjectManager();
  // Lookup functions for OcrContext using their names
  std::list<OcrEvtContextPtr> getOcrEvtContextList(std::list<std::string> evtList);
  std::list<OcrDbkContextPtr> getOcrDbkContextList(std::list<std::string> dbksList);

  // Functions to create shared_ptr for OcrContext
  std::list<OcrEvtContextPtr> registerOcrEvts(std::list<std::string> evtsNameList);
  OcrEvtContextPtr registerOcrEvt(std::string evtName);
  OcrDbkContextPtr registerOcrDbk(std::string dbkName, SgInitializedName* vdefn, std::list<SgNode*> allocStmts);
  OcrEdtContextPtr registerOcrEdt(std::string edtName, std::list<OcrEvtContextPtr> depEvts,
				  std::list<OcrDbkContextPtr> depDbks,
				  std::list<OcrEvtContextPtr> evtsToSatisfy,
				  std::list<SgVarRefExp*> depElems,
				  std::list<SgStatement*> taskStatements);

  // Access functions
  const OcrEdtObjectMap& getOcrEdtObjectMap() const;
};

#endif

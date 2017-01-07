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
  SgDeclarationStatement* m_decl;
  std::list<SgNode*> m_allocStmts;
public:
  OcrDbkContext(std::string name);
  OcrDbkContext(std::string name, SgDeclarationStatement* decl);
  OcrDbkContext(std::string name, SgDeclarationStatement* decl,
		std::list<SgNode*> allocStmt);
  void set_allocStmt(SgNode* allocStmt);
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
  std::list<SgNode*> m_depElems;
  std::list<SgNode*> m_statements;
public:
  OcrEdtContext(std::string name, std::list<OcrEvtContextPtr> depEvts,
		std::list<OcrDbkContextPtr> depDbks, std::list<OcrEvtContextPtr> evtsToSatisfy,
		std::list<SgNode*> depElems, std::list<SgNode*> taskStatements);
  std::string get_name() const;
  std::string str() const;
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
typedef std::map<std::string, OcrObjectContextPtr> OcrObjectMapType;
typedef std::pair<std::string, OcrObjectContextPtr> OcrObjectMapElemType;
typedef std::map<SgSymbol*, std::string> SgSymbolToOcrDbkMap;
class OcrObjectManager {
  //! Associates an OcrContext for each OcrObject
  //! Key: OcrObject name (string)
  //! Value: OcrContext
  OcrObjectMapType m_ocrObjectMap;
  //! Associate variable symbols to their OcrDbk names
  //! Key: SgSymbol*
  //! Value: OcrDbk name (string)
  SgSymbolToOcrDbkMap m_symbolToOcrDbkMap;
 public:
  OcrObjectManager();
  // Lookup functions for OcrContext using their names
  std::list<OcrEvtContextPtr> getOcrEvtContextList(std::list<std::string> evtList);
  std::list<OcrDbkContextPtr> getOcrDbkContextList(std::list<std::string> dbksList);
  // Functions to create shared_ptr for OcrContext
  OcrEvtContextPtr registerOcrEvt(std::string evtName);
  std::list<OcrEvtContextPtr> registerOcrEvts(std::list<std::string> evtsNameList);

  OcrDbkContextPtr registerOcrDbk(std::string dbkName);
  OcrEdtContextPtr registerOcrEdt(std::string edtName, std::list<OcrEvtContextPtr> depEvts,
				  std::list<OcrDbkContextPtr> depDbks,
				  std::list<OcrEvtContextPtr> evtsToSatisfy,
				  std::list<SgNode*> depElems,
				  std::list<SgNode*> taskStatements);
};

#endif

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
public:
  OcrDbkContext(std::string name);
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
  std::list<OcrDbkContextPtr> m_depDBs;
  std::list<OcrEvtContextPtr> m_evtsToSatisfy;
  std::list<SgStatement*> m_statements;
public:
  OcrEdtContext(std::string name);
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
class OcrObjectManager {
  std::map<std::string, OcrObjectContextPtr> m_ocrObjectMap;
 public:
  OcrObjectManager();
};

#endif

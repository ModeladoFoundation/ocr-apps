/*
 * Author: Sriram Aananthakrishnan, 2016 *
 */
#ifndef _OCRPRAGMAPARSING_H
#define _OCRPRAGMAPARSING_H

#include "ocrObjectInfo.h"
#include <boost/xpressive/xpressive_static.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include <exception>

/******************
 * MatchException *
 ******************/
class MatchException : public std::exception {
  std::string m_what;
 public:
  MatchException(std::string what_s);
  virtual const char* what() const throw();
  ~MatchException() throw();
};

/***********************
 * OcrTaskPragmaParser *
 ***********************/
class OcrTaskPragmaParser {
  std::string m_pragmaStr;
  OcrObjectManager& m_ocrObjectManager;
  SgPragmaDeclaration* m_sgpdecl;
  boost::xpressive::sregex identifier, attr, param, paramlist;
  boost::xpressive::sregex taskName, depEvts, depDbks, depElems, outEvts;
  boost::xpressive::sregex taskBeginPragma;
 public:
  OcrTaskPragmaParser(const char* pragmaStr, OcrObjectManager& objectManager, SgPragmaDeclaration* sgpdecl);
 private:
  // All these internal functions throw MatchException which will be
  // caught in the match() function
  bool matchTaskName(std::string input_s, std::string& taskName_s);
  bool matchDepEvts(std::string input_s, std::list<std::string>& depEvtsList);
  bool matchDepDbks(std::string input_s, std::list<std::string>& depDbksList);
  bool matchDepElems(std::string input_s, std::list<std::string>& depElemsList);
  bool matchParamList(std::string input_s, std::list<std::string>& paramList);
  bool matchParam(std::string input_s, std::list<std::string>& paramList);
  bool matchIdentifier(std::string input_s, std::string& identifier_s);
  bool matchEvtsToSatisfy(std::string input, std::list<std::string>& evtsNameToSatisfyList);
  SgNode* identifier2sgn(std::string identifier_);
  std::list<SgNode*> identifiers2sgnlist(std::list<std::string> identifierList);
 public:
  bool match();
  bool isMatchingPragma(SgNode* sgn);
  std::list<SgNode*> collectTaskStatements();
  std::string strlist2str(std::list<std::string>& identifiersList) const;
};

/********************
 * CollectAllocStmt *
 ********************/
class CollectAllocStmt : public AstSimpleProcessing {
  SgNode* m_root;
  std::list<SgNode*> m_allocStmtList;
 public:
  CollectAllocStmt(SgNode* root);
  void visit(SgNode* sgn);
  void atTraversalEnd();
  std::list<SgNode*> getAllocStmt() const;
};


/**********************
 * OcrDbkPragmaParser *
 **********************/
typedef std::map<SgNode*, std::list<SgNode*> > AllocStmtMap;
typedef std::pair<SgNode*, std::list<SgNode*> > AllocStmtMapElem;
class OcrDbkPragmaParser {
  SgPragmaDeclaration* m_sgpdecl;
  OcrObjectManager& m_ocrObjectManager;
  // sregex needed for parsing datablock annotations
  boost::xpressive::sregex identifier, param, paramlist;
  boost::xpressive::sregex dbkNames;
  boost::xpressive::sregex dbkBegin;
  AllocStmtMap allocStmtMapCache;
 private:
  bool matchDbkNames(std::string, std::list<std::string>& dbkNamesList);
  bool matchParamList(std::string, std::list<std::string>& dbkNamesList);
  bool matchParams(std::string, std::list<std::string>& dbkNamesList);
  bool isMatchingPragma(SgNode* sgn);
  SgSymbol* find_symbol(SgNode* sgn);
 public:
  OcrDbkPragmaParser(SgPragmaDeclaration* sgpdecl, OcrObjectManager& objectManager);
  std::list<SgNode*> collectDbkStatements();
  std::list<SgInitializedName*> collectDbkVars();
  std::list<SgNode*> varFilterAllocStmt(std::list<SgNode*>& allocStmtList, SgInitializedName* sgn);
  std::list<SgNode*> collectAllocStmt(SgNode* root);
  std::list<SgNode*> getAllocStmt(SgInitializedName* sgn);
  bool match();
};

/*******************
 * OcrPragmaParser *
 *******************/
class OcrPragmaParser : public AstSimpleProcessing {
 public:
    enum OcrPragmaType {
    e_TaskBegin,
    e_TaskEnd,
    e_DbkBegin,
    e_DbkEnd,
    e_NotOcr
  };
 private:
    OcrObjectManager m_ocrObjectManager;
 public:
  OcrPragmaParser();
  void visit(SgNode* sgn);
  OcrPragmaType identifyPragmaType(std::string pragmaString);
  void astIterate(SgPragmaDeclaration* sgn);
  bool isMatchingPragma(SgNode* sgn);
  void atTraversalEnd();
};

#endif

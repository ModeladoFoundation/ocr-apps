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

/******************************
 * OcrTaskBasicBlockTraversal *
 ******************************/
typedef bool SynthesizedAttribute;
class OcrTaskBasicBlockTraversal : public AstBottomUpProcessing<SynthesizedAttribute> {
  SgNode* m_root;
 public:
  OcrTaskBasicBlockTraversal(SgNode* root);
  bool isTaskPragmaType(std::string pragmaStr);
  SynthesizedAttribute defaultSynthesizedAttribute();
  // Propagates information up the AST
  SynthesizedAttribute evaluateSynthesizedAttribute(SgNode* sgn, SynthesizedAttributesList attrList);
};

/***********************
 * OcrTaskPragmaParser *
 ***********************/
class OcrTaskPragmaParser {
  std::string m_pragmaStr;
  OcrObjectManager& m_ocrObjectManager;
  SgPragmaDeclaration* m_sgpdecl;
  //! in-order traversal order in the AST where the pragma was encountered
  unsigned int m_taskOrder;
  boost::xpressive::sregex identifier, attr, param, paramlist;
  boost::xpressive::sregex taskName, depEvts, depDbks, depElems, outEvts, destroyDbks, destroyEvts;
  boost::xpressive::sregex taskRegEx;
 public:
  OcrTaskPragmaParser(const char* pragmaStr, OcrObjectManager& objectManager,
		      SgPragmaDeclaration* sgpdecl, unsigned int taskOrder);
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
  bool matchOutputEvtList(std::string input, std::list<std::string>& evtsNameToSatisfyList);
  bool matchOutputEvt(std::string input, std::string& outputEvt_s);
  bool matchDestroyDbks(std::string input, std::list<std::string>& objectNamesToDestroy);
  bool matchDestroyEvts(std::string input, std::list<std::string>& objectNamesToDestroy);
  SgVarRefExp* identifier2sgn(std::string identifier_);
  std::list<SgVarRefExp*> identifiers2sgnlist(std::list<std::string> identifierList);
  SgBasicBlock* getTaskBasicBlock();
 public:
  bool match();
  bool isMatchingPragma(SgNode* sgn);
  std::string strlist2str(std::list<std::string>& identifiersList) const;
};

/********************
 * CollectAllocStmt *
 ********************/
class CollectAllocStmt : public AstSimpleProcessing {
  SgNode* m_root;
  std::list<SgStatement*> m_allocStmtList;
 public:
  CollectAllocStmt(SgNode* root);
  void visit(SgNode* sgn);
  void atTraversalEnd();
  std::list<SgStatement*> getAllocStmt() const;
};


/**********************
 * OcrDbkPragmaParser *
 **********************/
typedef std::map<SgNode*, std::list<SgStatement*> > AllocStmtMap;
typedef std::pair<SgNode*, std::list<SgStatement*> > AllocStmtMapElem;
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
  std::list<SgInitializedName*> collectDbkVars(unsigned int ndbks);
  std::list<SgStatement*> varFilterAllocStmt(std::list<SgStatement*>& allocStmtList, SgInitializedName* sgn);
  std::list<SgStatement*> collectAllocStmt(SgNode* root);
  std::list<SgStatement*> getAllocStmt(SgInitializedName* sgn);
  bool match();
};

/***************************
 * OcrShutdownPragmaParser *
 ***************************/
class OcrShutdownPragmaParser {
  SgPragmaDeclaration* m_spgdecl;
  std::string m_input;
  OcrObjectManager& m_ocrObjectManager;
  // sregex needed for matching the pragma
  boost::xpressive::sregex sr_identifier, sr_param, sr_paramlist, sr_depevts;
 private:
  bool matchParams(std::string input, std::list<std::string>& paramList);
 public:
  OcrShutdownPragmaParser(SgPragmaDeclaration* spgdecl, std::string input, OcrObjectManager& ocrObjectManager);
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
    e_shutdown,
    e_NotOcr
  };
 private:
    OcrObjectManager m_ocrObjectManager;
    unsigned int m_taskOrderCounter;
 public:
  OcrPragmaParser();
  void visit(SgNode* sgn);
  const OcrObjectManager& getOcrObjectManager() const;
  OcrPragmaType identifyPragmaType(std::string pragmaString);
  void astIterate(SgPragmaDeclaration* sgn);
  bool isMatchingPragma(SgNode* sgn);
  void atTraversalEnd();
};

#endif

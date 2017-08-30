/*
 * Author: Sriram Aananthakrishnan, 2016 *
 */
#ifndef _OCRPRAGMAPARSING_H
#define _OCRPRAGMAPARSING_H

#include "ocrObjectInfo.h"
#include <boost/xpressive/xpressive_static.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include <exception>
#include <queue>

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

/*****************
 * OcrPragmaType *
 *****************/
struct OcrPragmaType {
  typedef enum {
    e_TaskEdt,
    e_LoopIterEdt,
    e_SpmdRegionEdt,
    e_SpmdFinalizeEdt,
    e_SpmdSend,
    e_SpmdRecv,
    e_SpmdReduce,
    e_Dbk,
    e_ShutdownEdt,
    e_NotOcr
  } OcrPragmaType_t;
  OcrPragmaType_t identifyPragmaType(SgPragmaDeclaration* sgpdecl);
};

/******************************
 * OcrTaskBasicBlockTraversal *
 ******************************/
/*!
 * \brief OcrTaskBasicBlockTraversal is a simple DFS on the basic block of a task annotation.
 *
 * We are interested in the following information of a task basic block.
 * 1. Is the task creating other tasks in its basic block?
 * 2. What are the datablocks created by this task?
 * 3. Are there any function calls that create tasks?
 * We will use the RoseAst::iterator for the search
 */
class OcrTaskBasicBlockTraversal {
  SgBasicBlock* m_basicblock;
  bool m_finishEdt;
  std::list<OcrDbkContextPtr> m_dbksToCreate;
  std::queue<SgNode*> m_queue;
  std::set<SgNode*> m_visited;
 private:
  void insertChildren(SgNode*);
  void bfs_search();
 public:
  OcrTaskBasicBlockTraversal(SgBasicBlock* taskBasicBlock);
  std::list<OcrDbkContextPtr> getDbksToCreate() const;
  bool isFinishEdt() const;
  void traverse();
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
  std::list<OcrDbkContextPtr> m_dbkContextList;
  // sregex needed for parsing datablock annotations
  boost::xpressive::sregex identifier, param, paramlist;
  boost::xpressive::sregex dbkNames;
  boost::xpressive::sregex dbkBegin;
  AllocStmtMap allocStmtMapCache;
  OcrObjectManager& m_ocrObjectManager;
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
  std::list<OcrDbkContextPtr> getDbkContextList() const;
};

/***************************
 * OcrLoopIterPragmaParser *
 ***************************/
class OcrLoopIterPragmaParser {
  SgPragmaDeclaration* m_sgpdecl;
  OcrObjectManager& m_ocrObjectManager;
  unsigned int m_taskOrder;
  boost::xpressive::sregex sr_identifier, sr_param, sr_paramlist;
  boost::xpressive::sregex sr_taskname, sr_depdbks, sr_depevts, sr_depelems, sr_oevent;
  boost::xpressive::sregex sr_loop;
 private:
  std::string matchTaskName(std::string input);
  std::list<std::string> matchParamNames(std::string input);
  std::list<std::string> matchDepDbks(std::string input);
  std::list<std::string> matchDepEvts(std::string input);
  std::list<std::string> matchDepElems(std::string input);
  std::string matchOutEvt(std::string input);
  SgStatement* getLoopStmt(SgPragmaDeclaration* sgpdecl);
 public:
  OcrLoopIterPragmaParser(SgPragmaDeclaration* spgdecl, OcrObjectManager& objectManager, unsigned int taskOrder);
  bool match();
};

/***************************
 * OcrShutdownPragmaParser *
 ***************************/
class OcrShutdownPragmaParser {
  SgPragmaDeclaration* m_sgpdecl;
  std::string m_input;
  unsigned int m_traversalOrder;
  OcrObjectManager& m_ocrObjectManager;
  // sregex needed for matching the pragma
  boost::xpressive::sregex sr_identifier, sr_param, sr_paramlist, sr_depevts;
 private:
  bool matchParams(std::string input, std::list<std::string>& paramList);
 public:
  OcrShutdownPragmaParser(SgPragmaDeclaration* sgpdecl, std::string input, unsigned int traversalOrder, OcrObjectManager& ocrObjectManager);
  bool match();
};

/*****************************
 * OcrSpmdRegionPragmaParser *
 *****************************/
class OcrSpmdRegionPragmaParser {
  SgPragmaDeclaration* m_sgpdecl;
  unsigned int m_traversalOrder;
  OcrObjectManager& m_ocrObjectManager;
  boost::xpressive::sregex sr_identifier, sr_param, sr_paramlist;
 private:
  std::string matchTaskName(std::string input);
  SgExpression* matchNTasks(std::string input);
  std::list<std::string> matchParamNames(std::string input);
  std::list<std::string> matchDepDbks(std::string input);
  std::list<std::string> matchDepEvts(std::string input);
  std::list<std::string> matchDepElems(std::string input);
  std::string matchOutEvt(std::string input);
  SgBasicBlock* getSpmdRegionBasicBlock();
 public:
  OcrSpmdRegionPragmaParser(SgPragmaDeclaration* sgpdecl, std::string input,
			    unsigned int traversalOrder, OcrObjectManager& ocrObjectManager);
  bool match();
};

/*******************************
 * OcrSpmdFinalizePragmaParser *
 *******************************/
class OcrSpmdFinalizePragmaParser {
  SgPragmaDeclaration* m_sgpdecl;
  unsigned int m_traversalOrder;
  OcrObjectManager& m_ocrObjectManager;
  boost::xpressive::sregex sr_identifier, sr_param, sr_paramlist;
 private:
  std::list<std::string> matchParamNames(std::string input);
  std::list<std::string> matchDepEvts(std::string input);
 public:
  OcrSpmdFinalizePragmaParser(SgPragmaDeclaration* sgpdecl, std::string input,
			      unsigned int traversalOrder, OcrObjectManager& ocrObjectManager);
  bool match();
};

/***************************
 * OcrSpmdSendPragmaParser *
 ***************************/
class OcrSpmdSendPragmaParser {
  SgPragmaDeclaration* m_sgpdecl;
  unsigned int m_traversalOrder;
  OcrObjectManager& m_ocrObjectManager;
  boost::xpressive::sregex sr_identifier, sr_param, sr_paramlist;
 private:
  std::string matchSendDbk(std::string input);
  std::list<std::string> matchDepEvts(std::string input);
  std::string matchOutEvt(std::string input);
  std::list<std::string> matchParamNames(std::string input);
 public:
  OcrSpmdSendPragmaParser(SgPragmaDeclaration* sgpdecl,
			  unsigned int traversalOrder, OcrObjectManager& ocrObjectManager);
  bool match();
};

/***************************
 * OcrSpmdRecvPragmaParser *
 ***************************/
class OcrSpmdRecvPragmaParser {
  SgPragmaDeclaration* m_sgpdecl;
  unsigned int m_traversalOrder;
  OcrObjectManager& m_ocrObjectManager;
  boost::xpressive::sregex sr_identifier, sr_param, sr_paramlist;
 private:
  std::string matchRecvEvt(std::string input);
  std::list<std::string> matchDepEvts(std::string input);
  std::string matchOutEvt(std::string input);
  std::list<std::string> matchParamNames(std::string input);
  SgVariableSymbol* getRecvBuffVariableSymbol_rec(SgExpression* rbuffExp);
  SgVariableSymbol* getRecvBuffVariableSymbol(SgFunctionCallExp* recvCallExp);
 public:
  OcrSpmdRecvPragmaParser(SgPragmaDeclaration* sgpdecl,
			  unsigned int traversalOrder, OcrObjectManager& ocrObjectManager);
  bool match();
};

/*****************************
 * OcrSpmdReducePragmaParser *
 *****************************/
class OcrSpmdReducePragmaParser {
  SgPragmaDeclaration* m_sgpdecl;
  unsigned int m_traversalOrder;
  OcrObjectManager& m_ocrObjectManager;
  boost::xpressive::sregex sr_identifier, sr_param, sr_paramlist;
 private:
  std::string matchReduceEvt(std::string input);
  std::list<std::string> matchDepEvts(std::string input);
  std::string matchOutEvt(std::string input);
  std::list<std::string> matchParamNames(std::string input);
  SgVariableSymbol* getReduceBuffVariableSymbol_rec(SgExpression* rbuffExp);
  SgVariableSymbol* getReduceBuffVariableSymbol(SgFunctionCallExp* recvCallExp);
 public:
  OcrSpmdReducePragmaParser(SgPragmaDeclaration* sgpdecl,
			  unsigned int traversalOrder, OcrObjectManager& ocrObjectManager);
  bool match();
};

/*******************
 * OcrPragmaParser *
 *******************/
class OcrPragmaParser : public AstSimpleProcessing {
 private:
    OcrObjectManager m_ocrObjectManager;
    // inorder traversal counter for annotations that will be tasks
    unsigned int m_taskOrderCounter;
    OcrPragmaType m_ocrPragmaType;
 public:
  OcrPragmaParser();
  void visit(SgNode* sgn);
  OcrObjectManager& getOcrObjectManager();
  void atTraversalEnd();
};

#endif

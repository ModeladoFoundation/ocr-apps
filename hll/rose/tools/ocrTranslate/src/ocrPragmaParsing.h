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
  boost::xpressive::sregex identifier, attr, param, paramlist;
  boost::xpressive::sregex taskName, depEvts, depDbks, depElems, outEvts;
  boost::xpressive::sregex taskBeginPragma, taskEndPragma;
 public:
  OcrTaskPragmaParser(const char* pragmaStr);
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
 public:
  bool match();
  std::string strlist2str(std::list<std::string>& identifiersList) const;
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

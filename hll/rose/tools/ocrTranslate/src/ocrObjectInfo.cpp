/*
 * Author: Sriram Aananthakrishnan, 2016
 */

#include "sage3basic.h"
#include "ocrObjectInfo.h"

using namespace std;

/********************
 * OcrObjectContext *
 ********************/
OcrObjectContext::OcrObjectContext() { }
OcrObjectContext::~OcrObjectContext() { }

/*****************
 * OcrDbkContext *
 *****************/
OcrDbkContext::OcrDbkContext(std::string name)
  : OcrObjectContext(),
    m_name(name) { }

string OcrDbkContext::str() const {
  ostringstream oss;
  oss << "[DB: " << m_name << "]\n";
  return oss.str();
}

OcrDbkContext::~OcrDbkContext() {
  // No dynamic memory here to cleanup
}

/*****************
 * OcrEvtContext *
 *****************/
OcrEvtContext::OcrEvtContext(std::string name)
  : OcrObjectContext(),
    m_name(name) { }

string OcrEvtContext::str() const {
  ostringstream oss;
  oss << "[EVT: " << m_name << "]\n";
  return oss.str();
}

OcrEvtContext::~OcrEvtContext() {
  // No dynamic memory here to cleanup
}

/*****************
 * OcrEdtContext *
 *****************/
OcrEdtContext::OcrEdtContext(std::string name) : m_name(name) { }

string OcrEdtContext::str() const {
  ostringstream oss;
  oss << "[EDT: " << m_name << "]\n";
  return oss.str();
}

OcrEdtContext::~OcrEdtContext() {
  // no cleanup required
}

/********************
 * OcrObjectManager *
 ********************/
OcrObjectManager::OcrObjectManager() { }

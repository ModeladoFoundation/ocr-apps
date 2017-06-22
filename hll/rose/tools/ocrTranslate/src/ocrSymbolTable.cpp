/******************************************
 * Author : Sriram Aananthakrishnan, 2017 *
 ******************************************/

#include "sage3basic.h"
#include "ocrSymbolTable.h"
#include <iostream>
#include "ocrObjectInfo.h"
#include "ocrAstInfo.h"
#include "logger.h"

using namespace std;

template<class ObjectType>
OcrObjectSymbolTable<ObjectType>::OcrObjectSymbolTable() { }

template<class ObjectType>
void OcrObjectSymbolTable<ObjectType>::insertObjectPtr(std::string objectKey, boost::shared_ptr<ObjectType> objectPtr, SgScopeStatement* scope) {
  typedef map<string, boost::shared_ptr<ObjectType> > String2OcrObjectMap;
  String2OcrObjectMap& ocrObjectMap = m_symbolTableMap[scope];

  typename String2OcrObjectMap::iterator f = ocrObjectMap.find(objectKey);
  if(f != ocrObjectMap.end()) {
    cerr << "ERROR: OCR Object with key=" << objectKey << " already exists in the map\n";
    std::terminate();
  }
  ocrObjectMap[objectKey] = objectPtr;
}

template<class ObjectType>
boost::shared_ptr<ObjectType> OcrObjectSymbolTable<ObjectType>::getObjectPtr_rec(std::string objectKey, SgScopeStatement* scope) {
  Logger::Logger lg("getObjectPtr_rec");
  Logger::debug(lg) << "objectKey=" << objectKey << ", " << "scope=" << scope << endl;
  // Get the Object map at the current scope
  typedef map<string, boost::shared_ptr<ObjectType> > String2OcrObjectMap;
  String2OcrObjectMap& ocrObjectMap = m_symbolTableMap[scope];

  typename String2OcrObjectMap::iterator f = ocrObjectMap.find(objectKey);
  // Base case of the recusion
  // First check if we have an entry in the current scope
  // If found return
  if(f != ocrObjectMap.end()) return f->second;

  // We did not find an entry in the map
  // If the current scope is global then there is no parent scope
  // base case
  boost::shared_ptr<ObjectType> NullObjectTypePtr;

  if(isSgGlobal(scope)) return NullObjectTypePtr;

  // Get the parent scope
  SgScopeStatement* pscope = SageInterface::getEnclosingScope(scope, false);

  if(pscope == NULL) return NullObjectTypePtr;
  // recurse on the parent scope
  else return getObjectPtr_rec(objectKey, pscope);
}

template<class ObjectType>
boost::shared_ptr<ObjectType> OcrObjectSymbolTable<ObjectType>::getObjectPtr(std::string objectKey, SgScopeStatement* scope) {
  boost::shared_ptr<ObjectType> objectTypePtr = getObjectPtr_rec(objectKey, scope);
  if(!objectTypePtr) {
    cerr << "ERROR: OCR Object with key=" << objectKey << " not found in scope=" << scope <<"\n";
    std::terminate();
  }
  return objectTypePtr;
}

template<class ObjectType>
list<boost::shared_ptr<ObjectType> > OcrObjectSymbolTable<ObjectType>::flatten() const {
  typedef map<string, boost::shared_ptr<ObjectType> > String2OcrObjectMap;
  typedef map<SgScopeStatement*, String2OcrObjectMap> Scope2OcrObjectMap;
  typename Scope2OcrObjectMap::const_iterator om = m_symbolTableMap.begin();
  list<boost::shared_ptr<ObjectType> > objectTypePtrList;
  for( ; om != m_symbolTableMap.end(); ++om) {
    const String2OcrObjectMap& string2OcrObjectMap = om->second;
    if(string2OcrObjectMap.size() > 0) {
      typename String2OcrObjectMap::const_iterator im = string2OcrObjectMap.begin();
      for( ; im != string2OcrObjectMap.end(); ++im) {
  	boost::shared_ptr<ObjectType> objectPtr = im->second;
  	objectTypePtrList.push_back(objectPtr);
      }
    }
  }
  return objectTypePtrList;
}

template<class ObjectType>
string OcrObjectSymbolTable<ObjectType>::str() const {
  ostringstream oss;
  string indent = "  ";
  typedef map<string, boost::shared_ptr<ObjectType> > String2OcrObjectMap;
  typedef map<SgScopeStatement*, String2OcrObjectMap> Scope2OcrObjectMap;
  typename Scope2OcrObjectMap::const_iterator om = m_symbolTableMap.begin();
  for( ; om != m_symbolTableMap.end(); ++om) {
    SgScopeStatement* key = om->first;
    oss << "[key=" << key << " ->\n";
    const String2OcrObjectMap& string2OcrObjectMap = om->second;
    if(string2OcrObjectMap.size() > 0) {
      typename String2OcrObjectMap::const_iterator im = string2OcrObjectMap.begin();
      for( ; im != string2OcrObjectMap.end(); ++im) {
	oss << indent;
	oss << "[" << im->first << ", ";
  	boost::shared_ptr<ObjectType> objectPtr = im->second;
	oss << im->second->str() << "]\n";
      }
    }
    oss << "]-----------\n";
  }
  return oss.str();
}


// Explicit template instantiation
template class OcrObjectSymbolTable<OcrDbkContext>;
template class OcrObjectSymbolTable<DbkAstInfo>;
template class OcrObjectSymbolTable<EvtAstInfo>;

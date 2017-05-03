/******************************************
 * Author : Sriram Aananthakrishnan, 2017 *
 ******************************************/

#include "sage3basic.h"
#include "ocrSymbolTable.h"
#include <iostream>
#include "ocrObjectInfo.h"
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
  // Get the Object map at the current scope
  typedef map<string, boost::shared_ptr<ObjectType> > String2OcrObjectMap;
  String2OcrObjectMap& ocrObjectMap = m_symbolTableMap[scope];

  typename String2OcrObjectMap::iterator f = ocrObjectMap.find(objectKey);
  // Base case of the recusion
  // First check if we have an entry in the current scope
  // If found return
  if(f != ocrObjectMap.end()) return f->second;

  // Get the parent scope
  SgScopeStatement* pscope = SageInterface::getEnclosingScope(scope, false);
  // base case
  boost::shared_ptr<ObjectType> NullObjectTypePtr;
  if(pscope == NULL) return NullObjectTypePtr;
  // recurse on the parent scope
  else return getObjectPtr_rec(objectKey, pscope);
}

template<class ObjectType>
boost::shared_ptr<ObjectType> OcrObjectSymbolTable<ObjectType>::getObjectPtr(std::string objectKey, SgScopeStatement* scope) {
  boost::shared_ptr<ObjectType> objectTypePtr = getObjectPtr_rec(objectKey, scope);
  if(!objectTypePtr) {
    cerr << "ERROR: OCR Object with key=" << objectKey << " not found\n";
    std::terminate();
  }
  return objectTypePtr;
}

// Explicit template instantiation
template class OcrObjectSymbolTable<OcrDbkContext>;

/*****************************************
 * Author: Sriram Aananthakrishnan, 2017 *
 *****************************************/
#ifndef _OCRSYMBOLTABLE_H
#define _OCRSYMBOLTABLE_H

#include <boost/shared_ptr.hpp>
#include <map>

/************************
 * OcrObjectSymbolTable *
 ************************/
/*!
 * \brief Generic container for maintaings OCR Object contexts at the Scope level
 */
template <typename ObjectType>
class OcrObjectSymbolTable {
  std::map<SgScopeStatement*, std::map<std::string, boost::shared_ptr<ObjectType> > > m_symbolTableMap;
 private:
  boost::shared_ptr<ObjectType> getObjectPtr_rec(std::string, SgScopeStatement* scope);
 public:
  OcrObjectSymbolTable();
  void insertObjectPtr(std::string objectKey, boost::shared_ptr<ObjectType> objectPtr, SgScopeStatement* scope);
  boost::shared_ptr<ObjectType> getObjectPtr(std::string, SgScopeStatement* scope);
};
#endif

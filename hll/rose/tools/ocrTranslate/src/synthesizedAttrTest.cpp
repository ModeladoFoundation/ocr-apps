#include "rose.h"
#include <iostream>
#include <sstream>
#include "RoseAst.h"
#include "AstFromString.h"
#include <numeric>

typedef SgNode* InheritedAttribute;
typedef bool SynthesizedAttribute;

class OcrTaskPragmaTraversal {
  std::set<SgPragmaDeclaration*> visited;
  SgProject* m_project;
public:
  OcrTaskPragmaTraversal(SgProject* project);
  bool isFirstVisit(SgPragmaDeclaration* sgpdecl);
  void markVisited(SgPragmaDeclaration* sgpdecl);
  void traverse(SgNode* root);
};

bool isTaskPragmaType(std::string pragmaStr) {
  AstFromString::c_char = pragmaStr.c_str();
  if(AstFromString::afs_match_substr("ocr task")) return true;
  return false;
}

class OcrTaskBasicBlockTraversal : public AstTopDownBottomUpProcessing<InheritedAttribute, SynthesizedAttribute> {
  OcrTaskPragmaTraversal* m_taskPragmaTraversal;
public:
  OcrTaskBasicBlockTraversal(OcrTaskPragmaTraversal* taskPragmaTraversal);
  InheritedAttribute evaluateInheritedAttribute(SgNode* sgn, InheritedAttribute attr);
  SynthesizedAttribute evaluateSynthesizedAttribute(SgNode* sgn, InheritedAttribute attr, SynthesizedAttributesList attrList);
};

OcrTaskBasicBlockTraversal::OcrTaskBasicBlockTraversal(OcrTaskPragmaTraversal* taskPragmaTraversal)
  : m_taskPragmaTraversal(taskPragmaTraversal) {
}

InheritedAttribute OcrTaskBasicBlockTraversal::evaluateInheritedAttribute(SgNode* sgn, InheritedAttribute attr) {
  SgPragmaDeclaration* sgpdecl = isSgPragmaDeclaration(sgn);
  if(sgpdecl) {
    InheritedAttribute parent = attr;
    if(isTaskPragmaType(sgpdecl->get_pragma()->get_pragma())) {
      m_taskPragmaTraversal->markVisited(sgpdecl);
      return (InheritedAttribute)sgpdecl;
    }
    else return parent;
  }
}

SynthesizedAttribute OcrTaskBasicBlockTraversal::evaluateSynthesizedAttribute(SgNode* sgn, InheritedAttribute attr, SynthesizedAttributesList attrList) {
  bool mattr = false;;
  if(SgPragmaDeclaration* spgdecl = isSgPragmaDeclaration(sgn)) {
    if(isTaskPragmaType(spgdecl->get_pragma()->get_pragma())) mattr = true;
  }
  else {
    mattr = std::accumulate(attrList.begin(), attrList.end(), false, std::logical_or<bool>());
  }
  return mattr;
}

OcrTaskPragmaTraversal::OcrTaskPragmaTraversal(SgProject* project)
  : m_project(project) { }

bool OcrTaskPragmaTraversal::isFirstVisit(SgPragmaDeclaration* sgpdecl) {
  return visited.find(sgpdecl) == visited.end();
}

void OcrTaskPragmaTraversal::markVisited(SgPragmaDeclaration* sgpdecl) {
  std::cout << "Marking " << sgpdecl->unparseToString() << std::endl;
  if(visited.find(sgpdecl) == visited.end()) {
    visited.insert(sgpdecl);
  }
}

void OcrTaskPragmaTraversal::traverse(SgNode* project) {
  RoseAst root(project);
  RoseAst::iterator a = root.begin();
  while(a != root.end()) {
    SgPragmaDeclaration* sgpdecl = isSgPragmaDeclaration(*a);
    if(sgpdecl && isFirstVisit(sgpdecl)) {
      if(isTaskPragmaType(sgpdecl->get_pragma()->get_pragma())) {
	// Mark the pragma as visited
	markVisited(sgpdecl);
	// Skip pragma node's children
	a.skipChildrenOnForward();
	// Get the basic block of the task which is the next statement following the pragma
	++a;
	SgBasicBlock* taskBasicBlock = isSgBasicBlock(*a);
	assert(taskBasicBlock);
	OcrTaskBasicBlockTraversal taskBasicBlockTraversal(this);
	InheritedAttribute parent = sgpdecl;
	taskBasicBlockTraversal.traverse(taskBasicBlock, parent);

	// Don't visit the basic block as we use our own traversal to parse the task
	a.skipChildrenOnForward();
      }
    }
    // Go to the next child for all cases
    ++a;
  }
}

int main(int argc, char* argv[]) {
  SgProject* project = frontend(argc, argv);

  OcrTaskPragmaTraversal taskPragmaTraversal(project);
  taskPragmaTraversal.traverse(project);


  return 0;
}

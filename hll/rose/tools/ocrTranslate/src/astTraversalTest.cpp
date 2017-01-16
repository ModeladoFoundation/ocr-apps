#include "sage3basic.h"
#include <iostream>
#include <string>
#include "logger.h"

using namespace std;

class GetEnclosingScope : public AstSimpleProcessing {
public:
  GetEnclosingScope() { }
  void visit(SgNode* sgn);
  void atTraversalEnd();
};

void GetEnclosingScope::visit(SgNode* sgn) {
  if(SgVariableDeclaration* sgvdecl = isSgVariableDeclaration(sgn)) {
    string filename = sgvdecl->get_file_info()->get_filenameString();
    string library = "/usr/include/";
    // Skip the declaration if it is in library
    if(filename.compare(0, library.size(), library) != 0) {
      SgScopeStatement* scope = SageInterface::getEnclosingScope(sgn);
      cout << AstDebug::astTypeName(scope) << endl;
    }
  }
}

void GetEnclosingScope::atTraversalEnd() { }

int main(int argc, char* argv[]) {
  SgProject* project = frontend(argc, argv);

  GetEnclosingScope getScopeVisitor;
  getScopeVisitor.traverse(project, preorder);
}

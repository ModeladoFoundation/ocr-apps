#include "rose.h"

int main(int argc, char* argv[]) {
  SgProject* project = frontend(argc, argv);

  // Generate the DOT graph of the AST for each file
  generateDOT(*project);

  // Generate whole program AST
  #ifdef WHOLE_PROG
  const int max_nodes = 10000;
  generateAstGraph(project, max_nodes);
  #endif

  return 0;
}

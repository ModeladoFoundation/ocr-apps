#include <iostream>

#include "sage3basic.h"
#include "unparser.h"
#include "ocrObjectInfo.h"
#include "ocrPragmaParsing.h"
#include "logger.h"


int main(int argc, char* argv[]) {
  SgProject* project = frontend(argc, argv);

  OcrPragmaParser parser;
  parser.traverse(project, preorder);

  // Run internal consistency tests on AST
  // AstTests::runAllTests(project);

  // OcrTranslateEngine translator(project, parser.getOcrObjectManager());
  // translator.translate();

  unparseProject(project);
  return 0;
}

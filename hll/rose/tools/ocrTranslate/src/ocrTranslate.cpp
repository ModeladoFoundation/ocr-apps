#include <iostream>

#include "sage3basic.h"
#include "unparser.h"
#include "ocrObjectInfo.h"
#include "ocrPragmaParsing.h"
#include "ocrTranslateEngine.h"
#include "logger.h"
#include "AstConsistencyTests.h"

int main(int argc, char* argv[]) {
  SgProject* project = frontend(argc, argv);

  OcrPragmaParser parser;
  parser.traverse(project, preorder);

  OcrObjectManager& ocrObjectManager = parser.getOcrObjectManager();

  OcrTranslator translator(project, ocrObjectManager);
  translator.translate();

  // Run internal consistency tests on AST
  AstTests::runAllTests(project);

  unparseProject(project);
  return 0;
}

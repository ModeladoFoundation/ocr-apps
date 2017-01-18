#include <iostream>

#include "sage3basic.h"
#include "unparser.h"
#include "ocrObjectInfo.h"
#include "ocrPragmaParsing.h"
#include "ocrTranslateEngine.h"
#include "logger.h"


int main(int argc, char* argv[]) {
  SgProject* project = frontend(argc, argv);

  OcrPragmaParser parser;
  parser.traverse(project, preorder);

  const OcrObjectManager& ocrObjectManager = parser.getOcrObjectManager();

  // Run internal consistency tests on AST
  // AstTests::runAllTests(project);

  OcrTranslator translator(project, ocrObjectManager);
  translator.translate();

  unparseProject(project);
  return 0;
}

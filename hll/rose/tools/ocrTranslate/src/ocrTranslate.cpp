#include <iostream>

#include "sage3basic.h"
#include "ocrObjectInfo.h"
#include "ocrPragmaParsing.h"
#include "logger.h"


int main(int argc, char* argv[]) {
  SgProject* project = frontend(argc, argv);

  OcrPragmaParser parser;
  parser.traverse(project, preorder);
}

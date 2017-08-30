/*****************************************
 * Author: Sriram Aananthakrishnan, 2016 *
 *****************************************/
#ifndef _OCRTASKGRAPH_H
#define _OCRTASKGRAPH_H

class OcrObject {
  long m_id;
 public:
  OcrObject(long id);
};

class OcrEdt : public OcrObject {
};

class OcrEvt : public OcrObject {
};

class OcrDataBlock : public OcrObject {
};

#endif

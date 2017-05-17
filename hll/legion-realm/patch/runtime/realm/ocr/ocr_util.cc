/* Copyright 2017 Rice University, Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if USE_OCR_LAYER

#include <assert.h>
#include "ocr_util.h"
#include "extensions/ocr-affinity.h"

namespace Realm {
  /*static*/ ocrHint_t * OCRUtil::ocrHintArr = NULL;

  /*static*/ void OCRUtil::static_init() {
    //create a table of hints indexed using their policy domain rank
    assert(ocrHintArr == NULL);
    int numPD = OCRUtil::ocrNbPolicyDomains();
    ocrHintArr = new ocrHint_t[numPD];
    ocrGuid_t myAffinity;
    ocrHint_t myHNT;
    ocrHintInit(&myHNT, OCR_HINT_EDT_T);
    for(int i=0; i<numPD; i++) {
      ocrAffinityGetAt(AFFINITY_PD, i, &(myAffinity));
      ocrSetHintValue(&myHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(myAffinity));
      ocrHintArr[i] = myHNT;
    }
  }

  /*static*/ void OCRUtil::static_destroy() {
    delete [] ocrHintArr;
    ocrHintArr = NULL;
  }
};

#ifdef USE_GASNET
#ifndef GASNET_PAR
#define GASNET_PAR
#endif
#include <gasnet.h>

namespace Realm {

/*static*/ u64 OCRUtil::ocrNbPolicyDomains() {
  return gasnet_nodes();
}

/*static*/ u64 OCRUtil::ocrCurrentPolicyDomain() {
  return gasnet_mynode();
}

/*static*/ void  OCRUtil::ocrBarrier() {
  //TODO: implement barrier using OCR or use from OCR apps library
  gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);
  gasnet_barrier_wait(0, GASNET_BARRIERFLAG_ANONYMOUS);
}

};

#else // USE_GASNET

namespace Realm {

/*static*/ u64 OCRUtil::ocrNbPolicyDomains() {
  return 1;
}

/*static*/ u64 OCRUtil::ocrCurrentPolicyDomain() {
  return 0;
}

/*static*/ void OCRUtil::ocrBarrier() {
}

};

#endif // USE_GASNET

#endif // USE_OCR_LAYER


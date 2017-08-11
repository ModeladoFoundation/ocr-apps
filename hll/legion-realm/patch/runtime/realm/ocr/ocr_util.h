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

// Utility functions for OCR Realm

#ifndef OCR_REALM_UTIL_IMPL_H
#define OCR_REALM_UTIL_IMPL_H

#if USE_OCR_LAYER

#include <ocr.h>

namespace Realm {

  class OCRUtil {
    public:
      static ocrHint_t * ocrHintArr;

      static void static_init();

      static void static_destroy();

      static u64 ocrNbPolicyDomains();

      static u64 ocrCurrentPolicyDomain();

      static void ocrBarrier();

      static void ocrLegacyBlock(ocrGuid_t dep);
  };
};
#endif // USE_OCR_LAYER

#endif // OCR_REALM_UTIL_IMPL_H

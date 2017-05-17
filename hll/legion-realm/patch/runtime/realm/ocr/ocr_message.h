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

// Internode Messaging implementations for OCR Realm

#ifndef OCR_REALM_MSG_IMPL_H
#define OCR_REALM_MSG_IMPL_H

#if USE_OCR_LAYER

#include "ocr_util.h"
#include "extensions/ocr-affinity.h"

template <class MSGTYPE, void (*FNPTR)(MSGTYPE)>
class MessageHandlerShort {
    static ocrGuid_t ocr_realm_handle_request_func(u32 argc, u64 *argv, u32 depc, ocrEdtDep_t depv[]){
      assert(false);
      return NULL_GUID;
    }

  public:
    static void request(const u32 dest, const MSGTYPE &args) {
      assert(false);
    }
};

template <class MSGTYPE, void (*FNPTR)(MSGTYPE, const void *, size_t)>
class MessageHandlerMedium {

    struct ArgsMsgEDT{
      MSGTYPE args;
      size_t datalen;
      char data[0];
    };

    static ocrGuid_t ocr_realm_handle_request_edt_t;

    static ocrGuid_t ocr_realm_handle_request_func(u32 argc, u64 *argv, u32 depc, ocrEdtDep_t depv[]) {
      assert(argc > 0 && depc == 0);
      //extract args and data, call the handle_request function
      ArgsMsgEDT *args_ptr = (ArgsMsgEDT *)argv;
      FNPTR(args_ptr->args, args_ptr->data, args_ptr->datalen);
      return NULL_GUID;
    }

  public:

    static void static_init() {
      ocrEdtTemplateCreate(&ocr_realm_handle_request_edt_t, ocr_realm_handle_request_func, EDT_PARAM_UNK, EDT_PARAM_UNK);
    }

    static void static_destroy() {
      ocrEdtTemplateDestroy(ocr_realm_handle_request_edt_t);
    }

    static void request(const u32 dest, const MSGTYPE &args,
                        const void *data, const size_t datalen,
                        const int payload_mode) {
      //pack args and data
      int size = sizeof(ArgsMsgEDT) + datalen;
      int argc = U64_COUNT(size);
      u64 argv[argc];
      ArgsMsgEDT *argv_ptr = (ArgsMsgEDT *) argv;
      argv_ptr->args = args;
      argv_ptr->datalen = datalen;
      memcpy(argv_ptr->data, data, datalen);

      //create the EDT on on the dest PD
      ocrGuid_t ocr_realm_handle_request_edt;
      ocrEdtCreate(&ocr_realm_handle_request_edt, ocr_realm_handle_request_edt_t,
        argc, argv, 0, NULL, EDT_PROP_NONE, &(Realm::OCRUtil::ocrHintArr[dest]), NULL);
    }
};

template<class MSGTYPE, void (*FNPTR)(MSGTYPE, const void *, size_t)>
ocrGuid_t MessageHandlerMedium<MSGTYPE, FNPTR>::ocr_realm_handle_request_edt_t = NULL_GUID;
#endif // USE_OCR_LAYER

#endif // OCR_REALM_MSG_IMPL_H

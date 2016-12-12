/* Copyright (C) 2016 Reservoir Labs, Inc. All rights reserved. */

#ifndef ROCR_COMMON_H
#define ROCR_COMMON_H

#include "ocr.h"

// type specialization for the OCR backends
#if (ROCR_OCR_BACKEND == ROCR_OCR_BACKEND_INTEL)
    // type of a policy domain descriptor
    typedef ocrGuid_t ocrPdDesc_t;

#elif (ROCR_OCR_BACKEND == ROCR_OCR_BACKEND_POCR)
    // type of a policy domain descriptor
    typedef int ocrPdDesc_t;

#endif

// special null policy domain
extern ocrPdDesc_t nullPd;

#endif


#ifndef ROCR_ASSERT_H
#define ROCR_ASSERT_H

// Copyright (C) 2012-2015 Reservoir Labs, Inc. All rights reserved.

// PERFECT rights

/**
 * A few assertion macros.
 * @author Benoit Meister <meister@reservoir.com>
 * @author Benoit Pradelle <pradelle@reservoir.com>
*/
#include <stdlib.h>

#include "rocr_config.h"

#if (ROCR_OCR_BACKEND == ROCR_OCR_BACKEND_INTEL)

#define ENABLE_EXTENSION_PARAMS_EVT
#include "ocr.h"
#include "ocr-std.h"

#else

#include "ocr.h"

#endif

// make it compatible w/ standard assert()
#ifndef NDEBUG

// assertion with message
#define assert_msg(expr, msg, ...)\
    do {\
        if (!(expr)) {\
            ocrPrintf("%s:%d ", __FILE__, __LINE__);\
            ocrPrintf(msg, ##__VA_ARGS__);\
            ocrAssert(expr);\
            ocrAbort(1);\
        }\
    } while (0)

#define dbg(msg, ...) ocrPrintf(msg, ##__VA_ARGS__)

#else // NDEBUG

#define assert_msg(expr, msg, ...)
#define dbg(msg, ...)

#endif // NDEBUG

#endif // ROCR_ASSERT_H

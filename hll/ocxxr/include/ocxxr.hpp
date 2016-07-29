#ifndef OCXXR_HPP_
#define OCXXR_HPP_

#if __cplusplus >= 201402L  // C++14
#define OCXXR_USING_CXX14
#endif

extern "C" {
#define ENABLE_EXTENSION_PARAMS_EVT
#define ENABLE_EXTENSION_COUNTED_EVT
#define ENABLE_EXTENSION_CHANNEL_EVT
#define ENABLE_EXTENSION_LABELING
#include <ocr.h>
}

#include <functional>
#include <type_traits>

#ifndef OCXXR_USING_CXX14
// Need C++11 compatibility
#include <ocxxr-internal/ocxxr-cxx11-compat.hpp>
#endif

#include <ocxxr-internal/ocxxr-util.hpp>

#include <ocxxr-internal/ocxxr-handle.hpp>

#include <ocxxr-internal/ocxxr-hint.hpp>

#include <ocxxr-internal/ocxxr-core.hpp>

#include <ocxxr-internal/ocxxr-extension.hpp>

#include <ocxxr-internal/ocxxr-arena.hpp>

#include <ocxxr-internal/ocxxr-relptr.hpp>

#endif  // OCXXR_HPP_

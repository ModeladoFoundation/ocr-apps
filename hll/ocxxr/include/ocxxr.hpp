#ifndef OCXXR_HPP_
#define OCXXR_HPP_
/// @file

#if __cplusplus >= 201402L  // C++14
#define OCXXR_USING_CXX14
#endif

extern "C" {
#define ENABLE_EXTENSION_PARAMS_EVT
#define ENABLE_EXTENSION_COUNTED_EVT
#define ENABLE_EXTENSION_CHANNEL_EVT
#define ENABLE_EXTENSION_LABELING
#define ENABLE_EXTENSION_DB_INFO
#include <ocr.h>
#include <extensions/ocr-db-info.h>
}

#include <cassert>
#include <cstdlib>
#include <functional>
#include <type_traits>
#include <utility>

// Fix problem where disabling assertions causes unused variable warnings
#ifdef NDEBUG
#undef assert
#define assert(x) ((void)sizeof(x))
#endif

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

#if OCXXR_USE_NATIVE_POINTERS
#pragma message "Using native pointers (shared-memory only)"
#endif  // OCXXR_USE_NATIVE_POINTERS

#include <ocxxr-internal/ocxxr-relptr.hpp>

#include <ocxxr-internal/ocxxr-relptr-util.hpp>

#include <ocxxr-internal/ocxxr-task-state.hpp>

/// @brief Convenience macro for creating ocxxr task templates.
/// @param[in] fn_ptr Name of a global function used to run tasks created from
///                   this template. Note that this *must* be a global function
///                   name (not a variable containing an arbitrary function
///                   pointer) because the function is used as a compile-time
///                   argument to the ocxxr::TaskTemplate#Create function.
#define OCXXR_TEMPLATE_FOR(fn_ptr) \
    (::ocxxr::TaskTemplate<decltype(fn_ptr)>::template Create<fn_ptr>())

#endif  // OCXXR_HPP_

#ifndef OCXXR_HPP_
#define OCXXR_HPP_

extern "C" {
#include <ocr.h>
}

#if __cplusplus >= 201402L  // C++14
#define OCXXR_USING_CXX14
#endif

#include <ocxxr-core.hpp>

#endif  // OCXXR_HPP_

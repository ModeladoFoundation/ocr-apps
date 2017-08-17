#ifndef _TEMPEST_REFACTOR_H
#define _TEMPEST_REFACTOR_H

#define TEMPEST_USE_NATIVE_PTRS (OCXXR_USE_NATIVE_POINTERS || (!defined(USE_OCR_TEST) && !defined(USE_OCR)))

#include "ocxxr.hpp"

#if !TEMPEST_USE_NATIVE_PTRS

#include "ocr_relative_ptr.hpp"
//#include "ocr_db_alloc.hpp"
#include "ocr_vector.hpp"

#else

#pragma message "Using native vector in Tempest"

#include <vector>

#endif

namespace TR {

#if !TEMPEST_USE_NATIVE_PTRS

    template<typename T> using Ptr = Ocr::RelPtr<T>;
    template<typename T, size_t N=256> using Vector = Ocr::VectorN<T, N>;

#   define TR_DELETE(p) /* no-op */
#   define TR_ARRAY_DELETE(p) /* no-op */

#else

    template<typename T> using Ptr = T*;
    template<typename T> using Vector = std::vector<T>;

#   if 0
#   define TR_DELETE(p) delete p
#   define TR_ARRAY_DELETE(p) delete[] p
#   else
#   define TR_DELETE(p) /* no-op */
#   define TR_ARRAY_DELETE(p) /* no-op */
#   endif

#endif

};

#endif /* _TEMPEST_REFACTOR_H */

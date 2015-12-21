#ifndef _TEMPEST_REFACTOR_H
#define _TEMPEST_REFACTOR_H

#if defined(USE_OCR) || defined(USE_OCR_TEST)
#include "ocr_relative_ptr.hpp"
#endif

namespace TR {

#ifdef USE_OCR_TEST
    template<typename T> using Ptr = Ocr::RelPtr<T>;
#else
    template<typename T> using Ptr = T*;
#endif

};

#endif /* _TEMPEST_REFACTOR_H */

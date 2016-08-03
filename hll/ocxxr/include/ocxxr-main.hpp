#ifndef OCXXR_MAIN_HPP_
#define OCXXR_MAIN_HPP_

#include <ocxxr.hpp>

extern "C" {
// This should generate a pretty obvious linker error if this header
// is included in multiple source files in the same program.
char Only_include_ocxxr_main_hpp_in_the_source_file_with_your_Main_task = '\0';
}

#include <ocxxr-internal/ocxxr-define-once.inc>

#endif  // OCXXR_MAIN_HPP_

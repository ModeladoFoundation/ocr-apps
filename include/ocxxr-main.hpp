#ifndef OCXXR_MAIN_HPP_
#define OCXXR_MAIN_HPP_

#include <ocxxr.hpp>

namespace ocxxr {

//! Wrapper class for datablock argument of the mainEdt.
class MainTaskArgs {
 public:
    u64 argc() { return getArgc(this); }
    char *argv(u64 index) { return getArgv(this, index); }
};

// prototype for user's main function
void Main(Datablock<MainTaskArgs> args);

}  // namespace ocxxr

extern "C" {
char Only_include_ocxxr_main_hpp_in_the_source_file_with_your_Main_task = '\0';

ocrGuid_t mainEdt(u32 paramc, u64 /*paramv*/[], u32 depc, ocrEdtDep_t depv[]) {
    ASSERT(paramc == 0 && depc == 1);
    ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>(depv[0]));
    return NULL_GUID;
}
}

#endif  // OCXXR_MAIN_HPP_

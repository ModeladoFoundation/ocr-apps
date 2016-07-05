#include <ocxxr.hpp>

extern "C" ocrGuid_t mainEdt(u32 paramc, u64 paramv[], u32 depc,
                             ocrEdtDep_t depv[]) {
    PRINTF("Hello world!\n");
    ocxxr::Shutdown();
    return NULL_GUID;
}

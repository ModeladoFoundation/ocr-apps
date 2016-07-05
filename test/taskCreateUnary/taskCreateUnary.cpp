#include <ocxxr.hpp>

static constexpr u32 kPayload = 7;

ocxxr::NullHandle ChildTask(ocxxr::AcquiredDatablock<u32> arg) {
    PRINTF("Child task ran (arg=%d)! Shutting down...\n", arg.data());
    ASSERT(arg.data() == kPayload);
    ocxxr::Shutdown();
    return ocxxr::NullHandle();
}

extern "C" ocrGuid_t mainEdt(u32 paramc, u64 paramv[], u32 depc,
                             ocrEdtDep_t depv[]) {
    PRINTF("Creating child task\n");
    auto datablock = ocxxr::AcquiredDatablock<u32>();
    datablock.data() = kPayload;
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    task_template.CreateTask(datablock);
    return NULL_GUID;
}

#include <ocxxr.hpp>

static constexpr float kPayload = 7.65f;

ocxxr::NullHandle ChildTask(ocxxr::AcquiredDatablock<double> arg) {
    PRINTF("Child task ran! (arg=%d)\n", arg.data());
    ASSERT(arg.data() == kPayload);
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
    return ocxxr::NullHandle();
}

extern "C" ocrGuid_t mainEdt(u32 paramc, u64 paramv[], u32 depc,
                             ocrEdtDep_t depv[]) {
    PRINTF("Creating child task\n");
    auto datablock = ocxxr::AcquiredDatablock<float>();
    datablock.data() = kPayload;
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    task_template.CreateTask(datablock);
    return NULL_GUID;
}

#include <ocxxr.hpp>

static constexpr u32 kPayload = 765;

void ChildTask(ocxxr::AcquiredDatablock<u32> arg) {
    PRINTF("Child task ran! (arg=%d)\n", arg.data());
    ASSERT(arg.data() == kPayload);
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::AcquiredDatablock<ocxxr::MainTaskArgs>) {
    PRINTF("Creating child task\n");
    auto datablock = ocxxr::AcquiredDatablock<u32>::Create();
    datablock.data() = kPayload;
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    task_template.CreateTask(datablock);
}

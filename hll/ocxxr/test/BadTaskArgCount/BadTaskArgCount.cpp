#include <ocxxr-main.hpp>

static constexpr u32 kPayload = 765;

void ChildTask(ocxxr::Datablock<u32> arg) {
    PRINTF("Child task ran! (arg=%d)\n", arg.data());
    assert(arg.data() == kPayload);
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    PRINTF("Creating child task\n");
    auto datablock = ocxxr::Datablock<u32>::Create();
    datablock.data() = kPayload;
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    task_template().CreateTaskPartial(datablock, datablock);
}

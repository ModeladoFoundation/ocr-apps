#include <ocxxr-main.hpp>

static constexpr u32 kPayload = 765;

void ChildTask(ocxxr::Datablock<u32> arg) {
    PRINTF("Child task ran! (arg=%d)\n", arg.data());
    assert(arg.data() == kPayload);
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    auto datablock = ocxxr::Datablock<u32>::Create();
    datablock.data() = kPayload;
    PRINTF("Creating child task...\n");
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    auto task = task_template().CreateTaskPartial();
    PRINTF("Adding dependence...\n");
    task.DependOn<0>(datablock);
}

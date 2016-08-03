#include <ocxxr-main.hpp>

static constexpr int kPayload = 765;

void ChildTask(ocxxr::DatablockList<int> args) {
    auto arg = args[0];
    PRINTF("Child task ran! (arg=%d)\n", arg.data());
    ASSERT(arg.data() == kPayload);
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    PRINTF("Creating child task\n");
    auto datablock = ocxxr::Datablock<int>::Create();
    datablock.data() = kPayload;
    ocxxr::DatablockList<int> deps(1);
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    task_template().CreateTask(deps.Add(datablock));
}

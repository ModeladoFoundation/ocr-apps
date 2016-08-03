#include <ocxxr-main.hpp>

static constexpr float kPayload = 7.65f;

void ChildTask(ocxxr::Datablock<double> arg) {
    PRINTF("Child task ran! (arg=%f)\n", arg.data());
    ASSERT(arg.data() == kPayload);
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    auto datablock = ocxxr::Datablock<float>::Create();
    datablock.data() = kPayload;
    PRINTF("Creating child task...\n");
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    auto task = task_template().CreateTaskPartial();
    PRINTF("Adding dependence...\n");
    task.DependOn<0>(datablock);
}

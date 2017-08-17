#include <ocxxr-main.hpp>

static constexpr double kPayload = 7.65;

void ChildTask(ocxxr::Datablock<double> arg) {
    PRINTF("Child task ran with event! (arg=%.2f)\n", arg.data());
    assert(kPayload == arg.data());
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    PRINTF("Creating child task...\n");
    auto event = ocxxr::StickyEvent<double>::Create();
    auto datablock = ocxxr::Datablock<double>::Create();
    datablock.data() = kPayload;
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    task_template().CreateTask(event);
    event.Satisfy(datablock);
}

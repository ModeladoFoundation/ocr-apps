#include <ocxxr.hpp>

static constexpr double kPayload = 7.65;

void ChildTask(ocxxr::AcquiredDatablock<double> arg) {
    PRINTF("Child task ran with event! (arg=%.2f)\n", arg.data());
    ASSERT(kPayload == arg.data());
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::AcquiredDatablock<ocxxr::MainTaskArgs>) {
    PRINTF("Creating child task...\n");
    auto event = ocxxr::StickyEvent<double>();
    auto datablock = ocxxr::AcquiredDatablock<double>();
    datablock.data() = kPayload;
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    task_template.CreateTask(event);
    event.Satisfy(datablock);
}

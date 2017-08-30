#include <ocxxr-main.hpp>

constexpr double kPayload = 123.45;

ocxxr::NullHandle ChildTask(double param1, double param2) {
    PRINTF("Child task ran! (param=%.2f)\n", param1 + param2);
    assert(param1 == kPayload);
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
    return ocxxr::NullHandle();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    PRINTF("Creating child task\n");
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    task_template().CreateFuturePartial(kPayload, kPayload);
}

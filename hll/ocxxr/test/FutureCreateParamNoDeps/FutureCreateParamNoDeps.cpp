#include <ocxxr-main.hpp>

constexpr double kPayload = 123.45;

ocxxr::NullHandle ChildTask(double param, ocxxr::Datablock<void>) {
    PRINTF("Child task ran! (param=%.2f)\n", param);
    ASSERT(param == kPayload);
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
    return ocxxr::NullHandle();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    PRINTF("Creating child task\n");
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    auto future = task_template().CreateFuturePartial(kPayload);
    future.task().DependOn<0>(NullHandle());
}

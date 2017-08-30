#include <ocxxr-main.hpp>

void ChildTask(ocxxr::Datablock<u32>) {
    PRINTF("Child task ran! (BAD!)\n");
    assert(false);
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    PRINTF("Creating child task...\n");
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    auto task = task_template().CreateTaskPartial();
    PRINTF("Destroying child task...\n");
    task.Destroy();
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
}

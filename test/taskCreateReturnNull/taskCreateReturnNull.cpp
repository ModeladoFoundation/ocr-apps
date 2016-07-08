#include <ocxxr-main.hpp>

ocxxr::NullHandle ChildTask() {
    PRINTF("Child task ran!\n");
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
    return ocxxr::NullHandle();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    PRINTF("Creating child task\n");
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    task_template.CreateTask();
}

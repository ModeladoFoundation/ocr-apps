#include <ocxxr.hpp>

void ChildTask() {
    PRINTF("Child task ran!\n");
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::AcquiredDatablock<ocxxr::MainTaskArgs>) {
    PRINTF("Creating child task\n");
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    task_template.CreateTask();
}

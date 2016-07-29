#include <ocxxr-main.hpp>

void ChildTask(ocxxr::Datablock<void>) {
    PRINTF("Child task ran with event!\n");
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    PRINTF("Creating child task...\n");
    auto event = ocxxr::StickyEvent<void>::Create();
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    task_template().CreateTask(event);
    event.Satisfy();
}

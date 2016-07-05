#include <ocxxr.hpp>

void ChildTask(ocxxr::AcquiredDatablock<void>) {
    PRINTF("Child task ran with event!\n");
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::AcquiredDatablock<ocxxr::MainTaskArgs>) {
    PRINTF("Creating child task...\n");
    auto event = ocxxr::Event<void>(OCR_EVENT_STICKY_T);
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    task_template.CreateTask(event);
    event.Satisfy();
}

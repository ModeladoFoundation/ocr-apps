#include <ocxxr-main.hpp>

void ChildTask(ocxxr::Arena<int>) {
    PRINTF("Running child task\n");
    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    auto arena = ocxxr::ArenaHandle<float>::Create(1024);
    PRINTF("Creating child task\n");
    task_template().CreateTask(arena);
}

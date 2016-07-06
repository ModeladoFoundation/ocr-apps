#include <ocxxr.hpp>

ocxxr::Datablock<char> TaskA(ocxxr::AcquiredDatablock<char> arg) {
    char label[] = {arg.data(), '\0'};
    PRINTF("Running task %s...\n", label);
    arg.data() = 'B';
    return arg;
}

void TaskB(ocxxr::AcquiredDatablock<char> arg) {
    char label[] = {arg.data(), '\0'};
    PRINTF("Running task %s...\n", label);
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::AcquiredDatablock<ocxxr::MainTaskArgs>) {
    PRINTF("Creating child tasks...\n");
    // set up task A
    auto template_a = OCXXR_TEMPLATE_FOR(TaskA);
    auto task_arg = ocxxr::AcquiredDatablock<char>();
    task_arg.data() = 'A';
    auto future = template_a.CreateFuture(task_arg);
    // set up task B
    auto template_b = OCXXR_TEMPLATE_FOR(TaskB);
    template_b.CreateTask(future.event());
    // release task A (future)
    future.Release();
}

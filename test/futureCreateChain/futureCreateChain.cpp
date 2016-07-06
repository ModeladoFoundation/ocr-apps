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
    auto template_a = OCXXR_TEMPLATE_FOR(TaskA);
    auto template_b = OCXXR_TEMPLATE_FOR(TaskB);
    auto out_event = ocxxr::OnceEvent<char>();
    template_b.CreateTask(out_event);
    auto task_arg = ocxxr::AcquiredDatablock<char>();
    task_arg.data() = 'A';
    template_a.CreateFuture(out_event, task_arg);
}

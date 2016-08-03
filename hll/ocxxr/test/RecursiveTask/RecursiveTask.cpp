#include <ocxxr-main.hpp>

constexpr u32 kTaskDepth = 10;

struct TaskParams;
void RecursiveTask(TaskParams &params);

struct TaskParams {
    u32 n;
    ocxxr::TaskTemplate<decltype(RecursiveTask)> task_template;
};

void RecursiveTask(TaskParams &params) {
    PRINTF("Running RecursiveTask %" PRIu32 "\n", params.n);
    if (params.n == kTaskDepth) {  // Base case
        PRINTF("Done!\n");
        params.task_template.Destroy();
        ocxxr::Shutdown();
    } else {  // Recursive case
        params.n += 1;
        params.task_template().CreateTask(params);
    }
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    PRINTF("Starting main task...\n");

    auto task_template = OCXXR_TEMPLATE_FOR(RecursiveTask);
    TaskParams params = {0, task_template};
    task_template().CreateTask(params);
}

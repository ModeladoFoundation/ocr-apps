#include <ocxxr-main.hpp>

static constexpr int kPayload = 765;

void ChildTask(ocxxr::Datablock<int> arg1, ocxxr::DatablockList<int> vargs) {
    auto arg2 = vargs[0];
    PRINTF("Child task ran! (arg1=%d, arg2=%d)\n", *arg1, *arg2);
    ASSERT(*arg1 == kPayload);
    ASSERT(*arg1 == *arg2);
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    PRINTF("Creating child task\n");
    auto datablock = ocxxr::Datablock<int>::Create();
    datablock.data() = kPayload;
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    auto task = task_template().CreateTaskPartial(1);
    task.DependOn<0>(datablock);
    task.DependOnWithinList(0, datablock);
}

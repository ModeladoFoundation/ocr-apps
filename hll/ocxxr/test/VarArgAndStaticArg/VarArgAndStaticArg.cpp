#include <ocxxr-main.hpp>

static constexpr int kPayload = 765;

void ChildTask(ocxxr::Datablock<int> arg1, ocxxr::DatablockList<int> vargs) {
    auto arg2 = vargs[0];
    PRINTF("Child task ran! (arg1=%d, arg2=%d)\n", *arg1, *arg2);
    assert(*arg1 == kPayload);
    assert(*arg1 == *arg2);
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    PRINTF("Creating child task\n");
    auto datablock = ocxxr::Datablock<int>::Create();
    datablock.data() = kPayload;
    ocxxr::DatablockList<int> deps(1);
    deps.Add(datablock);
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    task_template().CreateTask(datablock, deps);
}

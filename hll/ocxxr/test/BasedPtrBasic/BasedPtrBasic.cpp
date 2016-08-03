#include <ocxxr-main.hpp>

constexpr int kPayload = 25;

void ChildTask(ocxxr::Datablock<ocxxr::BasedPtr<int>> db, ocxxr::Arena<void>) {
    PRINTF("Running child task\n");
    int actual = **db;
    PRINTF("%d vs %d\n", actual, kPayload);
    ASSERT(actual == kPayload);
    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    auto db = ocxxr::Datablock<ocxxr::BasedPtr<int>>::Create();
    auto arena = ocxxr::Arena<void>::Create(100 * sizeof(int));
    for (int i = 0; i < 50; i++) {
        int* p = arena.New<int>(i);
        if (i == kPayload) {
            PRINTF("Setting based pointer value\n");
            *db = p;
        }
    }
    db.Release();
    arena.Release();
    PRINTF("Creating child task\n");
    task_template().CreateTask(db, arena);
}

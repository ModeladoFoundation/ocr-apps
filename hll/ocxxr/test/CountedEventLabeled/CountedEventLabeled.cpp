#include <unistd.h>
#include <ocxxr-main.hpp>

void MyTask(u64 id, ocxxr::Datablock<void>) {
    PRINTF("Task %" PRIu32 " ran.\n", id);
}

void EndTask(ocxxr::Datablock<void>) {
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    constexpr u64 kDepCount = 4;
    auto constexpr flags =
            ocxxr::Properties::kLabeled | ocxxr::Properties::kBlocking;
    // 1st counted labeled event
    PRINTF("Creating labeled counted event...\n");
    auto range = ocxxr::HandleRange<ocxxr::CountedEvent<void>>::Create(1);
    auto event1 = ocxxr::CountedEvent<void>::Create(kDepCount, flags, range[0]);
    event1.Satisfy();
    // consumer tasks
    PRINTF("Creating tasks...\n");
    auto task_template = OCXXR_TEMPLATE_FOR(MyTask);
    for (u64 i = 0; i < kDepCount; i++) {
        task_template().CreateTask(i, event1);
    }
    usleep(100000);
    // 2nd counted labeled event
    auto event2 = ocxxr::CountedEvent<void>::Create(kDepCount, flags, range[0]);
    event2.Satisfy();
    // end task
    auto end_template = OCXXR_TEMPLATE_FOR(EndTask);
    end_template().CreateTask(event2);
}

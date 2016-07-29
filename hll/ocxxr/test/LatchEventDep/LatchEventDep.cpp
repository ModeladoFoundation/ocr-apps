#include <ocxxr-main.hpp>

void GoodTask(ocxxr::Datablock<void>) {
    PRINTF("Good task ran with latch event!\n");
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
}

void BadTask(ocxxr::Datablock<void>) {
    PRINTF("Bad task ran with unsatisfied latch event!\n");
    PRINTF("Abort!\n");
    ocxxr::Abort(-1);
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    // Good latch
    constexpr u64 kCount = 2;
    PRINTF("Creating good latch(%" PRIu64 ")...\n", kCount);
    auto good_latch = ocxxr::LatchEvent<void>::Create(kCount);
    // Bad latch (never triggered)
    constexpr u64 kBadCount = kCount + 1;
    PRINTF("Creating bad latch(%" PRIu64 ")...\n", kBadCount);
    auto bad_latch = ocxxr::LatchEvent<void>::Create(kBadCount);
    // Bad task should not run because its latch is never triggered
    PRINTF("Creating bad task...\n");
    auto bad_template = OCXXR_TEMPLATE_FOR(BadTask);
    bad_template().CreateTask(bad_latch);
    // Good task should run when latch triggers
    PRINTF("Creating good task...\n");
    auto good_template = OCXXR_TEMPLATE_FOR(GoodTask);
    good_template().CreateTask(good_latch);
    // Latches down to 1, 2
    PRINTF("Latch down...\n");
    good_latch.Down();
    bad_latch.Down();
    // Latch down to 0, 1
    PRINTF("Latch down...\n");
    good_latch.Down();
    bad_latch.Down();
}

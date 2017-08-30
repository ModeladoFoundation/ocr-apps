#include <unistd.h>
#include <ocxxr-main.hpp>

void ConsumerTask(s32 expected, ocxxr::Datablock<s32> actual) {
    PRINTF("Consumer %" PRIu32 " got value %" PRIu32 ".\n", expected,
           actual.data());
    assert(expected == actual.data());
    if (expected == 0) {
        PRINTF("Shutting down...\n");
        ocxxr::Shutdown();
    }
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    constexpr s32 kTaskCount = 5;
    // Good latch
    PRINTF("Creating channel event...\n");
    auto channel = ocxxr::ChannelEvent<s32>::Create(kTaskCount + 1);
    // Good task should run when latch triggers
    PRINTF("Creating tasks...\n");
    auto task_template = OCXXR_TEMPLATE_FOR(ConsumerTask);
    for (s32 i = kTaskCount; i >= 0; i--) {
        task_template().CreateTask(i, channel);
    }
    // Messages on channel
    for (s32 i = kTaskCount; i >= 0; i--) {
        auto value = ocxxr::Datablock<s32>::Create();
        value.data() = i;
        channel.Satisfy(value);
        usleep(50000);
    }
}

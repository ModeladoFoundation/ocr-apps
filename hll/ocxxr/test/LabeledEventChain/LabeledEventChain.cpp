#include <ocxxr-main.hpp>

static constexpr u32 kTaskCount = 4;
typedef ocxxr::HandleRange<ocxxr::StickyEvent<u32>> RangeType;

void ChainedTask(RangeType range, ocxxr::Datablock<u32> id) {
    PRINTF("Task %" PRIu32 " ran!\n", id.data());
    if (id.data() == kTaskCount) {
        // Last task in the chain shuts down
        PRINTF("Shutting down...\n");
        ocxxr::Shutdown();
    } else {
        // Run the next task
        u32 next = id.data();
        id.data() += 1;
        range[next].Satisfy(id);
    }
}

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>) {
    auto constexpr flags = ocxxr::Properties::kLabeled;
    // Create labeled events
    PRINTF("Creating %" PRIu32 " tasks and labeled events\n", kTaskCount);
    auto task_template = OCXXR_TEMPLATE_FOR(ChainedTask);
    auto range = RangeType::Create(kTaskCount);
    for (u32 i = 0; i < kTaskCount; i++) {
        auto event = ocxxr::StickyEvent<u32>::Create(flags, range[i]);
        task_template().CreateTask(range, event);
    }
    // Start with task 1
    auto id = Datablock<u32>::Create();
    id.data() = 1;
    range[0].Satisfy(id);
}

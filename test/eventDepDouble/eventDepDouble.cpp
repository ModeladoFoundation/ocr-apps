#include <ocxxr.hpp>

static constexpr double kPayload = 7.65;

ocxxr::NullHandle ChildTask(ocxxr::AcquiredDatablock<double> arg) {
    PRINTF("Child task ran with event! (arg=%.2f)\n", arg.data());
    ASSERT(kPayload == arg.data());
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
    return ocxxr::NullHandle();
}

extern "C" ocrGuid_t mainEdt(u32 paramc, u64 paramv[], u32 depc,
                             ocrEdtDep_t depv[]) {
    PRINTF("Creating child task...\n");
    auto event = ocxxr::Event<double>(OCR_EVENT_STICKY_T);
    auto datablock = ocxxr::AcquiredDatablock<double>();
    datablock.data() = kPayload;
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    task_template.CreateTask(event);
    event.Satisfy(datablock);
    return NULL_GUID;
}

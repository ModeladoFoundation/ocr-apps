#include <ocxxr.hpp>

ocxxr::NullHandle ChildTask() {
    PRINTF("Child task ran!\n");
    PRINTF("Shutting down...\n");
    ocxxr::Shutdown();
    return ocxxr::NullHandle();
}

extern "C" ocrGuid_t mainEdt(u32 paramc, u64 paramv[], u32 depc,
                             ocrEdtDep_t depv[]) {
    PRINTF("Creating child task\n");
    auto task_template = OCXXR_TEMPLATE_FOR(ChildTask);
    task_template.CreateTask();
    return NULL_GUID;
}

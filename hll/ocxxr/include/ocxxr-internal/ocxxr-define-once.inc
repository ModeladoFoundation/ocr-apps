//===============================================
// OCR main function (entry point)
//===============================================

extern "C" {
ocrGuid_t mainEdt(u32 paramc, u64 /*paramv*/[], u32 depc, ocrEdtDep_t depv[]) {
    ASSERT(paramc == 0 && depc == 1);
    ocxxr::internal::PushTaskState();
    ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>(depv[0]));
    ocxxr::internal::PopTaskState();
    return NULL_GUID;
}
}

namespace ocxxr {
namespace internal {

OCXXR_THREAD_LOCAL TaskLocalState *_task_local_state;

}  // namespace internal
}  // namespace ocxxr

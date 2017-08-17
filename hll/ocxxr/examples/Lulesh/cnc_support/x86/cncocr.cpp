#include "ocxxr.hpp"

extern "C" {
#include "cncocr_internal.h"

#ifdef CNC_DEBUG_LOG
FILE *cncDebugLog;
#endif /* CNC_DEBUG_LOG */

static ocrGuid_t _shutdownEdt(u32 paramc, u64 paramv[], u32 depc, ocrEdtDep_t depv[]) {
#ifdef CNC_DEBUG_LOG
    fclose(cncDebugLog);
#endif /* CNC_DEBUG_LOG */
    ocxxr::Shutdown();
    return NULL_GUID;
}

void cncAutomaticShutdown(ocrGuid_t doneEvent) {
    ocrGuid_t shutdownEdtGuid, templGuid;
    ocrEdtTemplateCreate(&templGuid, _shutdownEdt, 0, 1);
    ocrHint_t hint;
    ocrEdtCreate(&shutdownEdtGuid, templGuid,
        /*paramc=*/EDT_PARAM_DEF, /*paramv=*/NULL,
        /*depc=*/EDT_PARAM_DEF, /*depv=*/&doneEvent,
        /*properties=*/EDT_PROP_NONE,
        /*hint=*/_cncCurrentEdtAffinityHint(&hint), /*outEvent=*/NULL);
    ocrEdtTemplateDestroy(templGuid);
}

void *cncItemAlloc(size_t count) {
    ocrGuid_t handle;
    CnCItemMeta *meta;
    // add space for our meta-data
    const size_t metaBytes = count + sizeof(*meta);
    // allocate datablock
    // XXX - do I need to check for busy (and do a retry)?
    //ocrDbCreate(&handle, (void**)&meta, metaBytes, DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    _CNC_DBCREATE(&handle, (void**)&meta, metaBytes);
    ASSERT(meta && "ERROR: out of memory");
    // store guid
    meta->guid = handle;
    // store item size
    meta->size = count;
    // clear released flag
    meta->releasedFlag = 0;
#ifdef CNC_DEBUG
    // set cookie / released flag bit
    meta->cookie = _CNC_ITEM_COOKIE;
#endif
    // return offset user pointer
    return &meta->data;
}

void cncItemFree(void *itemPtr) {
    if (itemPtr) {
        _cncItemCheckCookie(itemPtr);
        ocrDbDestroy(_cncItemGuid(itemPtr));
    }
}

// ocxxr indirect calls

void ocxxr_start_task(void) {
    ocxxr::internal::PushTaskState();
}

void ocxxr_end_task(void) {
    ocxxr::internal::PopTaskState();
}

void ocxxr_add_db(ocrEdtDep_t *dep) {
    ocxxr::internal::bookkeeping::AddDatablock(dep->guid, dep->ptr);
}

void ocxxr_remove_db(ocrEdtDep_t *dep) {
    ocxxr::internal::bookkeeping::RemoveDatablock(dep->guid);
}

} // extern C

#ifndef OCXXR_TASK_STATE_HPP_
#define OCXXR_TASK_STATE_HPP_

#include <algorithm>
#include <cstddef>
#include <limits>

#ifdef __APPLE__
// For some reason they don't support the standard C++ thread_local,
// but they *do* support the non-standard __thread extension for C.
#define OCXXR_THREAD_LOCAL __thread
#else
#define OCXXR_THREAD_LOCAL thread_local
#endif /* __APPLE__ */

namespace ocxxr {
namespace internal {

//===============================================
// Task-local state
//===============================================

namespace bookkeeping {

class DbPair {
 public:
    DbPair() = default;

    DbPair(ocrGuid_t guid, ptrdiff_t base_addr)
            : guid_(guid), base_addr_(base_addr) {}

    DbPair(ocrGuid_t guid, void *base_addr)
            : DbPair(guid, reinterpret_cast<ptrdiff_t>(base_addr)) {}

    explicit DbPair(ocrGuid_t guid) : DbPair(guid, nullptr) {}
    explicit DbPair(ptrdiff_t base_addr) : DbPair(ERROR_GUID, base_addr) {}

    static bool CompareGuids(const DbPair &a, const DbPair &b) {
        return ocrGuidIsLt(a.guid_, b.guid_);
    }

    static bool CompareBases(const DbPair &a, const DbPair &b) {
        return a.base_addr_ < b.base_addr_;
    }

    ocrGuid_t guid() const { return guid_; }

    ptrdiff_t base_addr() const { return base_addr_; }

 private:
    ocrGuid_t guid_;
    ptrdiff_t base_addr_;
};

#ifndef OCXXR_MAX_DB_ACQUIRE_COUNT
#define OCXXR_MAX_DB_ACQUIRE_COUNT 256
#endif

struct AcquiredDbInfo {
    u32 acquired_db_count;

    // TODO - make these arrays more flexible (currently fixed at 256 slots)
    // this is also currently very inefficient (lots of resorting),
    // so I should really just re-implement this with a better data structure,
    // e.g., a splay tree

    DbPair dbs_by_guid[OCXXR_MAX_DB_ACQUIRE_COUNT];  // sorted by guid
    DbPair dbs_by_addr[OCXXR_MAX_DB_ACQUIRE_COUNT];  // sorted by base address

    DbPair *dbs_by_guid_start() { return &dbs_by_guid[0]; }

    DbPair *dbs_by_guid_end() { return &dbs_by_guid[acquired_db_count]; }

    DbPair *dbs_by_addr_start() { return &dbs_by_addr[0]; }

    DbPair *dbs_by_addr_end() { return &dbs_by_addr[acquired_db_count]; }
};

}  // namespace bookkeeping

struct TaskLocalState {
    bookkeeping::AcquiredDbInfo acquired_dbs;
    dballoc::DatablockAllocator arena_allocator;
    TaskLocalState *parent;
};

// defined in ocxxr-define-once.inc
extern OCXXR_THREAD_LOCAL TaskLocalState *_task_local_state;

/* Note: The push/pop task state functions are currently necessary because the
 * Traleika Glacier OCR implementation uses a "work-shifting" strategy to try
 * to do some useful work while an EDT is blocked. This behavior is triggered
 * by legacy blocking constructs, as well as for remote operations in x86-mpi.
 */

inline void PushTaskState() {
    TaskLocalState *parent_state = _task_local_state;
    _task_local_state = OCXXR_TEMP_NEW_ZERO(TaskLocalState);
    bookkeeping::AcquiredDbInfo *db_info = &_task_local_state->acquired_dbs;
    ASSERT(db_info->acquired_db_count == 0);  // should do zero-init
    _task_local_state->parent = parent_state;
}

inline void PopTaskState() {
    TaskLocalState *child_state = _task_local_state;
    _task_local_state = _task_local_state->parent;
    OCXXR_TEMP_DELETE(child_state);
}

//===============================================
// Datablock bookkeeping
//===============================================

namespace bookkeeping {

inline void AddDatablock(ocrGuid_t guid, void *base_address) {
    // don't track NULL_GUID
    if (!base_address) return;
    bookkeeping::AcquiredDbInfo *db_info = &_task_local_state->acquired_dbs;
    // don't add duplicate GUIDs
    auto g_start = db_info->dbs_by_guid_start();
    auto g_end = db_info->dbs_by_guid_end();
    if (!std::binary_search(g_start, g_end, DbPair(guid),
                            DbPair::CompareGuids)) {
        ASSERT(db_info->acquired_db_count < OCXXR_MAX_DB_ACQUIRE_COUNT &&
               "Acquired too many datablocks in one task");
        // OK, this is a new GUID that we should actually track...
        DbPair new_info(guid, base_address);
        // add to by_guid array
        *(g_end++) = new_info;
        std::sort(g_start, g_end, DbPair::CompareGuids);
        // add to by_addr array
        auto a_start = db_info->dbs_by_addr_start();
        auto a_end = db_info->dbs_by_addr_end();
        *(a_end++) = new_info;
        std::sort(a_start, a_end, DbPair::CompareBases);
        // now we have one more
        ++db_info->acquired_db_count;
    }
}

inline void RemoveDatablock(ocrGuid_t guid) {
    if (ocrGuidIsNull(guid)) return;
    bookkeeping::AcquiredDbInfo *db_info = &_task_local_state->acquired_dbs;
    auto g_start = db_info->dbs_by_guid_start();
    auto g_end = db_info->dbs_by_guid_end();
    auto i = std::lower_bound(g_start, g_end, DbPair(guid),
                              DbPair::CompareGuids);
    ASSERT(ocrGuidIsEq(guid, i->guid()) &&
           "Released untracked non-null datablock");
    ptrdiff_t base_addr = i->base_addr();
    std::swap(i, --g_end);
    std::sort(g_start, g_end, DbPair::CompareGuids);
    auto a_start = db_info->dbs_by_addr_start();
    auto a_end = db_info->dbs_by_addr_end();
    auto j = std::lower_bound(a_start, a_end, DbPair(base_addr),
                              DbPair::CompareBases);
    ASSERT(base_addr == j->base_addr() &&
           "Matching base address for datablock GUID not found");
    std::swap(j, --a_end);
    std::sort(a_start, a_end, DbPair::CompareBases);
}

}  // namespace bookkeeping

inline ptrdiff_t AddressForGuid(ocrGuid_t guid) {
    using bookkeeping::DbPair;
    ASSERT(!ocrGuidIsNull(guid) && "Should not query for NULL_GUID");
    bookkeeping::AcquiredDbInfo *db_info = &_task_local_state->acquired_dbs;
    auto g_start = db_info->dbs_by_guid_start();
    auto g_end = db_info->dbs_by_guid_end();
    auto i = std::lower_bound(g_start, g_end, DbPair(guid),
                              DbPair::CompareGuids);
    ASSERT(i != g_end && "Lookup of untracked non-null datablock");
    ASSERT(ocrGuidIsEq(guid, i->guid()) && "Datablock lookup mismatch");
    return i->base_addr();
}

inline void GuidOffsetForAddress(const void *target, const void *source,
                                 ocrGuid_t *guid_out, ptrdiff_t *offset_out) {
    using bookkeeping::DbPair;
    // All three cases are handled internally by this function call.
    // optimized case: treat as intra-datablock RelPtr
    // normal case: inter-datablock pointer
    // specail case: nullptr => NULL_GUID
    if (target == nullptr) {
        *guid_out = NULL_GUID;
        *offset_out = 0;
    } else {
        // Find closest (<=) base address with binary search
        bookkeeping::AcquiredDbInfo *db_info = &_task_local_state->acquired_dbs;
        auto a_start = db_info->dbs_by_addr_start();
        auto a_end = db_info->dbs_by_addr_end();
        ptrdiff_t dst_addr = reinterpret_cast<ptrdiff_t>(target);
        ptrdiff_t src_addr = reinterpret_cast<ptrdiff_t>(source);
        auto j = std::upper_bound(a_start, a_end, DbPair(dst_addr),
                                  DbPair::CompareBases);
        ASSERT(j != a_start);
        auto i = j - 1;
        ptrdiff_t end_addr = (j == a_end)
                                     ? std::numeric_limits<ptrdiff_t>::max()
                                     : j->base_addr();
        ASSERT(i != a_end && i->base_addr() <= dst_addr &&
               dst_addr <= end_addr &&
               "Based pointer must point into an acquired datablock");
        // output results
        if (i->base_addr() <= src_addr && src_addr <= end_addr) {
            // optimized case: treat as intra-datablock RelPtr
            *guid_out = UNINITIALIZED_GUID;
            *offset_out = dst_addr - src_addr;
        } else {
            // normal case: inter-datablock pointer
            *guid_out = i->guid();
            *offset_out = dst_addr - i->base_addr();
        }
    }
}

//===============================================
// Arena support
//===============================================

namespace dballoc {
/// Get the current implicit arena allocator
inline DatablockAllocator &AllocatorGet(void) {
    return _task_local_state->arena_allocator;
}

/// Set the implicit arena allocator
inline void AllocatorSetDb(void *dbPtr) {
    ::new (&_task_local_state->arena_allocator) DatablockAllocator(dbPtr);
}

}  // namespace dballoc
}  // namespace internal
}  // namespace ocxxr

#endif  // OCXXR_TASK_STATE_HPP_

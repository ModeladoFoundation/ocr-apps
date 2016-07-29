#ifndef OCXXR_ARENA_HPP_
#define OCXXR_ARENA_HPP_

#include <cassert>
#include <cstddef>
#include <memory>

// TODO - Rename variables (imported from older repo)
// TODO - Get rid of C-style casts

namespace ocxxr {

namespace internal {

namespace dballoc {

struct AllocatorState {
    ptrdiff_t offset;
};

struct DbArenaHeader {
    s64 size;
    ptrdiff_t offset;
};

class DatablockAllocator {
 private:
    char *const m_dbBuf;
    DbArenaHeader *const m_info;

 public:
    constexpr DatablockAllocator(void) : m_dbBuf(nullptr), m_info(nullptr) {}

    DatablockAllocator(void *dbPtr)
            : m_dbBuf((char *)dbPtr), m_info((DbArenaHeader *)dbPtr) {}

    AllocatorState saveState(void) { return {m_info->offset}; }

    void restoreState(AllocatorState state) { m_info->offset = state.offset; }

    inline void *allocateAligned(size_t size, int alignment) const {
        assert(m_info != nullptr && "Uninitialized allocator");
        const ptrdiff_t offset = m_info->offset;
        ptrdiff_t start = (offset + alignment - 1) & (-alignment);
        m_info->offset = start + size;
        assert(m_info->offset <= m_info->size &&
               "Datablock allocator overflow");
        return (void *)&m_dbBuf[start];
    }

    // FIXME - I don't know if these alignment checks are sufficient,
    // especially if we do weird things like allocate a char[N]
    // so we have N bytes to store some struct or something.
    inline void *allocate(size_t size, size_t count) const {
        assert(size > 0);
        if (size == 1) {
            return allocateAligned(1 * count, 1);
        } else if (size <= 4) {
            return allocateAligned(size * count, 4);
        } else if (size <= 8) {
            return allocateAligned(size * count, 8);
        } else {
            return allocateAligned(size * count, 16);
        }
    }

    inline void *allocate(size_t size) const { return allocate(size, 1); }
};

DatablockAllocator &AllocatorGet(void);
void AllocatorSetDb(void *dbPtr);
void AllocatorDbInit(void *dbPtr, size_t dbSize);

}  // namespace dballoc
}  // namespace internal

// ArenaState management
using ArenaState = internal::dballoc::DbArenaHeader;
using AllocatorState = internal::dballoc::DatablockAllocator;

namespace internal {
namespace dballoc {

static inline void InitializeArena(void *dbPtr, size_t dbSize) {
    internal::dballoc::AllocatorDbInit(dbPtr, dbSize);
}

static inline void SetCurrentArena(void *dbPtr) {
    internal::dballoc::AllocatorSetDb(dbPtr);
}

static inline DatablockAllocator &GetCurrentAllocator(void) {
    return internal::dballoc::AllocatorGet();
}

template <typename T>
static inline T &GetArenaRoot(void *dbPtr) {
    typedef internal::dballoc::DbArenaHeader HT;
    HT *header = (HT *)dbPtr;
    return *(T *)&header[1];
}

// New

template <typename T, typename... Ts>
static inline T *NewIn(internal::dballoc::DatablockAllocator arena,
                       Ts &&... args) {
    auto mem = arena.allocate(sizeof(T));
    return ::new (mem) T(std::forward<Ts>(args)...);
}

// Constructor helper for NewArray

template <typename T, typename NoInit = void>
struct TypeInitializer {
    static inline void init(T &target) { ::new (::std::addressof(target)) T(); }
};

template <typename T>
struct TypeInitializer<
        T, typename std::enable_if<std::is_scalar<T>::value>::type> {
    static inline void init(T &) {}
};

// NewArray

template <typename T>
static inline T *NewArrayIn(internal::dballoc::DatablockAllocator arena,
                            size_t count) {
    T *data = reinterpret_cast<T *>(arena.allocate(sizeof(T), count));
    for (size_t i = 0; i < count; i++) {
        TypeInitializer<T>::init(data[i]);
    }
    return data;
}

}  // namespace dballoc
}  // namespace internal

template <typename T, typename... Ts>
static inline T *New(Ts &&... args) {
    auto arena = internal::dballoc::AllocatorGet();
    return internal::dballoc::NewIn<T, Ts...>(arena, std::forward<Ts>(args)...);
}

template <typename T>
static inline T *NewArray(size_t count) {
    auto arena = internal::dballoc::AllocatorGet();
    return internal::dballoc::NewArrayIn<T>(arena, count);
}

template <typename T>
class ArenaHandle : public DatablockHandle<ArenaState> {
 public:
    explicit ArenaHandle(u64 bytes) : ArenaHandle(nullptr, bytes, nullptr) {}

    explicit ArenaHandle(u64 bytes, const DatablockHint &hint)
            : ArenaHandle(nullptr, bytes, &hint) {}

    explicit ArenaHandle(ArenaState **data_ptr, u64 bytes,
                         const DatablockHint *hint)
            : DatablockHandle<ArenaState>(data_ptr, bytes + sizeof(ArenaState),
                                          hint) {
        ASSERT(bytes >= sizeof(T) &&
               "Arena must be big enough to hold root object");
    }

    explicit ArenaHandle(ocrGuid_t guid = NULL_GUID)
            : DatablockHandle<ArenaState>(guid) {}

    // create a datablock, but don't acquire it.
    static ArenaHandle Create(u64 bytes) { return ArenaHandle(bytes); }
};

template <typename T>
class Arena : public AcquiredData {
 public:
    explicit Arena(u64 bytes) : Arena(nullptr, bytes, nullptr) {}

    explicit Arena(u64 bytes, const DatablockHint &hint)
            : Arena(nullptr, bytes, &hint) {}

    // This version gets called from the task setup code
    explicit Arena(ocrEdtDep_t dep)
            : handle_(dep.guid), state_(static_cast<ArenaState *>(dep.ptr)) {}

    // Create empty arena datablock
    explicit Arena(std::nullptr_t np = nullptr)
            : handle_(NULL_GUID), state_(np) {}

    static Arena<T> Create(u64 bytes) { return Arena<T>(bytes); }

    template <typename U = T, internal::EnableIfNotVoid<U> = 0>
    U &data() const {
        // The template type U is only here to get enable_if to work.
        static_assert(std::is_same<T, U>::value, "Template types must match.");
        return *data_ptr();
    }

    void *base_ptr() const { return state_; }

    s64 size() const { return state_->size; }

    T *data_ptr() const { return &internal::dballoc::GetArenaRoot<T>(state_); }

    bool is_null() const { return state_ == nullptr; }

    ArenaHandle<T> handle() const { return handle_; }

    void Release() const { internal::OK(ocrDbRelease(this->guid())); }

    operator ArenaHandle<T>() const { return handle_; }

    template <typename U, typename... Args>
    U *New(Args &&... args) {
        return internal::dballoc::NewIn<U, Args...>(
                *state_, std::forward<Args>(args)...);
    }

    template <typename U>
    U *NewArray(size_t count) {
        return internal::dballoc::NewArrayIn<U>(*state_, count);
    }

    template <typename U>
    friend void SetImplicitArena(Arena<U> arena);

 private:
    Arena(ArenaState *tmp, u64 bytes, const DatablockHint *hint)
            : handle_(&tmp, bytes, hint), state_(tmp) {
        internal::dballoc::AllocatorDbInit(state_, bytes);
    }

    const ArenaHandle<T> handle_;
    ArenaState *const state_;
};

template <typename T>
static inline void SetImplicitArena(Arena<T> arena) {
    internal::dballoc::SetCurrentArena(arena.state_);
}

}  // namespace ocxxr

#endif  // OCXXR_ARENA_HPP_

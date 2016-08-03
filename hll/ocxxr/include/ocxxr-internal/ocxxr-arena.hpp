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
            : m_dbBuf(reinterpret_cast<char *>(dbPtr)),
              m_info(reinterpret_cast<DbArenaHeader *>(dbPtr)) {}

    AllocatorState saveState(void) { return {m_info->offset}; }

    void restoreState(AllocatorState state) { m_info->offset = state.offset; }

    inline void *allocateAligned(size_t size, int alignment) const {
        assert(m_info != nullptr && "Uninitialized allocator");
        const ptrdiff_t offset = m_info->offset;
        ptrdiff_t start = (offset + alignment - 1) & (-alignment);
        m_info->offset = start + size;
        assert(m_info->offset <= m_info->size &&
               "Datablock allocator overflow");
        return &m_dbBuf[start];
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

// defined in ocxxr-task-state.hpp
inline DatablockAllocator &AllocatorGet(void);

// defined in ocxxr-task-state.hpp
inline void AllocatorSetDb(void *dbPtr);

inline void AllocatorDbInit(void *dbPtr, size_t dbSize) {
    DbArenaHeader *const info = static_cast<DbArenaHeader *>(dbPtr);
    assert(dbSize >= sizeof(*info) && "Datablock is too small for allocator");
    info->size = dbSize;
    info->offset = sizeof(*info);
}

}  // namespace dballoc
}  // namespace internal

// ArenaState management
using AllocatorState = internal::dballoc::DatablockAllocator;

template <typename T>
struct ArenaState {
    internal::dballoc::DbArenaHeader header;
};

namespace internal {
namespace dballoc {

inline void InitializeArena(void *dbPtr, size_t dbSize) {
    internal::dballoc::AllocatorDbInit(dbPtr, dbSize);
}

inline void SetCurrentArena(void *dbPtr) {
    internal::dballoc::AllocatorSetDb(dbPtr);
}

inline DatablockAllocator &GetCurrentAllocator(void) {
    return internal::dballoc::AllocatorGet();
}

template <typename T>
T &GetArenaRoot(void *dbPtr) {
    auto header = static_cast<internal::dballoc::DbArenaHeader *>(dbPtr);
    return *reinterpret_cast<T *>(&header[1]);
}

// New

template <typename T, typename... Ts>
T *NewIn(internal::dballoc::DatablockAllocator arena, Ts &&... args) {
    auto mem = arena.allocate(sizeof(T));
    return ::new (mem) T(std::forward<Ts>(args)...);
}

// Constructor helper for NewArray

template <typename T, typename NoInit = void>
struct TypeInitializer {
    static void init(T &target) { ::new (::std::addressof(target)) T(); }
};

template <typename T>
struct TypeInitializer<
        T, typename std::enable_if<std::is_scalar<T>::value>::type> {
    static void init(T &) {}
};

// NewArray

template <typename T>
T *NewArrayIn(internal::dballoc::DatablockAllocator arena, size_t count) {
    T *data = reinterpret_cast<T *>(arena.allocate(sizeof(T), count));
    for (size_t i = 0; i < count; i++) {
        TypeInitializer<T>::init(data[i]);
    }
    return data;
}

}  // namespace dballoc
}  // namespace internal

template <typename T, typename... Ts>
T *New(Ts &&... args) {
    auto arena = internal::dballoc::AllocatorGet();
    return internal::dballoc::NewIn<T, Ts...>(arena, std::forward<Ts>(args)...);
}

template <typename T>
T *NewArray(size_t count) {
    auto arena = internal::dballoc::AllocatorGet();
    return internal::dballoc::NewArrayIn<T>(arena, count);
}

template <typename T>
class ArenaHandle : public DatablockHandle<ArenaState<T>> {
 public:
    explicit ArenaHandle(u64 bytes) : ArenaHandle(nullptr, bytes, nullptr) {}

    explicit ArenaHandle(u64 bytes, const DatablockHint &hint)
            : ArenaHandle(nullptr, bytes, &hint) {}

    explicit ArenaHandle(ArenaState<T> **data_ptr, u64 bytes,
                         const DatablockHint *hint)
            : DatablockHandle<ArenaState<T>>(
                      data_ptr, bytes + sizeof(ArenaState<T>), hint) {
        ASSERT(bytes >= sizeof(internal::SizeOf<T>) &&
               "Arena must be big enough to hold root object");
    }

    explicit ArenaHandle(ocrGuid_t guid = NULL_GUID)
            : DatablockHandle<ArenaState<T>>(guid) {}

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
            : handle_(dep.guid), state_(static_cast<ArenaState<T> *>(dep.ptr)) {
        internal::bookkeeping::AddDatablock(dep.guid, dep.ptr);
    }

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

    template <typename U = T, internal::EnableIfNotVoid<U> = 0>
    U &operator*() const {
        return data<U>();
    }

    void *base_ptr() const { return state_; }

    s64 size() const { return state_->header.size; }

    T *data_ptr() const { return &internal::dballoc::GetArenaRoot<T>(state_); }

    T *operator->() const { return data_ptr(); }

    bool is_null() const { return state_ == nullptr; }

    ArenaHandle<T> handle() const { return handle_; }

    void Release() const { internal::OK(ocrDbRelease(handle_.guid())); }

    operator ArenaHandle<T>() const { return handle(); }

    template <typename U, typename... Args>
    U *New(Args &&... args) {
        auto alloc = internal::dballoc::DatablockAllocator(state_);
        return internal::dballoc::NewIn<U, Args...>(
                alloc, std::forward<Args>(args)...);
    }

    template <typename U>
    U *NewArray(size_t count) {
        auto alloc = internal::dballoc::DatablockAllocator(state_);
        return internal::dballoc::NewArrayIn<U>(alloc, count);
    }

    template <typename U>
    friend void SetImplicitArena(Arena<U> arena);

 private:
    Arena(ArenaState<T> *tmp, u64 bytes, const DatablockHint *hint)
            : handle_(&tmp, bytes, hint), state_(tmp) {
        internal::dballoc::AllocatorDbInit(state_, bytes);
    }

    const ArenaHandle<T> handle_;
    ArenaState<T> *const state_;
};

template <typename T>
void SetImplicitArena(Arena<T> arena) {
    internal::dballoc::SetCurrentArena(arena.state_);
}

namespace internal {

template <typename T>
struct Unpack<Arena<T>> {
    typedef ArenaState<T> Parameter;
};

}  // namespace internal
}  // namespace ocxxr

#endif  // OCXXR_ARENA_HPP_

#ifndef OCXXR_ARENA_HPP_
#define OCXXR_ARENA_HPP_

#include <cstddef>
#include <memory>

// TODO - Rename variables (imported from older repo)
// TODO - Get rid of C-style casts

namespace ocxxr {

struct AllocatorState {
    ptrdiff_t offset;
};

namespace internal {

namespace dballoc {

struct DbArenaHeader {
    s64 size;
    ptrdiff_t offset;

    AllocatorState SaveState(void) { return {offset}; }

    void RestoreState(AllocatorState state) { offset = state.offset; }
};

class DatablockAllocator {
 private:
    char *const buffer_;
    DbArenaHeader *state_;

 public:
    constexpr DatablockAllocator(void) : buffer_(nullptr), state_(nullptr) {}

    DatablockAllocator(void *dbPtr)
            : buffer_(reinterpret_cast<char *>(dbPtr)),
              state_(reinterpret_cast<DbArenaHeader *>(dbPtr)) {}

    AllocatorState SaveState(void) { return state_->SaveState(); }

    void RestoreState(AllocatorState state) { state_->RestoreState(state); }

    inline void *allocateAligned(size_t size, int alignment) const {
        assert(state_ != nullptr && "Uninitialized allocator");
        const ptrdiff_t offset = state_->offset;
        ptrdiff_t start = (offset + alignment - 1) & (-alignment);
        const ptrdiff_t offset_prime = start + size;
        if (offset_prime > state_->size) {
            // return NULL if allocation fails
            return nullptr;
        } else {
            state_->offset = offset_prime;
            return &buffer_[start];
        }
    }

    // FIXME - I don't know if these alignment checks are sufficient,
    // especially if we do weird things like allocate a char[N]
    // so we have N bytes to store some struct or something.
    // FIXME - use alignof() operator to do this correctly,
    // and make a CreateFor<T>() for arenas to better calculate sizes
    // based on a struct that you'd want to store inside,
    // which would help calculate the needed alignment padding
    inline void *allocate(size_t size, size_t count) const {
        assert(size > 0);
        if (size == 1) {
            return allocateAligned(1 * count, 1);
        } else if (size <= 4) {
            return allocateAligned(size * count, 4);
        } else if (size <= 8) {
            return allocateAligned(size * count, 8);
        } else {
            // FIXME - 16-byte alignment was causing problems, so now everything
            // is 8-byte aligned
            return allocateAligned(size * count, 8);
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
    if (!mem) return nullptr;
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
    if (data) {
        for (size_t i = 0; i < count; i++) {
            TypeInitializer<T>::init(data[i]);
        }
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
    static constexpr size_t kHeaderSize =
            sizeof(internal::dballoc::DbArenaHeader);

    explicit ArenaHandle(u64 bytes) : ArenaHandle(nullptr, bytes, nullptr) {}

    explicit ArenaHandle(u64 bytes, const DatablockHint &hint)
            : ArenaHandle(nullptr, bytes, &hint) {}

    explicit ArenaHandle(ArenaState<T> **data_ptr, u64 bytes,
                         const DatablockHint *hint)
            : DatablockHandle<ArenaState<T>>(
                      data_ptr, bytes + sizeof(ArenaState<T>), hint) {
        assert(bytes >= sizeof(internal::SizeOf<T>) &&
               "Arena must be big enough to hold root object");
    }

    explicit ArenaHandle(ocrGuid_t guid = NULL_GUID)
            : DatablockHandle<ArenaState<T>>(guid) {}

    // XXX - are we actually using this?
    operator ArenaHandle<void>() const {
        return *reinterpret_cast<ArenaHandle<void> *>(
                const_cast<ArenaHandle<T> *>(this));
    }

    // create an arena, but don't acquire it.
    static ArenaHandle Create(u64 bytes) {
        return ArenaHandle(kHeaderSize + bytes);
    }

    // this is just here for supporting Unpack
    DatablockHandle<ArenaState<T>> handle() const {
        return *reinterpret_cast<DatablockHandle<ArenaState<T>> *>(
                const_cast<ArenaHandle<T> *>(this));
    }
};

template <typename T>
class Arena : public AcquiredData {
 public:
    static constexpr size_t kHeaderSize = ArenaHandle<T>::kHeaderSize;

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

    static Arena<T> Create(u64 bytes) { return Arena<T>(kHeaderSize + bytes); }

    static Arena<T> CreateArray(u64 count) {
        return Arena<T>(kHeaderSize + sizeof(T) * count);
    }

    template <typename U = T, internal::EnableIfNotVoid<U> = 0>
    U &data() const {
        // The template type U is only here to get enable_if to work.
        static_assert(std::is_same<T, U>::value, "Template types must match.");
        return *data_ptr();
    }

    template <typename U = T, internal::EnableIfNotVoid<U> = 0>
    U &operator*() const {
        // The template type U is only here to get enable_if to work.
        static_assert(std::is_same<T, U>::value, "Template types must match.");
        return data<U>();
    }

    void *base_ptr() const { return state_; }

    s64 size() const { return state_->header.size; }

    T *data_ptr() const { return &internal::dballoc::GetArenaRoot<T>(state_); }

    T *operator->() const { return data_ptr(); }

    bool is_null() const { return state_ == nullptr; }

    ArenaHandle<T> handle() const { return handle_; }

    void Release() const {
        internal::OK(ocrDbRelease(handle_.guid()));
        internal::bookkeeping::RemoveDatablock(handle_.guid());
    }

    operator ArenaHandle<T>() const { return handle(); }

    Arena<void> Untyped() const {
        return *reinterpret_cast<Arena<void> *>(const_cast<Arena<T> *>(this));
    }

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

    /// Destroy this Arena.
    void Destroy() const { handle_.Destroy(); }

    AllocatorState SaveState(void) { return state_->header.SaveState(); }

    void RestoreState(AllocatorState state) {
        state_->header.RestoreState(state);
    }

    void *end_ptr() {
        char *buffer = reinterpret_cast<char *>(state_);
        return reinterpret_cast<void *>(&buffer[state_->header.offset]);
    }

    template <typename U>
    friend void SetImplicitArena(Arena<U> arena);

 private:
    Arena(ArenaState<T> *tmp, u64 bytes, const DatablockHint *hint)
            : handle_(&tmp, bytes, hint), state_(tmp) {
        internal::dballoc::AllocatorDbInit(state_, bytes);
    }

    ArenaHandle<T> handle_;
    ArenaState<T> *state_;
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

template <typename T>
using ArenaList = DatablockList<T, Arena>;

}  // namespace ocxxr

#endif  // OCXXR_ARENA_HPP_

#ifndef OCXXR_H_
#define OCXXR_H_

#include <functional>
#include <type_traits>

extern "C" {
#include <ocr.h>
}

namespace ocxxr {

namespace internal {

template <typename T>
struct IsLegalHandle {
    // Check if a type can be reinterpreted to/from ocrGuid_t
    static constexpr bool value = std::is_trivially_copyable<T>::value &&
                                  sizeof(T) == sizeof(ocrGuid_t);
};

// Check error status of C API call
inline void OK(u8 status) { ASSERT(status == 0); }
}

using namespace internal;

//! Abstract base class for all OCR objects with GUIDs.
class ObjectHandle {
 public:
    ocrGuid_t guid() const { return guid_; }

    bool is_null() const { return ocrGuidIsNull(guid_); }
    bool is_uninitialized() const { return ocrGuidIsUninitialized(guid_); }
    bool is_error() const { return ocrGuidIsError(guid_); }

    bool operator==(const ObjectHandle &rhs) const {
        return ocrGuidIsEq(guid_, rhs.guid_);
    }
    bool operator!=(const ObjectHandle &rhs) const { return !(*this == rhs); }

    bool operator<(const ObjectHandle &rhs) const {
        return ocrGuidIsLt(guid_, rhs.guid_);
    }
    bool operator>(const ObjectHandle &rhs) const { return rhs < *this; }
    bool operator<=(const ObjectHandle &rhs) const { return !(*this > rhs); }
    bool operator>=(const ObjectHandle &rhs) const { return !(*this < rhs); }

    ~ObjectHandle() = default;

 protected:
    explicit ObjectHandle(ocrGuid_t guid) : guid_(guid) {}

 private:
    const ocrGuid_t guid_;
};

static_assert(IsLegalHandle<ObjectHandle>::value,
              "ObjectHandle must be castable to/from ocrGuid_t.");

// FIXME - this can't be its own class
// I'll need a Null() function on each of the concrete handle classes
class NullHandle : public ObjectHandle {
 public:
    NullHandle() : ObjectHandle(NULL_GUID) {}
};

static_assert(IsLegalHandle<ObjectHandle>::value,
              "NullHandle must be castable to/from ocrGuid_t.");

void Shutdown() { ocrShutdown(); }

void Abort(u8 error_code) { ocrAbort(error_code); }

//! Wrapper class for datablock argument of the mainEdt.
class MainTaskArgs {
 public:
    u64 argc() { return getArgc(this); }
    char *argv(u64 index) { return getArgv(this, index); }
};

// TODO - add Hint subclasses: TaskHint, DatablockHint
class Hint {
 public:
    const ocrHint_t *internal() const { return &hint_; }

 private:
    ocrHint_t hint_;
};

//! Datablock creation.
template <typename T>
class Datablock : public ObjectHandle {
 public:
    // TODO - add overloaded versions supporting hint, etc.
    explicit Datablock(u64 count = 1)
            : ObjectHandle(init(sizeof(T) * count, false, nullptr)) {}

    explicit Datablock(T **data_ptr, u64 count = 1)
            : ObjectHandle(init(data_ptr, sizeof(T) * count, true, nullptr)) {}

    void Destroy() const { OK(ocrDbDestroy(guid())); }

 protected:
    explicit Datablock(ocrGuid_t guid) : ObjectHandle(guid) {}

 private:
    static ocrGuid_t init(u64 bytes, const Hint *hint) {
        T **data_ptr;
        return init(&data_ptr, bytes, false, hint);
    }

    static ocrGuid_t init(T **data_ptr, u64 bytes, bool acquire,
                          const Hint *hint) {
        ocrGuid_t guid;
        const u16 flags = acquire ? DB_PROP_NONE : DB_PROP_NO_ACQUIRE;
        // TODO - open bug for adding const qualifiers in OCR C API.
        // E.g., "const ocrHint_t *hint" in ocrDbCreate.
        ocrHint_t *raw_hint = const_cast<ocrHint_t *>(hint->internal());
        void **raw_data_ptr = reinterpret_cast<void **>(data_ptr);
        OK(ocrDbCreate(&guid, raw_data_ptr, bytes, flags, raw_hint, NO_ALLOC));
        return guid;
    }
};

static_assert(IsLegalHandle<Datablock<int>>::value,
              "Datablock must be castable to/from ocrGuid_t.");

template <typename T>
class AcquiredDatablock : public Datablock<T> {
 public:
    // TODO - add overloaded versions supporting hint, count, etc.
    explicit AcquiredDatablock(u64 count = 1)
            : AcquiredDatablock(nullptr, count) {}

    // This version gets called from the task setup code
    explicit AcquiredDatablock(ocrEdtDep_t dep)
            : Datablock<T>(dep.guid), data_(static_cast<T *>(dep.ptr)) {}

    T &data() const { return *data_; }

    T *data_ptr() const { return data_; }

    void Release() const { OK(ocrDbRelease(this->guid())); }

 private:
    AcquiredDatablock(T *tmp, u64 count)
            : Datablock<T>(&tmp, count), data_(tmp) {}
    T *const data_;
};

namespace internal {

template <typename T>
struct FnInfo;

template <typename R, typename... Args>
struct FnInfo<R(Args...)> {
    static constexpr size_t arg_count = sizeof...(Args);
    typedef std::function<R(Args...)> fn_type;
    typedef std::tuple<Args...> arg_types;
    typedef R result_type;
    template <size_t I>
    struct arg {
        typedef typename std::tuple_element<I, arg_types>::type type;
    };
};

template <typename T>
struct DependenceArgFor;

template <typename T>
struct DependenceArgFor<Datablock<T>> {
    typedef AcquiredDatablock<T> type;
};
template <typename T>
struct DependenceArgFor<AcquiredDatablock<T>> {
    typedef AcquiredDatablock<T> type;
};
template <>
struct DependenceArgFor<NullHandle> {
    typedef AcquiredDatablock<void> type;
};

template <typename T, typename... U>
struct TaskArgsMatchDeps;

// XXX - this could be simplified with a C++17 fold expression
template <typename R, typename A, typename... Args, typename D,
          typename... Deps>
struct TaskArgsMatchDeps<R(A, Args...), D, Deps...> {
    static constexpr bool value =
            std::is_base_of<typename DependenceArgFor<D>::type, A>::value &&
            TaskArgsMatchDeps<R(Args...), Deps...>::value;
};

template <typename R>
struct TaskArgsMatchDeps<R()> {
    static constexpr bool value = true;
};

template <typename T, T *t, typename U>
class TaskImplementation;

// TODO - add static check to make sure Args have AcquiredDatablock types
template <typename F, F *user_fn, typename R, typename... Args>
class TaskImplementation<F, user_fn, R(Args...)> {
 public:
    static_assert(std::is_same<F, R(Args...)>::value,
                  "Task function must have a consistent type.");

    static ocrGuid_t internal_fn(u32 paramc, u64 *paramv, u32 depc,
                                 ocrEdtDep_t depv[]) {
        ASSERT(paramc == 0);
        ASSERT(depc == sizeof...(Args));
        return launch(depv, std::index_sequence_for<Args...>{});
    }

 private:
    // XXX - using index_sequence requires C++14,
    // but we could roll our own if we really want C++11 compatibility.
    template <size_t... Indices>
    static ocrGuid_t launch(ocrEdtDep_t deps[],
                            std::index_sequence<Indices...>) {
        // This calls the AcquiredDatablock(ocrEdtDep_t) constructor
        return user_fn((Args{deps[Indices]})...).guid();
    }
};

}  // namespace internal

template <typename F>
class TaskTemplateBase : public ObjectHandle {
 public:
    ~TaskTemplateBase() = default;

    void Destroy() const { OK(ocrEdtTemplateDestroy(guid())); }

 protected:
    explicit TaskTemplateBase(ocrGuid_t guid) : ObjectHandle(guid) {}

    typedef typename FnInfo<F>::result_type R;
    static_assert(std::is_base_of<ObjectHandle, R>::value,
                  "User's task function must return an OCR object type.");
};

// TODO - add static check to make sure Deps extend ObjectHandle
template <typename F, typename... Deps>
class Task : public ObjectHandle {
 public:
    // TODO - add support for hints, output events, etc
    Task(TaskTemplateBase<F> task_template, Deps... deps)
            : ObjectHandle(init(task_template, deps...)) {}

    void Destroy() const { OK(ocrEdtDestroy(guid())); }

    // TODO - AddDependence

 private:
    static constexpr u64 depc = FnInfo<F>::arg_count;

    static ocrGuid_t init(TaskTemplateBase<F> task_template, Deps... deps) {
        ocrGuid_t guid;
        ocrGuid_t depv[depc + 1] = {(deps.guid())..., NULL_GUID};
        ocrEdtCreate(&guid, task_template.guid(), EDT_PARAM_DEF, nullptr,
                     EDT_PARAM_DEF, depc ? depv : nullptr, EDT_PROP_NONE,
                     nullptr, nullptr);
        return guid;
    }

    static_assert(depc == sizeof...(Deps),
                  "Dependence argument count must match task argument count.");

    static_assert(TaskArgsMatchDeps<F, Deps...>::value,
                  "Dependence argument types must match task argument types.");
};

template <typename F>
class TaskTemplate : public TaskTemplateBase<F> {
 public:
    template <F *user_fn>
    static TaskTemplate<F> Create() {
        ocrGuid_t guid;
        ocrEdt_t internal_fn = TaskImplementation<F, user_fn, F>::internal_fn;
        constexpr u64 depc = FnInfo<F>::arg_count;
        ocrEdtTemplateCreate(&guid, internal_fn, 0, depc);
        return TaskTemplate<F>(guid);
    }

    template <typename... Deps>
    Task<F, Deps...> CreateTask(Deps... deps) {
        return Task<F, Deps...>(*this, deps...);
    }

    ~TaskTemplate() = default;

 private:
    explicit TaskTemplate(ocrGuid_t guid) : TaskTemplateBase<F>(guid) {}
};

namespace internal {

typedef NullHandle DummyTaskFnType(AcquiredDatablock<int>,
                                   AcquiredDatablock<double>);

typedef Task<DummyTaskFnType, Datablock<int>, Datablock<double>> DummyTaskType;

}  // namespace internal

static_assert(IsLegalHandle<TaskTemplateBase<DummyTaskFnType>>::value,
              "TaskTemplateBase must be castable to/from ocrGuid_t.");

static_assert(IsLegalHandle<DummyTaskType>::value,
              "Task must be castable to/from ocrGuid_t.");

static_assert(IsLegalHandle<TaskTemplate<DummyTaskFnType>>::value,
              "TaskTemplate must be castable to/from ocrGuid_t.");

}  // namespace ocxxr

#define OCXXR_TEMPLATE_FOR(fn_ptr) \
    ocxxr::TaskTemplate<decltype(fn_ptr)>::Create<fn_ptr>();

#endif  // OCXXR_H_

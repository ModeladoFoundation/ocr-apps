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

template <typename T, typename U = void>
using EnableIfVoid =
        typename std::enable_if<std::is_same<void, T>::value, U>::type;

template <typename T, typename U = void>
using EnableIfNotVoid =
        typename std::enable_if<!std::is_same<void, T>::value, U>::type;

template <typename T, typename U, typename V = void>
using EnableIfBaseOf =
        typename std::enable_if<std::is_base_of<T, U>::value, V>::type;

// Check error status of C API call
inline void OK(u8 status) { ASSERT(status == 0); }
}

// NOTE: I add more declarations to the internal namespace later,
// but this directive brings them all into the current scope.
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

template <typename T>
class DataHandle : public ObjectHandle {
 public:
    ~DataHandle() = default;

 protected:
    explicit DataHandle(ocrGuid_t guid) : ObjectHandle(guid) {}
};

static_assert(IsLegalHandle<DataHandle<int>>::value,
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

//! Datablocks
template <typename T>
class Datablock : public DataHandle<T> {
 public:
    // TODO - add overloaded versions supporting hint, etc.
    explicit Datablock(u64 count = 1)
            : DataHandle<T>(init(sizeof(T) * count, false, nullptr)) {}

    explicit Datablock(T **data_ptr, u64 count = 1)
            : DataHandle<T>(init(data_ptr, sizeof(T) * count, true, nullptr)) {}

    void Destroy() const { OK(ocrDbDestroy(this->guid())); }

 protected:
    explicit Datablock(ocrGuid_t guid) : DataHandle<T>(guid) {}

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

    // create empty datablock
    AcquiredDatablock(std::nullptr_t)
            : Datablock<T>(NULL_GUID), data_(nullptr) {}

    template <typename U = T>
    EnableIfNotVoid<U, U> &data() const {
        // The template type U is only here to get enable_if to work.
        static_assert(std::is_same<T, U>::value, "Template types must match.");
        return *data_;
    }

    T *data_ptr() const { return data_; }

    void Release() const { OK(ocrDbRelease(this->guid())); }

 private:
    AcquiredDatablock(T *tmp, u64 count)
            : Datablock<T>(&tmp, count), data_(tmp) {}
    T *const data_;
};

//! Events
template <typename T>
class Event : public DataHandle<T> {
 public:
    explicit Event(ocrEventTypes_t type) : DataHandle<T>(init(type)) {}

    void Destroy() const { OK(ocrEventDestroy(this->guid())); }

    void Satisfy() const { OK(ocrEventSatisfy(this->guid(), NULL_GUID)); }

    void Satisfy(DataHandle<T> data) const {
        OK(ocrEventSatisfy(this->guid(), data.guid()));
    }

 private:
    static ocrGuid_t init(ocrEventTypes_t type) {
        ocrGuid_t guid;
        static constexpr bool kIsVoid = std::is_same<void, T>::value;
        constexpr u16 flags = kIsVoid ? EVT_PROP_NONE : EVT_PROP_TAKES_ARG;
        OK(ocrEventCreate(&guid, type, flags));
        return guid;
    }
};

static_assert(IsLegalHandle<Event<int>>::value,
              "Event must be castable to/from ocrGuid_t.");

template <typename T>
class OnceEvent : public Event<T> {
 public:
    explicit OnceEvent() : Event<T>(OCR_EVENT_ONCE_T) {}
};

static_assert(IsLegalHandle<OnceEvent<int>>::value,
              "OnceEvent must be castable to/from ocrGuid_t.");

template <typename T>
class IdempotentEvent : public Event<T> {
 public:
    explicit IdempotentEvent() : Event<T>(OCR_EVENT_IDEM_T) {}
};

static_assert(IsLegalHandle<IdempotentEvent<int>>::value,
              "IdempotentEvent must be castable to/from ocrGuid_t.");

template <typename T>
class StickyEvent : public Event<T> {
 public:
    explicit StickyEvent() : Event<T>(OCR_EVENT_STICKY_T) {}
};

static_assert(IsLegalHandle<StickyEvent<int>>::value,
              "StickyEvent must be castable to/from ocrGuid_t.");

template <typename T>
class LatchEvent : public Event<T> {
 public:
    explicit LatchEvent() : Event<T>(OCR_EVENT_LATCH_T) {}
    // TODO - SatisfySlot
    // TODO - Up
    // TODO - Down
};

static_assert(IsLegalHandle<LatchEvent<int>>::value,
              "LatchEvent must be castable to/from ocrGuid_t.");

class NullHandle : public ObjectHandle {
 public:
    NullHandle() : ObjectHandle(NULL_GUID) {}

    // auto-convert NullHandle to any ObjectHandle type
    template <typename T>
    const operator T() const {
        static_assert(std::is_base_of<ObjectHandle, T>::value,
                      "Only use NullHandle for ObjectHandle types.");
        static_assert(IsLegalHandle<T>::value,
                      "Only use NullHandle for simple handle types.");
        return *reinterpret_cast<const T *>(this);
    }

    template <typename T>
    const operator AcquiredDatablock<T>() const {
        return AcquiredDatablock<T>(nullptr);
    }
};

static_assert(IsLegalHandle<NullHandle>::value,
              "NullHandle must be castable to/from ocrGuid_t.");

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

// NOTE: These definitions are separate from those of the associated data types
// in order to break the cyclic dependence between the two types.
template <typename T>
struct DependenceArgFor;

template <template <typename> class T, typename U>
struct DependenceArgFor<T<U>> {
    typedef EnableIfBaseOf<DataHandle<U>, T<U>, AcquiredDatablock<U>> type;
};
template <>
struct DependenceArgFor<NullHandle> {
    typedef AcquiredDatablock<void> type;
};

template <size_t ArgCount, size_t ParamCount>
struct TaskArgCountCheck {
    static_assert(ArgCount == ParamCount,
                  "Dependence argument count must match task parameter count.");
    static constexpr bool value = true;
};

template <typename A, typename D, size_t Position>
struct TaskArgTypeMatchesParamType {
    typedef typename DependenceArgFor<D>::type DT;
    static_assert(std::is_base_of<DT, A>::value,
                  "Dependence argument type must match task parameter type.");
    static constexpr bool value = true;
};

template <size_t Posiiton, typename T, typename... U>
struct TaskArgsMatchDeps;

template <size_t Position, typename R, typename A, typename... Args, typename D,
          typename... Deps>
struct TaskArgsMatchDeps<Position, R(A, Args...), D, Deps...> {
    static constexpr bool value =
            TaskArgTypeMatchesParamType<A, D, Position>::value &&
            TaskArgsMatchDeps<Position + 1, R(Args...), Deps...>::value;
};

template <size_t Position, typename R>
struct TaskArgsMatchDeps<Position, R()> {
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

    static ocrGuid_t internal_fn(u32 paramc, u64 /*paramv*/[], u32 depc,
                                 ocrEdtDep_t depv[]) {
        ASSERT(paramc == 0);
        ASSERT(depc == sizeof...(Args));
        return launch(depv, std::index_sequence_for<Args...>{});
    }

 private:
    // XXX - using index_sequence requires C++14,
    // but we could roll our own if we really want C++11 compatibility.
    template <typename U = R, size_t... Indices>
    static EnableIfNotVoid<U, ocrGuid_t> launch(
            ocrEdtDep_t deps[], std::index_sequence<Indices...>) {
        // This calls the AcquiredDatablock(ocrEdtDep_t) constructor
        return user_fn((Args{deps[Indices]})...).guid();
    }

    template <typename U = R, size_t... Indices>
    static EnableIfVoid<U, ocrGuid_t> launch(ocrEdtDep_t deps[],
                                             std::index_sequence<Indices...>) {
        // This calls the AcquiredDatablock(ocrEdtDep_t) constructor
        user_fn((Args{deps[Indices]})...);
        return NULL_GUID;
    }
};

}  // namespace internal

template <typename F>
class TaskTemplateBase : public ObjectHandle {
 public:
    ~TaskTemplateBase() = default;

    void Destroy() const { OK(ocrEdtTemplateDestroy(this->guid())); }

 protected:
    explicit TaskTemplateBase(ocrGuid_t guid) : ObjectHandle(guid) {}

    typedef typename FnInfo<F>::result_type R;
    static_assert(std::is_same<void, R>::value ||
                          std::is_base_of<ObjectHandle, R>::value,
                  "User's task function must return an OCR object type.");
};

// TODO - add static check to make sure Deps extend ObjectHandle
template <typename F, typename... Deps>
class Task : public ObjectHandle {
 public:
    // TODO - add support for hints, output events, etc
    Task(TaskTemplateBase<F> task_template, Deps... deps)
            : ObjectHandle(init(task_template, deps...)) {}

    void Destroy() const { OK(ocrEdtDestroy(this->guid())); }

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

    // Note: this assertion won't ever fail because another insertion
    // in the internal checks will always fail first (or none at all).
    static_assert(TaskArgCountCheck<sizeof...(Deps), depc>::value &&
                          TaskArgsMatchDeps<0, F, Deps...>::value,
                  "Check for args/paramters mismatch.");
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

// prototype for user's main function
void Main(AcquiredDatablock<MainTaskArgs> args);

}  // namespace ocxxr

#define OCXXR_TEMPLATE_FOR(fn_ptr) \
    ocxxr::TaskTemplate<decltype(fn_ptr)>::Create<fn_ptr>();

// XXX - Need to move this to source file!
extern "C" ocrGuid_t mainEdt(u32 paramc, u64 /*paramv*/[], u32 depc,
                             ocrEdtDep_t depv[]) {
    ASSERT(paramc == 0 && depc == 1);
    ocxxr::Main(ocxxr::AcquiredDatablock<ocxxr::MainTaskArgs>(depv[0]));
    return NULL_GUID;
}

#endif  // OCXXR_H_

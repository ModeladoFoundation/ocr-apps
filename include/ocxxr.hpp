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
}  // namespace internal

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

static_assert(internal::IsLegalHandle<ObjectHandle>::value,
              "ObjectHandle must be castable to/from ocrGuid_t.");

template <typename T>
class DataHandle : public ObjectHandle {
 public:
    ~DataHandle() = default;

 protected:
    explicit DataHandle(ocrGuid_t guid) : ObjectHandle(guid) {}
};

static_assert(internal::IsLegalHandle<DataHandle<int>>::value,
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
            : DataHandle<T>(Init(sizeof(T) * count, false, nullptr)) {}

    explicit Datablock(T **data_ptr, u64 count = 1)
            : DataHandle<T>(Init(data_ptr, sizeof(T) * count, true, nullptr)) {}

    void Destroy() const { internal::OK(ocrDbDestroy(this->guid())); }

 protected:
    explicit Datablock(ocrGuid_t guid) : DataHandle<T>(guid) {}

 private:
    static ocrGuid_t Init(u64 bytes, const Hint *hint) {
        T **data_ptr;
        return Init(&data_ptr, bytes, false, hint);
    }

    static ocrGuid_t Init(T **data_ptr, u64 bytes, bool acquire,
                          const Hint *hint) {
        ocrGuid_t guid;
        const u16 flags = acquire ? DB_PROP_NONE : DB_PROP_NO_ACQUIRE;
        // TODO - open bug for adding const qualifiers in OCR C API.
        // E.g., "const ocrHint_t *hint" in ocrDbCreate.
        ocrHint_t *raw_hint = const_cast<ocrHint_t *>(hint->internal());
        void **raw_data_ptr = reinterpret_cast<void **>(data_ptr);
        internal::OK(ocrDbCreate(&guid, raw_data_ptr, bytes, flags, raw_hint,
                                 NO_ALLOC));
        return guid;
    }
};

static_assert(internal::IsLegalHandle<Datablock<int>>::value,
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
    internal::EnableIfNotVoid<U, U> &data() const {
        // The template type U is only here to get enable_if to work.
        static_assert(std::is_same<T, U>::value, "Template types must match.");
        return *data_;
    }

    T *data_ptr() const { return data_; }

    void Release() const { internal::OK(ocrDbRelease(this->guid())); }

 private:
    AcquiredDatablock(T *tmp, u64 count)
            : Datablock<T>(&tmp, count), data_(tmp) {}
    T *const data_;
};

//! Events
template <typename T>
class Event : public DataHandle<T> {
 public:
    explicit Event(ocrEventTypes_t type) : DataHandle<T>(Init(type)) {}

    void Destroy() const { internal::OK(ocrEventDestroy(this->guid())); }

    void Satisfy() const {
        internal::OK(ocrEventSatisfy(this->guid(), NULL_GUID));
    }

    void Satisfy(DataHandle<T> data) const {
        internal::OK(ocrEventSatisfy(this->guid(), data.guid()));
    }

    void AddDependence(DataHandle<T> src) const {
        constexpr u32 slot = 0;
        constexpr ocrDbAccessMode_t mode = DB_DEFAULT_MODE;
        internal::OK(ocrAddDependence(src.guid(), this->guid(), slot, mode));
    }

 private:
    static ocrGuid_t Init(ocrEventTypes_t type) {
        ocrGuid_t guid;
        static constexpr bool kIsVoid = std::is_same<void, T>::value;
        constexpr u16 flags = kIsVoid ? EVT_PROP_NONE : EVT_PROP_TAKES_ARG;
        internal::OK(ocrEventCreate(&guid, type, flags));
        return guid;
    }
};

static_assert(internal::IsLegalHandle<Event<int>>::value,
              "Event must be castable to/from ocrGuid_t.");

template <typename T>
class OnceEvent : public Event<T> {
 public:
    explicit OnceEvent() : Event<T>(OCR_EVENT_ONCE_T) {}
};

static_assert(internal::IsLegalHandle<OnceEvent<int>>::value,
              "OnceEvent must be castable to/from ocrGuid_t.");

template <typename T>
class IdempotentEvent : public Event<T> {
 public:
    explicit IdempotentEvent() : Event<T>(OCR_EVENT_IDEM_T) {}
};

static_assert(internal::IsLegalHandle<IdempotentEvent<int>>::value,
              "IdempotentEvent must be castable to/from ocrGuid_t.");

template <typename T>
class StickyEvent : public Event<T> {
 public:
    explicit StickyEvent() : Event<T>(OCR_EVENT_STICKY_T) {}
};

static_assert(internal::IsLegalHandle<StickyEvent<int>>::value,
              "StickyEvent must be castable to/from ocrGuid_t.");

template <typename T>
class LatchEvent : public Event<T> {
 public:
    explicit LatchEvent() : Event<T>(OCR_EVENT_LATCH_T) {}
    // TODO - SatisfySlot
    // TODO - Up
    // TODO - Down
};

static_assert(internal::IsLegalHandle<LatchEvent<int>>::value,
              "LatchEvent must be castable to/from ocrGuid_t.");

class NullHandle : public ObjectHandle {
 public:
    NullHandle() : ObjectHandle(NULL_GUID) {}

    // auto-convert NullHandle to any ObjectHandle type
    template <typename T>
    const operator T() const {
        static_assert(std::is_base_of<ObjectHandle, T>::value,
                      "Only use NullHandle for ObjectHandle types.");
        static_assert(internal::IsLegalHandle<T>::value,
                      "Only use NullHandle for simple handle types.");
        return *reinterpret_cast<const T *>(this);
    }

    template <typename T>
    const operator AcquiredDatablock<T>() const {
        return AcquiredDatablock<T>(nullptr);
    }
};

static_assert(internal::IsLegalHandle<NullHandle>::value,
              "NullHandle must be castable to/from ocrGuid_t.");

namespace internal {

template <typename T>
struct FnInfo;

template <typename R, typename... Args>
struct FnInfo<R(Args...)> {
    static constexpr size_t kArgCount = sizeof...(Args);
    typedef std::function<R(Args...)> FnType;
    typedef std::tuple<Args...> ArgTypes;
    typedef R ResultType;
    template <size_t I>
    struct Arg {
        typedef typename std::tuple_element<I, ArgTypes>::type Type;
    };
};

// NOTE: These definitions are separate from those of the associated data types
// in order to break the cyclic dependence between the two types.
template <typename T>
struct TypeMapping;

template <template <typename> class T, typename U>
struct TypeMapping<T<U>> {
    typedef EnableIfBaseOf<DataHandle<U>, T<U>, AcquiredDatablock<U>> DepType;
    typedef EnableIfBaseOf<DataHandle<U>, T<U>, Event<U>> OutEventType;
};
template <>
struct TypeMapping<NullHandle> {
    typedef AcquiredDatablock<void> DepType;
    typedef Event<void> OutEventType;
};
template <>
struct TypeMapping<void> {
    typedef Event<void> OutEventType;
};

template <size_t ArgCount, size_t ParamCount>
struct TaskArgCountCheck {
    static_assert(ArgCount == ParamCount,
                  "Dependence argument count must match task parameter count.");
    static constexpr bool value = true;
};

template <typename A, typename D, size_t Position>
struct TaskArgTypeMatchesParamType {
    typedef typename TypeMapping<D>::DepType DT;
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

    static ocrGuid_t InternalFn(u32 paramc, u64 /*paramv*/[], u32 depc,
                                ocrEdtDep_t depv[]) {
        ASSERT(paramc == 0);
        ASSERT(depc == sizeof...(Args));
        return Launch(depv, std::index_sequence_for<Args...>{});
    }

 private:
    // XXX - using index_sequence requires C++14,
    // but we could roll our own if we really want C++11 compatibility.
    template <typename U = R, size_t... Indices>
    static EnableIfNotVoid<U, ocrGuid_t> Launch(
            ocrEdtDep_t deps[], std::index_sequence<Indices...>) {
        // This calls the AcquiredDatablock(ocrEdtDep_t) constructor
        return user_fn((Args{deps[Indices]})...).guid();
    }

    template <typename U = R, size_t... Indices>
    static EnableIfVoid<U, ocrGuid_t> Launch(ocrEdtDep_t deps[],
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

    void Destroy() const { internal::OK(ocrEdtTemplateDestroy(this->guid())); }

 protected:
    explicit TaskTemplateBase(ocrGuid_t guid) : ObjectHandle(guid) {}

    typedef typename internal::FnInfo<F>::ResultType R;
    static_assert(std::is_same<void, R>::value ||
                          std::is_base_of<ObjectHandle, R>::value,
                  "User's task function must return an OCR object type.");
};

// TODO - add static check to make sure Deps extend ObjectHandle
template <typename F, typename... Deps>
class Task : public ObjectHandle {
 public:
    typedef typename internal::TypeMapping<
            typename internal::FnInfo<F>::ResultType>::OutEventType
            OutEventType;

    // TODO - add support for hints, output events, etc
    Task(TaskTemplateBase<F> task_template, Deps... deps)
            : ObjectHandle(Init(NullHandle(), task_template, deps...)) {}

    Task(OutEventType out_event, TaskTemplateBase<F> task_template,
         Deps... deps)
            : ObjectHandle(Init(out_event, task_template, deps...)) {}

    void Destroy() const { internal::OK(ocrEdtDestroy(this->guid())); }

    // TODO - AddDependence

 private:
    static constexpr u32 depc = internal::FnInfo<F>::kArgCount;

    static ocrGuid_t Init(OutEventType user_out_event,
                          TaskTemplateBase<F> task_template, Deps... deps) {
        bool is_future = !user_out_event.is_null();
        ocrGuid_t guid;
        ocrGuid_t internal_out_event;
        ocrGuid_t *out_ptr = is_future ? &internal_out_event : nullptr;
        ocrGuid_t depv[depc + 1] = {(deps.guid())..., NULL_GUID};
        ocrGuid_t last_dep;
        if (is_future) {
            last_dep = depv[depc - 1];
            depv[depc - 1] = UNINITIALIZED_GUID;
        }
        ocrEdtCreate(&guid, task_template.guid(), EDT_PARAM_DEF, nullptr,
                     EDT_PARAM_DEF, depc ? depv : nullptr, EDT_PROP_NONE,
                     nullptr, out_ptr);
        if (is_future) {
            ocrAddDependence(internal_out_event, user_out_event.guid(), 0,
                             DB_DEFAULT_MODE);
            // satisfy last slot
            ocrAddDependence(last_dep, guid, depc - 1, DB_DEFAULT_MODE);
        }
        return guid;
    }

    // Note: this assertion won't ever fail because another insertion
    // in the internal checks will always fail first (or none at all).
    static_assert(internal::TaskArgCountCheck<sizeof...(Deps), depc>::value &&
                          internal::TaskArgsMatchDeps<0, F, Deps...>::value,
                  "Check for args/paramters mismatch.");
};

template <typename F>
class TaskTemplate : public TaskTemplateBase<F> {
 public:
    typedef typename internal::TypeMapping<
            typename internal::FnInfo<F>::ResultType>::OutEventType
            OutEventType;

    template <F *user_fn>
    static TaskTemplate<F> Create() {
        ocrGuid_t guid;
        ocrEdt_t internal_fn =
                internal::TaskImplementation<F, user_fn, F>::InternalFn;
        constexpr u64 depc = internal::FnInfo<F>::kArgCount;
        ocrEdtTemplateCreate(&guid, internal_fn, 0, depc);
        return TaskTemplate<F>(guid);
    }

    template <typename... Deps>
    Task<F, Deps...> CreateTask(Deps... deps) {
        return Task<F, Deps...>(*this, deps...);
    }

    template <typename... Deps>
    Task<F, Deps...> CreateFuture(OutEventType out_event, Deps... deps) {
        return Task<F, Deps...>(out_event, *this, deps...);
    }

    ~TaskTemplate() = default;

 private:
    explicit TaskTemplate(ocrGuid_t guid) : TaskTemplateBase<F>(guid) {}
};

namespace internal {

typedef NullHandle DummyTaskFnType(AcquiredDatablock<int>,
                                   AcquiredDatablock<double>);

typedef Task<DummyTaskFnType, Datablock<int>, Datablock<double>> DummyTaskType;

typedef TaskTemplateBase<DummyTaskFnType> DummyTemplateBaseType;

typedef TaskTemplate<DummyTaskFnType> DummyTemplateType;

}  // namespace internal

static_assert(internal::IsLegalHandle<internal::DummyTemplateBaseType>::value,
              "TaskTemplateBase must be castable to/from ocrGuid_t.");

static_assert(internal::IsLegalHandle<internal::DummyTaskType>::value,
              "Task must be castable to/from ocrGuid_t.");

static_assert(internal::IsLegalHandle<internal::DummyTemplateType>::value,
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

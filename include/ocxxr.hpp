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

template <typename T, typename U = int>
using EnableIfVoid =
        typename std::enable_if<std::is_same<void, T>::value, U>::type;

template <typename T, typename U = int>
using EnableIfNotVoid =
        typename std::enable_if<!std::is_same<void, T>::value, U>::type;

template <typename T, typename U, typename V = int>
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
              "DataHandle must be castable to/from ocrGuid_t.");

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
class DatablockHandle : public DataHandle<T> {
 public:
    // TODO - add overloaded versions supporting hint, etc.
    explicit DatablockHandle(u64 count)
            : DataHandle<T>(Init(sizeof(T) * count, false, nullptr)) {}

    explicit DatablockHandle(T **data_ptr, u64 count)
            : DataHandle<T>(Init(data_ptr, sizeof(T) * count, true, nullptr)) {}

    void Destroy() const { internal::OK(ocrDbDestroy(this->guid())); }

    explicit DatablockHandle(ocrGuid_t guid = NULL_GUID)
            : DataHandle<T>(guid) {}

    // create a datablock, but don't acquire it.
    static DatablockHandle<T> Create(u64 count = 1) {
        return DatablockHandle<T>(count);
    }

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

static_assert(internal::IsLegalHandle<DatablockHandle<int>>::value,
              "DatablockHandle must be castable to/from ocrGuid_t.");

template <typename T>
class Datablock {
 public:
    // TODO - add overloaded versions supporting hint, count, etc.
    explicit Datablock(u64 count) : Datablock(nullptr, count) {}

    // This version gets called from the task setup code
    explicit Datablock(ocrEdtDep_t dep)
            : handle_(dep.guid), data_(static_cast<T *>(dep.ptr)) {}

    // create empty datablock
    explicit Datablock(std::nullptr_t np = nullptr)
            : handle_(NULL_GUID), data_(np) {}

    static Datablock<T> Create(u64 count = 1) { return Datablock<T>(count); }

    template <typename U = T, internal::EnableIfNotVoid<U> = 0>
    U &data() const {
        // The template type U is only here to get enable_if to work.
        static_assert(std::is_same<T, U>::value, "Template types must match.");
        return *data_;
    }

    T *data_ptr() const { return data_; }

    DatablockHandle<T> handle() const { return handle_; }

    void Release() const { internal::OK(ocrDbRelease(this->guid())); }

    operator DatablockHandle<T>() const { return handle_; }

 private:
    Datablock(T *tmp, u64 count) : handle_(&tmp, count), data_(tmp) {}
    const DatablockHandle<T> handle_;
    T *const data_;
};

//! Events
template <typename T>
class Event : public DataHandle<T> {
 public:
    explicit Event(ocrEventTypes_t type) : DataHandle<T>(Init(type)) {}

    explicit Event(ocrGuid_t guid = NULL_GUID) : DataHandle<T>(guid) {}

    void Destroy() const { internal::OK(ocrEventDestroy(this->guid())); }

    void Satisfy() const {
        internal::OK(ocrEventSatisfy(this->guid(), NULL_GUID));
    }

    void Satisfy(DatablockHandle<T> data) const {
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
    const operator Datablock<T>() const {
        return Datablock<T>(nullptr);
    }
};

static_assert(internal::IsLegalHandle<NullHandle>::value,
              "NullHandle must be castable to/from ocrGuid_t.");

// Used only for placing "holes" during task dependence creation
template <typename T>
class UnknownDependence : public DataHandle<T> {
 public:
    UnknownDependence() : ObjectHandle(UNINITIALIZED_GUID) {}
};

static_assert(internal::IsLegalHandle<UnknownDependence<int>>::value,
              "UnknownDependence must be castable to/from ocrGuid_t.");

namespace internal {

template <typename T>
struct FnInfo;

template <typename R, typename... As>
struct FnInfo<R(As...)> {
    static constexpr size_t kArgCount = sizeof...(As);
    typedef std::function<R(As...)> Fn;
    typedef std::tuple<As...> Args;
    typedef R Result;
    template <size_t I>
    struct Arg {
        typedef typename std::tuple_element<I, Args>::type Type;
    };
};

template <typename T>
struct Unpack;

template <template <typename> class T, typename U>
struct Unpack<T<U>> {
    typedef U Parameter;
    static_assert(std::is_convertible<T<U>, DataHandle<U>>::value,
                  "Expected an OCR data container type.");
};
template <>
struct Unpack<NullHandle> {
    typedef void Parameter;
};
template <>
struct Unpack<void> {
    typedef void Parameter;
};

template <typename F>
using ReturnTypeParameter =
        typename Unpack<typename FnInfo<F>::Result>::Parameter;

template <size_t ArgCount, size_t ParamCount>
struct TaskArgCountCheck {
    static_assert(
            ArgCount <= ParamCount,
            "Dependence argument count must not exceed task parameter count.");
    static constexpr bool value = true;
};

template <typename A, typename D, size_t Position>
struct TaskArgTypeMatchesParamType {
    typedef Datablock<typename Unpack<D>::Parameter> DB;
    static_assert(std::is_base_of<A, DB>::value,
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

template <size_t Position, typename F>
struct TaskArgsMatchDeps<Position, F> {
    static constexpr bool value = true;
};

template <typename T, T *t, typename U>
class TaskImplementation;

// TODO - add static check to make sure Args have Datablock types
template <typename F, F *user_fn, typename R, typename... Args>
class TaskImplementation<F, user_fn, R(Args...)> {
 public:
    static_assert(std::is_same<F, R(Args...)>::value,
                  "Task function must have a consistent type.");

    static ocrGuid_t InternalFn(u32 paramc, u64 /*paramv*/[], u32 depc,
                                ocrEdtDep_t depv[]) {
        ASSERT(paramc == 0);
        ASSERT(depc - 1 == sizeof...(Args));
        return Launch(depv, std::index_sequence_for<Args...>{});
    }

 private:
    // XXX - using index_sequence requires C++14,
    // but we could roll our own if we really want C++11 compatibility.
    template <typename U = R, EnableIfNotVoid<U> = 0, size_t... I>
    static ocrGuid_t Launch(ocrEdtDep_t deps[], std::index_sequence<I...>) {
        // This calls the Datablock(ocrEdtDep_t) constructor
        return user_fn((Args{deps[I]})...).guid();
    }

    template <typename U = R, EnableIfVoid<U> = 0, size_t... I>
    static ocrGuid_t Launch(ocrEdtDep_t deps[], std::index_sequence<I...>) {
        // This calls the Datablock<?>(ocrEdtDep_t) constructor
        user_fn((Args{deps[I]})...);
        return NULL_GUID;
    }
};

}  // namespace internal

template <typename F>
class TaskTemplate;

// TODO - Add static check to make sure Deps are convertible to DataHandle
template <typename F>
class Task : public ObjectHandle {
 public:
    static constexpr u32 kDepc = internal::FnInfo<F>::kArgCount;

    // Parameter type of the task's return-type container object
    typedef internal::ReturnTypeParameter<F> R;

    void Destroy() const { internal::OK(ocrEdtDestroy(this->guid())); }

    void AddDependence(DataHandle<R> src, u32 slot,
                       ocrDbAccessMode_t mode = DB_DEFAULT_MODE) const {
        internal::OK(ocrAddDependence(src.guid(), this->guid(), slot, mode));
    }

 protected:
    friend class TaskTemplate<F>;

    template <typename T>
    using DataHandleOf = DataHandle<typename internal::Unpack<T>::Parameter>;

    // TODO - add support for hints, output events, etc
    Task(Event<R> *out_event, ocrGuid_t task_template, ocrGuid_t depv[])
            : ObjectHandle(Init(out_event, task_template, depv)) {}

    template <typename... Deps>
    static Task<F> CreateTask(Event<R> *out_event, ocrGuid_t task_template,
                              Deps... deps) {
        // Note: this assertion will never fail because another assertion
        // in the internal checks will always fail first (or none at all).
        static_assert(
                internal::TaskArgCountCheck<sizeof...(Deps), kDepc>::value &&
                        internal::TaskArgsMatchDeps<0, F, Deps...>::value,
                "Check for args/paramters mismatch.");
        bool is_future = out_event != nullptr;
        ocrGuid_t dummy_dep = is_future ? UNINITIALIZED_GUID : NULL_GUID;
        ocrGuid_t depv[1 + kDepc] = {
                (static_cast<DataHandleOf<Deps>>(deps).guid())..., dummy_dep};
        return Task<F>(out_event, task_template, depv);
    }

 private:
    static ocrGuid_t Init(Event<R> *out_event, ocrGuid_t task_template,
                          ocrGuid_t depv[]) {
        ocrGuid_t guid;
        ocrGuid_t *out_guid = reinterpret_cast<ocrGuid_t *>(out_event);
        ocrEdtCreate(&guid, task_template, EDT_PARAM_DEF, nullptr,
                     EDT_PARAM_DEF, depv, EDT_PROP_NONE, nullptr, out_guid);
        return guid;
    }
};

template <typename F>
class DelayedFuture {
 public:
    typedef internal::ReturnTypeParameter<F> R;

    DelayedFuture(Task<F> task, Event<R> event) : task_(task), event_(event) {}

    Task<F> task() const { return task_; }

    Event<R> event() const { return event_; }

    void Release() const {
        constexpr u32 slot = Task<F>::kDepc;  // dummy slot is last
        ocrGuid_t dest = task_.guid();
        constexpr ocrDbAccessMode_t mode = DB_DEFAULT_MODE;
        internal::OK(ocrAddDependence(NULL_GUID, dest, slot, mode));
    }

 private:
    const Task<F> task_;
    const Event<R> event_;
};

template <typename F>
class TaskTemplate : public ObjectHandle {
 public:
    typedef internal::ReturnTypeParameter<F> R;

    // TODO - add support for paramv
    template <F *user_fn>
    static TaskTemplate<F> Create() {
        ocrGuid_t guid;
        ocrEdt_t internal_fn =
                internal::TaskImplementation<F, user_fn, F>::InternalFn;
        constexpr u64 depc = internal::FnInfo<F>::kArgCount;
        ocrEdtTemplateCreate(&guid, internal_fn, 0, 1 + depc);
        return TaskTemplate<F>(guid);
    }

    void Destroy() const { internal::OK(ocrEdtTemplateDestroy(this->guid())); }

    template <typename... Deps>
    Task<F> CreateTask(Deps... deps) {
        return Task<F>::CreateTask(nullptr, this->guid(), deps...);
    }

    template <typename... Deps>
    DelayedFuture<F> CreateFuture(Deps... deps) {
        Event<R> out_event;
        auto task = Task<F>::CreateTask(&out_event, this->guid(), deps...);
        return DelayedFuture<F>(task, out_event);
    }

    ~TaskTemplate() = default;

 private:
    explicit TaskTemplate(ocrGuid_t guid) : ObjectHandle(guid) {}

    typedef typename internal::FnInfo<F>::Result Res;
    static_assert(std::is_same<void, Res>::value ||
                          std::is_base_of<ObjectHandle, Res>::value,
                  "User's task function must return an OCR object type.");
};

namespace internal {

typedef NullHandle DummyTaskFnType(Datablock<int>, Datablock<double>);

typedef Task<DummyTaskFnType> DummyTaskType;

typedef TaskTemplate<DummyTaskFnType> DummyTemplateType;

}  // namespace internal

static_assert(internal::IsLegalHandle<internal::DummyTaskType>::value,
              "Task must be castable to/from ocrGuid_t.");

static_assert(internal::IsLegalHandle<internal::DummyTemplateType>::value,
              "TaskTemplate must be castable to/from ocrGuid_t.");

// prototype for user's main function
void Main(Datablock<MainTaskArgs> args);

}  // namespace ocxxr

#define OCXXR_TEMPLATE_FOR(fn_ptr) \
    ocxxr::TaskTemplate<decltype(fn_ptr)>::Create<fn_ptr>();

// XXX - Need to move this to source file!
extern "C" ocrGuid_t mainEdt(u32 paramc, u64 /*paramv*/[], u32 depc,
                             ocrEdtDep_t depv[]) {
    ASSERT(paramc == 0 && depc == 1);
    ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs>(depv[0]));
    return NULL_GUID;
}

#endif  // OCXXR_H_

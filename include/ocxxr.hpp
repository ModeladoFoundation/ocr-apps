#ifndef OCXXR_H_
#define OCXXR_H_

#include <functional>
#include <type_traits>

extern "C" {
#include <ocr.h>
}

#if __cplusplus >= 201402L  // C++14
#define OCXXR_USING_CXX14
#else  // C++11
#include <ocxxr-cxx11-compat.hpp>
#endif

namespace ocxxr {

namespace internal {

#ifdef OCXXR_USING_CXX14
// std::is_trivially_copyable is missing in GCC's C++11
template <typename T>
using IsTriviallyCopyable = std::is_trivially_copyable<T>;

// index_sequence is C++14 only
template <size_t... Indices>
using IndexSeq = std::index_sequence<Indices...>;
template <size_t N>
using MakeIndexSeq = std::make_index_sequence<N>;
template <typename... T>
using IndexSeqFor = std::index_sequence_for<T...>;
#endif

template <typename T>
struct IsLegalHandle {
    // Check if a type can be reinterpreted to/from ocrGuid_t
    static constexpr bool value =
            IsTriviallyCopyable<T>::value && sizeof(T) == sizeof(ocrGuid_t);
};

template <bool condition, typename T = int>
using EnableIf = typename std::enable_if<condition, T>::type;

template <typename T, typename U = int>
using EnableIfVoid = EnableIf<std::is_same<void, T>::value, U>;

template <typename T, typename U = int>
using EnableIfNotVoid = EnableIf<!std::is_same<void, T>::value, U>;

template <typename T, typename U, typename V = int>
using EnableIfBaseOf = EnableIf<std::is_base_of<T, U>::value, V>;

template <typename T, typename U, typename V = int>
using EnableIfSame = EnableIf<std::is_same<T, U>::value, V>;

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

// TODO - add Hint subclasses: TaskHint, DatablockHint
class Hint {
 public:
    Hint(ocrHintType_t hint_type) {
        internal::OK(ocrHintInit(&hint_, hint_type));
    }

    Hint &Set(ocrHintProp_t prop, u64 value) {
        internal::OK(ocrSetHintValue(&hint_, prop, value));
        return *this;
    }

    Hint &Unset(ocrHintProp_t prop) {
        internal::OK(ocrUnsetHintValue(&hint_, prop));
        return *this;
    }

    u64 Get(ocrHintProp_t prop) {
        u64 value;
        internal::OK(ocrGetHintValue(&hint_, prop, &value));
        return value;
    }

    Hint &AttachTo(ObjectHandle object) {
        internal::OK(ocrSetHint(object.guid(), &hint_));
        return *this;
    }

    Hint &ReadFrom(ObjectHandle object) {
        internal::OK(ocrGetHint(object.guid(), &hint_));
        return *this;
    }

    const ocrHint_t *internal() const { return &hint_; }

    ocrHint_t *internal() { return &hint_; }

 private:
    ocrHint_t hint_;
};

class TaskHint : public Hint {
 public:
    TaskHint() : Hint(OCR_HINT_EDT_T) {}
};

class DatablockHint : public Hint {
 public:
    DatablockHint() : Hint(OCR_HINT_DB_T) {}
};

//! Datablocks
template <typename T>
class DatablockHandle : public DataHandle<T> {
 public:
    explicit DatablockHandle(u64 count)
            : DataHandle<T>(Init(sizeof(T) * count, false, nullptr)) {}

    explicit DatablockHandle(u64 count, const Hint &hint)
            : DataHandle<T>(Init(sizeof(T) * count, false, &hint)) {}

    explicit DatablockHandle(T **data_ptr, u64 count, const Hint *hint)
            : DataHandle<T>(Init(data_ptr, sizeof(T) * count, true, hint)) {}

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
    explicit Datablock(u64 count) : Datablock(nullptr, count, nullptr) {}

    explicit Datablock(u64 count, const Hint &hint)
            : Datablock(nullptr, count, &hint) {}

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
    Datablock(T *tmp, u64 count, const Hint *hint)
            : handle_(&tmp, count, hint), data_(tmp) {}

    const DatablockHandle<T> handle_;
    T *const data_;
};

//! Events
template <typename T>
class Event : public DataHandle<T> {
 public:
    static constexpr bool kIsVoid = std::is_same<void, T>::value;
    static constexpr u16 kDefaultFlags =
            kIsVoid ? EVT_PROP_NONE : EVT_PROP_TAKES_ARG;

    explicit Event(ocrEventTypes_t type, u16 flags = kDefaultFlags)
            : DataHandle<T>(Init(type, flags)) {}

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
    static ocrGuid_t Init(ocrEventTypes_t type, u16 flags) {
        ocrGuid_t guid;
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
    operator T() const {
        static_assert(std::is_base_of<ObjectHandle, T>::value,
                      "Only use NullHandle for ObjectHandle types.");
        static_assert(internal::IsLegalHandle<T>::value,
                      "Only use NullHandle for simple handle types.");
        // Note: this is a bit ugly, but since the base class ObjectHandle
        // guarantees that the internal guid is immutable, it should be OK.
        // Plus this is returning by-value, so unless it's captured as an
        // rvalue reference or something, we'll get a copy anyway.
        return *reinterpret_cast<T *>(const_cast<NullHandle *>(this));
    }

    template <typename T>
    operator Datablock<T>() const {
        return Datablock<T>(nullptr);
    }
};

static_assert(internal::IsLegalHandle<NullHandle>::value,
              "NullHandle must be castable to/from ocrGuid_t.");

// Used only for placing "holes" in a task dependence list
template <typename T>
class UnknownDependence : public DataHandle<T> {
 public:
    UnknownDependence() : DataHandle<T>(UNINITIALIZED_GUID) {}
};

static_assert(internal::IsLegalHandle<UnknownDependence<int>>::value,
              "UnknownDependence must be castable to/from ocrGuid_t.");

class DefaultDependence : public ObjectHandle {
 public:
    DefaultDependence() : ObjectHandle(UNINITIALIZED_GUID) {}

    explicit DefaultDependence(size_t) : DefaultDependence() {}

    template <typename T>
    operator DataHandle<T>() const {
        return UnknownDependence<T>();
    }
};

static_assert(internal::IsLegalHandle<DefaultDependence>::value,
              "DefaultDependence must be castable to/from ocrGuid_t.");

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

// XXX - all non-type template arguments should use variable naming style
template <size_t ArgCount, size_t ParamCount>
struct TaskArgCountCheck {
    static_assert(
            ArgCount <= ParamCount,
            "Dependence argument count must not exceed task parameter count.");
    static constexpr bool value = true;
};

template <typename D, typename A, size_t Position>
struct TaskArgTypeMatchesParamType {
    typedef Datablock<typename Unpack<D>::Parameter> DepDH;
    typedef Datablock<typename Unpack<A>::Parameter> ArgDH;
    static_assert(std::is_base_of<ArgDH, DepDH>::value,
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
        return Launch(depv);
    }

 private:
    template <typename U = R, EnableIfVoid<U> = 0>
    static ocrGuid_t Launch(ocrEdtDep_t deps[]) {
        UnpackDeps(deps, internal::IndexSeqFor<Args...>());
        return NULL_GUID;
    }

    template <typename U = R, EnableIfNotVoid<U> = 0>
    static ocrGuid_t Launch(ocrEdtDep_t deps[]) {
        return UnpackDeps(deps, internal::IndexSeqFor<Args...>()).guid();
    }

    template <size_t... I>
    static R UnpackDeps(ocrEdtDep_t deps[], internal::IndexSeq<I...>) {
        static_cast<void>(deps);  // possibly unused if deps is empty
        static_assert(sizeof...(Args) == sizeof...(I), "?!");
        return user_fn((Args{deps[I]})...);
    }
};

}  // namespace internal

template <typename T>
using DataHandleOf = DataHandle<typename internal::Unpack<T>::Parameter>;

template <typename F>
class TaskTemplate;

template <typename F>
class TaskBuilder;

template <typename F>
class Task;

// TODO - optimize case when no deps are provided (or needed)
template <typename Ret, typename... Args>
class Task<Ret(Args...)> : public ObjectHandle {
 public:
    typedef Ret(F)(Args...);
    typedef typename internal::Unpack<Ret>::Parameter R;

    static constexpr u32 kDepc = sizeof...(Args);

    void Destroy() const { internal::OK(ocrEdtDestroy(this->guid())); }

    template <u32 slot, typename U>
    void AddDependence(U src, ocrDbAccessMode_t mode = DB_DEFAULT_MODE) const {
        namespace i = internal;
        using Expected = typename i::FnInfo<F>::template Arg<slot>::Type;
        static_assert(i::TaskArgTypeMatchesParamType<Expected, U, slot>::value,
                      "Dependence argument must match slot type.");
        ocrGuid_t src_guid = static_cast<DataHandleOf<U>>(src).guid();
        internal::OK(ocrAddDependence(src_guid, this->guid(), slot, mode));
    }

 protected:
    // TODO - paramv support (check if first arg of F isn't a Datablock)
    friend class TaskBuilder<F>;

    // TODO - add support for hints, output events, etc
    Task(Event<R> *out_event, ocrGuid_t task_template, ocrGuid_t depv[],
         const Hint *hint, u16 flags)
            : ObjectHandle(Init(out_event, task_template, depv, hint, flags)) {}

 private:
    static ocrGuid_t Init(Event<R> *out_event, ocrGuid_t task_template,
                          ocrGuid_t depv[], const Hint *hint, u16 flags) {
        ocrGuid_t guid;
        ocrGuid_t *out_guid = reinterpret_cast<ocrGuid_t *>(out_event);
        // TODO - open bug for adding const qualifiers in OCR C API.
        // E.g., "const ocrHint_t *hint" in ocrEdtCreate.
        ocrHint_t *raw_hint = const_cast<ocrHint_t *>(hint->internal());
        ocrEdtCreate(&guid, task_template, EDT_PARAM_DEF, nullptr,
                     EDT_PARAM_DEF, depv, flags, raw_hint, out_guid);
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

template <typename Ret, typename... Args>
class TaskBuilder<Ret(Args...)> {
 public:
    typedef Ret(F)(Args...);
    typedef typename internal::Unpack<Ret>::Parameter R;

    TaskBuilder(ocrGuid_t template_guid, const Hint *hint, u16 flags)
            : template_guid_(template_guid), hint_(hint), flags_(flags) {}

    Task<F> CreateTask(DataHandleOf<Args>... deps) {
        return HelpCreateTask(nullptr, deps...);
    }

    template <typename... Deps,
              internal::EnableIf<sizeof...(Deps) != sizeof...(Args)> = 0>
    Task<F> CreateTask(Deps... deps) {
        constexpr s64 kDeps = sizeof...(Deps);
        constexpr s64 kArgs = sizeof...(Args);
        constexpr s64 kDiff = kArgs - kDeps;
        // XXX - std::max isn't constexpr in C++11
        constexpr s64 kMissing = kDiff < 0 ? 0 : kDiff;
        // Note: this assertion will never fail because another assertion
        // in the internal check will always fail first (or none at all).
        static_assert(internal::TaskArgCountCheck<kDeps, kArgs>::value,
                      "Check for args/paramters mismatch.");
        return PadTask(internal::MakeIndexSeq<kMissing>{}, deps...);
    }

    DelayedFuture<F> CreateFuture(DataHandleOf<Args>... deps) {
        Event<R> out_event;
        auto task = HelpCreateTask(&out_event, deps...);
        return DelayedFuture<F>(task, out_event);
    }

    template <typename... Deps,
              internal::EnableIf<sizeof...(Deps) != sizeof...(Args)> = 0>
    Task<F> CreateFuture(Deps... deps) {
        constexpr s64 kDeps = sizeof...(Deps);
        constexpr s64 kArgs = sizeof...(Args);
        constexpr s64 kDiff = kArgs - kDeps;
        // XXX - std::max isn't constexpr in C++11
        constexpr s64 kMissing = kDiff < 0 ? 0 : kDiff;
        // Note: this assertion will never fail because another assertion
        // in the internal check will always fail first (or none at all).
        static_assert(internal::TaskArgCountCheck<kDeps, kArgs>::value,
                      "Check for args/paramters mismatch.");
        return PadFuture(internal::MakeIndexSeq<kMissing>{}, deps...);
    }

 private:
    Task<F> HelpCreateTask(Event<R> *out_event, DataHandleOf<Args>... deps) {
        namespace i = internal;
        bool is_future = out_event != nullptr;
        // Set provided dependences
        ocrGuid_t dummy_dep = is_future ? UNINITIALIZED_GUID : NULL_GUID;
        ocrGuid_t depv[1 + Task<F>::kDepc] = {
                (static_cast<DataHandleOf<Args>>(deps).guid())..., dummy_dep};
        // Set dummy dependence
        return Task<F>(out_event, template_guid_, depv, hint_, flags_);
    }

    template <size_t... I, typename... Deps>
    Task<F> PadTask(internal::IndexSeq<I...>, Deps... deps) {
        static_assert(sizeof...(I) + sizeof...(Deps) == sizeof...(Args),
                      "Correct total number of arguments for task.");
        return CreateTask(deps..., DefaultDependence(I)...);
    }

    template <size_t... I, typename... Deps>
    Task<F> PadFuture(internal::IndexSeq<I...>, Deps... deps) {
        static_assert(sizeof...(I) + sizeof...(Deps) == sizeof...(Args),
                      "Correct total number of arguments for task.");
        return CreateFuture(deps..., DefaultDependence(I)...);
    }

    const ocrGuid_t template_guid_;
    const Hint *const hint_;
    const u16 flags_;
};

template <typename F>
class TaskTemplate : public ObjectHandle {
 public:
    // TODO - add support for paramv
    // TODO - ensure that function's parameters are Datablocks
    template <F *user_fn>
    static TaskTemplate<F> Create() {
        ocrGuid_t guid;
        ocrEdt_t internal_fn =
                internal::TaskImplementation<F, user_fn, F>::InternalFn;
        constexpr u64 depc = internal::FnInfo<F>::kArgCount;
        ocrEdtTemplateCreate(&guid, internal_fn, 0, 1 + depc);
        return TaskTemplate<F>(guid);
    }

    TaskBuilder<F> operator()(u16 flags = EDT_PROP_NONE) const {
        return TaskBuilder<F>(this->guid(), nullptr, flags);
    }

    TaskBuilder<F> operator()(const Hint &hint,
                              u16 flags = EDT_PROP_NONE) const {
        return TaskBuilder<F>(this->guid(), &hint, flags);
    }

    void Destroy() const { internal::OK(ocrEdtTemplateDestroy(this->guid())); }

    ~TaskTemplate() = default;

 private:
    explicit TaskTemplate(ocrGuid_t guid) : ObjectHandle(guid) {}

    typedef typename internal::FnInfo<F>::Result Result;
    static_assert(std::is_same<void, Result>::value ||
                          std::is_base_of<ObjectHandle, Result>::value,
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

}  // namespace ocxxr

#define OCXXR_TEMPLATE_FOR(fn_ptr) \
    ocxxr::TaskTemplate<decltype(fn_ptr)>::Create<fn_ptr>();

#endif  // OCXXR_H_

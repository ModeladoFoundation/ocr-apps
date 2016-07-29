#ifndef OCXXR_EXTENSION_HPP_
#define OCXXR_EXTENSION_HPP_

extern "C" {
#include <extensions/ocr-labeling.h>
}

namespace ocxxr {

template <typename T>
class CountedEvent : public Event<T> {
 public:
    static CountedEvent Create(u64 count, u16 flags = 0,
                               Event<T> self = NullHandle()) {
        return CountedEvent(count, flags, self);
    }

 private:
    CountedEvent(u64 count, u16 flags, Event<T> self)
            : Event<T>(OCR_EVENT_COUNTED_T, flags, MakeParams(count), self) {}

    static ocrEventParams_t MakeParams(u64 count) {
        ocrEventParams_t params;
        params.EVENT_COUNTED.nbDeps = count;
        return params;
    }
};

template <typename T>
class ChannelEvent : public Event<T> {
 public:
    static ChannelEvent Create(u32 max_buffered, u16 flags = 0,
                               Event<T> self = NullHandle()) {
        return ChannelEvent(max_buffered, flags, self);
    }

 private:
    ChannelEvent(u32 max_buf, u16 flags, Event<T> self)
            : Event<T>(OCR_EVENT_CHANNEL_T, flags, MakeParams(max_buf), self) {}

    static ocrEventParams_t MakeParams(u32 max_buffered) {
        ocrEventParams_t params;
        params.EVENT_CHANNEL.maxGen = max_buffered;
        params.EVENT_CHANNEL.nbSat = 1;
        params.EVENT_CHANNEL.nbDeps = 1;
        return params;
    }
};

namespace internal {

template <typename T>
struct HandleRangeKind;

template <typename T>
struct HandleRangeKind<DatablockHandle<T>> {
    static constexpr ocrGuidUserKind kind = GUID_USER_DB;
};

template <typename T>
struct HandleRangeKind<Task<T>> {
    static constexpr ocrGuidUserKind kind = GUID_USER_EDT;
};

template <typename T>
struct HandleRangeKind<TaskTemplate<T>> {
    static constexpr ocrGuidUserKind kind = GUID_USER_EDT_TEMPLATE;
};

template <typename T>
struct HandleRangeKind<OnceEvent<T>> {
    static constexpr ocrGuidUserKind kind = GUID_USER_EVENT_ONCE;
};

template <typename T>
struct HandleRangeKind<CountedEvent<T>> {
    static constexpr ocrGuidUserKind kind = GUID_USER_EVENT_COUNTED;
};

template <typename T>
struct HandleRangeKind<IdempotentEvent<T>> {
    static constexpr ocrGuidUserKind kind = GUID_USER_EVENT_IDEM;
};

template <typename T>
struct HandleRangeKind<StickyEvent<T>> {
    static constexpr ocrGuidUserKind kind = GUID_USER_EVENT_STICKY;
};

template <typename T>
struct HandleRangeKind<LatchEvent<T>> {
    static constexpr ocrGuidUserKind kind = GUID_USER_EVENT_LATCH;
};

}  // namespace internal

template <typename T>
class HandleRange : public ObjectHandle {
 public:
    static HandleRange Create(u64 count, ocrGuidUserKind kind) {
        ocrGuid_t guid;
        internal::OK(ocrGuidRangeCreate(&guid, count, kind));
        return HandleRange(guid);
    }

    template <ocrGuidUserKind kind = internal::HandleRangeKind<T>::kind>
    static HandleRange Create(u64 count) {
        return Create(count, kind);
    }

    void Destroy() const { internal::OK(ocrGuidMapDestroy(this->guid())); }

    T operator[](u64 index) const {
        ocrGuid_t indexed_guid;
        internal::OK(ocrGuidFromIndex(&indexed_guid, this->guid(), index));
        return *reinterpret_cast<T *>(&indexed_guid);
    }

 protected:
    explicit HandleRange(ocrGuid_t guid) : ObjectHandle(guid) {}

    static_assert(internal::IsLegalHandle<T>::value,
                  "HandleRange type parameter must be a legal handle type.");
};

}  // namespace ocxxr

#endif  // OCXXR_EXTENSION_HPP_

#ifndef OCXXR_RELPTR_HPP_
#define OCXXR_RELPTR_HPP_

namespace ocxxr {

/**
 * This is our "relative pointer" class.
 * You should be able to use it pretty much just like a normal pointer.
 * However, you still need to be careful to only point to memory within
 * the same datablock. Nothing keeps you from creating a "relative pointer"
 * into another datablock, or even into the stack.
 */
template <typename T>
class RelPtr {
 public:
    // offset of 1 is impossible since this is larger than 1 byte
    constexpr RelPtr() : offset_(1) {}

    RelPtr(const RelPtr &other) { set(other); }

    RelPtr(const T *other) { set(other); }

    // TODO - ensure that default assignment operator still works correctly
    RelPtr<T> &operator=(const RelPtr &other) {
        set(other);
        return *this;
    }

    RelPtr<T> &operator=(const T *other) {
        set(other);
        return *this;
    }

    T &operator*() const { return *get(); }

    T *operator->() const { return get(); }

    T &operator[](const int index) const { return get()[index]; }

    operator T *() const { return get(); }

    // TODO - implement math operators, like increment and decrement

 private:
    ptrdiff_t offset_;

    ptrdiff_t base_ptr() const { return reinterpret_cast<ptrdiff_t>(this); }

    void set(const RelPtr &other) { set(other.get()); }

    void set(const T *other) {
        if (other == nullptr) {
            offset_ = 0;
        } else {
            offset_ = reinterpret_cast<ptrdiff_t>(other) - base_ptr();
        }
    }

    T *get() const {
        ASSERT(offset_ != 1);
        if (offset_ == 0) {
            return nullptr;
        } else {
            ptrdiff_t target = base_ptr() + offset_;
            return reinterpret_cast<T *>(target);
        }
    }
};

/**
 * This is our "based pointer" class.
 * You should be able to use it pretty much just like a normal pointer.
 * This class is safer than RelPtr, but not as efficient.
 */
template <typename T>
class BasedPtr {
 public:
    constexpr BasedPtr() : target_guid_(ERROR_GUID), offset_(0) {}

    BasedPtr(const BasedPtr &other) { set(other); }

    BasedPtr(const T *other) { set(other); }

    // TODO - ensure that default assignment operator still works correctly
    // must handle special cases too
    BasedPtr &operator=(const T *other) {
        set(other);
        return *this;
    }

    BasedPtr &operator=(const BasedPtr &other) {
        set(other);
        return *this;
    }

    T &operator*() const { return *get(); }

    T *operator->() const { return get(); }

    T &operator[](const int index) const { return get()[index]; }

    operator T *() const { return get(); }

    // TODO - implement math operators, like increment and decrement

 private:
    ocrGuid_t target_guid_;
    ptrdiff_t offset_;

    ptrdiff_t base_ptr() const { return reinterpret_cast<ptrdiff_t>(this); }

    void set(const BasedPtr &other) {
        if (ocrGuidIsUninitialized(other.target_guid_)) {
            set(other.get());
        } else {
            target_guid_ = other.target_guid_;
            offset_ = other.offset_;
        }
    }

    void set(const T *other) {
        internal::GuidOffsetForAddress(other, this, &target_guid_, &offset_);
    }

    T *get() const {
        ASSERT(!ocrGuidIsError(target_guid_));
        if (ocrGuidIsNull(target_guid_)) {
            return nullptr;
        } else if (ocrGuidIsUninitialized(target_guid_)) {
            // optimized case: treat as intra-datablock RelPtr
            ptrdiff_t target = base_ptr() + offset_;
            return reinterpret_cast<T *>(target);
        } else {
            // normal case: inter-datablock pointer
            ptrdiff_t target = internal::AddressForGuid(target_guid_) + offset_;
            return reinterpret_cast<T *>(target);
        }
    }
};

namespace internal {

template <typename T, unsigned N, template <typename> class P = RelPtr>
struct PointerNester {
    typedef P<typename PointerNester<T, N - 1, P>::Type> Type;
};

template <typename T, template <typename> class P>
struct PointerNester<T, 0, P> {
    typedef T Type;
};

template <typename T, template <typename> class P = RelPtr>
struct PointerConvertor;

template <typename T, template <typename> class P>
struct PointerConvertor<T *, P> {
    typedef P<T> Type;
};

template <typename T, template <typename> class P>
struct PointerConvertor<T **, P> {
    typedef P<typename PointerConvertor<T *, P>::Type> Type;
};

}  // namespace internal

template <typename T, unsigned N>
using NestedRelPtr = typename internal::PointerNester<T, N>::Type;

template <typename T>
using RelPtrFor = typename internal::PointerConvertor<T>::Type;

template <typename T, unsigned N>
using NestedBasedPtr = typename internal::PointerNester<T, N, BasedPtr>::Type;

template <typename T>
using BasedPtrFor = typename internal::PointerConvertor<T, BasedPtr>::Type;

}  // namespace ocxxr

#endif  // OCXXR_RELPTR_HPP_

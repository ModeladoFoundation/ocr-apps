#ifndef OCXXR_RELPTR_HPP_
#define OCXXR_RELPTR_HPP_

#ifdef INSTRUMENT_POINTER_OP
#include <atomic>
#include <iostream>
#endif

namespace ocxxr {

// FIXME - should also support conversion to superclass pointer types
// e.g., BasedPtr<Y> y = ...; BasedPtr<X> x = y; should work if Y extends X

#if OCXXR_USE_NATIVE_POINTERS
template <typename T>
using RelPtr = T*;
#else
template <typename T>
class RelPtr;
# endif

namespace internal {
#ifdef INSTRUMENT_POINTER_OP
extern std::atomic<u64> rp_indirect_count, rp_arrow_count, rp_subscript_count,
        rp_cast_count, rp_equal_count, rp_assign_count, rp_negate_count, rp_get_count, rp_set_count;
extern std::atomic<u64> bp_indirect_count, bp_arrow_count, bp_subscript_count,
        bp_cast_count, bp_equal_count, bp_assign_count, bp_negate_count, bp_get_count, bp_set_count;
void outputAllCount();
#endif

#ifdef SANITY_CHECK
inline void sanityCheck(ptrdiff_t base_ptr, u64 db_size, ptrdiff_t start_ptr) {
    ptrdiff_t end_ptr = base_ptr + db_size;
    if (start_ptr < base_ptr || start_ptr >= end_ptr) {
//        FIX ME: failed in BinaryTree and UTS
//        PRINTF("base_ptr = %lx, db_size = %lu, start_ptr = %lx\n", base_ptr, db_size, start_ptr);
//        abort();
    }
}
#endif

template <typename T, bool embedded>
class BasedPtrImpl {
 public:
    constexpr BasedPtrImpl() : target_guid_(ERROR_GUID), offset_(0) {}

    BasedPtrImpl(ocrGuid_t target, ptrdiff_t offset)
            : target_guid_(target), offset_(offset) {}

    template <typename U = T,
              EnableIf<embedded && std::is_same<T, U>::value> = 0>
    BasedPtrImpl(const BasedPtrImpl &other) {
        set(other);
    }

    // Should be auto-generated if above version is disabled
    // BasedPtrImpl(const BasedPtrImpl &) = default;

    BasedPtrImpl(const T *other) { set(other); }

    // TODO - ensure that default assignment operator still works correctly
    // must handle special cases too
    BasedPtrImpl &operator=(const T *other) {
#ifdef INSTRUMENT_POINTER_OP
        bp_assign_count++;
#endif
        set(other);
        return *this;
    }

    template <typename U = T,
              EnableIf<embedded && std::is_same<T, U>::value> = 0>
    BasedPtrImpl &operator=(const BasedPtrImpl &other) {
#ifdef INSTRUMENT_POINTER_OP
        bp_assign_count++;
#endif
        set(other);
        return *this;
    }

    // Should be auto-generated if above version is disabled
    // BasedPtrImpl &operator=(const BasedPtrImpl &) = default;

    template <typename U = T, EnableIfNotVoid<U> = 0>
    U &operator*() const {
#ifdef INSTRUMENT_POINTER_OP
        bp_indirect_count++;
#endif
        return *get();
    }

    T *operator->() const {
#ifdef INSTRUMENT_POINTER_OP
        bp_arrow_count++;
#endif
        return get();
    }

#if 0 // shoudln't need this?
    template <typename U = T, EnableIfNotVoid<U> = 0>
    U &operator[](const int index) const {
#ifdef INSTRUMENT_POINTER_OP
        bp_subscript_count++;
#endif
        return get()[index];
    }
#endif

    operator T *() const {
#ifdef INSTRUMENT_POINTER_OP
        bp_cast_count++;
#endif
        return get();
    }

#if !(OCXXR_USE_NATIVE_POINTERS)
    operator RelPtr<T>() const {
#ifdef INSTRUMENT_POINTER_OP
        bp_cast_count++;
#endif
        return get();
    }
#endif

    // Allow conversion from optimized "embedded" case to general case
    operator BasedPtrImpl<T, false>() const {
#ifdef INSTRUMENT_POINTER_OP
        bp_cast_count++;
#endif
        return get();
    }

    bool operator!() const {
#ifdef INSTRUMENT_POINTER_OP
        bp_negate_count++;
#endif
        return ocrGuidIsNull(target_guid_);
    }

    bool operator==(const BasedPtrImpl &other) const {
#ifdef INSTRUMENT_POINTER_OP
        bp_equal_count++;
#endif
        return ocrGuidIsEq(target_guid_, other.target_guid_) &&
               offset_ == other.offset_;
    }

    ocrGuid_t target_guid() const { return target_guid_; }

    ArenaHandle<void> target_handle() const {
        return ArenaHandle<void>(target_guid_);
    }

    Arena<void> target_arena() const {
        void *ptr = reinterpret_cast<void *>(
                internal::AddressForGuid(target_guid_));
        ocrEdtDep_t dep = {.guid = target_guid_, .ptr = ptr};
        return Arena<void>(dep);
    }

    // TODO - implement math operators, like increment and decrement

    template <typename U = T,
              EnableIf<embedded && std::is_same<T, U>::value> = 0>
    bool target_is_local() {
        return ocrGuidIsUninitialized(target_guid_);
    }

 private:
    ocrGuid_t target_guid_;
    ptrdiff_t offset_;

 protected:
    ptrdiff_t base_ptr() const { return reinterpret_cast<ptrdiff_t>(this); }

    void set(const BasedPtrImpl &other) {
        if (embedded && ocrGuidIsUninitialized(other.target_guid_)) {
            set(other.get());
        } else {
            target_guid_ = other.target_guid_;
            offset_ = other.offset_;
//#ifdef SANITY_CHECK
//            u64 db_size;
//            ptrdiff_t base = internal::AddressForGuid(target_guid_);
//            ptrdiff_t target = internal::CombineBaseOffset(base, offset_);
//            ocrDbGetSize(target_guid_, &db_size);
//            sanityCheck(base, db_size, target);
//#endif

#ifdef INSTRUMENT_POINTER_OP
            bp_set_count++;
#endif
        }
    }

    void set(const T *other) {
        internal::GuidOffsetForAddress(other, this, &target_guid_, &offset_,
                                       embedded);

//#ifdef SANITY_CHECK
//        if (other) {
//            u64 db_size;
//            ptrdiff_t base = internal::AddressForGuid(target_guid_);
//            ocrDbGetSize(target_guid_, &db_size);
//            ptrdiff_t ptr = reinterpret_cast<ptrdiff_t>(other);
//            sanityCheck(base, db_size, ptr);
//        }
//#endif

#ifdef INSTRUMENT_POINTER_OP
            bp_set_count++;
#endif
    }

    T *get() const {
#ifdef INSTRUMENT_POINTER_OP
            bp_get_count++;
#endif
        assert(!ocrGuidIsError(target_guid_));
        if (ocrGuidIsNull(target_guid_)) {
            return nullptr;
        } else if (embedded && ocrGuidIsUninitialized(target_guid_)) {
            // optimized case: treat as intra-datablock RelPtr
            ptrdiff_t target = internal::CombineBaseOffset(base_ptr(), offset_);
            return reinterpret_cast<T *>(target);
        } else {
// normal case: inter-datablock pointer
#if OCXXR_USE_NATIVE_POINTERS
            constexpr ptrdiff_t base = 0;
#else   // !OCXXR_USE_NATIVE_POINTERS
            ptrdiff_t base = internal::AddressForGuid(target_guid_);
#endif  // OCXXR_USE_NATIVE_POINTERS
            ptrdiff_t target = internal::CombineBaseOffset(base, offset_);

#ifdef SANITY_CHECK
            u64 db_size;
            ocrDbGetSize(target_guid_, &db_size);
            sanityCheck(base, db_size, target);
#endif
            return reinterpret_cast<T *>(target);
        }
    }
};

}  // namespace internal

/**
 * This is our "based pointer" class.
 * You should be able to use it pretty much just like a normal pointer.
 * This class is safer than RelPtr, but not as efficient.
 */
template <typename T>
using BasedPtr = internal::BasedPtrImpl<T, false>;

/**
 * This is our "based datablock pointer" class.
 * Behaves like a BasedPtr in most cases, but can be optimized
 * to behave like a RelPtr if this and the target object
 * are in the same datablock. As a consequence, these pointer
 * objects may only be allocated within an acquired datablock.
 */
template <typename T>
using BasedDbPtr = internal::BasedPtrImpl<T, true>;

#if !(OCXXR_USE_NATIVE_POINTERS)
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
#ifdef INSTRUMENT_POINTER_OP
        internal::rp_assign_count++;
#endif
        set(other);
        return *this;
    }

    RelPtr<T> &operator=(const T *other) {
#ifdef INSTRUMENT_POINTER_OP
        internal::rp_assign_count++;
#endif
        set(other);
        return *this;
    }

    template <typename U = T, internal::EnableIfNotVoid<U> = 0>
    U &operator*() const {
#ifdef INSTRUMENT_POINTER_OP
        internal::rp_indirect_count++;
#endif
        return *get();
    }

    T *operator->() const {
#ifdef INSTRUMENT_POINTER_OP
        internal::rp_arrow_count++;
#endif
        return get();
    }

#if 0 // shoudln't need this?
    template <typename U = T, internal::EnableIfNotVoid<U> = 0>
    U &operator[](const int index) const {
#ifdef INSTRUMENT_POINTER_OP
        internal::rp_subscript_count++;
#endif
        return get()[index];
    }
#endif

    operator T *() const {
#ifdef INSTRUMENT_POINTER_OP
        internal::rp_cast_count++;
#endif
        return get();
    }

    operator BasedPtr<T>() const {
#ifdef INSTRUMENT_POINTER_OP
        internal::rp_cast_count++;
#endif
        return get();
    }

    bool operator!() const {
#ifdef INSTRUMENT_POINTER_OP
        internal::rp_negate_count++;
#endif
        return offset_ == 0;
    }

    bool operator==(const RelPtr &other) const {
#ifdef INSTRUMENT_POINTER_OP
        internal::rp_equal_count++;
#endif
        return get() == other.get();
    }

    // TODO - implement math operators, like increment and decrement

 private:
    ptrdiff_t offset_;

    ptrdiff_t base_ptr() const { return reinterpret_cast<ptrdiff_t>(this); }

    void set(const RelPtr &other) { set(other.get()); }

    void set(const T *other) {
#ifdef INSTRUMENT_POINTER_OP
            internal::rp_set_count++;
#endif
        if (other == nullptr) {
            offset_ = 0;
        } else {
            ptrdiff_t ptr = reinterpret_cast<ptrdiff_t>(other);
            offset_ = internal::CombineBaseOffset(-base_ptr(), ptr);
#ifdef SANITY_CHECK
            ocrGuid_t guid_out;
            ptrdiff_t offset_out;
            internal::GuidOffsetForAddress(this, this, &guid_out, &offset_out,
                                           false);
#endif
        }
    }

    T *get() const {
#ifdef INSTRUMENT_POINTER_OP
            internal::rp_get_count++;
#endif
        assert(offset_ != 1);
        if (offset_ == 0) {
            return nullptr;
        } else {
            ptrdiff_t target = internal::CombineBaseOffset(base_ptr(), offset_);
//#ifdef SANITY_CHECK
//            ocrGuid_t guid_out;
//            ptrdiff_t offset_out;
//            internal::GuidOffsetForAddress(this, this, &guid_out, &offset_out,
//                                           false);
//#endif
            return reinterpret_cast<T *>(target);
        }
    }
};
#endif // OCXXR_USE_NATIVE_POINTERS

}  // namespace ocxxr

#endif  // OCXXR_RELPTR_HPP_

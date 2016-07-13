#ifndef OCXXR_RELPTR_HPP_
#define OCXXR_RELPTR_HPP_

#include <cassert>
#include <cstddef>

namespace ocxxr {

// TODO - Rename variables (imported from older repo)
// TODO - Get rid of C-style casts

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
    constexpr RelPtr() : offset(1) {}

    RelPtr(const T *other) { set(other); }

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
    ptrdiff_t offset;

    void set(const T *other) {
        if (other == nullptr) {
            offset = 0;
        } else {
            offset = (char *)other - (char *)this;
        }
    }

    T *get() const {
        assert(offset != 1);
        if (offset == 0)
            return nullptr;
        else {
            char *target = (char *)this + offset;
            return (T *)target;
        }
    }
};

namespace internal {

template <typename T, unsigned N>
struct PointerNester {
    typedef RelPtr<typename PointerNester<T, N - 1>::Type> Type;
};

template <typename T>
struct PointerNester<T, 0> {
    typedef T Type;
};

}  // namespace internal

template <typename T, unsigned N>
using NestedRelPtr = typename internal::PointerNester<T, N>::Type;

}  // namespace ocxxr

#endif  // OCXXR_RELPTR_HPP_

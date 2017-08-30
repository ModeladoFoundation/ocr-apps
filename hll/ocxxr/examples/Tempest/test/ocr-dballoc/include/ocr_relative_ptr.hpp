#ifndef _OCR_RELATIVE_PTR_HPP_
#define _OCR_RELATIVE_PTR_HPP_

#include <cstddef>
#include <cassert>
#include <ocxxr.hpp>

namespace Ocr {
#if 0 // DISABLED
    /**
     * This is our "relative pointer" class.
     * You should be able to use it pretty much just like a normal pointer.
     * However, you still need to be careful to only point to memory within
     * the same datablock. Nothing keeps you from creating a "relative pointer"
     * into another datablock, or even into the stack.
     */
    template <typename T>
    struct OcrRelativePtr {
        private:
            ptrdiff_t offset;

            void set(const T *other) {
                if (other == nullptr) {
                    offset = 0;
                }
                else {
                    offset = (char*)other - (char*)this;
                }
            }

            T *get() const {
                assert(offset != 1);
                if (offset == 0) return nullptr;
                else {
                    char *target = (char*)this + offset;
                    return (T*)target;
                }
            }

        public:
            // offset of 1 is impossible since this is larger than 1 byte
            constexpr OcrRelativePtr(): offset(1) {}

            OcrRelativePtr(const T *other) { set(other); }

            OcrRelativePtr<T> &operator=(const T *other) {
                set(other);
                return *this;
            }

            T &operator*() const { return *get(); }

            T *operator->() const { return get(); }

            T &operator[](const int index) const { return get()[index]; }

            operator T*() const { return get(); }

            /* TODO - implement math operators, like increment and decrement */
    };

    template <typename T> using RelPtr = OcrRelativePtr<T>;

    namespace Internal {
        template <typename T, unsigned N>
        struct PointerNester {
            typedef RelPtr<typename PointerNester<T, N-1>::type> type;
        };

        template <typename T>
        struct PointerNester<T, 0> {
            typedef T type;
        };
    }

    template <typename T, unsigned N> using NestedRelPtr = typename Internal::PointerNester<T,N>::type;
#else
    template <typename T> using OcrRelativePtr = ocxxr::RelPtr<T>;
    template <typename T> using RelPtr = ocxxr::RelPtr<T>;
    template <typename T, unsigned N> using NestedRelPtr = ocxxr::NestedRelPtr<T, N>;
#endif // DISABLED

}

#endif /* _OCR_RELATIVE_PTR_HPP_ */



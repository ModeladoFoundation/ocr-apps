#ifndef OCXXR_RELPTR_UTIL_HPP_
#define OCXXR_RELPTR_UTIL_HPP_

namespace ocxxr {
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

#endif  // OCXXR_RELPTR_UTIL_HPP_

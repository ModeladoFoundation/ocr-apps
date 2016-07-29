#ifndef OCXXR_UTIL_HPP_
#define OCXXR_UTIL_HPP_

namespace ocxxr {
namespace internal {

#ifdef OCXXR_USING_CXX14
// std::is_trivially_copyable is missing in GCC-4
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

// TODO - move all of these to a "util" namespace
// (nothing in "internal" should be in the public API)
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

template <typename T, typename U, typename V = int>
using EnableIfNotSame = EnableIf<!std::is_same<T, U>::value, V>;

// Check error status of C API call
inline void OK(u8 status) { ASSERT(status == 0); }

}  // namespace internal
}  // namespace ocxxr

#endif  // OCXXR_UTIL_HPP_

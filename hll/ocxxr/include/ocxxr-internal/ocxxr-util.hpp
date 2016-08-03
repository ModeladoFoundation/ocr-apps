#ifndef OCXXR_UTIL_HPP_
#define OCXXR_UTIL_HPP_

#include <cstddef>

// TODO - These temporary-data allocation macros should be defined
// per-platform with an efficient (and legal) implementation.
// The delete macros must be null-safe.

#define OCXXR_TEMP_NEW(type) (new type)
#define OCXXR_TEMP_NEW_ZERO(type) (new type{})
#define OCXXR_TEMP_DELETE(var) (delete var)

#define OCXXR_TEMP_ARRAY_NEW(type, count) (new type[count])
#define OCXXR_TEMP_ARRAY_NEW_ZERO(type, count) (new type[count]{})
#define OCXXR_TEMP_ARRAY_DELETE(var) (delete[] var)

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

template <typename T>
struct IsVoid {
    static constexpr bool Value = std::is_same<T, void>::value;
};

template <typename T, typename U>
struct IsBaseOf {
    static constexpr bool Value = std::is_base_of<T, U>::value;
};

template <typename T, typename U = int>
using EnableIfVoid = EnableIf<IsVoid<T>::Value, U>;

template <typename T, typename U = int>
using EnableIfNotVoid = EnableIf<!IsVoid<T>::Value, U>;

template <typename T, typename U, typename V = int>
using EnableIfBaseOf = EnableIf<IsBaseOf<T, U>::Value, V>;

template <typename T, typename U, typename V = int>
using EnableIfSame = EnableIf<std::is_same<T, U>::value, V>;

template <typename T, typename U, typename V = int>
using EnableIfNotSame = EnableIf<!std::is_same<T, U>::value, V>;

template <typename T, typename U = unsigned char>
using IfVoid = typename std::conditional<IsVoid<T>::Value, U, T>::type;

template <typename T>
struct SizeOf {
    static constexpr size_t Value = IsVoid<T>::Value ? 0 : sizeof(IfVoid<T>);
};

// Check error status of C API call
inline void OK(u8 status) { ASSERT(status == 0); }

// defined in ocxxr-task-state.hpp
inline void PushTaskState();

// defined in ocxxr-task-state.hpp
inline void PopTaskState();

namespace bookkeeping {

// defined in ocxxr-task-state.hpp
inline void RemoveDatablock(ocrGuid_t guid);

// defined in ocxxr-task-state.hpp
inline void AddDatablock(ocrGuid_t guid, void *base_address);

}  // namespace bookkeeping

// defined in ocxxr-task-state.hpp
inline ptrdiff_t AddressForGuid(ocrGuid_t);

// defined in ocxxr-task-state.hpp
inline void GuidOffsetForAddress(const void *target, const void *source,
                                 ocrGuid_t *guid_out, ptrdiff_t *offset_out);

}  // namespace internal
}  // namespace ocxxr

#endif  // OCXXR_UTIL_HPP_

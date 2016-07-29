#ifndef OCXXR_CXX11_COMPAT_HPP_
#define OCXXR_CXX11_COMPAT_HPP_

namespace ocxxr {
namespace internal {

namespace fromboost {
#include <ocxxr-internal/boost-code/ocxxr-boost-index-seq.inc>
}  // namespace fromboost

template <typename T>
struct DummyLookup {
    static constexpr bool value = true;
};

// std::is_trivially_copyable is missing in GCC-4
template <typename T>
using IsTriviallyCopyable = DummyLookup<T>;

// index_sequence is C++14 only
template <size_t... Indices>
using IndexSeq = typename fromboost::index_sequence<Indices...>;
template <size_t N>
using MakeIndexSeq = typename fromboost::make_index_sequence<N>;
template <typename... T>
using IndexSeqFor = typename fromboost::make_index_sequence<sizeof...(T)>;

}  // namespace internal
}  // namespace ocxxr

#endif  // OCXXR_CXX11_COMPAT_HPP_

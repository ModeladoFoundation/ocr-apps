// This file has been modified from its original version.
// The original can be found at the following URL:
// http://www.boost.org/doc/libs/1_60_0/boost/fusion/support/detail/index_sequence.hpp
/*=============================================================================
    Copyright (c) 2015 Agustin K-ballo Berge
    Copyright (c) 2015 Kohei Takahashi

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

template <std::size_t... Ints>
struct index_sequence {
    typedef std::size_t value_type;

    constexpr static std::size_t size() noexcept { return sizeof...(Ints); }

    // non standard extension
    typedef index_sequence type;
};

template <typename Left, typename Right>
struct _make_index_sequence_join;

template <std::size_t... Left, std::size_t... Right>
struct _make_index_sequence_join<index_sequence<Left...>,
                                 index_sequence<Right...> >
        : index_sequence<Left..., (sizeof...(Left) + Right)...> {};

template <std::size_t N>
struct make_index_sequence
        : _make_index_sequence_join<
                  typename make_index_sequence<N / 2>::type,
                  typename make_index_sequence<N - N / 2>::type> {};

template <>
struct make_index_sequence<1> : index_sequence<0> {};

template <>
struct make_index_sequence<0> : index_sequence<> {};

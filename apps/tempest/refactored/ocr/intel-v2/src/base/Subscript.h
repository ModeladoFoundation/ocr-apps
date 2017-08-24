///////////////////////////////////////////////////////////////////////////////
///
///	\file    Subscript.h
///	\author  Bryce Adelstein-Lelbach aka wash
///	\version January 28, 2016
///
///	<summary>
///		This file contains Subscript, a simple C++ meta-programming facility
///     which enables C-style subscript indexing for contiguous
///     multi-dimensional arrays.
///	</summary>
///	<remarks>
///		Copyright 2016 Bryce Adelstein-Lelbach aka wash
///
///     Distributed under the Boost Software License, Version 1.0. (See
///     accompanying file LICENSE_1_0.txt or copy at
///     http://www.boost.org/LICENSE_1_0.txt)
///	</remarks>

#if !defined(_SUBSCRIPT_H_)
#define _SUBSCRIPT_H_

///////////////////////////////////////////////////////////////////////////////

#include <array>
#include <type_traits>
#include <cstddef>

// expr := subscript+
// subscript := '[' c++-integral-expression ']'

template <typename T, std::ptrdiff_t FreeDims, std::ptrdiff_t NumDims>
struct Subscript;

template <typename T, std::ptrdiff_t FreeDims, std::ptrdiff_t NumDims>
struct Subscript
{
    typedef std::ptrdiff_t size_type;

    // The index of the first free dimension.
    enum { Dim = NumDims - FreeDims };

    T& object_;
    std::array<size_type, Dim> indices_;

    constexpr Subscript(T& object) noexcept
      : object_(object), indices_() {}

    Subscript(
        size_type head
      , Subscript<T, FreeDims + 1, NumDims> const& tail
      ) noexcept
      : object_(tail.object_)
    {
        for (size_type i = 0; i < Dim - 1; ++i)
            indices_[i] = tail.indices_[i];
        indices_[Dim - 1] = head;
    }

    Subscript<T, FreeDims - 1, NumDims> operator[](size_type idx) const noexcept
    {
        return Subscript<T, FreeDims - 1, NumDims>(idx, *this);
    }
};

// This specialization is instantiated for the final index, and completes the
// indexing operation.
template <typename T, std::ptrdiff_t NumDims>
struct Subscript<T, 1, NumDims>
{
    typedef std::ptrdiff_t size_type;

    // The index of the first free dimension.
    enum { Dim = NumDims - 1 };

    T& object_;
    std::array<size_type, Dim> indices_;

    constexpr Subscript(T& object) noexcept
      : object_(object), indices_() {}

    Subscript(
        size_type head
      , Subscript<T, 2, NumDims> const& tail
       ) noexcept
      : object_(tail.object_)
    {
        for (size_type i = 0; i < Dim - 1; ++i)
            indices_[i] = tail.indices_[i];
        indices_[Dim - 1] = head;
    }

    // Conversion-to-pointer operator, which allows us to return contigous
    // unit-stride slices.
    operator typename std::conditional<
        // If we're bound to a const object...
        std::is_const<typename std::remove_reference<T>::type>::value,
        // ... then we're convertible to a const pointer ...
        typename T::ValueType const*,
        // ... otherwise, we're convertible to a non-const pointer.
        typename T::ValueType*
    >::type() const
    {
        return object_(indices_);
    }
};

#endif // _SUBSCRIPT_H_


//
// cancellation_type.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_CANCELLATION_TYPE_HPP
#define XIO_CANCELLATION_TYPE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#include <xio/detail/push_options.h>

namespace xio {


    enum class cancellation_type : unsigned int {
        none = 0,
        terminal = 1,
        partial = 2,
        total = 4,
        all = 0xFFFFFFFF
    };

    typedef cancellation_type cancellation_type_t;


    /// Negation operator.
    /**
 * @relates cancellation_type
 */
    inline constexpr bool operator!(cancellation_type_t x) {
        return static_cast<unsigned int>(x) == 0;
    }

    /// Bitwise and operator.
    /**
 * @relates cancellation_type
 */
    inline constexpr cancellation_type_t operator&(
        cancellation_type_t x, cancellation_type_t y) {
        return static_cast<cancellation_type_t>(
            static_cast<unsigned int>(x) & static_cast<unsigned int>(y));
    }

    /// Bitwise or operator.
    /**
 * @relates cancellation_type
 */
    inline constexpr cancellation_type_t operator|(
        cancellation_type_t x, cancellation_type_t y) {
        return static_cast<cancellation_type_t>(
            static_cast<unsigned int>(x) | static_cast<unsigned int>(y));
    }

    /// Bitwise xor operator.
    /**
 * @relates cancellation_type
 */
    inline constexpr cancellation_type_t operator^(
        cancellation_type_t x, cancellation_type_t y) {
        return static_cast<cancellation_type_t>(
            static_cast<unsigned int>(x) ^ static_cast<unsigned int>(y));
    }

    /// Bitwise negation operator.
    /**
 * @relates cancellation_type
 */
    inline constexpr cancellation_type_t operator~(cancellation_type_t x) {
        return static_cast<cancellation_type_t>(~static_cast<unsigned int>(x));
    }

    /// Bitwise and-assignment operator.
    /**
 * @relates cancellation_type
 */
    inline cancellation_type_t &operator&=(
        cancellation_type_t &x, cancellation_type_t y) {
        x = x & y;
        return x;
    }

    /// Bitwise or-assignment operator.
    /**
 * @relates cancellation_type
 */
    inline cancellation_type_t &operator|=(
        cancellation_type_t &x, cancellation_type_t y) {
        x = x | y;
        return x;
    }

    /// Bitwise xor-assignment operator.
    /**
 * @relates cancellation_type
 */
    inline cancellation_type_t &operator^=(
        cancellation_type_t &x, cancellation_type_t y) {
        x = x ^ y;
        return x;
    }


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_CANCELLATION_TYPE_HPP

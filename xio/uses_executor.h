//
// uses_executor.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_USES_EXECUTOR_HPP
#define XIO_USES_EXECUTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>

#include <xio/detail/push_options.h>

namespace xio {


    /// A special type, similar to std::nothrow_t, used to disambiguate
/// constructors that accept executor arguments.
    /**
 * The executor_arg_t struct is an empty structure type used as a unique type
 * to disambiguate constructor and function overloading. Specifically, some
 * types have constructors with executor_arg_t as the first argument,
 * immediately followed by an argument of a type that satisfies the Executor
 * type requirements.
 */
    struct executor_arg_t {
        /// Constructor.
        constexpr executor_arg_t() noexcept {
        }
    };

    /// A special value, similar to std::nothrow, used to disambiguate constructors
/// that accept executor arguments.
    /**
 * See xio::executor_arg_t and xio::uses_executor
 * for more information.
 */
inline constexpr executor_arg_t executor_arg;

    /// The uses_executor trait detects whether a type T has an associated executor
/// that is convertible from type Executor.
    /**
 * Meets the BinaryTypeTrait requirements. The Asio library provides a
 * definition that is derived from std::false_type. A program may specialize this
 * template to derive from std::true_type for a user-defined type T that can be
 * constructed with an executor, where the first argument of a constructor has
 * type executor_arg_t and the second argument is convertible from type
 * Executor.
 */
    template<typename T, typename Executor>
    struct uses_executor : std::false_type {
    };


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_USES_EXECUTOR_HPP

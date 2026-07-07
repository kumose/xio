//
// is_executor.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IS_EXECUTOR_HPP
#define ASIO_IS_EXECUTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/is_executor.h>

#include <xio/detail/push_options.h>

namespace xio {


    /// The is_executor trait detects whether a type T meets the Executor type
/// requirements.
    /**
 * Class template @c is_executor is a UnaryTypeTrait that is derived from @c
 * std::true_type if the type @c T meets the syntactic requirements for Executor,
 * otherwise @c std::false_type.
 */




    template<typename T>
    struct is_executor
#if defined(GENERATING_DOCUMENTATION)
            : std::integral_constant<bool, automatically_determined>
#else // defined(GENERATING_DOCUMENTATION)
            : xio::detail::is_executor<T>
#endif // defined(GENERATING_DOCUMENTATION)
    {
    };


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_IS_EXECUTOR_HPP

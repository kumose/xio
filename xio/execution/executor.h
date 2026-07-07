//
// execution/executor.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXECUTION_EXECUTOR_HPP
#define ASIO_EXECUTION_EXECUTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>
#include <xio/execution/invocable_archetype.h>
#include <xio/traits/equality_comparable.h>
#include <xio/traits/execute_member.h>
#include <xio/detail/push_options.h>

namespace xio {


    namespace execution {
        namespace detail {
            template<typename T, typename F,
                typename = void, typename = void, typename = void, typename = void,
                typename = void, typename = void, typename = void, typename = void>
            struct is_executor_of_impl : std::false_type {
            };

            template<typename T, typename F>
            struct is_executor_of_impl<T, F,
                std::enable_if_t<
                    traits::execute_member<std::add_const_t<T>, F>::is_valid
                >,
                void_t <
                result_of_t < std::decay_t<F> & ()>
            >
            ,
            std::enable_if_t<
                std::is_constructible<std::decay_t<F>, F>::value
            >
            ,
            std::enable_if_t<
                std::is_move_constructible<std::decay_t<F> >::value
            >
            ,
            std::enable_if_t<
                std::is_nothrow_copy_constructible<T>::value
            >
            ,
            std::enable_if_t<
                std::is_nothrow_destructible<T>::value
            >
            ,
            std::enable_if_t<
                traits::equality_comparable<T>::is_valid
            >
            ,
            std::enable_if_t<
                traits::equality_comparable<T>::is_noexcept
            >
            >
            :
            std::true_type {
            };
        } // namespace detail

        /// The is_executor trait detects whether a type T satisfies the
/// execution::executor concept.
        /**
 * Class template @c is_executor is a UnaryTypeTrait that is derived from @c
 * std::true_type if the type @c T meets the concept definition for an executor,
 * otherwise @c std::false_type.
 */
        template<typename T>
        struct is_executor :
#if defined(GENERATING_DOCUMENTATION)
                std::integral_constant<bool, automatically_determined>
#else // defined(GENERATING_DOCUMENTATION)
                detail::is_executor_of_impl<T, invocable_archetype>
#endif // defined(GENERATING_DOCUMENTATION)
        {
        };


        template<typename T>
        constexpr const bool is_executor_v = is_executor<T>::value;


#if defined(ASIO_HAS_CONCEPTS)

        template<typename T>
        ASIO_CONCEPT executor = is_executor<T>::value;

#define ASIO_EXECUTION_EXECUTOR ::xio::execution::executor

#else // defined(ASIO_HAS_CONCEPTS)

#define ASIO_EXECUTION_EXECUTOR typename

#endif // defined(ASIO_HAS_CONCEPTS)
    } // namespace execution

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_EXECUTION_EXECUTOR_HPP

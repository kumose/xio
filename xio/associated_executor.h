//
// associated_executor.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_ASSOCIATED_EXECUTOR_HPP
#define XIO_ASSOCIATED_EXECUTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/associator.h>
#include <functional>
#include <xio/detail/type_traits.h>
#include <xio/execution/executor.h>
#include <xio/execution_context.h>
#include <xio/inline_executor.h>
#include <xio/is_executor.h>

#include <xio/detail/push_options.h>

namespace xio {

    template<typename T, typename Executor>
    struct associated_executor;

    namespace detail {
        template<typename T, typename = void>
        struct has_executor_type : std::false_type {
        };

        template<typename T>
        struct has_executor_type<T, void_t<typename T::executor_type> >
                : std::true_type {
        };

        template<typename T, typename E, typename = void, typename = void>
        struct associated_executor_impl {
            typedef void xio_associated_executor_is_unspecialised;

            typedef E type;

            static type get(const T &) noexcept {
                return type();
            }

            static const type &get(const T &, const E &e) noexcept {
                return e;
            }
        };

        template<typename T, typename E>
        struct associated_executor_impl<T, E, void_t<typename T::executor_type> > {
            typedef typename T::executor_type type;

            static auto get(const T &t) noexcept
                -> decltype(t.get_executor()) {
                return t.get_executor();
            }

            static auto get(const T &t, const E &) noexcept
                -> decltype(t.get_executor()) {
                return t.get_executor();
            }
        };

        template<typename T, typename E>
        struct associated_executor_impl<T, E,
            std::enable_if_t <
            !has_executor_type<T>::value
        >
        ,
        void_t<
            typename associator<associated_executor, T, E>::type
        >
        >
        :
        associator<associated_executor, T, E> {
        };
    } // namespace detail

    /// Traits type used to obtain the executor associated with an object.
    /**
 * A program may specialise this traits type if the @c T template parameter in
 * the specialisation is a user-defined type. The template parameter @c
 * Executor shall be a type meeting the Executor requirements.
 *
 * Specialisations shall meet the following requirements, where @c t is a const
 * reference to an object of type @c T, and @c e is an object of type @c
 * Executor.
 *
 * @li Provide a nested typedef @c type that identifies a type meeting the
 * Executor requirements.
 *
 * @li Provide a noexcept static member function named @c get, callable as @c
 * get(t) and with return type @c type or a (possibly const) reference to @c
 * type.
 *
 * @li Provide a noexcept static member function named @c get, callable as @c
 * get(t,e) and with return type @c type or a (possibly const) reference to @c
 * type.
 */
    template<typename T, typename Executor = inline_executor>
    struct associated_executor
            : detail::associated_executor_impl<T, Executor>
    {
    };

    /// Helper function to obtain an object's associated executor.
    /**
 * @returns <tt>associated_executor<T>::get(t)</tt>
 */
    template<typename T>
    [[nodiscard]] inline

    typename associated_executor<T>::type
    get_associated_executor(const T &t) noexcept {
        return associated_executor<T>::get(t);
    }

    /// Helper function to obtain an object's associated executor.
    /**
 * @returns <tt>associated_executor<T, Executor>::get(t, ex)</tt>
 */
    template<typename T, typename Executor>
[[nodiscard]] inline auto get_associated_executor(
        const T &t, const Executor &ex,
        constraint_t<
            is_executor<Executor>::value || execution::is_executor<Executor>::value
        > = 0) noexcept
        -> decltype(associated_executor<T, Executor>::get(t, ex)) {
        return associated_executor<T, Executor>::get(t, ex);
    }

    /// Helper function to obtain an object's associated executor.
    /**
 * @returns <tt>associated_executor<T, typename
 * ExecutionContext::executor_type>::get(t, ctx.get_executor())</tt>
 */
    template<typename T, typename ExecutionContext>
    [[nodiscard]] inline

    typename associated_executor<T,
        typename ExecutionContext::executor_type>::type
    get_associated_executor(const T &t, ExecutionContext &ctx,
                            constraint_t<std::is_convertible<ExecutionContext &,
                                execution_context &>::value> = 0) noexcept {
        return associated_executor<T,
            typename ExecutionContext::executor_type>::get(t, ctx.get_executor());
    }

    template<typename T, typename Executor = inline_executor>
    using associated_executor_t = typename associated_executor<T, Executor>::type;

    namespace detail {
        template<typename T, typename E, typename = void>
        struct associated_executor_forwarding_base {
        };

        template<typename T, typename E>
        struct associated_executor_forwarding_base<T, E,
            std::enable_if_t <
            std::is_same <
            typename associated_executor<T,
                E>::xio_associated_executor_is_unspecialised,
            void>::value
        >
        >
{
  typedef void xio_associated_executor_is_unspecialised;
};
    } // namespace detail

    /// Specialisation of associated_executor for @c std::reference_wrapper.
    template<typename T, typename Executor>
    struct associated_executor<std::reference_wrapper<T>, Executor>
            : detail::associated_executor_forwarding_base<T, Executor>
    {
        /// Forwards @c type to the associator specialisation for the unwrapped type
  /// @c T.
        typedef typename associated_executor<T, Executor>::type type;

        /// Forwards the request to get the executor to the associator specialisation
  /// for the unwrapped type @c T.
        static type get(std::reference_wrapper<T> t) noexcept {
            return associated_executor<T, Executor>::get(t.get());
        }

        /// Forwards the request to get the executor to the associator specialisation
  /// for the unwrapped type @c T.
        static auto get(std::reference_wrapper<T> t, const Executor &ex) noexcept
            -> decltype(associated_executor<T, Executor>::get(t.get(), ex)) {
            return associated_executor<T, Executor>::get(t.get(), ex);
        }
    };


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_ASSOCIATED_EXECUTOR_HPP

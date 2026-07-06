//
// experimental/impl/use_coro.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2021-2023 Klemens D. Morgenstern
//                         (klemens dot morgenstern at gmx dot net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXPERIMENTAL_IMPL_USE_CORO_HPP
#define ASIO_EXPERIMENTAL_IMPL_USE_CORO_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/deferred.h>
#include <xio/experimental/coro.h>

#include <xio/detail/push_options.h>

namespace xio {
    ASIO_INLINE_NAMESPACE_BEGIN

#if !defined(GENERATING_DOCUMENTATION)



    template<typename Allocator, typename R>
    struct async_result<experimental::use_coro_t<Allocator>, R()> {
        template<typename Initiation, typename... InitArgs>
        static auto initiate_impl(Initiation initiation,
                                  std::allocator_arg_t, Allocator, InitArgs... args)
            -> experimental::coro<void() noexcept, void,
                xio::associated_executor_t<Initiation>, Allocator> {
            co_await deferred_async_operation < R(), Initiation, InitArgs
            ...
            >
            (
                deferred_init_tag{}, std::move(initiation), std::move(args)
            ...
            )
            ;
        }

        template<typename... InitArgs>
        static auto initiate_impl(xio::detail::initiation_archetype<R()>,
                                  std::allocator_arg_t, Allocator, InitArgs... args)
            -> experimental::coro<void(), void,
                xio::any_io_executor, Allocator>;

        template<typename Initiation, typename... InitArgs>
        static auto initiate(Initiation initiation,
                             experimental::use_coro_t<Allocator> tk, InitArgs &&... args) {
            return initiate_impl(std::move(initiation), std::allocator_arg,
                                 tk.get_allocator(), std::forward<InitArgs>(args)...);
        }
    };

    template<typename Allocator, typename R>
    struct async_result<
                experimental::use_coro_t<Allocator>, R(xio::error_code)> {
        template<typename Initiation, typename... InitArgs>
        static auto initiate_impl(Initiation initiation,
                                  std::allocator_arg_t, Allocator, InitArgs... args)
            -> experimental::coro<void() noexcept, void,
                xio::associated_executor_t<Initiation>, Allocator> {
            co_await deferred_async_operation <
                    R(xio::error_code), Initiation, InitArgs
            ...
            >
            (
                deferred_init_tag{}, std::move(initiation), std::move(args)
            ...
            )
            ;
        }

        template<typename... InitArgs>
        static auto initiate_impl(
            xio::detail::initiation_archetype<R(xio::error_code)>,
            std::allocator_arg_t, Allocator, InitArgs... args)
            -> experimental::coro<void(), void,
                xio::any_io_executor, Allocator>;

        template<typename Initiation, typename... InitArgs>
        static auto initiate(Initiation initiation,
                             experimental::use_coro_t<Allocator> tk, InitArgs &&... args) {
            return initiate_impl(std::move(initiation), std::allocator_arg,
                                 tk.get_allocator(), std::forward<InitArgs>(args)...);
        }
    };

    template<typename Allocator, typename R>
    struct async_result<
                experimental::use_coro_t<Allocator>, R(std::exception_ptr)> {
        template<typename Initiation, typename... InitArgs>
        static auto initiate_impl(Initiation initiation,
                                  std::allocator_arg_t, Allocator, InitArgs... args)
            -> experimental::coro<void(), void,
                xio::associated_executor_t<Initiation>, Allocator> {
            co_await deferred_async_operation <
                    R(std::exception_ptr), Initiation, InitArgs
            ...
            >
            (
                deferred_init_tag{}, std::move(initiation), std::move(args)
            ...
            )
            ;
        }

        template<typename... InitArgs>
        static auto initiate_impl(
            xio::detail::initiation_archetype<R(std::exception_ptr)>,
            std::allocator_arg_t, Allocator, InitArgs... args)
            -> experimental::coro<void(), void,
                xio::any_io_executor, Allocator>;

        template<typename Initiation, typename... InitArgs>
        static auto initiate(Initiation initiation,
                             experimental::use_coro_t<Allocator> tk, InitArgs &&... args) {
            return initiate_impl(std::move(initiation), std::allocator_arg,
                                 tk.get_allocator(), std::forward<InitArgs>(args)...);
        }
    };

    template<typename Allocator, typename R, typename T>
    struct async_result<experimental::use_coro_t<Allocator>, R(T)> {
        template<typename Initiation, typename... InitArgs>
        static auto initiate_impl(Initiation initiation,
                                  std::allocator_arg_t, Allocator, InitArgs... args)
            -> experimental::coro<void() noexcept, T,
                xio::associated_executor_t<Initiation>, Allocator> {
            co_return co_await deferred_async_operation < R(T), Initiation, InitArgs
            ...
            >
            (
                deferred_init_tag{}, std::move(initiation), std::move(args)
            ...
            )
            ;
        }

        template<typename... InitArgs>
        static auto initiate_impl(xio::detail::initiation_archetype<R(T)>,
                                  std::allocator_arg_t, Allocator, InitArgs... args)
            -> experimental::coro<void() noexcept, T,
                xio::any_io_executor, Allocator>;

        template<typename Initiation, typename... InitArgs>
        static auto initiate(Initiation initiation,
                             experimental::use_coro_t<Allocator> tk, InitArgs &&... args) {
            return initiate_impl(std::move(initiation), std::allocator_arg,
                                 tk.get_allocator(), std::forward<InitArgs>(args)...);
        }
    };

    template<typename Allocator, typename R, typename T>
    struct async_result<
                experimental::use_coro_t<Allocator>, R(xio::error_code, T)> {
        template<typename Initiation, typename... InitArgs>
        static auto initiate_impl(Initiation initiation,
                                  std::allocator_arg_t, Allocator, InitArgs... args)
            -> experimental::coro<void(), T,
                xio::associated_executor_t<Initiation>, Allocator> {
            co_return co_await deferred_async_operation <
                      R(xio::error_code, T), Initiation, InitArgs
            ...
            >
            (
                deferred_init_tag{}, std::move(initiation), std::move(args)
            ...
            )
            ;
        }

        template<typename... InitArgs>
        static auto initiate_impl(
            xio::detail::initiation_archetype<
                R(xio::error_code, T)>,
            std::allocator_arg_t, Allocator, InitArgs... args)
            -> experimental::coro<void(), T, xio::any_io_executor, Allocator>;

        template<typename Initiation, typename... InitArgs>
        static auto initiate(Initiation initiation,
                             experimental::use_coro_t<Allocator> tk, InitArgs &&... args) {
            return initiate_impl(std::move(initiation), std::allocator_arg,
                                 tk.get_allocator(), std::forward<InitArgs>(args)...);
        }
    };

    template<typename Allocator, typename R, typename T>
    struct async_result<
                experimental::use_coro_t<Allocator>, R(std::exception_ptr, T)> {
        template<typename Initiation, typename... InitArgs>
        static auto initiate_impl(Initiation initiation,
                                  std::allocator_arg_t, Allocator, InitArgs... args)
            -> experimental::coro<void(), T,
                xio::associated_executor_t<Initiation>, Allocator> {
            co_return co_await deferred_async_operation <
                      R(std::exception_ptr, T), Initiation, InitArgs
            ...
            >
            (
                deferred_init_tag{}, std::move(initiation), std::move(args)
            ...
            )
            ;
        }

        template<typename... InitArgs>
        static auto initiate_impl(
            xio::detail::initiation_archetype<R(std::exception_ptr, T)>,
            std::allocator_arg_t, Allocator, InitArgs... args)
            -> experimental::coro<void(), T, xio::any_io_executor, Allocator>;

        template<typename Initiation, typename... InitArgs>
        static auto initiate(Initiation initiation,
                             experimental::use_coro_t<Allocator> tk, InitArgs &&... args) {
            return initiate_impl(std::move(initiation), std::allocator_arg,
                                 tk.get_allocator(), std::forward<InitArgs>(args)...);
        }
    };

#endif // !defined(GENERATING_DOCUMENTATION)

    ASIO_INLINE_NAMESPACE_END
} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_EXPERIMENTAL_IMPL_USE_CORO_HPP

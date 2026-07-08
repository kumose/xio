//
// experimental/detail/coro_traits.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2021-2023 Klemens D. Morgenstern
//                         (klemens dot morgenstern at gmx dot net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_EXPERIMENTAL_DETAIL_CORO_TRAITS_HPP
#define XIO_EXPERIMENTAL_DETAIL_CORO_TRAITS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <optional>
#include <variant>
#include <xio/any_io_executor.h>

namespace xio {


    namespace experimental {
        namespace detail {
            template<class From, class To>
            concept convertible_to = std::is_convertible_v<From, To>;

            template<typename T>
            concept decays_to_executor = execution::executor<std::decay_t<T> >;

            template<typename T, typename Executor = any_io_executor>
            concept execution_context = requires(T &t)
            {
                { t.get_executor() } -> convertible_to<Executor>;
            };

            template<typename Yield, typename Return>
            struct coro_result {
                using type = std::variant<Yield, Return>;
            };

            template<typename Yield>
            struct coro_result<Yield, void> {
                using type = std::optional<Yield>;
            };

            template<typename Return>
            struct coro_result<void, Return> {
                using type = Return;
            };

            template<typename YieldReturn>
            struct coro_result<YieldReturn, YieldReturn> {
                using type = YieldReturn;
            };

            template<>
            struct coro_result<void, void> {
                using type = void;
            };

            template<typename Yield, typename Return>
            using coro_result_t = typename coro_result<Yield, Return>::type;

            template<typename Result, bool IsNoexcept>
            struct coro_handler;

            template<>
            struct coro_handler<void, false> {
                using type = void(

                std::exception_ptr
                );
            };

            template<>
            struct coro_handler<void, true> {
                using type = void();
            };

            template<typename T>
            struct coro_handler<T, false> {
                using type = void(

                std::exception_ptr
                ,
                T
                );
            };

            template<typename T>
            struct coro_handler<T, true> {
                using type = void(T);
            };

            template<typename Result, bool IsNoexcept>
            using coro_handler_t = typename coro_handler<Result, IsNoexcept>::type;
        } // namespace detail

        template<typename Yield, typename Return, typename Executor>
        struct coro_traits {
            using input_type = void;
            using yield_type = Yield;
            using return_type = Return;
            using result_type = detail::coro_result_t<yield_type, return_type>;
            using signature_type = result_type();
            constexpr static bool is_noexcept = false;
            using error_type = std::conditional_t<is_noexcept, void, std::exception_ptr>;
            using completion_handler = detail::coro_handler_t<result_type, is_noexcept>;
        };

        template<typename T, typename Return, typename Executor>
        struct coro_traits<T(), Return, Executor> {
            using input_type = void;
            using yield_type = T;
            using return_type = Return;
            using result_type = detail::coro_result_t<yield_type, return_type>;
            using signature_type = result_type();
            constexpr static bool is_noexcept = false;
            using error_type = std::conditional_t<is_noexcept, void, std::exception_ptr>;
            using completion_handler = detail::coro_handler_t<result_type, is_noexcept>;
        };

        template<typename T, typename Return, typename Executor>
        struct coro_traits<T() noexcept, Return, Executor> {
            using input_type = void;
            using yield_type = T;
            using return_type = Return;
            using result_type = detail::coro_result_t<yield_type, return_type>;
            using signature_type = result_type();
            constexpr static bool is_noexcept = true;
            using error_type = std::conditional_t<is_noexcept, void, std::exception_ptr>;
            using completion_handler = detail::coro_handler_t<result_type, is_noexcept>;
        };

        template<typename T, typename U, typename Return, typename Executor>
        struct coro_traits<T(U), Return, Executor> {
            using input_type = U;
            using yield_type = T;
            using return_type = Return;
            using result_type = detail::coro_result_t<yield_type, return_type>;
            using signature_type = result_type(input_type);
            constexpr static bool is_noexcept = false;
            using error_type = std::conditional_t<is_noexcept, void, std::exception_ptr>;
            using completion_handler = detail::coro_handler_t<result_type, is_noexcept>;
        };

        template<typename T, typename U, typename Return, typename Executor>
        struct coro_traits<T(U) noexcept, Return, Executor> {
            using input_type = U;
            using yield_type = T;
            using return_type = Return;
            using result_type = detail::coro_result_t<yield_type, return_type>;
            using signature_type = result_type(input_type);
            constexpr static bool is_noexcept = true;
            using error_type = std::conditional_t<is_noexcept, void, std::exception_ptr>;
            using completion_handler = detail::coro_handler_t<result_type, is_noexcept>;
        };

        template<typename Executor>
        struct coro_traits<void() noexcept, void, Executor> {
            using input_type = void;
            using yield_type = void;
            using return_type = void;
            using result_type = detail::coro_result_t<yield_type, return_type>;
            using signature_type = result_type(input_type);
            constexpr static bool is_noexcept = true;
            using error_type = std::conditional_t<is_noexcept, void, std::exception_ptr>;
            using completion_handler = detail::coro_handler_t<result_type, is_noexcept>;
        };


    } // namespace experimental

} // namespace xio

#endif // XIO_EXPERIMENTAL_DETAIL_CORO_TRAITS_HPP

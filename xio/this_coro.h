//
// this_coro.hpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_THIS_CORO_HPP
#define XIO_THIS_CORO_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace this_coro {
        /// Awaitable type that returns the executor of the current coroutine.
        struct executor_t {
            constexpr executor_t() {
            }
        };

        /// Awaitable object that returns the executor of the current coroutine.
inline constexpr executor_t executor;

        /// Awaitable type that returns the cancellation state of the current coroutine.
        struct cancellation_state_t {
            constexpr cancellation_state_t() {
            }
        };

        /// Awaitable object that returns the cancellation state of the current
/// coroutine.
        /**
 * @par Example
 * @code xio::awaitable<void> my_coroutine()
 * {
 *   xio::cancellation_state cs
 *     = co_await xio::this_coro::cancellation_state;
 *
 *   // ...
 *
 *   if (cs.cancelled() != xio::cancellation_type::none)
 *     // ...
 * } @endcode
 */
inline constexpr cancellation_state_t cancellation_state;


        struct reset_cancellation_state_0_t {
            constexpr reset_cancellation_state_0_t() {
            }
        };

        [[nodiscard]] inline constexpr reset_cancellation_state_0_t

        reset_cancellation_state() {
            return reset_cancellation_state_0_t();
        }

        template<typename Filter>
        struct reset_cancellation_state_1_t {
            template<typename F>
            explicit constexpr reset_cancellation_state_1_t(
                F &&filt)
                : filter(static_cast<F &&>(filt)) {
            }

            Filter filter;
        };

        template<typename Filter>
        [[nodiscard]] inline constexpr reset_cancellation_state_1_t<
            std::decay_t<Filter> >

        reset_cancellation_state(Filter &&filter) {
            return reset_cancellation_state_1_t<std::decay_t<Filter> >(
                static_cast<Filter &&>(filter));
        }

        template<typename InFilter, typename OutFilter>
        struct reset_cancellation_state_2_t {
            template<typename F1, typename F2>
            constexpr reset_cancellation_state_2_t(
                F1 &&in_filt, F2 &&out_filt)
                : in_filter(static_cast<F1 &&>(in_filt)),
                  out_filter(static_cast<F2 &&>(out_filt)) {
            }

            InFilter in_filter;
            OutFilter out_filter;
        };

        template<typename InFilter, typename OutFilter>
        [[nodiscard]] inline constexpr
        reset_cancellation_state_2_t<std::decay_t<InFilter>, std::decay_t<OutFilter> >

        reset_cancellation_state(InFilter &&in_filter, OutFilter &&out_filter) {
            return reset_cancellation_state_2_t<std::decay_t<InFilter>, std::decay_t<OutFilter> >(
                static_cast<InFilter &&>(in_filter),
                static_cast<OutFilter &&>(out_filter));
        }

        struct throw_if_cancelled_0_t {
            constexpr throw_if_cancelled_0_t() {
            }
        };

        [[nodiscard]] inline constexpr throw_if_cancelled_0_t

        throw_if_cancelled() {
            return throw_if_cancelled_0_t();
        }

        struct throw_if_cancelled_1_t {
            explicit constexpr throw_if_cancelled_1_t(bool val)
                : value(val) {
            }

            bool value;
        };

        [[nodiscard]] inline constexpr throw_if_cancelled_1_t

        throw_if_cancelled(bool value) {
            return throw_if_cancelled_1_t(value);
        }

    } // namespace this_coro

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_THIS_CORO_HPP

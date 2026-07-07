//
// inline_executor.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_INLINE_EXECUTOR_HPP
#define ASIO_INLINE_EXECUTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/non_const_lvalue.h>
#include <xio/detail/type_traits.h>
#include <xio/execution.h>

#include <xio/detail/push_options.h>

namespace xio {


    /// An executor that always executes the function object inline.




    template<typename InlineExceptionHandling>
    class basic_inline_executor {
    public:
        /// Default constructor.
        basic_inline_executor() noexcept {
        }

#if !defined(GENERATING_DOCUMENTATION)

    private:
        friend struct XIO_VERSIONED_NAME (require_fn)::impl;
        friend struct XIO_VERSIONED_NAME (prefer_fn)::impl;
#endif // !defined(GENERATING_DOCUMENTATION)

        /// Obtain an executor with the @c inline_exception_handling.propagate
  /// property.
        /**
   * Do not call this function directly. It is intended for use with the
   * xio::require customisation point.
   *
   * For example:
   * @code xio::inline_executor ex1;
   * auto ex2 = xio::require(ex1,
   *     xio::execution::inline_exception_handling.propagate); @endcode
   */
        basic_inline_executor<execution::inline_exception_handling_t::propagate_t>
        require(execution::inline_exception_handling_t::propagate_t) const noexcept {
            return basic_inline_executor<
                execution::inline_exception_handling_t::propagate_t>();
        }

        /// Obtain an executor with the @c inline_exception_handling.terminate
  /// property.
        /**
   * Do not call this function directly. It is intended for use with the
   * xio::require customisation point.
   *
   * For example:
   * @code xio::inline_executor ex1;
   * auto ex2 = xio::require(ex1,
   *     xio::execution::inline_exception_handling.terminate); @endcode
   */
        basic_inline_executor<execution::inline_exception_handling_t::terminate_t>
        require(execution::inline_exception_handling_t::terminate_t) const noexcept {
            return basic_inline_executor<
                execution::inline_exception_handling_t::terminate_t>();
        }

#if !defined(GENERATING_DOCUMENTATION)

    private:
        friend struct XIO_VERSIONED_NAME (query_fn)::impl;
        friend struct xio::execution::detail::blocking_t<0>;
        friend struct xio::execution::detail::mapping_t<0>;
        friend struct xio::execution::detail::inline_exception_handling_t<0>;
#endif // !defined(GENERATING_DOCUMENTATION)

        /// Query the current value of the @c mapping property.
        /**
   * Do not call this function directly. It is intended for use with the
   * xio::query customisation point.
   *
   * For example:
   * @code xio::inline_executor ex;
   * if (xio::query(ex, xio::execution::mapping)
   *       == xio::execution::mapping.thread)
   *   ... @endcode
   */
        static constexpr execution::mapping_t query(
            execution::mapping_t) noexcept {
            return execution::mapping.thread;
        }

        /// Query the current value of the @c inline_exception_handling property.
        /**
   * Do not call this function directly. It is intended for use with the
   * xio::query customisation point.
   *
   * For example:
   * @code xio::inline_executor ex;
   * if (xio::query(ex,
   *       xio::execution::inline_exception_handling)
   *     == xio::execution::inline_exception_handling.propagate)
   *   ... @endcode
   */
        static constexpr execution::inline_exception_handling_t query(
            execution::inline_exception_handling_t) noexcept {
            return InlineExceptionHandling();
        }

        /// Query the current value of the @c blocking property.
        /**
   * Do not call this function directly. It is intended for use with the
   * xio::query customisation point.
   *
   * For example:
   * @code xio::inline_executor ex;
   * if (xio::query(ex, xio::execution::blocking)
   *     == xio::execution::blocking.always)
   *   ... @endcode
   */
        static constexpr execution::blocking_t query(
            execution::blocking_t) noexcept {
            return execution::blocking.always;
        }

    public:
        /// Compare two executors for equality.
        /**
   * Two inline executors are always considered equal.
   */
        friend bool operator==(const basic_inline_executor &,
                               const basic_inline_executor &) noexcept {
            return true;
        }

        /// Compare two executors for inequality.
        /**
   * Two inline executors are never considered unequal.
   */
        friend bool operator!=(const basic_inline_executor &,
                               const basic_inline_executor &) noexcept {
            return false;
        }

        /// Execution function.
        template<typename Function>
        void execute(Function &&f) const {
#if !defined(ASIO_NO_EXCEPTIONS)
            try
#endif // !defined(ASIO_NO_EXCEPTIONS)
            {
                detail::non_const_lvalue<Function> f2(f);
                static_cast<decay_t<Function> &&>(f2.value)();
            }
#if !defined(ASIO_NO_EXCEPTIONS)
            catch (...) {
                if (is_same<InlineExceptionHandling,
                    execution::inline_exception_handling_t::terminate_t>::value) {
                    std::terminate();
                } else {
                    throw;
                }
            }
#endif // !defined(ASIO_NO_EXCEPTIONS)
        }
    };

    /// An executor that always executes the function object inline.
    typedef basic_inline_executor<
        execution::inline_exception_handling_t::propagate_t>
    inline_executor;


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_INLINE_EXECUTOR_HPP

//
// detail/null_thread.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_NULL_THREAD_HPP
#define ASIO_DETAIL_NULL_THREAD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(ASIO_HAS_THREADS)

#include <xio/detail/throw_error.h>
#include <xio/error.h>

#include <xio/detail/push_options.h>

namespace xio {
    ASIO_INLINE_NAMESPACE_BEGIN

    namespace detail {
        class null_thread {
        public:
            // Construct in a non-joinable state.
            null_thread() noexcept {
            }

            // Constructor.
            template<typename Function>
            null_thread(Function, unsigned int = 0) {
                xio::detail::throw_error(
                    xio::error::operation_not_supported, "thread");
            }

            // Construct with custom allocator.
            template<typename Allocator, typename Function>
            null_thread(allocator_arg_t, const Allocator &, Function, unsigned int = 0) {
                xio::detail::throw_error(
                    xio::error::operation_not_supported, "thread");
            }

            // Move constructor.
            null_thread(null_thread &&) noexcept {
            }

            // Destructor.
            ~null_thread() {
            }

            // Move assignment.
            null_thread &operator=(null_thread &&) noexcept {
                return *this;
            }

            // Whether the thread can be joined.
            bool joinable() const {
                return false;
            }

            // Wait for the thread to exit.
            void join() {
            }

            // Get number of CPUs.
            static std::size_t hardware_concurrency() {
                return 1;
            }
        };
    } // namespace detail
    ASIO_INLINE_NAMESPACE_END
} // namespace xio

#include <xio/detail/pop_options.h>

#endif // !defined(ASIO_HAS_THREADS)

#endif // ASIO_DETAIL_NULL_THREAD_HPP

//
// detail/impl/win_iocp_io_context.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_IMPL_WIN_IOCP_IO_CONTEXT_HPP
#define XIO_DETAIL_IMPL_WIN_IOCP_IO_CONTEXT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_IOCP)

#include <xio/detail/completion_handler.h>
#include <xio/detail/fenced_block.h>
#include <xio/detail/handler_alloc_helpers.h>
#include <xio/detail/memory.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        template<typename TimeTraits, typename Allocator>
        void win_iocp_io_context::add_timer_queue(
            timer_queue<TimeTraits, Allocator> &queue) {
            do_add_timer_queue(queue);
        }

        template<typename TimeTraits, typename Allocator>
        void win_iocp_io_context::remove_timer_queue(
            timer_queue<TimeTraits, Allocator> &queue) {
            do_remove_timer_queue(queue);
        }

        template<typename TimeTraits, typename Allocator>
        void win_iocp_io_context::schedule_timer(
            timer_queue<TimeTraits, Allocator> &queue,
            const typename TimeTraits::time_type &time,
            typename timer_queue<TimeTraits, Allocator>::per_timer_data &timer,
            wait_op *op) {
            // If the service has been shut down we silently discard the timer.
            if (::InterlockedExchangeAdd(&shutdown_, 0) != 0) {
                post_immediate_completion(op, false);
                return;
            }

            mutex::scoped_lock lock(dispatch_mutex_);

            bool earliest = queue.enqueue_timer(time, timer, op);
            work_started();
            if (earliest)
                update_timeout();
        }

        template<typename TimeTraits, typename Allocator>
        std::size_t win_iocp_io_context::cancel_timer(
            timer_queue<TimeTraits, Allocator> &queue,
            typename timer_queue<TimeTraits, Allocator>::per_timer_data &timer,
            std::size_t max_cancelled) {
            // If the service has been shut down we silently ignore the cancellation.
            if (::InterlockedExchangeAdd(&shutdown_, 0) != 0)
                return 0;

            mutex::scoped_lock lock(dispatch_mutex_);
            op_queue<win_iocp_operation> ops;
            std::size_t n = queue.cancel_timer(timer, ops, max_cancelled);
            lock.unlock();
            post_deferred_completions(ops);
            return n;
        }

        template<typename TimeTraits, typename Allocator>
        void win_iocp_io_context::cancel_timer_by_key(
            timer_queue<TimeTraits, Allocator> &queue,
            typename timer_queue<TimeTraits, Allocator>::per_timer_data *timer,
            void *cancellation_key) {
            // If the service has been shut down we silently ignore the cancellation.
            if (::InterlockedExchangeAdd(&shutdown_, 0) != 0)
                return;

            mutex::scoped_lock lock(dispatch_mutex_);
            op_queue<win_iocp_operation> ops;
            queue.cancel_timer_by_key(timer, ops, cancellation_key);
            lock.unlock();
            post_deferred_completions(ops);
        }

        template<typename TimeTraits, typename Allocator>
        void win_iocp_io_context::move_timer(timer_queue<TimeTraits, Allocator> &queue,
                                             typename timer_queue<TimeTraits, Allocator>::per_timer_data &to,
                                             typename timer_queue<TimeTraits, Allocator>::per_timer_data &from) {
            xio::detail::mutex::scoped_lock lock(dispatch_mutex_);
            op_queue<operation> ops;
            queue.cancel_timer(to, ops);
            queue.move_timer(to, from);
            lock.unlock();
            post_deferred_completions(ops);
        }
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(XIO_HAS_IOCP)

#endif // XIO_DETAIL_IMPL_WIN_IOCP_IO_CONTEXT_HPP

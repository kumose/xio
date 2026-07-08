//
// detail/winrt_timer_scheduler.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_WINRT_TIMER_SCHEDULER_HPP
#define XIO_DETAIL_WINRT_TIMER_SCHEDULER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_WINDOWS_RUNTIME)

#include <cstddef>
#include <xio/detail/event.h>
#include <limits>
#include <xio/detail/mutex.h>
#include <xio/detail/op_queue.h>
#include <xio/detail/thread.h>
#include <xio/detail/timer_queue_base.h>
#include <xio/detail/timer_queue_set.h>
#include <xio/detail/wait_op.h>
#include <xio/execution_context.h>

#if defined(XIO_HAS_IOCP)
#include <xio/detail/win_iocp_io_context.h>
#else // defined(XIO_HAS_IOCP)
#include <xio/detail/scheduler.h>
#endif // defined(XIO_HAS_IOCP)

#if defined(XIO_HAS_IOCP)
#include <xio/detail/thread.h>
#endif // defined(XIO_HAS_IOCP)

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        class winrt_timer_scheduler
                : public execution_context_service_base<winrt_timer_scheduler> {
        public:
            // Constructor.
            XIO_DECL winrt_timer_scheduler(execution_context &context);

            // Destructor.
            XIO_DECL ~winrt_timer_scheduler();

            // Destroy all user-defined handler objects owned by the service.
            XIO_DECL void shutdown();

            // Recreate internal descriptors following a fork.
            XIO_DECL void notify_fork(execution_context::fork_event fork_ev);

            // Initialise the task. No effect as this class uses its own thread.
            XIO_DECL void init_task();

            // Add a new timer queue to the reactor.
            template<typename TimeTraits, typename Allocator>
            void add_timer_queue(timer_queue<TimeTraits, Allocator> &queue);

            // Remove a timer queue from the reactor.
            template<typename TimeTraits, typename Allocator>
            void remove_timer_queue(timer_queue<TimeTraits, Allocator> &queue);

            // Schedule a new operation in the given timer queue to expire at the
            // specified absolute time.
            template<typename TimeTraits, typename Allocator>
            void schedule_timer(timer_queue<TimeTraits, Allocator> &queue,
                                const typename TimeTraits::time_type &time,
                                typename timer_queue<TimeTraits, Allocator>::per_timer_data &timer,
                                wait_op *op);

            // Cancel the timer operations associated with the given token. Returns the
            // number of operations that have been posted or dispatched.
            template<typename TimeTraits, typename Allocator>
            std::size_t cancel_timer(timer_queue<TimeTraits, Allocator> &queue,
                                     typename timer_queue<TimeTraits, Allocator>::per_timer_data &timer,
                                     std::size_t max_cancelled = (std::numeric_limits<std::size_t>::max)());

            // Move the timer operations associated with the given timer.
            template<typename TimeTraits, typename Allocator>
            void move_timer(timer_queue<TimeTraits, Allocator> &queue,
                            typename timer_queue<TimeTraits, Allocator>::per_timer_data &to,
                            typename timer_queue<TimeTraits, Allocator>::per_timer_data &from);

        private:
            // Run the select loop in the thread.
            XIO_DECL void run_thread();

            // Entry point for the select loop thread.
            XIO_DECL static void call_run_thread(winrt_timer_scheduler *reactor);

            // Helper function to add a new timer queue.
            XIO_DECL void do_add_timer_queue(timer_queue_base &queue);

            // Helper function to remove a timer queue.
            XIO_DECL void do_remove_timer_queue(timer_queue_base &queue);

// The scheduler implementation used to post completions.
#if defined(XIO_HAS_IOCP)
typedef class win_iocp_io_context scheduler_impl;
#else
typedef class scheduler scheduler_impl;
#endif
scheduler_impl &scheduler_;

// Mutex used to protect internal variables.
xio::detail::mutex mutex_;

// Event used to wake up background thread.
xio::detail::event event_;

// The timer queues.
timer_queue_set timer_queues_;

// The background thread that is waiting for timers to expire.
xio::detail::thread thread_;

// Does the background thread need to stop.
bool stop_thread_;

// Whether the service has been shut down.
bool shutdown_;
};

} // namespace detail
} // namespace xio

#include <xio/detail/pop_options.h>

#include <xio/detail/impl/winrt_timer_scheduler.h>


#endif // defined(XIO_WINDOWS_RUNTIME)

#endif // XIO_DETAIL_WINRT_TIMER_SCHEDULER_HPP

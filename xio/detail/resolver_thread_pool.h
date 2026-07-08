//
// detail/resolver_thread_pool.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_RESOLVER_THREAD_POOL_HPP
#define XIO_DETAIL_RESOLVER_THREAD_POOL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/execution_context.h>
#include <xio/detail/mutex.h>
#include <xio/detail/resolve_op.h>
#include <xio/detail/scheduler.h>
#include <xio/detail/thread_group.h>

#if defined(XIO_HAS_IOCP)
#include <xio/detail/win_iocp_io_context.h>
#else // defined(XIO_HAS_IOCP)
#include <xio/detail/scheduler.h>
#endif // defined(XIO_HAS_IOCP)

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        class resolver_thread_pool :
                public execution_context_service_base<resolver_thread_pool> {
        public:
#if defined(XIO_HAS_IOCP)
            typedef class win_iocp_io_context scheduler_impl;
#else
            typedef class scheduler scheduler_impl;
#endif

            // Constructor.
            XIO_DECL resolver_thread_pool(execution_context &context);

            // Destructor.
            XIO_DECL ~resolver_thread_pool();

            // Destroy all user-defined handler objects owned by the service.
            XIO_DECL void shutdown();

            // Perform any fork-related housekeeping.
            XIO_DECL void notify_fork(execution_context::fork_event fork_ev);

            // Helper function to start an asynchronous resolve operation.
            XIO_DECL void start_resolve_op(resolve_op *op);

            // Get the underlying scheduler implementation.
            scheduler_impl &scheduler() {
                return scheduler_;
            }

        private:
            // Helper class to run the work scheduler in a thread.
            class work_scheduler_runner;

            // Start the work scheduler if it's not already running.
            XIO_DECL void start_work_threads();

            // The scheduler implementation used to post completions.
            scheduler_impl &scheduler_;

            // Mutex to protect access to internal data.
            xio::detail::mutex mutex_;

            // Private scheduler used for performing asynchronous host resolution.
            scheduler_impl work_scheduler_;

            // Threads used for running the work scheduler's run loop.
            thread_group<execution_context::allocator<void> > work_threads_;

            // The number of threads used to run the work scheduler.
            unsigned int num_work_threads_;

            // Whether the scheduler locking is enabled.
            bool scheduler_locking_;

            // Whether the scheduler has been shut down.
            bool shutdown_;
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>


#endif // XIO_DETAIL_RESOLVER_THREAD_POOL_HPP

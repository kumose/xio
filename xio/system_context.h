//
// system_context.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_SYSTEM_CONTEXT_HPP
#define ASIO_SYSTEM_CONTEXT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/scheduler.h>
#include <xio/detail/thread_group.h>
#include <xio/execution.h>
#include <xio/execution_context.h>

#include <xio/detail/push_options.h>

namespace xio {

    template<typename Blocking, typename Relationship, typename Allocator>
    class basic_system_executor;

    /// The executor context for the system executor.
    class system_context : public execution_context {
    public:
        /// The executor type associated with the context.
        typedef basic_system_executor<
            execution::blocking_t::possibly_t,
            execution::relationship_t::fork_t,
            std::allocator<void>
        > executor_type;

        /// Destructor shuts down all threads in the system thread pool.
        ASIO_DECL ~system_context();

        /// Obtain an executor for the context.
        executor_type get_executor() noexcept;

        /// Signal all threads in the system thread pool to stop.
  ASIO_DECL void stop();

        /// Determine whether the system thread pool has been stopped.
  ASIO_DECL bool stopped() const noexcept;

        /// Join all threads in the system thread pool.
  ASIO_DECL void join();

#if defined(GENERATING_DOCUMENTATION)
    private:
#endif // defined(GENERATING_DOCUMENTATION)
        // Constructor creates all threads in the system thread pool.
        ASIO_DECL system_context();

    private:
        template<typename, typename, typename>
        friend class basic_system_executor;

        struct thread_function;

        // Helper function to create the underlying scheduler.
        ASIO_DECL detail::scheduler &add_scheduler(detail::scheduler *s);

        // The underlying scheduler.
        detail::scheduler &scheduler_;

        // The threads in the system thread pool.
        detail::thread_group<std::allocator<void> > threads_;

        // The number of threads in the pool.
        std::size_t num_threads_;
    };


} // namespace xio

#include <xio/detail/pop_options.h>

#include <xio/impl/system_context.h>


#endif // ASIO_SYSTEM_CONTEXT_HPP

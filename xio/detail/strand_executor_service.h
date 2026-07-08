//
// detail/strand_executor_service.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_STRAND_EXECUTOR_SERVICE_HPP
#define XIO_DETAIL_STRAND_EXECUTOR_SERVICE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/atomic_count.h>
#include <xio/detail/executor_op.h>
#include <xio/detail/memory.h>
#include <xio/detail/mutex.h>
#include <xio/detail/op_queue.h>
#include <xio/detail/scheduler_operation.h>
#include <xio/detail/slim_mutex.h>
#include <xio/detail/type_traits.h>
#include <xio/execution.h>
#include <xio/execution_context.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        // Default service implementation for a strand.
        class strand_executor_service
                : public execution_context_service_base<strand_executor_service> {
        public:
            // The underlying implementation of a strand.
            class strand_impl {
            public:
                XIO_DECL ~strand_impl();

            private:
                friend class strand_executor_service;

                // Mutex to protect access to internal data.
#if defined(XIO_HAS_STD_ATOMIC_WAIT) \
  || defined(XIO_HAS_FUTEX)
                slim_mutex mutex_;

                void lock_mutex() {
                    mutex_.lock();
                }

                void unlock_mutex() {
                    mutex_.unlock();
                }
#else // defined(XIO_HAS_STD_ATOMIC_WAIT)
                //   || defined(XIO_HAS_FUTEX)
                mutex *mutex_;

                void lock_mutex() {
                    mutex_->lock();
                }

                void unlock_mutex() {
                    mutex_->unlock();
                }
#endif // defined(XIO_HAS_STD_ATOMIC_WAIT)
                //   || defined(XIO_HAS_FUTEX)

                // Indicates whether the strand is currently "locked" by a handler. This
                // means that there is a handler upcall in progress, or that the strand
                // itself has been scheduled in order to invoke some pending handlers.
                bool locked_;

                // Indicates that the strand has been shut down and will accept no further
                // handlers.
                bool shutdown_;

                // The handlers that are waiting on the strand but should not be run until
                // after the next time the strand is scheduled. This queue must only be
                // modified while the mutex is locked.
                op_queue<scheduler_operation> waiting_queue_;

                // The handlers that are ready to be run. Logically speaking, these are the
                // handlers that hold the strand's lock. The ready queue is only modified
                // from within the strand and so may be accessed without locking the mutex.
                op_queue<scheduler_operation> ready_queue_;

                // Pointers to adjacent handle implementations in linked list.
                strand_impl *next_;
                strand_impl *prev_;

                // The strand service in where the implementation is held.
                strand_executor_service *service_;
            };

            typedef shared_ptr<strand_impl> implementation_type;

            // Construct a new strand service for the specified context.
            XIO_DECL explicit strand_executor_service(execution_context &context);

            // Destroy all user-defined handler objects owned by the service.
  XIO_DECL void shutdown();

            // Create a new strand_executor implementation.
  XIO_DECL implementation_type create_implementation();

            // Request invocation of the given function.
            template<typename Executor, typename Function>
            static void execute(const implementation_type &impl, Executor &ex,
                                Function &&function,
                                std::enable_if_t<
                                    can_query<Executor, execution::allocator_t<void> >::value
                                > * = 0);

            // Request invocation of the given function.
            template<typename Executor, typename Function>
            static void execute(const implementation_type &impl, Executor &ex,
                                Function &&function,
                                std::enable_if_t<
                                    !can_query<Executor, execution::allocator_t<void> >::value
                                > * = 0);

            // Request invocation of the given function.
            template<typename Executor, typename Function, typename Allocator>
            static void dispatch(const implementation_type &impl, Executor &ex,
                                 Function &&function, const Allocator &a);

            // Request invocation of the given function and return immediately.
            template<typename Executor, typename Function, typename Allocator>
            static void post(const implementation_type &impl, Executor &ex,
                             Function &&function, const Allocator &a);

            // Request invocation of the given function and return immediately.
            template<typename Executor, typename Function, typename Allocator>
            static void defer(const implementation_type &impl, Executor &ex,
                              Function &&function, const Allocator &a);

            // Determine whether the strand is running in the current thread.
  XIO_DECL static bool running_in_this_thread(
                const implementation_type &impl);

        private:
            friend class strand_impl;
            template<typename F, typename Allocator>
            class allocator_binder;
            template<typename Executor, typename = void>
            class invoker;

            // Adds a function to the strand. Returns true if it acquires the lock.
  XIO_DECL static bool enqueue(const implementation_type &impl,
                                scheduler_operation *op);

            // Transfers waiting handlers to the ready queue. Returns true if one or more
            // handlers were transferred.
  XIO_DECL static bool push_waiting_to_ready(implementation_type &impl);

            // Invokes all ready-to-run handlers.
  XIO_DECL void run_ready_handlers(implementation_type &impl);

            // Helper function to request invocation of the given function.
            template<typename Executor, typename Function, typename Allocator>
            static void do_execute(const implementation_type &impl, Executor &ex,
                                   Function &&function, const Allocator &a);

            // Mutex to protect access to the service-wide state.
            mutex mutex_;

#if !defined(XIO_HAS_STD_ATOMIC_WAIT) \
  && !defined(XIO_HAS_FUTEX)
            // Number of mutexes shared between all strand objects.
            enum { num_mutexes = 193 };

            // Pool of mutexes.
            shared_ptr<mutex> mutexes_[num_mutexes];

            // Extra value used when hashing to prevent recycled memory locations from
            // getting the same mutex.
            std::size_t salt_;
#endif // !defined(XIO_HAS_STD_ATOMIC_WAIT)
            //   && !defined(XIO_HAS_FUTEX)

            // The head of a linked list of all implementations.
            strand_impl *impl_list_;

            // Cached success value to avoid accessing category singleton.
            const xio::error_code success_ec_;
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#include <xio/detail/impl/strand_executor_service.h>


#endif // XIO_DETAIL_STRAND_EXECUTOR_SERVICE_HPP

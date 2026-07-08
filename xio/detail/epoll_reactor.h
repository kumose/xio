//
// detail/epoll_reactor.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_EPOLL_REACTOR_HPP
#define XIO_DETAIL_EPOLL_REACTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_EPOLL)

#include <xio/detail/atomic_count.h>
#include <xio/detail/conditionally_enabled_mutex.h>
#include <limits>
#include <xio/detail/object_pool.h>
#include <xio/detail/op_queue.h>
#include <xio/detail/reactor_op.h>
#include <xio/detail/scheduler_task.h>
#include <xio/detail/select_interrupter.h>
#include <xio/detail/slim_mutex.h>
#include <xio/detail/socket_types.h>
#include <xio/detail/timer_queue_base.h>
#include <xio/detail/timer_queue_set.h>
#include <xio/detail/wait_op.h>
#include <xio/execution_context.h>

#if defined(XIO_HAS_TIMERFD)
# include <sys/timerfd.h>
#endif // defined(XIO_HAS_TIMERFD)

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        class epoll_reactor
                : public execution_context_service_base<epoll_reactor>,
                  public scheduler_task {
        private:
            // The mutex type used by this reactor.
            typedef conditionally_enabled_mutex<slim_mutex> mutex;

        public:
            enum op_types {
                read_op = 0, write_op = 1,
                connect_op = 1, except_op = 2, max_ops = 3
            };

            // Per-descriptor queues.
            struct descriptor_state : operation {
                descriptor_state *next_;
                descriptor_state *prev_;

                mutex mutex_;
                epoll_reactor *reactor_;
                int descriptor_;
                uint32_t registered_events_;
                op_queue<reactor_op> op_queue_[max_ops];
                bool try_speculative_[max_ops];
                bool shutdown_;

                XIO_DECL descriptor_state(bool locking, int spin_count);

                void set_ready_events(uint32_t events) { task_result_ = events; }
                void add_ready_events(uint32_t events) { task_result_ |= events; }

    XIO_DECL operation *perform_io(uint32_t events);

    XIO_DECL static void do_complete(
                    void *owner, operation *base,
                    const xio::error_code &ec, std::size_t bytes_transferred);
            };

            // Per-descriptor data.
            typedef descriptor_state *per_descriptor_data;

            // Constructor.
            XIO_DECL epoll_reactor(xio::execution_context &ctx);

            // Destructor.
            XIO_DECL ~epoll_reactor();

            // Destroy all user-defined handler objects owned by the service.
  XIO_DECL void shutdown();

            // Recreate internal descriptors following a fork.
  XIO_DECL void notify_fork(
                xio::execution_context::fork_event fork_ev);

            // Initialise the task.
  XIO_DECL void init_task();

            // Register a socket with the reactor. Returns 0 on success, system error
            // code on failure.
  XIO_DECL int register_descriptor(socket_type descriptor,
                                    per_descriptor_data &descriptor_data);

            // Register a descriptor with an associated single operation. Returns 0 on
            // success, system error code on failure.
  XIO_DECL int register_internal_descriptor(
                int op_type, socket_type descriptor,
                per_descriptor_data &descriptor_data, reactor_op *op);

            // Move descriptor registration from one descriptor_data object to another.
  XIO_DECL void move_descriptor(socket_type descriptor,
                                 per_descriptor_data &target_descriptor_data,
                                 per_descriptor_data &source_descriptor_data);

            // Post a reactor operation for immediate completion.
            void post_immediate_completion(operation *op, bool is_continuation) const;

            // Post a reactor operation for immediate completion.
  XIO_DECL static void call_post_immediate_completion(
                operation *op, bool is_continuation, const void *self);

            // Start a new operation. The reactor operation will be performed when the
            // given descriptor is flagged as ready, or an error has occurred.
  XIO_DECL void start_op(int op_type, socket_type descriptor,
                          per_descriptor_data &descriptor_data, reactor_op *op,
                          bool is_continuation, bool allow_speculative,
                          void (*on_immediate)(operation *, bool, const void *),
                          const void *immediate_arg);

            // Start a new operation. The reactor operation will be performed when the
            // given descriptor is flagged as ready, or an error has occurred.
            void start_op(int op_type, socket_type descriptor,
                          per_descriptor_data &descriptor_data, reactor_op *op,
                          bool is_continuation, bool allow_speculative) {
                start_op(op_type, descriptor, descriptor_data,
                         op, is_continuation, allow_speculative,
                         &epoll_reactor::call_post_immediate_completion, this);
            }

            // Cancel all operations associated with the given descriptor. The
            // handlers associated with the descriptor will be invoked with the
            // operation_aborted error.
  XIO_DECL void cancel_ops(socket_type descriptor,
                            per_descriptor_data &descriptor_data);

            // Cancel all operations associated with the given descriptor and key. The
            // handlers associated with the descriptor will be invoked with the
            // operation_aborted error.
  XIO_DECL void cancel_ops_by_key(socket_type descriptor,
                                   per_descriptor_data &descriptor_data,
                                   int op_type, void *cancellation_key);

            // Cancel any operations that are running against the descriptor and remove
            // its registration from the reactor. The reactor resources associated with
            // the descriptor must be released by calling cleanup_descriptor_data.
  XIO_DECL void deregister_descriptor(socket_type descriptor,
                                       per_descriptor_data &descriptor_data, bool closing);

            // Remove the descriptor's registration from the reactor. The reactor
            // resources associated with the descriptor must be released by calling
            // cleanup_descriptor_data.
  XIO_DECL void deregister_internal_descriptor(
                socket_type descriptor, per_descriptor_data &descriptor_data);

            // Perform any post-deregistration cleanup tasks associated with the
            // descriptor data.
  XIO_DECL void cleanup_descriptor_data(
                per_descriptor_data &descriptor_data);

            // Add a new timer queue to the reactor.
            template<typename TimeTraits, typename Allocator>
            void add_timer_queue(timer_queue<TimeTraits, Allocator> &timer_queue);

            // Remove a timer queue from the reactor.
            template<typename TimeTraits, typename Allocator>
            void remove_timer_queue(timer_queue<TimeTraits, Allocator> &timer_queue);

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

            // Cancel the timer operations associated with the given key.
            template<typename TimeTraits, typename Allocator>
            void cancel_timer_by_key(timer_queue<TimeTraits, Allocator> &queue,
                                     typename timer_queue<TimeTraits, Allocator>::per_timer_data *timer,
                                     void *cancellation_key);

            // Move the timer operations associated with the given timer.
            template<typename TimeTraits, typename Allocator>
            void move_timer(timer_queue<TimeTraits, Allocator> &queue,
                            typename timer_queue<TimeTraits, Allocator>::per_timer_data &target,
                            typename timer_queue<TimeTraits, Allocator>::per_timer_data &source);

            // Run epoll once until interrupted or events are ready to be dispatched.
  XIO_DECL void run(long usec, op_queue<operation> &ops);

            // Interrupt the select loop.
  XIO_DECL void interrupt();

        private:
            // The hint to pass to epoll_create to size its data structures.
            enum { epoll_size = 20000 };

            // Create the epoll file descriptor. Throws an exception if the descriptor
            // cannot be created.
  XIO_DECL static int do_epoll_create();

            // Create the timerfd file descriptor. Does not throw.
  XIO_DECL static int do_timerfd_create();

            // Allocate a new descriptor state object.
  XIO_DECL descriptor_state *allocate_descriptor_state();

            // Free an existing descriptor state object.
  XIO_DECL void free_descriptor_state(descriptor_state *s);

            // Helper function to add a new timer queue.
  XIO_DECL void do_add_timer_queue(timer_queue_base &queue);

            // Helper function to remove a timer queue.
  XIO_DECL void do_remove_timer_queue(timer_queue_base &queue);

            // Called to recalculate and update the timeout.
  XIO_DECL void update_timeout();

            // Get the timeout value for the epoll_wait call. The timeout value is
            // returned as a number of milliseconds. A return value of -1 indicates
            // that epoll_wait should block indefinitely.
  XIO_DECL int get_timeout(int msec);

#if defined(XIO_HAS_TIMERFD)
// Get the timeout value for the timer descriptor. The return value is the
// flag argument to be used when calling timerfd_settime.
  XIO_DECL int get_timeout(itimerspec & ts);
#endif // defined(XIO_HAS_TIMERFD)

// The scheduler implementation used to post completions.
scheduler &scheduler_;

// Mutex to protect access to internal data.
mutex mutex_;

// The interrupter is used to break a blocking epoll_wait call.
select_interrupter interrupter_;

// The epoll file descriptor.
int epoll_fd_;

// The timer file descriptor.
int timer_fd_;

// The timer queues.
timer_queue_set timer_queues_;

// Whether the service has been shut down.
bool shutdown_;

// Whether I/O locking is enabled.
const bool io_locking_;

// How any times to spin waiting for the I/O mutex.
const int io_locking_spin_count_;

// Mutex to protect access to the registered descriptors.
mutex registered_descriptors_mutex_;

// Keep track of all registered descriptors.
object_pool<descriptor_state, execution_context::allocator<void> >
registered_descriptors_;

// Helper class to do post-perform_io cleanup.
struct perform_io_cleanup_on_block_exit;
friend struct perform_io_cleanup_on_block_exit;
};

} // namespace detail
} // namespace xio

#include <xio/detail/pop_options.h>

#include <xio/detail/impl/epoll_reactor.h>


#endif // defined(XIO_HAS_EPOLL)

#endif // XIO_DETAIL_EPOLL_REACTOR_HPP

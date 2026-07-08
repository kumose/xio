//
// detail/handler_tracking.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_HANDLER_TRACKING_HPP
#define XIO_DETAIL_HANDLER_TRACKING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

namespace xio {


    class execution_context;


} // namespace xio

#if defined(XIO_CUSTOM_HANDLER_TRACKING)
# include XIO_CUSTOM_HANDLER_TRACKING
#elif defined(XIO_ENABLE_HANDLER_TRACKING)
#include <xio/error_code.h>
#include <cstdint>
#include <xio/detail/static_mutex.h>
#include <xio/detail/tss_ptr.h>
#endif // defined(XIO_ENABLE_HANDLER_TRACKING)

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
#if defined(XIO_CUSTOM_HANDLER_TRACKING)

        // The user-specified header must define the following macros:
        // - XIO_INHERIT_TRACKED_HANDLER
        // - XIO_ALSO_INHERIT_TRACKED_HANDLER
        // - XIO_HANDLER_TRACKING_INIT
        // - XIO_HANDLER_CREATION(args)
        // - XIO_HANDLER_COMPLETION(args)
        // - XIO_HANDLER_INVOCATION_BEGIN(args)
        // - XIO_HANDLER_INVOCATION_END
        // - XIO_HANDLER_OPERATION(args)
        // - XIO_HANDLER_REACTOR_REGISTRATION(args)
        // - XIO_HANDLER_REACTOR_DEREGISTRATION(args)
        // - XIO_HANDLER_REACTOR_READ_EVENT
        // - XIO_HANDLER_REACTOR_WRITE_EVENT
        // - XIO_HANDLER_REACTOR_ERROR_EVENT
        // - XIO_HANDLER_REACTOR_EVENTS(args)
        // - XIO_HANDLER_REACTOR_OPERATION(args)

# if !defined(XIO_ENABLE_HANDLER_TRACKING)
#  define XIO_ENABLE_HANDLER_TRACKING 1
# endif /// !defined(XIO_ENABLE_HANDLER_TRACKING)

#elif defined(XIO_ENABLE_HANDLER_TRACKING)

        class handler_tracking {
        public:
            class completion;

            // Base class for objects containing tracked handlers.
            class tracked_handler {
            private:
                // Only the handler_tracking class will have access to the id.
                friend class handler_tracking;
                friend class completion;
                uint64_t id_;

            protected:
                // Constructor initialises with no id.
                tracked_handler() : id_(0) {
                }

                // Prevent deletion through this type.
                ~tracked_handler() {
                }
            };

            // Initialise the tracking system.
  XIO_DECL static void init();

            class location {
            public:
                // Constructor adds a location to the stack.
                XIO_DECL explicit location(const char *file,
                                            int line, const char *func);

                // Destructor removes a location from the stack.
                XIO_DECL ~location();

            private:
                // Disallow copying and assignment.
                location(const location &) = delete;

                location &operator=(const location &) = delete;

                friend class handler_tracking;
                const char *file_;
                int line_;
                const char *func_;
                location *next_;
            };

            // Record the creation of a tracked handler.
  XIO_DECL static void creation(
                execution_context &context, tracked_handler &h,
                const char *object_type, void *object,
                uintmax_t native_handle, const char *op_name);

            class completion {
            public:
                // Constructor records that handler is to be invoked with no arguments.
                XIO_DECL explicit completion(const tracked_handler &h);

                // Destructor records only when an exception is thrown from the handler, or
                // if the memory is being freed without the handler having been invoked.
                XIO_DECL ~completion();

                // Records that handler is to be invoked with no arguments.
    XIO_DECL void invocation_begin();

                // Records that handler is to be invoked with one arguments.
    XIO_DECL void invocation_begin(const xio::error_code &ec);

                // Constructor records that handler is to be invoked with two arguments.
    XIO_DECL void invocation_begin(
                    const xio::error_code &ec, std::size_t bytes_transferred);

                // Constructor records that handler is to be invoked with two arguments.
    XIO_DECL void invocation_begin(
                    const xio::error_code &ec, int signal_number);

                // Constructor records that handler is to be invoked with two arguments.
    XIO_DECL void invocation_begin(
                    const xio::error_code &ec, const char *arg);

                // Record that handler invocation has ended.
    XIO_DECL void invocation_end();

            private:
                friend class handler_tracking;
                uint64_t id_;
                bool invoked_;
                completion *next_;
            };

            // Record an operation that is not directly associated with a handler.
  XIO_DECL static void operation(execution_context &context,
                                  const char *object_type, void *object,
                                  uintmax_t native_handle, const char *op_name);

            // Record that a descriptor has been registered with the reactor.
  XIO_DECL static void reactor_registration(execution_context &context,
                                             uintmax_t native_handle, uintmax_t registration);

            // Record that a descriptor has been deregistered from the reactor.
  XIO_DECL static void reactor_deregistration(execution_context &context,
                                               uintmax_t native_handle, uintmax_t registration);

            // Record a reactor-based operation that is associated with a handler.
  XIO_DECL static void reactor_events(execution_context &context,
                                       uintmax_t registration, unsigned events);

            // Record a reactor-based operation that is associated with a handler.
  XIO_DECL static void reactor_operation(
                const tracked_handler &h, const char *op_name,
                const xio::error_code &ec);

            // Record a reactor-based operation that is associated with a handler.
  XIO_DECL static void reactor_operation(
                const tracked_handler &h, const char *op_name,
                const xio::error_code &ec, std::size_t bytes_transferred);

            // Write a line of output.
  XIO_DECL static void write_line(const char *format, ...);

        private:
            struct tracking_state;

  XIO_DECL static tracking_state *get_state();
        };

# define XIO_INHERIT_TRACKED_HANDLER \
  : public xio::detail::handler_tracking::tracked_handler

# define XIO_ALSO_INHERIT_TRACKED_HANDLER \
  , public xio::detail::handler_tracking::tracked_handler

# define XIO_HANDLER_TRACKING_INIT \
  xio::detail::handler_tracking::init()

# define XIO_HANDLER_LOCATION(args) \
  xio::detail::handler_tracking::location tracked_location args

# define XIO_HANDLER_CREATION(args) \
  xio::detail::handler_tracking::creation args

# define XIO_HANDLER_COMPLETION(args) \
  xio::detail::handler_tracking::completion tracked_completion args

# define XIO_HANDLER_INVOCATION_BEGIN(args) \
  tracked_completion.invocation_begin args

# define XIO_HANDLER_INVOCATION_END \
  tracked_completion.invocation_end()

# define XIO_HANDLER_OPERATION(args) \
  xio::detail::handler_tracking::operation args

# define XIO_HANDLER_REACTOR_REGISTRATION(args) \
  xio::detail::handler_tracking::reactor_registration args

# define XIO_HANDLER_REACTOR_DEREGISTRATION(args) \
  xio::detail::handler_tracking::reactor_deregistration args

# define XIO_HANDLER_REACTOR_READ_EVENT 1
# define XIO_HANDLER_REACTOR_WRITE_EVENT 2
# define XIO_HANDLER_REACTOR_ERROR_EVENT 4

# define XIO_HANDLER_REACTOR_EVENTS(args) \
  xio::detail::handler_tracking::reactor_events args

# define XIO_HANDLER_REACTOR_OPERATION(args) \
  xio::detail::handler_tracking::reactor_operation args

#else // defined(XIO_ENABLE_HANDLER_TRACKING)

# define XIO_INHERIT_TRACKED_HANDLER
# define XIO_ALSO_INHERIT_TRACKED_HANDLER
# define XIO_HANDLER_TRACKING_INIT (void)0
# define XIO_HANDLER_LOCATION(loc) (void)0
# define XIO_HANDLER_CREATION(args) (void)0
# define XIO_HANDLER_COMPLETION(args) (void)0
# define XIO_HANDLER_INVOCATION_BEGIN(args) (void)0
# define XIO_HANDLER_INVOCATION_END (void)0
# define XIO_HANDLER_OPERATION(args) (void)0
# define XIO_HANDLER_REACTOR_REGISTRATION(args) (void)0
# define XIO_HANDLER_REACTOR_DEREGISTRATION(args) (void)0
# define XIO_HANDLER_REACTOR_READ_EVENT 0
# define XIO_HANDLER_REACTOR_WRITE_EVENT 0
# define XIO_HANDLER_REACTOR_ERROR_EVENT 0
# define XIO_HANDLER_REACTOR_EVENTS(args) (void)0
# define XIO_HANDLER_REACTOR_OPERATION(args) (void)0

#endif // defined(XIO_ENABLE_HANDLER_TRACKING)
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>


#endif // XIO_DETAIL_HANDLER_TRACKING_HPP

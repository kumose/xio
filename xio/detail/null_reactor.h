//
// detail/null_reactor.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_NULL_REACTOR_HPP
#define XIO_DETAIL_NULL_REACTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_IOCP) \
  || defined(XIO_WINDOWS_RUNTIME) \
  || defined(XIO_HAS_IO_URING_AS_DEFAULT)

#include <xio/detail/scheduler_operation.h>
#include <xio/detail/scheduler_task.h>
#include <xio/execution_context.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        class null_reactor
                : public execution_context_service_base<null_reactor>,
                  public scheduler_task {
        public:
            struct per_descriptor_data {
            };

            // Constructor.
            null_reactor(xio::execution_context &ctx)
                : execution_context_service_base<null_reactor>(ctx) {
            }

            // Destructor.
            ~null_reactor() {
            }

            // Initialise the task.
            void init_task() {
            }

            // Destroy all user-defined handler objects owned by the service.
            void shutdown() {
            }

            // No-op because should never be called.
            void run(long /*usec*/, op_queue<scheduler_operation> & /*ops*/) {
            }

            // No-op.
            void interrupt() {
            }
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(XIO_HAS_IOCP)
//   || defined(XIO_WINDOWS_RUNTIME)
//   || defined(XIO_HAS_IO_URING_AS_DEFAULT)

#endif // XIO_DETAIL_NULL_REACTOR_HPP

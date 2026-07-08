//
// detail/scheduler_operation.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_SCHEDULER_OPERATION_HPP
#define XIO_DETAIL_SCHEDULER_OPERATION_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/error_code.h>
#include <xio/detail/handler_tracking.h>
#include <xio/detail/op_queue.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        class scheduler;

        // Base class for all operations. A function pointer is used instead of virtual
        // functions to avoid the associated overhead.
        class scheduler_operation XIO_INHERIT_TRACKED_HANDLER {
        public:
            typedef scheduler_operation operation_type;

            void complete(void *owner, const xio::error_code &ec,
                          std::size_t bytes_transferred) {
                func_(owner, this, ec, bytes_transferred);
            }

            void destroy() {
                func_(0, this, xio::error_code(), 0);
            }

        protected:
            typedef void (*func_type)(void *,
                                      scheduler_operation *,
                                      const xio::error_code &, std::size_t);

            scheduler_operation(func_type func)
                : next_(0),
                  func_(func),
                  task_result_(0) {
            }

            // Prevents deletion through this type.
            ~scheduler_operation() {
            }

        private:
            friend class op_queue_access;
            scheduler_operation *next_;
            func_type func_;

        protected:
            friend class scheduler;
            unsigned int task_result_; // Passed into bytes transferred.
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_DETAIL_SCHEDULER_OPERATION_HPP

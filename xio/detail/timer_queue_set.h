//
// detail/timer_queue_set.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_TIMER_QUEUE_SET_HPP
#define XIO_DETAIL_TIMER_QUEUE_SET_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/timer_queue_base.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        class timer_queue_set {
        public:
            // Constructor.
            XIO_DECL timer_queue_set();

            // Add a timer queue to the set.
  XIO_DECL void insert(timer_queue_base *q);

            // Remove a timer queue from the set.
  XIO_DECL void erase(timer_queue_base *q);

            // Determine whether all queues are empty.
  XIO_DECL bool all_empty() const;

            // Get the wait duration in milliseconds.
  XIO_DECL long wait_duration_msec(long max_duration) const;

            // Get the wait duration in microseconds.
  XIO_DECL long wait_duration_usec(long max_duration) const;

            // Dequeue all ready timers.
  XIO_DECL void get_ready_timers(op_queue<operation> &ops);

            // Dequeue all timers.
  XIO_DECL void get_all_timers(op_queue<operation> &ops);

        private:
            timer_queue_base *first_;
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>


#endif // XIO_DETAIL_TIMER_QUEUE_SET_HPP

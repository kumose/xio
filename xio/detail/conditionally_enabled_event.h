//
// detail/conditionally_enabled_event.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_CONDITIONALLY_ENABLED_EVENT_HPP
#define ASIO_DETAIL_CONDITIONALLY_ENABLED_EVENT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/conditionally_enabled_mutex.h>
#include <xio/detail/event.h>
#include <xio/detail/mutex.h>
#include <xio/detail/noncopyable.h>
#include <xio/detail/null_event.h>
#include <xio/detail/scoped_lock.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        // Mutex adapter used to conditionally enable or disable locking.
        class conditionally_enabled_event
                : private noncopyable {
        public:
            // Constructor.
            conditionally_enabled_event() {
            }

            // Destructor.
            ~conditionally_enabled_event() {
            }

            // Signal the event. (Retained for backward compatibility.)
            void signal(conditionally_enabled_mutex<mutex>::scoped_lock &lock) {
                if (lock.mutex_.enabled())
                    event_.signal(lock);
            }

            // Signal all waiters.
            void signal_all(conditionally_enabled_mutex<mutex>::scoped_lock &lock) {
                if (lock.mutex_.enabled())
                    event_.signal_all(lock);
            }

            // Unlock the mutex and signal one waiter.
            void unlock_and_signal_one(
                conditionally_enabled_mutex<mutex>::scoped_lock &lock) {
                if (lock.mutex_.enabled())
                    event_.unlock_and_signal_one(lock);
            }

            // Unlock the mutex and signal one waiter who may destroy us.
            void unlock_and_signal_one_for_destruction(
                conditionally_enabled_mutex<mutex>::scoped_lock &lock) {
                if (lock.mutex_.enabled())
                    event_.unlock_and_signal_one(lock);
            }

            // If there's a waiter, unlock the mutex and signal it.
            bool maybe_unlock_and_signal_one(
                conditionally_enabled_mutex<mutex>::scoped_lock &lock) {
                if (lock.mutex_.enabled())
                    return event_.maybe_unlock_and_signal_one(lock);
                else
                    return false;
            }

            // Reset the event.
            void clear(conditionally_enabled_mutex<mutex>::scoped_lock &lock) {
                if (lock.mutex_.enabled())
                    event_.clear(lock);
            }

            // Wait for the event to become signalled.
            void wait(conditionally_enabled_mutex<mutex>::scoped_lock &lock) {
                if (lock.mutex_.enabled())
                    event_.wait(lock);
                else
                    null_event().wait(lock);
            }

            // Timed wait for the event to become signalled.
            bool wait_for_usec(
                conditionally_enabled_mutex<mutex>::scoped_lock &lock, long usec) {
                if (lock.mutex_.enabled())
                    return event_.wait_for_usec(lock, usec);
                else
                    return null_event().wait_for_usec(lock, usec);
            }

        private:
            xio::detail::event event_;
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_DETAIL_CONDITIONALLY_ENABLED_EVENT_HPP

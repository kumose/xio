//
// detail/win_event.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_WIN_EVENT_HPP
#define ASIO_DETAIL_WIN_EVENT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(ASIO_WINDOWS)

#include <cstddef>
# include <cassert>
#include <xio/detail/noncopyable.h>
#include <xio/detail/socket_types.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        class win_event
                : private noncopyable {
        public:
            // Constructor.
            ASIO_DECL win_event();

            // Destructor.
            ASIO_DECL ~win_event();

            // Signal the event. (Retained for backward compatibility.)
            template<typename Lock>
            void signal(Lock &lock) {
                this->signal_all(lock);
            }

            // Signal all waiters.
            template<typename Lock>
            void signal_all(Lock &lock) {
                assert(lock.locked());
                (void) lock;
                state_ |= 1;
                ::SetEvent(events_[0]);
            }

            // Unlock the mutex and signal one waiter.
            template<typename Lock>
            void unlock_and_signal_one(Lock &lock) {
                assert(lock.locked());
                state_ |= 1;
                bool have_waiters = (state_ > 1);
                lock.unlock();
                if (have_waiters)
                    ::SetEvent(events_[1]);
            }

            // Unlock the mutex and signal one waiter who may destroy us.
            template<typename Lock>
            void unlock_and_signal_one_for_destruction(Lock &lock) {
                assert(lock.locked());
                state_ |= 1;
                bool have_waiters = (state_ > 1);
                if (have_waiters)
                    ::SetEvent(events_[1]);
                lock.unlock();
            }

            // If there's a waiter, unlock the mutex and signal it.
            template<typename Lock>
            bool maybe_unlock_and_signal_one(Lock &lock) {
                assert(lock.locked());
                state_ |= 1;
                if (state_ > 1) {
                    lock.unlock();
                    ::SetEvent(events_[1]);
                    return true;
                }
                return false;
            }

            // Reset the event.
            template<typename Lock>
            void clear(Lock &lock) {
                assert(lock.locked());
                (void) lock;
                ::ResetEvent(events_[0]);
                state_ &= ~std::size_t(1);
            }

            // Wait for the event to become signalled.
            template<typename Lock>
            void wait(Lock &lock) {
                assert(lock.locked());
                while ((state_ & 1) == 0) {
                    state_ += 2;
                    lock.unlock();
#if defined(ASIO_WINDOWS_APP)
::WaitForMultipleObjectsEx (2, events_, false, INFINITE, false);
#else // defined(ASIO_WINDOWS_APP)
::WaitForMultipleObjects (2, events_, false, INFINITE);
#endif // defined(ASIO_WINDOWS_APP)
lock.lock();
state_-= 2;
    }
  }

// Timed wait for the event to become signalled.
template<typename Lock>
bool wait_for_usec(Lock &lock, long usec) {
    assert(lock.locked());
    if ((state_ & 1) == 0) {
        state_ += 2;
        lock.unlock();
        DWORD msec = usec > 0 ? (usec < 1000 ? 1 : usec / 1000) : 0;
#if defined(ASIO_WINDOWS_APP)
::WaitForMultipleObjectsEx (2, events_, false, msec, false);
#else // defined(ASIO_WINDOWS_APP)
::WaitForMultipleObjects (2, events_, false, msec);
#endif // defined(ASIO_WINDOWS_APP)
lock.lock();
state_-= 2;
    }
    return (state_ &1) != 0;
  }

private:
HANDLE events_[2];
std::size_t state_;
};

} // namespace detail
} // namespace xio

#include <xio/detail/pop_options.h>


#endif // defined(ASIO_WINDOWS)

#endif // ASIO_DETAIL_WIN_EVENT_HPP

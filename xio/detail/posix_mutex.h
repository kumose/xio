//
// detail/posix_mutex.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_POSIX_MUTEX_HPP
#define XIO_DETAIL_POSIX_MUTEX_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_PTHREADS)

#include <pthread.h>
#include <xio/detail/noncopyable.h>
#include <xio/detail/scoped_lock.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        class posix_event;

        class posix_mutex
                : private noncopyable {
        public:
            typedef xio::detail::scoped_lock<posix_mutex> scoped_lock;

            // Constructor.
            XIO_DECL posix_mutex();

            // Destructor.
            ~posix_mutex() {
                ::pthread_mutex_destroy(&mutex_); // Ignore EBUSY.
            }

            // Try to lock the mutex.
            bool try_lock() {
                return ::pthread_mutex_trylock(&mutex_) == 0; // Ignore EINVAL.
            }

            // Lock the mutex.
            void lock() {
                (void) ::pthread_mutex_lock(&mutex_); // Ignore EINVAL.
            }

            // Unlock the mutex.
            void unlock() {
                (void) ::pthread_mutex_unlock(&mutex_); // Ignore EINVAL.
            }

        private:
            friend class posix_event;
            ::pthread_mutex_t mutex_;
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>


#endif // defined(XIO_HAS_PTHREADS)

#endif // XIO_DETAIL_POSIX_MUTEX_HPP

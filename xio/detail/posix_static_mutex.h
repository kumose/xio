//
// detail/posix_static_mutex.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_POSIX_STATIC_MUTEX_HPP
#define XIO_DETAIL_POSIX_STATIC_MUTEX_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_PTHREADS)

#include <pthread.h>
#include <xio/detail/scoped_lock.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        struct posix_static_mutex {
            typedef xio::detail::scoped_lock<posix_static_mutex> scoped_lock;

            // Initialise the mutex.
            void init() {
                // Nothing to do.
            }

            // Lock the mutex.
            void lock() {
                (void) ::pthread_mutex_lock(&mutex_); // Ignore EINVAL.
            }

            // Unlock the mutex.
            void unlock() {
                (void) ::pthread_mutex_unlock(&mutex_); // Ignore EINVAL.
            }

            ::pthread_mutex_t mutex_;
        };

#define XIO_POSIX_STATIC_MUTEX_INIT { PTHREAD_MUTEX_INITIALIZER }
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(XIO_HAS_PTHREADS)

#endif // XIO_DETAIL_POSIX_STATIC_MUTEX_HPP

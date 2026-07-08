//
// detail/null_static_mutex.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_NULL_STATIC_MUTEX_HPP
#define XIO_DETAIL_NULL_STATIC_MUTEX_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(XIO_HAS_THREADS)

#include <xio/detail/scoped_lock.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        struct null_static_mutex {
            typedef xio::detail::scoped_lock<null_static_mutex> scoped_lock;

            // Initialise the mutex.
            void init() {
            }

            // Try to lock the mutex without blocking.
            bool try_lock() {
                return true;
            }

            // Lock the mutex.
            void lock() {
            }

            // Unlock the mutex.
            void unlock() {
            }

            int unused_;
        };

#define XIO_NULL_STATIC_MUTEX_INIT { 0 }
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // !defined(XIO_HAS_THREADS)

#endif // XIO_DETAIL_NULL_STATIC_MUTEX_HPP

//
// detail/null_mutex.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_NULL_MUTEX_HPP
#define XIO_DETAIL_NULL_MUTEX_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#include <xio/detail/noncopyable.h>
#include <xio/detail/scoped_lock.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        class null_mutex
                : private noncopyable {
        public:
            typedef xio::detail::scoped_lock<null_mutex> scoped_lock;

            // Constructor.
            null_mutex() {
            }

            // Destructor.
            ~null_mutex() {
            }

            // Try to lock the mutex.
            bool try_lock() {
                return true;
            }

            // Lock the mutex.
            void lock() {
            }

            // Unlock the mutex.
            void unlock() {
            }
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_DETAIL_NULL_MUTEX_HPP

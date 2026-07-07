//
// detail/win_critsec_mutex.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_WIN_CRITSEC_MUTEX_HPP
#define ASIO_DETAIL_WIN_CRITSEC_MUTEX_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(ASIO_WINDOWS)

#include <xio/detail/noncopyable.h>
#include <xio/detail/scoped_lock.h>
#include <xio/detail/socket_types.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        class win_critsec_mutex
                : private noncopyable {
        public:
            typedef xio::detail::scoped_lock<win_critsec_mutex> scoped_lock;

            // Constructor.
            ASIO_DECL win_critsec_mutex();

            // Destructor.
            ~win_critsec_mutex() {
                ::DeleteCriticalSection(&crit_section_);
            }

            // Try to lock the mutex.
            bool try_lock() {
                return ::TryEnterCriticalSection(&crit_section_) != 0;
            }

            // Lock the mutex.
            void lock() {
                ::EnterCriticalSection(&crit_section_);
            }

            // Unlock the mutex.
            void unlock() {
                ::LeaveCriticalSection(&crit_section_);
            }

        private:
            // Initialisation must be performed in a separate function to the constructor
            // since the compiler does not support the use of structured exceptions and
            // C++ exceptions in the same function.
  ASIO_DECL int do_init();

            ::CRITICAL_SECTION crit_section_;
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>


#endif // defined(ASIO_WINDOWS)

#endif // ASIO_DETAIL_WIN_CRITSEC_MUTEX_HPP

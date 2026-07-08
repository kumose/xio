//
// detail/null_signal_blocker.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_NULL_SIGNAL_BLOCKER_HPP
#define XIO_DETAIL_NULL_SIGNAL_BLOCKER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(XIO_HAS_THREADS) \
  || defined(XIO_WINDOWS) \
  || defined(XIO_WINDOWS_RUNTIME) \
  || defined(XIO_CYGWIN_W32_SOCKETS) \
  || defined(__SYMBIAN32__)

#include <xio/detail/noncopyable.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        class null_signal_blocker
                : private noncopyable {
        public:
            // Constructor blocks all signals for the calling thread.
            null_signal_blocker() {
            }

            // Destructor restores the previous signal mask.
            ~null_signal_blocker() {
            }

            // Block all signals for the calling thread.
            void block() {
            }

            // Restore the previous signal mask.
            void unblock() {
            }
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // !defined(XIO_HAS_THREADS)
// || defined(XIO_WINDOWS)
// || defined(XIO_WINDOWS_RUNTIME)
// || defined(XIO_CYGWIN_W32_SOCKETS)
// || defined(__SYMBIAN32__)

#endif // XIO_DETAIL_NULL_SIGNAL_BLOCKER_HPP

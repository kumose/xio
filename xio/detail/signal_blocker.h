//
// detail/signal_blocker.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_SIGNAL_BLOCKER_HPP
#define XIO_DETAIL_SIGNAL_BLOCKER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(XIO_HAS_THREADS) || defined(XIO_WINDOWS) \
  || defined(XIO_WINDOWS_RUNTIME) \
  || defined(XIO_CYGWIN_W32_SOCKETS) || defined(__SYMBIAN32__)
#include <xio/detail/null_signal_blocker.h>
#elif defined(XIO_HAS_PTHREADS)
#include <xio/detail/posix_signal_blocker.h>
#else
# error Only Windows and POSIX are supported!
#endif

namespace xio {


    namespace detail {
#if !defined(XIO_HAS_THREADS) || defined(XIO_WINDOWS) \
  || defined(XIO_WINDOWS_RUNTIME) \
  || defined(XIO_CYGWIN_W32_SOCKETS) || defined(__SYMBIAN32__)
        typedef null_signal_blocker signal_blocker;
#elif defined(XIO_HAS_PTHREADS)
        typedef posix_signal_blocker signal_blocker;
#endif
    } // namespace detail

} // namespace xio

#endif // XIO_DETAIL_SIGNAL_BLOCKER_HPP

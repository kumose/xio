//
// detail/mutex.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_MUTEX_HPP
#define XIO_DETAIL_MUTEX_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(XIO_HAS_THREADS)
#include <xio/detail/null_mutex.h>
#elif defined(XIO_WINDOWS) && defined(XIO_HAS_WINDOWS_SRWLOCK)
#include <xio/detail/win_mutex.h>
#elif defined(XIO_WINDOWS)
#include <xio/detail/win_critsec_mutex.h>
#elif defined(XIO_HAS_PTHREADS)
#include <xio/detail/posix_mutex.h>
#else
#include <xio/detail/std_mutex.h>
#endif

namespace xio {


    namespace detail {
#if !defined(XIO_HAS_THREADS)
        typedef null_mutex mutex;
#elif defined(XIO_WINDOWS) && defined(XIO_HAS_WINDOWS_SRWLOCK)
        typedef win_mutex mutex;
#elif defined(XIO_WINDOWS)
        typedef win_critsec_mutex mutex;
#elif defined(XIO_HAS_PTHREADS)
        typedef posix_mutex mutex;
#else
        typedef std_mutex mutex;
#endif
    } // namespace detail

} // namespace xio

#endif // XIO_DETAIL_MUTEX_HPP

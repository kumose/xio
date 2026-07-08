//
// detail/static_mutex.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_STATIC_MUTEX_HPP
#define XIO_DETAIL_STATIC_MUTEX_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(XIO_HAS_THREADS)
#include <xio/detail/null_static_mutex.h>
#elif defined(XIO_WINDOWS)
#include <xio/detail/win_static_mutex.h>
#elif defined(XIO_HAS_PTHREADS)
#include <xio/detail/posix_static_mutex.h>
#else
#include <xio/detail/std_static_mutex.h>
#endif

namespace xio {


    namespace detail {
#if !defined(XIO_HAS_THREADS)
        typedef null_static_mutex static_mutex;
# define XIO_STATIC_MUTEX_INIT XIO_NULL_STATIC_MUTEX_INIT
#elif defined(XIO_WINDOWS)
        typedef win_static_mutex static_mutex;
# define XIO_STATIC_MUTEX_INIT XIO_WIN_STATIC_MUTEX_INIT
#elif defined(XIO_HAS_PTHREADS)
        typedef posix_static_mutex static_mutex;
# define XIO_STATIC_MUTEX_INIT XIO_POSIX_STATIC_MUTEX_INIT
#else
        typedef std_static_mutex static_mutex;
# define XIO_STATIC_MUTEX_INIT XIO_STD_STATIC_MUTEX_INIT
#endif
    } // namespace detail

} // namespace xio

#endif // XIO_DETAIL_STATIC_MUTEX_HPP

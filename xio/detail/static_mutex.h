//
// detail/static_mutex.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_STATIC_MUTEX_HPP
#define ASIO_DETAIL_STATIC_MUTEX_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(ASIO_HAS_THREADS)
#include <xio/detail/null_static_mutex.h>
#elif defined(ASIO_WINDOWS)
#include <xio/detail/win_static_mutex.h>
#elif defined(ASIO_HAS_PTHREADS)
#include <xio/detail/posix_static_mutex.h>
#else
#include <xio/detail/std_static_mutex.h>
#endif

namespace xio {


    namespace detail {
#if !defined(ASIO_HAS_THREADS)
        typedef null_static_mutex static_mutex;
# define ASIO_STATIC_MUTEX_INIT ASIO_NULL_STATIC_MUTEX_INIT
#elif defined(ASIO_WINDOWS)
        typedef win_static_mutex static_mutex;
# define ASIO_STATIC_MUTEX_INIT ASIO_WIN_STATIC_MUTEX_INIT
#elif defined(ASIO_HAS_PTHREADS)
        typedef posix_static_mutex static_mutex;
# define ASIO_STATIC_MUTEX_INIT ASIO_POSIX_STATIC_MUTEX_INIT
#else
        typedef std_static_mutex static_mutex;
# define ASIO_STATIC_MUTEX_INIT ASIO_STD_STATIC_MUTEX_INIT
#endif
    } // namespace detail

} // namespace xio

#endif // ASIO_DETAIL_STATIC_MUTEX_HPP

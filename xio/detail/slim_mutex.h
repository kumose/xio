//
// detail/slim_mutex.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_SLIM_MUTEX_HPP
#define XIO_DETAIL_SLIM_MUTEX_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(XIO_HAS_THREADS)
#include <xio/detail/null_mutex.h>
#elif defined(XIO_HAS_FUTEX)
#include <xio/detail/futex_slim_mutex.h>
#elif defined(XIO_HAS_STD_ATOMIC_WAIT)
#include <xio/detail/atomic_slim_mutex.h>
#else
#include <xio/detail/mutex.h>
#endif

namespace xio {


    namespace detail {
#if !defined(XIO_HAS_THREADS)
        typedef null_mutex slim_mutex;
#elif defined(XIO_HAS_FUTEX)
        typedef futex_slim_mutex slim_mutex;
#elif defined(XIO_HAS_STD_ATOMIC_WAIT)
        typedef atomic_slim_mutex slim_mutex;
#else
        typedef mutex slim_mutex;
#endif
    } // namespace detail

} // namespace xio

#endif // XIO_DETAIL_SLIM_MUTEX_HPP

//
// detail/event.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_EVENT_HPP
#define ASIO_DETAIL_EVENT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(ASIO_HAS_THREADS)
#include <xio/detail/null_event.h>
#elif defined(ASIO_WINDOWS)
#include <xio/detail/win_event.h>
#elif defined(ASIO_HAS_PTHREADS)
#include <xio/detail/posix_event.h>
#else
#include <xio/detail/std_event.h>
#endif

namespace xio {


    namespace detail {
#if !defined(ASIO_HAS_THREADS)
        typedef null_event event;
#elif defined(ASIO_WINDOWS)
        typedef win_event event;
#elif defined(ASIO_HAS_PTHREADS)
        typedef posix_event event;
#else
        typedef std_event event;
#endif
    } // namespace detail

} // namespace xio

#endif // ASIO_DETAIL_EVENT_HPP

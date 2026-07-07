//
// detail/reactor.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_REACTOR_HPP
#define ASIO_DETAIL_REACTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(ASIO_HAS_IOCP) || defined(ASIO_WINDOWS_RUNTIME)
#include <xio/detail/null_reactor.h>
#elif defined(ASIO_HAS_IO_URING_AS_DEFAULT)
#include <xio/detail/null_reactor.h>
#elif defined(ASIO_HAS_EPOLL)
#include <xio/detail/epoll_reactor.h>
#elif defined(ASIO_HAS_KQUEUE)
#include <xio/detail/kqueue_reactor.h>
#elif defined(ASIO_HAS_DEV_POLL)
#include <xio/detail/dev_poll_reactor.h>
#else
#include <xio/detail/select_reactor.h>
#endif

namespace xio {


    namespace detail {
#if defined(ASIO_HAS_IOCP) || defined(ASIO_WINDOWS_RUNTIME)
        typedef null_reactor reactor;
#elif defined(ASIO_HAS_IO_URING_AS_DEFAULT)
        typedef null_reactor reactor;
#elif defined(ASIO_HAS_EPOLL)
        typedef epoll_reactor reactor;
#elif defined(ASIO_HAS_KQUEUE)
        typedef kqueue_reactor reactor;
#elif defined(ASIO_HAS_DEV_POLL)
        typedef dev_poll_reactor reactor;
#else
        typedef select_reactor reactor;
#endif
    } // namespace detail

} // namespace xio

#endif // ASIO_DETAIL_REACTOR_HPP

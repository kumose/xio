//
// detail/timer_scheduler.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_TIMER_SCHEDULER_HPP
#define XIO_DETAIL_TIMER_SCHEDULER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/timer_scheduler_fwd.h>

#if defined(XIO_WINDOWS_RUNTIME)
#include <xio/detail/winrt_timer_scheduler.h>
#elif defined(XIO_HAS_IOCP)
#include <xio/detail/win_iocp_io_context.h>
#elif defined(XIO_HAS_IO_URING_AS_DEFAULT)
#include <xio/detail/io_uring_service.h>
#elif defined(XIO_HAS_EPOLL)
#include <xio/detail/epoll_reactor.h>
#elif defined(XIO_HAS_KQUEUE)
#include <xio/detail/kqueue_reactor.h>
#elif defined(XIO_HAS_DEV_POLL)
#include <xio/detail/dev_poll_reactor.h>
#else
#include <xio/detail/select_reactor.h>
#endif

#endif // XIO_DETAIL_TIMER_SCHEDULER_HPP

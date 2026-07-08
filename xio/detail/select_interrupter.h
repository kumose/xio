//
// detail/select_interrupter.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_SELECT_INTERRUPTER_HPP
#define XIO_DETAIL_SELECT_INTERRUPTER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(XIO_WINDOWS_RUNTIME)

#if defined(XIO_WINDOWS) \
  || defined(XIO_CYGWIN_W32_SOCKETS) || defined(__SYMBIAN32__)
#include <xio/detail/socket_select_interrupter.h>
#elif defined(XIO_HAS_EVENTFD)
#include <xio/detail/eventfd_select_interrupter.h>
#else
#include <xio/detail/pipe_select_interrupter.h>
#endif

namespace xio {


    namespace detail {
#if defined(XIO_WINDOWS) \
  || defined(XIO_CYGWIN_W32_SOCKETS) || defined(__SYMBIAN32__)
        typedef socket_select_interrupter select_interrupter;
#elif defined(XIO_HAS_EVENTFD)
        typedef eventfd_select_interrupter select_interrupter;
#else
        typedef pipe_select_interrupter select_interrupter;
#endif
    } // namespace detail

} // namespace xio

#endif // !defined(XIO_WINDOWS_RUNTIME)

#endif // XIO_DETAIL_SELECT_INTERRUPTER_HPP
